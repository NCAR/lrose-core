

#include "SpreadSheetDelegate.hh"
#include "TextEdit.hh"

#include <QtWidgets>

SpreadSheetDelegate::SpreadSheetDelegate(QObject *parent)
        : QItemDelegate(parent) {}


/*
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), completer(0)
{
    createMenu();

    completingTextEdit = new TextEdit;
    completer = new QCompleter(this);
    completer->setModel(modelFromFile(":/resources/wordlist.txt"));
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWripsound(false);
    completingTextEdit->setCompleter(completer);

    setCentralWidget(completingTextEdit);
    resize(500, 300);
    setWindowTitle(tr("Completer"));
}

void MainWindow::createMenu()
{
    QAction *exitAction = new QAction(tr("Exit"), this);
    QAction *aboutAct = new QAction(tr("About"), this);
    QAction *aboutQtAct = new QAction(tr("About Qt"), this);

    connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    QMenu* fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(exitAction);

    QMenu* helpMenu = menuBar()->addMenu(tr("About"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

QAbstractItemModel *MainWindow::modelFromFile(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return new QStringListModel(completer);

#ifndef QT_NO_CURSOR
QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif
    QStringList words;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty())
	words << line.trimmed();
    }

#ifndef QT_NO_CURSOR
QApplication::restoreOverrideCursor();
#endif
    return new QStringListModel(words, completer);
}

*/

QWidget *SpreadSheetDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &,
                                          const QModelIndex &index) const
{

  //    if (index.column() == 1) {
  //      QDateTimeEdit *editor = new QDateTimeEdit(parent);
  //      editor->setDisplayFormat("dd/M/yyyy");
  //      editor->setCalendarPopup(true);
  //      return editor;
  //  }

    // TODO: here create a TextEditor with custom auto complete ...
    TextEdit *editor = new TextEdit(parent);
    
    /*
    QLineEdit *editor = new QLineEdit(parent);

    // create a completer with the strings in the column as model
    QStringList allStrings;

    allStrings.append(QString("AC_VEL()"));
    allStrings.append(QString("ABS()"));
    allStrings.append(QString("HIST()"));
    allStrings.append(QString("STDEV()"));
   

    QCompleter *autoComplete = new QCompleter(allStrings);
    editor->setCompleter(autoComplete);
    */

    
    //connect(editor, &QLineEdit::editingFinished, this, &SpreadSheetDelegate::commitAndCloseEditor);
    //connect(editor, &FunctionEditor::editingFinished, this, &SpreadSheetDelegate::commitAndCloseEditor);
    return editor;
}

/*
bool SpreadSheetDelegate::runFunctionDialog()
{

  QDialog functionDialog(this);
  //functionDialog.setWindowTitle(title);                                                                        

  TextEdit textEditArea();

  QPushButton cancelButton(tr("Cancel"), &functionDialog);
  connect(&cancelButton, &QAbstractButton::clicked, &functionDialog, &QDialog::reject);

  QPushButton okButton(tr("OK"), &functionDialog);
  okButton.setDefault(true);
  connect(&okButton, &QAbstractButton::clicked, &functionDialog, &QDialog::accept);

  QHBoxLayout *buttonsLayout = new QHBoxLayout;
  buttonsLayout->addStretch(1);
  buttonsLayout->addWidget(&okButton);
  buttonsLayout->addSpacing(10);
  buttonsLayout->addWidget(&cancelButton);

  QHBoxLayout *dialogLayout = new QHBoxLayout(&functionDialog);
  dialogLayout->addWidget(&textEditArea);
  dialogLayout->addStretch(1);
  dialogLayout->addItem(buttonsLayout);

  if (addDialog.exec()) {
    QString formula = textEditArea.getText();
    return true;
  }

  return false;
}
*/

void SpreadSheetDelegate::commitAndCloseEditor()
{
    QLineEdit *editor = qobject_cast<QLineEdit *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

void SpreadSheetDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const
{
    QLineEdit *edit = qobject_cast<QLineEdit*>(editor);
    if (edit) {
        edit->setText(index.model()->data(index, Qt::EditRole).toString());
        return;
    }

    QDateTimeEdit *dateEditor = qobject_cast<QDateTimeEdit *>(editor);
    if (dateEditor) {
        dateEditor->setDate(QDate::fromString(
                                index.model()->data(index, Qt::EditRole).toString(),
                                "d/M/yyyy"));
    }
}

void SpreadSheetDelegate::setModelData(QWidget *editor,
    QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *edit = qobject_cast<QLineEdit *>(editor);
    if (edit) {
        model->setData(index, edit->text());
        return;
    }

    QDateTimeEdit *dateEditor = qobject_cast<QDateTimeEdit *>(editor);
    if (dateEditor)
        model->setData(index, dateEditor->date().toString("dd/M/yyyy"));
}
