#include "mainwindow.h"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ecgPlot(new QCustomPlot(this)),
    heartRatePlot(new QCustomPlot(this)),
    startTime(QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0),
    serialReader(new SerialPortReader(this))
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(ecgPlot);
    layout->addWidget(heartRatePlot);

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setLayout(layout);

    setCentralWidget(mainWidget);

    ecgPlot->addGraph();
    ecgPlot->xAxis->setRange(0, 10);
    ecgPlot->yAxis->setRange(-500, 500);

    heartRatePlot->addGraph();
    heartRatePlot->xAxis->setRange(0, 10);
    heartRatePlot->yAxis->rescale();

    connect(serialReader, &SerialPortReader::newECGData, this, &MainWindow::addECGData);
    connect(serialReader, &SerialPortReader::newHeartRateData, this, &MainWindow::addHeartRateData);

    serialReader->start();
}


MainWindow::~MainWindow()
{
    delete serialReader;
}

void MainWindow::addECGData(int data)
{
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0 - startTime;

    ecgData.append(data);
    ecgTime.append(key);

    // Adjust the size of data to keep only the last 10 seconds of data (3600 points)
    while (ecgTime.size() > 3600) {
        ecgTime.removeFirst();
        ecgData.removeFirst();
    }

    // Adjust the x-axis range to make the data scroll
    if (key > ecgPlot->xAxis->range().upper) {
        ecgPlot->xAxis->setRange(key - 10, key);
    }

    ecgPlot->graph(0)->setData(ecgTime, ecgData);
    ecgPlot->replot();
}


void MainWindow::addHeartRateData(unsigned int data)
{
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0 - startTime;

    heartRateData.append(data);
    heartRateTime.append(key);

    heartRatePlot->graph(0)->setData(heartRateTime, heartRateData);

    // Auto-adjust both the x-axis and the y-axis
    heartRatePlot->xAxis->rescale();
    heartRatePlot->yAxis->rescale();

    heartRatePlot->replot();
}




