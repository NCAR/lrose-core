#include <qapplication.h>
#include "CP2PPI.h"
#include "CP2PPIicon.h"
#include <stdlib.h>


int main( int argc, char** argv )
{
    QApplication app( argc, argv );
	app.setWindowIcon(QPixmap(CP2PPIicon));

	QDialog* dialog = new QDialog;

	// create our main window. It may contain a PPI sometime, and 
	// other buttons etc.
	CP2PPI cp2ppi(dialog);

	dialog->show();
	
	return app.exec();
}

