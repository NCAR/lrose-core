#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAbstractItemModel;
class QComboBox;
class QCompleter;
class QLabel;
class QLineEdit;
class QProgressBar;

class TextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);

private slots:
    void about();

private:
    void createMenu();
    QAbstractItemModel *modelFromFile(const QString& fileName);

    QCompleter *completer;
    TextEdit *completingTextEdit;
};

#endif // MAINWINDOW_H

