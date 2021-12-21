#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "canbus_interface.h"
#include <QtCore>
#include <QtGui>
#include <QCanBusFrame>
#include <QDataStream>
#include <QTableView>
#include <QPushButton>
#include <QtSerialPort>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_adcModel = new IIO_Model();
    for(int i=0;i<8;i++){
        m_adcModel->addBlankRecord();
    }

    ui->tv_adc->setModel(m_adcModel);

    m_canInterface = new CANBUS_Interface();

    m_canInterface->startSocketCan("can0",250000);
    m_canInterface->startSocketCan("can1",250000);

    m_canInterface->deviceConnect("can0");
    m_canInterface->deviceConnect("can1");

    connect(m_canInterface,&CANBUS_Interface::FrameReceived,this,&MainWindow::OnCANBUS_MessageIn);

    connect(ui->pb_dac_update_0,&QPushButton::clicked,this,&MainWindow::OnUpdateDAC);
    connect(ui->pb_dac_update_1,&QPushButton::clicked,this,&MainWindow::OnUpdateDAC);
    connect(ui->pb_dac_update_2,&QPushButton::clicked,this,&MainWindow::OnUpdateDAC);
    connect(ui->pb_dac_update_3,&QPushButton::clicked,this,&MainWindow::OnUpdateDAC);

    connect(ui->pb_com_send,&QPushButton::clicked,this,&MainWindow::OnSerialWrite);
    connect(ui->pb_port_open,&QPushButton::clicked,this,&MainWindow::OnPortControl);

    serialPort = new QSerialPort();
    connect(serialPort,&QSerialPort::readyRead,this,&MainWindow::OnSerialPortReceived);

    connect(ui->pb_iso_out_0,&QPushButton::clicked,this,&MainWindow::OnSetDigitalOut);
    connect(ui->pb_iso_out_1,&QPushButton::clicked,this,&MainWindow::OnSetDigitalOut);
    connect(ui->pb_iso_out_2,&QPushButton::clicked,this,&MainWindow::OnSetDigitalOut);
    connect(ui->pb_iso_out_3,&QPushButton::clicked,this,&MainWindow::OnSetDigitalOut);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/************ SLOTS ******************/
void MainWindow::OnCANBUS_MessageIn(QCanBusFrame f)
{
    QString ioState;
    if(f.frameType() == QCanBusFrame::DataFrame){
        int msgID = f.frameId() & 0xfff;
        QByteArray b = f.payload();
        QDataStream ds(&b,QIODevice::ReadOnly);
        ds.setByteOrder(QDataStream::LittleEndian);
        ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
        int32_t intv;
        float pos,vel;
        int channel;
        switch(msgID){
        case 0x160: // a/d
        case 0x161:
        case 0x162:
        case 0x163:
            channel = (msgID - 0x160)*2;
            ds >> intv;
            m_adcModel->setRawData(channel++,intv);
            ds >> intv;
            m_adcModel->setRawData(channel++,intv);
            break;
        case 0x170: // resolver
        case 0x171:
            channel = (msgID - 0x170);
            //ds.setByteOrder(QDataStream::BigEndian);
            ds >> pos;
            ds >> vel;
            switch(channel){
            case 0:
                ui->le_resolver_pos1->setText(QString::number(pos,'f',3));
                ui->le_resolver_vecl1->setText(QString::number(vel,'f',3));
                break;
            case 1:
                ui->le_resolver_pos2->setText(QString::number(pos,'f',3));
                ui->le_resolver_vecl2->setText(QString::number(vel,'f',3));
                break;
            default:break;
            }

            break;
        case 0x140: // digital output
            ioState = "";
            for(int i=0;i<4;i++){
                if((b[0] & (1 <<i)) == (1 << i)){
                    ioState += QString(" DO[%1] ON").arg(i+1);
                }
                else{
                    ioState += QString(" DO[%1] OFF").arg(i+1);
                }
            }

            break;
        case 0x141: // digital input, 0~3 byte for ttl, 4~7 for iso
            ds >> intv;
            ui->le_ttl->setText("0x"+QString::number(intv,16).toUpper());
            ds >> intv;
            ui->le_iso_di->setText("0x"+QString::number(intv,16).toUpper());
            break;
        default:break;
        }
    }

    ui->tv_adc->viewport()->update();
}

void MainWindow::OnUpdateDAC()
{
    QPushButton *btn = static_cast<QPushButton*>(sender());
    QCanBusFrame f;
    QByteArray b;
    QDataStream ds(&b, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
    quint8 ch,option;
    qint16 dacv;
    float voltage;
    f.setFrameId(0x150);
    if(btn->objectName() == "pb_dac_update_0"){
        ch = 0;
        dacv = ui->le_dac_ch1->text().toInt();
        voltage = ui->le_dac_ch1->text().toFloat();
    }
    else if(btn->objectName() == "pb_dac_update_1"){
        ch = 1;
        dacv = ui->le_dac_ch2->text().toInt();
        voltage = ui->le_dac_ch2->text().toFloat();
    }
    else if(btn->objectName() == "pb_dac_update_2"){
        ch = 2;
        dacv = ui->le_dac_ch3->text().toInt();
        voltage = ui->le_dac_ch3->text().toFloat();
    }
    else if(btn->objectName() == "pb_dac_update_3"){
        ch = 3;
        dacv = ui->le_dac_ch4->text().toInt();
        voltage = ui->le_dac_ch4->text().toFloat();
    }
    if(ui->cb_dac_use_raw->isChecked()){
        option = 0;
        ds << ch;
        ds << option;
        ds << dacv;
    }
    else{
        option = 7;
        ds << ch;
        ds << option;
        ds << voltage;
    }
    f.setPayload(b);
    f.setFrameType(QCanBusFrame::DataFrame);
    f.setExtendedFrameFormat(true);

    m_canInterface->writeFrame(f);
}

void MainWindow::OnSerialWrite()
{
    if(serialPort->isOpen()){
        serialPort->write(ui->le_serial_data->text().toUtf8());
    }
}

void MainWindow::OnSerialPortReceived()
{
    QString msg = ui->te_serial_data->toPlainText();
    msg += QString(serialPort->readAll());
    ui->te_serial_data->setPlainText(msg);
//    QThread::msleep(200);

//    QString msg = QString("Recrived from:%1\nData:%2").arg(serialPort->portName()).arg(QString(serialPort->readAll()));
//    ui->te_serial_data->document()->setPlainText(msg);
}

void MainWindow::OnPortControl()
{
    if(serialPort->isOpen()){
        serialPort->close();
        ui->pb_port_open->setText("Open");
    }
    else{
        QString connection = QString("/dev/%1").arg(ui->cbtty->currentText());
        serialPort->setBaudRate(9600);
        serialPort->setParity(QSerialPort::NoParity);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);
        serialPort->setPortName(connection);

        if(serialPort->open(QIODevice::ReadWrite)){
            ui->pb_port_open->setText("Close");
        }
    }
}

void MainWindow::OnSetDigitalOut()
{
    QPushButton *btn = static_cast<QPushButton*>(sender());
    QCanBusFrame f;
    QByteArray b;
    QDataStream ds(&b, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
    quint8 ch,option;
    qint16 dacv;
    float voltage;
    f.setFrameId(0x140);
    if(btn->objectName() == "pb_iso_out_0"){
        ch = 0;
    }
    else if(btn->objectName() == "pb_iso_out_1"){
        ch = 1;
    }
    else if(btn->objectName() == "pb_iso_out_2"){
        ch = 2;
    }
    else if(btn->objectName() == "pb_iso_out_3"){
        ch = 3;
    }
    option = btn->isChecked()?1:0;
    ch |= (quint8)0x80;
    ds << ch;
    ds << option;
    f.setPayload(b);
    f.setFrameType(QCanBusFrame::DataFrame);
    f.setExtendedFrameFormat(true);

    m_canInterface->writeFrame(f);
}
