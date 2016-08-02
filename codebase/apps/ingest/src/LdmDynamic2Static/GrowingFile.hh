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
/**
 * @file GrowingFile.hh
 * @brief GrowingFile This class keeps track of a file growing in real-time. 
 *        When triggered, unprocessed data is written to a file of static size
 *        with a unique time stamp. After a configurable amount of time, the 
 *        file will be closed.
 *
 *
 * @class GrowingFile
 * @brief GrowingFile This class keeps track of a file growing in real-time. 
 *        When triggered, unprocessed data is written to a file of static size 
 *        with a unique time stamp. After a configurable amount of time, the 
 *        file will be closed.
 */

#include <cstdio>
#include <vector>
#include <toolsa/umisc.h>
#include <dsserver/DsLdataInfo.hh>
#include <sys/stat.h>
#include "Params.hh"

using namespace std;

class GrowingFile {

public:

  /**
   * Constructor  Private members are set.
   *
   * @param[in] fileName  Name of file
   * @param[in] fileTime  Time of file corresponding to time in the fileName
   * @param[in] outDirPath  Directory for output data.
   * @param[in] outPrecursor  Prefix for output files. May indicate data type
   * @param[in] modTimeThresh Integer number of hours passed fileTime that 
   *                          file will remain open.
   * @param[in] writeLdataInfo  Flag to indicate writing a latest data 
   *                            information file when output file is written
   * @param[in] pDebug  Flag to turn on debug messaging
   *
   */
  GrowingFile(const Params &params,
              string fileName, 
              time_t fileTime);

  /**
   * Destructor: Close the file if it is open.
   */
  ~GrowingFile();

  /**
   * Open file.
   * @return true if file is successfully opened. Otherwise false.
   */
  int open();

  /**
   * @return Return the unix time representation of the time in the file name.
   */
  const time_t filetime() const {return pFileTime;}  
  
  /**
   * @return Return the growing file name
   */
  const string filename() const {return pFileName;}

  /**
   * Process the file: Check to see of the file has grown since the last time
   * it was processed. If it has, write the new data to a file. If the file is old
   * or there were problems accessing the file, set the delete flag to true.
   * @return Return true if new data was written and otherwise false to indicate 
   *         the file did not change. 
   */
  bool process(bool &deleteFlag);

private:

protected:

  const Params &_params;

  /**
   *  Pointer to growing file
   */
  FILE *pFilePtr;

  /**
   * Name (filepath) of growing file
   */
  string pFileName;

  /**
   * Static time associated with growing file
   */
  time_t pFileTime;

  /**
   * Stat struct for the growing file ( This class will use the st_size member
   * to determine of file has grown or not)
   */
  struct stat pStats;

  /**
   * This integer references the last byte in the file that processed.
   */
  long pOffset;

  /**
   * atest data information file when output file is written
   */  
  DsLdataInfo pLdataInfo;

  /**
   * If the file is open, close it
   */
  void pClose();

  /**
   * Read the new data into a buffer and write the contents of the buffer to a 
   * unique file with name containing the current time. Sleep for 2 seconds after
   * writing data to guarantee unique output filenames ( and therefore static 
   * output file size. Write a latest data information file to the output 
   * directory if necessary.
   */
  int pWriteData(); 
};






