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
////////////////////////////////////////////////////////////////////////////////
// RealtimeDirInputManager: Class for handling realtime input files in a      //
//    directory with a latest_data_info file.                                 //
//                                                                            //
// Steve Mueller (Modified code by Nancy Rehak).                              //
//                                                                            //
// May 2006                                                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// Local Include Files
#include "LatestDataInputManager.hh"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                 CONSTRUCTOR                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
LatestDataInputManager::LatestDataInputManager(const string &data_dir, const int max_valid_age, heartbeat_t heartbeat_func, const bool debug)
   {
   setMaxValidAge(max_valid_age);
   setHeartbeat(heartbeat_func);
   _ldataInfo.setDir(data_dir);
   } // End of LatestDataInputManager constructor.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                  DESTRUCTOR                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
LatestDataInputManager::~LatestDataInputManager()
   {
   } // End of LatestDataInputManager destructor.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  LatestDataInputManager::checkForNewFile()                                 //
//                                                                            //
// Description:                                                               //
//  Monitors a specified input directory for latest data info updates.        //
//                                                                            //
// Input: None (Access to RealtimeDirInputManager internal data members).     //
//                                                                            //
// Output:                                                                    //
//  Sets the _currentInputFile private data member. Returns true for success, //
//  false for failure.                                                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool LatestDataInputManager::checkForNewFile()
   {
   const string methodName = "LatestDataInputManager::checkForNewFile()";
   // Wait for the next input file

   _ldataInfo.readBlocking(getMaxValidAge(), 1000, getHeartbeatFunction());
  
   setCurrentInputFile(_ldataInfo.getDataPath());
  
   if (_debug)
      {
      cerr << "---> Next input file: " << getCurrentInputFile() << endl;
      }
  
   return(true);
   } // End of checkForNewFile() method.
