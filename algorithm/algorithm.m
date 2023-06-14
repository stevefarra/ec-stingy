% Packages
pkg load signal;

% Macros
window = @(R) 2 * R + 1;

% Algorithm parameters
Fs = 360; % Sampling frequency
N = 25; % High-pass filter parameter
s = 7; % Triangle template matching parameter
L = 5; % Low-pass filter parameter
beta = 2.5; % Modified threshold calculation parameter
M = 150; % Modified threshold calculation parameter
MIN_RR_DIST_MS = 272; % Minimum distance between consecutive RR peaks

% ECG signal import
data_file = 'scope.csv'; % Adjust accoridngly
data = csvread(data_file);
x = data(:,1);

% Frequency analysis
signal_length = length(x);
frequency_resolution = Fs / signal_length;
frequency_axis = (0:(signal_length/2)) * frequency_resolution;
signal_no_dc = x - mean(x);
signal_fft = fft(signal_no_dc);
magnitude_spectrum = abs(signal_fft);
magnitude_spectrum = magnitude_spectrum(1:(signal_length/2 + 1));

% Notch filter
notch_frequency = 59; % Adjust accordingly
notch_bandwidth = 2; % Adjust accordingly
notch_frequency_normalized = notch_frequency / (Fs/2);
notch_bandwidth_normalized = notch_bandwidth / (Fs/2);
[b, a] = pei_tseng_notch(notch_frequency_normalized, notch_bandwidth_normalized);
notched = filter(b, a, x);

% High-pass filter
x_bar = movmean(x, window(N));
h_hat = x - x_bar;
h = abs(h_hat);

% Triangle template matching filter
for i = s+1:length(h)-s
    t(i) = (h(i) - h(i-s)) * (h(i) - h(i+s));
end

% Low-pass filter
l = movmean(t, window(L));

% Threshold calculation
ma = movmean(l, window(M));
theta = mean(l)/4;
th = beta*ma + theta;

% Peak detection initialization
buffer = [];    % Hold current peak candidate values
flag = 0;       % Indicate whether a peak candidate is under consideration
r_peaks = [];   % Array to hold the detected R-peak indices
prev_r_peak_idx = -1; % Index of the previously detected R-peak

% Peak detection
for i = 1:length(l)
    if l(i) >= th(i)  % Check if the current value is above the threshold
        if flag == 0  % If no peak is under consideration, start considering the current one
            buffer = [];  % Reset the buffer for new peak
            flag = 1;  % Set the flag to indicate peak under consideration
        end
        buffer = [buffer; i, y(i)];  % Append the current value to the buffer
    else  % If current value is below threshold
        if flag == 1  % If a peak was under consideration
            [~, max_idx] = max(buffer(:, 2));  % Find the maximum value in the buffer
            r_peak_idx = buffer(max_idx, 1);  % Get the index of the maximum value

            if prev_r_peak_idx ~= -1  % If there was a previously detected R-peak
                r_peak_distance = (r_peak_idx - prev_r_peak_idx) / Fs * 1000;  % Compute the distance to the previous R-peak
                if r_peak_distance <= MIN_RR_DIST_MS  % If the distance is too short
                    if y(r_peak_idx) >= y(prev_r_peak_idx)  % If the current R-peak is higher than the previous one
                        r_peaks(end) = [];  % Remove the last detected R-peak
                    else  % If the previous R-peak is higher
                        continue;  % Skip the current R-peak
                    end
                end
            end

            r_peaks = [r_peaks; r_peak_idx];  % Add the current R-peak to the list of detected R-peaks
            prev_r_peak_idx = r_peak_idx;  % Update the index of the last detected R-peak
        end
        flag = 0;  % Reset the flag to indicate no peak under consideration
    end
end


% Print heartrates to command window
r_peak_times = r_peaks / Fs;
bpm_readings = 60 ./ diff(r_peak_times);
disp('bpm readings:');
disp(bpm_readings);

% Plot the signals on separate subplots in the first figure
t_plot = (1:length(x)) / Fs;

figure(1);

% Original ECG signal
subplot(3,1,1);
plot(t_plot, x);
xlabel('Time (s)');
ylabel('Amplitude');
title('Original ECG Signal');

% FFT plot
subplot(3,1,2);
plot(frequency_axis, magnitude_spectrum);
xlabel('Frequency (Hz)');
ylabel('Magnitude');
title('Magnitude Spectrum (DC noise removed)');

% Notch-filtered signal
subplot(3,1,3);
plot(t_plot, notched);
xlabel('Time (s)');
ylabel('Amplitude');
title('ECG Signal (Powerline Interference Removed)');

% Plot the signals on separate subplots in the second figure
figure(2);

% Filtered ECG signal with R-peaks overlay
subplot(3,1,1);
plot(t_plot, y);
hold on;
scatter(r_peak_times, y(r_peaks), 'r', 'filled');
hold off;
xlabel('Time (s)');
ylabel('Amplitude');
title('High-pass Filtered ECG Signal with R-peaks Overlay');
legend('Filtered ECG', 'R-peaks');

% Triangle template matching output
subplot(3,1,2);
plot(t_plot, t);
xlabel('Time (s)');
ylabel('Amplitude');
title('Triangle Template Matching Output');

% Low-pass filtered signal with modified threshold overlay
subplot(3,1,3);
plot(t_plot, l, 'b');
hold on;
plot(t_plot, th, 'r');
hold off;
xlabel('Time (s)');
ylabel('Amplitude');
title('Low-pass Filtered Triangle Template Matching Output with Modified Threshold Overlay');
legend('Low-pass Filtered', 'Modified Threshold');

