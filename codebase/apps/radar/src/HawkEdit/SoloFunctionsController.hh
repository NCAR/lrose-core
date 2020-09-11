#ifndef SOLOFUNCTIONSCONTROLLER_H
#define SOLOFUNCTIONSCONTROLLER_H

#include <stdio.h>
#include <float.h>
#include <QtWidgets>
#include <QModelIndex>
#include <QJSEngine>
#include <QJSValue>

#include <vector>
#include <iostream>

#include <Radx/RadxVol.hh>
#include "SoloFunctionsModel.hh"

//#include "HawkEyeLogger.hh"

using namespace std;

class SoloFunctionsController : public QObject
{
  Q_OBJECT

public:
  //  SoloFunctions(SpreadSheetController *controller);
  SoloFunctionsController(RadxVol *data, QObject *parent = nullptr);
  //SoloFunctions(QObject *parent = nullptr) : QObject(parent) { }



  //Q_PROPERTY(QVector<float> qilist MEMBER m_qilist) 

  //  Q_INVOKABLE QString ZERO(QString field); // return the name of the new field that contains the result
  //  Q_INVOKABLE QString REMOVE_AIRCRAFT_MOTION(QString field); // return the name of the new field that contains the result

  Q_INVOKABLE QString ZERO_MIDDLE_THIRD(QString field); // return the name of the new field that contains the result
  Q_INVOKABLE QString ZERO_INSIDE_BOUNDARY(QString field); // return the name of the new field that contains the result

  Q_INVOKABLE QString DESPECKLE(QString field, size_t speckle_length, float bad_data, size_t clip_gate); // return the name of the new field that contains the result
  Q_INVOKABLE QString REMOVE_AIRCRAFT_MOTION(QString field, float nyquist, float bad_data, size_t clip_gate); // return the name of the new field that contains the result

 // return the name of the new field that contains the result
  Q_INVOKABLE QString BB_UNFOLDING_FIRST_GOOD_GATE(QString field, float nyquist,
						   int max_pos_folds,
						   int max_neg_folds,
						   size_t ngates_averaged,
						   float bad_data, size_t clip_gate);

  Q_INVOKABLE QString ASSERT_BAD_FLAGS(QString field, float bad_data,
				       size_t clip_gate, QString badFlagMaskFieldName);

  Q_INVOKABLE QString CLEAR_BAD_FLAGS(QString field);

  Q_INVOKABLE QString COMPLEMENT_BAD_FLAGS(QString field);

  // return the name of the new field that contains the bad flag mask
  Q_INVOKABLE QString SET_BAD_FLAGS_ABOVE(QString field, float constant, float bad_data,
					  size_t clip_gate);

  Q_INVOKABLE QString SET_BAD_FLAGS_BELOW(QString field, float constant, float bad_data,
					  size_t clip_gate);

  Q_INVOKABLE QString SET_BAD_FLAGS_BETWEEN(QString field, float lower_threshold,
					    float upper_threshold, float bad_data,
					    size_t clip_gate);

  Q_INVOKABLE QString FLAGGED_ADD(QString field, float constant, float bad_data,
                                             size_t clip_gate, QString bad_flag_field);
  Q_INVOKABLE QString FLAGGED_MULTIPLY(QString field, float constant, float bad_data,
                                             size_t clip_gate, QString bad_flag_field);
  Q_INVOKABLE QString AND_BAD_FLAGS_ABOVE(QString field, float constant, 
					    QString mask_field, float bad_data = FLT_MIN,
					    size_t clip_gate = SIZE_MAX);
  Q_INVOKABLE QString AND_BAD_FLAGS_BELOW(QString field, float constant, 
					    QString mask_field, float bad_data = FLT_MIN,
					    size_t clip_gate = SIZE_MAX);
  Q_INVOKABLE QString AND_BAD_FLAGS_BETWEEN(QString field, float constantLower,
                                            float constantUpper,
					    QString mask_field, float bad_data = FLT_MIN,
					    size_t clip_gate = SIZE_MAX);
  Q_INVOKABLE QString OR_BAD_FLAGS_ABOVE(QString field, float constant, 
					    QString mask_field, float bad_data = FLT_MIN,
					    size_t clip_gate = SIZE_MAX);
  Q_INVOKABLE QString OR_BAD_FLAGS_BELOW(QString field, float constant, 
					    QString mask_field, float bad_data = FLT_MIN,
					    size_t clip_gate = SIZE_MAX);
  Q_INVOKABLE QString OR_BAD_FLAGS_BETWEEN(QString field, float constantLower,
                                            float constantUpper, 
					    QString mask_field, float bad_data = FLT_MIN,
					    size_t clip_gate = SIZE_MAX);
  Q_INVOKABLE QString XOR_BAD_FLAGS_ABOVE(QString field, float constant, 
					    QString mask_field, float bad_data = FLT_MIN,
					    size_t clip_gate = SIZE_MAX);
  Q_INVOKABLE QString XOR_BAD_FLAGS_BELOW(QString field, float constant, 
					    QString mask_field, float bad_data = FLT_MIN,
					    size_t clip_gate = SIZE_MAX);
  Q_INVOKABLE QString XOR_BAD_FLAGS_BETWEEN(QString field, 
					    float constantLower, float constantUpper, 
					    QString mask_field, float bad_data = FLT_MIN,
					    size_t clip_gate = SIZE_MAX);

  Q_INVOKABLE QString COPY_BAD_FLAGS(QString field, float bad_data = FLT_MIN,
				     size_t clip_gate = SIZE_MAX);

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

  void setCurrentRayToFirst();
  bool moreRays();
  void nextRay();

  void setCurrentSweepToFirst();
  bool moreSweeps();
  void nextSweep();

  void applyBoundary(bool useBoundaryMask);
  const vector<bool> *GetBoundaryMask();
  void assign(string tempName, string userDefinedName);
  const vector<float> *getData(string &fieldName);
  void setData(string &fieldName, vector<float> *fieldData);

private:

  RadxVol *_data;
  size_t _currentSweepIdx;
  size_t _currentRayIdx;
  size_t _nRays;
  size_t _nSweeps;
  SoloFunctionsModel soloFunctionsModel;

  template<typename Out>
  void split(const string &s, char delim, Out result);
  vector<string> split(const string &s, char delim);
  vector<double> splitDouble(const string &s, char delim);
 

};


#endif
