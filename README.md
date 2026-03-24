<div align="centre">
    # Shellac
    
    A chess engine written in C++. It is compatible with any GUI with UCI support.
</div>

# Strength

| Version | CCRL Rating |
|---------|-------------|
| 1       | 1100 *1     |
| 2       | 2170 *2     |

\* These ratings are not from the CCRL itself, but estimated by comparing to known engines.

\*1 Seemed to want to break cutechess so I couldn't run the tournament, but from the small samples Stash v8 seemed to
put up a fight.

\*2 Shellac 2 performed better than Stash v15 but worse than Stash v16. This is an interpolated ELO estimation.

# Compiling

To compile, CMake and a C++17 compatible compiler is needed. You may find the engine to perform better with GCC as the
engine uses several GCC builtin functions to access CPU intrinsics.

To build, run the following.

```shell
mkdir build
cd build
cmake ..
make
```