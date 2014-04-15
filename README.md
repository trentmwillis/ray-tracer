# Advanced Ray-Tracer
An advanced ray tracing program for CAP 4730 at UF.

## Compilation
Included is a Makefile that is tested for use on Linux Ubuntu machines.
To compile, simply run:
    make main

To execute, simply run:
    ./main

To both compile and execute, simply run:
    make main && ./main

## User Instructions
There are a hand full of operations that can be called inside the program. To view a list of these while the program is executing, press the 'h' key; this will print a brief help menu to the terminal window.

Other options are:
* 'a' - toggles antialiasing of the image on and off
* 'd' - toggles depth of field rendering on and off (note: this will automatically run antialiasing as well)
* 'r' - toggles rendering of reflections on and off
* 't' - toggles rendering of transparency/refraction rays on and off
* 'l' - increases the number of lights in the scene, with a maximum of three
* 'k' - decreases the number of lights in the scene, with a minimum of one
