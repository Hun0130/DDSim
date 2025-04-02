cmake_minimum_required(VERSION 3.16)
project(DDSim)

# FastDDS 경로 설정
set(FASTDDS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/Fast-DDS/Fast-DDS/install")

# C++11 설정
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 소스 코드 설정
file(GLOB DDS_HELLOWORLD_SOURCES_CXX "src/*.cxx")

# 실행 파일 추가
add_executable(DDSHelloWorldPublisher src/HelloWorldPublisher.cpp ${DDS_HELLOWORLD_SOURCES_CXX})
add_executable(DDSHelloWorldSubscriber src/HelloWorldSubscriber.cpp ${DDS_HELLOWORLD_SOURCES_CXX})
add_executable(DDSHelloWorldSimulator src/HelloWorldSimulator.cpp ${DDS_HELLOWORLD_SOURCES_CXX})

# 타겟별 설정 - 모던 CMake 스타일 적용
foreach(TARGET DDSHelloWorldPublisher DDSHelloWorldSubscriber DDSHelloWorldSimulator)
    target_include_directories(${TARGET} PRIVATE 
        "${FASTDDS_ROOT}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
    )
    
    target_link_directories(${TARGET} PRIVATE "${FASTDDS_ROOT}/lib")
    
    target_link_libraries(${TARGET} PRIVATE fastdds fastcdr pthread)
    
    # RPATH 설정
    set_target_properties(${TARGET} PROPERTIES
        INSTALL_RPATH "${FASTDDS_ROOT}/lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endforeach() 