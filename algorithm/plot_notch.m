% Load data
data = csvread('output.csv');

% Extract columns into variables
x = data(:,1);
ecg = data(:,2);

% Create a new figure
figure

% Subplot for input signal
subplot(4,1,1);
plot(x);
title('Input signal');

% Subplot for output signal
subplot(4,1,2);
plot(ecg);
title('ECG signal');


