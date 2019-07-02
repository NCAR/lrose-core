#ifndef SOLOFUNCTIONS_H
#define SOLOFUNCTIONS_H

#include <stdio.h>
#include <QtWidgets>
#include <QModelIndex>
#include <QJSEngine>
#include <QJSValue>

#include <vector>
#include <iostream>

#include "Radx/RadxVol.hh"

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


class SoloFunctions : public QObject
{
  Q_OBJECT

public:
  //  SoloFunctions(SpreadSheetController *controller);
  SoloFunctions(RadxVol &data, QObject *parent = nullptr) : QObject(parent) {_data = data;}
  //SoloFunctions(QObject *parent = nullptr) : QObject(parent) { }

  Q_INVOKABLE QString cat(QString animal) {return animal+"_cat"; }
  Q_INVOKABLE QString REMOVE_AIRCRAFT_MOTION(QString field); // return the name of the new field that contains the result
  //  Q_INVOKABLE QString  REMOVE_AIRCRAFT_MOTION(QString field); //  { return field+"_trump"; }
  Q_INVOKABLE double sqrt(double value) { return qSqrt(value); }
  Q_INVOKABLE QVector<double> add(QVector<double> v, QVector<double> v2) {
    int size = v.size();
    if (size != v2.size()) {
      // TODO: throw exception                                                                           
      // return the size of the shorter?                                                                 
      QVector<double> v0(0);
      return v0;
    }
    //    QVector<double> v3 = v + v2; return v3; } // this appends one v2 elements to v 
                                                                                                       
    QVector<double> v3(size);
    for (int i=0; i<size; i++)
      v3[i]=v[i]+v2[i];
    return v3;
  }

  /* { 
    int size = v.size(); if (size == v2.size()) {
    //    QVector<double> v3 = v + v2; return v3; } // this appends one v2 elements to v
      QVector<double> v3(size); 
     for (int i=0; i<3; i++) v3[i]=v[i]+v2[i]; return v3; }
  */
  Q_INVOKABLE QVector<int> addI(QVector<int> v, QVector<int> v2) { QVector<int> v3(3); for (int i=0; i<3; i++) v3[i]=v[i]+v2[i]; return v3; }
 
private:

  RadxVol _data;

  template<typename Out>
  void split(const string &s, char delim, Out result);
  vector<string> split(const string &s, char delim);
  vector<double> splitDouble(const string &s, char delim);
 
};


#endif
