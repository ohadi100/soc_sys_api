# vwos-sok

## Overview

This repository contains source code for Freshness value Manager (FVM) This repo is maintained by SOK team in ComSec ART.

FVM is a plugin library for the ARA SecOC component R20.11.

FVM provides freshness values to the SecOC to enable replay protection in secure PDUs.

FVM allows applications to consume and provide PDUs via the Challenge - Response protocol (CR):
For triggering CR on the consumer side:
```
FvmErrorCode TriggerCrRequest(SokFreshnessValueId)
```
For offering PDU with CR protocol on the provider side:
```
FvmErrorCode OfferCrRequest(SokFreshnessValueId, ChallengeReceivedIndicationCb)
```

**See sequence diagrams under doc/diagrams for more information**

SOK in different stacks:
* Micro stack - Integration of SecOC and FVM are not done by us.
* Mid stack - Integration of SecOC from ARA and implementation of FVM by us.
* Macro stack - Implementation of SecOC and FVM by us. No release planned for now.
(reference for Macro code can be found here - https://git.hub.vwgroup.com/CARIAD/vwos-comsec-sok-lib/tree/develop/src/sok/secoc).

## How-tos

### Clean build via `conan create`

Create local packages using local channel (that is `vwos/local`) to distinguish these from the ones created by CI which uses e.g. `vwos/testing`.
Override deployment package requirement in the main recipe to enable usage of locally created packages without recipe modification.

build the different targets by using the different conanfiles 
* conanfile.py -> FVM libraries (for FVM server and FVM participant)
* sample_sink.conanfile.py -> Sink demo application (dependent on FVM participant library)
* sample_source.conanfile.py -> Source demo application (dependent on FVM server library)

```shell
conan create <conan file> vwos/local -pr:h linux-host
```

### Incremental builds via `conan install / conan build`

Useful for e.g. exploring the generated source code or quick-fixing build errors where using clean builds for every change is too slow.

```shell
rm -rf build_fvm
conan install <conan file> vwos/local -pr:h linux-host -if build_<fvm|sample_source|sample_sink>
conan build <conan file> -if build_<fvm|sample_source|sample_sink>
```

After `conan install` Conan generates all the necessary CMake files in `build` folder, including toolchain files, CMake preset, import files for Conan requirements, etc.
During the first `conan build` code will be generated from ARXMLs. It will not be re-generated during next incremental re-builds, unless ARXMLs will be modified.

#### Publishing the conan package to the conan cache

useful if it is desired to create RTE from the locally created conan package
```shell
conan export-pkg <conan file> -if build_<fvm|sample_source|sample_sink> --force
```

### Building unit tests
```shell
conan install conanfile.py vwos/local -pr:h linux-host -s vwos-sok-fm:build_type=Debug -o vwos-sok-fm:gtest=True -if build_fvm
conan build conanfile.py -if build_fvm
```
