# objModifier

## Requirements

* Eigen
* C++17
* libtiff
* tinyexr(included)


## Build

mac/linux
```
>>mkdir build
>>cd build
>>cmake -G "Unix Makefiles" -DCMAKE_CXX_COMPILER=g++-11 -DEIGEN_ROOT_DIR=/full/path/to/eigen-3.3.9 -DCMAKE_BUILD_TYPE=Release ../
>>cmake --build . --config Release
```

