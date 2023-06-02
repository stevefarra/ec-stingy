pkg load signal

% Parameters
DATA_FILE = 'ecg.txt';
Fs = 256;
NUM_BEATS = 10;

% Generate ECG signal
sig_mV = ecgsyn(Fs, NUM_BEATS);

len = length(sig_mV);

% Create time vector, in seconds
t = transpose(linspace(0,len/Fs,len));

% Create signal vector, in volts
sig_V = sig_mV * 1e-3;

% Write to file compatible with LTspice:
% File containing comma-separated rows with no headers
file_id = fopen(DATA_FILE,'w');
for k = 1:len
   fprintf(file_id, '%6.6f,%6.6f\n' , t(k), sig_V(k));
end
fclose(file_id);

% Print figure
plot(t, sig_mV);
title('Simulated ECG signal');
xlabel('Time (s)');
ylabel('Potential (mV)');
