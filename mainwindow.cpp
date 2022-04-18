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
#include <QScrollBar>
#include <QTimer>

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
    connect(ui->pb_sv1_en,&QPushButton::clicked,this,&MainWindow::OnSetDigitalOut);
    connect(ui->pb_sv1_on,&QPushButton::clicked,this,&MainWindow::OnSetDigitalOut);
    connect(ui->pb_sv2_en,&QPushButton::clicked,this,&MainWindow::OnSetDigitalOut);
    connect(ui->pb_sv2_on,&QPushButton::clicked,this,&MainWindow::OnSetDigitalOut);

    connect(ui->pb_ttl_lb_dir,&QPushButton::clicked,this,&MainWindow::OnTTLIOChanged);
    connect(ui->pb_ttl_hb_dir,&QPushButton::clicked,this,&MainWindow::OnTTLIOChanged);
    connect(ui->pb_ttl_set_lb,&QPushButton::clicked,this,&MainWindow::OnSetDigitalOut);
    connect(ui->pb_ttl_set_hb,&QPushButton::clicked,this,&MainWindow::OnSetDigitalOut);

    //m_view = ui->graphicsView;
    // insert series
    m_chart = new QChart;
    QValueAxis *xb = new QValueAxis;
    QValueAxis *yl = new QValueAxis;
    QValueAxis *yr = new QValueAxis;

    QDateTimeAxis *xt = new QDateTimeAxis;
    xt->setTickCount(9);
    xt->setFormat("mm:ss");
//    xt->setMin(QDateTime::currentMSecsSinceEpoch());
//    xt->setMax(QDateTime::currentMSecsSinceEpoch()+120);
    xt->setMin(QDateTime::currentDateTime());
    xt->setMax(QDateTime::currentDateTime().addSecs(120));
    xb->setMin(0);
    xb->setMax(100);
    yl->setMin(-10);
    yl->setMax(10);
    yr->setMin(-100);
    yr->setMax(100);

    xt->setTitleText("Time(s)");
    xb->setTitleText("Time");
    yl->setTitleText("Voltage");
    yr->setTitleText("Value");
    m_chart->addAxis(xb,Qt::AlignBottom);
    m_chart->addAxis(yl,Qt::AlignLeft);
    m_chart->addAxis(yr,Qt::AlignRight);
    m_chart->addAxis(xt,Qt::AlignTop);
    // add series
    QLineSeries *s = new QLineSeries;
    m_chart->addSeries(s);
    s->attachAxis(xt);
    s->attachAxis(yl);
    s->setName("TQ 1-1");

    s = new QLineSeries;
    m_chart->addSeries(s);
    s->attachAxis(xt);
    s->attachAxis(yl);
    s->setName("TQ 1-2");

    s = new QLineSeries;
    m_chart->addSeries(s);
    s->attachAxis(xt);
    s->attachAxis(yr);
    s->setName("PV");

    s = new QLineSeries;
    m_chart->addSeries(s);
    s->attachAxis(xt);
    s->attachAxis(yr);
    s->setName("SP");

    s = new QLineSeries;
    m_chart->addSeries(s);
    s->attachAxis(xt);
    s->attachAxis(yl);
    s->setName("RPM");

    s = new QLineSeries;
    m_chart->addSeries(s);
    s->attachAxis(xt);
    s->attachAxis(yl);
    s->setName("PI");

    //    // add dummy data
//    s = new QLineSeries();
//    m_chart->addSeries(s);
//    s->attachAxis(ah);
//    s->attachAxis(av);
//    s->setName("Test");
//    for(int i=0;i<1000;i++){
//        s->append(QPointF(i,i));
//    }

    m_piddata = new PID_DataModel;

    m_piddata->insertData("PID_KP_P","0",1);
    m_piddata->insertData("PID_KP_S","0",2);
    m_piddata->insertData("PID_KI_S","0",3);
    m_piddata->insertData("PID_KD_S","0",4);
    m_piddata->insertData("PID_MOT_T_MAX","0",5);
    m_piddata->insertData("PID_MOT_S_MAX","0",6);
    m_piddata->insertData("PID_G_A2M","0",7);
    m_piddata->insertData("PID_TQBC_MAX","0",8);
    m_piddata->insertData("PID_TQBC_MIN","0",9);
    m_piddata->insertData("PID_G","0",10);
    m_piddata->insertData("PID_ZCP","0",11);
//    m_piddata->insertData("PID_TQBC_MIN","0",9);

    m_piddata->insertData("MODE 1","0",0);
    m_piddata->insertData("MB 1-1","0",0);
    m_piddata->insertData("MB 1-2","0",0);
    m_piddata->insertData("SP 1","0",0);
    m_piddata->insertData("TQ 1-1","0",0);
    m_piddata->insertData("TQ 1-2","0",0);


    ui->tvPidParam->setModel(m_piddata);
    ui->tvPidParam->viewport()->update();
    //    connect(m_axisModel,&axisModel::dataChanged,this,&chartView2::handleAxisModelChanged);
        //ui->graphicsView->setRubberBand(QChartView::HorizontalRubberBand);

        ui->graphicsView->setChart(m_chart);

        ui->graphicsView->setContextMenuPolicy(Qt::ActionsContextMenu);
        //connect(m_view,&QChartView::customContextMenuRequested,this,&chartView2::showChartContextMenu);

    connect(ui->pbPIDStart,&QPushButton::clicked,this,&MainWindow::OnPIDControl);
    connect(ui->pbPIDStop,&QPushButton::clicked,this,&MainWindow::OnPIDControl);
    connect(ui->pbPIDClearFault,&QPushButton::clicked,this,&MainWindow::OnPIDControl);
    connect(ui->pbPIDSP,&QPushButton::clicked,this,&MainWindow::OnPIDControl);

    QTimer *mt = new QTimer;
    connect(mt,&QTimer::timeout,this,&MainWindow::OnTimeout);
    mt->setInterval(1000);
    mt->start();

    connect(ui->pb_read_pid,&QPushButton::clicked,this,&MainWindow::OnParamRead);
    connect(ui->pb_update_pid,&QPushButton::clicked,this,&MainWindow::OnParamUpdate);
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
        int16_t shortv;
        quint8 b8;
        float pos,vel;
        int channel;
        switch(msgID){
        case 0x101: // parameters
            ds >> b8;
            ds >> pos;
            m_piddata->updateData(b8,pos);
            ui->tvPidParam->viewport()->update();
            break;
        case 0x110: // pid
            if(b[0] == 0x01){
                m_piddata->updateData("MODE 1","IDLE");
            }
            else if(b[0] == 0x02){
                m_piddata->updateData("MODE 1","ACTIVE");
            }
            else{
                m_piddata->updateData("MODE 1","UNKNOW");
            }
            ds >> shortv;
            ds >> shortv;
            ds >> shortv;
            m_piddata->updateData("MB 1-1",QString::number(shortv));
            ds >> shortv;
            m_piddata->updateData("MB 1-2",QString::number(shortv));

            break;
        case 0x111: // pid
            ds >> pos;
            if(ui->cbPIDMode->currentIndex() == 0){
                m_piddata->updateData("PV",QString::number(pos));
                m_seriesData.currentPV = pos;
            }
            //qDebug()<<"SP 1:"<<QString::number(pos);
            ds >> pos;
            //qDebug()<<"SP 2:"<<QString::number(pos);
            break;
        case 0x112:
            ds >> pos;
            m_piddata->updateData("TQ 1-1",QString::number(pos));
            m_seriesData.torque1 = pos;
            ds >> pos;
            m_piddata->updateData("TQ 1-2",QString::number(pos));
            m_seriesData.torque2 = pos;
            break;
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
            pos /= 60.; // to degree
            vel *= 60.; // to rpm
            switch(channel){
            case 0:
                ui->le_resolver_pos1->setText(QString::number(pos,'f',3));
                ui->le_resolver_vecl1->setText(QString::number(vel,'f',3));
                if(ui->cbPIDMode->currentIndex() != 0){
                    m_piddata->updateData("PV",QString::number(pos));
                    m_seriesData.currentPV = pos;
                }
                m_seriesData.pi = pos * 3.1416/180.;
                m_seriesData.rpm = vel;
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
            ui->le_ttl_lb->setText("0x"+QString::number(intv&0xff,16).toUpper());
            ui->le_ttl_hb->setText("0x"+QString::number((intv>>8)&0xff,16).toUpper());
            ds >> intv;
            ui->le_iso_di->setText("0x"+QString::number(intv,16).toUpper());
            break;
        default:break;
        }
    }

    ui->tv_adc->viewport()->update();
    //qDebug()<<"CAN Message IN";
}

void MainWindow::OnDACSliderMoved(int value)
{
    QScrollBar *sb = static_cast<QScrollBar*>(sender());
    QCanBusFrame f;
    QByteArray b;
    QDataStream ds(&b, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
    quint8 ch,option;
    qint16 dacv;
    float voltage;
    f.setFrameId(0x150);

    if(sb->objectName() == "sb_dac_0"){
        ch = 0;
        dacv = ui->le_dac_ch1->text().toInt();
        voltage = ui->le_dac_ch1->text().toFloat();
    }
    else if(sb->objectName() == "sb_dac_1"){
        ch = 1;
        dacv = ui->le_dac_ch2->text().toInt();
        voltage = ui->le_dac_ch2->text().toFloat();
    }
    else if(sb->objectName() == "sb_dac_2"){
        ch = 2;
        dacv = ui->le_dac_ch3->text().toInt();
        voltage = ui->le_dac_ch3->text().toFloat();
    }
    else if(sb->objectName() == "sb_dac_3"){
        ch = 3;
        dacv = ui->le_dac_ch4->text().toInt();
        voltage = ui->le_dac_ch4->text().toFloat();
    }
    if(ui->cb_dac_use_raw->isChecked()){
        option = 0;
        dacv = sb->value();
        ds << ch;
        ds << option;
        ds << dacv;
    }
    else{
        option = 7;
        voltage = (float)(sb->value()/1000.);
        ds << ch;
        ds << option;
        ds << voltage;
    }
    f.setPayload(b);
    f.setFrameType(QCanBusFrame::DataFrame);
    f.setExtendedFrameFormat(true);

    m_canInterface->writeFrame(f);
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
    if(btn->objectName() == "pb_iso_out_0"){
        ch = 0x80;
        option = btn->isChecked()?1:0;
    }
    else if(btn->objectName() == "pb_iso_out_1"){
        ch = 0x81;
        option = btn->isChecked()?1:0;
    }
    else if(btn->objectName() == "pb_iso_out_2"){
        ch = 0x82;
        option = btn->isChecked()?1:0;
    }
    else if(btn->objectName() == "pb_iso_out_3"){
        ch = 0x83;
        option = btn->isChecked()?1:0;
    }
    else if(btn->objectName() == "pb_ttl_set_lb"){
        ch = 0x40;
        option = ui->le_ttl_lb_weite->text().toInt();
    }
    else if(btn->objectName() == "pb_ttl_set_hb"){
        ch = 0x41;
        option = ui->le_ttl_hb_write->text().toInt();
    }
    else if(btn->objectName() == "pb_sv1_on"){
        ch = 0x84;
        option = btn->isChecked()?1:0;
    }
    else if(btn->objectName() == "pb_sv1_en"){
        ch = 0x85;
        option = btn->isChecked()?1:0;
    }
    else if(btn->objectName() == "pb_sv2_on"){
        ch = 0x86;
        option = btn->isChecked()?1:0;
    }
    else if(btn->objectName() == "pb_sv2_en"){
        ch = 0x87;
        option = btn->isChecked()?1:0;
    }

    ds << ch;
    ds << option;
    f.setFrameId(0x140);
    f.setPayload(b);
    f.setFrameType(QCanBusFrame::DataFrame);
    f.setExtendedFrameFormat(true);

    m_canInterface->writeFrame(f);
}

void MainWindow::on_pbClear_clicked()
{
    ui->te_serial_data->setPlainText("");
}

void MainWindow::OnTTLIOChanged(bool checked)
{
    QPushButton *btn = static_cast<QPushButton*>(sender());
    QCanBusFrame f;
    QByteArray b;
    QDataStream ds(&b, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
    quint8 ch,option;
    if(btn == ui->pb_ttl_lb_dir){
        ch = 0x0;
    }
    else if(btn == ui->pb_ttl_hb_dir){
        ch = 0x1;
    }
    if(btn->isChecked()){ // output
        option = 0x10;// push pull
        btn->setText("Output");
    }
    else{
        option = 0x01; // input pullup
        btn->setText("Input");
    }
    ds << ch;
    ds << option;
    f.setFrameId(0x142);
    f.setPayload(b);
    f.setFrameType(QCanBusFrame::DataFrame);
    f.setExtendedFrameFormat(true);

    m_canInterface->writeFrame(f);
}

void MainWindow::OnPIDControl()
{
    QPushButton *btn = static_cast<QPushButton*>(sender());
    QCanBusFrame f;
    QByteArray b;
    QDataStream ds(&b, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
    quint8 state = 0;
    quint8 cmd = 0;


    if(btn->objectName() == "pbPIDStart"){
        state = 0x2;
    }
    else if(btn->objectName() == "pbPIDStop"){
        state = 0x1;
    }
    else if(btn->objectName() == "pbPIDClearFault"){
        state = 0x04;
    }
    else if(btn->objectName() == "pbPIDSP"){

    }

    m_seriesData.target = ui->lePIDSP->text().toDouble();

    if(ui->cbPIDMode->currentIndex() == 0){ // speed
        qint16 cmdValue = 0;
        state |= 0x00;
        cmdValue = ui->lePIDSP->text().toInt()*100;

        ds << state;
        ds << cmd;
        ds << cmdValue;

        QAbstractAxis *a = m_chart->axes(Qt::Vertical).back();
        a->setMin(-10);
        a->setMax(10);

    }
    else{
        quint16 cmdValue = 0;
        state |= (ui->cbPIDMode->currentIndex() << 4);
        cmdValue = ui->lePIDSP->text().toInt()*100;
        //if(cmdValue < 200) cmdValue = 20;
        //if(cmdValue > 35800) cmdValue = 35800;
        ds << state;
        ds << cmd;
        ds << cmdValue;
        QAbstractAxis *a = m_chart->axes(Qt::Vertical).back();
        a->setMin(0);
        a->setMax(360);
    }
    f.setFrameId(0x100);
    f.setPayload(b);
    f.setFrameType(QCanBusFrame::DataFrame);
    f.setExtendedFrameFormat(true);

    m_canInterface->writeFrame(f);

}

void MainWindow::OnParamRead()
{
    QList<PID_Data*> items = m_piddata->dataItem();
    quint8 cmd;
    foreach (PID_Data *d, items) {
        if(d->key != 0){
            //if(d->edited){
                QCanBusFrame f;
                QByteArray b;
                QDataStream ds(&b, QIODevice::WriteOnly);
                ds.setByteOrder(QDataStream::LittleEndian);
                ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
                //float fv = d->set_value.toFloat();
                cmd = d->key;
                ds << cmd;
                //ds << fv;
                f.setFrameId(0x101);
                f.setFrameType(QCanBusFrame::DataFrame);
                f.setExtendedFrameFormat(true);
                f.setPayload(b);
                m_canInterface->writeFrame(f);
                QThread::msleep(100);
            //}
        }
    }
}

void MainWindow::OnParamUpdate()
{
    QList<PID_Data*> items = m_piddata->dataItem();
    quint8 cmd;
    foreach (PID_Data *d, items) {
        if(d->key != 0 && d->edited){
            QCanBusFrame f;
            QByteArray b;
            QDataStream ds(&b, QIODevice::WriteOnly);
            ds.setByteOrder(QDataStream::LittleEndian);
            ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
            float fv = d->set_value.toFloat();
            cmd = d->key;
            ds << cmd;
            ds << fv;
            f.setFrameId(0x101);
            f.setFrameType(QCanBusFrame::DataFrame);
            f.setExtendedFrameFormat(true);
            f.setPayload(b);
            m_canInterface->writeFrame(f);
            QThread::msleep(200);
        }
    }

    // send update packet
    QCanBusFrame f;
    QByteArray b;
    QDataStream ds(&b, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::LittleEndian);
    ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
    float fv = 1.0;
    cmd = 0xc;
    ds << cmd;
    ds << fv;
    f.setFrameId(0x101);
    f.setFrameType(QCanBusFrame::DataFrame);
    f.setExtendedFrameFormat(true);
    f.setPayload(b);
    m_canInterface->writeFrame(f);
    QThread::msleep(100);
}

void MainWindow::OnTimeout()
{
    qint64 ct = QDateTime::currentDateTime().toMSecsSinceEpoch();

    tq1_pts.append(QPointF(ct,m_seriesData.torque1));
    tq2_pts.append(QPointF(ct,m_seriesData.torque2));
    sp_pts.append(QPointF(ct,m_seriesData.target));
    pv_pts.append(QPointF(ct,m_seriesData.currentPV));
    pi_pts.append(QPointF(ct,m_seriesData.pi));
    rpm_pts.append(QPointF(ct,m_seriesData.rpm));

    if(tq1_pts.size() > 120){
        tq1_pts.removeFirst();
        tq2_pts.removeFirst();
        sp_pts.removeFirst();
        pv_pts.removeFirst();
        pi_pts.removeFirst();
        rpm_pts.removeFirst();
    }

    QChart *c = ui->graphicsView->chart();

    for(int i=0;i<m_chart->series().count();i++){
        QLineSeries *s = static_cast<QLineSeries*>(m_chart->series().at(i));
        //c->removeSeries(s);
        //int x = s->points().count()>0?s->points().last().x()+1:0;
        //qDebug()<<Q_FUNC_INFO<<" N="<<n;

        if(s->name() == "TQ 1-1"){
//            s->points().replace(0,tq1_pts);
            //s->points().clear();
            s->clear();
            s->append(tq1_pts);
            //s->append(x,m_seriesData.torque1);
        }
        else if(s->name() == "TQ 1-2"){
            //s->append(x,m_seriesData.torque2);
            s->clear();
            s->append(tq2_pts);
        }
        else if(s->name() == "PV"){
            //s->append(x,m_seriesData.currentPV);
            s->clear();
            s->append(pv_pts);
        }
        else if(s->name() == "SP"){
            //s->append(x,m_seriesData.target);
            s->clear();
            s->append(sp_pts);
        }
        else if(s->name() == "PI"){
            //s->points().clear();
            s->clear();
            s->append(pi_pts);
        }
        else if(s->name() == "RPM"){
            s->clear();
            s->append(rpm_pts);
        }
        //c->addSeries(s);
//        int n = s->points().count();
//        if(n > 100){
//            s->points().removeFirst();
//            QAbstractAxis *a = m_chart->axes(Qt::Horizontal).back();
//            a->setMin(x-100);
//            a->setMax(x);

//        }


    }


    //QChart *c = ui->graphicsView->chart();
    QList<QAbstractAxis*> x =c->axes(Qt::Horizontal);
    foreach (QAbstractAxis *a, x) {
        if(a->titleText() == "Time(s)"){
            QDateTimeAxis *xa = (QDateTimeAxis*)a;
            if(xa->max().toMSecsSinceEpoch() < ct){
                qreal dw = c->plotArea().width()*0.2;
                c->scroll(dw,0);
            }
        }
    }

}
