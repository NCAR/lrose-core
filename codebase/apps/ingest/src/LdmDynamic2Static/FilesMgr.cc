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

#include <cstdio>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
   
#include "FilesMgr.hh"

using namespace std;

// Constructor. Initialises local vars by finding first file.
// File is not read - offset is set to 0.

FilesMgr::FilesMgr(const Params &params) :
        _params(params)
{
 
}

FilesMgr::~FilesMgr()
{
  //
  // Close any open files and delete the GrowingFile objects
  //
  for(int i = 0; i < (int)pFileVec.size(); i++)
  {
    delete pFileVec[i];
  }

}

FilesMgr::Status_t FilesMgr::newSearch()
{
  date_time_t fileTime;

  //
  // Get the current time and put in date_time struct 
  //
  ugmtime(&fileTime);

  //
  // Set the minutes and seconds to 0 (as is the case with times in
  // the filenames)
  //
  fileTime.min=0; 
  
  fileTime.sec=0;
  
  fileTime.unix_time = uconvert_to_utime(&fileTime);

  //
  // Search backward in time
  //
  Status_t ret =  pFileSearch( fileTime, true);

  if ( ret == FILESMGR_FAILURE && _params.Debug )
  {
    fprintf(stderr, "FilesMgr::newSearch(): Failed to find any recent files in "
	    "%s\n", _params.InDir);
  }

  return ret;
}

FilesMgr::Status_t FilesMgr::pFileSearch(const date_time_t startTime, 
					 const bool searchBackward)
{
  //
  // Get the current time. We dont want search past the current
  // time if looking forward from last filetime
  //
  date_time_t currentTime;
  
  ugmtime(&currentTime);
  
  //
  // Set the file time that we are starting with
  //
  date_time_t newFileTime = startTime;
  
  uconvert_from_utime(&newFileTime);
  
  //
  // Initialize file variables and search hour iterator
  //
  Status_t  gotFile = FILESMGR_FAILURE;
  
  int i = 0;
  
  do 
  { 
    
    i++;
    
    gotFile = pGetFile(newFileTime);
    
    if (gotFile == FILESMGR_FAILURE)
    {
      if( searchBackward)
	newFileTime.unix_time = newFileTime.unix_time - 3600 * i;
      else
	newFileTime.unix_time = newFileTime.unix_time + 3600 * i;
      
      uconvert_from_utime(&newFileTime);
    }
  }
  while(gotFile ==  FILESMGR_FAILURE && 
	(i < _params.MaxGap) &&
	(currentTime.unix_time > newFileTime.unix_time));
  
  return gotFile;
}

FilesMgr::Status_t FilesMgr::pAddNewFile( const string newFileName, 
					  const date_time_t newFileTime)
{
  //
  // Create new file
  //
  GrowingFile *singleFile = new GrowingFile(_params,
                                            newFileName, 
                                            newFileTime.unix_time);
  //
  // Open the file
  //
  if (singleFile->open())
  {
    delete singleFile;

    if(_params.Debug)
    {
      fprintf(stderr,"FilesMgr::pAddNewFile(): Failure to open %s. Not keeping "
	      "file.", newFileName.c_str());
    }

    return FILESMGR_FAILURE; 
  }
  else
  {
    //
    // Process the new file.
    //
    bool deleteFlag;
    
    singleFile->process(deleteFlag);
    
    //
    // If processing was successful, keep the file
    //
    if (!deleteFlag)
    {
      pFileVec.push_back(singleFile);

      if(_params.Debug)
      {
	fprintf(stderr,"FilesMgr::pAddNewFile(): Added %s to file list\n", 
		newFileName.c_str());
      }
      return FILESMGR_SUCCESS;
    }
    else
    {
      delete  singleFile;
     
      if(_params.Debug)
      {
	fprintf(stderr,"FilesMgr::pAddNewFile(): Unsuccessful processing of %s. "
		"Not keeping file.", newFileName.c_str());
      }

      return FILESMGR_FAILURE;
    }
  }
}

FilesMgr::Status_t FilesMgr::update()
{
  if (_params.Debug) 
  {
    fprintf(stderr,"FilesMgr::update(): Updating to %s\n", _params.OutDir);
  }

  int processedCount = 0;

  //
  // Loop through the files we have. Process files that have grown, close and 
  // delete any that havent had any changes in _params.MaxGap
  //
  vector <GrowingFile*>::iterator  fileIt = pFileVec.begin();
  
  while(fileIt != pFileVec.end())
  {
    bool deleteFile;
        
    bool processed = (*fileIt)->process(deleteFile);
    
    if (processed)
    {
      processedCount++;
    }

    if(deleteFile)
    {
       if (_params.Debug) 
       {
	 fprintf(stderr,"FilesMgr::update(): Deleting %s from list of files\n", 
		 (*fileIt)->filename().c_str() );
       }


      delete *fileIt;

      pFileVec.erase(fileIt);      
    }
    else
    {
      fileIt++;
    }
  }
   
  //
  // See if a file exists with filetime after the latest (check ahead
  // from last file)
  //
  int numFiles =  (int)pFileVec.size();

  if (numFiles > 0)
  {
    date_time_t newFileTime;

    newFileTime.unix_time = pFileVec[numFiles-1]->filetime() + 3600;
   
    Status_t gotFile = pFileSearch(newFileTime, false);

    if(gotFile == FILESMGR_SUCCESS)
    {
      processedCount++;
    }
  }

  if(_params.Debug)
  {
    fprintf(stderr,"FilesMgr::update(): %d Files have updated and been processed."
	    "\n", processedCount);
  }
  
  if (processedCount > 0)
  {
    return FILESMGR_SUCCESS;
  }
  else
  {
    return FILESMGR_FAILURE;
  }
}

FilesMgr::Status_t FilesMgr::FilesMgr::pGetFile(date_time_t t)
{
  //
  // Create an input file name and see if the exists.
  //
  char name[MAX_PATH_LEN];

  string inDirPath(_params.InDir);
  if (inDirPath[inDirPath.size()-1] != '/') {
    inDirPath += '/';
  }
  if (_params.InHasDayDir) {
    sprintf(name,              
            "%s%d%02d%02d/%d%02d%02d%02d.%s",
            inDirPath.c_str(),t.year,t.month,t.day,
            t.year,t.month,t.day,t.hour, _params.InExt);
  } else {
    sprintf(name,              
            "%s%d%02d%02d%02d.%s",
            inDirPath.c_str(),
            t.year,t.month,t.day,t.hour, _params.InExt);
  }

  if (_params.Verbose) {
    cerr << "Searching for file: " << name << endl;
  }

  struct stat fileStat;

  if (stat(name, &fileStat))
  {
    return FILESMGR_FAILURE;
  } 
  else 
  {
    Status_t addStatus;

    addStatus =  pAddNewFile(name, t);
  
    return addStatus;
  }
}





