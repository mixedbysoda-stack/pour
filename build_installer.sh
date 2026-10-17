#!/bin/bash
set -euo pipefail

# ============================================================================
# Pour - macOS Installer Builder
# Component pkgs -> Distribution -> productbuild -> sign -> notarize -> staple
#
# Same structure as TALLBOY's scripts/build_installer.sh. v1.0.0 shipped as a
# flat pkgbuild pkg with no Distribution file, so Installer.app ran it under
# Rosetta on Apple Silicon (Q-57). The Distribution below declares
# hostArchitectures="arm64,x86_64", and scripts/check_pkg_arch.sh gates both
# the unsigned and the final pkg so a regression cannot ship.
#
# Usage: ./build_installer.sh [version] [--demo]
#   version defaults to project() VERSION in CMakeLists.txt and must match it.
#   Paid: ./distribute.sh && ./build_installer.sh
#   Demo: ./distribute.sh --demo && ./build_installer.sh --demo
# ============================================================================

PRODUCT="Pour"            # name shown in the wizard
FILE_PREFIX="Pour"        # pkg filename prefix
DIST_SLUG="pour"          # dist/<slug>-v<ver>[-demo]
BUNDLE="Pour"             # Pour.vst3 / .component / .aaxplugin
IDENT_ROOT="com.carbonatedaudio.pour"

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
CMAKE_VERSION="$(sed -n 's/^project([A-Za-z]* VERSION \([0-9.]*\).*/\1/p' "$PROJECT_DIR/CMakeLists.txt")"

VERSION=""; DEMO=0
for a in "$@"; do
    case "$a" in
        --demo) DEMO=1;;
        -*) echo "unknown arg: $a"; exit 1;;
        *) VERSION="$a";;
    esac
done
VERSION="${VERSION:-$CMAKE_VERSION}"
if [ "$VERSION" != "$CMAKE_VERSION" ]; then
    echo "ERROR: version $VERSION does not match CMakeLists.txt ($CMAKE_VERSION). Bump CMake first."
    exit 1
fi

SUFFIX=""; DSUF=""; IDENT_BASE="$IDENT_ROOT"
if [ "$DEMO" = 1 ]; then SUFFIX="-Demo"; DSUF="-demo"; IDENT_BASE="$IDENT_ROOT.demo"; fi

DIST_DIR="$PROJECT_DIR/dist/${DIST_SLUG}-v${VERSION}${DSUF}"
INSTALLER_DIR="$PROJECT_DIR/installer"
RESOURCES="$INSTALLER_DIR/Resources"
PKG_SIGNED="$INSTALLER_DIR/${FILE_PREFIX}-v${VERSION}${SUFFIX}-Installer.pkg"
GATE="$PROJECT_DIR/scripts/check_pkg_arch.sh"

SIGN_ID_INSTALLER="Developer ID Installer: Miguel Silverio (K7FRP77ZZK)"
NOTARY_PROFILE="carbonator-notary"

echo "=== ${PRODUCT} v${VERSION}${SUFFIX} - macOS Installer ==="
echo ""

if [ ! -d "$DIST_DIR" ]; then
    echo "ERROR: Distribution folder not found: $DIST_DIR"
    if [ "$DEMO" = 1 ]; then echo "Run ./distribute.sh ${VERSION} --demo first."; else echo "Run ./distribute.sh ${VERSION} first."; fi
    exit 1
fi

# Branded wizard pages are required by the Shipping Playbook - fail without them.
for f in sidebar.png welcome.html license.html conclusion.html conclusion-demo.html; do
    [ -f "$RESOURCES/$f" ] || { echo "ERROR: missing installer resource $RESOURCES/$f"; exit 1; }
done

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
PKGS=()

# Resolve @PRODUCT@ / @VERSION@ and pick the paid or demo conclusion page.
RES="$WORK/Resources"; mkdir -p "$RES"
cp "$RESOURCES/sidebar.png" "$RESOURCES/license.html" "$RES/"
CONC="conclusion.html"; [ "$DEMO" = 1 ] && CONC="conclusion-demo.html"
sed -e "s/@PRODUCT@/$PRODUCT/g" -e "s/@VERSION@/$VERSION/g" "$RESOURCES/welcome.html" > "$RES/welcome.html"
sed -e "s/@PRODUCT@/$PRODUCT/g" -e "s/@VERSION@/$VERSION/g" "$RESOURCES/$CONC"        > "$RES/conclusion.html"

component() { # $1 bundle path, $2 install dir, $3 pkg suffix
    local src="$1" dest="$2" suf="$3"
    [ -d "$src" ] || { echo "ERROR: missing $suf bundle: $src"; exit 1; }
    local root="$WORK/root_$suf"
    mkdir -p "$root$dest"
    # ditto, never cp -R: cp corrupts the PACE symlink inside the AAX bundle.
    ditto "$src" "$root$dest/$(basename "$src")"
    pkgbuild --root "$root" --identifier "$IDENT_BASE.$suf" --version "$VERSION" \
             --install-location "/" "$WORK/$suf.pkg" > /dev/null
    PKGS+=("$suf")
    echo "  packaged $suf: $(basename "$src")"
}

# --- Step 1: Component packages ---
echo "[1/6] Building component packages..."
component "$DIST_DIR/VST3/${BUNDLE}.vst3"          "/Library/Audio/Plug-Ins/VST3"                     vst3
component "$DIST_DIR/AU/${BUNDLE}.component"       "/Library/Audio/Plug-Ins/Components"               au
component "$DIST_DIR/AAX/${BUNDLE}.aaxplugin"      "/Library/Application Support/Avid/Audio/Plug-Ins" aax
echo ""

# --- Step 2: Distribution + productbuild ---
echo "[2/6] Building product archive (Distribution, hostArchitectures=arm64,x86_64)..."
DIST="$WORK/distribution.xml"
TITLE="$PRODUCT"; [ "$DEMO" = 1 ] && TITLE="$PRODUCT Demo"
{
    echo '<?xml version="1.0" encoding="utf-8"?>'
    echo '<installer-gui-script minSpecVersion="1">'
    echo "    <title>$TITLE</title>"
    echo '    <welcome    file="welcome.html"/>'
    echo '    <license    file="license.html"/>'
    echo '    <conclusion file="conclusion.html"/>'
    echo '    <background          file="sidebar.png" alignment="bottomleft" scaling="proportional"/>'
    echo '    <background-darkAqua file="sidebar.png" alignment="bottomleft" scaling="proportional"/>'
    echo '    <options customize="never" require-scripts="false" hostArchitectures="arm64,x86_64"/>'
    echo '    <choices-outline>'
    echo '        <line choice="default">'
    for n in "${PKGS[@]}"; do echo "            <line choice=\"$IDENT_BASE.$n\"/>"; done
    echo '        </line>'
    echo '    </choices-outline>'
    echo '    <choice id="default"/>'
    for n in "${PKGS[@]}"; do
        echo "    <choice id=\"$IDENT_BASE.$n\" visible=\"false\"><pkg-ref id=\"$IDENT_BASE.$n\"/></choice>"
        echo "    <pkg-ref id=\"$IDENT_BASE.$n\" version=\"$VERSION\" onConclusion=\"none\">$n.pkg</pkg-ref>"
    done
    echo '</installer-gui-script>'
} > "$DIST"

PKG_UNSIGNED="$WORK/${FILE_PREFIX}-v${VERSION}${SUFFIX}-unsigned.pkg"
productbuild --distribution "$DIST" --resources "$RES" \
             --package-path "$WORK" "$PKG_UNSIGNED" > /dev/null
echo "  Built: $(basename "$PKG_UNSIGNED")"
echo ""

# --- Step 3: Release gate (before spending a notarization round-trip) ---
echo "[3/6] Release gate..."
"$GATE" "$PKG_UNSIGNED"
echo ""

# --- Step 4: Sign pkg ---
echo "[4/6] Signing package..."
rm -f "$PKG_SIGNED"
productsign --sign "$SIGN_ID_INSTALLER" "$PKG_UNSIGNED" "$PKG_SIGNED"
echo "  Signed: $(basename "$PKG_SIGNED")"
echo ""

# --- Step 5: Notarize ---
echo "[5/6] Submitting for Apple notarization..."
xcrun notarytool submit "$PKG_SIGNED" --keychain-profile "$NOTARY_PROFILE" --wait 2>&1 | tail -4
echo ""

# --- Step 6: Staple ---
echo "[6/6] Stapling notarization ticket..."
xcrun stapler staple "$PKG_SIGNED"
echo ""

# --- Verification ---
echo "=== Verification ==="
"$GATE" "$PKG_SIGNED"
pkgutil --check-signature "$PKG_SIGNED" 2>&1 | head -5
STAPLE_RESULT=$(xcrun stapler validate "$PKG_SIGNED" 2>&1 | grep -o "worked" || echo "FAILED")
echo "Notarization staple: $STAPLE_RESULT"
[ "$STAPLE_RESULT" = "worked" ] || { echo "ERROR: staple did not validate"; exit 1; }
if spctl --assess --type install "$PKG_SIGNED" 2>&1; then
    echo "Gatekeeper: PASSED"
else
    echo "Gatekeeper: FAILED"; exit 1
fi
echo ""
echo "Installer: $PKG_SIGNED ($(du -h "$PKG_SIGNED" | cut -f1))"
echo "=== Done! ${PRODUCT} v${VERSION}${SUFFIX} installer ready ==="
