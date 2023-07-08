# Algorithm prototyping
## File overview
[`algorithm.m`](algorithm/algorithm.m) - An Octave script for design, implementation, and visualization of the signal conditioning, R-peak detection, and heart rate calculation program used by this project.

[`algorithm.c`](algorithm/algorithm.c) - A real-time C adaptation of the program contained in the Octave script, optimized for minimal time and memory complexity.

[`plot_output.m`](algorithm/plot_output.m) - An Octave script to plot the output waveforms produced by the C program, for debugging purposes.

[`scope.csv`](algorithm/scope.csv) - Several seconds of an ECG signal captured by a serial oscilloscope for testing purposes; see [`firmware/`](firmware/) for more details about the prototype used to capture this data.
## Design process
Before starting, we want to record a signal that was as close as possible to what would be read by the ADC on our final PCB. Although the stock filter configuration on the AD8232 evaluation board isn't exactly what's intended for the final hardware design, the algorithm should be resilient to changes in filtering and should therefore be valid for our purposes.

![Original ECG Signal](https://raw.githubusercontent.com/stevefarra/ec-stingy/main/docs/visuals/original_ecg_signal_2.png)

Three things are immediately apparent about the raw signal:
1. Large DC offset
2. Mains hum prevalence
3. Noticeable waveform distortion (P and T waves are indiscernable)

We know the DC component dominates, so let's plot the frequency spectrum with it removed. Since our signal is sampled at the nominal rate of 360 Hz we see frequencies up to 180 Hz (as per the Nyquist theorem).

![Magnitude Spectrum](https://raw.githubusercontent.com/stevefarra/ec-stingy/main/docs/visuals/magnitude_spectrum.png)

Most of the signal contents lay in the 0-30 Hz range with a dominant frequency at exactly 60 Hz, confirming our observation about the mains hum earlier. We want this design to work with electrode cables without any EMI shielding and we also want to retain the ability to use the signal for cardiac monitoring, so filtering will have to be done in the digital domain. A Notch filter allows us to strongly attenuate this particular frequency while introducing minimal distortion to the rest of the signal.

Using Octave's `pei_tseng_notch()` function means we only need to use a bit of trial-and-error to find the minimum bandwidth that effectively suppresses the 60 Hz noise, which visually occurs around 5 Hz, and yields a filter with an attentuation reaching nearly -30 dB:

