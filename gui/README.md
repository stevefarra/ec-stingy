# GUI Development
![GUI](../docs/visuals/gui.gif)
## Directory overview
[`gui.exe`](gui.exe): Execute this file to launch the GUI, compiled and linked from the provided source code.

[`gui.pro`](gui.pro): Qt project file. Open this in Qt Creator to load the source files correctly.

[`main.cpp`](main.cpp): Entry point to the C++ program which creates and displays the main window.

[`mainwindow.cpp`](mainwindow.cpp), [`mainwindow.h`](mainwindow.h): Define the behavior of the main window and connects the GUI elements with the program's logic.

[`mainwindow.ui`](mainwindow.ui): Qt Designer form file. Allows for visual design in Qt Designer and used to generate a UI class for use in the application.

[`qcustomplot.cpp`](qcustomplot.cpp), [`qcustomplot.h`](qcustomplot.h): Source files for QCustomPlot, an external library for plotting and data visualization.

[`serialportreader.cpp`](serialportreader.cpp), [`serialportreader.h`](serialportreader.h): Source files for a class to read incoming data from the PC serial port.

## The Be-GUI-led: Using LLMs to accelerate prototyping

The specifications are straightforward: The GUI reads comma-separated integer data from the serial port sent at a sampling frequency of 360 Hz. The first integer represents the ECG signal, and the second, if available, represents the most recent heart rate reading in BPM. Both are displayed on separate plots.

Given the importance of processing speed and real-time data interaction, we chose C++ for implementation. To help navigate the large overhead and inherent complexities of the language, [GPT-4 comes to the rescue](https://chat.openai.com/share/ca196f5d-4867-4609-83e8-bf3a4e4dab14) by providing assistance every step of the way, from set-up to (incremental) development, debugging, and documentation.

## Next steps

**Add a menu:** While this GUI serves as a proof-of-concept, enhancing UX could involve adding numerous features. Some sort of interface to reset the plots, toggle data collection, or modify the appearance would be a good starting point.

**Make it cross-platform:** Currently, the project build supports only Windows. However, for broader support, consider using [Qt's macOS support](https://doc.qt.io/qt-6/macos.html) through Xcode.