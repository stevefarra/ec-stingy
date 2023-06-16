% Author: Steve Farra
%
% Description: This Octave script is used to prototype the software driving the
%              ec-stingy, a low-cost, fully open-sourced electrocardiogram.
%              the script has two components:
%
%              - Notch filter design and implementation, used to filter out
%              power line interference so that the signal may be used as a
%              cardiac monitor.'
%
%              - An end-to-end R-peak detection algorithm implementation for
%              the heart rate monitor. This algorithm closely follows a 2019
%              paper, "Low Resource Complexity R-peak Detection Based on
%              Triangle Template Matching and Moving Average Filter" by
%              Tam Nguyen, Xiaoli Qin, Anh Dinh, and Francis Bui. More details
%              can be found in the README.
%
%              Heart rate readings are printed to the console, in addition to
%              several plots that are displayed showing the filter(s) used by
%              the cardiac and heart rate monitors.
%
% License: This script is licensed under the MIT license.
%          See the accompanying LICENSE file for the full text of the license.

% Used for the FFT and notch filter sections of the script.
pkg load signal;

% The moving average window lengths in the R-peak detection paper used
% specified as one-sided. This macro is used to convert between the two.
window = @(R) 2 * R + 1;

% Sampling frequency. The paper uses the typical rate of 360 Hz, commonly used
% in databases such as the MIT-BIH Arrhythmia Database. The parameters which
% follow are dependent on the sampling frequency, so if you change it be sure
% to adjust the other parameters according to the paper.
Fs = 360;

% One-sided window length for the high-pass filter, used to remove DC noise
% and other artefacts such as baseline wander.
N = 25;

% Parameter for the triangle template matching filter.
s = 7;

% One-sided window length for the first low-pass filter. The output of this
% filter is used to smoothen out the R-peaks.
L = 5;

% One-sided window length for the second low-pass filter. The output of this
% filter is used to set the threshold value.
M = 150;

# Coefficient used to scale up the threshold value.
beta = 2.5;

% Since there may be several detected R-peaks within an area of interest, the
% paper uses the assumption that the maximum heart rate is 206 bpm, or a
% minimum of 272 ms between R-peaks, as error correction.
MIN_RR_DIST_MS = 272;

% File containing the ECG signal used to test R-peak detection. Modify the
% filename to your liking.
data_file = 'scope.csv';
data = csvread(data_file);
x = data(:,1);

% Before carrying out the R-peak detection algorithm, we do a preliminary
% analysis of the frequency content of our signal. In our case, there is a high
% amount of powerline interference present at 59 Hz since our electrode cables
% don't have any shielding.
signal_length = length(x);
frequency_resolution = Fs / signal_length;
frequency_axis = (0:(signal_length/2)) * frequency_resolution;
signal_no_dc = x - mean(x);
signal_fft = fft(signal_no_dc);
magnitude_spectrum = abs(signal_fft);
magnitude_spectrum = magnitude_spectrum(1:(signal_length/2 + 1));

% We use a Notch filter to eliminate powerline interference. Be sure to adjust
% the frequency and bandwidth of the filter according to your needs. Keep in
% mind this filter is not included in the paper and is unnecessary for R-peak
% detection. However, we include it so that our ECG can double as a cardiac
% monitor in addition to being a heart rate monitor.
notch_frequency = 59;
notch_bandwidth = 5;
notch_frequency_normalized = notch_frequency / (Fs/2);
notch_bandwidth_normalized = notch_bandwidth / (Fs/2);
[b, a] = pei_tseng_notch(notch_frequency_normalized,
                         notch_bandwidth_normalized);
notched = filter(b, a, x);

% High-pass filter, implemented as stated in the paper.
notched_bar = movmean(notched, window(N));
h_hat = notched - notched_bar;
h = abs(h_hat);

% Triangle template matching filter, implemented as stated in the paper.
t = zeros(length(h), 1);
for i = s+1:length(h)-s
    t(i) = (h(i) - h(i-s)) * (h(i) - h(i+s));
end

% First low-pass filter, implemented as stated in the paper.
l = movmean(t, window(L));

% Threshold calculation.  The paper suggests using the statistical mean of the
% output of the second low-pass filter as the value of theta.
ma = movmean(l, window(M));
theta = mean(l)/4;
th = beta*ma + theta;

% Holds current peak candidate values.
buffer = [];

% Indicates whether a peak candidate is under consideration.
flag = 0;

% Array to hold the detected R-peak indices.
r_peaks = [];

% Index of the previously detected R-peak, initialized to -1 to denote
% that a peak hasn't been detected yet.
prev_r_peak_idx = -1;

for i = 1:length(l)
    % Check if the current value is above the threshold.
    if l(i) >= th(i)
        % If no peak is under consideration, start considering the current one.
        if flag == 0
            % Reset the buffer for a new peak.
            buffer = [];

            % Set the flag to indicate a peak is under consideration.
            flag = 1;
        end
        % Append the current value to the buffer since the signal has met the
        % threshold. The output of the high pass filter is used for this.
        buffer = [buffer; i, h(i)];
  % If the current value is below threshold,
  else
        % and a peak was just previously under consideration,
        if flag == 1
            % find the maximum value in the buffer. This is a candidate R-peak.
            [~, max_idx] = max(buffer(:, 2));

            % Get the index of the maximum value, which is what we're interested
            % in.
            r_peak_idx = buffer(max_idx, 1);

            % If there was a previously detected R-peak,
            if prev_r_peak_idx ~= -1

                % we perform error correction by computing the distance to it.
                r_peak_distance = (r_peak_idx - prev_r_peak_idx) / Fs * 1000;

                % If the distance is too short,
                if r_peak_distance <= MIN_RR_DIST_MS

                    % and the current R-peak is higher than the previous one,
                    if h(r_peak_idx) >= h(prev_r_peak_idx)

                        % discard the previous R-peak.
                        r_peaks(end) = [];

                    % Otherwise,
                     else

                        % discard the current one.
                        continue;
                    end
                end
            end

            % Add the current R-peak to the list of detected R-peaks.
            r_peaks = [r_peaks; r_peak_idx];

            % Update the index of the last detected R-peak.
            prev_r_peak_idx = r_peak_idx;
        end

        % Reset the flag to indicate no peak is under consideration.
        flag = 0;
    end
end

% Heart rate readings displayed in units of bpm.
r_peak_times = r_peaks / Fs;
bpm_readings = 60 ./ diff(r_peak_times);
disp('bpm readings:');
disp(bpm_readings);

% x-axis for all the plots that follow in this script.
t_plot = (1:length(x)) / Fs;

figure(1);

% Plot of the ECG signal imported by the user.
subplot(3,1,1);
plot(t_plot, x);
xlabel('Time (s)');
ylabel('Amplitude');
title('Original ECG Signal');

% Frequency spectrum of the ECG signal.
subplot(3,1,2);
plot(frequency_axis, magnitude_spectrum);
xlabel('Frequency (Hz)');
ylabel('Magnitude');
title('Magnitude Spectrum (DC noise removed)');

% The original ECG signal after the notch filter has been applied to it.
subplot(3,1,3);
plot(t_plot, notched);
xlabel('Time (s)');
ylabel('Amplitude');
title('ECG Signal (Powerline Interference Removed)');

figure(2);

% Output of the high-pass filter with markers to indicate where R-peaks have
% been detected.
subplot(3,1,1);
plot(t_plot, h);
hold on;
scatter(r_peak_times, h(r_peaks), 'r', 'filled');
hold off;
xlabel('Time (s)');
ylabel('Amplitude');
title('High-pass Filtered ECG Signal with R-peaks Overlay');
legend('Filtered ECG', 'R-peaks');

% Output of the triangle template matching filter.
subplot(3,1,2);
plot(t_plot, t);
xlabel('Time (s)');
ylabel('Amplitude');
title('Triangle Template Matching Output');

% Output of the first low-pass filter with the threshold value superimposed.
% wherever the former is above the latter is where the R-peak search is
% performed.
subplot(3,1,3);
plot(t_plot, l, 'b');
hold on;
plot(t_plot, th, 'r');
hold off;
xlabel('Time (s)');
ylabel('Amplitude');
title("Low-pass Filtered Triangle Template Matching Output \
with Modified Threshold Overlay");
legend('Low-pass Filtered', 'Modified Threshold');

