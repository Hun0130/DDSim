#!/bin/bash

# 스크립트가 있는 디렉토리로 이동
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# 이미 빌드된 결과가 있는지 확인
REBUILD=0
if [ "$1" == "clean" ]; then
    echo "clean 옵션이 지정되어 새로 빌드합니다."
    REBUILD=1
elif [ ! -d "install" ] || [ ! -f "install/lib/libfastdds.so" ]; then
    echo "이전 빌드 결과가 없어 새로 빌드합니다."
    REBUILD=1
else
    echo "이전 빌드 결과를 발견했습니다. 증분 빌드를 수행합니다."
    # 증분 빌드만 진행
    cd "$SCRIPT_DIR/build"
    cmake --build . --target install -j$(nproc)
    if [ $? -eq 0 ]; then
        echo "증분 빌드 성공! 설치 위치: $SCRIPT_DIR/install"
        exit 0
    else
        echo "증분 빌드 실패, 전체 재빌드를 시도합니다."
        REBUILD=1
    fi
fi

if [ $REBUILD -eq 1 ]; then
    # 빌드 결과 디렉토리 제거 (정리)
    if [ -d "build" ]; then
        echo "기존 빌드 디렉토리 정리 중..."
        rm -rf build
    fi
    if [ -d "install" ]; then
        echo "기존 설치 디렉토리 정리 중..."
        rm -rf install
    fi

    # 코어 수 확인 (병렬 빌드 속도 최적화)
    NUM_CORES=$(nproc)
    echo "빌드에 $NUM_CORES 개의 코어를 사용합니다."

    # 설치 디렉토리 생성
    mkdir -p install

    # 필요한 의존성만 검사
    # Fast-CDR가 이미 빌드되어 있는지 확인
    if [ ! -f "$SCRIPT_DIR/install/lib/libfastcdr.so" ]; then
        echo "1. Fast-CDR 빌드 중..."
        cd "$SCRIPT_DIR/../Fast-CDR"
        mkdir -p build && cd build
        cmake .. -DCMAKE_INSTALL_PREFIX="$SCRIPT_DIR/install" \
                -DCMAKE_BUILD_TYPE=Release
        cmake --build . --target install -j$NUM_CORES
        if [ $? -ne 0 ]; then
            echo "Fast-CDR 빌드 실패!"
            exit 1
        fi
        echo "Fast-CDR 빌드 완료!"
    else
        echo "Fast-CDR가 이미 설치되어 있습니다. 빌드를 건너뜁니다."
    fi

    # foonathan_memory_vendor 이미 빌드되어 있는지 확인
    if [ ! -f "$SCRIPT_DIR/install/lib/libfoonathan_memory-0.7.3.so" ]; then
        echo "2. foonathan_memory_vendor 빌드 중..."
        cd "$SCRIPT_DIR/../foonathan_memory_vendor"
        mkdir -p build && cd build
        cmake .. -DCMAKE_INSTALL_PREFIX="$SCRIPT_DIR/install" \
                -DCMAKE_BUILD_TYPE=Release \
                -DBUILD_SHARED_LIBS=ON
        cmake --build . --target install -j$NUM_CORES
        if [ $? -ne 0 ]; then
            echo "foonathan_memory_vendor 빌드 실패!"
            exit 1
        fi
        echo "foonathan_memory_vendor 빌드 완료!"
    else
        echo "foonathan_memory_vendor가 이미 설치되어 있습니다. 빌드를 건너뜁니다."
    fi
fi

# Fast-DDS 빌드
echo "3. Fast-DDS 빌드 중..."
cd "$SCRIPT_DIR"

# 빌드 디렉토리 생성 및 이동
mkdir -p build && cd build

# Fast-CDR 경로 설정
FASTCDR_ROOT="$SCRIPT_DIR/install"

# foonathan_memory 경로 설정
FOONATHAN_MEMORY_ROOT="$SCRIPT_DIR/install"

# CMake 실행 - 최적화된 옵션
cmake .. -DCMAKE_INSTALL_PREFIX="$SCRIPT_DIR/install" \
         -DCMAKE_BUILD_TYPE=Debug \
         -DFASTCDR_ROOT="$FASTCDR_ROOT" \
         -DFOONATHAN_MEMORY_ROOT="$FOONATHAN_MEMORY_ROOT"

# 병렬 빌드 및 설치 (모든 사용 가능한 CPU 코어 활용)
cmake --build . --target install -j$NUM_CORES -- -k

if [ $? -eq 0 ]; then
    echo "빌드 성공! 설치 위치: $SCRIPT_DIR/install"
else
    echo "빌드 실패!"
    exit 1
fi 