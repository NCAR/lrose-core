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

    QPushButton *clear, *close;
    QLineEdit *editTop, *editBase;
    QChart *chart;

signals:

public slots:

private:
    QGroupBox *group;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;
    QLabel *labelTop, *labelBase;
    QChartView *chartView;
};

#endif // VIEWVSECTION_H
