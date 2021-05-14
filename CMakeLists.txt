cmake_minimum_required(VERSION 3.9)

project(objModifier CXX)

set(SOURCE_FILES
    src/main.cpp
    src/objIO.cpp
    src/timer.cpp
    )

# target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
# target_compile_options(${PROJECT_NAME} PRIVATE "${OpenMP_CXX_FLAGS}")

# if (OPENMP_CXX_FOUND)
#     target_link_libraries(${PROJECT_NAME} PRIVATE "${OpenMP_CXX_FLAGS}" tiff)
#     target_compile_options(${PROJECT_NAME} PRIVATE "${OpenMP_CXX_FLAGS}")
# endif()

set(CMAKE_CXX_STANDARD 17)

if (WIN32)
    if (MSVC_VERSION GREATER_EQUAL "1900")
        include(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
        if (_cpp_latest_flag_supported)
            add_compile_options("/std:c++latest")
        endif()
        include_directories("E:/app/tiff/include")
    endif()
else()
    # https://www.guyrutenberg.com/2014/01/05/enabling-c11-c0x-in-cmake/
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
    CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

    if (COMPILER_SUPPORTS_CXX17)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    endif()
endif()

find_package(OpenMP)
if (OPENMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE tiff)
if (MSVC_VERSION GREATER_EQUAL "1900")
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "setargv.obj")
endif()


