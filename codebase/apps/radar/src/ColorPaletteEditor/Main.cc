#include <QApplication>
#include <QStyleHints>
#include <QDesktopWidget>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

#include "Dialog.hh"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QGuiApplication::setApplicationDisplayName(Dialog::tr("Standard Dialogs"));

#ifndef QT_NO_TRANSLATION
    QString translatorFileName = QLatin1String("qt_");
    translatorFileName += QLocale::system().name();
    QTranslator *translator = new QTranslator(&app);
    if (translator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(translator);
#endif

    Dialog dialog;
    if (!QGuiApplication::styleHints()->showIsFullScreen() && !QGuiApplication::styleHints()->showIsMaximized()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(&dialog);
        dialog.resize(availableGeometry.width() / 3, availableGeometry.height() * 2 / 3);
        dialog.move((availableGeometry.width() - dialog.width()) / 2,
                    (availableGeometry.height() - dialog.height()) / 2);
    }
    dialog.show();

    return app.exec();
}
