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

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
/*
 *  $Id: Args.cc,v 1.13 2016/03/07 01:23:01 dixon Exp $
 *
 */

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////
//
// Class:	Args
//
// Author:	G. M. Cunning
//
// Date:	Wed Jun 21 11:53:03 2000
//
// Description: This class handles the command line arguments
//
//
//
//
//
//

// RAP Include Files
#include <toolsa/os_config.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/DateTime.hh>

// Local Include Files
#include "Args.hh"
#include "Params.hh"

using namespace std;

const string Args::_className = "Args";


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

Args::Args( int argc, char **argv, const string& prog_name ) : 
  isOK(true),
  _errStr("")
   {
   TDRP_init_override(&override);

   _runTime = -1;

   // search for command options
  
   for(int i = 1; i < argc; i++)
      {
      if(STRequal_exact(argv[i], "--") ||
         STRequal_exact(argv[i], "-h") ||
         STRequal_exact(argv[i], "-help") ||
         STRequal_exact(argv[i], "--help") ||
         STRequal_exact(argv[i], "-man"))
         {
         _usage(prog_name, cout);
         exit(0);
         }
      else if(STRequal_exact( argv[i], "-print_calibrations"))
         {
         }
      else if(STRequal_exact( argv[i], "-debug"))
         {
         _setOverride("debug = TRUE;");
         }
      else if(STRequal_exact( argv[i], "-run_once")) 
         {
         _runOnce = true;
         }
      else if(STRequal_exact( argv[i], "-run_time")) 
         {
	 _runTime = DateTime::parseDateTime(argv[i+1]);
	 if(DateTime::NEVER == _runTime)
            {
            cerr << "ERROR:" << endl;
            cerr << "   Invalid run time specified" << argv[i+1] << endl;
            cerr << "   EXITING" << endl;
            isOK = false;
            }
         _runOnce = true;
         _setOverride("mode", "ARCHIVE");
         }
      else if(STRequal_exact(argv[i], "-i"))
         {
         if (i < argc - 1)
            {
            _setOverride("instance", argv[i+1]);
            }
         else
            {
            cerr << _errStr;
            cerr << "User must provide instance name with -i option" << endl;
	
            isOK = false;
            }
         }
      else if(STRequal_exact(argv[i], "-out_url"))
         {
         if (i < argc - 1)
            {
            _setOverride("output_url", argv[i+1]);
            }
         else
            {
            cerr << _errStr;
            cerr << "User must provide instance name with -i option" << endl;
	
            isOK = false;
            }
         }
      } /* endfor - i */

   if(!isOK)
      {
      _usage(prog_name, cerr);
      }
    
   } // End of Args constructor.

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
Args::~Args()
   {
   TDRP_free_override(&override);
   }


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
// _usage() - Print the usage for this program.
//

void Args::_usage(const string& prog_name, ostream &out)
   {
   out << "Usage: " << prog_name << " [options as below]\n"
      << "options:\n"
      << "       [ --, -help, --help, -h, -man ] produce this list.\n"
      << "       [ -debug ] debugging on\n"
      << "       [-i instance_name] instance string (no blanks)\n"
      << "       [-out_url url] location for output\n"
      << "       [ -run_once] process most recent time only and exit\n"
      << "       [-run_time 'YYYY MM DD HH MM SS'] archive to run app.\n"
      << endl << endl;
   Params::usage(out);
   } // End of _usage() method.


//////////////////////////////////////////////////
// _setOverride
//
// this function sets TDRP overrides
//
void Args::_setOverride(const string& ov_str)
   {
   TDRP_add_override(&override, (char *)ov_str.c_str());
   } // End of _setOverride() method.


void Args::_setOverride(const string& ov_str, const char* val_str)
   {
   string tmp_str = ov_str + " = \"" + val_str + "\";";
   TDRP_add_override(&override, (char *)tmp_str.c_str());
   } // End of _setOverride() method.
