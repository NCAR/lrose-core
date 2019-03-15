#ifndef FUNTIONEDITOR_H
#define FUNTIONEDITOR_H

#include <QDialog>
#include <QWidget>

class FunctionEditor : public QDialog
{
    Q_OBJECT

public:
    FunctionEditor(QWidget *parent = 0);

    void editingFinished();

/*
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

private slots:
    void commitAndCloseEditor();
*/


};

#endif // FUNTIONEDITOR_H
