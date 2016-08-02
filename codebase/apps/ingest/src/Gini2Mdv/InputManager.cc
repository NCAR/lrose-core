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
//////////////////////////////////////////////////////////
//
// InputManager: Base class for handling multiple input sources.
//
// Steve Mueller (smueller@ucar.edu)
//
// Summer 2006
//
//////////////////////////////////////////////////////////

// C++ Standard Include Files
#include <sys/stat.h>

// RAP Include Files
#include <toolsa/pmu.h>
#include <toolsa/file_io.h>

// Local Include Files
#include "InputManager.hh"
#include "Gini2MdvUtilities.hh"

using namespace std;

// Static assignments
//  _maxValidAge and _newFileTimeout will be reassigned based on
//   values specified in the parameter file.
int    InputManager::_maxValidAge           = 1800;
time_t InputManager::_firstFileReceivedTime = 0;
int    InputManager::_newFileTimeout        = 300;
time_t InputManager::_runTime               = -1;

InputManager::InputManager(const bool debug)
   {
   setDebug(debug);
   } // End of InputManager constructor.


InputManager::~InputManager()
   {
   } // End of InputManager destructor.


// Static member function (cannot be inlined).
void InputManager::setMaxValidAge(int maxValidAge)
   {
   InputManager::_maxValidAge = maxValidAge;
   } // End of setMaxValidAge() method.


// Static member function (cannot be inlined).
void InputManager::setFirstFileReceivedTime(time_t fileReceivedTime)
   {
   InputManager::_firstFileReceivedTime = fileReceivedTime;
   } // End of setFirstFileReceivedTime() method.


// Static member function (cannot be inlined).
void InputManager::setNewFileTimeout(int newFileTimeout)
   {
   InputManager::_newFileTimeout = newFileTimeout;
   } // End of setNewFileTimeout() method.

// Static member function (cannot be inlined).
void InputManager::setRunTime(time_t runTime)
   {
   InputManager::_runTime = runTime;
   } // End of setRunTime() method.


// Static member function (cannot be inlined).
int InputManager::getMaxValidAge()
   {
   return InputManager::_maxValidAge;
   } // End of getMaxValidAge() method.


// Static member function (cannot be inlined).
time_t InputManager::getFirstFileReceivedTime()
   {
   return InputManager::_firstFileReceivedTime;
   } // End of getFirstFileReceivedTime() method.


// Static member function (cannot be inlined).
int InputManager::getNewFileTimeout()
   {
   return InputManager::_newFileTimeout;
   } // End of getNewFileTimeout() method.

// Static member function (cannot be inlined).
time_t InputManager::getRunTime()
   {
   return InputManager::_runTime;
   } // End of getRunTime() method.

// Static member function (cannot be inlined).
bool InputManager::checkForNewFile()
   {
     return true;
   // Stub for non-pure virtual function.
   } // End of checkForNewFile() method.

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  InputManager::getDataTimeFromFileName()                                   //
//                                                                            //
// Description:                                                               //
//  Static public method that determines the UNIX time associated with data   //
//  inside an input file by parsing the input file name.                      //
//                                                                            //
// Input:                                                                     //
//  filename - string representing input file name (e.g., VIS_20060519_1815). //
//                                                                            //
// Output:                                                                    //
//  Returns the UNIX time associated with the data as a time_t type.          //
//  Returns -1 for failure.                                                   //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
time_t InputManager::getDataTimeFromFileName(string filename, string fileTemplate)
   {
   const string methodName = "InputManager::getDataTimeFromFileName()";

   DateTime dateTime;

   time_t returnTime = dateTime.strpTime(fileTemplate, filename, true, true);

   return(returnTime);
   } // End of getDataTimeFromFileName() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  InputManager::directoryStatus()                                           //
//                                                                            //
// Description:                                                               //
//  Static public method that verifies the existence and the "stat"-ability   //
//  of a specified input directory.                                           //
//                                                                            //
// Input:                                                                     //
//  directory - string representing the directory path.                       //
//                                                                            //
// Output:                                                                    //
//  true if directory exists and has a successful stat() call.                //
//  false, otherwise.                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool InputManager::directoryStatus(string directory)
   {
   const string methodName = "InputManager::directoryStatus()";

   struct stat fileStat;
   int statReturn = ta_stat(directory.c_str(), &fileStat);

   // Verify that input directory can be "stat"-ed.
   if(statReturn != 0)
      {
      return(false);
      }

   // Verify that input string is actually a directory (i.e, not just a file).
   if(!S_ISDIR(fileStat.st_mode))
      {
      return(false);
      }

   return(true);
   } // End of directoryStatus() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  InputManager::resetInputProcessingFlags()                                 //
//                                                                            //
// Description:                                                               //
//  Resets the input processing flags to their initialization values.         //
//                                                                            //
// Input: None (Access to InputManager internal data members).                //
//                                                                            //
// Output:                                                                    //
//  None (Modifies internal data members).                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void InputManager::resetInputProcessingFlags()
   {
   const string methodName = "InputManager::resetInputProcessingFlags()";

   setWaitingForNewFile(false);
   setNewFilePendingProcessing(false);
   setTimedOutForNewFile(false);
   setProcessWithoutInput(false);

   InputManager::_firstFileReceivedTime = 0;
   } // End of resetInputProcessingFlags() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  InputManager::reinitializeInputManager()                                  //
//                                                                            //
// Description:                                                               //
//  Resets the data members to their initialization values.                   //
//                                                                            //
// Input: None (Access to InputManager internal data members).                //
//                                                                            //
// Output:                                                                    //
//  None (Modifies internal data members).                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void InputManager::reinitializeInputManager()
   {
   const string methodName = "InputManager::reinitializeInputManager()";

   setCurrentInputFile("");
   setCurrentFileCompletionTime(0);
   setCurrentDataTime(0);
   setWaitingForNewFile(true);
   setNewFilePendingProcessing(false);
   setTimedOutForNewFile(false);
   setProcessWithoutInput(false);

   setFirstFileReceivedTime(0);
   } // End of reinitializeInputManager() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  InputManager::waitForFileCompletion()                                     //
//                                                                            //
// Description:                                                               //
//  Public method to monitor a specified input file to determine that the     //
//  file has not been modified for a specific period of time.                 //
//                                                                            //
// Input: None                                                                //
//                                                                            //
// Notes:                                                                     //
//  Values for stableTimeLimitSecs and timeOutSecs are assigned in the header //
//     file for RealtimeDirInputManager.                                      //
//  Returns true if the stat parameters for the specified file do not change  //
//     for a period of stableTimeLimitSecs seconds.                           //
//  The value of timeOutSecs must always exceed that of stableTimeLimitSecs.  //
//                                                                            //
// Output:                                                                    //
//  Returns true if file satisfies completion criterion, false otherwise.     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool InputManager::waitForFileCompletion()
   {
   const string methodName = "InputManager::waitForFileCompletion()";

   string filePath = getCurrentInputFile();
   int stableTimeLimitSecs = STABLE_TIME_LIMIT_SECS;
   int timeOutSecs = TIME_OUT_SECS;

   // Verify that timeOutSecs exceeds stableTimeLimitSecs.
   if(stableTimeLimitSecs >= timeOutSecs)
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Input argument conflict - timeOutSecs must exceed stableTimeLimitSecs" << endl;
      exit(-1);
      }
  
   // Wait for the file to be complete

   int stableCounterSecs = 0;
   int timeCounterSecs   = 0;
   int oldFileBytes      = 0;
  
   while(true)
      {
      sleep(1);
      timeCounterSecs++;
      if(timeCounterSecs >= timeOutSecs)
         {
	         cerr << endl << methodName << ": " << filePath << " timed out." << endl;
	         return(false);
         }
    
      struct stat fileStat;
  
      if(stat(filePath.c_str(), &fileStat) != 0)
         {
         // File status is unavailable. Assume doesn't exist or corrupt.
         cout << methodName << ": Cannot stat directory " << filePath << endl;
         return(false);
         }
  
      if(fileStat.st_size == oldFileBytes)
         {
         stableCounterSecs++;
         }

      oldFileBytes = fileStat.st_size;

      if(stableCounterSecs >= stableTimeLimitSecs)
         {
	         if (timeCounterSecs > 0)
		         {
			         cerr << endl;
		         }
	         return(true);
         }
      if (timeCounterSecs == 1)
	      {
		      cerr << "Waiting for " << filePath << " to be stable.";
	      }
      else
	      {
		      cerr << ".";
	      }
      }
   } // End of waitForFileCompletion() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  InputManager::inputFileStatus()                                           //
//                                                                            //
// Description:                                                               //
//  Public method to check the status of an input file that is pending        //
//  processing.                                                               //
//                                                                            //
// Input: None (file checked is that returned by getCurrentInputFile()).      //
//                                                                            //
// Output:                                                                    //
//  Returns true if the input file exists and is non-zero length.             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool InputManager::inputFileStatus()
   {
   const string methodName = "InputManager::inputFileStatus()";

   struct stat fileStat;
   int statReturn = ta_stat(getCurrentInputFile().c_str(), &fileStat);

   if(statReturn != 0)
      {
      return(false);
      }
   else if(0 == fileStat.st_size)
      {
      return(false);
      }
   else
      {
      return(true);
      }
   } // End of inputFileStatus() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  InputManager::dumpInputManager()                                          //
//                                                                            //
// Description:                                                               //
//  Dumps the contents of an input manager for debugging.                     //
//                                                                            //
// Input: None (access to class data members).                                //
//                                                                            //
// Output:                                                                    //
//  None (Screen output).                                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void InputManager::dumpInputManager()
   {
   const string methodName = "InputManager::dumpInputManager()";

   time_t currentFileCompletionTime = getCurrentFileCompletionTime();
   string currentFileCompletionTimeStr = gini2MdvUtilities::formattedStrFromTimeT(currentFileCompletionTime);

   time_t currentDataTime = getCurrentDataTime();
   string currentDataTimeStr = gini2MdvUtilities::formattedStrFromTimeT(currentDataTime);

   cout << "************************************************************************" << endl;
   cout << "************************************************************************" << endl;
   cout << "Input Manager: " << getShortMdvFieldName() << endl;
   cout << " Constant Data Members" << endl;
   cout << "   baseDirectory: " << getBaseDirectory() << endl;
   cout << "   appendDateSubdirectory: " << boolalpha << getAppendDateSubDirectory() << noboolalpha << endl;
   cout << "   shortMdvFieldName: " << getShortMdvFieldName() << endl;
   cout << "   longMdvFieldName: " << getLongMdvFieldName() << endl;
   cout << "   fileTemplate: " << getFileTemplate() << endl;
   cout << "   calibrationCurveName: " << getCalibrationCurveName() << endl;
   cout << "   mandatoryDataStatus: " << boolalpha << getMandatoryDataStatus() << noboolalpha << endl;
   cout << " Variable Data Members" << endl;
   cout << "   currentInputDirectory: " << getCurrentInputDirectory() << endl;
   cout << "   currentInputFile: " << getCurrentInputFile() << endl;
   cout << "   currentFileCompletionTime: " << getCurrentFileCompletionTime() << "(" << currentFileCompletionTimeStr << ")" << endl;
   cout << "   currentDataTime: " << getCurrentDataTime() << "(" << currentDataTimeStr << ")" << endl;
   cout << "   waitingForNewFile: " << boolalpha << getWaitingForNewFile() << noboolalpha << endl;
   cout << "   newFilePendingProcessing: " << boolalpha << getNewFilePendingProcessing() << noboolalpha << endl;
   cout << "   timedOutForNewFile: " << boolalpha << getTimedOutForNewFile() << noboolalpha << endl;
   cout << "   processWithoutInput: " << boolalpha << getProcessWithoutInput() << noboolalpha << endl;
   cout << "************************************************************************" << endl;
   cout << "************************************************************************" << endl;
   cout << endl;
   cout << endl;
   } // End of dumpInputManager() method.
