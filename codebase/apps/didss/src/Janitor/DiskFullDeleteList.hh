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
// DiskFullDeleteList.hh - class to deal with a list of files that
//                         could be deleted if the disk is too full.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2001
//
/////////////////////////////////////////////////////////////

/**
 * @class DiskFullDeleteList
 *
 * Class to contain a list of files that could be deleted if disk too full.
 *
 * \#include "DiskFullDeleteList.hh" <BR>
 *
 * @author Nancy Rehak
 * @version $Id: DiskFullDeleteList.hh,v 1.7 2016/04/01 22:01:53 dixon Exp $
 * @see something.

 */

#ifndef DiskFullDeleteList_HH
#define DiskFullDeleteList_HH

#include <string>
#include <map>
#include <toolsa/udatetime.h> // For time structure date_time_t
#include "Delete.hh"

using namespace std;

/**
 * @addtogroup Janitor
 */
/*@{*/

class DiskFullDeleteList
{
  
public:

  // constructor

  /**
   * Alternate constructor.
   *
   * @param[in] debug_flag - debug flag. Default value = false.
   */
  DiskFullDeleteList(const string &progName,
                     const Params &params);

  // Destructor

  /**
   * Destructor.
   */
  ~DiskFullDeleteList();

  ////////////////////
  // Public methods //
  ////////////////////

  // Add a file to the list
  /**
   * Add a file to the list.
   *
   * @param[in] file_path - path of file to add.
   * @param[in] file_time - modification time of file.
   * @param[in] disk_use_threshold - disk too full if usage above threshold.
   * @param[in] disk_delete_threshold - deleting will stop when disk usage below thr.
   */
  void addFile(const string file_path,
	       const time_t file_time,
	       const int disk_use_threshold,
	       const int disk_delete_threshold);

  // Clear the list
  /**
   * Clear the list.
   */
  void clearList();

  // Delete files in the list until the disks aren't too full
  /**
   * Delete files in the list until the disks aren't too full.
   */
  void deleteFiles();
  

private:

  ///////////////////
  // Private types //
  ///////////////////

  /**
   * struct to contain pertinent file parameters.
   */
  typedef struct
  {
    string file_path;		/**< file path.					*/
    time_t file_time;		/**< file modification time.			*/
    int disk_use_threshold;	/**< disk too full if usage above threshold.	*/
    int disk_delete_threshold;	/**< files will be deleted until disk usage	*
				 *   is below threshold.			*/
  } delete_info_t;

  /////////////////////
  // Private members //
  /////////////////////

  string _progName;
  const Params &_params;
  Delete _delete;

  multimap< time_t, delete_info_t > _fileList;	/**< file list.			*/

};

/*@}*/

#endif
