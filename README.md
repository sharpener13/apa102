# APA102 LEDs' library

Overview
---
This is a quick attempt to explore a little more efficient path to handle longer chains (approx. 300 LEDs) of `APA102` devices connected to the `Raspberry Pi zero`.

First experiments in `python` showed that it is nice for quick checking, but for real usage it was terribly slooow. 

So, took the `python` files and "converted" them (manually) to `C`. After that some more enhancements were performed and here it is (planning to try `C++`, shouldn't be such performance penalty and would be much more convenient to use some inheritance and encapsulation).

Current state
---
- running on 12MHz (corresponds to 20MHz request in spidev)
- driving 360 LED's
- using 4 Larsons ;-)
- one frame takes approx 1.5ms

Which is not as bad, this might imply approx 500fps.

Stuff available
---
- `apa102spi`: SPI open/close/write layer
- `apa102`: rendering and pixel manipulation, the idea is: let one frame being rendered and prepare another one simultaneously.
- `apa102_test`: simple tests of all the stuff.

Notes
---
- if not installing the .so libraries to standard places, do not forget to perform `export LD_LIBRARY_PATH=.`.
- tried `clang` in `Makefile`, feel free to use `gcc` instead.
- using `pthread` and `math` ;-)
