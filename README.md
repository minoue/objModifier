# objModifier
A command line tool to apply vector/normal displacement map to an obj file.

## Requirements

* [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
* [libtiff](http://www.libtiff.org)
* [tinyexr](https://github.com/syoyo/tinyexr) (included)
* C++17

## Build

```sh
git clone https:/github.com/minoue/objModifier 
cd objModifier
git submodule update --init --recursive
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
cmake --build . --config Release
```

To use OpenMP on MacOS, install gcc via homebrew and use it to build by adding `-DCMAKE_CXX_COMPILER=g++11`.
eg.
```
cmake -DCMAKE_CXX_COMPILER=g++11 ../
```

#### If libtiff is not installed but built in your local dir

```
cmake -DTIFF_INSTALL_DIR=/libtiff/built/dir -DCMAKE_BUILD_TYPE=Release ../
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
This software uses the following libraries.

[CLI11: Command line parser for C++11](https://github.com/CLIUtils/CLI11) / The 3-Clause BSD License / Henry Schreiner

[tinyexr](https://github.com/syoyo/tinyexr) / The 3-Clause BSD License / Shoyo Fujita

## License
[MIT License](./LICENSE.md)
