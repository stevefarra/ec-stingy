% Load the data
data = dlmread('data_clean.txt');

% Extract vectors
frequency = data(:, 1);
magnitude = data(:, 2);
phase = data(:, 3);

% Plot Magnitude
figure(1);
semilogx(frequency, magnitude);
title('Bode Magnitude Plot');
xlabel('Frequency (Hz)');
ylabel('Magnitude (dB)');

% Plot Phase
figure(2);
semilogx(frequency, phase);
title('Bode Phase Plot');
xlabel('Frequency (Hz)');
ylabel('Phase (degrees)');
