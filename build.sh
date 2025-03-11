#!/bin/sh
#Assumes cmake and compdb(via pip) are installed
BUILD_DIR=build

mkdir -p "${BUILD_DIR}"

echo "-------STARTING BUILD-------"
if  ! command -v cmake >/dev/null 2>&1 
then
      echo "\nERROR::BUILD_FAILED::install cmake to system\n"
      exit 1
fi

cmake -DCMAKE_BUILD_TYPE=Debug \
      -S. \
      -Wall \
      -Werror \
      -Wno-dev \
      -B "${BUILD_DIR}" 

cd "./${BUILD_DIR}"
echo "----making------"
make 
#pip install compdb
cd ..

if  ! command -v compdb >/dev/null 2>&1 
then
      echo "\nERROR::compdb not installed in system\n"
      exit 1
fi

compdb -p build/ list > compile_commands.json
cp compile_commands.json ..
echo "-------FINISHED BUILD-------"