#!/bin/bash

# 스크립트가 있는 디렉토리로 이동
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Fast-CDR 경로 설정
FASTCDR_ROOT="$( cd "$SCRIPT_DIR/install" && pwd )"

# foonathan_memory 경로 설정
FOONATHAN_MEMORY_ROOT="$( cd "$SCRIPT_DIR/install" && pwd )"

# 빌드 디렉토리 생성 및 이동
mkdir -p build && cd build

# CMake 실행
cmake .. -DCMAKE_INSTALL_PREFIX="$SCRIPT_DIR/install" \
         -DCMAKE_BUILD_TYPE=Release \
         -DFASTCDR_ROOT="$FASTCDR_ROOT" \
         -DFOONATHAN_MEMORY_ROOT="$FOONATHAN_MEMORY_ROOT"

# 병렬 빌드 및 설치
cmake --build . --target install -j$(nproc) 