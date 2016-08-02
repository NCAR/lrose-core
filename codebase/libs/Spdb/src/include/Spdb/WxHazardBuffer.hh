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
 * WxHazardBuffer.hh: Object representing a buffer of weather hazards
 *                    (manipulated as WxHazard objects) that can be
 *                    stored in an SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * June 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef WxHazardBuffer_HH
#define WxHazardBuffer_HH

/*
 **************************** includes **********************************
 */

#include <cstdio>
#include <ctime>
#include <vector>

#include <dataport/port_types.h>
#include <Spdb/DsSpdb.hh>
#include <Spdb/WxHazard.hh>
using namespace std;


/*
 ******************************* defines ********************************
 */


/*
 ******************************* structures *****************************
 */

/*
 ************************* global variables *****************************
 */

/*
 ***************************** function prototypes **********************
 */

/*
 ************************* class definitions ****************************
 */

class WxHazardBuffer
{
 public:

  // Constructors

  WxHazardBuffer(time_t valid_time, time_t expire_time,
		 int data_type = 0, bool debug_flag = false);
  
  WxHazardBuffer(Spdb::chunk_ref_t *header,
		 void *data,
		 bool debug_flag = false);
  
  // Destructor

  ~WxHazardBuffer(void);
  

  ///////////////////////
  // Iteration methods //
  ///////////////////////

  // Iterate through the hazards in the buffer.  These routines
  // return 0 if there is no hazard to return.  To iterate
  // through the hazard buffer, do something like the following:
  //
  //   for (WxHazard *hazard = hazard_buffer.getFirstHazard();
  //        hazard != 0;
  //        hazard = hazard_list.getNextHazard())
  //              ...

  WxHazard *getFirstHazard(void);
  WxHazard *getNextHazard(void);
  

  //////////////////////////
  // Input/output methods //
  //////////////////////////

  // Writes the current weather hazard information to the database.

  void writeToDatabase(char *database_url);

  // Prints the buffer information to the given stream.

  void print(FILE *stream);
  
  
  ////////////////////
  // Access methods //
  ////////////////////

  // Get the number of hazards in the buffer

  int getNumHazards(void)
  {
    return _hazardList.size();
  }
  
  // Add the given hazard to the buffer.

  void addHazard(WxHazard *hazard);
  
  // Clear the weather hazard information.

  void clear(void);
  
  // Set the buffer's valid time to the given value.

  void setValidTime(time_t valid_time)
  {
    _validTime = valid_time;
  }
  
  // Retrieve the buffer's valid time

  time_t getValidTime(void)
  {
    return _validTime;
  }
  
  // Set the buffer's expire time to the given value

  void setExpireTime(time_t expire_time)
  {
    _expireTime = expire_time;
  }
  
  // Retrieve the buffer's expire time

  time_t getExpireTime(void)
  {
    return _expireTime;
  }
  
  // Set the buffer's data type to the given value

  void setDataType(int data_type)
  {
    _dataType = data_type;
  }
  
  // Retrieve the buffer's data type

  int getDataType(void)
  {
    return _dataType;
  }
  

 private:
  
  // Structure containing the weather hazard header information when
  // the buffer is stored in an SPDB database.  In the database, the
  // entire buffer looks like this:
  //
  //             spdb_header_t
  //             hazard 0 in SPDB format
  //             hazard 1 in SPDB format
  //                  .
  //                  .
  //                  .
  //             hazard n-1 in SPDB format

  typedef struct
  {
    si32 num_hazards;
    si32 spare;
  } spdb_header_t;
  
  // Debug flag

  bool _debugFlag;
  
  // Buffer information (for database)

  time_t _validTime;
  time_t _expireTime;
  int _dataType;
  
  // The list of hazards in this buffer

  vector< WxHazard* > _hazardList;
  
  // The template list iterator, manipulated using getFirstTemplate()
  // and getNextTemplate()

  vector< WxHazard* >::iterator _hazardListIterator;
  
  // Database object for storing hazard information

  DsSpdb _database;
  
  // Swaps the spdb_header_t structure from native format to
  // big-endian format.

  static void _spdbHeaderToBigend(spdb_header_t *header);
  
  // Swaps the spdb_header_t structure from big-endian format
  // to native format.

  static void _spdbHeaderToNative(spdb_header_t *header);
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("WxHazardBuffer");
  }
  
};


#endif
