QT += widgets
requires(qtConfig(combobox))

CONFIG += qt debug

HEADERS     = Dialog.hh
HEADERS    += FieldDisplay.hh
HEADERS    += HiddenContextMenu.hh
HEADERS    += YourButton.hh

SOURCES     = Dialog.cc
SOURCES    += FieldDisplay.cc
SOURCES    += HiddenContextMenu.cc
SOURCES    += YourButton.cc
SOURCES    += Main.cc

# install
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/layouts/dynamiclayouts
INSTALLS += target
