#ifndef CONTFIELDDOCK_H
#define CONTFIELDDOCK_H

#include <QDockWidget>
#include <QListWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QGroupBox>

class contFieldDock
{
public:
    contFieldDock();
    ~contFieldDock();
    QDockWidget *fieldDock;
private:
    QListWidget *fieldList;
    QPushButton *field[3][10];
    QGridLayout *grid;
    QVBoxLayout *fieldLayout;
    QGroupBox *fieldGroup;
};

#endif // CONTFIELDDOCK_H
