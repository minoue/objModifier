cmake_minimum_required(VERSION 3.9)

project(objModifier CXX)

set(SOURCE_FILES
    src/main.cpp
    src/objIO.cpp
    src/timer.cpp
    src/util.cpp
    src/image.cpp
    )

set(CMAKE_CXX_STANDARD 11)

if (NOT EIGEN_ROOT_DIR)
    message(FATAL_ERROR "Eigen3 dir must be set")
else()
    message(STATUS "Using EIGEN_DIR ${EIGEN_DIR}")
endif()

if (WIN32)
    if (MSVC_VERSION GREATER_EQUAL "1900")
        # include(CheckCXXCompilerFlag)
        # CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
        # if (_cpp_latest_flag_supported)
        #     add_compile_options("/std:c++latest")
        # endif()
        # include_directories("E:/app/tiff/include")
    endif()
else()
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release)
    endif()

    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

find_package(OpenMP)
if (OPENMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    add_definitions(-DUSE_OPENMP)
endif()

include_directories(/usr/local/include)
include_directories(${EIGEN_ROOT_DIR})

link_directories(/usr/local/lib)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

if (MSVC_VERSION GREATER_EQUAL "1900")
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "setargv.obj")
endif()

if (USE_TIFF)
    message(STATUS "Using tiff library")
    target_link_libraries(${PROJECT_NAME} PRIVATE tiff)
    add_definitions(-DUSE_TIFF=1)
else()
    message(STATUS "Not using tiff library, only exr textures can be used")
endif()
