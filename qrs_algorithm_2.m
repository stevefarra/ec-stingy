clear;

% Load the ECG signal
data_file = 'scope.csv';
data = csvread(data_file);
ECG = data(:,1);
Fs = 360; % Sampling frequency

% Algorithm parameters
N = 25;      % High-pass filter parameter
s = 7;       % Triangle template matching parameter
L = 5;       % Low-pass filter parameter
beta = 2.5;  % Modified threshold calculation parameter
M = 150;     % Modified threshold calculation parameter

% Initialize output signals
y_hat = zeros(length(ECG), 1);
y = zeros(length(ECG), 1);
t = zeros(length(ECG), 1);
l = zeros(length(ECG), 1);
l_avg = zeros(length(ECG), 1);
th = zeros(length(ECG), 1);

% Implement the high-pass filter
for i = N+1:length(ECG)-N-1
    y_hat(i+1) = y_hat(i) + ECG(i+1) - ECG(i) + 1/(2*N+1)*(ECG(i-N) - ECG(i+N+1));
    y(i) = abs(y_hat(i));
end

% Implement the triangle template matching
for i = s+1:length(y)-s
    t(i) = (y(i) - y(i-s)) * (y(i) - y(i+s));
end

% Implement the low-pass filter
for i = L+1:length(t)-L-1
    l(i+1) = l(i) + 1/(2*L+1)*(t(i+L+1) - t(i-L));
end

% Initialize the modified threshold calculation
theta = mean(l)/4;
l_avg(M+1) = sum(l(1:2*M+1)) / (2*M+1);

% Implement the modified threshold calculation
for i = M+1:length(l)-M-1
    l_avg(i+1) = l_avg(i) + 1/(2*M+1)*(l(i+M+1) - l(i-M));
    th(i) = beta * l_avg(i) + theta;
end

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
        buffer = [buffer; i, y(i)];
    else
        if flag == 1
            [~, max_idx] = max(buffer(:, 2));
            r_peak_idx = buffer(max_idx, 1);

            if prev_r_peak_idx ~= -1
                r_peak_distance = (r_peak_idx - prev_r_peak_idx) / Fs * 1000;
                if r_peak_distance <= 272
                    if y(r_peak_idx) >= y(prev_r_peak_idx)
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
t_plot = (1:length(ECG)) / Fs;
r_peak_times = r_peaks / Fs;

figure;
% Original ECG signal
subplot(4,1,1);
plot(t_plot, ECG);
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
