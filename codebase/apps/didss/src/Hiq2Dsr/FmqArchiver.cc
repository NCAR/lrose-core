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
//   $Date: 2016/03/06 23:53:40 $
//   $Id: FmqArchiver.cc,v 1.2 2016/03/06 23:53:40 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * FmqArchiver: Class of objects that write given datasets to a DsFmq
 *              archive.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <rapformats/ds_radar.h>
#include <toolsa/pmu.h>

#include "FmqArchiver.hh"
using namespace std;


FmqArchiver::FmqArchiver(const bool debug) :
  Archiver(debug)
{
}

FmqArchiver::~FmqArchiver() 
{
}


/*********************************************************************
 * init() - Initialize the archiver
 */

bool FmqArchiver::init(const string fmq_url,
		       const string program_name,
		       const bool compress_flag,
		       const int num_slots,
		       const int fmq_size)
{
  static const string method_name = "FmqArchiver::init()";

  if (_archiveQueue.init(fmq_url.c_str(),
			 program_name.c_str(),
			 _debug,
			 DsFmq::READ_WRITE, DsFmq::END,
			 compress_flag,
			 num_slots,
			 fmq_size,
			 1000))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Could not initialize archive queue '"
	 << fmq_url << "'" << endl;

    return false;
  }

  return true;
}


/*********************************************************************
 * archiveData() - Write the given data buffer to the archive.
 */

void FmqArchiver::archiveData(const char *input_buffer,
			      const int input_buffer_len)
{
  PMU_auto_register("Writing raw beam to archive fmq");

  if (_debug)
    cerr << "Writing raw beam to archive fmq" << endl;

  _archiveQueue.writeMsg(DS_MESSAGE_TYPE_EEC_ASCII,
			 0, input_buffer, input_buffer_len);
}

