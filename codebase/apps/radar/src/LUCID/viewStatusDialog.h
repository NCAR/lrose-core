#ifndef VIEWSTATUSDIALOG_H
#define VIEWSTATUSDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

class viewStatusDialog : public QDialog
{
    Q_OBJECT
public:
    explicit viewStatusDialog(QWidget *parent = nullptr);
    ~viewStatusDialog();

signals:

public slots:

private:
    QListWidget *statuses;
    QPushButton *clear, *close;
    QGroupBox *group;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;

};

#endif // VIEWSTATUSDIALOG_H
