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
/************************************************************************
 * FmqArchiver: Class of objects that write given datasets to a DsFmq
 *              archive.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2003
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef FmqArchiver_hh
#define FmqArchiver_hh

#include <Fmq/DsFmq.hh>

#include "Archiver.hh"

using namespace std;


class FmqArchiver : public Archiver
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /*********************************************************************
   * Constructor
   */

  FmqArchiver(const bool debug = false);


  /*********************************************************************
   * Destructor
   */

  virtual ~FmqArchiver();


  /*********************************************************************
   * init() - Initialize the archiver
   */

  bool init(const string fmq_url,
	    const string program_name,
	    const bool compress_flag,
	    const int num_slots,
	    const int fmq_size);


  /*********************************************************************
   * archiveData() - Write the given data buffer to the archive.
   */

  virtual void archiveData(const char *input_buffer,
			   const int input_buffer_len);


protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  DsFmq _archiveQueue;

};

#endif
