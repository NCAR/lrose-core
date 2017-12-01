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
// Directory.hh: Part of the Janitor program.
//
// Niles Oien and
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
/////////////////////////////////////////////////////////////

/**
 * @class Directory
 *
 * Class to handle Janitor processing of a directory.
 *
 * \#include "Directory.hh" <BR>
 *
 * @author Niles Oien and Mike Dixon
 * @version $Id: Directory.hh,v 1.18 2017/11/03 22:04:28 dixon Exp $
 * @see something
 */

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string>

#include <didss/DataFileNames.hh>

#include "DiskFullDeleteList.hh"
#include "EventList.hh"
#include "Delete.hh"

using namespace std;


class Params;

/**
 * @addtogroup Janitor
 */
/*@{*/

class Directory
{
  
public:

  // constructor
  
  /**
   * Alternate constructor.
   *
   * @param[in] prog_name - program name.
   * @param[in] params - directory management parameters.
   * @param[in] top_dir - top directory in the tree.
   * @param[in] dir_path - current directory path.
   * @param[in,out] delete_list - list of files to delete.
   */
  Directory (const string prog_name, Params *params,
	     const string top_dir,
	     const string dir_path,
	     DiskFullDeleteList *delete_list);

  // Destructor

  /**
   * Destructor.
   */
  ~Directory();

  // public data
  
  int OK;	/**< flag that construction was successful.	*/

  // Process the directory
  
  /**
   * Process the directory.
   *
   * @returns 0 on success; -1 on error, 1 if dir is locked.
   */
  int process();

protected:
  
private:

  // Event lists

  EventList _masterEventList;		/**< master event list.		*/
  EventList _localEventList;		/**< local event list.		*/

  bool _eventListsOkay;			/**< flag for event lists okay.	*/
  
//  static const string PARAM_FILE_NAME;
  
  // Check to see if the current hostname matches the expected one
  /**
   * Check to see if the current hostname matches the expected one.
   *
   * @param[in] expected_hostname - host name to match.
   *
   * @return true if host names match; false otherwise.
   */
  bool _hostNamesMatch(const string expected_hostname) const;

  // Check to see if the given directory name matches a RAP date
  // directory.
  /**
   * Class method to see if given directory is a date directory.
   *
   * @param[in] dir_path - directory to check.
   *
   * @return true if directory matches date format; false otherwise.
   */
  static bool _dirMatchesDateFormat(const string dir_path);

  // Process the given directory recursively
  /**
   * Process the given directory recursively.
   *
   * @param[in] current_path - directory to process.
   */
  void _processDirectory(const string current_path);

  // Process the given file
  /**
   * Process the specified file.
   *
   * @param[in] current_path - path of file to process.
   * @param[in] current_path_info - information about current path.
   * @param[out] file_deleted - true if file added to delete list.
   */
  void _processFile(const string current_path,
		    const DataFileNames &current_path_info,
		    bool &file_deleted);

  // Compress the given file
  /**
   * Compress the given file.
   *
   * @param[in] file_path - path of file to compress.
   */
  void _compressFile(const string file_path);
  
  // Write a report
  /**
   * Write a report.
   *
   * @param[in] dirName - directory name.
   * @param[in] local_event_filename - filename of local event.
   * @param[in] local_event_return - return code from local event.
   * @param[in] master_event_filename - filename of master event.
   * @param[in] master_event_return - return code from master event.
   */
  void _writeReport(const string dirName,
		    const string local_event_filename,
		    int local_event_return,
		    const string master_event_filename,
		    int master_event_return);

  void _writeParams(FILE *fp);

  // Load the local parameters
  /**
   * Load the local parameters.
   *
   * @param[in] param_filename - filename to load from.
   * @param[in[ global_params - global parameters to start with.
   *
   * @return true if parameters found in file; false otherwise.
   */
  bool _loadLocalParams(const string param_filename,
			const Params &global_params);
  
  bool _isEmpty(const string dirPath);

  string _progName;		/**< program name.				*/
  Params *_params;		/**< processing parameters.			*/
  Delete *_delete;
  string _topDir;		/**< top of directory tree.			*/
  string _dirPath;		/**< current directory path.			*/
  bool _paramsAreLocal;		/**< flag local parameters for directory.	*/
  int _maxNoModSecsDelete;	/**< maximum no mod age for deletion.   	*/
  int _maxDirAgeSecs;		/**< maximum directory age in seconds.		*/
  int _maxNoAccessSecsCompress;	/**< maximum no access age for compression.	*/
  int _maxNoModSecsCompress;	/**< maximum no mod age for compression.   	*/
  
  DiskFullDeleteList *_diskFullDeleteList;	/**< list of files to delete.	*/

  long _nFilesFound;
  long _nFilesRemaining;
  long _nFilesDeleted;

  long _nFilesCompressed;
  double _nBytesUncompressed;
  double _nBytesCompressed;
  
};

/*@}*/

#endif



