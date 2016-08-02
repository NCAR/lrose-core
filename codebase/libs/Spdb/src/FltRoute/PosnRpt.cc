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
 * PosnRpt.cc: Class representing an aircraft position report as stored
 *             in an SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1999
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <cstdio>
#include <ctime>
#include <unistd.h>

#include <dataport/bigend.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include <Spdb/Product_defines.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/PosnRpt.hh>
#include <Spdb/WayPoint.hh>
using namespace std;

// Initialize global variables

const int PosnRpt::HASH_MULT = 314159;
const int PosnRpt::HASH_PRIME = 516595003;

  
/*********************************************************************
 * Constructors
 */

PosnRpt::PosnRpt(const string& flight_num,
		 const string& tail_num,
		 const double& current_lat,
		 const double& current_lon,
		 const DateTime& current_time,
		 const WayPoint& way_pt0,
		 const WayPoint& way_pt1,
		 const WayPoint& way_pt2,
		 const bool debug_flag) :
  _debugFlag(debug_flag),
  _flightNum(flight_num),
  _tailNum(tail_num),
  _currentLat(current_lat),
  _currentLon(current_lon),
  _currentTime(current_time),
  _wayPt0(way_pt0),
  _wayPt1(way_pt1),
  _wayPt2(way_pt2)
{
  // Do nothing
}


PosnRpt::PosnRpt(const string& flight_num,
		 const string& tail_num,
		 const double& current_lat,
		 const double& current_lon,
		 const time_t& current_time,
		 const WayPoint& way_pt0,
		 const WayPoint& way_pt1,
		 const WayPoint& way_pt2,
		 const bool debug_flag) :
  _debugFlag(debug_flag),
  _flightNum(flight_num),
  _tailNum(tail_num),
  _currentLat(current_lat),
  _currentLon(current_lon),
  _currentTime(current_time),
  _wayPt0(way_pt0),
  _wayPt1(way_pt1),
  _wayPt2(way_pt2)
{
  // Do nothing
}


PosnRpt::PosnRpt(const void *spdb_buffer,
		 const bool debug_flag) :
  _debugFlag(debug_flag)
{
  // Create the position report object from the SPDB buffer

  spdb_posn_rpt_t posn_rpt = *(spdb_posn_rpt_t *)spdb_buffer;
  spdbToNative(&posn_rpt);
  
  _flightNum = posn_rpt.flight_num;
  _tailNum = posn_rpt.tail_num;
  _currentLat = posn_rpt.current_lat;
  _currentLon = posn_rpt.current_lon;
  _currentTime = posn_rpt.current_time;
  
  char *buffer_ptr = (char *)spdb_buffer + sizeof(spdb_posn_rpt_t);
  
  WayPoint way_pt0(buffer_ptr, debug_flag);
  _wayPt0 = way_pt0;
  buffer_ptr += way_pt0.getSpdbNumBytes();
  
  WayPoint way_pt1(buffer_ptr, debug_flag);
  _wayPt1 = way_pt1;
  buffer_ptr += way_pt1.getSpdbNumBytes();
  
  WayPoint way_pt2(buffer_ptr, debug_flag);
  _wayPt2 = way_pt2;
  
}


/*********************************************************************
 * Destructor
 */

PosnRpt::~PosnRpt()
{
  // Do nothing
}


/********************************************************************
 * calcDataType() - Generates a fairly unique, non-zero hash value
 *                  for the given flight number.
 *
 * Taken from code written by Gerry Wiener, based on code by Knuth.
 */

int PosnRpt::calcDataType(const string& flight_num) const
{
  return calcDataType(flight_num.c_str());
}


int PosnRpt::calcDataType(const char *flight_num) const
{
  int hash;

  for (hash = 0; *flight_num; flight_num++)
  {
    hash += (hash ^ (hash >> 1)) + HASH_MULT * (unsigned char)*flight_num;
    while (hash >= HASH_PRIME)
      hash -= HASH_PRIME;
  }

  if (hash == 0)
    hash = 1;

  return(hash);
}


/*********************************************************************
 * print() - Print the position report to the given stream.
 */

void PosnRpt::print(FILE *stream) const
{
  fprintf(stream, "Position Report:\n");
  fprintf(stream, "   flight_num = <%s>\n", _flightNum.c_str());
  fprintf(stream, "   tail_num = <%s>\n", _tailNum.c_str());
  fprintf(stream, "   current_lat = %f\n", _currentLat);
  fprintf(stream, "   current_lon = %f\n", _currentLon);
  fprintf(stream, "   current_time = %s\n",
	  _currentTime.dtime());
  
  fprintf(stream, "   way_pt0:\n");
  _wayPt0.print(stream);
  
  fprintf(stream, "   way_pt1:\n");
  _wayPt1.print(stream);
  
  fprintf(stream, "   way_pt2:\n");
  _wayPt2.print(stream);
  
}


/*********************************************************************
 * writeSpdb() - Write the position report to the given byte buffer
 *               in SPDB format. Note that the data will be written to
 *               the buffer in big-endian format.  Note also that it is
 *               assumed that the given buffer is big enough for the
 *               data.
 */

void PosnRpt::writeSpdb(void *buffer) const
{
  // Put the position report information in the buffer

  spdb_posn_rpt_t *posn_rpt = (spdb_posn_rpt_t *)buffer;
  
  STRcopy(posn_rpt->flight_num, _flightNum.c_str(), MAX_FLT_NUM_LEN);
  STRcopy(posn_rpt->tail_num, _tailNum.c_str(), MAX_TAIL_NUM_LEN);
  posn_rpt->current_lat = _currentLat;
  posn_rpt->current_lon = _currentLon;
  posn_rpt->current_time = _currentTime.utime();
  posn_rpt->spare = 0;
  
  spdbToBigend(posn_rpt);
  
  // Put each of the way points in the buffer

  char *buffer_ptr = (char *)buffer + sizeof(spdb_posn_rpt_t);
  _wayPt0.writeSpdb((void *)buffer_ptr);

  buffer_ptr += _wayPt0.getSpdbNumBytes();
  _wayPt1.writeSpdb((void *)buffer_ptr);

  buffer_ptr += _wayPt1.getSpdbNumBytes();
  _wayPt2.writeSpdb((void *)buffer_ptr);
  
}


/*********************************************************************
 * writeToDatabase() - Writes the position report to the indicated database.
 */

void PosnRpt::writeToDatabase(const char *database_url,
			       const time_t valid_time,
			       const time_t expire_time) const
{
  const char *routine_name = "writeToDatabase()";
  
  // Allocate space for the SPDB buffer

  int buffer_size = getSpdbNumBytes();
  ui08 *spdb_buffer = new ui08[buffer_size];

  // Put the position report into the buffer

  writeSpdb((void *)spdb_buffer);
  
  // Write the position report to the database

  DsSpdb database;
  
  if (database.put(database_url,
		   SPDB_POSN_RPT_ID,
		   SPDB_POSN_RPT_LABEL,
		   calcDataType(_flightNum),
		   valid_time,
		   expire_time,
		   buffer_size,
		   (void *)spdb_buffer) != 0)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr, "Error writing position report to URL <%s>\n",
	    database_url);
  }
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * spdbToBigend() - Swaps the spdb_posn_rpt_t structure from native
 *                  format to big-endian format.
 */

void PosnRpt::spdbToBigend(spdb_posn_rpt_t *posn_rpt)
{
  int char_array_len = MAX_FLT_NUM_LEN + MAX_TAIL_NUM_LEN;
  
  BE_from_array_32((char *)posn_rpt + char_array_len,
		   sizeof(spdb_posn_rpt_t) - char_array_len);
}


/*********************************************************************
 * spdbToNative() - Swaps the spdb_posn_rpt_t structure from big-endian
 *                  format to native format.
 */

void PosnRpt::spdbToNative(spdb_posn_rpt_t *posn_rpt)
{
  int char_array_len = MAX_FLT_NUM_LEN + MAX_TAIL_NUM_LEN;
  
  BE_to_array_32((char *)posn_rpt + char_array_len,
		 sizeof(spdb_posn_rpt_t) - char_array_len);
}
