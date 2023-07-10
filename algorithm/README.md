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

### A Tribe Called QRS: Implementing heart rate detection
Every prominent R-peak detection algorithm has three distinct stages: signal conditioning, thresholding, and R-peak searching. This project uses an algorithm published in 2019 that improves upon previous approaches by introducing a triangle template matching filter to reduce the resource complexity present in other algorithms used in embedded devices. To explain the signal conditioning we first introduce some notation for a moving average filter centered around the current element (assume the signal is zero-padded):
$$\text{MA}(x[i],R):=\frac{1}{2R+1}\sum_{-R}^{R}x[i+R]$$
And the triangle template matching filter:
$$\text{TR}(x[i],R):=(x[i]-x[i-R])(x[i]-x[i+R])$$
With our notched ECG signal above denoted as $\text{ECG}[i]$, we begin cascading filters. The first is a high-pass filter:
$$\overline{\text{ECG}}[i] = \text{MA}(\text{ECG}[i],N)$$
$$\hat{h}[i] = \text{ECG}[i] - \overline{\text{ECG}}[i]$$
$$h[i] = |\hat{h}[i]|$$
With DC noise and negative values forgone, we have a signal we can eventually perform peak detection on:

![High pass filter](https://raw.githubusercontent.com/stevefarra/ec-stingy/main/docs/visuals/high_pass_filter.png)

Next is the triangle filter, $t[i]=\text{TR}(h[i],s)$, which accentuates the QRS complex:

![Triangle](https://raw.githubusercontent.com/stevefarra/ec-stingy/main/docs/visuals/triangle_filter.png)

Finally, the thresholding filters:
$$l_1[i] = \text{MA}(t[i],L)$$
$$l_2[i] = \text{MA}(t[i],M)$$
$$\text{threshold}[i] = \beta l_2[i] + \theta$$

The first low pass filter, $l_1$, is used to "smoothen out" the output of the triangle filter while $l_2$ is used to set the threshold value. The result is

![Threshold](https://github.com/stevefarra/ec-stingy/blob/main/docs/visuals/threshold.png?raw=true)

Regions where $l_1$ (blue plot) is greater than the threshold value (red plot) is considered an AOI (area of interest) and maxima within each area is considered an R-peak. An error correction step must also be applied, however, because the peaks produced by $l_1$ are not perfectly convex, AOIs that should be one contiguous region are sometimes detected as two separate regions. For an example of this, notice how the fourth QRS region in $l_1$ crosses the threshold, dips back down, and crosses over once again, resulting in false positives. To ameliorate this, the algorithm leverages the fact that the the theoretical maximum heart rate is 206 bpm, so when a detected R-R interval which exceeds this value the lower amplitude R-peak is discarded. With these rules applied, the detected R-peaks look like:

![R-peaks](https://raw.githubusercontent.com/stevefarra/ec-stingy/main/docs/visuals/r_peaks.png)

The heart rate readings, in units of bpm, are trivially calculated as $60/\text{RR}$, where $\text{RR}$ is the distance between successive R-peaks.

(Mention parameters here)

### Sync or Swim: Adaptations for a real-time environment

The next step 
