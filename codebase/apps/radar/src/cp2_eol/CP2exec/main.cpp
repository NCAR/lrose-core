#include <qapplication.h>
#include <iostream>
#include <qdialog.h>

#include "CP2Exec.h"
#include "CP2ExecThread.h"
#include "CP2ExecIcon.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
int 
main(int argc, char* argv[], char* envp[])
{
	// create the Qt application
	QApplication app( argc, argv );
	app.setWindowIcon(QPixmap(CP2ExecIcon));

	QDialog* dialog = new QDialog(0, Qt::WindowMinimizeButtonHint);

	// create our main window. It wants to know about the piraq executin
	// thread so that it can query the thread for status information.
	CP2Exec cp2exec(dialog);

	// if we don't show() the  dialog, nothing appears!
	dialog->show();

	return app.exec();
}


