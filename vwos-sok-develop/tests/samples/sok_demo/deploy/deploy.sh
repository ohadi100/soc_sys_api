#!/bin/sh

if [ -d sok_demo ]; then
    pushd sok_demo
    for i in *; do
        mkdir -p "$VWOS_APPS"/"$i"

        cp -R "$i" "$VWOS_APPS"/
        cp -R "../deploy/secoc_config.json" "$VWOS_APPS"/"$i"/
    done
    popd
fi