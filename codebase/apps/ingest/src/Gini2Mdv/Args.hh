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
 *  $Id: Args.hh,v 1.7 2016/03/07 01:23:01 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	Args
// 
// Author:	G M Cunning
// 
// Date:	Wed Jun 21 11:53:03 2000
// 
// Description:	This class manages the argument list.
// 
// 
// 
// 

#ifndef    ARGS_H
#define    ARGS_H

// C++ Standard Include Files
#include <cstdio>
#include <ctime>
#include <string>
#include <vector>
#include <iostream>

// RAP Include Files
#include <tdrp/tdrp.h>

using namespace std;

class Args
   {
   public:
      // Flag indicating whether the current object status is okay.
      bool isOK;

      // TDRP overrides specified in the command line arguments.
      tdrp_override_t override;

      Args(int argc, char **argv, const string& prog_name);
      ~Args();

      ////////////////////
      // Access methods //
      ////////////////////

      const vector<string>& getInputFileList(void) const
         {
         return _inputFileList;
         }

      int getNbrInputFiles()
         {
         return _inputFileList.size();
         }

      const bool runOnce() const
         {
         return _runOnce;
         }

      const time_t getRunTime() const
         {
         return _runTime;
         }



   protected:
  
   private:
      vector<string> _inputFileList;
      string _errStr;

      static const string _className;

      bool _runOnce;

      time_t _runTime;

      string _paramsFilePath;

      // Disallow the copy constructor and assignment operator
      Args(const Args &);
      Args &operator=(const Args &);

      void _setOverride(const string&);
      void _setOverride(const string&, const char*);
      void _usage(const string &prog_name, ostream& out);
   };
# endif     /* ARGS_H */
