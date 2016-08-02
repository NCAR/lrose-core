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

// C++ Standard Include Files
#include <cstdio>
#include <sys/stat.h>

// RAP Include Files
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <Mdv/MdvxField.hh>
#include <toolsa/umisc.h>
#include <toolsa/Path.hh>

// Local Include Files
#include "Gini2Mdv.hh"
#include "Args.hh"
#include "Params.hh"
#include "InputManager.hh"
#include "ArchiveInputManager.hh"
#include "LatestDataInputManager.hh"
#include "RealtimeDirInputManager.hh"
#include "GiniCalibrationCurve.hh"
#include "ProcessGiniFile.hh"
#include "OutputUrl.hh"
#include "Gini2MdvUtilities.hh"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method: Constructor                                                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
Gini2Mdv::Gini2Mdv(int argc, char **argv)
   {
    // set programe name
   Path pathParts(argv[0]);
   string progName = pathParts.getBase();

   ucopyright(const_cast<char*>(progName.c_str()));
   // Process the command-line arguments
   _args = new Args(argc, argv, "Gini2Mdv");
   if (!_args->isOK)
      {
      cerr << "ERROR: Gini2Mdv" << endl;
      cerr << "   Problem with command-line arguments" << endl;
      exit(-1);
      }

   // Get the TDRP parameters
   _params = new Params();
   char *paramsPath = const_cast<char *>(string("unknown").c_str());
   if (_params->loadFromArgs(argc, argv, _args->override.list, &paramsPath))
      {
      _errStr += "\tProblem with TDRP parameters.\n";
      _isOK = false;
      return;
      }

   init();
  
   _isOK = true;
   } // End of Gini2Mdv constructor.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method: Destructor                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
Gini2Mdv::~Gini2Mdv()
   {
   //
   // Unregister with process mapper
   //
   PMU_auto_unregister();

   delete _args;
   delete _params;
   delete _outputUrl;

   //   Delete [] _rawBuffer;
 
   //  Loop through _inputManagerMap and cleanup elements.
   map<string, InputManager*>::iterator inputManagerMapIterator;
   for(inputManagerMapIterator=_inputManagerMap.begin(); inputManagerMapIterator!=_inputManagerMap.end(); inputManagerMapIterator++)
      {
      if(0 != inputManagerMapIterator->second) // 0 is NULL
         {
         delete inputManagerMapIterator->second;
         inputManagerMapIterator->second = 0;
         }
      } // End of inputManagerMapIterator loop.

   // Loop through _calibrationCurveMap and cleanup elements.
   map<string, GiniCalibrationCurve*>::iterator calibrationCurveMapIterator;
   for(calibrationCurveMapIterator=_calibrationCurveMap.begin(); calibrationCurveMapIterator!=_calibrationCurveMap.end(); calibrationCurveMapIterator++)
      {
      if(0 != calibrationCurveMapIterator->second) // 0 is NULL
         {
         delete calibrationCurveMapIterator->second;
         calibrationCurveMapIterator->second = 0;
         }
      } // End of calibrationCurveMapIterator loop.
   }


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::init()                                                          //
//                                                                            //
// Description:                                                               //
//  Public method to initialize the Gini2Mdv object. This is where the input  //
//  manager maps are created and their contents instantiated.                 //
//                                                                            //
// Input: None                                                                //
//                                                                            //
// Output:                                                                    //
//  true for success, false for failure.                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool Gini2Mdv::init()
   {
   const string methodName = "Gini2Mdv::init()";

   // Check in.
   PMU_auto_init("Gini2Mdv", _params->instance, PROCMAP_REGISTER_INTERVAL);

   setDebug(_params->debug);

   // Create map of input data structures (InputManager::input_data_t structs).
   //  The elements of this map, which are derived from the parameter file, will
   //  be used to generate a map of input manager objects.
   setInputDataMap();

   // Instantiate calibration curve objects.
   buildCalibrationCurveMap();

   if(Params::ARCHIVE == _params->mode)
      {
      // ARCHIVE MODE
      if(_args->getRunTime() < 0)
      {
        cerr << "ERROR: " << methodName << endl;
        cerr << "   ARCHIVE mode requires a -run_time 'YYYY MM DD HH MM SS' argument." << endl;
        cerr << "   EXITING" << endl;
        exit(-1);
      }
  
      if(!_initArchiveMode(_args->getRunTime(), _params->archive_search_mode))
         {
         cerr << "ERROR: " << methodName << endl;
         cerr << "   _initArchiveMode() failure" << endl;
         cerr << "   EXITING" << endl;
         exit(-1);
         }
      }
   else if(Params::REALTIME == _params->mode)
      {
      // REALTIME mode. NOT SUPPORTED.
      cerr << "ERROR: " << methodName << endl;
      cerr << "   REALTIME input mode not yet operational." << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }
   else if(Params::REALTIME_DIR == _params->mode)
      {
      // REALTIME_DIR mode.
      if(!_initRealtimeDirMode())
         {
         cerr << "ERROR: " << methodName << endl;
         cerr << "   _initRealtimeDirMode() failure" << endl;
         cerr << "   EXITING" << endl;
         exit(-1);
         }
      }
   else
      {
      // Invalid input mode.
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Invalid input data mode (Options are ARCHIVE, REALTIME and REALTIME_DIR)" << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }

   return(true);
   } // End of init() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::_filePendingProcessing()                                        //
//                                                                            //
// Description:                                                               //
//  Loops through all input managers to determine if at least one has a file  //
//  that is pending processing.                                               //
//                                                                            //
// Input: None                                                                //
//                                                                            //
// Output:                                                                    //
//  true for if at least one file is pending processing, false otherwise.     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool Gini2Mdv::_filePendingProcessing()
   {
   const string methodName = "Gini2Mdv::_filePendingProcessing()";

   map<string, InputManager*>::iterator inputManagerMapIterator;
   for(inputManagerMapIterator=_inputManagerMap.begin(); inputManagerMapIterator!=_inputManagerMap.end(); inputManagerMapIterator++)
      {
      if(inputManagerMapIterator->second->getNewFilePendingProcessing())
         {
         return(true);
         }
      }
   return(false);
   } // End of _filePendingProcessing() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::_initArchiveMode()                                              //
//                                                                            //
// Description:                                                               //
//  Private method to initialize the Gini2Mdv object for the archive mode.    //
//   This is where the input manager maps are created and their contents      //
//   instantiated.                                                            //
//                                                                            //
// Input: None                                                                //
//                                                                            //
// Output:                                                                    //
//  true for success, false for failure.                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool Gini2Mdv::_initArchiveMode(time_t archive_time, Params::op_search_mode archive_search_mode)
   {
   const string methodName = "Gini2Mdv::_initArchiveMode()";

   // Local Declarations
   time_t runTimeUnix;

   // Assign parameter file-based static variables.
   InputManager::setMaxValidAge(_params->max_valid_age_secs);

   /////////////////////////////////////////////////////////////////////////
   // Select a mandatory data set to use in search for closest data time. //
   /////////////////////////////////////////////////////////////////////////
   // Note that the following will return a non-mandatory data directory if no
   //  mandatory data sets exist.
   string mandatoryBaseDirectory = ArchiveInputManager::findMandatoryDataDirectory(_inputDataMap);
   string mandatoryFileTemplate = ArchiveInputManager::findMandatoryFileTemplate(_inputDataMap);
   bool mandatoryAppendDateSubDirectory = ArchiveInputManager::findMandatoryAppendDateSubDirectory(_inputDataMap);
   //////////////////////////////////////////
   // Convert input run time to UNIX time. //
   //////////////////////////////////////////
   // Convert "YYYY MM DD HH MM SS" run_time input string to UNIX time.
   //   runTimeUnix = _args->getRunTime();
   runTimeUnix = archive_time;
   time_t closestDataTimeUnix = ArchiveInputManager::getClosestDataTime(runTimeUnix, mandatoryBaseDirectory, _params->max_valid_age_secs, archive_search_mode, mandatoryFileTemplate, mandatoryAppendDateSubDirectory, getDebug());

   // Loop through input data sources and instantiate an input manager for each.
   //  The constructor uses closestDataTimeUnix to set current directory (including date-based subdirectory) and current input file name.
   _inputManagerMap.clear();
   map<string, InputManager::input_data_t>::const_iterator inputDataMapIterator;
   for(inputDataMapIterator=_inputDataMap.begin(); inputDataMapIterator!=_inputDataMap.end(); inputDataMapIterator++)
      {
      // Instantiate an ArchiveInputManager for each data set.
      InputManager *inputManager = new ArchiveInputManager(inputDataMapIterator->second, closestDataTimeUnix,
                                                           (ArchiveInputManager::heartbeat_t)PMU_auto_register, getDebug());
      if(0 == inputManager) // 0 is NULL
         {
         cerr << "ERROR: " << methodName << endl;
         cerr << "   Could not instantiate an InputManager object for " << inputDataMapIterator->first << endl;
         cerr << "   Skipping this input data source" << endl;
         }
      else
         {
         // Add the ArchiveInputManager to the _inputManagerMap map.
         _inputManagerMap[(inputDataMapIterator->second).shortMdvFieldName] = inputManager;
         }
      } // End of inputDataMapIterator loop.

   return(true);
   } // End of _initArchiveMode() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::_initRealtimeDirMode()                                          //
//                                                                            //
// Description:                                                               //
//  Private method to initialize the Gini2Mdv object for the realtime         //
//   directory mode. This is where the input manager maps are created and     //
//   their contents instantiated.                                             //
//                                                                            //
// Input: None                                                                //
//                                                                            //
// Output:                                                                    //
//  true for success, false for failure.                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool Gini2Mdv::_initRealtimeDirMode()
   {
   const string methodName = "Gini2Mdv::_initRealtimeDirMode()";

   // Assign InputManager (abstract class) static variables.
   InputManager::setMaxValidAge(_params->max_valid_age_secs);
   InputManager::setNewFileTimeout(_params->new_file_timeout);

   // Loop through input data sources and instantiate an input manager for each.
   _inputManagerMap.clear();
   map<string, InputManager::input_data_t>::const_iterator inputDataMapIterator;
   for(inputDataMapIterator=_inputDataMap.begin(); inputDataMapIterator!=_inputDataMap.end(); inputDataMapIterator++)
      {
      // Instantiate a RealtimeDirInputManager.
      InputManager *inputManager = new RealtimeDirInputManager(inputDataMapIterator->second,
                                    _args->runOnce(),
                                    (RealtimeDirInputManager::heartbeat_t)PMU_auto_register,
                                    getDebug());
      if(0 == inputManager) // 0 is NULL
         {
         cerr << "ERROR: " << methodName << endl;
         cerr << "   Could not instantiate an InputManager object for " << inputDataMapIterator->first << endl;
         cerr << "   Skipping this input data source" << endl;
         continue;
         }

      // Add the RealtimeDirInputManager to _inputManagerMap map.
      _inputManagerMap[(inputDataMapIterator->second).shortMdvFieldName] = inputManager;

      // Verify that calibration parameters have been assigned for this input data set.
      string calibrationCurveName = (inputDataMapIterator->second).calibrationCurveName;
      string shortFieldName = (inputDataMapIterator->second).shortMdvFieldName;
      map<string, GiniCalibrationCurve*>::iterator calibrationCurveIter = _calibrationCurveMap.find(calibrationCurveName);
      if(calibrationCurveIter == _calibrationCurveMap.end())
         {
         cerr << "ERROR: " << methodName << endl;
         cerr << "   No calibration curve parameters assigned for " << shortFieldName << endl;
         cerr << "   EXITING" << endl;
         exit(-1);
         }
      } // End of inputDataMapIterator loop.

   return(true);
   } // End of _initRealtimeDirMode() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::run()                                                           //
//                                                                            //
// Description:                                                               //
//  Public method to run the Gini2Mdv object.                                 //
//                                                                            //
// Input: None                                                                //
//                                                                            //
// Output:                                                                    //
//  0 for success, -1 for failure.                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
int Gini2Mdv::run()
   {
   const string methodName = "Gini2Mdv::run()";
   PMU_auto_register("Waiting for data");

   // Instantiate an OutputUrl object.
   _outputUrl = new OutputUrl(_params);
   if(0 == _outputUrl) // 0 is NULL
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Could not instantiate OutputUrl object" << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }

   if(Params::ARCHIVE == _params->mode)
      {
      // ARCHIVE mode.
      if(!_runArchiveMode())
         {
         cerr << "ERROR: " << methodName << endl;
         cerr << "   _runArchiveMode() failure" << endl;
         cerr << "   EXITING" << endl;
         return -1;
         }
      }
   else if(Params::REALTIME == _params->mode)
      {
      // REALTIME mode. NOT SUPPORTED.
      cerr << "ERROR: " << methodName << endl;
      cerr << "   REALTIME input mode not yet implemented." << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }
   else if(Params::REALTIME_DIR == _params->mode)
      {
      // REALTIME_DIR mode.
      if(!_runRealtimeDirMode())
         {
         cerr << "ERROR: " << methodName << endl;
         cerr << "   _runRealtimeDirMode() failure" << endl;
         cerr << "   EXITING" << endl;
         return -1;
         }
      } // End of if(... == _params->mode) conditional.

   return 0;
   } // End of run() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::_runArchiveMode()                                               //
//                                                                            //
// Description:                                                               //
//  Private method to run Gini2Mdv in archive mode.                           //
//                                                                            //
// Input: None                                                                //
//                                                                            //
// Output:                                                                    //
//  true for success, false for failure.                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

bool Gini2Mdv::_runArchiveMode()
{
  //  time_t start_time =  _inputManagerMap.begin()->second->getCurrentDataTime();
  time_t start_time =  _args->getRunTime();
  time_t search_time = start_time;
  time_t earliest_time = start_time - _params->max_valid_age_secs;
  cerr << "Search limit is: " << gini2MdvUtilities::formattedStrFromTimeT(earliest_time) << endl;
  while(search_time > earliest_time)
  {
    cerr << "Attempting using: "
         << "  search_time = " 
         << gini2MdvUtilities::formattedStrFromTimeT(search_time) 
         << endl;
    if(_runArchiveModeOnce())
    {
      return true;
    }
    if(_params->run_archive_once)
    {
      return false;
    }
    search_time -= _params->lookback_step;
    _params->max_valid_age_secs -= _params->lookback_step;
    _initArchiveMode(search_time, Params::NEAREST);
  }
  cerr << "ERROR: Could not find a set of files to process" << endl;
  return false;
}

bool Gini2Mdv::_runArchiveModeOnce()
   {
   const string methodName = "Gini2Mdv::_runArchiveMode()";
   int validFileCount = 0;

   map<string, InputManager*>::const_iterator _inputManagerMapIterator;
   _outputUrl->clear();

   // Write Mdv master header.
   _outputUrl->setMdvMasterHeader();

   // Set times for master header.
   //  Data times should be synchronized, so use time from first element in _inputManagerMap.
   _outputUrl->setTimes(_inputManagerMap.begin()->second->getCurrentDataTime());

   // Loop through input data managers.
   for(_inputManagerMapIterator=_inputManagerMap.begin(); _inputManagerMapIterator!=_inputManagerMap.end(); _inputManagerMapIterator++)
      {
      ProcessGiniFile *processGiniFile;

      // Check existence of input Gini file before processing.
      if(_inputManagerMapIterator->second->inputFileStatus())
	      {
		      //check that input file is not still arriving
		      if (_params->wait_for_archive_data)
			      {
				      if(!_inputManagerMapIterator->second->waitForFileCompletion())
					      {
						      cerr << "WARNING:  " << _inputManagerMapIterator->second->getCurrentInputFile() << " was changing size, and I gave up waiting for it.\n";
						      return false;
					      }
			      }
         validFileCount++;
         cout << "Processing: " << _inputManagerMapIterator->second->getCurrentInputFile() << endl;
         }
      else
         {
         // Input file does not exist.
          if(_inputManagerMapIterator->second->getMandatoryDataStatus())
            {
            // Missing input file is mandatory.
            cerr << "WARNING: " << methodName << endl;
            cerr << "   Archive file " << _inputManagerMapIterator->second->getCurrentInputFile() << " does not exist or is empty." << endl; 
            cerr << "   Missing input file is mandatory." << endl;
            return false;
            }
         else
            {
            // Missing input file is not mandatory.
            //  Set processing flag to replace missing input data with missing value indicators.
            cerr << "WARNING: " << methodName << endl;
            cerr << "   Archive file " << _inputManagerMapIterator->second->getCurrentInputFile() << " does not exist or is empty." << endl; 
            cerr << "   Processing " << _inputManagerMapIterator->second->getShortMdvFieldName() << " without input." << endl;
            _inputManagerMapIterator->second->setProcessWithoutInput(true);
            }
         }
 
      // Open Gini input data file.
      string currentInputFile = _inputManagerMapIterator->second->getCurrentInputFile();
      if(!openGiniInputFile(_inputManagerMapIterator->second->getCurrentInputFile())) // Opens file and sets Gini2Mdv::_giniInputFilePtr.
         {
         // Input file cannot be opened.
         cerr << "WARNING: " << methodName << endl;
         cerr << "   Could not open Gini input file " << _inputManagerMapIterator->second->getCurrentInputFile() << "." << endl;
         if(_inputManagerMapIterator->second->getMandatoryDataStatus())
            {
            // Corrupt input file is mandatory.
            cerr << "   Corrupt input file is mandatory." << endl;
            return false;
            }
         else
            {
            // Corrupt input file is not mandatory.
            //  Set processing flag to replace unavailable input data with missing value indicators.
            cerr << "   Processing without input." << endl;
            _inputManagerMapIterator->second->setProcessWithoutInput(true);
            }
         }

      // Instantiate ProcessGiniFile object (necessary to process file).
      if(_inputManagerMapIterator->second->getProcessWithoutInput())
         {
         processGiniFile = new ProcessGiniFile(0, "", _inputManagerMapIterator->second->getShortMdvFieldName(),
                                               _calibrationCurveMap[_inputManagerMapIterator->second->getCalibrationCurveName()], _params);
         }
      else
         {
	   cout << "map name = " << _inputManagerMapIterator->second->getCalibrationCurveName() << endl;
         processGiniFile = new ProcessGiniFile(getGiniInputFilePtr(), _inputManagerMapIterator->second->getCurrentInputFile(),
                                               _inputManagerMapIterator->second->getShortMdvFieldName(),
                                                  _calibrationCurveMap[_inputManagerMapIterator->second->getCalibrationCurveName()], _params);
         }
      if(0 == processGiniFile) // 0 is NULL
         {
         cerr << "WARNING: " << methodName << endl;
         cerr << "   Could not instantiate ProcessGiniFile object for " <<  _inputManagerMapIterator->second->getCurrentInputFile() << "." << endl;
         cerr << "   Continuing to process remaining files." << endl;
         continue; // Skip to next input data manager.
         }

      string infoStr;
      if(_inputManagerMapIterator->second->getProcessWithoutInput())
         {
         // Process without input data (i.e., create data set of missing value indicators).
         //  First, resest procssing flag for next iteration.
         _inputManagerMapIterator->second->setProcessWithoutInput(false);

         // Fill missing data.
         processGiniFile->fillGiniData("visible", _params->sector_designator_for_data_fill);

         infoStr = "No input file for " + _inputManagerMapIterator->second->getShortMdvFieldName();
         }
      else
         {
         // Read and process input file.
         bool pgf_ret = processGiniFile->readGiniFile();
         fclose(getGiniInputFilePtr());
         if (!pgf_ret)
	         {
		         cerr << "processGiniFile->readGiniFile() returned false - giving up on this time.\n";
		         return false;		         		         
	         }

         infoStr = _inputManagerMapIterator->second->getCurrentInputFile();
         }

      _outputUrl->addToInfo(infoStr + string("\n"));

      // Create the Mdv field (including all headers, etc.).
      MdvxField *mdvxField = _outputUrl->buildMdvField(processGiniFile,
                                                       _inputManagerMapIterator->second,
                                                       _calibrationCurveMap[_inputManagerMapIterator->second->getCalibrationCurveName()],
                                                       _params);

      // Check MdvxField object
      if(0 == mdvxField)
         {
         cout << "WARNING: " << methodName << endl;
         cout << "MdvxField object not instantiated." << endl;
         return false;
         }

      // Add the Mdv field to the output Mdvx object.
      _outputUrl->addField(mdvxField);

      delete processGiniFile;

      } // End of _inputManagerMapIterator loop.

   if(0 == validFileCount)
      {
      cerr << "WARNING: " << methodName << endl;
      cerr << "   No valid input files for archive mode." << endl;
      return false;
      }

    _outputUrl->writeVol();

   return(true);
   } // End of _runArchiveMode() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::_runRealtimeDirMode()                                           //
//                                                                            //
// Description:                                                               //
//  Private method to run Gini2Mdv in realtime directory mode.                //
//                                                                            //
// Input: None                                                                //
//                                                                            //
// Output:                                                                    //
//  true for success, false for failure.                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool Gini2Mdv::_runRealtimeDirMode()
   {
   const string methodName = "Gini2Mdv::_runRealtimeDirMode()";

   map<string, InputManager*>::const_iterator _inputManagerMapIterator;
   int numSleeps = 0;
   int maxNumSleeps = (_params->new_file_timeout/_params->sleep_before_repeating_directory_search);

   if(_args->runOnce())
      {
      // Run-once mode
      InputManager::setRunTime(time(0));
      }

   while(true)
      {
      if(beginProcessing() == false)
         {
         _weReallyHaveToGetGoing = false;

         // Flags indicate that processing is not ready to begin. Continue to monitor input managers
         //  for new files.
         // Loop through input managers.
         for(_inputManagerMapIterator=_inputManagerMap.begin(); _inputManagerMapIterator!=_inputManagerMap.end(); _inputManagerMapIterator++)
            {
            // Determine if the directory associated with the current input manager has
            //  a new file or has "timed out" before receiving a new file.
            // If the directory has timed out, the timedOutForNewFile data member is set to true.
            // If a new file is found, the currentInputFile data member is set to the new file
            //  (including path), and the newFilePendingProcessing data member is set to true.
            _inputManagerMapIterator->second->checkForNewFile();
            }
         }
      else
         {
         // Flags indicate that processing should begin.

         _outputUrl->clear();
         numSleeps = 0;

         // Write Mdv master header.
         _outputUrl->setMdvMasterHeader();

         // Set times for master header.
         //  Data times should be synchronized, so use time from first mandatory element in _inputManagerMap.
         // Loop through input managers.
         for(_inputManagerMapIterator=_inputManagerMap.begin(); _inputManagerMapIterator!=_inputManagerMap.end(); _inputManagerMapIterator++)
            {
            if(_inputManagerMapIterator->second->getMandatoryDataStatus())
               {
               _outputUrl->setTimes(_inputManagerMapIterator->second->getCurrentDataTime());
               break;
               }
            }

         // Loop through input data managers.
         for(_inputManagerMapIterator=_inputManagerMap.begin(); _inputManagerMapIterator!=_inputManagerMap.end(); _inputManagerMapIterator++)
            {
            // Determine if input file has stabilized (finished writing). If not,
            //  check mandatory data status and assign processWithoutInput accordingly.
            if(_inputManagerMapIterator->second->waitForFileCompletion())
               {
               // Input file is stable (ready to be processed). No need to proceed without input.
               _inputManagerMapIterator->second->setProcessWithoutInput(false);
               }
            else
               {
               // Input file is unstable (indicating a possible I/O problem).
               //  Determine if it is permissible to proceed in the absence of an input file.
               if(_inputManagerMapIterator->second->getMandatoryDataStatus())
                  {
                  // Data set is mandatory. Cannot proceed without input file.
                  _inputManagerMapIterator->second->setProcessWithoutInput(false);
                  cerr << "ERROR: " << methodName << endl;
                  cerr << "   Mandatory input data set is unavailable - " << _inputManagerMapIterator->second->getShortMdvFieldName() << endl;
                  if(_args->runOnce())
	                  {
		                  // Run once mode - exit.
		                  cerr << "   EXITING" << endl;
		                  exit(-1);
	                  }
                  else
	                  {
		                  // Continuous mode - reset data managers, break out of input manager loop, and continue.
		                  _reinitializeAllDataManagers(true);
		                  break;
	                  }
                  }
               else
                  {
                  // Data set is not mandatory. May proceed without input file.
                  _inputManagerMapIterator->second->setProcessWithoutInput(true);
                  }
               } // End of _inputManagerMapIterator loop.

            string shortFieldName = _inputManagerMapIterator->second->getShortMdvFieldName();
            string calibrationCurveName = _inputManagerMapIterator->second->getCalibrationCurveName();
            ProcessGiniFile *processGiniFile;

            // Determine if processing should begin without input.
            if(_inputManagerMapIterator->second->getProcessWithoutInput())
               {
               // Processing of this data manager should proceed in the absence of an input data file.
               //  Generate output fields consisting entirely of "missing data" values.
               cout << "Processing " << _inputManagerMapIterator->second->getShortMdvFieldName() << " without input" << endl;
               processGiniFile = new ProcessGiniFile(0, "", shortFieldName, _calibrationCurveMap[calibrationCurveName], _params);
               if(0 == processGiniFile) // 0 is NULL
                  {
                  cerr << "ERROR: " << methodName << endl;
                  cerr << "   Could not instantiate ProcessGiniFile object for fillGiniData() method" << endl;
                  cerr << "   EXITING" << endl;
                  exit(-1);
                  }

               processGiniFile->fillGiniData("visible", _params->sector_designator_for_data_fill);

               string infoStr = "No input file for " + shortFieldName;
               _outputUrl->addToInfo(infoStr + string("\n"));

               // Reset processWithoutInput flag.
               _inputManagerMapIterator->second->setProcessWithoutInput(false);
               }
            else
               {
               // The input file associated with the current input manager is valid.
               string giniInputFileWithPath = _inputManagerMapIterator->second->getCurrentInputFile();
               cout << "Processing: " << giniInputFileWithPath << endl;

               // Verify that input file is not empty before processing.
               if(!_inputManagerMapIterator->second->inputFileStatus())
                  {
                  cerr << "ERROR: " << methodName << endl;
                  cerr << "   " << _inputManagerMapIterator->second->getCurrentInputFile() << " does not exist or is empty." << endl;
                  if(_inputManagerMapIterator->second->getMandatoryDataStatus())
                     {
                     cerr << "   This input file is mandatory." << endl;
                     cerr << "   Clearing output object and reinitializing all input manager processing flags." << endl;
                     _outputUrl->clear();
                     _reinitializeAllDataManagers(true);
                     break; // Break out of _inputManagerMapIterator loop.
                     }
                  }

               // Open Gini input data file.
               openGiniInputFile(giniInputFileWithPath); // Opens file and sets Gini2Mdv::_giniInputFilePtr to the returned pointer.

               // Instantiate ProcessGiniFile object (necessary to process file).
               processGiniFile = new ProcessGiniFile(getGiniInputFilePtr(), giniInputFileWithPath, shortFieldName,
							_calibrationCurveMap[calibrationCurveName], _params);
               if(0 == processGiniFile)
                  {
                  cerr << "ERROR: " << methodName << endl;
                  cerr << "   Could not instantiate ProcessGiniFile object for readGiniFile() method" << endl;
                  cerr << "   EXITING" << endl;
                  exit(-1);
                  }

               // Read and process input file.
               bool pgf_ret = processGiniFile->readGiniFile();

               // GMC NOTE -- Why not close file in readGiniFile() method?
               //  The readGiniFile() method should return a status of success or failure.
               //  Don't want to continue on failure.
               fclose(getGiniInputFilePtr());

               if (!pgf_ret)
	               {		              
		               cerr << "   This input file is incomplete." << endl;
		               cerr << "   Clearing output object and reinitializing all input manager processing flags." << endl;
		               _outputUrl->clear();
		               _reinitializeAllDataManagers(true);
		               break; // Break out of _inputManagerMapIterator loop. continue;
	               }

               _outputUrl->addToInfo(giniInputFileWithPath + string("\n"));
               } // End of if(_inputManagerMapIterator->second->getProcessWithoutInput()) conditional.

            // Create the Mdv field (including all headers, etc.).
            MdvxField *mdvxField = _outputUrl->buildMdvField(processGiniFile, _inputManagerMapIterator->second,
			                                           _calibrationCurveMap[calibrationCurveName], _params);
            if(0 == mdvxField) // 0 is NULL
               {
               cerr << "ERROR: " << methodName << endl;
               cerr << "MdvxField object not instantiated." << endl;
               cerr << "   EXITING" << endl;
               exit(-1);
               }

            // Add the Mdv field to the output Mdvx object.
            _outputUrl->addField(mdvxField);


            // Reset the processing flags for this input data manager.
            //  This will force the application into the first (file waiting) conditional on the next iteration.
            _inputManagerMapIterator->second->resetInputProcessingFlags();

            delete processGiniFile;
            } // End of _inputManagerMapIterator loop.

         _outputUrl->writeVol();

	 _weReallyHaveToGetGoing = false;

         if(_args->runOnce()) 
           {
           return(true);
           }    

	 _reinitializeAllDataManagers(false);

         } // End of if(!beginProcessing()) conditional.

      if(_params->mode == Params::REALTIME || _params->mode == Params::REALTIME_DIR)
         {
         if(getDebug())
            {
            DateTime currentTime = time(0);
            cerr << "Watching for data: " << currentTime.str() << endl;
            }
         PMU_auto_register("Waiting for data");
         sleep(_params->sleep_before_repeating_directory_search);
         ++numSleeps;
	 if(_filePendingProcessing() && _args->runOnce())
            {
            if(numSleeps >= maxNumSleeps)
               {
               _weReallyHaveToGetGoing = true;
               if(!_forcedProcessCheck())
                  {
                  // Cannot begin processing. Problematic input file is mandatory.
                  cerr << "ERROR: " << methodName << endl;
                  cerr << "   Maximum number sleeps exceeded and _forcedProcessCheck() failed." << endl;
                  cerr << "   EXITING" << endl;
                  exit(-1);
                  }
               }
            }
         }
      } // End of while(true) loop.

   return(true);
   } // End of _runRealtimeDirMode() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::beginProcessing()                                               //
//                                                                            //
// Description:                                                               //
//  Public method to determine if all data sets have been accounted for and   //
//  processing should be initiated.                                           //
//                                                                            //
// Input: None                                                                //
//                                                                            //
// Output:                                                                    //
//  true if ready for processing, false otherwise.                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool Gini2Mdv::beginProcessing()
   {
   const string methodName = "Gini2Mdv::beginProcessing()";

   // GMC DEVELOPMENT NOTE -- something is really convoluted in this logic. At some point
   // the input manager has process the files available. We need a short circuit
   if(_weReallyHaveToGetGoing)
     {
     return(true);
     }

   //   time_t latestDataTime = time(0);
   time_t latestDataTime = _inputManagerMap.begin()->second->getCurrentDataTime();
   map<string, InputManager*>::const_iterator inputManagerMapIterator;

   // Loop through input managers. Unless the directory associated with an input manager
   //  has a new file pending processing or has not timed out, return false. When an input
   //  manager has a new file pending processing, compare the data time to the last assigned
   //  value of latestDataTime. Reassign latestDataTime if necessary.


   // DEVELOPMENT NOTE: GMC
   // added pending file count and a delta time test to check for a complete set of files
   // if the there are enough files and they are from the same scan, then process them.
   size_t filePendingCount = 0;
   time_t deltaT = 0;
   for(inputManagerMapIterator=_inputManagerMap.begin(); inputManagerMapIterator!=_inputManagerMap.end(); inputManagerMapIterator++)
      {
      if(inputManagerMapIterator->second->getNewFilePendingProcessing() || inputManagerMapIterator->second->getTimedOutForNewFile())
         {
         // Relevant directory either has a file pending or has timed out.
         if(inputManagerMapIterator->second->getNewFilePendingProcessing())
            {
	      ++filePendingCount;
            // New file is pending processing. Check the associated data time and, if
	    //  appropriate, reassign latestDataTime. Continue to check remaining directories.
            time_t currentDataTime = inputManagerMapIterator->second->getCurrentDataTime();
	    deltaT += currentDataTime - latestDataTime;
            if(currentDataTime > latestDataTime)
               {
               latestDataTime = currentDataTime;
               }
            }
         continue;
         }
      else
         {
         // No file pending processing in a directory that has not time out.
         //  Processing should not begin.
         return(false);
         }
      } // End of (1st) inputManagerMapIterator loop.


   cout << "the pending file count is " << filePendingCount << endl;
   cout << "deltaT is " <<  deltaT << endl;
   if( (filePendingCount == _inputManagerMap.size()) && (deltaT == 0))
     {
     cout << "the pending file count (" << filePendingCount << ") equals the container size (" << _inputManagerMap.size() << ")" << endl;
     cout << "Let's process some files!" << endl;
     return(true);
     }

   // None of the input managers are still waiting for data (otherwise, the above
   //  iteration would have returned false and exited).
   // Verify the following
   //  1) If an input directory has timed out, is it permissible to replace the output
   //     that would have been derived from the input file with a "missing value" data
   //     set?
   //  2) Do all the input files that are pending processing correspond to the same
   //     data time?
   for(inputManagerMapIterator=_inputManagerMap.begin(); inputManagerMapIterator!=_inputManagerMap.end(); inputManagerMapIterator++)
      {
      if(inputManagerMapIterator->second->getTimedOutForNewFile())
         {
         // Directory associated with this input manager has timed out.
         //  Determine whether or not the associated data set is non-mandatory (i.e., can be replaced with "missing data" values).
         if(inputManagerMapIterator->second->getMandatoryDataStatus())
            {
            // The status of this input directory is mandatory. It is "not allowed" to time out.
            //  Reset ALL processing flags for ALL input data managers.
            cout << inputManagerMapIterator->second->getShortMdvFieldName() << " has timed out." << endl
                 << "   This data set is mandatory. Processing will not proceed. Resetting ALL processing flags for ALL input managers." << endl;
            _reinitializeAllDataManagers(true);
            return(false);
            }
         else
            {
            // The data represented by this input file is NOT mandatory. May replace data with
	    //  "missing data" values. Set processWithoutInput flag to indicate that this should occur.
            cout << inputManagerMapIterator->second->getShortMdvFieldName() << " has timed out." << endl
                 << "   The corresponding output will be replaced with 'missing data' values." << endl;
            inputManagerMapIterator->second->setProcessWithoutInput(true);
            }
         }
      else if(inputManagerMapIterator->second->getNewFilePendingProcessing())
         {
         // Directory associated with this input manager has an input file that is
         //  pending processing. Verify that the data time is consistent with latestDataTime
	 //  (i.e., the file is not associated with "old" data).
         if(latestDataTime != inputManagerMapIterator->second->getCurrentDataTime())
            {
            // The input file does not correspond to the latest data time available.
            //  Reinitialize the input data manager. This will force it to look for the next file.
            inputManagerMapIterator->second->reinitializeInputManager();
               cout << inputManagerMapIterator->second->getShortMdvFieldName()
                    << "(" << inputManagerMapIterator->second->getCurrentInputFile()
                    << ") is not consistent with latest data time ("
                    << gini2MdvUtilities::formattedStrFromTimeT(latestDataTime) << ")" << endl
                    << "   Resetting processing flags for the corresponding data manager." << endl;
            return(false);
            }
         }
      else
         {
         // Error. Should not get here.
         cerr << "ERROR: " << methodName << endl;
         cerr << "   " << inputManagerMapIterator->second->getShortMdvFieldName() << " has inconsistent processing flags." << endl;
         }
      } // End of (2nd) inputManagerMapIterator loop.

   // Haven't returned with false, indicating that all directories that have timed out are
   //  "allowed" to do so, and that all files pending processing are associated with the same
   //  data time.
   return(true);
   } // End of beginProcessing() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::setInputDataMap()                                               //
//                                                                            //
// Description:                                                               //
//  Public method to create a series of InputManager::input_data_t structures //
//  used by the input data managers. These are added to the private data      //
//  member Gini2Mdv::_inputDataMap.                                           //
//                                                                            //
// Input: None (access to class data members).                                //
//                                                                            //
// Output:                                                                    //
//  Returns true for success, false otherwise. Populates the private data     //
//  member Gini2Mdv::_inputDataMap.                                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool Gini2Mdv::setInputDataMap()
   {
   // Create an STL map of InputManager::input_data_t structures and of calibration files (including paths).
   //  Use short Mdv field names as keys for both. The first map will be copied into the associated input
   //  managers. The second will be used to set the calibration file in the ProcessGiniFiles class.
   const string methodName = "Gini2Mdv::setInputDataMap()";

   _inputDataMap.clear();
   InputManager::input_data_t inputDataStruct;
   for(int inputDataIndex=0; inputDataIndex<_params->input_data_array_n; inputDataIndex++)
      {
      // Assign parameters that should not change.
      inputDataStruct.baseDirectory             = _params->_input_data_array[inputDataIndex].base_directory;
      inputDataStruct.appendDateSubDirectory    = _params->append_date_subdirectory;
      inputDataStruct.shortMdvFieldName         = _params->_input_data_array[inputDataIndex].short_mdv_field_name;
      inputDataStruct.longMdvFieldName          = _params->_input_data_array[inputDataIndex].long_mdv_field_name;
      inputDataStruct.fileTemplate            = _params->_input_data_array[inputDataIndex].file_template;
      inputDataStruct.calibrationCurveName      = _params->_input_data_array[inputDataIndex].calibration_curve_name;      
      inputDataStruct.mandatoryDataStatus       = _params->_input_data_array[inputDataIndex].mandatory_data_status;
      inputDataStruct.exclusionStr              = _params->_input_data_array[inputDataIndex].exclusion_str;

      // Initialize parameters that should change.
      inputDataStruct.currentInputDirectory     = "";
      inputDataStruct.currentInputFile          = "";
      inputDataStruct.currentFileCompletionTime = 0;
      inputDataStruct.currentDataTime           = 0;
      inputDataStruct.waitingForNewFile         = true;
      inputDataStruct.newFilePendingProcessing  = false;
      inputDataStruct.timedOutForNewFile        = false;

      //  Verify that base directory is valid.
      // If it is not, it could be that is simply does
      // not yet exist. This is often the case if the directory
      // is made by the LDM and the LDM has just started running.
      // So in that event, loop, registering with procmap, until
      // the directory is valid.

      if(!InputManager::directoryStatus(inputDataStruct.baseDirectory))
	{
	  do {
	    cerr << "WARNING : " << methodName << endl;
	    cerr << "   Invalid base directory for " << inputDataStruct.shortMdvFieldName << endl;
	    cerr << "   Waiting for 5 seconds before re-checking..." << endl;
	    cerr << "   inputDataStruct.baseDirectory: " << inputDataStruct.baseDirectory << endl;
	    PMU_auto_register("Waiting for input directory");
	    sleep(5);
	  } while (!InputManager::directoryStatus(inputDataStruct.baseDirectory));
	}
      _inputDataMap[_params->_input_data_array[inputDataIndex].short_mdv_field_name] = inputDataStruct;
      }

   return(true);
   } // End of setInputDataMap() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::buildCalibrationCurveMap()                                      //
//                                                                            //
// Description:                                                               //
//  Public method to instantiate a series of GiniCalibrationCurve objects,    //
//  are stored in the private data member Gini2Mdv::_calibrationCurveMap.     //
//                                                                            //
// Input: None (access to class data members).                                //
//                                                                            //
// Output:                                                                    //
//  Returns true if successful, false otherwise. Populates the private data   //
//  member Gini2Mdv::_calibrationCurveMap.                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool Gini2Mdv::buildCalibrationCurveMap()
   {
   const string methodName = "Gini2Mdv::buildCalibrationCurveMap()";

   _calibrationCurveMap.clear();
   for(int index=0; index < _params->calibration_data_n; index++)
      {
      Params::calibration_t paramsCalibrationData = (_params->_calibration_data)[index];
      _calibrationCurveMap[_params->_calibration_data[index].name] = new GiniCalibrationCurve((_params->_calibration_data)[index]);
      if(0 == _calibrationCurveMap[_params->_calibration_data[index].name]) // 0 is NULL
         {
         cerr << "ERROR: " << methodName << endl;
         cerr << "   Could not allocate _calibrationCurveMap memory for " << _params->_calibration_data[index].name << endl;
         cerr << "   EXITING" << endl;
	 exit(-1);
         }

      if(_params->calibration_log_file[0] != '\0')
         {
         // Output calibration curve information to log file.
         GiniCalibrationCurve::calibration_data_t calibrationDataStruct = (_calibrationCurveMap[_params->_calibration_data[index].name])->buildCalibrationDataStruct(paramsCalibrationData);
         (_calibrationCurveMap[_params->_calibration_data[index].name])->printCalibrationDataStruct(calibrationDataStruct, "calibration.log");
         (_calibrationCurveMap[_params->_calibration_data[index].name])->printCalibrationCurve("calibration.log");
         }
      }

   return(true);
   } // End of buildCalibrationCurveMap() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::openGiniInputFile()                                             //
//                                                                            //
// Description:                                                               //
//  Public method that opens the Gini input data file and sets the file       //
//  handle to the private data member Gini2Mdv::_giniInputFilePtr to the      //
//  returned pointer.                                                         //
//                                                                            //
// Input: None (access to class data members).                                //
//                                                                            //
// Output:                                                                    //
//  Returns true if successful, false otherwise. Assigns the private data     //
//  member Gini2Mdv::_giniInputFilePtr.                                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool Gini2Mdv::openGiniInputFile(string fileWithPath)
   {
   const string methodName = "Gini2Mdv::openGiniInputFile()";

   _giniInputFilePtr = fopen(fileWithPath.c_str(), "rb");
   if (0 == _giniInputFilePtr) // 0 is NULL
      {
      cerr << "WARNING: " << methodName << endl;
      cerr << "   " << fileWithPath << " not found." << endl;
      return(false);
      }
   else
      {
      return(true);
      }
   } // End of openGiniInputFile() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::_reinitializeAllDataManagers()                                  //
//                                                                            //
// Description:                                                               //
//  Private method to reset each input data manager to initialization values. //
//                                                                            //
// Input:                                                                     //
//  rewindDirFlag- Boolean flag that indicates if directory is to be rewound. //
//                                                                            //
// Output:                                                                    //
//  None (Modifies internal data members).                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void Gini2Mdv::_reinitializeAllDataManagers(bool rewindDirFlag)
   {
   const string methodName = "Gini2Mdv::_reinitializeAllDataManagers()";

   map<string, InputManager*>::iterator inputManagerMapIterator;
   for(inputManagerMapIterator=_inputManagerMap.begin(); inputManagerMapIterator!=_inputManagerMap.end(); inputManagerMapIterator++)
      {
      inputManagerMapIterator->second->reinitializeInputManager();
      if(rewindDirFlag)
         {
         (dynamic_cast<RealtimeDirInputManager *> (inputManagerMapIterator->second))->getInputDir()->rewindDir(1);
         }
      }

   InputManager::setFirstFileReceivedTime(0);
   } // End of _reinitializeAllDataManagers() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::_forcedProcessCheck()                                           //
//                                                                            //
// Description:                                                               //
//                                                                            //
// Input: None (access to class data members).                                //
//                                                                            //
// Output:                                                                    //
//  True if forced processing can begin, false otherwise.                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool Gini2Mdv::_forcedProcessCheck()
   {
   const string methodName = "Gini2Mdv::_forcedProcessCheck()";

   map<string, InputManager*>::iterator inputManagerMapIterator;
   for(inputManagerMapIterator=_inputManagerMap.begin(); inputManagerMapIterator!=_inputManagerMap.end(); inputManagerMapIterator++)
      {
      if(inputManagerMapIterator->second->getWaitingForNewFile())
         {
         // Input associated with this data manager has not been found.
         if(inputManagerMapIterator->second->getMandatoryDataStatus())
            {
            // This data set is mandatory. Return false.
            return(false);
            }
         else
            {
            // This data set is not mandatory.
            // Set input_data_t::processWithoutInput flag.
            inputManagerMapIterator->second->setProcessWithoutInput(true);
            }
         } // End of getWaitingForNewFile() conditional.
      } // End of inputManagerMapIterator loop.
   return(true);
   } // End of _forcedProcessCheck() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  Gini2Mdv::_dumpAllInputManagers()                                         //
//                                                                            //
// Description:                                                               //
//  Private method to print the contents of all input managers.               //
//                                                                            //
// Input: None (access to class data members).                                //
//                                                                            //
// Output:                                                                    //
//  None (Screen dump).                                                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void Gini2Mdv::_dumpAllInputManagers()
   {
   const string methodName = "Gini2Mdv::_dumpAllInputManagers()";

   map<string, InputManager*>::iterator inputManagerMapIterator;
   for(inputManagerMapIterator=_inputManagerMap.begin(); inputManagerMapIterator!=_inputManagerMap.end(); inputManagerMapIterator++)
      {
      //_dumpInputManager(inputManagerMapIterator->second);
      inputManagerMapIterator->second->dumpInputManager();
      }
   } // End of _dumpAllInputManagers() method.
