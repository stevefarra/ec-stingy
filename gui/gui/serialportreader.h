#ifndef SERIALPORTREADER_H
#define SERIALPORTREADER_H

#include <QObject>
#include <QSerialPort>

class SerialPortReader : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortReader(QObject *parent = nullptr);
    ~SerialPortReader();

    void start();

signals:
    void newData(const QByteArray &data);
    void newECGData(int ecgData);
    void newHeartRateData(unsigned int heartRate);

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort m_serialPort;
    QByteArray m_readData;
};

#endif // SERIALPORTREADER_H
