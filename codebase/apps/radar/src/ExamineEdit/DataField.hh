#ifndef DATAFIELD_H
#define DATAFIELD_H

#include <stdio.h>
#include <QtWidgets>
#include <QModelIndex>
#include <QJSEngine>
#include <QJSValue>

#include <vector>
#include <iostream>

//#include "SpreadSheetController.hh"

using namespace std;

/*
static QJSValue REMOVE_AIRCRAFT_MOTION(QJSContext *context, QJSEngine *engine)
{
  QJSValue callee = context->callee();
  if (context->argumentCount() == 1) { // writing?                                                                             
    callee.setProperty("value", context->argument(0));
  }
  return callee.property("value");
}

static QJSValue VectorOp(QJSContext *context, QJSEngine *engine)
{
  QJSValue callee = context->callee();
  if (context->argumentCount() == 1) { // writing?                                                                             
    QJSValue arg = context->argument(0);
    if (arg.isArray()) {
      cerr << "it is an Array" << endl;
      QVector<int> v = qscriptvalue_cast<QVector<int> >(context->argument(0)); 
      cerr << "inside VectorOp ";
      cerr << " VectorOp size = " << v.size() << endl;
      for (QVector<int>::iterator i=v.begin(); i != v.end(); i++)                                                                                     
	cerr << *i << endl; // outputs "[1, 2, 3, 5]"                                                                            

      qSort(v.begin(), v.end());                                                                                                    
      QJSValue jsArray = engine->newArray(v.size()); // toScriptValue(v);
      for (int i = 0; i < v.size(); ++i) {
	jsArray.setProperty(i, v.at(i));
      } 
      callee.setProperty("value", jsArray); // context->argument(0));
  }
}
  return callee.property("value");
}
*/



class DataField : public QObject
{

  Q_OBJECT
public:
  DataField(QObject *parent = nullptr) : QObject(parent) {}
  DataField(QVector<double> someData, QObject *parent = nullptr) : QObject(parent) {_values = someData; }

  //  Q_PROPERTY(QVector<int> values MEMBER _values)
  Q_PROPERTY(QVector<double> values MEMBER _values)

 //add(QVector<int> v, QVector<int> v2) { QVector<int> v3(3); for (int i=0; i<3; i++) v3[i]=v[i]+v2[i]; return v3; }

private:
  QVector<double> _values;
  //  persisted to underlying model (RadxVol)
};


#endif
