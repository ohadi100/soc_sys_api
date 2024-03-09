#!/bin/sh

# copy binaries
cp -P lib/*.so "$VWOS_LIBS"

# configure 
if [ -z "${VWOS_NETWORK_INTERFACE_RTE:-}" ] ; then
    echo "VWOS_NETWORK_INTERFACE_RTE env variable not configured. using default network interface:\"lo\""
    export VWOS_NETWORK_INTERFACE_RTE="lo"
fi

# copy config file
mkdir -p "$VWOS_DATA_STATIC"/fvm
envsubst < config/config_participant_sink.json > "$VWOS_DATA_STATIC"/fvm/config_participant_sink.json
envsubst < config/config_server_source.json > "$VWOS_DATA_STATIC"/fvm/config_server_source.json