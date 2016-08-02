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
//
// ArchiveInputManager.hh
//
// Steve Mueller - August 2006
//
/////////////////////////////////////////////////////////////

#ifndef _ArchiveInputManager_
#define _ArchiveInputManager_

// C++ Standard Include Files
#include <set>
#include <string>
#include <vector>
#include <map>

// No RAP Include Files

// Local Include Files
#include "InputManager.hh"
#include "Params.hh"

// No Forward Declarations

using namespace std;

// Class Declaration
class ArchiveInputManager : public InputManager
   {
   public:
      /////////////////////////
      // PUBLIC DATA MEMBERS //
      /////////////////////////
      static const int UNIX_TIME_MILLENIUM = 946684800; // UNIX time for Jan 1, 2000 (Lower limit for Gini file age).

      ////////////////////
      // PUBLIC METHODS //
      ////////////////////

      // Constructors and Destructors
      ArchiveInputManager(InputManager::input_data_t inputDataStructure, time_t closestDataTimeUnix, heartbeat_t heartbeat_func = 0, const bool debug=false);
      virtual ~ArchiveInputManager();

      // Static methods
      static string getMinDateStr(time_t runTimeUnix, int maxValidSecs, Params::op_search_mode archiveSearchMode);
      static string getMaxDateStr(time_t runTimeUnix, int maxValidSecs, Params::op_search_mode archiveSearchMode);
      static void   getFilesAndUnixTimes(vector< pair<time_t, string> > *unixTimeFileNameVec, string directory, string fileTemplate);
      static time_t getClosestDataTime(time_t runTimeUnix, string mandatoryDirectory, int maxTime, Params::op_search_mode archiveSearchMode, 					       string fileTemplate, bool appendDateSubDirectory, bool debug);
      static string findMandatoryDataDirectory(map<string, InputManager::input_data_t> inputDataMap);
      static string findMandatoryFileTemplate(map<string, InputManager::input_data_t> inputDataMap);
      static bool findMandatoryAppendDateSubDirectory(map<string, InputManager::input_data_t> inputDataMap);

      // General Methods
      string buildFileNameFromUnixTime(time_t fileTimeUnix);

   protected:
      ///////////////////////////////
      // NO PROTECTED DATA MEMBERS //
      ///////////////////////////////

      //////////////////////////
      // NO PROTECTED METHODS //
      //////////////////////////
  
   private:
      /////////////////////////////
      // NO PRIVATE DATA MEMBERS //
      /////////////////////////////

     string _fileTemplate;

      ////////////////////////
      // NO PRIVATE METHODS //
      ////////////////////////
   };
#endif // _ArchiveInputManager_
