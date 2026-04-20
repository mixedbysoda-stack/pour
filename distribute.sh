#!/bin/bash
set -euo pipefail

# ============================================================================
# Pour — macOS Distribution Build
# Universal binary -> codesign -> notarize -> PACE AAX sign -> stage dist
# ============================================================================
# Usage: ./distribute.sh [version]
# Example: ./distribute.sh 1.0.0
# ============================================================================

VERSION="${1:-1.0.0}"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build-release"
ARTEFACTS="$BUILD_DIR/Pour_artefacts/Release"
DIST_DIR="$PROJECT_DIR/dist/pour-v${VERSION}"

SIGN_ID="45C26CF1655F48EBC8A21802BDA053514719E1F0"
NOTARY_PROFILE="carbonator-notary"

WRAPTOOL="/Applications/PACEAntiPiracy/Eden/Fusion/Versions/5/bin/wraptool"
PACE_ACCOUNT="sodanswishers"
PACE_WCGUID="5F45D2A0-3560-11F1-B6A4-00505692AD3E"

VST3="$ARTEFACTS/VST3/Pour.vst3"
AU="$ARTEFACTS/AU/Pour.component"
AAX="$ARTEFACTS/AAX/Pour.aaxplugin"
AAX_SIGNED="$ARTEFACTS/AAX/Pour_signed.aaxplugin"
STANDALONE="$ARTEFACTS/Standalone/Pour.app"

echo "=== Pour v${VERSION} — Distribution Build ==="
echo ""

# --- Step 1: Rebuild as Universal Binary ---
echo "[1/6] Building universal binary (arm64 + x86_64)..."
cd "$PROJECT_DIR"
cmake -B build-release \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="10.13" > /dev/null 2>&1
cmake --build build-release --target Pour_VST3 Pour_AU Pour_AAX Pour_Standalone --config Release --clean-first 2>&1 | tail -1

echo "  Verifying architectures..."
for bin in "$VST3/Contents/MacOS/Pour" "$AU/Contents/MacOS/Pour" \
           "$STANDALONE/Contents/MacOS/Pour" "$AAX/Contents/MacOS/Pour"; do
    ARCH=$(lipo -info "$bin" 2>/dev/null | grep -o "x86_64 arm64" || true)
    if [ "$ARCH" != "x86_64 arm64" ]; then
        echo "  ERROR: $bin is not universal!"
        exit 1
    fi
done
echo "  All binaries are universal."
echo ""

# --- Step 1b: Inject LSMinimumSystemVersion into plists ---
echo "  Setting LSMinimumSystemVersion=10.13 in all Info.plists..."
for plist in "$VST3/Contents/Info.plist" "$AU/Contents/Info.plist" \
             "$AAX/Contents/Info.plist" "$STANDALONE/Contents/Info.plist"; do
    /usr/libexec/PlistBuddy -c "Delete :LSMinimumSystemVersion" "$plist" 2>/dev/null || true
    /usr/libexec/PlistBuddy -c "Add :LSMinimumSystemVersion string 10.13" "$plist"
done
echo "  Done."
echo ""

# --- Step 2: Code Sign VST3, AU, Standalone ---
echo "[2/6] Code signing VST3, AU, Standalone..."
for plugin in "$VST3" "$AU" "$STANDALONE"; do
    codesign --force --deep --options runtime \
        --sign "$SIGN_ID" "$plugin"
    NAME=$(basename "$plugin")
    echo "  Signed: $NAME"
done
echo ""

# --- Step 3: Notarize VST3, AU, Standalone ---
echo "[3/6] Submitting for Apple notarization..."
NOTARY_PIDS=()
for plugin in "$VST3" "$AU" "$STANDALONE"; do
    NAME=$(basename "$plugin" | sed 's/\..*//')
    FORMAT=$(basename "$plugin" | sed 's/.*\.//')
    ZIP="/tmp/pour_${NAME}_${FORMAT}.zip"
    ditto -c -k --keepParent "$plugin" "$ZIP"
    echo "  Submitting $(basename "$plugin")..."
    xcrun notarytool submit "$ZIP" \
        --keychain-profile "$NOTARY_PROFILE" --wait 2>&1 | tail -2 &
    NOTARY_PIDS+=($!)
done

NOTARY_FAILED=0
for pid in "${NOTARY_PIDS[@]}"; do
    if ! wait "$pid"; then
        NOTARY_FAILED=1
    fi
done
if [ "$NOTARY_FAILED" -eq 1 ]; then
    echo "  ERROR: notarization failed."
    echo "  Run: xcrun notarytool log <submission-id> --keychain-profile $NOTARY_PROFILE"
    exit 1
fi
echo "  All notarization submissions accepted."
echo ""

# --- Step 4: Staple ---
echo "[4/6] Stapling notarization tickets..."
for plugin in "$VST3" "$AU" "$STANDALONE"; do
    xcrun stapler staple "$plugin" > /dev/null 2>&1
    echo "  Stapled: $(basename "$plugin")"
done
echo ""

# --- Step 5: Sign AAX with PACE wraptool ---
echo "[5/6] Signing AAX with PACE wraptool..."
read -sp "  PACE password for '$PACE_ACCOUNT': " PACE_PASSWORD
echo ""

rm -rf "$AAX_SIGNED"
"$WRAPTOOL" sign \
    --verbose \
    --account "$PACE_ACCOUNT" \
    --password "$PACE_PASSWORD" \
    --wcguid "$PACE_WCGUID" \
    --signid "$SIGN_ID" \
    --in "$AAX" \
    --out "$AAX_SIGNED" 2>&1 | grep -E "^(Successfully|Error)" || true
echo "  AAX signed: $(basename "$AAX_SIGNED")"
echo ""

# --- Step 6: Stage distribution folder ---
echo "[6/6] Packaging distribution..."
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"/{VST3,AU,AAX,Standalone}

cp -R "$VST3"        "$DIST_DIR/VST3/"
cp -R "$AU"          "$DIST_DIR/AU/"
cp -R "$AAX_SIGNED"  "$DIST_DIR/AAX/Pour.aaxplugin"
cp -R "$STANDALONE"  "$DIST_DIR/Standalone/"

echo "  Distribution folder: $DIST_DIR"
echo ""

# --- Verification ---
echo "=== Verification ==="
echo ""
echo "Code signatures:"
for plugin in "$DIST_DIR/VST3/Pour.vst3" \
              "$DIST_DIR/AU/Pour.component" \
              "$DIST_DIR/Standalone/Pour.app" \
              "$DIST_DIR/AAX/Pour.aaxplugin"; do
    AUTH=$(codesign -dvv "$plugin" 2>&1 | grep "Authority=Developer" | head -1 | sed 's/Authority=//')
    printf "  %-40s %s\n" "$(basename "$plugin")" "$AUTH"
done
echo ""

echo "Notarization:"
for plugin in "$DIST_DIR/VST3/Pour.vst3" \
              "$DIST_DIR/AU/Pour.component" \
              "$DIST_DIR/Standalone/Pour.app"; do
    RESULT=$(xcrun stapler validate "$plugin" 2>&1 | grep -o "worked" || echo "FAILED")
    printf "  %-40s %s\n" "$(basename "$plugin")" "$RESULT"
done
echo ""

echo "=== Done! Pour v${VERSION} ready to package ==="
echo "  Next: ./build_installer.sh ${VERSION}"
