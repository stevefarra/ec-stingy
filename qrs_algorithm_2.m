% Load the ECG signal
data_file = 'scope.csv';
data = csvread(data_file);
x = data(:,1);
Fs = 360; % Sampling frequency

% Algorithm parameters
N = 25;      % High-pass filter parameter
s = 7;       % Triangle template matching parameter
L = 5;       % Low-pass filter parameter
beta = 2.5;  % Modified threshold calculation parameter
M = 150;     % Modified threshold calculation parameter

x_bar = zeros(length(x), 1);
y = zeros(length(x), 1);
t = zeros(length(x), 1);
l = zeros(length(x), 1);
ma = zeros(length(x), 1);
theta = zeros(length(x), 1);
th = zeros(length(x), 1);

% Implement the high-pass filter
x_bar = movmean(x, 2*N + 1);
y = abs(x - x_bar);

% Implement the triangle template matching
for i = s+1:length(y)-s
    t(i) = (y(i) - y(i-s)) * (y(i) - y(i+s));
end

% Implement the low-pass filter
l = movmean(t, 2*L + 1);

% Implement the modified threshold calculation
ma = movmean(l, 2*M + 1);
theta = mean(l)/4;
th = beta*ma + theta;

% Peak detection initialization
buffer = [];
flag = 0;
r_peaks = [];
prev_r_peak_idx = -1;

% Implement the peak detection
for i = 1:length(l)
    if l(i) >= th(i)
        if flag == 0
            buffer = [];
            flag = 1;
        end
        buffer = [buffer; i, x(i)];
    else
        if flag == 1
            [~, max_idx] = max(buffer(:, 2));
            r_peak_idx = buffer(max_idx, 1);

            if prev_r_peak_idx ~= -1
                r_peak_distance = (r_peak_idx - prev_r_peak_idx) / Fs * 1000;
                if r_peak_distance <= 272
                    if x(r_peak_idx) >= x(prev_r_peak_idx)
                        r_peaks(end) = [];
                    else
                        continue;
                    end
                end
            end

            r_peaks = [r_peaks; r_peak_idx];
            prev_r_peak_idx = r_peak_idx;
        end
        flag = 0;
    end
end

% Plot the signals on separate subplots in the same figure
t_plot = (1:length(x)) / Fs;
r_peak_times = r_peaks / Fs;

% Print heartrates to command window
disp(60 ./ diff(r_peak_times));

figure;
% Original ECG signal
subplot(4,1,1);
plot(t_plot, x);
xlabel('Time (s)');
ylabel('Amplitude');
title('Original ECG Signal');

% Filtered ECG signal with R-peaks overlay
subplot(4,1,2);
plot(t_plot, y);
hold on;
scatter(r_peak_times, y(r_peaks), 'r', 'filled');
hold off;
xlabel('Time (s)');
ylabel('Amplitude');
title('High-pass Filtered ECG Signal with R-peaks Overlay');
legend('Filtered ECG', 'R-peaks');

% Triangle template matching output
subplot(4,1,3);
plot(t_plot, t);
xlabel('Time (s)');
ylabel('Amplitude');
title('Triangle Template Matching Output');

% Low-pass filtered signal with modified threshold overlay
subplot(4,1,4);
plot(t_plot, l, 'b');
hold on;
plot(t_plot, th, 'r');
hold off;
xlabel('Time (s)');
ylabel('Amplitude');
title('Low-pass Filtered Triangle Template Matching Output with Modified Threshold Overlay');
legend('Low-pass Filtered', 'Modified Threshold');
