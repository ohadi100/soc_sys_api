#!/bin/bash

# script for building linux rte for testing sok functionalities and flows
# can be executed from anywhere

SOK_REPO_BASE_DIR=$( dirname $(cd "$(dirname "$0")" && pwd ) )
SOK_REPO_RTE_DIR=$SOK_REPO_BASE_DIR/rte
RTE_IMAGE_FOLDER=$SOK_REPO_RTE_DIR/build/images

# clenup previous image
if [ -d "$RTE_IMAGE_FOLDER" ]; then rm -Rf $RTE_IMAGE_FOLDER; fi

if ! conan lock create $SOK_REPO_RTE_DIR/sok_rte.conanfile.py -pr linux-host --lockfile-out $SOK_REPO_RTE_DIR/sok_rte.linux.custum.lock; then
    echo failed creating lock file for RTE
    exit 0
fi

if ! conan install --update $SOK_REPO_RTE_DIR/sok_rte.conanfile.py --lockfile $SOK_REPO_RTE_DIR/sok_rte.linux.custum.lock -if $SOK_REPO_RTE_DIR/build; then
    echo failed installing RTE
    exit 0
fi

if ! conan build $SOK_REPO_RTE_DIR/sok_rte.conanfile.py -bf $SOK_REPO_RTE_DIR/build; then
    echo failed building RTE
    exit 0
fi

echo Built RTE successfully

exit 1