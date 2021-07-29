# objModifier
A command line tool to apply vector/normal displacement map to an obj file.

## Requirements

* [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
* [tinyexr](https://github.com/syoyo/tinyexr) (included)
* [libtiff](http://www.libtiff.org) (optional)

## Build

mac/linux
```
>>mkdir build
>>cd build
>>cmake -G "Unix Makefiles" -DCMAKE_CXX_COMPILER=g++-11 -DEIGEN_ROOT_DIR=/full/path/to/eigen-3.3.9 -DCMAKE_BUILD_TYPE=Release ../
```

with libtiff
```
>>cmake -G "Unix Makefiles" -DUSE_TIFF=1 -DCMAKE_CXX_COMPILER=g++-11 -DEIGEN_ROOT_DIR=/full/path/to/eigen-3.3.9 -DCMAKE_BUILD_TYPE=Release ../
```

## Usage

```
>>./objModifier -h
Usage: ./objModifier [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -t,--textures TEXT ...      Displacement textures
  -i,--input TEXT             Input obj file
  -n,--normalDisplacement     Normal displacement
```

## Example
```
>>./objModifier -i ../test/VDispDiagnosticMesh_high.obj -t ../test/testVDM.*.exr
```

"out_displaced.obj" will be generated in the same directory of the source mesh.
