#ifndef VIEWMISCCONFIGDIALOG_H
#define VIEWMISCCONFIGDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QLine>
#include <QHBoxLayout>

class viewMiscConfigDialog : public QDialog
{
    Q_OBJECT
public:
    explicit viewMiscConfigDialog(QWidget *parent = nullptr);
    ~viewMiscConfigDialog();

    QLabel *symprodLabel, *constraintsLabel, *urlLabel, *dataTypeLabel, *allowBeforeLabel, *allowAfterLabel, *textThresholdLabel, *topoUrlLabel, *landUseLabel;
    QComboBox *symprodSelector, *constraintsSelector;
    QLineEdit *urlInput, *allowBeforeInput, *allowAfterInput, *textThresholdInput, *topoUrlInput, *landUseInput;
    QCheckBox *dataTypeBox;
    QHBoxLayout *miscLayoutH1, *miscLayoutH2, *miscLayoutH3, *miscLayoutH4, *miscLayoutH5, *miscLayoutH6, *miscLayoutH7;
    QVBoxLayout *miscLayoutV1;
    QFrame *line;

signals:

public slots:
};

#endif // VIEWMISCCONFIGDIALOG_H
