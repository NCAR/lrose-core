#include "CP2Scope.h"
#include "CP2ScopeIcon.h"

#include <qapplication.h>
#include <QDialog>
#include <qstring.h>
#include <iostream>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qcheckbox.h>

int
main(int argc, char** argv)
{

	QApplication app(argc, argv);
	app.setWindowIcon(QPixmap(cp2ScopeIcon));

	QDialog* dialog = new QDialog(0, Qt::WindowMinMaxButtonsHint);

	// create our test dialog. It will contain a ScopePlot and 
	// other buttons etc.
	CP2Scope scope(dialog);

	// connect the grid select and pause buttons
	QObject::connect(scope.xGrid, SIGNAL(toggled(bool)),
        scope._scopePlot, SLOT(enableXgrid(bool)));
	QObject::connect(scope.yGrid, SIGNAL(toggled(bool)),
        scope._scopePlot, SLOT(enableYgrid(bool)));
	QObject::connect(scope.pauseButton, SIGNAL(toggled(bool)),
        scope._scopePlot, SLOT(pause(bool)));

	// if we don't show() the test dialog, nothing appears!
	dialog->show();

 	// run the whole thing
	app.exec();

	return 0;
}
