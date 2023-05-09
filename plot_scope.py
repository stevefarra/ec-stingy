import csv
import numpy as np
import matplotlib.pyplot as plt

# Load the ECG signal
with open('scope.csv', 'r') as csvfile:
    data = list(csv.reader(csvfile))
ECG = np.array([float(row[0]) for row in data])

# Set the sampling rate (Fs) and create the time vector
Fs = 360
time_vector = np.arange(len(ECG)) / Fs

# Plot the original ECG signal
fig, ax = plt.subplots()
line, = ax.plot(time_vector, ECG)
ax.set_title('Original ECG Signal')
ax.set_xlabel('Time (s)')
ax.set_ylabel('Amplitude')

# Define the function to display the x-y values on hover
def on_plot_hover(event):
    for line in ax.get_lines():
        if line.contains(event)[0]:
            x, y = event.xdata, event.ydata
            annot.xy = (x, y)
            annot.set_text(f'X: {x:.3f} s\nY: {y:.2f}')
            annot.set_visible(True)
            fig.canvas.draw_idle()
        else:
            annot.set_visible(False)
            fig.canvas.draw_idle()

# Create the annotation box
annot = ax.annotate("", xy=(0, 0), xytext=(20, 20),
                    textcoords="offset points",
                    bbox=dict(boxstyle="round,pad=0.3", edgecolor="black", facecolor="white"),
                    arrowprops=dict(arrowstyle="->"))
annot.set_visible(False)

# Connect the hover event to the function
fig.canvas.mpl_connect('motion_notify_event', on_plot_hover)

# Show the plot
plt.show()