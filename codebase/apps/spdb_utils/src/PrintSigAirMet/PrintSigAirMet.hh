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
// PrintSigAirMet.hh
//
// PrintSigAirMet object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2003
//
///////////////////////////////////////////////////////////////

#ifndef PrintSigAirMet_H
#define PrintSigAirMet_H

#include <string>
#include <map>
#include <rapformats/coord_export.h>
#include <rapformats/SigAirMet.hh>
#include <Spdb/DsSpdb.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

typedef multimap<double, SigAirMet*, less<double> > distmap_t;
typedef pair<const double, SigAirMet*> distpair_t;

////////////////////////
// This class

class PrintSigAirMet {
  
public:

  // constructor

  PrintSigAirMet (int argc, char **argv);

  // destructor
  
  ~PrintSigAirMet();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  coord_export_t *_coordShmem;
  DsSpdb _spdb;
  vector<SigAirMet *> _reports;

  // methods

  int _run_once();

  int _follow_cidd();
  
  //  int _retrieve(time_t data_time);
  bool _retrieve(time_t retrieveTime, bool doDataTypes,
		 int dataType, int dataType2, bool doValid,
		 time_t startTime, time_t endTime);
    
  void _sortByDistance(double search_lat, double search_lon);
  
  void _sortByBoundingBox(double minLat, double minLon,
			  double maxLat, double maxLon);

  void _sortByWeatherType(string weatherType);

  void _sortByDataType(vector <int> dataTypes, int dataType2, time_t retrieveTime,
		       bool doValid, time_t startTime, time_t endTime);

  void _printNoDataMsg();

  void _print();

  void _tokenize(const string &str,
		 const string &spacer,
		 vector<string> &toks);

  void _clearReports();

  int _WildCard(char *WildSpec, char *String, int debug);

};

#endif

