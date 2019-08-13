#ifndef V_SECTION_H
#define V_SECTION_H

#include <QMainWindow>
#include <QPushButton>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
// #include <QChartView>

#include <QToolBar>
#include <QString>
#include <QToolTip>
#include <QtWidgets>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QHorizontalStackedBarSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QCategoryAxis>

#include <QtCharts/QLineSeries>
#include <QPointF>
#include <QPalette>
#include <QCategoryAxis>
#include <QBrush>
#include <QtCharts/QValueAxis>


namespace Ui {
class V_section;
}

class V_section : public QMainWindow
{
    Q_OBJECT

public:
    explicit V_section(QWidget *parent = nullptr);
    ~V_section();


private:
    Ui::V_section *ui;
    QPushButton *clear, *close;
    QGroupBox *group;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
    QLabel *labelTop, *labelBase;
    QLineEdit *editTop, *editBase;



};

#endif // V_SECTION_H
