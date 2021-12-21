#ifndef CANBUS_INTERFACE_H
#define CANBUS_INTERFACE_H

#include <QObject>
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusDeviceInfo>

class CANBUSDevice{
public:
    QString name = "";
    bool connected = false;
    int bitRate = 250000;

};

class CANBUS_Interface : public QObject
{
    Q_OBJECT
public:
    explicit CANBUS_Interface(QObject *parent = nullptr);
    void log(QString who, QString msg, int type);
    void startSocketCan(QString name, int bitrate, int restart_ms = 1000);
    bool deviceConnect(QString name);
    bool deviceDisconnect(QString name);
    void writeFrame(QCanBusFrame f);
public slots:
    void OnCanbusError(QCanBusDevice::CanBusError error);
    void OnCanbusReceived();

signals:
    void FrameReceived(QCanBusFrame);

private:
    QList<QCanBusDevice*> m_canbusDevices;
};

#endif // CANBUS_INTERFACE_H
