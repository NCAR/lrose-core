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
// October 2022
//
// Static class to access data files? the problem is adding a new field to a ray,
// the merging of new fields with existing fields into a new file?? or into
//  I guess it is always writing the output to a temporary file in the undo/redo stack.
// Read ray of data with some fields; edit the fields, create new fields,
// then merge the modified vector along with the unmodified 
// fields finally write to the temp file.
// can I write a partial volume?
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _ScriptsDataModel_HH
#define _ScriptsDataModel_HH

#include <Radx/RadxRay.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxGeoref.hh>

#include <vector>

using namespace std;


class ScriptsDataModel {

public:

  // 
  // CRUD: Create, Read, Update, Delete fields one ray at a time
  //

  ScriptsDataModel();
  ~ScriptsDataModel();

  // every method needs:
  // string &inputPath, 
  // bool debug_verbose, 
  // bool debug_extra,
  // string fieldName,
  // size_t rayIdx,
  // size_t sweepNumber (optional)

  bool okToStream();
/*
  int openRead(string &inputPath, int sweepNumber,
    vector<string> *fieldNames, bool debug_verbose = false, 
    bool debug_extra = false);
*/
  //void setData(RadxVol *vol);
  void readData(string path, bool applyCorrectionFactors,
    bool debug_verbose = false, bool debug_extra = false);
  void readData(string path, bool applyCorrectionFactors,
    vector<string> &fieldNames,
    bool debug_verbose = false, bool debug_extra = false);
  void readData(string path, bool applyCorrectionFactors,
    vector<string> &fieldNames,
    int sweepNumber,
    bool debug_verbose = false, bool debug_extra = false);
  void readFileMetaData(string fileName);
  void getRayData(string path, vector<string> &fieldNames,
  int sweepNumber);

  void writeData(string path);
  void writeData(string path, RadxVol *vol);

  int mergeDataFiles(string dest_path, string source_path, string original_path);

  vector<string> *getPossibleFieldNames(string fileName);
  vector<string> *getUniqueFieldNameList();

  const RadxVol& getVolume();

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
  void copyField2(size_t rayIdx, string fromFieldName, string toFieldName);
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


  vector<RadxRay *> &getRays();
  RadxRay *getRay(size_t rayIdx);
  vector<float> *getRayData(size_t rayIdx, string fieldName); // , double sweepHeight);
  float getMissingFl32(string fieldName);
  double getRayAzimuthDeg(size_t rayIdx);
  double getRayNyquistVelocityMps(size_t rayIdx);

  int getNSweeps();
  const RadxSweep &getSweep(size_t sweepIndex);
  vector<double> *getSweepAngles();
  vector<int> *getSweepNumbers();
  //vector<double> *getSweepAngles(string fileName);
  //vector<double> *getPossibleSweepAngles(string fileName);  // TODO: load sweep angles (cache) when fetching field names
  int getSweepNumber(float elevation);
  int getSweepNumber(int sweepIndex);
  int getSweepIndexFromSweepNumber(int sweepNumber);
  int getSweepIndexFromSweepAngle(float elevation);
  double getSweepAngleFromSweepNumber(int sweepNumber);

  const string &getPathInUse();
  const RadxPlatform &getPlatform();

  float getLatitudeDeg();
  float getLongitudeDeg();
  float getAltitudeKm();

  void getPredomRayGeom(double *startRangeKm, double *gateSpacingKm);
  double getRadarBeamWidthDegV();
  double getCfactorRotationCorr();
  double getCfactorPitchCorr();
  double getCfactorHeadingCorr();
  double getCfactorDriftCorr();
  double getCfactorRollCorr();
  double getCfactorTiltCorr();
  double getCfactorElevationCorr();
  double getCfactorAzimuthCorr();
  const RadxGeoref *getGeoreference(size_t rayIdx);

  int getNGates(size_t rayIdx, string fieldName = "", double sweepHeight = 0.0);

  size_t findClosestRay(float azimuth, int sweepNumber); // float elevation);
  size_t getRayIndex(size_t baseIndex, int offset, int sweepNumber);

  RadxVol *mergeDataFields(string originalSourcePath);
  RadxVol *mergeDataFields(string currentVersionPath, string originalSourcePath);

  Radx::PrimaryAxis_t getPrimaryAxis();

  void printAzimuthInRayOrder();
  void writeWithMergeData(string outputPath, string originalSourcePath);
  void writeWithMergeData(string outputPath, string currentVersionPath, string originalSourcePath);

  RadxVol *getRadarVolume(string path, vector<string> *fieldNames,
     bool debug_verbose = false, bool debug_extra = false);

  RadxVol *getRadarVolume(string path, vector<string> *fieldNames,
    int sweepNumber,
    bool debug_verbose = false, bool debug_extra = false);

  void getSweepsAndFields(string fileName);
  void deleteLookAhead();
  void moveToLookAhead();
  void clearVolume();

  bool sweepRaysAreIndexed(size_t sweepIndex);
  double getSweepAngleResDeg(size_t sweepIndex);

private:
  


  void init();
  void _initForRead();
  void _readData(RadxFile &file, string path);
  void _setupVolRead(RadxFile &file, vector<string> &fieldNames,
    int sweepNumber,
    bool debug_verbose, bool debug_extra);
  void _setupVolRead(RadxFile &file, vector<string> &fieldNames,
    bool debug_verbose, bool debug_extra);

  void applyCorrectionFactors(RadxRay *ray);

  void _selectFieldsNotInVolume(vector<string> *allFieldNames);
  void _selectFieldsNotInCurrentVersion(
    vector<string> *currentVersionFieldNames, vector<string> *allFieldNames);

  size_t calculateRayIndex_f(size_t idx, size_t start, size_t end, int offset);

//ScriptsDataModel *ScriptsDataModel::_instance = NULL;  
//  static ScriptsDataModel *_instance;

  string _currentFilePath;
  
  RadxVol *_vol;

  bool _applyCorrectionFactors;

  //NcfRadxStream *_vol;
  bool _okToStream;

  // cache stores metadata until the entire data file is read and _vol is filled
  // cache contains all sweeps but only the selected fields
  bool _cacheMetaDataValid;
  //const 
  vector<int> _cacheSweepNumbers;
  vector<double> _cacheSweepAngles;
  vector<RadxField *> _cacheFields;

  // look ahead continas all sweeps and all fields for the next (n+1) file to open
  vector<int> _lookAheadSweepNumbers;
  vector<double> _lookAheadSweepAngles;
  vector<RadxField *> _lookAheadFields;  

};

#endif
