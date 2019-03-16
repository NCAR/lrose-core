#ifndef CONTEXTEDITINGVIEW_H
#define CONTEXTEDITINGVIEW_H

#include <QMainWindow>
#include <QLabel>
#include <QErrorMessage>
#include <QMessageBox>
#include "PolarWidget.hh"

class ContextEditingView : QWidget
{
  Q_OBJECT

  public:

  ContextEditingView(QWidget *parent);
  ~ContextEditingView();
  void ShowContextMenu(const QPoint &pos);

  private slots:

  void contextMenuParameterColors();
  void contextMenuView();
  void contextMenuEditor();
  void contextMenuExamine();
  void contextMenuDataWidget();
  void contextMenuCancel();
  void setFont();

  private:

  //  PolarWidget  *window;
  void notImplemented();
  void criticalMessage();
  void informationMessage();
  void questionMessage();
  void warningMessage();
  void errorMessage();
  void ExamineEdit();
  
  QLabel *fontLabel;
  QLabel *directoryLabel;
  QLabel *openFileNameLabel;
  QLabel *openFileNamesLabel;
  QLabel *saveFileNameLabel;
  QLabel *criticalLabel;
  QLabel *informationLabel;
  QLabel *questionLabel;
  QLabel *warningLabel;
  QLabel *errorLabel;
  
  QErrorMessage *errorMessageDialog;
  
};

#endif
