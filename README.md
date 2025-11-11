# cgbemu
A simple gameboy emulator in C.

In development. Using SDL2. Does not include boot ROM. Should be able to boot simple games like Tetris and Dr. Mario at the moment but output needs to be fixed, MBC not implemented yet. DN.

To compile, in /src, do
```
mkdir build
cd build
cmake ..
make --build .
```

Can use the makefile if using gcc and SDL2 is installed in PATH.
```
make
```

Or if object files are desired:
```
make objects
```

The emulator is started by commandline
```
Usage: (NAME OF PROGRAM) <game.gb> [boot.gb]\n"
```
[] means optional.
Before or after providing the game (and optionally boot rom), use the flag `--debug` or `-d` for debugging.
also `-dCPU` for debugging the CPU, `-dPPU` for the PPU, `-dMEM` for memory and `-dBOOT` for boot.
