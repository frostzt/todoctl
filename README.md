# TodoCtl

Most overengineered Todo app.

Note: this was compiled on a Mac

## Compiling and building

This is something I wrote for fun and was built primarily on a Mac however it should
have no issues with Linux too.

### Building using CMake

Create a new directory called `build` and use cmake to generate makefile

```shell
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
make install
```

Note that `make install` might require `sudo` priviledges
