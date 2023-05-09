clear;
pkg load signal;

% QRS Detector function
function [R_Peaks] = QRS_Detector(ECG, Fs, F1, F2, W1, W2, Beta)

  % Bandpass filter
  [b, a] = butter(3, [F1, F2] / (Fs/2));
  Filtered = filter(b, a, ECG);

  % Square and moving average
  Sq_Filtered = Filtered .^ 2;
  MAqrs = movmean(Sq_Filtered, W1);
  MAbeat = movmean(Sq_Filtered, W2);

  % Threshold calculation
  z = mean(Sq_Filtered);
  Alpha = Beta * z + MAbeat;
  THR1 = MAbeat + Alpha;

  % Detect blocks of interest
  BlocksOfInterest = zeros(1, length(MAqrs));
  for n = 1:length(MAqrs)
    if MAqrs(n) > THR1(n)
      BlocksOfInterest(n) = 0.1;
    else
      BlocksOfInterest(n) = 0;
    end
  end

  % Find onset and offset
  onset = find(diff(BlocksOfInterest) > 0);
  offset = find(diff(BlocksOfInterest) < 0);

  % Detect R-peaks
  R_Peaks = [];
  THR2 = W1;
  for j = 1:length(onset)
    if (offset(j) - onset(j)) >= THR2
      [~, idx] = max(ECG(onset(j):offset(j)));
      R_Peaks = [R_Peaks, onset(j) + idx - 1];
    end
  end
end

% Load the ECG signal
data = csvread('scope.csv');
ECG = data(:,1);
Fs = 360; % Sampling frequency

% Define constants
F1 = 8; % Lower cut-off frequency
F2 = 20; % Upper cut-off frequency
W1 = round(97 * 0.001 * Fs); % QRS moving average window size
W1 = W1 + mod(W1 - 1, 2); % Round to the nearest odd integer
W2 = round(611 * 0.001 * Fs); % Beat moving average window size
W2 = W2 + mod(W2 - 1, 2); % Round to the nearest odd integer
Beta = 0.08; % Offset fraction

% Run QRS detection
R_Peaks = QRS_Detector(ECG, Fs, F1, F2, W1, W2, Beta);

% Calculate time between R-peaks
R_Peak_Times = diff(R_Peaks) / Fs;

% Convert times to heartrate (bpm)s
hr_bpm = 60 ./ R_Peak_Times;

% Load ground truth data from "r_peaks.txt"
fid = fopen('r_peaks.txt', 'r');
gt_data_cell = textscan(fid, 'Peak %*d: %f s', 'CollectOutput', true);
fclose(fid);
gt_data = gt_data_cell{1}';  % Convert cell array to row vector

% Calculate ground truth heart rates (bpm)
gt_diff = diff(gt_data);
gt_hr = 60 ./ gt_diff;

% Compare ground truth heart rates with algorithm heart rates
gt_hr_last6 = gt_hr(end-5:end);

disp('Ground truth heart rates (last 6, bpm):');
disp(gt_hr_last6);

disp('Algorithm heart rates (bpm):');
disp(hr_bpm);

% Calculate the difference between ground truth and algorithm heart rates
hr_diff = abs(gt_hr_last6 - hr_bpm);
