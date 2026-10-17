#!/bin/bash
# check_pkg_arch.sh — release gate: refuse any installer pkg that would trigger
# the Rosetta prompt on Apple Silicon (Q-57).
#
# Installer.app runs a pkg under Rosetta unless its Distribution file declares
# hostArchitectures including arm64. A flat pkgbuild pkg has no Distribution at
# all, so it always prompts. Every binary we ship is already universal; the
# prompt is purely a packaging bug, and this check makes it impossible to ship.
#
# Usage: ./scripts/check_pkg_arch.sh path/to/Installer.pkg
# Exit 0 = pass, 1 = fail. Run it on the unsigned AND the final signed pkg.
set -euo pipefail

PKG="${1:-}"
[[ -n "$PKG" && -f "$PKG" ]] || { echo "usage: $0 <installer.pkg>"; exit 1; }

WORK="$(mktemp -d)"
trap 'rc=$?; rm -rf "$WORK"; exit $rc' EXIT
pkgutil --expand "$PKG" "$WORK/x"

fail() { echo "  RELEASE GATE FAIL ($(basename "$PKG")): $1"; exit 1; }

DIST="$WORK/x/Distribution"
[[ -f "$DIST" ]] || fail "no Distribution file - this is a flat pkgbuild pkg. Build it with productbuild --distribution."

OPTS="$(grep -o '<options[^>]*/>' "$DIST" || true)"
[[ -n "$OPTS" ]] || fail "Distribution has no <options/> element."

ARCHS="$(printf '%s' "$OPTS" | sed -n 's/.*hostArchitectures="\([^"]*\)".*/\1/p')"
[[ -n "$ARCHS" ]] || fail "hostArchitectures missing from <options/>: $OPTS"
[[ ",$ARCHS," == *",arm64,"* ]]  || fail "hostArchitectures=\"$ARCHS\" lacks arm64"
[[ ",$ARCHS," == *",x86_64,"* ]] || fail "hostArchitectures=\"$ARCHS\" lacks x86_64"

echo "  release gate PASS ($(basename "$PKG")): hostArchitectures=\"$ARCHS\""
