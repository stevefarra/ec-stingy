import serial
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from collections import deque
import time

# Number of samples to retain
num_samples = 1080

# Data list for your signal
signal_a = deque(maxlen=num_samples)

# Open serial port
ser = serial.Serial('COM8', 115200)

# Function to get data from serial port and add to list
def get_data():
    if ser.in_waiting > 0:
        line = ser.readline().decode('utf-8').strip()
        data = line.split(',')
        
        if len(data) >= 1:
            a = float(data[0])

            signal_a.append(a)
            return True

    return False

# Create figure for plotting
fig, ax1 = plt.subplots()

# Function to update plot
def update(i):
    if get_data():
        ax1.cla()
        ax1.plot(signal_a, label='Signal a')
        ax1.legend(loc='upper left')
        ax1.set_title('Signal a')

# Using matplotlib's animation function to continuously update the plot
ani = FuncAnimation(fig, update, interval=1)

plt.show()
