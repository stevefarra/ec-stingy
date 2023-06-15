pkg load signal;

% Load the ECG signal
data_file = 'scope.csv';
data = csvread(data_file);
x = data(:,1);
Fs = 360; % Sampling frequency

% View the frequency spectrum of a signal
plot((0:length(x) - 1) * (Fs / length(x)), abs(fft(x)));

f_notch = 59; % notch frequency in Hz
bw = 10; % bandwidth in Hz

% Normalized frequencies
wo = f_notch/(Fs/2);
bw_norm = bw/(Fs/2);

% Create the notch filter (butterworth)
[b, a] = pei_tseng_notch(wo, bw_norm);
ecg = filter(b, a, y_hat);


