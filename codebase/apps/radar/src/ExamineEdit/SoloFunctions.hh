#ifndef SOLOFUNCTIONS_H
#define SOLOFUNCTIONS_H

#include <stdio.h>
#include <QtWidgets>
#include <QModelIndex>

#include <vector>
#include <iostream>

#include "SpreadSheetController.hh"

using namespace std;

class SoloFunctions : public QObject
{

  Q_OBJECT
public:
  SoloFunctions(SpreadSheetController *controller);

  Q_INVOKABLE QString cat(QString animal);
  Q_INVOKABLE QString REMOVE_AIRCRAFT_MOTION(QString field);

private:

  SpreadSheetController *_controller;
};

#endif
