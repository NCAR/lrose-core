#include <QtWidgets>
//#include <QPixmap>
#include <iostream>
#include <QMenu>                                                                                   
//#include <QLayout>

//#include "FlowLayout.hh"
#include "ContextEditingView.hh"
//#include "ParameterColorContextEditingView.hh"
//#include "ContextEditingViewOptionsWidget.hh"
//#include "../IpsEye/ColorMap.hh"
//#include "../IpsEye/ColorBar.hh"

#include "SpreadSheetView.hh"
#include "SpreadSheetController.hh"

#define MESSAGE \
  ContextEditingView::tr("<p>Message boxes have a caption, a text, " \
               "and any number of buttons, each with standard or custom texts." \
               "<p>Click a button to close the message box. Pressing the Esc button " \
	     "will activate the detected escape button (if any).")
#define MESSAGE_DETAILS \
  ContextEditingView::tr("If a message box has detailed text, the user can reveal it " \
	     "by pressing the Show Details... button.")


ContextEditingView::ContextEditingView(PolarWidget *parent) : QWidget(parent) {
  errorMessageDialog = new QErrorMessage(parent);
  informationLabel = new QLabel();
  _parent = parent;
  //_data = _vol;
}

ContextEditingView::~ContextEditingView() {
  delete errorMessageDialog;
  delete informationLabel;
}

void ContextEditingView::ShowContextMenu(const QPoint &pos)
{
  QMenu contextMenu("Context menu", this);
  
  QAction action1("Cancel", this);
  connect(&action1, SIGNAL(triggered()), this, SLOT(contextMenuCancel()));
  contextMenu.addAction(&action1);
  /*
  QAction action2("Sweepfiles", this);
  connect(&action2, SIGNAL(triggered()), this, SLOT(contextMenuSweepfiles()));
  contextMenu.addAction(&action2);
  */

  QAction action3("Parameters + Colors", this);
  connect(&action3, SIGNAL(triggered()), this, SLOT(contextMenuParameterColors()));
  contextMenu.addAction(&action3);

  QAction action4("View", this);
  connect(&action4, SIGNAL(triggered()), this, SLOT(contextMenuView()));
  contextMenu.addAction(&action4);

  QAction action5("Editor", this);
  connect(&action5, SIGNAL(triggered()), this, SLOT(contextMenuEditor()));
  contextMenu.addAction(&action5);

  QAction action6("Examine", this);
  connect(&action6, SIGNAL(triggered()), this, SLOT(contextMenuExamine(pos)));
  contextMenu.addAction(&action6);

  QAction action7("Data Widget", this);
  connect(&action7, SIGNAL(triggered()), this, SLOT(contextMenuDataWidget()));
  contextMenu.addAction(&action7);
  /*                                                                                                                         
   QAction action8("Metadata", window);                                                                                        
   connect(&action8, SIGNAL(triggered()), window, SLOT(contextMenuMetadata()));                                                
   contextMenu.addAction(&action8);                                                                                          
  */

  contextMenu.exec(this->mapToGlobal(pos));
}


// slots for context editing; create and show the associated modeless dialog and return

void ContextEditingView::contextMenuCancel()
{
  informationMessage();

  //notImplemented();
}

void ContextEditingView::contextMenuParameterColors()
{
  //  setColor();
  informationMessage();

  //  notImplemented();

}

void ContextEditingView::contextMenuView()
{
  informationMessage();
  //  notImplemented();
}

void ContextEditingView::contextMenuEditor()
{
  informationMessage();
  //  notImplemented();
}

void ContextEditingView::contextMenuExamine() // RadxRay *closestRay)
{
  //informationMessage();

  //   notImplemented();                                                                                     
  ExamineEdit(); // closestRay);
}

void ContextEditingView::contextMenuDataWidget()
{
  informationMessage();

  //  notImplemented();
}

void ContextEditingView::setFont()
{
  /*
  const QFontContextEditingView::FontContextEditingViewOptions options = QFlag(fontContextEditingViewOptionsWidget->value());
  bool ok;
  QFont font = QFontContextEditingView::getFont(&ok, QFont(fontLabel->text()), window, "Select Font", options);
  if (ok) {
    fontLabel->setText(font.key());
    fontLabel->setFont(font);
  }
  */
}


void ContextEditingView::ExamineEdit(QPoint pos) {

  // get the data associated with the request
  // copy it and 
  // give to controller ...
  //const RadxRay *closestRay = _parent->_getClosestRay(pos.x(), pos.y());
  //RadxRay closestRayCopy = *closestRay;

  SpreadSheetView *sheetView;

  sheetView = new SpreadSheetView();
  SpreadSheetController sheetControl(sheetView, closestRayCopy);
  sheetView->show();
  sheetView->layout()->setSizeConstraint(QLayout::SetFixedSize);

}

void ContextEditingView::ExamineEdit() {
  SpreadSheetView *sheetView;

  sheetView = new SpreadSheetView();
  SpreadSheetController sheetControl(sheetView);
  sheetView->show();
  sheetView->layout()->setSizeConstraint(QLayout::SetFixedSize);

}

void ContextEditingView::notImplemented()
{
  cerr << "inside notImplemented() ... " << endl;

  errorMessageDialog->showMessage("This option is not implemented yet.");
  QLabel errorLabel;
  int frameStyle = QFrame::Sunken | QFrame::Panel;
  errorLabel.setFrameStyle(frameStyle);
  errorLabel.setText("If the box is unchecked, the message "
			 "won't appear again.");

  cerr << "exiting notImplemented() " << endl;

}


void ContextEditingView::criticalMessage()
{
  QMessageBox::StandardButton reply;
  reply = QMessageBox::critical(this, "QMessageBox::critical()",
				MESSAGE,
				QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
  if (reply == QMessageBox::Abort)
    criticalLabel->setText("Abort");
  else if (reply == QMessageBox::Retry)
    criticalLabel->setText("Retry");
  else
    criticalLabel->setText("Ignore");
}

void ContextEditingView::informationMessage()
{
  QMessageBox::StandardButton reply;
  reply = QMessageBox::information(this, "QMessageBox::information()", "Not implemented");
  if (reply == QMessageBox::Ok)
    informationLabel->setText("OK");
  else
    informationLabel->setText("Escape");
}

void ContextEditingView::questionMessage()
{
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, "QMessageBox::question()",
				MESSAGE,
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
  if (reply == QMessageBox::Yes)
    questionLabel->setText("Yes");
  else if (reply == QMessageBox::No)
    questionLabel->setText("No");
  else
    questionLabel->setText("Cancel");
}

void ContextEditingView::warningMessage()
{
  QMessageBox msgBox(QMessageBox::Warning, "QMessageBox::warning()",
		     MESSAGE, 0, this);
  msgBox.setDetailedText(MESSAGE_DETAILS);
  msgBox.addButton("Save &Again", QMessageBox::AcceptRole);
  msgBox.addButton("&Continue", QMessageBox::RejectRole);
  if (msgBox.exec() == QMessageBox::AcceptRole)
    warningLabel->setText("Save Again");
  else
    warningLabel->setText("Continue");

}

void ContextEditingView::errorMessage()
{
  errorMessageDialog->showMessage(
				  "This dialog shows and remembers error messages. "
               "If the checkbox is checked (as it is by default), "
               "the shown message will be shown again, "
               "but if the user unchecks the box the message "
               "will not appear again if QErrorMessage::showMessage() "
				     "is called with the same message.");
  errorLabel->setText("If the box is unchecked, the message "
			 "won't appear again.");
}
