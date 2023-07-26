# GUI Development
![GUI](../docs/visuals/gui.gif)
## Directory overview
[`gui.exe`](gui.exe): Compiled and linked from the provided source code. Run this to launch the GUI.

[`gui.pro`](gui.pro): Qt project file. Open this in Qt Creator to load the source files correctly.

[`main.cpp`](main.cpp): Entry point to the C++ program which creates and displays the main window.

[`mainwindow.cpp`](mainwindow.cpp), [`mainwindow.h`](mainwindow.h): Define the behavior of the main window and connects the GUI elements with the program's logic.


[`mainwindow.ui`](mainwindow.ui): Qt Designer form file. Allows for visual design in Qt Designer and used to generate a UI class for use in the application.

[`qcustomplot.cpp`](qcustomplot.cpp), [`qcustomplot.h`](qcustomplot.h): Source files for QCustomPlot, an external library for plotting and data visualization.

[`serialportreader.cpp`](serialportreader.cpp), [`serialportreader.h`](serialportreader.h): Source files for a class to read incoming data from the PC serial port.

