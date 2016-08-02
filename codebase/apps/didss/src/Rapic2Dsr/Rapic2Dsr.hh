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
// Rapic2Dsr.hh
//
// Rapic2Dsr object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2000
//
///////////////////////////////////////////////////////////////

#ifndef Rapic2Dsr_HH
#define Rapic2Dsr_HH

#include <string>
#include <vector>
#include "Args.hh"
#include "Params.hh"
#include "sRadl.hh"
#include "Linebuff.hh"
#include "ScanParams.hh"
#include "PPIField.hh"

#include <Fmq/DsRadarQueue.hh>
using namespace std;


class DsInputPath;

////////////////////////
// This class

class Rapic2Dsr {
  
public:

  typedef struct {
    int scan_num;
    int station_id;
    int flag1;
    double elev_angle;
    int field_num;
    int n_fields;
    int flag2;
    int flag3;
    char time_str[32];
  } scan_description_t;

  // constructor

  Rapic2Dsr (int argc, char **argv);

  // destructor
  
  ~Rapic2Dsr();

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
  DsInputPath *_input;
  DsRadarQueue _rQueue;

  int _volNum;
  int _scanType;

  bool _scanListComplete;
  int _nScans;
  int _nFields;
  vector<scan_description_t> _scanList;
  vector<double> _elevList;
  vector<PPIField *> _ppiFields;

  int _processFile(const char *file_path);
  int _processImage(const char *file_path, Linebuff &lineBuf, int imageNum);
  int _processPPI(int ppi_num, Linebuff &line_buf, ScanParams &sparams);
  void _clearPpiFields();
  int _decodeRadial(Linebuff &lineBuf, sRadl &radial,
		    bool &isBinary, int rLevels);

  int _findImageStart(Linebuff &lineBuf);
  int _findImageEnd(Linebuff &lineBuf);

  void _clearScanList();
  int _addToScanList(Linebuff &lineBuf);
  void _printScanList(ostream &out);

  int _loadScanParams(ScanParams &sParams, Linebuff &lineBuf);
  int _checkScanParams(const ScanParams &sParams);
  
  int _writeRadarAndFieldParams(int maxGates,
				const ScanParams &sParams);

  
  int _writeBeams(int ppi_num,
		  int maxGates,
		  int maxBeams,
		  const ScanParams &sParams);

  bool _azLessThan(double az1, double az2);

};

#endif
