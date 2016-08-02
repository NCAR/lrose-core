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
// RealtimeDirInputManager.hh
//
// Steve Mueller - August 2006
//
/////////////////////////////////////////////////////////////

#ifndef _RealtimeDirInputManager_
#define _RealtimeDirInputManager_

// C++ Standard Include Files
#include <string>

// RAP Include Files
#include <toolsa/InputDir.hh>

// Local Include Files
#include "InputManager.hh"

// No Forward Declarations

using namespace std;

// Class Declaration
class RealtimeDirInputManager : public InputManager
   {
   public:
      ////////////////////////////
      // NO PUBLIC DATA MEMBERS //
      ////////////////////////////

      ////////////////////
      // PUBLIC METHODS //
      ////////////////////

      // Constructors and Destructors
      RealtimeDirInputManager(InputManager::input_data_t inputDataStructure, bool runOnce = false, heartbeat_t heartbeat_func = 0, const bool debug = false);
      virtual ~RealtimeDirInputManager();

      // Get Methods
      InputDir* getInputDir() { return _inputDir; }

      // General Methods
      bool checkForNewFile();

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
      InputDir *_inputDir;
      string _baseInputDir;
      string _dateSubDir;

      /////////////////////
      // PRIVATE METHODS //
      /////////////////////
      bool _updateDateSubDirectory();
   };
#endif // _RealtimeDirInputManager_
