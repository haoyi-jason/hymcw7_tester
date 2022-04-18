#include "pid_datamodel.h"

PID_DataModel::PID_DataModel(QObject *parent)
{

}

int PID_DataModel::rowCount(const QModelIndex &parent) const
{
    return m_data.size();
}

int PID_DataModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

QVariant PID_DataModel::data(const QModelIndex &index, int role) const
{
    if(m_data.size() == 0) return QVariant();
    int row = index.row();
    int column = index.column();

    switch(role){
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch(column){
        case 0: return m_data.at(row)->name;break;
        case 1: return m_data.at(row)->get_value;break;
        case 2: return m_data.at(row)->set_value;break;
        default:return QVariant();break;
        }
    case Qt::TextAlignmentRole:
        switch(column){
        case 0: return Qt::AlignLeft;break;
        default:return Qt::AlignRight;break;
        }
    default:break;
    }
    return QVariant();
}

QVariant PID_DataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal){
        switch(section){
        case 0: return QVariant("Name");break;
        case 1: return QVariant("GET");break;
        case 2: return QVariant("SET");break;
        }
    }
    return QVariant();
}

bool PID_DataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int row = index.row();
    int col = index.column();
    if(row < m_data.size() && col == 2){
        QString v = value.toString();
        if(v != m_data.at(row)->set_value){
            m_data.at(row)->set_value = v;
            m_data.at(row)->edited = true;
            return true;
        }
    }
    return false;
}

Qt::ItemFlags PID_DataModel::flags(const QModelIndex &index) const
{
    if(index.column() ==2){
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | QAbstractItemModel::flags(index);
    }
    else{
        return QAbstractItemModel::flags(index);
    }
}

void PID_DataModel::updateData(QString name, QString value)
{
    foreach (PID_Data  *d, m_data) {
        if(d->name == name){
            d->get_value = value;
            return;
        }
    }
}

void PID_DataModel::updateData(quint8 id, float value)
{
    foreach (PID_Data  *d, m_data) {
        if(d->key == id){
            d->get_value = QString::number(value);
            return;
        }
    }
}


void PID_DataModel::insertData(QString name, QString value, quint8 key)
{
    PID_Data *d = new PID_Data;
    d->name = name;
    d->get_value = d->set_value = value;
    d->key = key;
    m_data.append(d);
}
