#ifndef SOLOFUNCTIONS_H
#define SOLOFUNCTIONS_H

#include <stdio.h>
#include <QtWidgets>
#include <QModelIndex>

#include <vector>
#include <iostream>

using namespace std;

class SoloFunctions : public QObject
{

  Q_OBJECT
public:
  SoloFunctions();

  Q_INVOKABLE QString cat(QString animal);
};

#endif
