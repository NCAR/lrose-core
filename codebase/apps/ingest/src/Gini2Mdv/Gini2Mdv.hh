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

#ifndef _Gini2Mdv_
#define _Gini2Mdv_

// C++ Standard Include Files
#include <cstdio>
#include <map>

// No RAP Include Files

// Local Include Files
#include "InputManager.hh"
#include "Params.hh"

// Forward Declarations
class Args;
class Params;
class GiniCalibrationCurve;
class OutputUrl;
class GiniWriteMdv;

using namespace std;

// Class Declaration
class Gini2Mdv
   {
   public:
      ////////////////////////////
      // NO PUBLIC DATA MEMBERS //
      ////////////////////////////

      ////////////////////
      // PUBLIC METHODS //
      ////////////////////

      // Constructors and Destructors
      Gini2Mdv(int argc, char **argv);
      virtual ~Gini2Mdv();

      // Set Methods
      bool setInputDataMap();
      void setDebug(bool debug) { _debug = debug; }

      // Get Methods
      FILE* getGiniInputFilePtr()     { return _giniInputFilePtr; }
      bool getDebug()                 { return _debug; }
      const bool isOK() const         { return _isOK; }
      const string& getErrStr() const { return _errStr; }

      // General Methods
      int run();
      bool init();
      bool buildCalibrationCurveMap();
      bool beginProcessing();
      bool calibrateGiniData();
      bool openGiniInputFile(string fileWithPath);

   protected:
      ///////////////////////////////
      // NO PROTECTED DATA MEMBERS //
      ///////////////////////////////

      //////////////////////////
      // NO PROTECTED METHODS //
      //////////////////////////

   private:
      //////////////////////////
      // PRIVATE DATA MEMBERS //
      //////////////////////////
      bool _isOK;
      string _errStr;
      bool _debug;
      Args *_args;
      Params *_params;
      map<string, InputManager::input_data_t> _inputDataMap;
      map<string, InputManager*> _inputManagerMap;
      map<string, GiniCalibrationCurve*> _calibrationCurveMap;
      FILE *_giniInputFilePtr;
      fl32 *_calibratedBuffer;
      OutputUrl *_outputUrl;
      bool _weReallyHaveToGetGoing;

      /////////////////////
      // PRIVATE METHODS //
      /////////////////////
      bool _filePendingProcessing();
     bool _initArchiveMode(time_t archive_time,Params::op_search_mode);
      bool _initRealtimeDirMode();
      bool _runArchiveMode();
      bool _runArchiveModeOnce();
      bool _runRealtimeDirMode();
      bool _forcedProcessCheck();
      void _reinitializeAllDataManagers(bool rewindDirFlag);
      void _dumpAllInputManagers();
   };
#endif // _Gini2Mdv_
