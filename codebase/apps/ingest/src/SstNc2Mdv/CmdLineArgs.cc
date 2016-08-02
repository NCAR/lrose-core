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
// CmdLineArgs.cc
//
// Source file for class CmdLineArgs
//-----------------------------------------------------------------------------

#define CmdLineArgs_cc

// SYSTEM INCLUDES 

// PROJECT INCLUDES

// LOCAL INCLUDES

#include "CmdLineArgs.hh"

// LOCAL CONSTS

const string  CLASS_NAME = "CmdLineArgs";

// FORWARD REFERENCES 

//----------------------------------------------------------------------------
// CmdLineArgs() - default constructor
//----------------------------------------------------------------------------

CmdLineArgs::
CmdLineArgs( MsgLog *log )
   : mClassIDStr( "CmdLineArgs" )

{ //--- begin CmdLineArgs

msgLog = log;

} //--- end CmdLineArgs

//----------------------------------------------------------------------------
// ~CmdLineArgs() - destructor
//----------------------------------------------------------------------------

CmdLineArgs::
~CmdLineArgs()

{ // begin ~CmdLineArgs

} // end ~CmdLineArgs

//----------------------------------------------------------------------------
// operator &
//----------------------------------------------------------------------------

CmdLineArgs& 
CmdLineArgs::
operator=(CmdLineArgs&)

{ // begin CmdLineArgs&

return *this;

} // end CmdLineArgs&

//----------------------------------------------------------------------------
// ReadArgs
//----------------------------------------------------------------------------

int
CmdLineArgs::
ReadArgs(int argc, char** argv)

{ //--- begin ReadArgs

 const string METHOD_NAME = "ReadArgs";

 int
    iArg,
    iToken;

 bool
    file_list_found  = false,
    start_time_found = false,
    end_time_found   = false,
    args_ok          = true;

 //--- Assign command line argument default values (if any)

 mStartTime       = DateTime::NEVER;
 mEndTime         = DateTime::NEVER;
 mDebugLevel      = 0;
 mDiagOutLevel    = 0;
 mNcdDiagOutLevel = 0;

 //--- Override defaults with command line args

 char paramStr[256];

 TDRP_init_override( &tdrpOverride );

 for ( iArg = 1; iArg < argc; ++iArg )
    {

    if (string(argv[iArg]) == cmdArgTags[ARG_HELP])
       {
       return CMDARG_PRINT_USAGE;
       }
    else if (string(argv[iArg]) == cmdArgTags[ARG_HELPLONG])
       {
       return CMDARG_PRINT_USAGE;
       }
    else if (string(argv[iArg]) == cmdArgTags[ARG_INSTANCE])
       {
       if(iArg+1 < argc)
          {
          mInstance.assign( argv[++iArg] );

          // override corresponding TDRP parameter
          sprintf(paramStr, "instance = ""%s"";", mInstance.c_str());
          TDRP_add_override( &tdrpOverride, paramStr );
          }
       else
          {
          POSTMSG(ERROR,
		  "Argument '%s' needs a value",
		  cmdArgTags[ARG_INSTANCE].c_str());
          args_ok = false;
          }

       }
    else if (string(argv[iArg]) == cmdArgTags[ARG_RUN_MODE] ||
	     string(argv[iArg]) == "-mode")
       {
       if(iArg+1 < argc)
          {
          mRunModeStr.assign(argv[++iArg]);

          // override corresponding TDRP parameter
          sprintf(paramStr, "run_mode = ""%s"";", mRunModeStr.c_str());
          TDRP_add_override( &tdrpOverride, paramStr );
          }
       else
          {
          POSTMSG(ERROR,
		  "Argument '%s' needs a value",
		  cmdArgTags[ARG_RUN_MODE].c_str());
          args_ok = false;
          }

       }
    else if (string(argv[iArg]) == cmdArgTags[ARG_FNAME_TYPE])
       {
       if(iArg+1 < argc)
          {
          mFnameType.assign( argv[++iArg] );

          // override corresponding TDRP parameter
          sprintf(paramStr, "input_filename_type = %s;", mFnameType.c_str());
          TDRP_add_override( &tdrpOverride, paramStr );
          }
       else
          {
          POSTMSG(ERROR,
		  "Argument '%s' needs a value",
		  cmdArgTags[ARG_FNAME_TYPE].c_str());
          args_ok = false;
          }

       }
    else if (string(argv[iArg]) == cmdArgTags[ARG_INPFILELIST])
       {

       // read in list of input files

       if( iArg+1 < argc ) // if there exists at least one more arg
          {

          // Search until next token starting with "-" (dash) is found,
          // or until there are no more args. Assume each argument found
          // thusly is the name of a input file to be processed.

          for( iToken = iArg+1; iToken < argc; ++iToken )
             {
             if ((argv[iToken][0] == '-') || (iToken >= argc) )
                break;
             else
                mInputFileList.push_back( argv[iToken] );
             }

          // Make sure at least one filename follows the "-file_list" arg

          if ( mInputFileList.size() > 0 )
             {
             file_list_found = true;
             // override corresponding TDRP parameter
             TDRP_add_override(&tdrpOverride, "run_mode = ARCHIVE_FILE_LIST;");
             }
          else
             {
             POSTMSG( ERROR, "Missing file list specification." );
             args_ok = false;
             }

          // Advance iArg to point to the next argument after the last
          // filename following the "-file_list" argument.

          iArg = iToken;

          }
       else // missing file list: iArg+1 >= argc
          {
          args_ok = false;

          POSTMSG(
             ERROR, "Not enough arguments: Missing file list specification." );
          } // end if iArg+1 < argc

       }
    else if (string(argv[iArg]) == cmdArgTags[ARG_INPUTDIR])
       {

       if(iArg+1 < argc)
          {
          mInputDir.assign( argv[++iArg] );

          // override corresponding TDRP parameter
          sprintf(paramStr, "input_dir = ""%s"";", mInputDir.c_str());
          TDRP_add_override( &tdrpOverride, paramStr );
          }
       else
          {
          POSTMSG(ERROR,
		  "Argument '%s' needs a value",
		  cmdArgTags[ARG_INPUTDIR].c_str());
          args_ok = false;
          }

       }
    else if (string(argv[iArg]) == cmdArgTags[ARG_OUTPUTDIR])
       {

       if(iArg+1 < argc)
          {
          mOutputDir.assign( argv[++iArg] );

          // override corresponding TDRP parameter
          sprintf(paramStr, "output_dir = ""%s"";", mOutputDir.c_str());
          TDRP_add_override( &tdrpOverride, paramStr );
          }
       else
          {
          POSTMSG(ERROR,
		  "Argument '%s' needs a value",
		  cmdArgTags[ARG_OUTPUTDIR].c_str());
          args_ok = false;
          }

       }
    else if (string(argv[iArg]) == cmdArgTags[ARG_DIAGDIR])
       {

       if(iArg+1 < argc)
          {
          mDiagDir.assign( argv[++iArg] );

          // override corresponding TDRP parameter
          sprintf(paramStr, "diag_dir = ""%s"";", mDiagDir.c_str());
          TDRP_add_override( &tdrpOverride, paramStr );
          }
       else
          {
          POSTMSG(ERROR,
		  "Argument '%s' needs a value",
		  cmdArgTags[ARG_DIAGDIR].c_str());
          args_ok = false;
          }

       }
    else if (string(argv[iArg]) == cmdArgTags[ARG_DEBUG_LVL])
       {

       if(iArg+1 < argc)
          {
          mDebugLevel = atoi(argv[++iArg]);

          // override corresponding TDRP parameter
          sprintf(paramStr, "debug_level = %d;", mDebugLevel);
          TDRP_add_override( &tdrpOverride, paramStr );
          }
       else
          {
          POSTMSG(ERROR,
		  "Argument '%s' needs a value",
		  cmdArgTags[ARG_DEBUG_LVL].c_str());
          args_ok = false;
          }

       }
    else if (string(argv[iArg]) == cmdArgTags[ARG_DIAG_LVL])
       {

       if(iArg+1 < argc)
          {
          mDiagOutLevel = atoi(argv[++iArg]);

          // override corresponding TDRP parameter
          sprintf(paramStr, "diag_level = %d;", mDiagOutLevel);
          TDRP_add_override( &tdrpOverride, paramStr );
          }
       else
          {
          POSTMSG(ERROR,
		  "Argument '%s' needs a value",
		  cmdArgTags[ARG_DIAG_LVL].c_str());
          args_ok = false;
          }

       }
    else if (string(argv[iArg]) == cmdArgTags[ARG_NCD_LVL])
       {

       if(iArg+1 < argc)
          {
          mNcdDiagOutLevel = atoi(argv[++iArg]);

          // override corresponding TDRP parameter
          sprintf(paramStr, "ncd_diag_level = %d;", mNcdDiagOutLevel);
          TDRP_add_override( &tdrpOverride, paramStr );
          }
       else
          {
          POSTMSG(ERROR,
		  "Argument '%s' needs a value",
		  cmdArgTags[ARG_NCD_LVL].c_str());
          args_ok = false;
          }

       }
    else if (string(argv[iArg]) == cmdArgTags[ARG_STARTTIME] ||
	     string(argv[iArg]) == "-start")
       {

       if(iArg+1 < argc)
          {
          mStartTimeStr.assign( argv[++iArg] );
          mStartTime = DateTime::parseDateTime( mStartTimeStr.c_str() );
          if ( mStartTime == DateTime::NEVER )
             {
             POSTMSG( ERROR, "Invalid -start_time specification.");
             args_ok = false;
             }
          else
             {
             start_time_found = true;
             }
          }
       else
          {
          POSTMSG(ERROR,
		  "Argument '%s' needs a value",
		  cmdArgTags[ARG_STARTTIME].c_str());
          args_ok = false;
          }

       }
    else if (string(argv[iArg]) == cmdArgTags[ARG_ENDTIME] ||
	     string(argv[iArg]) == "-end")
       {

       if(iArg+1 < argc)
          {
          mEndTimeStr.assign( argv[++iArg] );
          mEndTime = DateTime::parseDateTime( mEndTimeStr.c_str() );
          if ( mEndTime == DateTime::NEVER )
             {
             POSTMSG( ERROR, "Bad syntax in -end_time specification." );
             args_ok = false;
             }
          else
             {
             end_time_found = true;
             }
          }
       else
          {
          POSTMSG(ERROR,
		  "Argument '%s' needs a value",
		  cmdArgTags[ARG_ENDTIME].c_str());
          args_ok = false;
          }

       }
    else if (string(argv[iArg]) == "-params") // pass through to TDRP & save
       mParamsFileName.assign(argv[++iArg]);
    else if (string(argv[iArg]) == "-print_params") // pass through to TDRP
       continue;
    else if (string(argv[iArg]) == "-check_params") // pass through to TDRP
       continue;
    else
       {

       fprintf(stderr, "\nERROR: %s::%s:\n",
          CLASS_NAME.c_str(), METHOD_NAME.c_str());

       fprintf(stderr, "   Unknown argument '%s'\n", argv[iArg]);

       return CMDARG_UNKNOWN_ARG;
       }

    } // end for iArg

 //
 // Check for consistency 
 //

 if (start_time_found && !end_time_found)
    {
    POSTMSG( ERROR, "Found '-start_time' but not '-end_time'" );
    args_ok = false;
    }

 if (!start_time_found && end_time_found)
    {
    POSTMSG( ERROR, "Found '-end_time' but not '-start_time'" );
    args_ok = false;
    }

 if (start_time_found && end_time_found)
 {
    if( !file_list_found )
       {
       // override corresponding TDRP parameter
       // Force run mode ARCHIVE_START_END_TIMES when args "-start_time"
       // and "-end_time" are both present.
       TDRP_add_override(&tdrpOverride, "run_mode = ARCHIVE_START_END_TIMES;");
       }
    else
       {
       POSTMSG( ERROR, "Found '-end_time' & '-start_time' & '-file_list'" );
       POSTMSG( ERROR,
          "'-end_time'/'-start_time' & '-file_list' are mutually exclusive" );

       args_ok = false;
       } // end if !file_list_found
 }
 
 if( args_ok )
    return SUCCESS;
 else
    return FAILURE;

} //--- end ReadArgs

//----------------------------------------------------------------------------
// PrintArgs
//----------------------------------------------------------------------------

void
CmdLineArgs::PrintArgs(FILE *OUT)

{ //--- begin PrintArgs

 int i;
 const string METHOD_NAME = "PrintArgs";

 fprintf(OUT, "\n%s::%s:\n", CLASS_NAME.c_str(), METHOD_NAME.c_str());
 fprintf(OUT, "   Command line arguments read in:\n");

 fprintf(OUT, "   %16s = '%s'\n",
	 cmdArgTags[ARG_INSTANCE].c_str(), mInstance.c_str() );

 fprintf(OUT, "   %16s = %s\n",
    "-params", mParamsFileName.c_str());

 fprintf(OUT, "   %16s = '%s'\n",
	 cmdArgTags[ARG_INPUTDIR].c_str(), mInputDir.c_str());

 fprintf(OUT, "   %16s = '%s'\n",
	 cmdArgTags[ARG_OUTPUTDIR].c_str(), mOutputDir.c_str());

 fprintf(OUT, "   %16s = '%s'\n",
	 cmdArgTags[ARG_DIAGDIR].c_str(), mDiagDir.c_str());

 fprintf(OUT, "   %16s = %d\n",
	 cmdArgTags[ARG_DEBUG_LVL].c_str(), mDebugLevel);

 fprintf(OUT, "   %16s = %d\n",
	 cmdArgTags[ARG_DIAG_LVL].c_str(), mDiagOutLevel);

 fprintf(OUT, "   %16s = %d\n",
	 cmdArgTags[ARG_NCD_LVL].c_str(), mNcdDiagOutLevel);

 fprintf(OUT, "   %16s = '%s' (unix time:%ld)\n",
	 cmdArgTags[ARG_STARTTIME].c_str(), mStartTimeStr.c_str(), mStartTime);

 fprintf(OUT, "   %16s = '%s' (unix time:%ld)\n",
	 cmdArgTags[ARG_ENDTIME].c_str(), mEndTimeStr.c_str(), mEndTime);

 fprintf(OUT, "   %16s : ", cmdArgTags[ARG_INPFILELIST].c_str());
 fprintf(OUT, "(# of input files = %d)\n", (int) mInputFileList.size());

 for( i=0; i < (int) mInputFileList.size(); ++i)
    {
    fprintf(OUT, "      %s\n", mInputFileList[i].c_str());
    }

} //--- end PrintArgs

//----------------------------------------------------------------------------
// usage
//----------------------------------------------------------------------------

void CmdLineArgs::usage()
{
 //
 // New-style command lines
 //
 cerr << "\n\nUsage: " << PROGRAM_NAME   << " [options as below]\n"
         "       [ -h, -help ]            produce this list\n"
         "       [ -instance <name> ]     set the instance\n"
         "       [ -run_mode <mode> ]     run mode; choices are:\n"
         "                                   REALTIME\n"
         "                                   ARCHIVE_FILE_LIST or FILE_LIST\n"
         "                                   ARCHIVE_START_END_TIMES or TIME_LIST\n"
         "       [ -fname_type <type> ]   type of input file to be read;\n"
         "                                supported types are:\n"
         "                                   FNAME_RAL\n"
         "                                   FNAME_NASA\n"
         "                                   FNAME_NOAA\n"
         "       [ -file_list ]           list of input file paths\n"
         "       [ -input_dir ]           input file name\n"
         "       [ -output_dir ]          output file name\n"
         "       [ -diag_dir ]            diagnostic file name\n"
         "       [ -debug_level ]         produce verbose debug messages\n"
         "       [ -diag_level ]          enable diagnostic output\n"
         "       [ -ncd_diag_level ]      enable diagnostic output from NetcdfDataset\n"
         "\n"
         "       [ -params params_file ]  set parameter file name\n"
         "\n"
         "       In ARCHIVE_START_END_TIMES mode only:\n"
         "       [ -start_time \"yyyy/mm/dd hh:mm:ss\"]\n"
         "       [ -end_time   \"yyyy/mm/dd hh:mm:ss\"]\n"
         "\n"
      << endl;

 TDRP_usage( stderr );
}
