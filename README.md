# CS153A lab3b source code

This repo contains the source code for a chromatic tuner
on the Nexys A7 FPGA board.
It is able to distingunish tones between 40Hz-7000Hz
with <1 Hz accuracy. The UI is fully responsive.

Contained within the code is a fixed-width FFT implementation
modified from [here](https://gist.github.com/Tomwi/3842231).

# Code Structure

`/io` contains code that interacts with low-level hardware directly,
including LCD.

`/virtio` handles encoder rotation direction, button debouncing,
FFT, and provides several timer wrapper classes.

# Limitations

To recognize tones as accurate as possible,
this program allocates a lot of stack and heap space
for fixed-width FFT.

At least 16 MB of stack and 4MB of heap is recommended.

