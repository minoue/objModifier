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


if (WIN32)
    if (MSVC_VERSION GREATER_EQUAL "1900")
        include(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
        if (_cpp_latest_flag_supported)
            add_compile_options("/std:c++latest")
        endif()
    endif()
else()
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
include_directories(eigen)

# libtiff
find_package(TIFF)
if (TIFF_FOUND)
    message("Tiff include dir : ${TIFF_INCLUDE_DIR}")
    message("Tiff libraries : ${TIFF_LIBRARIES}")
    include_directories(${TIFF_INCLUDE_DIR})
else ()
    message(NOTICE "Tiff package not found. Using custom libtiff dir")

    if (NOT TIFF_INSTALL_DIR)
        message(FATAL_ERROR "Tiff install dir must be set")
    endif()

    set(TIFF_INCLUDE_DIR "${TIFF_INSTALL_DIR}/include")
    set(TIFF_LIB_DIR "${TIFF_INSTALL_DIR}/lib")
    message(STATUS "Tiff include dir : ${TIFF_INCLUDE_DIR}")
    message(STATUS "Tiff lib dir : ${TIFF_LIB_DIR}")

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
