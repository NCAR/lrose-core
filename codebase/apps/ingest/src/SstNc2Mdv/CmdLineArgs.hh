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
//-----------------------------------------------------------------------------
// CmdLineArgs.hh
//
// Header file for class CmdLineArgs
//-----------------------------------------------------------------------------

#ifndef CmdLineArgs_h
#define CmdLineArgs_h

// SYSTEM INCLUDES

#include <vector>
#include <string>

// PROJECT INCLUDES

#include <toolsa/MsgLog.hh>
#include <toolsa/DateTime.hh>

// LOCAL INCLUDES

#include "SstNc2MdvGlobals.hh"
#include "Params.hh"

// DEFINES

#define STR_EQL(test,std) (strcmp(test,std)==0)

//--- command line arguments

// NEW ARGS NEEDED:
// -old_sst_file
// -old_sst_dir
// -run_mode


#ifdef CmdLineArgs_cc
   static string cmdArgTags[] =
      {
      "-h",
      "-help",
      "-instance",
      "-run_mode",
      "-fname_type",
      "-file_list",
      "-input_dir",
      "-output_dir",
      "-diag_dir",
      "-debug_level",
      "-diag_level",
      "-ncd_diag_level",
      "-start_time",
      "-end_time"
      };
#endif // CmdLineArgs_cc

enum
   {
   ARG_HELP = 0,
   ARG_HELPLONG,
   ARG_INSTANCE,
   ARG_RUN_MODE,
   ARG_FNAME_TYPE,
   ARG_INPFILELIST,
   ARG_INPUTDIR,
   ARG_OUTPUTDIR,
   ARG_DIAGDIR,
   ARG_DEBUG_LVL,
   ARG_DIAG_LVL,
   ARG_NCD_LVL,
   ARG_STARTTIME,
   ARG_ENDTIME,
   ARG_NARGS
   };

// CONSTS

#define CMDARG_PRINT_USAGE   1
#define CMDARG_UNKNOWN_ARG   2
#define CMDARG_SUCCESS       3

// FORWARD REFERENCES

class CmdLineArgs

{ // begin class CmdLineArgs

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

// DATA MEMBERS

   // Command line arguments

   vector<string>
      mInputFileList;

   string
      mInstance,
      mRunModeStr,
      mFnameType,
      mStartTimeStr,
      mEndTimeStr,
      mInputDir,
      mOutputDir,
      mDiagDir,
      mParamsFileName;

   time_t
      mStartTime,
      mEndTime;

   int
      mDebugLevel,
      mDiagOutLevel,
      mNcdDiagOutLevel;

   // TDRP overrides

   tdrp_override_t
      tdrpOverride;

// LIFECYCLE

   // default constructor

   CmdLineArgs( MsgLog *log );

   // copy constructor
   // @param from - I - The value to copy to this object.

   CmdLineArgs(const CmdLineArgs& from);

   // destructor

   ~CmdLineArgs(void);

// OPERATORS

   // assignment operator (=)
   // @param from - I - The value to assign to this object.
   // @returns A reference to this object.

   CmdLineArgs& operator=(CmdLineArgs& from);  

// OPERATIONS

   // read command line arguments

   int
      ReadArgs(int argc, char** argv);

   void
      PrintArgs(FILE *OUT);

   MsgLog&
      GetMsgLog(){ return *msgLog; };

   void
      usage( void );
   
// ACCESS
  
// INQUIRY

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:

   MsgLog
      *msgLog;

   string
      mClassIDStr;
  
}; // end class CmdLineArgs

// INLINE METHODS

// EXTERNAL REFERENCES

#endif  // CmdLineArgs_h
