#ifndef PID_DATAMODEL_H
#define PID_DATAMODEL_H

#include <QObject>
#include <QAbstractTableModel>

class PID_Data:public QObject{
    Q_OBJECT
public:
    PID_Data(QObject *parent = nullptr){
        name = "Name";
        set_value =get_value= "0";
        edited = false;
    }

public:
    QString name;
    QString set_value;
    QString get_value;
    bool edited;
    quint8 key;
};

class PID_DataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PID_DataModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void updateData(QString name,QString value);
    void updateData(quint8 id,float value);
    void insertData(QString name, QString value, quint8 key);

    QList<PID_Data*> dataItem(){return m_data;}

signals:

private:
    QList<PID_Data*> m_data;

};



#endif // PID_DATAMODEL_H
