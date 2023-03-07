# objModifier
A command line tool to apply vector/normal displacement map to an obj file.

## Requirements

* [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
* [libtiff](http://www.libtiff.org)
* [tinyexr](https://github.com/syoyo/tinyexr) (included)

## Build

#### mac/linux with libtiff installed

```sh
mkdir build
cd build
cmake -G "Unix Makefiles" -DEIGEN_ROOT_DIR=/full/path/to/eigen-3.3.9 -DCMAKE_BUILD_TYPE=Release ../
cmake --build . --config Release
```

To use OpenMP on MacOS, install gcc via homebrew and use it to build by adding `-DCMAKE_CXX_COMPILER=g++11`.
eg.
```
cmake -G "Unix Makefiles" -DCMAKE_CXX_COMPILER=g++11 -DEIGEN_ROOT_DIR=/full/path/to/eigen-3.3.9 -DCMAKE_BUILD_TYPE=Release ../
```

#### mac/linux without libtiff installed

```sh
cmake -G "Unix Makefiles" \
    -DEIGEN_ROOT_DIR=/Volumes/CT1000MX/git/objModifier/eigen-3.3.9 \
    -DTIFF_INCLUDE_DIR=/Users/minoue/.local/include \
    -DTIFF_LIB_DIR=/Users/minoue/.local/lib \
    -DCMAKE_BUILD_TYPE=Release \
    ../
cmake --build . --config Release
```

#### Windows10 with litiff installed
```
cmake -G "Visual Studio 17 2022" -DEIGEN_ROOT_DIR=C:\path\path\eigen -DCMAKE_BUILD_TYPE=Release ../
cmake --build . --config Release
```


## Usage

```
>>./objModifier -h
Usage: ./objModifier [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -t,--textures TEXT ...      Displacement textures
  -i,--input TEXT             Input obj file
  -o,--output TEXT            Output name
  -n,--normalDisplacement     Normal displacement
```

## Example
```
>>./objModifier -i /test/VDispDiagnosticMesh_high.obj -o out -t /test/testVDM.*.exr
```

"out.obj" will be generated in the same directory of the source mesh.


## Credits
[CLI11: Command line parser for C++11](https://github.com/CLIUtils/CLI11) / The 3-Clause BSD License / Henry Schreiner

[tinyexr](https://github.com/syoyo/tinyexr) / The 3-Clause BSD License / Shoyo Fujita

