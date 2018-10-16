
#ifndef DIALOGOPTIONSWIDGET_H
#define DIALOGOPTIONSWIDGET_H

#include <QtWidgets>
#include <QPixmap>

//#include "Dialog.hh"

class DialogOptionsWidget : public QGroupBox
{
public:
    explicit DialogOptionsWidget(QWidget *parent = 0);

    void addCheckBox(const QString &text, int value);
    void addSpacer();
    int value() const;

private:
    typedef QPair<QCheckBox *, int> CheckBoxEntry;
    QVBoxLayout *layout;
    QList<CheckBoxEntry> checkBoxEntries;
};

#endif
