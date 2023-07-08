# Algorithm prototyping
## File overview
[`algorithm.m`](algorithm/algorithm.m) - An Octave script for design, implementation, and visualization of the signal conditioning, R-peak detection, and heart rate calculation program used by this project.

[`algorithm.c`](algorithm/algorithm.c) - A real-time C adaptation of the program contained in the Octave script, optimized for minimal time and memory complexity.

[`plot_output.m`](algorithm/plot_output.m) - An Octave script to plot the output waveforms produced by the C program, for debugging purposes.

[`scope.csv`](algorithm/scope.csv) - Several seconds of an ECG signal captured by a serial oscilloscope for testing purposes; see [`firmware/`](firmware/) for more details about the prototype used to capture this data.
## Design process
### Hum's the Word: Removing powerline interference
Before starting, we want to record a signal that was as close as possible to what would be read by the ADC on our final PCB. Although the stock filter configuration on the AD8232 evaluation board isn't exactly what's intended for the final hardware design, the algorithm should be resilient to changes in filtering and should therefore be valid for our purposes.

![Original ECG signal](https://raw.githubusercontent.com/stevefarra/ec-stingy/main/docs/visuals/original_ecg_signal.png)

Three things are immediately apparent about the raw signal:
1. Large DC offset
2. Mains hum prevalence
3. Noticeable waveform distortion (P and T waves are indiscernable)

We know the DC component dominates, so let's plot the frequency spectrum with it removed. Since our signal is sampled at the nominal rate of 360 Hz we see frequencies up to 180 Hz (as per the Nyquist theorem).

![Magnitude spectrum](https://raw.githubusercontent.com/stevefarra/ec-stingy/main/docs/visuals/magnitude_spectrum.png)

Most of the signal contents lay in the 0-30 Hz range with a dominant frequency at exactly 60 Hz, confirming our observation about the mains hum earlier. We want this design to work with electrode cables without any EMI shielding and we also want to retain the ability to use the signal for cardiac monitoring, so filtering will have to be done in the digital domain. A Notch filter allows us to strongly attenuate this particular frequency while introducing minimal distortion to the rest of the signal.

Using Octave's `pei_tseng_notch()` function means we only need to use a bit of trial-and-error to find the minimum bandwidth that effectively suppresses the 60 Hz noise, which visually occurs around 5 Hz, and yields a filter with an attentuation reaching nearly -30 dB:

![Notch filter bode plot](https://raw.githubusercontent.com/stevefarra/ec-stingy/main/docs/visuals/notch_filter_bode_plot.png)

Here is the resultant signal, which now has something resembling a P-wave. The DC offset is still present but is addressed during R-peak detection, detailed below.

![Filtered ECG signal](https://raw.githubusercontent.com/stevefarra/ec-stingy/main/docs/visuals/ecg_signal_filtered.png)

### R-peak detection
Every prominent R-peak detection algorithm has three distinct stages: signal conditioning, thresholding, and R-peak searching. This project uses an algorithm published in 2019 that improves upon previous approaches by introducing a triangle template matching filter to reduce the resource complexity present in other algorithms used in embedded devices. To explain the signal conditioning we first introduce some notation for a moving average filter centered around the current element (assume the signal is zero-padded):
$$\text{MA}(x[n],N)=\frac{1}{2N+1}\sum_{-N}^{N}x[n+N]$$
And the triangle template matching filter:
$$\text{TR}(x[n],N)=(x[n]-x[n-N])(x[n]-x[n+N])$$
With our notched ECG signal above denoted as $\text{ECG}[n]$, we begin cascading filters. The first is a high-pass filter:
$$\bar{x} = \text{MA}(x[n],N)$$
$$\hat{h} = x - \bar{x}$$
$$h = |\hat{h}|$$
With DC noise and negative values forgone, we have a signal we can eventually perform peak detection on:

![High pass filter](https://raw.githubusercontent.com/stevefarra/ec-stingy/main/docs/visuals/high_pass_filter.png)

