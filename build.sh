#!/bin/bash

echo 'BOOST_PATH: ' ${BOOST_PATH}


ROOT_DIR=$(dirname "$(readlink -f "$0")")
rm -rf make && CC=clang CXX=clang++ cmake ${ROOT_DIR} -B ${ROOT_DIR}/make  -DBOOST_ROOT_DIR=${BOOST_PATH}/include -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" && cmake --build ${ROOT_DIR}/make --config Release