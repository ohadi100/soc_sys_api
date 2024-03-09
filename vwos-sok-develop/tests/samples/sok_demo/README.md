SOK Demo Apps
=============

## Description
This demo application demonstrate the basic SOK functionality:

1. unauthenticated & authenticated freshness value distribution between FVM Server & Participants 
2. Authentic broadcast
3. Challenge - Response

# How to run the sample SOK sink / source demos:
add to your integration RTE conanfile:
`vwos-sok-sink-sample/[~*version*]@vwos/integration`
`vwos-sok-source-sample/[~*version*]@vwos/integration`
build your integration RTE as described here: https://git.hub.vwgroup.com/swp-mid/vwos-mid-wiki/wiki/How-to-use-MID-stack-release

## Expected behavior - FV distribution:
From SOK source (SRC) that is acting as FVM Server: every second should output: `Published un-authenticated FV: ..`
From SOK sink (SINK) that is acting as FVM Participant: 
* once FV sync is needed: `FV updated from authentic distribution successfully. FV: ..`
* every second should output: `Received unauthenticated FV signal, FV: ..`

## Expected behavior - data exchange:
### Auth broadcast
From SOK source:
* `Sending auth pdu: 0, with data: SOK_SAMPLE_AUTH_PDU`
* `Created authenticator for authentic PDU: 0, successfully`

From SOK sink: 
* `Verifying incoming SOK msg successfully, PDU ID: 0, received data: SOK_SAMPLE_AUTH_PDU`

### Challenge - Response
From SOK source:
* `Received challenge signal: SOK_CR_sample_challenge_signal, connected with FV ID: 1`
* `App response to challenge CB was triggered with FV ID: 1, Responding with data: SOK_SAMPLE_CR_RESPONSE`
* `Created authenticator for authentic PDU: 1, successfully`

From SOK sink: 
* `Triggering challenge with pdu ID: 1`
* `Received confirmation for verification of a challenge response`
* `Verifying incoming SOK msg successfully, PDU ID: 1, received data: SOK_SAMPLE_CR_RESPONSE`
