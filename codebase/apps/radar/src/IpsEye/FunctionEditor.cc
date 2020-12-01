
#include "FunctionEditor.hh"
#include "TextEdit.hh"

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QDialog>

FunctionEditor::FunctionEditor(QWidget *parent)
{

  //   QDialog functionDialog(this);
  //functionDialog.setWindowTitle(title);                                                                                    

  TextEdit textEditArea(parent);

  QPushButton cancelButton(tr("Cancel"), this);
  connect(&cancelButton, &QAbstractButton::clicked, this, &QDialog::reject);

  QPushButton okButton(tr("OK"), this);
  okButton.setDefault(true);
  connect(&okButton, &QAbstractButton::clicked, this, &QDialog::accept);

  QHBoxLayout *buttonsLayout = new QHBoxLayout;
  buttonsLayout->addStretch(1);
  buttonsLayout->addWidget(&okButton);
  buttonsLayout->addSpacing(10);
  buttonsLayout->addWidget(&cancelButton);

  QHBoxLayout *dialogLayout = new QHBoxLayout;
  dialogLayout->addWidget(&textEditArea);
  dialogLayout->addStretch(1);
  dialogLayout->addItem(buttonsLayout);

  setLayout(dialogLayout);

  /*
  if (addDialog.exec()) {
    QString formula = textEditArea.getText();
    return true;
  }

  return false;
  */
}

// signal 
void FunctionEditor::editingFinished() {

}
