#include "canbus_interface.h"
#include <QtCore>
#include <QtDebug>

CANBUS_Interface::CANBUS_Interface(QObject *parent) : QObject(parent)
{

}

void CANBUS_Interface::log(QString who, QString msg, int type = 0)
{
    qDebug()<<who<<msg;
}
void CANBUS_Interface::startSocketCan(QString name, int bitrate, int restart_ms)
{
    QString cmd;
    QProcess *proc = new QProcess();

    cmd = QString("ip link set %1 down").arg(name);
    proc->start("sh",QStringList()<<"-c"<<cmd);
    proc->waitForFinished();

    cmd = QString("ip link set %1 up type can bitrate %2 restart-ms %3").arg(name).arg(bitrate).arg(restart_ms);
    proc->start("sh", QStringList()<<"-c"<<cmd);
    proc->waitForFinished();
    proc->close();
    QString errorString;
    QList<QCanBusDeviceInfo> info = QCanBus::instance()->availableDevices(QStringLiteral("socketcan"),&errorString);

    if(!errorString.isEmpty()){
        log(Q_FUNC_INFO,"Canbus Error, no device available");
    }

    foreach (QCanBusDeviceInfo c, info) {
        if(c.name() == name){
            QCanBusDevice *dev = QCanBus::instance()->createDevice(QStringLiteral("socketcan"),c.name(),&errorString);
            connect(dev,&QCanBusDevice::errorOccurred,this,&CANBUS_Interface::OnCanbusError);
            connect(dev,&QCanBusDevice::framesReceived,this,&CANBUS_Interface::OnCanbusReceived);
            m_canbusDevices.append(dev);
        }
    }
}

bool CANBUS_Interface::deviceConnect(QString name)
{

    foreach (QCanBusDevice *dev, m_canbusDevices) {    
        log(Q_FUNC_INFO,QString("objname:%1, devname:%2").arg(dev->objectName()).arg(name));
        //if(dev->objectName() == name){
        //    return dev->connectDevice();
       // }
        dev->connectDevice();
    }
    return true;
}

bool CANBUS_Interface::deviceDisconnect(QString name)
{
    foreach (QCanBusDevice *dev, m_canbusDevices) {
        if(dev->objectName() == name){
            dev->disconnectDevice();
            return true;
        }
    }
    return false;
}

void CANBUS_Interface::writeFrame(QCanBusFrame f)
{
    foreach (QCanBusDevice *dev, m_canbusDevices) {
        dev->writeFrame(f);
    }
}
/*********** Slots Section *********************/

void CANBUS_Interface::OnCanbusError(QCanBusDevice::CanBusError error)
{
    QCanBusDevice *dev = static_cast<QCanBusDevice*>(sender());
    QString msg = dev->objectName() + " Error:"+ QString(error);
    log(Q_FUNC_INFO,msg);
}

void CANBUS_Interface::OnCanbusReceived()
{
    //log(Q_FUNC_INFO,"Message Received");
    QCanBusDevice *dev = static_cast<QCanBusDevice*>(sender());
    while(dev->framesAvailable()){
        QCanBusFrame f = dev->readFrame();
        emit FrameReceived(f);
    }
}
