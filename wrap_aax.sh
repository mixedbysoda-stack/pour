#!/bin/bash
# Pour AAX sign + install to Pro Tools.
# Pour's PACE profile (WCGUID 7D6E77D0...) is "Signing Only", so we use
# `wraptool sign` instead of `wraptool wrap`.

if [ -z "$1" ]; then
    echo "Usage: ./wrap_aax.sh YOUR_PACE_PASSWORD"
    exit 1
fi

PASSWORD="$1"
AAX_IN="/Users/soda/Desktop/Pour/build-release/Pour_artefacts/Release/AAX/Pour.aaxplugin"
AAX_OUT="/Users/soda/Desktop/Pour/build-release/Pour_artefacts/Release/AAX/Pour_wrapped.aaxplugin"
PT_PLUGINS="/Library/Application Support/Avid/Audio/Plug-Ins"

echo "Signing Pour AAX (PACE sign-only profile)..."

/Applications/PACEAntiPiracy/Eden/Fusion/Versions/5/bin/wraptool sign \
    --verbose \
    --account sodanswishers \
    --password "$PASSWORD" \
    --wcguid 7D6E77D0-3C83-11F1-B00E-005056928F3B \
    --signid 45C26CF1655F48EBC8A21802BDA053514719E1F0 \
    --in "$AAX_IN" \
    --out "$AAX_OUT"

if [ $? -eq 0 ]; then
    echo "Signing successful!"
    echo "Installing to Pro Tools..."
    rm -rf "$PT_PLUGINS/Pour.aaxplugin"
    cp -R "$AAX_OUT" "$PT_PLUGINS/Pour.aaxplugin"
    echo "Done! Restart Pro Tools to load Pour."
else
    echo "Signing failed."
fi
