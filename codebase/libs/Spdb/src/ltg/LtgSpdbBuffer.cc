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
#include <Spdb/DsSpdb.hh>
#include <Spdb/LtgSpdbBuffer.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/mem.h>
#include <toolsa/umisc.h>
using namespace std;


// Global variables

const int STRIKE_BUFFER_INCR = 10;
  
/*********************************************************************
 * Constructor
 */

LtgSpdbBuffer::LtgSpdbBuffer(bool debug_flag,
			     bool put_mode_unique)
{
  _debugFlag = debug_flag;
  _putModeUnique = put_mode_unique;

  _conflictingTypes = false;
  
  _strikeBuffer = (LTG_strike_t *)NULL;
  _strikeBufferBE = (LTG_strike_t *)NULL;

  _strikeBufferExtended = (LTG_extended_t *)NULL;
  _strikeBufferBEextended = (LTG_extended_t *)NULL;

  _strikeBufferAlloc = 0;
  _strikeBufferUsed = 0;

  _dataType = LtgSpdbBuffer::LTG_DATA_TYPE_UNDEFINED; // Don't know what type we're dealing with yet.

  return;

}


/*********************************************************************
 * Destructor
 */

LtgSpdbBuffer::~LtgSpdbBuffer()
{

  // Free the local data buffer

  switch (_dataType) {

  case LtgSpdbBuffer::LTG_DATA_TYPE_CLASSIC :
    if (_strikeBuffer != (LTG_strike_t *)NULL)
      {
	ufree(_strikeBuffer);
	ufree(_strikeBufferBE);
      }

    break;
    
  case LtgSpdbBuffer::LTG_DATA_TYPE_EXTENDED :
    if (_strikeBufferExtended != (LTG_extended_t *)NULL)
      {
	ufree(_strikeBufferExtended);
	ufree(_strikeBufferBEextended);
      }
    
    break;

  default :
    return;
    break;
  }
  
  return;
  
}


/*********************************************************************
 * addStrike() - Adds the given strike to the strike buffer.
 */

void LtgSpdbBuffer::addStrike(LTG_strike_t *strike)
{

  if (_dataType == LtgSpdbBuffer::LTG_DATA_TYPE_UNDEFINED)
    _dataType = LtgSpdbBuffer::LTG_DATA_TYPE_CLASSIC;

  if (_dataType != LtgSpdbBuffer::LTG_DATA_TYPE_CLASSIC)
    {
      _conflictingTypes = true;
      return;
    }

  // Make sure there is enough room in the internal buffers

  _checkBufferAllocation(_strikeBufferUsed + 1);
  
  _strikeBuffer[_strikeBufferUsed] = *strike;
  _strikeBufferUsed++;
  
  return;

}

/*********************************************************************
 * addStrike() - Adds the given strike to the strike buffer. Overloaded.
 */

void LtgSpdbBuffer::addStrike(LTG_extended_t *strike)
{

  if (_dataType == LtgSpdbBuffer::LTG_DATA_TYPE_UNDEFINED)
    _dataType = LtgSpdbBuffer::LTG_DATA_TYPE_EXTENDED;

  if (_dataType != LtgSpdbBuffer::LTG_DATA_TYPE_EXTENDED)
    {
      _conflictingTypes = true;
      return;
    }

  // Make sure there is enough room in the internal buffers

  _checkBufferAllocation(_strikeBufferUsed + 1);
  
  _strikeBufferExtended[_strikeBufferUsed] = *strike;
  _strikeBufferUsed++;
  
  return;

}

/*********************************************************************
 * writeToDatabase() - Writes the current SPDB lightning buffer to the
 *                     database.
 *
 * Returns 0 on success, -1 on failure
 */

int LtgSpdbBuffer::writeToDatabase(char *database_url,
				   int expire_secs,
				   int check_old_data)
{
  // Make sure the buffer contains some data

  if ((_strikeBufferUsed <= 0) ||
      (_dataType == LtgSpdbBuffer::LTG_DATA_TYPE_UNDEFINED)) {
    return 0;
  }

  time_t strike_time =0L;
  int data_len =0;
  int data_type =0;
  DsSpdb spdb;
  int iret = 0;

  switch (_dataType) {

  case LtgSpdbBuffer::LTG_DATA_TYPE_CLASSIC :
  
    strike_time = _strikeBuffer[0].time;
  
    // Make sure the lightning data is in big-endian format
  
    memcpy((void *)_strikeBufferBE, (void *)_strikeBuffer,
	   _strikeBufferUsed * sizeof(LTG_strike_t));
  
    for (int strike = 0; strike < _strikeBufferUsed; strike++)
      LTG_to_BE(&(_strikeBufferBE[strike]));
  
    if (_debugFlag)
      fprintf(stderr,
	      "    %d strikes in SPDB buffer for %s\n",
	      _strikeBufferUsed, utimstr(strike_time));
    
    // Send the data to the database

    if (!check_old_data) {
      spdb.setPutMode(Spdb::putModeAdd);
    }

    if ( _putModeUnique )
      spdb.setPutMode(Spdb::putModeAddUnique);

    data_len = _strikeBufferUsed * sizeof(LTG_strike_t);
    if (spdb.put(database_url,
		 SPDB_KAV_LTG_ID,
		 SPDB_KAV_LTG_LABEL,
		 data_type,
		 strike_time,
		 strike_time + expire_secs,
		 data_len,
		 (void *)_strikeBufferBE)) {
      fprintf(stderr, "ERROR: LtgSpdbBuffer::writeToDatabase\n");
      fprintf(stderr, "  Error writing ltg to URL <%s>\n",
	      database_url);
      fprintf(stderr, "%s\n", spdb.getErrStr().c_str());
      iret = -1;
    }

    break;


  case LtgSpdbBuffer::LTG_DATA_TYPE_EXTENDED :
  
    strike_time = _strikeBufferExtended[0].time;
  
    // Make sure the lightning data is in big-endian format
  
    memcpy((void *)_strikeBufferBEextended, (void *)_strikeBufferExtended,
	   _strikeBufferUsed * sizeof(LTG_extended_t));
  
    for (int strike = 0; strike < _strikeBufferUsed; strike++)
      LTG_extended_to_BE(&(_strikeBufferBEextended[strike]));
  
    if (_debugFlag)
      fprintf(stderr,
	      "    %d extended type strikes in SPDB buffer for %s\n",
	      _strikeBufferUsed, utimstr(strike_time));
    
    // Send the data to the database

    if (!check_old_data) {
      spdb.setPutMode(Spdb::putModeAdd);
    }

    if ( _putModeUnique )
      spdb.setPutMode(Spdb::putModeAddUnique);

    data_type = 0;
    data_len = _strikeBufferUsed * sizeof(LTG_extended_t);
    if (spdb.put(database_url,
		 SPDB_LTG_ID,
		 SPDB_LTG_LABEL,
		 data_type,
		 strike_time,
		 strike_time + expire_secs,
		 data_len,
		 (void *)_strikeBufferBEextended)) {
      fprintf(stderr, "ERROR: LtgSpdbBuffer::writeToDatabase\n");
      fprintf(stderr, "  Error writing extended ltg to URL <%s>\n",
	      database_url);
      fprintf(stderr, "%s\n", spdb.getErrStr().c_str());
      iret = -1;
    }

    break;
    

  default :
    break;

  }

  if (_debugFlag && iret == 0) {
    fprintf(stderr,
	    "    Data put successfully to URL: %s\n", database_url);
  }

  return iret;

}


/*********************************************************************
 * clear() - Clear the buffer.
 */

void LtgSpdbBuffer::clear(void)
{
  _strikeBufferUsed = 0;
  _conflictingTypes = false;
  return;
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

    switch (_dataType) {

    case LtgSpdbBuffer::LTG_DATA_TYPE_CLASSIC :

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
      
      break;

    case LtgSpdbBuffer::LTG_DATA_TYPE_EXTENDED :
      
      _strikeBufferAlloc = num_strikes_needed + STRIKE_BUFFER_INCR;
      
      if (_strikeBufferExtended == (LTG_extended_t *)NULL)
	{
	  _strikeBufferExtended =
	    (LTG_extended_t *)umalloc(_strikeBufferAlloc *
				    sizeof(LTG_extended_t));
	  _strikeBufferBEextended =
	    (LTG_extended_t *)umalloc(_strikeBufferAlloc *
				    sizeof(LTG_extended_t));
	}
      else
	{
	  _strikeBufferExtended =
	    (LTG_extended_t *)urealloc(_strikeBufferExtended,
				       _strikeBufferAlloc *
				       sizeof(LTG_extended_t));
	  _strikeBufferBEextended =
	    (LTG_extended_t *)urealloc(_strikeBufferBEextended,
				       _strikeBufferAlloc *
				       sizeof(LTG_extended_t));
	}
      
      break;

    default :
      return;
      break;

    }
  }

  return;
}

