% Author: Steve Farra
%
% Description: This Octave script plots the waveforms generated by
%              "algorithm.c". Simply run that program to generate a data file
%              before running this script to view the waveforms.
%
% License: This script is licensed under the MIT license.
%          See the accompanying LICENSE file for the full text of the license.

% Load data written by "algorithm.c".
data = csvread('output.csv');

% Extract columns into variables. Make sure the printf() function in
% "algorithm.c" corresponds to the columns extracted below.

% Output of the notch filter. This signal is used as the cardiac monitor.
ecg = data(:,1);

% Output of the high pass filter.
y = data(:,2);

% Output of the triangle template matching filter.
t = data(:,3);

% Output of the first low pass filter.
l = data(:,4);

% Threshold value.
th = data(:,5);

% Area of interest flag, scaled up to be visible on the plot (you may need to
% adjust the scale factor).
aoi = data(:,6)*60000;

% Subplot for the ECG signal.
subplot(4,1,1);
plot(ecg);
title('ECG signal');

% Subplot for the output of the high pass filter.
subplot(4,1,2);
plot(y);
title('Output of highpass filter');

% Subplot for the output of the triangle filter.
subplot(4,1,3);
plot(t);
title('Output of triangle filter');

% Subplot for the output of the lowpass filter, threshold, and areas of
% interest.
subplot(4,1,4);
hold on; % This allows multiple plots on the same axes
plot(l);
plot(th);
plot(aoi,'k');
hold off;
legend('Lowpass Filter', 'Threshold', 'Areas of Interest');
title('Outputs overlayed');
