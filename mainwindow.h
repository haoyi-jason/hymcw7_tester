#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "iio_model.h"
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusDeviceInfo>
class CANBUS_Interface;
class QSerialPort;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void OnCANBUS_MessageIn(QCanBusFrame f);
    void OnUpdateDAC();
    void OnSerialWrite();
    void OnPortControl();
    void OnSerialPortReceived();
    //void OnDigitalIn();
    void OnSetDigitalOut();
private:
    Ui::MainWindow *ui;

    IIO_Model *m_adcModel;

    CANBUS_Interface *m_canInterface;

    QSerialPort *serialPort;
};
#endif // MAINWINDOW_H
