#ifndef VIEWVALUESDISPLAY_H
#define VIEWVALUESDISPLAY_H

#include <QDialog>
#include <QLabel>
#include <QTextBrowser>
#include <QHBoxLayout>
#include <QVBoxLayout>

class viewValuesDisplay : public QDialog
{
    Q_OBJECT
public:
    explicit viewValuesDisplay(QWidget *parent = nullptr);
    ~viewValuesDisplay();
    QTextBrowser *valueOf1, *valueOf2, *valueOf3, *valueOf4;

signals:

public slots:

private:
    QLabel *valueLabel1, *valueLabel2, *valueLabel3, *valueLabel4;
    QHBoxLayout *valueCombo1, *valueCombo2, *valueCombo3, *valueCombo4;
    QVBoxLayout *valuesLayout;
};

#endif // VIEWVALUESDISPLAY_H
