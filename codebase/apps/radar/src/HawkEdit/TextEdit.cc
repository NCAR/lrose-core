#include "TextEdit.hh"
#include <iostream>
#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QStringListModel>
#include <QFile>

TextEdit::TextEdit(QWidget *parent)
: QTextEdit(parent), c(0)
{
  //setPlainText(tr("f(x)="
  //                  " You can trigger autocompletion using ") +
  //                  QKeySequence("Ctrl+E").toString(QKeySequence::NativeText));

    // =====================
    // this should be in a factory, or init, as it need only be done once

    QString fileName(":/resources/wordlist.txt");

    QCompleter *completer = new QCompleter(this);

    QAbstractItemModel *abstractModel;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        abstractModel = new QStringListModel(completer);
    } else {
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
      abstractModel = new QStringListModel(words, completer);
    }


    completer->setModel(abstractModel); // modelFromFile(":/resources/wordlist.txt"));
    completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setWrapAround(false);
    setCompleter(completer);
     
    //=================
}


TextEdit::~TextEdit()
{
}

void TextEdit::setCompleter(QCompleter *completer)
{
    if (c)
        QObject::disconnect(c, 0, this, 0);

    c = completer;

    if (!c)
        return;

    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(c, SIGNAL(activated(QString)),
                     this, SLOT(insertCompletion(QString)));
}

QCompleter *TextEdit::completer() const
{
    return c;
}

void TextEdit::insertCompletion(const QString& completion)
{
    if (c->widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - c->completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

QString TextEdit::getText()
{
    QString theText = toPlainText();
    return theText;
}

QString TextEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void TextEdit::focusInEvent(QFocusEvent *e)
{
    if (c)
        c->setWidget(this);
    QTextEdit::focusInEvent(e);
}

/*
void TextEdit::focusOutEvent(QFocusEvent *e)
{
  //if (c)
  //      c->setWidget(this);
  std::cerr << "formula is " << getText().toStdString() << std::endl;
    QTextEdit::focusOutEvent(e);
}
*/

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    if (c && c->popup()->isVisible()) {


       // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Escape:
       case Qt::Key_Return:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if (!c || !isShortcut) // do not process the shortcut when we have a completer
        QTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!c || (ctrlOrShift && e->text().isEmpty()))
        return;

    // static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    static QString eow("~!@#$%^&*+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 2 // 3
                      || eow.contains(e->text().right(1)))) {
        c->popup()->hide();
        return;
    }

    if (completionPrefix != c->completionPrefix()) {
        c->setCompletionPrefix(completionPrefix);
        c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0)
                + c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr); // popup it up!
}


/* 
       if (e->key() == Qt::Key_Escape) {
            // signal done editing;  grab the text?
            QString theText = toPlainText();
            std::cerr << "content of TextEdit: " << theText.toStdString() << std::endl;
       }
*/
