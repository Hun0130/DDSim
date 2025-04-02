#!/bin/bash

# 빌드 디렉토리 생성 및 이동
mkdir -p build
cd build

# CMake 빌드
cmake ..
make -j$(nproc)

# 시뮬레이터 실행
./DDSHelloWorldSimulator

echo "완료되었습니다." 