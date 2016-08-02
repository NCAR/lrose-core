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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/03 18:06:33 $
//   $Id: DsFileListTrigger.cc,v 1.8 2016/03/03 18:06:33 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DsFileListTrigger.cc: Class implementing a DsTrigger based on a
 *                       given file list.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2000
 *
 * Nancy Rehak
 *
 * Refactored from code by Sue Dettling
 *
 *********************************************************************/

#include <dirent.h>
#include <sys/types.h>

#include <didss/ds_input_path.h>
#include <dsdata/DsFileListTrigger.hh>

using namespace std;



/**********************************************************************
 * Constructors
 */

DsFileListTrigger::DsFileListTrigger() :
  DsTrigger(TYPE_FILE_TRIGGER),
  _objectInitialized(false)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsFileListTrigger::~DsFileListTrigger()
{
  // Do nothing
}


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

int DsFileListTrigger::init(const vector<string> &input_file_list)
{
  const string method_name = "DsFileListTrigger::init()";
  
  _clearErrStr();
 
  // Get input data.

  _fileList = input_file_list;

  // Set the pointer to data to beginning of data set.  

  _fileListPtr = 0;

  _objectInitialized = true;
  
  return 0;
}


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

int DsFileListTrigger::init(const string &file_list_file)
{
  const string method_name = "DsFileListTrigger::init()";
  
  _clearErrStr();
 
  // Get input data.

  FILE *fp = fopen(file_list_file.c_str(), "rt");
  if (fp == NULL)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "List filename " << file_list_file << " not found." << endl;
    
    return -1;
  }

  int j = EOF;
  char tmp_str[PATH_MAX];
  
  while ((j = fscanf(fp, "%s", tmp_str)) != EOF)
  {
    if (strlen(tmp_str) == 0)   // If blank, assume EOF
    {
      j = EOF;
      break;
    }
    
    // Save the file name

    _fileList.push_back(tmp_str);
  }

  fclose(fp);

  // Set the pointer to data to beginning of data set.  

  _fileListPtr = 0;

  _objectInitialized = true;
  
  return 0;
}


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

int DsFileListTrigger::init(const string &input_directory,
			    const string &first_file,
			    const string &last_file)
{
  const string method_name = "DsFileListTrigger::init()";
  
  _clearErrStr();
 
  // Read the files from the given directory

  DIR *dir = opendir(input_directory.c_str());
  if (dir == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening directory: " << input_directory << endl;
    
    return -1;
  }
  
  struct dirent *dirent;
  
  while ((dirent = readdir(dir)) != 0)
  {
    string next_file = dirent->d_name;
    
    if (next_file >= first_file && next_file <= last_file)
      _fileList.push_back(input_directory + "/" + next_file);
  }
  
  closedir(dir);

  // Set the pointer to data to beginning of data set.  

  _fileListPtr = 0;

  _objectInitialized = true;
  
  return 0;
}


/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsFileListTrigger::next()
{
  const string method_name = "DsFileListTrigger::next()";
  
  assert(_objectInitialized);
  
  int iret;
  time_t issueTime;
  string filePath;
  
  // clear out the old stuff

  _clearErrStr();
  _triggerInfo.clear();

  // Check for end of trigger data

  if (endOfData())
      return -1;

  // Set the trigger info

  _triggerInfo.setFilePath( _fileList[_fileListPtr] );

  iret = _convertPath2Time( _fileList[_fileListPtr], issueTime ); 
    
  if ( iret)
  {
    _errStr = "WARNING - " + method_name + "\n";
    _errStr += "Unable to get time from filepath.\n";
    _triggerInfo.setIssueTime( -1 );
  }
  else
  {
    _triggerInfo.setIssueTime( issueTime );
  }

  _fileListPtr++;

  return 0;
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsFileListTrigger::endOfData() const
{
  assert(_objectInitialized);
  
  if (_fileListPtr >= _fileList.size()) 
    return true;
  
  return false;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsFileListTrigger::reset()
{
  assert(_objectInitialized);
  
  _fileListPtr = 0;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _convertPath2Time() - Get the data time from a filepath
 *
 * path: file path to parse for determining data time
 * dataTime: returned utime parsed from the data set path
 *
 * Returns 0 on success, -1 on error.
 */

int DsFileListTrigger::_convertPath2Time(const string& path,
					 time_t& dataTime) const
{
  // For now, just use the old DSINP_get_data_time from DsInputPath

  int results = DSINP_get_data_time( NULL, (char*)path.c_str() );
   
  if ( results == -1 )
  {
    return( results );
  }
  else
  {
    dataTime = results;
     
    return( 0 );
  }
}
