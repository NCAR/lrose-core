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
 * PosnRpt.hh: Class representing an aircraft position report as stored
 *             in an SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1999
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef PosnRpt_HH
#define PosnRpt_HH

/*
 **************************** includes **********************************
 */

#include <cstdio>
#include <string>
#include <ctime>

#include <dataport/port_types.h>
#include <Spdb/WayPoint.hh>
#include <toolsa/DateTime.hh>
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

class PosnRpt
{
 public:

  //////////////////////////////
  // Constructors/destructors //
  //////////////////////////////

  // Constructors

  PosnRpt(const string& flight_num,
	  const string& tail_num,
	  const double& current_lat,
	  const double& current_lon,
	  const DateTime& current_time,
	  const WayPoint& way_pt0,
	  const WayPoint& way_pt1,
	  const WayPoint& way_pt2,
	  const bool debug_flag = false);

  PosnRpt(const string& flight_num,
	  const string& tail_num,
	  const double& current_lat,
	  const double& current_lon,
	  const time_t& current_time,
	  const WayPoint& way_pt0,
	  const WayPoint& way_pt1,
	  const WayPoint& way_pt2,
	  const bool debug_flag = false);

  PosnRpt(const void *spdb_buffer,   // SPDB buffer as read directly from a
	                              // database (in big-endian format).
	   const bool debug_flag = false);
  
  // Destructor

  virtual ~PosnRpt(void);
  
  //////////////////////////
  // Input/output Methods //
  //////////////////////////

  // Calculate the number of bytes occupied by this position report when
  // stored in an SPDB database.

  inline int getSpdbNumBytes(void) const
  {
    return sizeof(spdb_posn_rpt_t) +
      3 * WayPoint::getSpdbNumBytes();
  }
  
  // Print the position report to the given stream.

  void print(FILE *stream) const;
  
  // Write the position report to the given byte buffer in SPDB format.
  // Note that the data will be written to the buffer in big-endian format.
  // Note also that it is assumed that the given buffer is big enough for
  // the data.

  void writeSpdb(void *buffer) const;
  
  // Writes the position report to the indicated database.

  void writeToDatabase(const char *database_url,
		       const time_t valid_time,
		       const time_t expire_time) const;
  

  ////////////////////
  // Access methods //
  ////////////////////

  const string& getFlightNum(void) const
  {
    return _flightNum;
  }
  
  const string& getTailNum(void) const
  {
    return _tailNum;
  }
  
  const double& getCurrentLat(void) const
  {
    return _currentLat;
  }
  
  const double& getCurrentLon(void) const
  {
    return _currentLon;
  }
  
  const DateTime& getCurrentTime(void) const
  {
    return _currentTime;
  }
  
  const WayPoint& getWayPoint0(void) const
  {
    return _wayPt0;
  }
  
  const WayPoint& getWayPoint1(void) const
  {
    return _wayPt1;
  }
  
  const WayPoint& getWayPoint2(void) const
  {
    return _wayPt2;
  }
  
  void setFlightNum(const string& flight_num)
  {
    _flightNum = flight_num;
  }
  
  void setTailNum(const string& tail_num)
  {
    _tailNum = tail_num;
  }
  
  void setCurrentPosition(const double& lat, const double& lon)
  {
    _currentLat = lat;
    _currentLon = lon;
  }
  
  void setCurrentTime(const DateTime& current_time)
  {
    _currentTime = current_time;
  }
  
  void setCurrentTime(const time_t& current_time)
  {
    _currentTime = current_time;
  }
  
  void setWayPoint0(const WayPoint& way_pt)
  {
    _wayPt0 = way_pt;
  }
  
  void setWayPoint1(const WayPoint& way_pt)
  {
    _wayPt1 = way_pt;
  }
  
  void setWayPoint2(const WayPoint& way_pt)
  {
    _wayPt2 = way_pt;
  }
  

  ///////////////////////////
  // Miscellaneous methods //
  ///////////////////////////

  // Generates a fairly unique, non-zero hash value for the given
  // flight number.

  int calcDataType(const char *flight_num) const;
  int calcDataType(const string& flight_num) const;
  

 protected:
  
  // Debug flag

  bool _debugFlag;
  
  // Position report information

  string _flightNum;
  string _tailNum;
  double _currentLat;
  double _currentLon;
  DateTime _currentTime;
  WayPoint _wayPt0;
  WayPoint _wayPt1;
  WayPoint _wayPt2;


  // Values used for calculating the SPDB data type based on the
  // flight number string.

  static const int HASH_MULT;
  static const int HASH_PRIME;

  // Structure containing the position report information when the report
  // is stored in an SPDB database.  This information is followed in the
  // database by 3 WayPoint::spdb_way_point_t structures which represent
  // the current and 2 following way points.

  static const int MAX_FLT_NUM_LEN = 12;
  static const int MAX_TAIL_NUM_LEN = 12;
  
  typedef struct
  {
    char flight_num[MAX_FLT_NUM_LEN];
    char tail_num[MAX_TAIL_NUM_LEN];
    fl32 current_lat;
    fl32 current_lon;
    ti32 current_time;
    si32 spare;
  } spdb_posn_rpt_t;
  
  // Swaps the spdb_posn_rpt_t structure from native format to
  // big-endian format.

  static void spdbToBigend(spdb_posn_rpt_t *posn_rpt);
  
  // Swaps the spdb_posn_rpt_t structure from big-endian format to
  // native format.

  static void spdbToNative(spdb_posn_rpt_t *posn_rpt);
  
 private:
  
  // Return the class name for error messages.

  static const char *_className(void)
  {
    return("PosnRpt");
  }
  
};


#endif
