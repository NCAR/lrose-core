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
//    "watched" directory.                                                    //
//                                                                            //
// Steve Mueller (Modified code by Nancy Rehak).                              //
//                                                                            //
// May 2006                                                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// C++ Standard Include Files 
#include <unistd.h>
#include <iostream>

// RAP Include Files
#include <sys/types.h>
#include <sys/stat.h>
#include <toolsa/DateTime.hh>

// Local Include Files
#include "RealtimeDirInputManager.hh"
#include "Gini2MdvUtilities.hh"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                 CONSTRUCTOR                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
RealtimeDirInputManager::RealtimeDirInputManager(InputManager::input_data_t inputDataStructure, bool runOnce, heartbeat_t heartbeat_func, const bool debug)
   {
   setHeartbeat(heartbeat_func);

   // Set InputManager data members that will not change.
   setBaseDirectory(inputDataStructure.baseDirectory);
   setAppendDateSubDirectory(inputDataStructure.appendDateSubDirectory);
   setShortMdvFieldName(inputDataStructure.shortMdvFieldName);
   setLongMdvFieldName(inputDataStructure.longMdvFieldName);
   setFileTemplate(inputDataStructure.fileTemplate);
   setCalibrationCurveName(inputDataStructure.calibrationCurveName);
   setMandatoryDataStatus(inputDataStructure.mandatoryDataStatus);
   setExclusionStr(inputDataStructure.exclusionStr);
   // Data InputManager data members that will change periodically.
   setCurrentInputFile("");
   setCurrentFileCompletionTime(0);
   setCurrentDataTime(0);
   setWaitingForNewFile(true);
   setNewFilePendingProcessing(false);
   setTimedOutForNewFile(false);
   setProcessWithoutInput(false);

   // Set RealtimeDirInputManager data members
   setRunOnce(runOnce);
   setRunTime(-1);
   setDebug(debug);

   // cerr << "I THINK I HAVE : " << getBaseDirectory() << endl;

   string inputDir;
   if(getAppendDateSubDirectory() == true)
      {

      DateTime dateTimeObj(time(0));
      _dateSubDir = dateTimeObj.getDateStrPlain();

      inputDir = getBaseDirectory() + "/" + _dateSubDir;

      // GMC DEVELOPMENT NOTE -- modify the time used to set date subdirectory
      // at startup. If application is started after day change over (0Z) and
      // new date directory doesn't exist. Roll back date sub-directory by one day
      if((dateTimeObj.getHour() == 0) && (InputManager::directoryStatus(inputDir) == false))
         {
	 dateTimeObj.set(time(0) - 3600);
	 _dateSubDir = dateTimeObj.getDateStrPlain();
	 inputDir = getBaseDirectory() + "/" + _dateSubDir;	   

         if(getDebug())
            {
            cout << "DEBUG: RealtimeDirInputManager::" << endl;
            cout << "   Roll back time is " << dateTimeObj.getStr() << endl;
            cout << "   Roll back one day to " << inputDir << endl;
            }
      }

      cerr << "RealtimeDirInputManager:: Input Directory is " << inputDir << endl;

      // If in -run_once mode, set last argument to false so we don't "look back".
      _inputDir = new InputDir(inputDir, "", getRunOnce(), getExclusionStr()); // NULL check at calling level
      }
   else
     {
       
       _baseInputDir = getBaseDirectory();

       if(getDebug())
	 {
	   cout << "Setting up for non-dated input directory " << _baseInputDir << endl;
	 }

       // cerr << "Setting up for non-dated input directory " << _baseInputDir << endl;

       _inputDir = new InputDir(_baseInputDir, "", getRunOnce(), getExclusionStr()); // NULL check at calling level
       inputDir  = getBaseDirectory();
     }

   setCurrentInputDirectory(inputDir);
   } // End of RealtimeDirInputManager constructor.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                  DESTRUCTOR                                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
RealtimeDirInputManager::~RealtimeDirInputManager()
   {
   } // End of RealtimeDirInputManager destructor.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  RealtimeDirInputManager::checkForNewFile()                                //
//                                                                            //
// Description:                                                               //
//  Public method to Monitor a specified input directory for the appearance   //
//  of a new data file. Sets the _currentInputFile private data member.       //
//                                                                            //
// Input: None (Access to RealtimeDirInputManager internal data members).     //
//                                                                            //
// Output:                                                                    //
//  Sets the _inputDataStruct.currentInputFile private data member.           //
//   Returns true if                                                          //
//    1) A new file is found.                                                 //
//    2) A file in the directory is pending processing.                       //
//    3) The monitored directory has timed out.                               //
//   Returns false otherwise.                                                 //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool RealtimeDirInputManager::checkForNewFile()
   {
   const string methodName = "RealtimeDirInputManager::checkForNewFile()";

   if(getNewFilePendingProcessing() || getTimedOutForNewFile())
      {
      // Either a new file is pending processing in this directory, or the monitoring
      //  period for the directory has expired. No need to evaluate.
      return true;
      }

   // Check for directory time out.
   //  Note - When processing has been completed and no files are pending processing,
   //  the static data member InputManager::_firstFileReceivedTime is initially set to
   //  zero. This renders the following conditional true only when at least one file is
   //  pending processing (no need to check for timeouts unless at least one file is
   //  pending processing).
   time_t now = time(0);

   if((now > (getFirstFileReceivedTime() + getNewFileTimeout())) && 
      (getFirstFileReceivedTime() != 0) && (!getRunOnce()))
      {
      // The monitoring period of the directory associated with the current input manager has
      //  expired. Set the appropriate flag.
      cout << methodName << ": Directory associated with " << getShortMdvFieldName() << 
	" has timed out without new input file" << endl;

      setTimedOutForNewFile(true);
      return true;
      }

   // Update date subdirectory (if necessary). The date subdirectory must be updated after midnight GMT.
   if(getAppendDateSubDirectory() == true) 
     {
       _updateDateSubDirectory();
     }

   // Determine if a new input data file is available.
   int maxValidAge = InputManager::getMaxValidAge();

   int fileCount = 0;
   char *newFile = 0;
   while((newFile = _inputDir->getNextFilename(1, maxValidAge)) != 0) // 0 is NULL
      {
      heartbeat_t _heartbeatFunctionPtr = getHeartbeatFunction();
      _heartbeatFunctionPtr("Checking for data");

      DateTime dateTimeObj(time(0));
      cout << "Found input file (at " << dateTimeObj.str() << "): " << newFile << endl;

      // Parse input file name to determine time represented by data. First must parse the path
      //  and name string to extract the file name.
      string newFileStr(newFile);
      size_t delimiterPosition = newFileStr.rfind('/');
      string fileName = newFileStr.substr(delimiterPosition+1);

      // If a file has already been found check to see if this is newer data
      // based on the file name - we were finding instances where more than one file were found and the
      // last in overwrites all data which causes no data to be processed if the last in was older data.
      // Potentially this could be solved by specifying a lower value for max_valid_age_secs in the parameter
      // file.  Note: this logic is flawed if more than two files are found within max_valid_age_secs
      // and the newest file is found last (e.g. t1, t0, t2)...
      if (fileCount > 0)
         {
         time_t prevFileDataTime = getCurrentDataTime();
         time_t curFileDataTime = getDataTimeFromFileName(fileName, getFileTemplate());
       
         if (curFileDataTime <= prevFileDataTime)
            {
            // New file is older. Do not allow the old data to overwrite the new.
            cout << "Input file: " << newFile << " is obsolete using previously found file" << endl;
	    return(true);
            }
         } 
        
      // Set inputDataStruct parameters.
      setCurrentInputFile(newFile);

      // Set processing status flags (these are used by the Gini2Mdv::beginProcessing() method).
      setNewFilePendingProcessing(true);
      setWaitingForNewFile(false);
      setTimedOutForNewFile(false);

      // Set the data time as determined above
      setCurrentDataTime(getDataTimeFromFileName(fileName, getFileTemplate()));


      // Determine the time that the file was written.
      struct stat fileStats;
      stat(newFile, &fileStats);
      time_t timeLastModified = fileStats.st_mtime;
      setCurrentFileCompletionTime(timeLastModified);
      if(getFirstFileReceivedTime() == 0)
         {
	   setFirstFileReceivedTime(time(0));
         }

      ++fileCount;
      } // End of while

   if(fileCount > 0)
     {
      // New file was found. Return true.
      return(true);
     }
   else
      {
      // New file was not found. Return false.
      return(false);
      }
   } // End of checkForNewFile() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  RealtimeDirInputManager::_updateDateSubDirectory()                        //
//                                                                            //
// Description:                                                               //
//  Private method that updates the input path with a new date subdirectory   //
//  when the date rolls over.                                                 //
//                                                                            //
// Input: None (Access to RealtimeDirInputManager internal data members).     //
//                                                                            //
// Output:                                                                    //
//  true for date change, false for no date change.                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool RealtimeDirInputManager::_updateDateSubDirectory()
   {
   const string methodName = "RealtimeDirInputManager::_updateDateSubDirectory()";

   DateTime dateTimeObj(time(0));
   string dateSubDir = dateTimeObj.getDateStrPlain();

   if(dateSubDir == _dateSubDir)
      {
      // Date has not changed.
      return(false);
      }
   else
      {
      // Date has changed.
      string inputDir = getBaseDirectory() + "/" + dateSubDir;
 
      // A new date-based subdirectory will not appear until a data file with
      //  the current date has been received. The existence of the new date-based
      //  directory must be verified before an attempt is made to instantiate a new
      //  InputDir object. If InputDir is initialized with a non-existent directory,
      //  it will fail and exit.
      if(InputManager::directoryStatus(inputDir))
         {
         // New directory exists.
         // Reassign relevant class data members and instantiate a new InputDir object.
         _dateSubDir = dateSubDir;

         // Debug output
         if(getDebug())
            {
            cout << "DEBUG: " << methodName << endl;
            cout << "   GMT Date Change Detected (Current time is " << dateTimeObj.getStr() << ")." << endl;
            cout << "   Updating RealtimeDirInputManager::_inputDir to " << inputDir << "." << endl;
            }

         // Set currentInputDirectory in inputDataStruct.
         setCurrentInputDirectory(inputDir);

         // Cleanup previous InputDir object.
         delete _inputDir;

         // Instantiate new InputDir object.
         //  Always set last argument to zero. When date has changed, we probably
         //  do not want to consider older files.
         // SMUELLER DEVELOPMENT NOTE - What about looking back at startup or run_once mode?
         _inputDir = new InputDir(inputDir.c_str(), "", true, getExclusionStr());
         if(0 == _inputDir) // 0 is NULL
            {
            cerr << "ERROR: " << methodName << endl;
            cerr << "   GMT Date change occurred, but could not instantiate a new InputDir object" << endl;
            cerr << "   EXITING" << endl;
            exit(-1);
            }

         return(true);
         }
      else
         {
         // New directory does not yet exist.

         // Debug output
         if(getDebug())
            {
            cout << "DEBUG: " << methodName << endl;
            cout << "   GMT Date Change Detected (Current time is " << dateTimeObj.getStr() << ")." << endl;
            cout << "   New date-based subdirectory (" << inputDir << ") does not yet exist." << endl;
            cout << "   Continuing to monitor previous date-based directory." << endl;
            }

         return(false);
         } // End of if(InputManager::directoryStatus(inputDir)) conditional.
      } // End of if(dateSubDir == _dateSubDir) conditional.
   } // End of _updateDateSubDirectory() method.
