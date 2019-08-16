#ifndef VIEWVSECTION_H
#define VIEWVSECTION_H

#include <QDialog>
#include <QPushButton>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QtCharts/QtCharts>


class viewVsection : public QDialog
{
    Q_OBJECT
public:
    explicit viewVsection(QWidget *parent = nullptr);
    ~viewVsection();

signals:

public slots:

private:

    QPushButton *clear, *close;
    QGroupBox *group;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
    QLabel *labelTop, *labelBase;
    QLineEdit *editTop, *editBase;
    QChart *chart;
    QLineSeries *series, *series1, *series2, *series3;
    QChartView *chartView;

};

#endif // VIEWVSECTION_H
