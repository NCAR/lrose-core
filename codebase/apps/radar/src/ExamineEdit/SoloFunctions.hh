#ifndef SOLOFUNCTIONS_H
#define SOLOFUNCTIONS_H

#include <stdio.h>
#include <QtWidgets>
#include <QModelIndex>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

#include <vector>
#include <iostream>

//#include "SpreadSheetController.hh"

using namespace std;


static QScriptValue REMOVE_AIRCRAFT_MOTION(QScriptContext *context, QScriptEngine *engine)
{
  QScriptValue callee = context->callee();
  if (context->argumentCount() == 1) { // writing?                                                                             
    callee.setProperty("value", context->argument(0));
  }
  return callee.property("value");
}

static QScriptValue VectorOp(QScriptContext *context, QScriptEngine *engine)
{
  QScriptValue callee = context->callee();
  if (context->argumentCount() == 1) { // writing?                                                                             
    QScriptValue arg = context->argument(0);
    if (arg.isArray()) {
      cerr << "it is an Array" << endl;
      QVector<int> v = qscriptvalue_cast<QVector<int> >(context->argument(0)); 
      cerr << "inside VectorOp ";
      cerr << " VectorOp size = " << v.size();
      for (QVector<int>::iterator i=v.begin(); i != v.end(); i++)                                                                                     
	cerr << *i << endl; // outputs "[1, 2, 3, 5]"                                                                            

      qSort(v.begin(), v.end());                                                                                                    
      QScriptValue a = engine->toScriptValue(v);
      callee.setProperty("value", a); // context->argument(0));
  }
}
  return callee.property("value");
}


/*
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
*/

#endif
