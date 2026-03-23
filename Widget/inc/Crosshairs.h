#ifndef CROSSHAIRS_H
#define CROSSHAIRS_H


#include <QtCharts/QChartGlobal>
#include <QtWidgets/QGraphicsItem>

#include <QtCharts/QChart>
#include <QtGui/QPainter>
#include <QtGui/QCursor>
#include <QtGui/QTextDocument>

// QT_CHARTS_BEGIN_NAMESPACE
// class QChart;
// QT_CHARTS_END_NAMESPACE

// QT_CHARTS_USE_NAMESPACE

class Crosshairs
{
public:
    Crosshairs();
    Crosshairs(QChart *chart);
    void updatePosition(QPointF position);

    bool get_Flag(){return _M_flag;}

private:
    QGraphicsLineItem *m_xLine, *m_yLine;
    QGraphicsTextItem *m_xText, *m_yText;
    QChart *m_chart;

    bool _M_flag;
};

#endif // CROSSHAIRS_H
