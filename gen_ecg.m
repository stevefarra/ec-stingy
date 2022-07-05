# Required by ecgsyn.m to run
pkg load signal

# Parameters
SAMPLING_FREQ_HZ = 256;
NUM_BEATS = 10;
FILENAME = 'ecg.txt';

ecg_mv = ecgsyn(SAMPLING_FREQ_HZ, NUM_BEATS);

# Output of ecgsyn() is in millivolts so convert to volts
ecg = ecg_mv * 1e-3;

# Write to file compatible with LTspice
file_id = fopen(FILENAME,'w');
for k = 1: length(ecg)
   fprintf(file_id, '%6.6f,%6.6f\n' , (k-1)/SAMPLING_FREQ_HZ, ecg(k));
end
fclose(file_id);
