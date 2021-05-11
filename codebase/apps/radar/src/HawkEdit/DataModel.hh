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
  void get();

  RadxTime getStartTimeSecs();
  RadxTime getEndTimeSecs();

  RadxField *fetchDataField(RadxRay *ray, string &fieldName);
  const float *fetchData(RadxRay *ray, string &fieldName);
  size_t getNRays(); // string fieldName, double sweepAngle);
  const vector<RadxRay *> &getRays();
  vector<float> *getRayData(size_t rayIdx, string fieldName, double sweepHeight);
  float getMissingFl32(string fieldName);
  double getRayAzimuthDeg(size_t rayIdx);

  int getNSweeps();
  vector<double> *getSweepAngles();
  const string &getPathInUse();
  const RadxPlatform &getPlatform();

  void getPredomRayGeom(double *startRangeKm, double *gateSpacingKm);
  const vector<string> &getUniqueFieldNameList();



private:
  
  DataModel();

  void init();
  void _setupVolRead(RadxFile &file, vector<string> &fieldNames,
    bool debug_verbose, bool debug_extra);

//DataModel *DataModel::_instance = NULL;  
  static DataModel *_instance;

  RadxVol _vol;

};

#endif
