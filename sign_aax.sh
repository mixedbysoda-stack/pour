#!/bin/bash
# Pour AAX Plugin Signing Script — produces a PACE-signed AAX for distribution.

if [ -z "$1" ]; then
    echo "Usage: ./sign_aax.sh YOUR_PACE_PASSWORD"
    exit 1
fi

PASSWORD="$1"
AAX_IN="/Users/soda/Desktop/Pour/build-release/Pour_artefacts/Release/AAX/Pour.aaxplugin"
AAX_OUT="/Users/soda/Desktop/Pour/build-release/Pour_artefacts/Release/AAX/Pour_signed.aaxplugin"

echo "Signing Pour AAX..."

/Applications/PACEAntiPiracy/Eden/Fusion/Versions/5/bin/wraptool sign \
    --account sodanswishers \
    --wcguid 7D6E77D0-3C83-11F1-B00E-005056928F3B \
    --password "$PASSWORD" \
    --signid 45C26CF1655F48EBC8A21802BDA053514719E1F0 \
    --in "$AAX_IN" \
    --out "$AAX_OUT" \
    --verbose

if [ $? -eq 0 ]; then
    echo "Signing successful!"
    echo "Signed plugin: $AAX_OUT"
else
    echo "Signing failed. Check the error above."
fi
