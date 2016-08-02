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
// InputManager: Base strategy class for handling input files.
//
// Nancy Rehak, RAP, NCAR, P.O. Box 3000, Boulder, CO 80307-3000, USA
//
// Sept 2002
//
/////////////////////////////////////////////////////////////

#ifndef _InputManager_
#define _InputManager_

// C++ Standard Include Files
#include <string>

// RAP Include Files
#include <didss/LdataInfo.hh>
#include <toolsa/DateTime.hh>

// No Local Include Files

// No Forward Declarations

using namespace std;

// Class Declaration
class InputManager
   {
   public:
      /////////////////////////
      // PUBLIC DATA MEMBERS //
      /////////////////////////
      static const int STABLE_TIME_LIMIT_SECS = 2;
      static const int TIME_OUT_SECS          = 30;

      //////////////////////////////
      // PUBLIC TYPE DECLARATIONS //
      //////////////////////////////
      typedef void (*heartbeat_t)(const char *label);
      typedef struct
         {
         // Parameters that will not change.
         string baseDirectory;
         bool   appendDateSubDirectory; // Flag indicating that date-based subdirectory should be appended to path.
         string shortMdvFieldName; // Short field name (e.g., 'IR').
         string longMdvFieldName; // Long field name (e.g., 'infrared').
         string fileTemplate; // String indicating file name data type (e.g., VIS in VIS_20060912_1215, etc.).
         string calibrationCurveName; // Calibration curve name (e.g., 'infrared').
	 bool   mandatoryDataStatus; // Flag indicating that current data set is mandatory for processing
	                             //  (i.e., cannot be replaced with "missing data" values).
         string exclusionStr; // String indicating that a file is to be excluded from selection (e.g., "gz").

         // Parameters that will change.
         string currentInputDirectory;
         string currentInputFile; // Note that this must include the path.
         time_t currentFileCompletionTime; // Time (UNIX) represented by file creation time.
         time_t currentDataTime; // Time (UNIX) represented by file name (should correspond to data time).
         bool   waitingForNewFile; // Flag indicating that the input manager is currently waiting for a new file.
         bool   newFilePendingProcessing; // Flag indicating that a new file is pending processing.
         bool   timedOutForNewFile; // Flag indicating that the input manager does not expect a new file to appear.
	 bool   processWithoutInput; // Flag indicating that output should be replaced with "missing data" values.
	                             //  Note that this field should never be 'true' when mandatoryDataStatus is also 'true'.
         } input_data_t;


      ////////////////////
      // PUBLIC METHODS //
      ////////////////////
 
      // Constructors and Destructors
      InputManager(const bool debug = false);
      virtual ~InputManager();

      // Static Methods (cannot be inlined)
      static void   setMaxValidAge(int maxValidAge);
      static void   setFirstFileReceivedTime(time_t fileTime);
      static void   setNewFileTimeout(int newFileTimeout);
      static void   setRunTime(time_t runTime);
      static int    getMaxValidAge();
      static time_t getFirstFileReceivedTime();
      static int    getNewFileTimeout();
      static time_t getRunTime(); 
      static time_t getDataTimeFromFileName(string fileName, string fileTemplate);
      static bool   directoryStatus(string directory);

      // Set Methods
      void setBaseDirectory(string baseDirectory)                 { _inputDataStruct.baseDirectory = baseDirectory; }
      void setAppendDateSubDirectory(bool appendSubDir)           { _inputDataStruct.appendDateSubDirectory = appendSubDir; }
      void setShortMdvFieldName(string shortMdvFieldName)         { _inputDataStruct.shortMdvFieldName = shortMdvFieldName; }
      void setLongMdvFieldName(string longMdvFieldName)           { _inputDataStruct.longMdvFieldName = longMdvFieldName; }
      void setFileTemplate(string fileTemplate)                   { _inputDataStruct.fileTemplate = fileTemplate; }
      void setCalibrationCurveName(string calibrationCurveName)   { _inputDataStruct.calibrationCurveName = calibrationCurveName; }
      void setMandatoryDataStatus(bool mandatoryDataStatus)       { _inputDataStruct.mandatoryDataStatus = mandatoryDataStatus; }
      void setExclusionStr(string exclusionStr)                   { _inputDataStruct.exclusionStr = exclusionStr; }
      void setCurrentInputDirectory(string currentInputDirectory) { _inputDataStruct.currentInputDirectory = currentInputDirectory; }
      void setCurrentInputFile(string currentInputFile)           { _inputDataStruct.currentInputFile = currentInputFile; }
      void setCurrentFileCompletionTime(time_t completionTime)    { _inputDataStruct.currentFileCompletionTime = completionTime; }
      void setCurrentDataTime(time_t currentDataTime)             { _inputDataStruct.currentDataTime = currentDataTime; }
      void setWaitingForNewFile(bool waitingForNewFile)           { _inputDataStruct.waitingForNewFile = waitingForNewFile; }
      void setNewFilePendingProcessing(bool newFilePending)       { _inputDataStruct.newFilePendingProcessing = newFilePending; }
      void setTimedOutForNewFile(bool timedOutForFile)            { _inputDataStruct.timedOutForNewFile = timedOutForFile; }
      void setProcessWithoutInput(bool processWithoutInput)       { _inputDataStruct.processWithoutInput = processWithoutInput; }
      void setRunOnce(bool runOnce)                               { _runOnce = runOnce; }
      void setHeartbeat(heartbeat_t heartbeatFunction)            { _heartbeatFunc = heartbeatFunction; }
      void setDebug(bool debug)                                   { _debug = debug; }

      // Get Methods
      heartbeat_t getHeartbeatFunction()    { return _heartbeatFunc; }
      bool   getDebug()                     { return _debug; }
      bool   getRunOnce()                   { return _runOnce; }
      string getBaseDirectory()             { return _inputDataStruct.baseDirectory; }
      bool   getAppendDateSubDirectory()    { return _inputDataStruct.appendDateSubDirectory; }
      string getShortMdvFieldName()         { return _inputDataStruct.shortMdvFieldName; }
      string getLongMdvFieldName()          { return _inputDataStruct.longMdvFieldName; }
      string getFileTemplate()              { return _inputDataStruct.fileTemplate; }
      string getCalibrationCurveName()      { return _inputDataStruct.calibrationCurveName; }
      bool   getMandatoryDataStatus()       { return _inputDataStruct.mandatoryDataStatus; }
      string getExclusionStr()              { return _inputDataStruct.exclusionStr; }
      string getCurrentInputDirectory()     { return _inputDataStruct.currentInputDirectory; }
      string getCurrentInputFile()          { return _inputDataStruct.currentInputFile; }
      time_t getCurrentFileCompletionTime() { return _inputDataStruct.currentFileCompletionTime; }
      time_t getCurrentDataTime()           { return _inputDataStruct.currentDataTime; }
      bool   getWaitingForNewFile()         { return _inputDataStruct.waitingForNewFile; }
      bool   getNewFilePendingProcessing()  { return _inputDataStruct.newFilePendingProcessing; }
      bool   getTimedOutForNewFile()        { return _inputDataStruct.timedOutForNewFile; }
      bool   getProcessWithoutInput()       { return _inputDataStruct.processWithoutInput; }

      // General Methods
      virtual bool checkForNewFile();
      void reinitializeInputManager();
      void resetInputProcessingFlags();
      bool waitForFileCompletion();
      bool inputFileStatus();
      void dumpInputManager();

   protected:
      ////////////////////////////
      // PROTECTED DATA MEMBERS //
      ////////////////////////////
      bool _debug;

      //////////////////////////
      // NO PROTECTED METHODS //
      //////////////////////////

   private:
      //////////////////////////
      // PRIVATE DATA MEMBERS //
      //////////////////////////
      // Static Data Members
      static int    _maxValidAge;           // Age (secs) beyond which a file is invalid
      static time_t _firstFileReceivedTime; // Time that first file currently pending processing was received.
      static int    _newFileTimeout;        // Amount of time (secs) to look for additional new files once first has been received.
      static time_t _runTime;               // the run time

      bool _runOnce;
      input_data_t _inputDataStruct;
      heartbeat_t _heartbeatFunc;

      ////////////////////////
      // NO PRIVATE METHODS //
      ////////////////////////
   };
#endif
