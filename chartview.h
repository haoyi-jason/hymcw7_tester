#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QObject>
#include <QAbstractTableModel>
#include <QChartView>
#include <QtCharts>

using namespace  QtCharts;

class ChartView : public QChartView
{
public:
    ChartView(QWidget *parent = nullptr);
    ChartView(QChart *chart, QWidget *parent = nullptr);
protected:
    bool viewportEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    bool m_isTouching;
    QPointF m_lastMousePos;
};



class SeriesModel:public QAbstractTableModel
{
    Q_OBJECT
public:
    SeriesModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool insertRows(int row, int count, const QModelIndex &parent);
    //bool insertColumns(int column, int count, const QModelIndex &parent);
    bool removeRows(int row, int count, const QModelIndex &parent);
    QLineSeries *getItemByName(QString name);
    QLineSeries *getItemByRow(int row);
    void clearData();

private:
    QList<QLineSeries*> m_lines;

};

#endif // CHARTVIEW_H
