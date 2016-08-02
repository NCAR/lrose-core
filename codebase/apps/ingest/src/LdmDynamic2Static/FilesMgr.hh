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
#include <vector>
#include <toolsa/umisc.h>
#include <dsserver/DsLdataInfo.hh>
#include "GrowingFile.hh"
#include "Params.hh"
using namespace std;

class FilesMgr {

public:
  
  /**
   * Return value for FilesMgr methods
   * FILESMGR_SUCCESS indicates successful method execution
   * FILESMGR_FAILURE indicates unsuccessful method execution
   */
  enum Status_t {FILESMGR_SUCCESS, FILESMGR_FAILURE};

  /**
   * Constructor  Private members are set.
   *
   * @param[in] inDirPath Directory containing growing files
   * @param[in] inExt Extension on files to be managed
   * @param[in] outDirPath  Directory for output data.
   * @param[in] outPrecursor  Prefix for output files. May indicate data type
   * @param[in] maxGapHours Integer number of hours passed fileTime that 
   *                        file will remain open. Maximum lookback for data
   *                        when search for files commences
   * @param[in] writeLdataInfo  Flag to indicate writing a latest data 
   *                            information file when output file is written
   * @param[in] pDebug  Flag to turn on debug messaging
   *
   */
  FilesMgr(const Params &params);

  /**
   * Destructor All files being tracked are deleted
   */
  ~FilesMgr();

  /**
   * Make a pass through the files we are tracking and process them. Delete
   * and files that have been flagged. Search forward from our last filetime to
   * see if new files have been created. If yes, process them and add to our 
   * list of files.
   * @return Return FILESMGR_SUCCESS if any files were processed or added. If 
   *         there arent any files to track or none of changed then return 
   *         FILESMGR_FAILURE. Eventually enough failures will trigger the search
   *         to reinitialize from the current time.
   */
  Status_t update();

  /**
   * Get the current time and search back in time for a file. The maximum integer
   * number of hours we ill look for a file is pMaxGap.
   * @return
   */
  Status_t newSearch();

private:
protected:

  const Params &_params;

  /**
   * A vector or growing files that we are monitoring and processing
   */
  vector <GrowingFile *> pFileVec;

  /**
   * Create a filename with the input time argument. Stat the file and if it 
   * exists add and processes it. Files are assumed to have the naming convention
   * pInDirPath/yyyymmdd/yyyymmddhh.METAR
   * @param[in] startTime  Time struct from which we can grab integer year, 
   *                       month, day, hour, minute, second
   * @return 
   */
  Status_t pGetFile( const date_time_t t);

  /**
   * Search forward or backward from the input time (depending on the input
   * argument) for files that havent been processed. The method will not look 
   * passed the current time and will only look pMaxGapHours into the past or 
   * after the most recent file being tracked.
   * @param[in] startTime  Initial time of search
   * @param[in] searchBackward  Flag to indicate searching backward of forward
   *                            from startTime. True backward, otherwise false. 
   * @return Return FILESMGR_SUCCESS if any files were found and processed. Else
   *         return FILESMGR_FAILURE.
   */
  Status_t pFileSearch(const date_time_t startTime, const bool searchBackward);

  /**
   * Create a new GrowingFile object to our list of files to track and process.
   * Process new data if possible.
   * @param[in] newFileName  Name of new GrowingFile object
   * @param[in] newFileTime  Time of the new file( time in the file name).
   * @return Return FILESMGR_SUCCESS if GrowingFile object was succesfully 
   *         instantiated and processed. Else, return FILESMGR_FAILURE.
   */
  Status_t pAddNewFile( const string newFileName, 
			const date_time_t newFileTime);

};






