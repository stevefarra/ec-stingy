# Algorithm prototyping
## File overview
[`algorithm.m`](algorithm/algorithm.m) - An Octave script for design, implementation, and visualization of the signal conditioning, R-peak detection, and heart rate calculation program used by this project.

[`algorithm.c`](algorithm/algorithm.c) - A real-time C adaptation of the program contained in the Octave script, optimized for minimal time and memory complexity.

[`plot_output.m`](algorithm/plot_output.m) - An Octave script to plot the output waveforms produced by the C program, for debugging purposes.

[`scope.csv`](algorithm/scope.csv) - Several seconds of an ECG signal captured by a serial oscilloscope for testing purposes; see [`firmware/`](firmware/) for more details about the prototype used to capture this data.
## Design process
Before starting, I wanted to record a signal that was as close as possible to what would be read by the ADC on our final PCB. Although the stock filter configuration on the AD8232 evaluation board isn't exactly what's intended for the final hardware design, the algorithm should be resilient to changes in filtering.
