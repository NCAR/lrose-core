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
//   $Date: 2016/03/04 02:22:15 $
//   $Id: RealtimeFileRetriever.cc,v 1.8 2016/03/04 02:22:15 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * RealtimeFileRetriever.cc: Class for retrieving realtime DsMdvx files.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2000
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/DsMdvx.hh>
#include <toolsa/pmu.h>
#include <unistd.h>

#include "RealtimeFileRetriever.hh"
using namespace std;


/*********************************************************************
 * Constructor
 */

RealtimeFileRetriever::RealtimeFileRetriever(const string& input_url,
                                             int sleep_interval,
					     const bool debug_flag) :
  FileRetriever(input_url, debug_flag),
  _lastFileTime(0),
  _sleepInterval(sleep_interval)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

RealtimeFileRetriever::~RealtimeFileRetriever(void)
{
  // Do nothing
}


/*********************************************************************
 * next() - Method for retrieving the next file to process.
 */

DsMdvx *RealtimeFileRetriever::next(void)
{
  const string routine_name = "next()";
  
  DsMdvx *new_mdv_file;
  
  // Try to read the next data file, if one is available.

  bool file_found = false;
  
  while (!file_found)
  {
    PMU_auto_register("Waiting for data");
    
    time_t current_time = time((time_t *)NULL);
    int search_margin = current_time - _lastFileTime - 1;
  
    if ((new_mdv_file = _readFile(current_time, search_margin)) == 0)
    {
      sleep(_sleepInterval);
      continue;
    }
    
    _lastFileTime = new_mdv_file->getMasterHeader().time_centroid;
    
    break;
  }
  
  return new_mdv_file;
}

DsMdvx *RealtimeFileRetriever::_readFile(const time_t search_time,
					 const int search_margin)
{
   const string routine_name = "_readFile()";

   if (_debugFlag)
     cout << "Reading file for time " << utimstr(search_time) << endl;

   // Create the file object

   DsMdvx *mdv_file = new DsMdvx();
   
   // Read in the data

   mdv_file->clearRead();
   mdv_file->setReadTime(Mdvx::READ_FIRST_BEFORE,
			 _inputUrl,
			 search_margin, search_time);
//   mdv_file->clearReadFields();
//   mdv_file->addReadField(_params->field_num);
   mdv_file->setReadEncodingType(Mdvx::ENCODING_FLOAT32);
   mdv_file->setReadCompressionType(Mdvx::COMPRESSION_NONE);
   mdv_file->setReadScalingType(Mdvx::SCALING_NONE);
   
   if (_debugFlag)
     mdv_file->printReadRequest(cout);
   
   if (mdv_file->readVolume() != 0)
   {
     delete mdv_file;
     
     return 0;
   }
   
   return mdv_file;
}

  
/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

