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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 18:06:33 $
 *   $Id: DsFileListTrigger.hh,v 1.5 2016/03/03 18:06:33 dixon Exp $
 *   $Revision: 1.5 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * DsFileListTrigger.hh: Class implementing a DsTrigger based on Ldata
 *                    information.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2000
 *
 * Nancy Rehak
 *
 * Refactored from code by Sue Dettling
 *
 * FILE_LIST_MODE:
 *     The setFileList passes in a vector of input paths. 
 *     
 *     nextIssueTime passes in a DateTime reference.
 *     We try to get time from the filepath. If successful the issue time
 *     is recorded in the DateTime object. If unsuccessful an error is returned.
 *
 *     nextFile passes in a reference to a string. When the routine is called, 
 *     the next filepath in the vector is recorded in the string. 
 *     
 *     If nextFile() returns an error, the vector data has been exhausted.
 *     endOfData returns true when the data is exhausted.
 ************************************************************************/

#ifndef DsFileListTrigger_HH
#define DsFileListTrigger_HH

/*
 **************************** includes **********************************
 */

#include <string>
#include <vector>
#include <cassert>

#include <didss/DsURL.hh>
#include <dsdata/DsTrigger.hh>
using namespace std;


/*
 ************************* class definitions ****************************
 */

class DsFileListTrigger : public DsTrigger
{

public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsFileListTrigger();


  /**********************************************************************
   * Destructor
   */

  virtual ~DsFileListTrigger();


  /**********************************************************************
   * init() - Initialize the object.  The object must be initialized
   *          before it can be used.
   *
   * input_file_list: list of files through which to iterate for reads.
   *
   *  Returns 0 on success, -1 on error.
   *
   * Use getErrStr() for error message.
   */

  int init(const vector<string> &input_file_list);
  

  /**********************************************************************
   * init() - Initialize the object.  The object must be initialized
   *          before it can be used.
   *
   * file_list_file: Path of file containing the list of files to process.
   *
   *  Returns 0 on success, -1 on error.
   *
   * Use getErrStr() for error message.
   */

  int init(const string &file_list_file);
  

  /**********************************************************************
   * init() - Initialize the object.  The object must be initialized
   *          before it can be used.
   *
   * input_directory: Directory containing the input file.
   *
   * first_file: The first file in the directory to process.
   *
   * last_file: The last file in the directory to process.
   *
   * In this case, all files in input_directory that fall alphabetically
   * between first_file and last_file will cause triggers.
   *
   *  Returns 0 on success, -1 on error.
   *
   * Use getErrStr() for error message.
   */

  int init(const string &input_directory,
	   const string &first_file,
	   const string &last_file);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * next() - Get the next trigger and set the triggerInfo accordingly
   *
   * Returns 0 upon success, -1 upon failure.
   */

  int next();
  

  /**********************************************************************
   * endOfData() - Check whether we are at the end of the data.
   */

  bool endOfData() const;
  

  /**********************************************************************
   * reset() - Reset to start of data list
   */

  void reset();
  

private:

  /////////////////////
  // Private members //
  /////////////////////

  bool _objectInitialized;
  
  vector <string> _fileList;
  size_t _fileListPtr;


  /////////////////////
  // Private methods //
  /////////////////////

  /**********************************************************************
   * _convertPath2Time() - Get the data time from a filepath
   *
   * path: file path to parse for determining data time
   * dataTime: returned utime parsed from the data set path
   *
   * Returns 0 on success, -1 on error.
   */

  int _convertPath2Time(const string& path,
			time_t& dataTime) const;
  
};

#endif /* DsFileListTrigger_HH */


