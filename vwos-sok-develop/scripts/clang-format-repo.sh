#!/bin/bash

# script for applying clang-format with style as in sok_lib/.clang-format on repo's headers and sources
# can be executed from anywhere
# prerequisite - clang-format (`sudo apt install clang-format`) 

SOK_REPO_BASE_DIR=$( dirname $(cd "$(dirname "$0")" && pwd ) )

find $SOK_REPO_BASE_DIR"/include" -iname *.hpp -o -iname *.cpp | xargs clang-format -i --style=file:.clang-format
find $SOK_REPO_BASE_DIR"/src" -iname *.hpp -o -iname *.cpp | xargs clang-format -i --style=file:.clang-format