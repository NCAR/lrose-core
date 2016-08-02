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
//
// ArchiveInputManager
//
// Steve Mueller - May 2006
//
////////////////////////////////////////////////////////////////////////////////

// C++ Standard Include Files
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>

// RAP Include Files
#include <toolsa/file_io.h>

// Local Include Files
#include "ArchiveInputManager.hh"
#include "Gini2MdvUtilities.hh"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method: Constructor                                                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ArchiveInputManager::ArchiveInputManager(InputManager::input_data_t inputDataStructure, time_t closestDataTimeUnix, heartbeat_t heartbeat_func, const bool debug)
   {
   setHeartbeat(heartbeat_func);

   // Set InputManager data members that will not change.
   setBaseDirectory(inputDataStructure.baseDirectory);
   setAppendDateSubDirectory(inputDataStructure.appendDateSubDirectory);
   setShortMdvFieldName(inputDataStructure.shortMdvFieldName);
   setLongMdvFieldName(inputDataStructure.longMdvFieldName);
   setFileTemplate(inputDataStructure.fileTemplate);
   _fileTemplate = inputDataStructure.fileTemplate;
   setCalibrationCurveName(inputDataStructure.calibrationCurveName);
   setMandatoryDataStatus(inputDataStructure.mandatoryDataStatus);
   // Data InputManager data members that will change periodically.
   setCurrentInputFile("");
   setCurrentFileCompletionTime(0);
   setCurrentDataTime(0);
   setWaitingForNewFile(true);
   setNewFilePendingProcessing(false);
   setTimedOutForNewFile(false);
   setProcessWithoutInput(false);

   if (getAppendDateSubDirectory() == true)
     {
       setCurrentInputDirectory(getBaseDirectory() + "/" + gini2MdvUtilities::yyyymmddFromUnixTime(closestDataTimeUnix));
     }
   else
     {
       setCurrentInputDirectory(getBaseDirectory());
     }

   setCurrentInputFile(getCurrentInputDirectory() + "/" + buildFileNameFromUnixTime(closestDataTimeUnix));
   setCurrentDataTime(closestDataTimeUnix);
   } // End of ArchiveInputManager constructor.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method: Destructor                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ArchiveInputManager::~ArchiveInputManager()
   {
   } // End of ArchiveInputManager destructor.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  ArchiveInputManager::getFilesAndUnixTimes()                               //
//                                                                            //
// Description:                                                               //
//  Static public method that returns a vector of file names (with paths) and //
//  the UNIX times corresponding to the data contained within the files.      //
//                                                                            //
// Input:                                                                     //
//  unixTimeFileNameVec - Vector of time_t:string pairs onto which directory  //
//   files are pushed.                                                        //
//  directory - string representing the directory to be searched (must        //
//   include date-based subdirectory).                                        //
//                                                                            //
// Output:                                                                    //
//  Vector of <time_t, string> pairs.                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void ArchiveInputManager::getFilesAndUnixTimes(vector< pair<time_t, string> > *unixTimeFileNameVec, string directory, string fileTemplate)
   {
   const string methodName = "ArchiveInputManager::getFilesAndUnixTimes()";

   // Check directory status
   if(!InputManager::directoryStatus(directory))
   {
     return;
   }

   DIR *archiveDirPtr;
   if(0 != (archiveDirPtr = opendir(directory.c_str())))
      {
      struct dirent *archiveFile;
      while((archiveFile = readdir(archiveDirPtr)) != 0) // 0 is NULL
         {
         if(!strcmp(archiveFile->d_name, ".") || !strcmp(archiveFile->d_name, ".."))
            {
            continue;
            }

         time_t dataUnixTime = InputManager::getDataTimeFromFileName(archiveFile->d_name, fileTemplate);
         pair<time_t, string> unixTimeFileNamePair(dataUnixTime, archiveFile->d_name);
         unixTimeFileNameVec->push_back(unixTimeFileNamePair);
         }
      }
   else
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Cannot open archive directory: " << directory << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }
   } // End of getFilesAndUnixTimes() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  ArchiveInputManager::getMinDateStr()                                      //
//                                                                            //
// Description:                                                               //
//  Static public method that returns a string representing the minimum date  //
//  associated with the run time (YYYYMMDD string format).                    //
//                                                                            //
// Input:                                                                     //
//  runTime - time_t structure representing the run time.                     //
//  maxValidSecs - integer representing the maximum number of seconds to      //
//   search from the specified run time.                                      //
//  archiveSearchMode - Enum representing NEAREST (0), NEAREST_BEFORE (1),    //
//   or NEAREST_AFTER (2).                                                    //
//                                                                            //
// Output:                                                                    //
//  String representing the date associated with the lower limit on the       //
//  search time. Returns empty string on error.                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
string ArchiveInputManager::getMinDateStr(time_t runTimeUnix, int maxValidSecs, Params::op_search_mode archiveSearchMode)
   {
   const string methodName = "ArchiveInputManager::getMinDateStr()";
   string minDateStr;

   // Determine minimum bounding time (UNIX time format).
   time_t minTimeUnix;
   if((Params::NEAREST == archiveSearchMode) || (Params::NEAREST_BEFORE == archiveSearchMode))
      {
      minTimeUnix = runTimeUnix - maxValidSecs;
      }
   else
      {
      minTimeUnix = runTimeUnix;
      }

   DateTime *minDateTime = new DateTime(minTimeUnix);
   if(0 == minDateTime) // 0 is NULL
      {
      minDateStr = "";
      }
   else
      {
      minDateStr = minDateTime->getDateStrPlain();
      }
   delete minDateTime;

   return minDateStr;
   } // End of getMinDateStr() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  ArchiveInputManager::getMaxDateStr()                                      //
//                                                                            //
// Description:                                                               //
//  Static public method that returns a string representing the maximum date  //
//  associated with the run time (YYYYMMDD string format).                    //
//                                                                            //
// Input:                                                                     //
//  runTime - time_t structure representing the run time.                     //
//  maxValidSecs - integer representing the maximum number of seconds to      //
//   search from the specified run time.                                      //
//  archiveSearchMode - Enum representing NEAREST (0), NEAREST_BEFORE (1),    //
//   or NEAREST_AFTER (2).                                                    //
//                                                                            //
// Output:                                                                    //
//  String representing the date associated with the upper limit on the       //
//  search time. Returns empty string on error.                               //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
string ArchiveInputManager::getMaxDateStr(time_t runTimeUnix, int maxValidSecs, Params::op_search_mode archiveSearchMode)
   {
   const string methodName = "ArchiveInputManager::getMaxDateStr()";
   string maxDateStr;

   // Determine maximum bounding time (UNIX time format).
   time_t maxTimeUnix;
   if((Params::NEAREST == archiveSearchMode) || (Params::NEAREST_AFTER == archiveSearchMode))
      {
      maxTimeUnix = runTimeUnix + maxValidSecs;
      }
   else
      {
      maxTimeUnix = runTimeUnix;
      }

   DateTime *maxDateTime = new DateTime(maxTimeUnix);
   if(0 == maxDateTime) // 0 is NULL
      {
      maxDateStr = "";
      }
   else
      {
      maxDateStr = maxDateTime->getDateStrPlain();
      }
   delete maxDateTime;

   return maxDateStr;
   } // End of getMaxDateStr() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  ArchiveInputManager::findMandatoryDataDirectory()                         //
//                                                                            //
// Description:                                                               //
//  Static public method that returns a string representing a mandatory data  //
//  directory. Such a directory is necessary to find a closest matching data  //
//  time in archive mode.                                                     //
//                                                                            //
// Input:                                                                     //
//  inputDataMap - A map <string, InputManager::input_data_t> of input data   //
//   structures. Each data set specifies a base directory and a mandatory     //
//   data status.                                                             //
//                                                                            //
// Output:                                                                    //
//  String representing the name of the mandatory directory (with path).      //
//  The directory associated with the first mandatory data set encountered    //
//  while iterating through the inputDataMap is returned.                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
string ArchiveInputManager::findMandatoryDataDirectory(map<string, InputManager::input_data_t> inputDataMap)
   {
   const string methodName = "ArchiveInputManager::findMandatoryDataDirectory()";

   string baseDirectory = "";
   bool foundMandatoryDataSet = false;
   map<string, InputManager::input_data_t>::const_iterator inputDataMapIterator;

   if(inputDataMap.empty())
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   inputDataMap is empty." << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }

   for(inputDataMapIterator=inputDataMap.begin(); inputDataMapIterator!=inputDataMap.end(); inputDataMapIterator++)
      {
      if(inputDataMapIterator->second.mandatoryDataStatus)
         {
         // A mandatory data set. Use the corresponding base directory.
         baseDirectory = inputDataMapIterator->second.baseDirectory;
	 foundMandatoryDataSet = true;
         return baseDirectory;
         }
      else
         {
         // Not a mandatory data set. Do not use.
         continue;
         }
      } // End of inputDataMapIterator loop.

   // Note that the 1st branch of this conditional is extraneous.
   if(!foundMandatoryDataSet)
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "Mandatory data set not found - Returning non-mandatory data directory." << endl;
      inputDataMap.begin()->second.baseDirectory;
      }

   return baseDirectory;
   } // End of findMandatoryDataDirectory() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  ArchiveInputManager::findMandatoryFileTemplate()                          //
//                                                                            //
// Description:                                                               //
//  Static public method that returns a string representing a mandatory       //
//  file template.                                                 //
//                                                                            //
// Input:                                                                     //
//  inputDataMap - A map <string, InputManager::input_data_t> of input data   //
//   structures. Each data set specifies a base directory and a mandatory     //
//   data status.                                                             //
//                                                                            //
// Output:                                                                    //
//  String representing the name of the mandatory file template.               //
//  The directory associated with the first mandatory data set encountered    //
//  while iterating through the inputDataMap is returned.                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
string ArchiveInputManager::findMandatoryFileTemplate(map<string, InputManager::input_data_t> inputDataMap)
   {
   const string methodName = "ArchiveInputManager::findMandatoryFileTemplate()";

   string fileTemplate = "";
   bool foundMandatoryDataSet = false;
   map<string, InputManager::input_data_t>::const_iterator inputDataMapIterator;

   if(inputDataMap.empty())
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   inputDataMap is empty." << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }

   for(inputDataMapIterator=inputDataMap.begin(); inputDataMapIterator!=inputDataMap.end(); inputDataMapIterator++)
      {
      if(inputDataMapIterator->second.mandatoryDataStatus)
         {
         // A mandatory data set. Use the corresponding base directory.
         fileTemplate = inputDataMapIterator->second.fileTemplate;
	 foundMandatoryDataSet = true;
         return fileTemplate;
         }
      else
         {
         // Not a mandatory data set. Do not use.
         continue;
         }
      } // End of inputDataMapIterator loop.

   // Note that the 1st branch of this conditional is extraneous.
   if(!foundMandatoryDataSet)
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "Mandatory data set not found - Returning non-mandatory data directory." << endl;
      inputDataMap.begin()->second.fileTemplate;
      }

   return fileTemplate;
   } // End of findMandatoryFileTemplate() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  ArchiveInputManager::findMandatoryAppendDateSubDirectory()                //
//                                                                            //
// Description:                                                               //
//  Static public method that returns a bool representing a mandatory         //
//  AppendDateSubDirectory.                                                   //
//                                                                            //
// Input:                                                                     //
//  inputDataMap - A map <string, InputManager::input_data_t> of input data   //
//   structures. Each data set specifies a base directory and a mandatory     //
//   data status.                                                             //
//                                                                            //
// Output:                                                                    //
//  Bool representing the append dated subdirectory flag.                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
bool ArchiveInputManager::findMandatoryAppendDateSubDirectory(map<string, InputManager::input_data_t> inputDataMap)
   {
   const string methodName = "ArchiveInputManager::findMandatoryAppendDateSubDirectory()";

   bool appendDateSubDirectory = false;
   bool foundMandatoryDataSet = false;
   map<string, InputManager::input_data_t>::const_iterator inputDataMapIterator;

   if(inputDataMap.empty())
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "   inputDataMap is empty." << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }

   for(inputDataMapIterator=inputDataMap.begin(); inputDataMapIterator!=inputDataMap.end(); inputDataMapIterator++)
      {
      if(inputDataMapIterator->second.mandatoryDataStatus)
         {
         // A mandatory data set. Use the corresponding base directory.
         appendDateSubDirectory = inputDataMapIterator->second.appendDateSubDirectory;
	 foundMandatoryDataSet = true;
         return appendDateSubDirectory;
         }
      else
         {
         // Not a mandatory data set. Do not use.
         continue;
         }
      } // End of inputDataMapIterator loop.

   // Note that the 1st branch of this conditional is extraneous.
   if(!foundMandatoryDataSet)
      {
      cerr << "ERROR: " << methodName << endl;
      cerr << "Mandatory data set not found - Returning non-mandatory data directory." << endl;
      inputDataMap.begin()->second.appendDateSubDirectory;
      }

   return appendDateSubDirectory;
   } // End of findMandatoryAppendDateSubDirectory() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  ArchiveInputManager::getClosestDataTime()                                 //
//                                                                            //
// Description:                                                               //
//  Static public method that returns a UNIX time corresponding to the data   //
//  time that satisfies the "closest match" archive criteria.                 //
//                                                                            //
// Input:                                                                     //
//  runTimeUnix - time_t representation of run time in UNIX format.           //
//  mandatorBaseDirectory - string representing the base directory of a       //
//   mandatory data set (an optional data set, such as VIS, cannot be used to //
//   determine the data time that most closely matches the specified run time //
//   because optional data files do not always exist).                        //
//  maxTime - int representing the maximum number of seconds to search from   //
//   the specified run time.                                                  //
//  archiveSearchMode - Enum representing the "closest matching" criterion--  //
//   NEAREST (0), NEAREST_BEFORE (1), or NEAREST_AFTER (2).                   //
//  debug - Boolean indicating debug output status.                           //
//                                                                            //
// Output:                                                                    //
//  UNIX time (time_t) representing the data time that most closely matches   //
//  the search criteria.                                                      //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
time_t ArchiveInputManager::getClosestDataTime(time_t runTimeUnix,
                                               string mandatoryBaseDirectory,
                                               int maxTime,
                                               Params::op_search_mode archiveSearchMode,
					       string fileTemplate,
					       bool appendDateSubDirectory,
                                               bool debug)
   {
   const string methodName = "ArchiveInputManager::getClosestDataTime()";

   time_t dataTimeUnix;
   int secsDiff;
   vector< pair<time_t, string> > unixTimeFileNameVec;

   if(appendDateSubDirectory == true)
     {
       // Get date ("YYYYMMDD" string) for lower time limit.
       string minDateStr = getMinDateStr(runTimeUnix, maxTime, archiveSearchMode);
       // Get date ("YYYYMMDD" string) for upper time limit.
       string maxDateStr = getMaxDateStr(runTimeUnix, maxTime, archiveSearchMode);

       // If dates differ by more than a single day, issue a warning.
       //  Gini2Mdv can be used in this manner, but it is not the expected usage.
       secsDiff = (int) (gini2MdvUtilities::unixTimeFromDateTimeStr(maxDateStr + "000001") - gini2MdvUtilities::unixTimeFromDateTimeStr(minDateStr + "235959"));
       if(secsDiff > 2)
	 {
	   cerr << "WARNING: " << methodName << endl;
	   cerr << "   More than two date-based archive directories searched." << endl;
	 }

       ////////////////////////////////////////////////////////////
       // Create vector of date-based subdirectories to inspect. //
       ////////////////////////////////////////////////////////////
       //vector<string> dateBasedSubdirectoriesVec = gini2MdvUtilities::getDateStrsFromUnixTimes(runTimeUnix-maxTime, runTimeUnix+maxTime);
       vector<string> dateBasedSubdirectoriesVec;
       dateBasedSubdirectoriesVec.push_back(minDateStr);
       if(minDateStr != maxDateStr)
	 {
	   dateBasedSubdirectoriesVec.push_back(maxDateStr);
	 }
   

       // Debug output
       if(debug)
	 {
	   cout << "DEBUG: " << methodName << endl;
	   cout << "   Dates searched in archive mode:" << endl;
	   vector<string>::const_iterator dateVecIterator;
	   for(dateVecIterator=dateBasedSubdirectoriesVec.begin(); dateVecIterator!=dateBasedSubdirectoriesVec.end(); dateVecIterator++)
	     {
	       cout << "    " << *dateVecIterator << endl;
	     }
	   cout << endl;
	 }

   
       /////////////////////////////////////////////////////////////////////////////////////
       // Loop through date-based subdirectories, and build list of files and data times. //
       /////////////////////////////////////////////////////////////////////////////////////
       //  For each directory get a list of files and associated data times (UNIX time format).
       vector<string>::const_iterator directoryIterator;
       for(directoryIterator=dateBasedSubdirectoriesVec.begin(); directoryIterator!=dateBasedSubdirectoriesVec.end(); directoryIterator++)
	 {
	   string dateSubDir = *directoryIterator;
	   string inputDirectory = mandatoryBaseDirectory + "/" + dateSubDir;
	   ArchiveInputManager::getFilesAndUnixTimes(&unixTimeFileNameVec, inputDirectory, fileTemplate);
	 }
       
     }
   else // no dataed subdirectories
     {
       string inputDirectory = mandatoryBaseDirectory;
       ArchiveInputManager::getFilesAndUnixTimes(&unixTimeFileNameVec, inputDirectory, fileTemplate);       
     }
   if(unixTimeFileNameVec.empty())
      {
      // No files were returned by getFilesAndUnixTimes(). The most likely explanation is
      //  an invalid input directory.
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Could not find data files in specified location for mandatory directory: " << mandatoryBaseDirectory << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }

   vector< pair<time_t, string> >::const_iterator pairVecIterator;
   int minSecsDiff;
   string closestDataFile = "";

   if(Params::NEAREST_AFTER == archiveSearchMode)
	 {
     minSecsDiff = runTimeUnix - unixTimeFileNameVec.begin()->first;
     if(minSecsDiff <= 0) {
       closestDataFile = unixTimeFileNameVec.begin()->second;
     }
     for(pairVecIterator=unixTimeFileNameVec.begin()+1; pairVecIterator!=unixTimeFileNameVec.end(); pairVecIterator++)
     {
       secsDiff = runTimeUnix - pairVecIterator->first;
       if(secsDiff <= 0 && (minSecsDiff > 0 || secsDiff > minSecsDiff) )
       {
         minSecsDiff = secsDiff;
         closestDataFile = pairVecIterator->second;
       }
     }
	 }
   else if(Params::NEAREST_BEFORE == archiveSearchMode)
	 {
     minSecsDiff = runTimeUnix - unixTimeFileNameVec.begin()->first;
     if(minSecsDiff >= 0) {
       closestDataFile = unixTimeFileNameVec.begin()->second;
     }
     for(pairVecIterator=unixTimeFileNameVec.begin()+1; pairVecIterator!=unixTimeFileNameVec.end(); pairVecIterator++)
     {
       secsDiff = runTimeUnix - pairVecIterator->first;
       if(secsDiff >= 0 && (minSecsDiff < 0 || secsDiff < minSecsDiff) )
       {
         minSecsDiff = secsDiff;
         closestDataFile = pairVecIterator->second;
       }
     }
	 }
   else
	 {
     minSecsDiff = abs(runTimeUnix - unixTimeFileNameVec.begin()->first);
     closestDataFile = unixTimeFileNameVec.begin()->second;
     for(pairVecIterator=unixTimeFileNameVec.begin()+1; pairVecIterator!=unixTimeFileNameVec.end(); pairVecIterator++) {
      secsDiff = abs(runTimeUnix - pairVecIterator->first);
      if(secsDiff < minSecsDiff)
         {
         minSecsDiff = secsDiff;
         closestDataFile = pairVecIterator->second;
         }
      }
	 }

   if(!strcmp(closestDataFile.c_str(),"")) {
	   cerr << "No valid file found according to search mode: " << archiveSearchMode << endl;
     cerr << "   EXITING" << endl;
     exit(-1);
   }

   dataTimeUnix = getDataTimeFromFileName(closestDataFile, fileTemplate);

   if(abs(dataTimeUnix - runTimeUnix) > maxTime)
      {
      // UNIX time corresponding to the data file closest to run time, is outside
      //  the specified search interval, this should not happen.
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Data file UNIX time that most closely satisfies archive search criteria is outside search limits." << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }

   if(dataTimeUnix < UNIX_TIME_MILLENIUM)
      {
      // UNIX time older than Jan 1, 2000 00:00:00.
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Data file UNIX time determined by archive search is older than Jan 1, 2000." << endl;
      cerr << "   EXITING" << endl;
      exit(-1);
      }

   return dataTimeUnix;
   } // End of getClosestDataTime() method.


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// Method:                                                                    //
//  ArchiveInputManager::buildFileNameFromUnixTime()                          //
//                                                                            //
// Description:                                                               //
//  Public method that returns a file name corresponding to a specified UNIX  //
//  time. For example, an input UNIX time of 1158018300 generates a return    //
//  file name of "VIS_20060911_2345" if the InputManager file identifier is   //
//  "VIS".                                                                    //
//                                                                            //
// Input:                                                                     //
//  fileTimeUnix - time_t representation of UNIX time.                        //
//                                                                            //
// Output:                                                                    //
//  String representing file name (e.g., "VIS_20060912_1245").                //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
string ArchiveInputManager::buildFileNameFromUnixTime(time_t fileTimeUnix)
   {
   const string methodName = "ArchiveInputManager::buildFileNameFromUnixTime()";

   // Instantiate a DateTime object to perform UNIX time conversions.
   DateTime fileDateTime(fileTimeUnix);

   // Construct complete file name.
   string fileNameStr = fileDateTime.strfTime(getFileTemplate());

   // Error checking.
   if("" == fileNameStr)
      {
      // File name was not assigned.
      cerr << "ERROR: " << methodName << endl;
      cerr << "   Name not assigned for archive file." << endl;
      }

   return fileNameStr;
   } // End of buildFileNameFromUnixTime() method.
