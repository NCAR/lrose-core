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
// DiskFullDeleteList.cc - Implementation of class to deal with event lists.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// December 1998
//
/////////////////////////////////////////////////////////////

/**
 * @file DiskFullDeleteList.cc
 *
 * List of files that can be deleted if disk is too full.
 *
 * @author Niles Oien
 * @see something
 */

#include <iostream>
#include <cstdio>
#include <ctime>

#include <didss/DataFileNames.hh>
#include "Params.hh"
#include "DiskFullDeleteList.hh"

using namespace std;


////////////// Constructor. ////////////

DiskFullDeleteList::DiskFullDeleteList(const string &progName,
                                       const Params &params) :
        _progName(progName),
        _params(params),
        _delete(progName, &params)
{
  // Do nothing
}


//////////////// Destructor. //////////

DiskFullDeleteList::~DiskFullDeleteList()
{
  // Do nothing
}


///////////////// addFile /////////////

void DiskFullDeleteList::addFile(const string file_path,
				 const time_t file_time,
				 const int disk_use_threshold,
				 const int disk_delete_threshold)
{
  delete_info_t delete_info;
  
  delete_info.file_path = file_path;
  delete_info.file_time = file_time;
  delete_info.disk_use_threshold = disk_use_threshold;
  delete_info.disk_delete_threshold = disk_delete_threshold;
  
  _fileList.insert(pair< time_t, delete_info_t >(file_time, delete_info));
}


///////////////// clearList /////////////

void DiskFullDeleteList::clearList()
{
  _fileList.erase(_fileList.begin(), _fileList.end());
}


///////////////// deleteFiles /////////////

void DiskFullDeleteList::deleteFiles()
{
  multimap< time_t, delete_info_t >::iterator list_iter;
  
  DataFileNames disk_info;
  
  for (list_iter = _fileList.begin(); list_iter != _fileList.end();
       ++list_iter)
  {
    // Get a pointer to the file information

    delete_info_t *file_info = &((*list_iter).second);
    
    // See how full the disk is that holds this file

    int percent_full = disk_info.PercentFullDisk(file_info->file_path);
    if (percent_full > file_info->disk_delete_threshold)
    {
      _delete.removeFile(file_info->file_path);
    }
    
  } /* endfor - list_iter */
  
}
