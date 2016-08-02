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
/*
-------------------------------------------------------------------------------
SstNc2MdvApp.hh - header file for SstNc2MdvApp application

Steve Carson, RAP, NCAR, Boulder, CO, 80307, USA
April 2006
-------------------------------------------------------------------------------
*/

#ifndef SstNc2MdvApp_h
#define SstNc2MdvApp_h

//
// SYSTEM INCLUDES
//

#include <string>
#include <vector>

#include <toolsa/Path.hh>
#include <toolsa/MsgLog.hh>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <didss/DsInputPath.hh>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <Mdv/MdvxField.hh>

#include "netcdf.hh"

//
// PROJECT INCLUDES
//

//
// LOCAL INCLUDES
//

#include "Params.hh"
#include "SstNc2MdvGlobals.hh"
#include "CmdLineArgs.hh"
#include "InputHandler.hh"
#include "NetcdfUtils.hh"
#include "NetcdfDataset.hh"

using namespace std;

//
// DEFINES
//

#define MAX_OUTPUT_LATS 1800
#define MAX_OUTPUT_LONS 3600
#define MAX_FIELD_PTS   (MAX_OUTPUT_LATS * MAX_OUTPUT_LONS)


class SstNc2MdvApp
{ // begin class SstNc2MdvApp

public:

   //
   //-----------------
   // Member Functions
   //-----------------
   //
   
   //
   //--- Constructor
   //

   SstNc2MdvApp();

   //
   // Destructor
   //

   ~SstNc2MdvApp();

   //
   // Initialization
   //

   int
      Init( int argc, char **argv );

   //
   // Messaging
   //

   MsgLog&
      GetMsgLog() { return mMsgLog; }

   //
   // Execution
   //

   int
      Run();

   //
   // Inquiry (data access)
   //

   const string&
      GetProgramName() { return mProgPath.getFile(); }

private:

   //
   //------------
   // Member Data
   //------------
   //

   vector<string>
      mInputPathList;

  // Object for handing input from the different netCDF file types

  InputHandler *_inputHandler;

   //
   // Mdvx SST objects;
   //

   DsMdvx
      mMdvxOutSst;

   MdvxPjg
      mOutputMdvxProj;

   int
      mOutSstNumRows,
      mOutSstNumCols,
      mOutSstNumDataPts;
  

   //
   // MsgLog  - provides access to log file
   //

   MsgLog
      mMsgLog;

   //
   // DsInputPath objects for SST data:
   //

   DsInputPath
      *mNewSstNcPath;

   //
   // Start time and end time;
   // used only in run mode ARCHIVE_START_END_TIMES
   //

   time_t
      mStartTime,
      mEndTime;

   //
   // Command line args
   //

   CmdLineArgs
      *mCmdLineArgs;

   //
   // "Path" provides a mechanism to get the full-path filename
   // of the executable that was invoked to start running this program
   //

   Path mProgPath;    // for full-path filename of this program

   //
   // TDRP parameters
   //

   Params
      mParams;

   char
      *paramPath;

   //
   //-----------------
   // Member Functions
   //-----------------
   //

   /////////////////////////////////////////////////////////////////////////
   // InitInputHandler
   //
   // Initialize the input handler object

   void
   InitInputHandler();

   /////////////////////////////////////////////////////////////////////////
   // InitOutputMdvxProj
   //
   // Initializes the projection SstNc2MdvApp::mOutputMdvxProj according
   // to the projection type mParams.remap_proj_type and the values in the
   // sst_output_grid_t struct argument.

   void
   InitOutputMdvxProj();

   int
   RunRealtimeMode( void );

   int
   RunArchiveMode( void );

  /*
    -------------------------------------------------------------------------------
    ProcessFile

    Process the given raw data file.
    -------------------------------------------------------------------------------
  */

  int ProcessFile( const string &inp_sst_nc_path );
  
   MdvxField*
   CreateMdvxField
      (
      MdvxPjg     *aMdvProj,
      time_t      aNetcdfFileUtime,
      fl32        aBadDataValue,
      fl32        aMissingDataValue,
      const char  *aFieldName,
      const char  *aFieldNameLong,
      const char  *aFieldUnits
      );

   //////////////////////////////////////////////////////////////////////////
   // InitMdvMasterHeader
   //
   // Initialized the master header of the "aMdvx" object.
   //
   // NOTE: this function must not be called until AFTER the Mdv
   // output projection "mOutputMdvxProj" has been initialized!

   void
   InitMdvMasterHeader
      (
      const string    &aInputFilePath, // input: Netcdf input file path
      time_t          aVolumeUnixTime, // input: Unix time of this volume
      const MdvxPjg   &arOutputProj,   // input: MdvxProj object for output    

      DsMdvx          *aMdvx           // output: modified DsMdvx object
      );

   //
   // TDRP parameter processing
   //

   int
   ReadParams( int argc, char **argv );

   int
   ProcessParams();

   //
   // Free memory allocated by SstNc2MdvApp
   //

   void
   FreeMemoryPerRun( void );

}; // end class SstNc2MdvApp

#endif /* SstNc2MdvApp_h */
