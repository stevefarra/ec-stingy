#include "serialportreader.h"
#include <QSerialPortInfo>
#include <QCoreApplication>

SerialPortReader::SerialPortReader(QObject *parent) :
    QObject(parent)
{
    m_serialPort.setPortName("COM8");
    m_serialPort.setBaudRate(QSerialPort::Baud115200);

    connect(&m_serialPort, &QSerialPort::readyRead, this, &SerialPortReader::handleReadyRead);
    connect(&m_serialPort, &QSerialPort::errorOccurred, this, &SerialPortReader::handleError);
}

SerialPortReader::~SerialPortReader()
{
    if (m_serialPort.isOpen())
        m_serialPort.close();
}

void SerialPortReader::start()
{
    if (m_serialPort.open(QIODevice::ReadOnly)) {
        qDebug() << "Started reading from" << m_serialPort.portName();
    } else {
        qDebug() << "Could not open port" << m_serialPort.portName() << ":" << m_serialPort.errorString();
    }
}

void SerialPortReader::handleReadyRead()
{
    m_readData.append(m_serialPort.readAll());

    while (m_readData.contains('\n')) {
        QByteArray line = m_readData.left(m_readData.indexOf('\n'));
        m_readData.remove(0, line.size() + 1);

        line = line.trimmed();

        if (line.contains(',')) {
            QList<QByteArray> parts = line.split(',');
            emit newECGData(parts[0].toInt());
            emit newHeartRateData(parts[1].toUInt());
        } else {
            emit newECGData(line.toInt());
        }
    }
}

void SerialPortReader::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ReadError) {
        qDebug() << "An I/O error occurred while reading the data from port" << m_serialPort.portName() << ", error:" << m_serialPort.errorString();
    }
}
