cmake_minimum_required(VERSION 3.10)
project(HelloWorldSimulator)

# C++11 표준 사용
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fast-DDS 설치 경로
set(FASTDDS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Fast-DDS/Fast-DDS/install)

# include 디렉토리 설정
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    # 로컬 헤더 파일을 우선 사용
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/Fast-DDS/Fast-DDS/include
    ${FASTDDS_DIR}/include
)

# 라이브러리 디렉토리 설정
link_directories(
    ${FASTDDS_DIR}/lib
)

# HelloWorldSimulator 실행 파일 생성
add_executable(HelloWorldSimulator 
    HelloWorldSimulator.cpp
    HelloWorldPubSubTypes.cxx
    HelloWorldTypeObjectSupport.cxx
)

# 필요한 라이브러리 링크
target_link_libraries(HelloWorldSimulator
    fastdds
    fastcdr
    pthread
) 