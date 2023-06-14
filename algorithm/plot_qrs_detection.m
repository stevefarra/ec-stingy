% Load data
data = csvread('output.csv');

% Extract columns into variables
ecg = data(:,1);
y = data(:,2);
t = data(:,3);
l = data(:,4);
th = data(:,5);
aoi = data(:,6)*60000;

% Create a new figure
figure

% Subplot for ECG signal
subplot(4,1,1);
plot(ecg);
title('ECG signal');

% Subplot for output of highpass filter
subplot(4,1,2);
plot(y);
title('Output of highpass filter');

% Subplot for output of triangle filter
subplot(4,1,3);
plot(t);
title('Output of triangle filter');

% Subplot for output of lowpass filter, threshold, and areas of interest
subplot(4,1,4);
hold on; % This allows multiple plots on the same axes
plot(l);
plot(th);
plot(aoi,'k');
hold off;
legend('Lowpass Filter', 'Threshold', 'Areas of Interest');
title('Outputs overlayed');
