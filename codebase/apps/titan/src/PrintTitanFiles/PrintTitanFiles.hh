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
// PrintTitanFiles.h
//
// PrintTitanFiles object
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2001
//
///////////////////////////////////////////////////////////////
//
// PrintTitanFiles produces ASCII output from TITAN binary files.
// Output goes to stdout.
//
///////////////////////////////////////////////////////////////

#ifndef PrintTitanFiles_H
#define PrintTitanFiles_H

#include "Args.hh"
#include <toolsa/Path.hh>
using namespace std;

class TitanStormFile;
class TitanTrackFile;
class TitanFile;

class PrintTitanFiles {
  
public:
  
  // constructor
  
  PrintTitanFiles (int argc, char **argv);
  
  // destructor
  
  ~PrintTitanFiles();

  // run 

  int Run();

  bool OK;
  
protected:
  
private:

  string _progName;
  Args _args;

  bool _isNcFile;
  bool _isStormFile;
  bool _isTrackFile;

  Path _ncFilePath;
  Path _stormFilePath;
  Path _trackFilePath;
  
  int _setPrintDetails();
  int _readLabel(char *label);
  
  int _printLegacy();

  int _printNcFile();
  int _printStormsNc(TitanFile &tFile);
  int _printTracksNc(TitanFile &tFile);

  int _printTrackFullNc(TitanFile &tFile);
  int _printTrackSummaryNc(TitanFile &tFile);

  int _printStormFileLegacy();
  int _printTrackFileLegacy();

  int _printTrackFullLegacy(TitanStormFile &sfile, TitanTrackFile &tfile);
  int _printTrackSummaryLegacy(TitanStormFile &sfile, TitanTrackFile &tfile);

  int _printCsvTableType1(TitanStormFile &sfile, TitanTrackFile &tfile);
  int _printCsvTableType2(TitanStormFile &sfile, TitanTrackFile &tfile);
  int _printCsvTableType3(TitanStormFile &sfile, TitanTrackFile &tfile);
  int _printCsvTableType4(TitanStormFile &sfile, TitanTrackFile &tfile);
  int _printCsvTableType5(TitanStormFile &sfile, TitanTrackFile &tfile);

  int _printStormsXml();
  int _printTracksXml();
  
};

#endif
