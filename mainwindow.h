#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "iio_model.h"
#include <QCanBus>
#include <QCanBusDevice>
#include <QCanBusDeviceInfo>
#include "chartview.h"
#include "pid_datamodel.h"
#include <QtCore>

class CANBUS_Interface;
class QSerialPort;
class QTimer;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class seriesData{
public:
    seriesData():
        torque1(0),torque2(0),target(0),currentPV(0),pi(0),rpm(0){}
    double torque1,torque2,target,currentPV, pi, rpm;
};

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
    void OnTTLIOChanged(bool checked);
    void OnDACSliderMoved(int);
    void OnPIDControl();
    void OnParamRead();
    void OnParamUpdate();
private slots:
    void on_pbClear_clicked();
    void OnTimeout();

private:
    Ui::MainWindow *ui;

    IIO_Model *m_adcModel;

    CANBUS_Interface *m_canInterface;

    QSerialPort *serialPort;

    ChartView *m_view;

    QChart *m_chart;

    QValueAxis *ah,*av;

    QLineSeries *s;
    QList<QLineSeries*> m_series;

    PID_DataModel *m_piddata;

    seriesData m_seriesData;

    QList<QPointF> tq1_pts,tq2_pts, sp_pts, pv_pts, pi_pts, rpm_pts;

    QTimer *m_timer;
};
#endif // MAINWINDOW_H
