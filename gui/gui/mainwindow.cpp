#include "mainwindow.h"
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ecgPlot(new QCustomPlot(this)),
    heartRatePlot(new QCustomPlot(this)),
    heartRateLabel(new QCPItemText(heartRatePlot)),
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
    ecgPlot->plotLayout()->insertRow(0); // Adds an empty row above the default axis rect
    ecgPlot->plotLayout()->addElement(0, 0, new QCPTextElement(ecgPlot, "ECG", QFont("sans", 12, QFont::Bold)));
    ecgPlot->xAxis->setRange(0, 10);
    ecgPlot->yAxis->setRange(-500, 500);

    heartRatePlot->addGraph();
    heartRatePlot->plotLayout()->insertRow(0); // Adds an empty row above the default axis rect
    heartRatePlot->plotLayout()->addElement(0, 0, new QCPTextElement(heartRatePlot, "Heart rate (bpm)", QFont("sans", 12, QFont::Bold)));
    heartRatePlot->graph(0)->setLineStyle(QCPGraph::lsLine);
    heartRatePlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
    heartRatePlot->xAxis->setRange(0, 10);
    heartRatePlot->yAxis->rescale();

    heartRatePlot->replot();

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

    // Adjust both axes with a margin
    heartRatePlot->xAxis->setRange(*std::min_element(heartRateTime.constBegin(), heartRateTime.constEnd()),
                                   *std::max_element(heartRateTime.constBegin(), heartRateTime.constEnd()) + 1);
    heartRatePlot->yAxis->setRange(*std::min_element(heartRateData.constBegin(), heartRateData.constEnd()) - 1,
                                   *std::max_element(heartRateData.constBegin(), heartRateData.constEnd()) + 2);

    heartRateLabel->position->setType(QCPItemPosition::ptPlotCoords);
    heartRateLabel->position->setCoords(key, data);
    heartRateLabel->setText(QString::number(data));
    heartRateLabel->setFont(QFont(font().family(), 12));
    heartRateLabel->setColor(Qt::blue);
    heartRateLabel->setPadding(QMargins(1, 1, 1, 1));
    heartRateLabel->setPositionAlignment(Qt::AlignHCenter|Qt::AlignBottom);
    heartRateLabel->position->setPixelPosition(heartRateLabel->position->pixelPosition() + QPoint(0, -10));

    heartRatePlot->replot();
}






