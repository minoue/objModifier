cmake_minimum_required(VERSION 3.9)

project(objModifier CXX)

set(SOURCE_FILES
    src/main.cpp
    src/objIO.cpp
    src/timer.cpp
    src/util.cpp
    src/image.cpp
    src/texture.cpp
    )

set(CMAKE_CXX_STANDARD 17)


if (NOT EIGEN_ROOT_DIR)
    message(FATAL_ERROR "Eigen3 dir must be set")
else()
    message(STATUS "Using EIGEN_DIR ${EIGEN_DIR}")
endif()


if (WIN32)
    if (MSVC_VERSION GREATER_EQUAL "1900")
        include(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
        if (_cpp_latest_flag_supported)
            add_compile_options("/std:c++latest")
        endif()
    endif()
else()
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release)
    endif()

    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()


# OpenMP
find_package(OpenMP)
if (OPENMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    add_definitions(-DUSE_OPENMP)
endif()


# Eigen3
include_directories(${EIGEN_ROOT_DIR})


# libtiff
find_package(TIFF)
if (TIFF_FOUND)
    message("Tiff include dir : ${TIFF_INCLUDE_DIR}")
    message("Tiff libraries : ${TIFF_LIBRARIES}")
    include_directories(${TIFF_INCLUDE_DIR})
else ()
    message(WARNING "Tiff package not found. Trying to use custom libtiff dir")

    if (NOT TIFF_INCLUDE_DIR)
        message(FATAL_ERROR "Tiff include dir must be set")
    else()
        message(STATUS "Tiff include dir : ${TIFF_INCLUDE_DIR}")
    endif()

    if (NOT TIFF_LIB_DIR)
        message(FATAL_ERROR "Tiff lib dir must be set")
    else()
        message(STATUS "Tiff lib dir : ${TIFF_LIB_DIR}")
    endif()

    include_directories(${TIFF_INCLUDE_DIR})
    link_directories(${TIFF_LIB_DIR})
endif()


add_executable(${PROJECT_NAME} ${SOURCE_FILES})


if (MSVC_VERSION GREATER_EQUAL "1900")
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "setargv.obj")
endif()


if (TIFF_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE TIFF::TIFF)
else ()
    target_link_libraries(${PROJECT_NAME} PRIVATE tiff)
endif()

