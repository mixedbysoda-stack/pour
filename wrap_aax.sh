#!/bin/bash
# Pour AAX Plugin Wrapping Script — installs to Pro Tools after wrap.

if [ -z "$1" ]; then
    echo "Usage: ./wrap_aax.sh YOUR_PACE_PASSWORD"
    exit 1
fi

PASSWORD="$1"
AAX_IN="/Users/soda/Desktop/Pour/build-release/Pour_artefacts/Release/AAX/Pour.aaxplugin"
AAX_OUT="/Users/soda/Desktop/Pour/build-release/Pour_artefacts/Release/AAX/Pour_wrapped.aaxplugin"
PT_PLUGINS="/Library/Application Support/Avid/Audio/Plug-Ins"

echo "Wrapping Pour AAX..."

/Applications/PACEAntiPiracy/Eden/Fusion/Versions/5/bin/wraptool wrap \
    --verbose \
    --account sodanswishers \
    --password "$PASSWORD" \
    --wcguid 5F45D2A0-3560-11F1-B6A4-00505692AD3E \
    --in "$AAX_IN" \
    --out "$AAX_OUT"

if [ $? -eq 0 ]; then
    echo "Wrapping successful!"
    echo "Installing to Pro Tools..."
    sudo rm -rf "$PT_PLUGINS/Pour.aaxplugin"
    sudo cp -R "$AAX_OUT" "$PT_PLUGINS/Pour.aaxplugin"
    echo "Done! Restart Pro Tools to load Pour."
else
    echo "Wrapping failed."
fi
