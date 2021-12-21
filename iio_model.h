#ifndef IIO_MODEL_H
#define IIO_MODEL_H

#include <QObject>
#include <QAbstractTableModel>

class IIO_Data:public QObject{
    Q_OBJECT
public:
    IIO_Data(QObject *parent = nullptr){
        m_RawLow = -131072;
        m_RawHigh = 131072;
        m_EngLow = -10;
        m_EngHigh = 10;
        m_RawPV = 0;
        m_EngPV = 0;
    }
    int RawPV(){return m_RawPV;};
    double EngPV(){return m_EngPV;};
    void setRaw(int value){
        m_RawPV = value;
        if(value <= m_RawLow){
            m_EngPV = m_EngLow;
        }
        else if(value >= m_RawHigh){
            m_EngPV = m_EngHigh;
        }
        else{
            double pv;
            pv = (value - m_RawLow)*(m_EngHigh - m_EngLow)/(m_RawHigh - m_RawLow);
            m_EngPV = pv + m_EngLow;
        }
    }
    void setEng(double value){
        m_EngPV = value;
        if(value <= m_EngLow){
            m_RawPV = m_RawLow;
        }
        else if(value >= m_EngHigh){
            m_RawPV = m_RawHigh;
        }
        else{
            int raw;
            raw = (value - m_EngLow)*(m_RawHigh - m_RawLow)/(m_EngHigh - m_EngLow);
            m_RawPV = raw + m_RawLow;
        }
    }

private:
    int m_RawPV;
    int m_RawHigh, m_RawLow;
    double m_EngHigh,m_EngLow;
    double m_EngPV;
};

class IIO_Model : public QAbstractTableModel
{
    Q_OBJECT
public:
    IIO_Model(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,Qt::Orientation orientation, int role) const override;
//    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
//    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
//    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    void addBlankRecord();
    void setRawData(int channel, int raw);
    void setEngData(int channel, double engv);

signals:
    void editCompleted(const QString&);
private:
    QList<IIO_Data*> m_data;
};

#endif // IIO_MODEL_H
