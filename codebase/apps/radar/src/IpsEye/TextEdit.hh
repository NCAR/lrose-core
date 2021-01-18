#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QTextEdit>
#include <QAbstractItemModel>

class QCompleter;

class TextEdit : public QTextEdit
{
    Q_OBJECT

public:
    TextEdit(QWidget *parent = 0);
    virtual ~TextEdit();

    void setCompleter(QCompleter *c);
    QCompleter *completer() const;
    QString getText();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;
  //void focusOutEvent(QFocusEvent *e) override;

private slots:
    void insertCompletion(const QString &completion);

private:
    QString textUnderCursor() const;
    QAbstractItemModel *modelFromFile(const QString& fileName);
private:
    QCompleter *c;
};

#endif // TEXTEDIT_H
