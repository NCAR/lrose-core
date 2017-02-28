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
/////////////////////////////////////////////////////////////
// Hsrl2Radx.hh
//
// Hsrl2Radx object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2015
//
///////////////////////////////////////////////////////////////

#ifndef Hsrl2Radx_HH
#define Hsrl2Radx_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <Radx/RadxTime.hh>
class RadxVol;
class RadxFile;
class MslFile;
using namespace std;


class CalVals{
private:
  string varName;
  string varUnits;
  vector<RadxTime> time;
  vector<string> dataStr;
  vector< vector<double> > dataNum;
  bool isNum, isStr;
public:

  CalVals();
  CalVals(string inName, string inUnits, vector< RadxTime > inTime, vector<string> inDataStr);//constructor for string type data
  CalVals(string inName, string inUnits, vector< RadxTime > inTime, vector< vector<double> > inDataNum);////constructor for num type data
 
  void setVarName(string inName);
  void setVarUnits(string inUnits);
  void setTime(vector<RadxTime> inTime);
  void addTime(RadxTime inTime);
  void setDataStr(vector<string> inDataStr);
  void addDataStr(string inDataStr);
  void setDataNum(vector< vector<double> > inDataNum);
  void addDataNum(vector<double> inDataNum);
  void setIsStr();
  void setIsNum();
  bool dataTypeisNum();
  bool dataTypeisStr(); string getVarName();
  string getVarUnits();
  vector<RadxTime> getTime();
  vector<string> getDataStr();
  vector< vector<double> > getDataNum();
  void printBlock();

  ~CalVals();
    
};

class Hsrl2Radx {
  
public:

  // constructor
  
  Hsrl2Radx (int argc, char **argv);

  // destructor
  
  ~Hsrl2Radx();

  // run 

  int Run();

  // data members

  int OK;

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  vector<string> _readPaths;

  int _runFilelist();
  int _runArchive();
  int _runRealtimeWithLdata();
  int _runRealtimeNoLdata();
  int _processFile(const string &filePath);
  int _processUwCfRadialFile(const string &filePath);
  void _setupRead(MslFile &file);
  void _overrideGateGeometry(RadxVol &vol);
  void _setRangeRelToInstrument(MslFile &file,
                                RadxVol &vol);
  void _convertFields(RadxVol &vol);
  void _setupWrite(RadxFile &file);
  void _setGlobalAttr(RadxVol &vol);
  int _writeVol(RadxVol &vol);

  int _processUwRawFile(const string &filePath);
  void _addEnvFields(RadxVol &vol);

  vector <vector<double> > _readBaselineCorrection(const char* file, bool debug);
  vector <vector<double> > _readDiffDefaultGeo(const char* file, bool debug);
  vector <vector<double> > _readGeofileDefault(const char* file, bool debug);
  vector <vector<double> > _readAfterPulse(const char* file, bool debug);
  CalVals _readCalvals(const char* file, const char* variable, bool debug);
  string _removeWhitespace(string s);
  int _checkForChar(string subSt, string s);
};

#endif
