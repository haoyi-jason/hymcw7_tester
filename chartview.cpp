#include "chartview.h"

ChartView::ChartView(QWidget *parent):
    QChartView(parent),m_isTouching(false)
{

}

ChartView::ChartView(QChart *chart, QWidget *parent):
    QChartView(chart,parent),m_isTouching(false)
{
    setRubberBand(QChartView::RectangleRubberBand);
}

bool ChartView::viewportEvent(QEvent *event)
{
    if(event->type() == QEvent::TouchBegin){
        m_isTouching = true;
        chart()->setAnimationOptions(QChart::NoAnimation);
    }
    return QChartView::viewportEvent(event);
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if(m_isTouching){
        return;
    }
    if(event->buttons() & Qt::MiddleButton){
        auto dPos = event->pos() - m_lastMousePos;
        chart()->scroll(-dPos.x(),dPos.y());
        m_lastMousePos = event->pos();
        event->accept();
    }
    QChartView::mouseMoveEvent(event);
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if(m_isTouching){
        return;
    }
    if(event->button() == Qt::MiddleButton){
        m_lastMousePos = event->pos();
        event->accept();
    }
    QChartView::mousePressEvent(event);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_isTouching)
        m_isTouching = false;

    chart()->setAnimationOptions(QChart::NoAnimation);

    QChartView::mouseReleaseEvent(event);
}

void ChartView::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()){
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    case Qt::Key_multiply:
        chart()->zoomReset();
        break;
    case Qt::Key_Left:
        chart()->scroll(-10,0);
        break;
    case Qt::Key_Right:
        chart()->scroll(10,0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0,10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0,-10);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}


/*  Series Data Model */

SeriesModel::SeriesModel(QObject *parent)
{

}

int SeriesModel::rowCount(const QModelIndex &parent) const
{
    return m_lines.size();
}

int SeriesModel::columnCount(const QModelIndex &parent) const
{
    return 5;
}

QVariant SeriesModel::data(const QModelIndex &index, int role) const
{

    return QVariant();
}

QVariant SeriesModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    return QVariant();
}

bool SeriesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

    return false;
}

Qt::ItemFlags SeriesModel::flags(const QModelIndex &index) const
{
    if(index.column() == 0){
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | QAbstractItemModel::flags(index);
    }
    else{
        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    }

}

bool SeriesModel::insertRows(int row, int count, const QModelIndex &parent)
{

    return true;
}

bool SeriesModel::removeRows(int row, int count, const QModelIndex &parent)
{

    return true;
}

QLineSeries *SeriesModel::getItemByName(QString name)
{
    foreach(QLineSeries *s,m_lines){
        if(s->name() == name){
            return s;
        }
    }
    return nullptr;
}

QLineSeries *SeriesModel::getItemByRow(int row)
{
    if(row < m_lines.size()){
        return m_lines.at(row);
    }
    return nullptr;
}

void SeriesModel::clearData()
{

}
