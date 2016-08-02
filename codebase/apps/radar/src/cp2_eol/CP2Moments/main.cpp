#include <qapplication.h>
#include "CP2Moments.h"
#include "CP2MomentsIcon.h"
#include <stdlib.h>


int main( int argc, char** argv )
{
    QApplication app( argc, argv );
	app.setWindowIcon(QPixmap(CP2MomentsIcon));

	QDialog* dialog = new QDialog(0, Qt::WindowMinimizeButtonHint);

	// create our main window. It may contain a PPI sometime, and 
	// other buttons etc.
	CP2Moments cp2moments(dialog);

	// if we don't show() the  dialog, nothing appears!
	dialog->show();
	
	return app.exec();
}

