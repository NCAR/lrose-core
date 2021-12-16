// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
////////////////////////////////////////////////////////////////////////////////
//
// Brenda Javornik, UCAR, Boulder, CO, 80307, USA
// March 2021
//
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _DATAMODEL_HH
#define _DATAMODEL_HH

#include <Radx/RadxRay.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxGeoref.hh>

#include <vector>


using namespace std;


class DataModel {

public:
  static DataModel *Instance();

  ~DataModel();

  //void setData(RadxVol *vol);
  void readData(string path, vector<string> &fieldNames,
    bool debug_verbose, bool debug_extra);
  void writeData(string path);

  void update();
  void SetDataByIndex(string &fieldName, 
            int rayIdx, int sweepIdx, vector<float> *fieldData);
  void SetData(string &fieldName, 
            float azimuth, float sweepAngle, vector<float> *fieldData);
  void SetData(string &fieldName, 
            int rayIdx, RadxSweep *sweep, vector<float> *fieldData);

  void SetData(string &fieldName, float value);

  // remove field from volume
  void RemoveField(string &fieldName); 
  void RemoveField(size_t rayIdx, string &fieldName);

  void renameField(string currentName, string newName);
  void renameField(size_t rayIdx, string currentName, string newName);
  void copyField(size_t rayIdx, string fromFieldName, string toFieldName);
  bool fieldExists(size_t rayIdx, string fieldName);

  void regularizeRays();

  void get();
  const vector<float> *GetData(string fieldName,
              int rayIdx, int sweepIdx);

  RadxTime getStartTimeSecs();
  RadxTime getEndTimeSecs();

  RadxField *fetchDataField(RadxRay *ray, string &fieldName);
  //const RadxField *fetchDataField(const RadxRay *ray, string &fieldName);
  const float *fetchData(RadxRay *ray, string &fieldName);
  size_t getNRays(); // string fieldName, double sweepAngle);
  size_t getNRays(int sweepNumber);
  size_t getNRaysSweepIndex(int sweepIndex);
  size_t getFirstRayIndex(int sweepIndex);
  size_t getLastRayIndex(int sweepIndex);  
  int getSweepNumber(int sweepIndex);
  vector<RadxRay *> &getRays();
  RadxRay *getRay(size_t rayIdx);
  vector<float> *getRayData(size_t rayIdx, string fieldName); // , double sweepHeight);
  float getMissingFl32(string fieldName);
  double getRayAzimuthDeg(size_t rayIdx);
  double getRayNyquistVelocityMps(size_t rayIdx);

  int getNSweeps();
  vector<double> *getSweepAngles();
  int getSweepNumber(float elevation);

  const string &getPathInUse();
  const RadxPlatform &getPlatform();

  float getLatitudeDeg();
  float getLongitudeDeg();
  float getAltitudeKm();

  void getPredomRayGeom(double *startRangeKm, double *gateSpacingKm);
  const RadxGeoref *getGeoreference(size_t rayIdx);
  vector<string> *getUniqueFieldNameList();

  int getNGates(size_t rayIdx, string fieldName = "", double sweepHeight = 0.0);

  size_t findClosestRay(float azimuth, int sweepNumber); // float elevation);
  size_t getRayIndex(size_t baseIndex, int offset, int sweepNumber);

  void printAzimuthInRayOrder();

  
private:
  
  DataModel();

  void init();
  void _setupVolRead(RadxFile &file, vector<string> &fieldNames,
    bool debug_verbose, bool debug_extra);

  size_t calculateRayIndex_f(size_t idx, size_t start, size_t end, int offset);

//DataModel *DataModel::_instance = NULL;  
  static DataModel *_instance;

  RadxVol *_vol;

};

#endif
