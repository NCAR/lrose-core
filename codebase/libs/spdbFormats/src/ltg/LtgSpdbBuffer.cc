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
//   $Date: 2016/03/03 18:47:40 $
//   $Id: LtgSpdbBuffer.cc,v 1.4 2016/03/03 18:47:40 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * LtgSpdbBuffer.cc: Object controlling access to a buffer of lightning
 *                   data in SPDB format.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>

#include <rapformats/ltg.h>
#include <spdbFormats/LtgSpdbBuffer.hh>
#include <symprod/spdb.h>
#include <symprod/spdb_client.h>
#include <symprod/spdb_products.h>
#include <toolsa/mem.h>
#include <toolsa/umisc.h>
using namespace std;


// Global variables

const int STRIKE_BUFFER_INCR = 10;
  
/*********************************************************************
 * Constructor
 */

LtgSpdbBuffer::LtgSpdbBuffer(bool debug_flag)
{
  _debugFlag = debug_flag;
  
  _strikeBuffer = (LTG_strike_t *)NULL;
  _strikeBufferBE = (LTG_strike_t *)NULL;
  _strikeBufferAlloc = 0;
  _strikeBufferUsed = 0;
}


/*********************************************************************
 * Destructor
 */

LtgSpdbBuffer::~LtgSpdbBuffer()
{
  // Free the local data buffer

  if (_strikeBuffer != (LTG_strike_t *)NULL)
  {
    ufree(_strikeBuffer);
    ufree(_strikeBufferBE);
  }
  
}


/*********************************************************************
 * addStrike() - Adds the given strike to the strike buffer.
 */

void LtgSpdbBuffer::addStrike(LTG_strike_t *strike)
{
  // Make sure there is enough room in the internal buffers

  _checkBufferAllocation(_strikeBufferUsed + 1);
  
  _strikeBuffer[_strikeBufferUsed] = *strike;
  _strikeBufferUsed++;
  
}


/*********************************************************************
 * writeToDatabase() - Writes the current SPDB lightning buffer to the
 *                     database.
 */

void LtgSpdbBuffer::writeToDatabase(char *database,
				    int expire_secs,
				    int check_old_data)
{
  // Make sure the buffer contains some data

  if (_strikeBufferUsed <= 0)
    return;
  
  time_t strike_time = _strikeBuffer[0].time;
  spdb_chunk_ref_t chunk_hdr;
  
  // Make sure the lightning data is in big-endian format
  
  memcpy((void *)_strikeBufferBE, (void *)_strikeBuffer,
	 _strikeBufferUsed * sizeof(LTG_strike_t));
  
  for (int strike = 0; strike < _strikeBufferUsed; strike++)
    LTG_to_BE(&(_strikeBufferBE[strike]));
  
  if (_debugFlag)
    fprintf(stderr,
	    "    %d strikes in SPDB buffer for %s\n",
	    _strikeBufferUsed, utimstr(strike_time));
  
  // Put the information in the chunk header

  chunk_hdr.valid_time = strike_time;
  chunk_hdr.expire_time = strike_time + expire_secs;
  chunk_hdr.data_type = 0;
  chunk_hdr.prod_id = SPDB_KAV_LTG_ID;
  chunk_hdr.offset = 0;
  chunk_hdr.len = _strikeBufferUsed * sizeof(LTG_strike_t);
  
  // Send the data to the database

  if (check_old_data)
    SPDB_put_over(database,
		  SPDB_KAV_LTG_ID,
		  SPDB_KAV_LTG_LABEL,
		  1,
		  &chunk_hdr,
		  (void *)_strikeBufferBE,
		  chunk_hdr.len);
  else
    SPDB_put_add(database,
		 SPDB_KAV_LTG_ID,
		 SPDB_KAV_LTG_LABEL,
		 1,
		 &chunk_hdr,
		 (void *)_strikeBufferBE,
		 chunk_hdr.len);
}


/*********************************************************************
 * clear() - Clear the buffer.
 */

void LtgSpdbBuffer::clear(void)
{
  _strikeBufferUsed = 0;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _checkBufferAllocation() - Makes sure the strike buffer has enough
 *                            room to store the indicated number of
 *                            strikes.
 */

void LtgSpdbBuffer::_checkBufferAllocation(int num_strikes_needed)
{
  if (num_strikes_needed > _strikeBufferAlloc)
  {
    _strikeBufferAlloc = num_strikes_needed + STRIKE_BUFFER_INCR;
    
    if (_strikeBuffer == (LTG_strike_t *)NULL)
    {
      _strikeBuffer =
	(LTG_strike_t *)umalloc(_strikeBufferAlloc *
				sizeof(LTG_strike_t));
      _strikeBufferBE =
	(LTG_strike_t *)umalloc(_strikeBufferAlloc *
				sizeof(LTG_strike_t));
    }
    else
    {
      _strikeBuffer =
	(LTG_strike_t *)urealloc(_strikeBuffer,
				 _strikeBufferAlloc *
				 sizeof(LTG_strike_t));
      _strikeBufferBE =
	(LTG_strike_t *)urealloc(_strikeBufferBE,
				 _strikeBufferAlloc *
				 sizeof(LTG_strike_t));
    }
  }
  
}

