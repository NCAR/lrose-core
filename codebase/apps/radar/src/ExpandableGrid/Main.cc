#include <QApplication>

#include "Dialog.hh"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Dialog dialog;
    dialog.show();
    return app.exec();
}
