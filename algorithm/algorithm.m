% Packages
pkg load signal;

% Algorithm parameters
Fs = 360; % Sampling frequency
N = 25; % High-pass filter parameter
s = 7; % Triangle template matching parameter
L = 5; % Low-pass filter parameter
beta = 2.5; % Modified threshold calculation parameter
M = 150; % Modified threshold calculation parameter
MIN_RR_DIST_MS = 272; % Minimum distance between consecutive RR peaks

% ECG signal import
data_file = 'scope.csv';
data = csvread(data_file);
x = data(:,1);

% Frequency analysis
signal_length = length(x); % Compute the length of the signal
frequency_resolution = Fs / signal_length; % Calculate the frequency resolution
frequency_axis = (0:(signal_length/2)) * frequency_resolution; % Generate the frequency axis
signal_no_dc = x - mean(x); % Remove the DC offset from the signal
signal_fft = fft(signal_no_dc); % Compute the Fast Fourier Transform (FFT) of the signal without DC offset
magnitude_spectrum = abs(signal_fft); % Compute the magnitude spectrum
magnitude_spectrum = magnitude_spectrum(1:(signal_length/2 + 1)); % Keep only the first N/2 + 1 points

% Notch filter
f_notch = 59; % Notch frequency in Hz
bw = 2; % Notch bandwidth in Hz
wo = f_notch/(Fs/2); % Normalized frequency
bw_norm = bw/(Fs/2); % Normalized bandwidth
[b, a] = pei_tseng_notch(wo, bw_norm); % Filter coefficients
notched = filter(b, a, x); % Output signal

% High-pass filter
x_bar = movmean(x, 2*N + 1); % Compute the moving average over a window of 2N+1 samples of the notched signal
y_hat = x - x_bar; % Subtract the moving average from the original notched signal, preserving the high frequencies
y = abs(y_hat); % Take the absolute of the resultant signal

% Triangle template matching filter
for i = s+1:length(y)-s
    % For each sample, compute the product of the differences from two samples away,
    % in both forward and backward directions.
    % This step is designed to highlight the QRS complex in the ECG signal
    t(i) = (y(i) - y(i-s)) * (y(i) - y(i+s));
end

% Low-pass filter
% Compute a moving average over a window of 2L+1 samples of the triangle template matching output,
% This is to smooth the signal and reduce high-frequency noise
l = movmean(t, 2*L + 1);

% Threshold calculation
ma = movmean(l, 2*M + 1); % Compute a moving average over a large window (2M+1 samples) of the low-pass filtered signal
theta = mean(l)/4; % Compute a baseline threshold as a quarter of the mean of the low-pass filtered signal
th = beta*ma + theta; % Calculate the final threshold for peak detection as the sum of the baseline threshold and the moving average, weighted by a factor beta

% Peak detection initialization
buffer = [];    % Buffer to hold current peak candidate values
flag = 0;       % Flag to indicate whether a peak candidate is under consideration
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

