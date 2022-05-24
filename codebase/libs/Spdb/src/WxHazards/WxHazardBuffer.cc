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
 * WxHazardBuffer.cc: Object representing a buffer of weather hazards
 *                    (manipulated as WxHazard objects) that can be
 *                    stored in an SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cassert>
#include <cstdio>
#include <ctime>
#include <unistd.h>
#include <vector>
#include <cstring>

#include <toolsa/udatetime.h>
#include <dataport/bigend.h>
#include <dataport/port_types.h>
#include <Spdb/DsSpdb.hh>
#include <Spdb/WxHazard.hh>
#include <Spdb/WxHazardBuffer.hh>
#include <Spdb/WxHazardFactory.hh>
#include <Spdb/Product_defines.hh>
using namespace std;


// Global variables

  
/*********************************************************************
 * Constructor - Create a weather hazards buffer with the given
 *               information.
 */

WxHazardBuffer::WxHazardBuffer(time_t valid_time, time_t expire_time,
			       int data_type,
			       bool debug_flag)
{
  // Save the object information

  _validTime = valid_time;
  _expireTime = expire_time;
  _dataType = data_type;
  _debugFlag = debug_flag;
  _hazardListIterator = _hazardList.end();
}


/*********************************************************************
 * Constructor - Create a weather hazards buffer from the given SPDB
 *               buffers.
 */

WxHazardBuffer::WxHazardBuffer(Spdb::chunk_ref_t *header,
			       void *data,
			       bool debug_flag)
{
  // Save the debug flag

  _debugFlag = debug_flag;
  
  // Save the information from the chunk header

  _validTime = header->valid_time;
  _expireTime = header->expire_time;
  _dataType = header->data_type;
  
  // Pull the header out of the buffer and byte-swap it.

  spdb_header_t *spdb_header = (spdb_header_t *)data;
  _spdbHeaderToNative(spdb_header);
  
  // Pull each of the hazards out of the data

  ui08 *data_ptr = (ui08 *)data + sizeof(spdb_header_t);
  WxHazardFactory factory(_debugFlag);
  
cerr << "Number of hazards is " << spdb_header->num_hazards << endl;

  for (int i = 0; i < spdb_header->num_hazards; i++)
  {
    WxHazard *hazard = factory.create(data_ptr);
    addHazard(hazard);
    
    data_ptr += hazard->getSpdbNumBytes();
  }
  
  _hazardListIterator = _hazardList.end();
  
}


/*********************************************************************
 * Destructor
 */

WxHazardBuffer::~WxHazardBuffer()
{
  // Reclaim the space for the hazard list

  vector< WxHazard* >::const_iterator iter;
  
  for (iter = _hazardList.begin();
       iter != _hazardList.end();
       ++iter)
    delete *iter;
  
  _hazardList.erase(_hazardList.begin(), _hazardList.end());

}


//////////////////////////////
// Hazard iteration methods //
//////////////////////////////

/**********************************************************************
 * getFirstHazard() - Retrieve the first hazard from the buffer.  Returns
 *                    NULL if there are no hazards in the buffer.
 */

WxHazard *WxHazardBuffer::getFirstHazard(void)
{
  _hazardListIterator = _hazardList.begin();
  
  if (_hazardListIterator == _hazardList.end())
    return (WxHazard *)NULL;
  
  return *_hazardListIterator;
}
  

/**********************************************************************
 * getNextHazard() - Retrieve the next hazard from the buffer.  Returns
 *                   NULL if there are no more hazards in the buffer.
 *
 * Note that getFirstHazard() MUST be called before using this method.
 */

WxHazard *WxHazardBuffer::getNextHazard(void)
{
  _hazardListIterator++;
  
  if (_hazardListIterator == _hazardList.end())
    return (WxHazard *)NULL;
  
  return *_hazardListIterator;
}
  

//////////////////////////
// Input/output methods //
//////////////////////////

/*********************************************************************
 * writeToDatabase() - Writes the current weather hazard information to
 *                     the database.
 */

void WxHazardBuffer::writeToDatabase(char *database_url)
{
  const char *routine_name = "writeToDatabase()";
  
  // Allocate space for the SPDB buffer

  int buffer_size = sizeof(spdb_header_t);
  
  for (WxHazard *hazard = getFirstHazard();
       hazard != (WxHazard *)NULL;
       hazard = getNextHazard())
    buffer_size += hazard->getSpdbNumBytes();
  
  ui08 *spdb_buffer = new ui08[buffer_size];
  memset(spdb_buffer, 0, buffer_size);
  
  // Load the header information into the buffer

  spdb_header_t *spdb_header =
    (spdb_header_t *)spdb_buffer;
  
  spdb_header->num_hazards = _hazardList.size();
  spdb_header->spare = 0;
  
  _spdbHeaderToBigend(spdb_header);
  
  // Load the hazards into the buffer

  ui08 *buffer_ptr = spdb_buffer + sizeof(spdb_header_t);
  
  for (WxHazard *hazard = getFirstHazard();
       hazard != (WxHazard *)NULL;
       hazard = getNextHazard())
  {
    hazard->writeSpdb(buffer_ptr);
    buffer_ptr += hazard->getSpdbNumBytes();
  }
  
  // Write the hazard information to the database

  if (_database.put(database_url,
		    SPDB_WX_HAZARDS_ID,
		    SPDB_WX_HAZARDS_LABEL,
		    _dataType,
		    _validTime,
		    _expireTime,
		    buffer_size,
		    (void *)spdb_buffer) != 0)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr, "Error writing weather hazard buffer to URL <%s>\n",
	    database_url);
  }

  delete [] spdb_buffer;

}


/*********************************************************************
 * print() - Prints the buffer information to the given stream.
 */

void WxHazardBuffer::print(FILE *stream)
{
  fprintf(stream, "\n");
  fprintf(stream, "WEATHER HAZARD BUFFER\n");
  fprintf(stream, "---------------------\n");
  
  fprintf(stream, "valid time = %s\n", utimstr(_validTime));
  fprintf(stream, "expire time = %s\n", utimstr(_expireTime));
  fprintf(stream, "data type = %d\n", _dataType);
  fprintf(stream, "\n");
  
  for (WxHazard *hazard = getFirstHazard();
       hazard != (WxHazard *)NULL;
       hazard = getNextHazard())
  {
    fprintf(stream, "Hazard type = %d\n", hazard->getHazardType());
    
    hazard->print(stream);
  }
  
  fprintf(stream, "\n");
}


////////////////////
// Access methods //
////////////////////

/*********************************************************************
 * addHazard() - Add the given hazard to the buffer.
 */

void WxHazardBuffer::addHazard(WxHazard *hazard)
{
  _hazardList.push_back(hazard);
}


/*********************************************************************
 * clear() - Clear the weather hazard information.
 */

void WxHazardBuffer::clear(void)
{
  _hazardList.erase(_hazardList.begin(), _hazardList.end());
  _hazardListIterator = _hazardList.end();
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _spdbHeaderToBigend() - Swaps the spdb_header_t structure from
 *                         native format to big-endian format.
 */

void WxHazardBuffer::_spdbHeaderToBigend(spdb_header_t *header)
{
  BE_from_array_32(header, sizeof(spdb_header_t));
}


/*********************************************************************
 * _spdbHeaderToNative() - Swaps the spdb_header_t structure from
 *                         big-endian format to native format.
 */

void WxHazardBuffer::_spdbHeaderToNative(spdb_header_t *header)
{
  BE_to_array_32(header, sizeof(spdb_header_t));
}
