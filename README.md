# objModifier

## Requirements

* [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
* [libtiff](http://www.libtiff.org)
* [tinyexr(included)](https://github.com/syoyo/tinyexr)

## Build

mac/linux
```
>>mkdir build
>>cd build
>>cmake -G "Unix Makefiles" -DCMAKE_CXX_COMPILER=g++-11 -DEIGEN_ROOT_DIR=/full/path/to/eigen-3.3.9 -DCMAKE_BUILD_TYPE=Release ../
>>cmake --build . --config Release
```

## Usage

```
>>./objModifier -h
Usage: objModifier [options] textures 

Positional arguments:
textures     

Optional arguments:
-h --help       shows help message and exits
-v --version    prints version information and exits
-o --object     specify the input obj file [required]
-v --vector     Vector displacement [default: true]
```

## Example
```
>>./objModifier -o ../test/VDispDiagnosticMesh_high.obj ../test/testVDM.*.exr
```
