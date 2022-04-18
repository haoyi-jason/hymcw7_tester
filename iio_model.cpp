#include "iio_model.h"

IIO_Model::IIO_Model(QObject *parent):
    QAbstractTableModel(parent)
{

}

int IIO_Model::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

int IIO_Model::columnCount(const QModelIndex &parent) const
{
    return 3;
}

QVariant IIO_Model::data(const QModelIndex &index, int role) const
{
    if(m_data.size()==0) return QVariant();
    int row = index.row();
    int column = index.column();

    switch(role){
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(column){
        case 0: return QVariant(row +1);break;
        case 1: return QVariant(m_data.at(row)->RawPV());break;
        case 2: return QVariant(QString::number(m_data.at(row)->EngPV(),'f',3));break;
        default:return QVariant();break;
        }
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignRight;
        break;
    default:break;
    }

    return QVariant();
}

QVariant IIO_Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal){
        switch(section){
        case 0:return QVariant("Channel");break;
        case 1:return QVariant("Raw Data");break;
        case 2:return QVariant("Voltage(V)");break;
        default: return QVariant();break;
        }
    }

    return QVariant();
}

Qt::ItemFlags IIO_Model::flags(const QModelIndex &index) const
{
    return QAbstractTableModel::flags((index));
}

void IIO_Model::addBlankRecord()
{
    m_data.append((new IIO_Data()));
}

void IIO_Model::setRawData(int channel, int raw)
{
    if(channel < m_data.size()){
        m_data[channel]->setRaw(raw);
    }
}

void IIO_Model::setEngData(int channel, double engv)
{
    if(channel < m_data.size()){
        m_data[channel]->setEng(engv);
    }
}
