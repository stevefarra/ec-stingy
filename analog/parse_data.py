# Initialize lists
frequency = []
magnitude = []
phase = []

# Open and read the file
with open('ecg_bandpass.txt', 'r') as f:
    lines = f.readlines()

# Process each line
for line in lines[1:]:
    line = line.strip()  # remove newline
    freq, rest = line.split('\t')
    magnitude_str, phase_str = rest.strip('()').split(',')
    magnitude_str = magnitude_str.strip('dB')
    phase_str = phase_str.strip('°')  # strip degree symbol

    # Append to lists
    frequency.append(float(freq))
    magnitude.append(float(magnitude_str))
    phase.append(float(phase_str))

# Save arrays to txt file
with open('data_clean.txt', 'w') as f:
    for freq, mag, ph in zip(frequency, magnitude, phase):
        f.write(f"{freq} {mag} {ph}\n")
