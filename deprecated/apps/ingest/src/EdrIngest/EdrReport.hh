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
* EdrReport.hh : 
*                    implementation for the EdrReport class.
*                    The EdrReport class is an abstract base
*                    class for various airline carrier objects
*                    (United, Southwest, Delta) responsible for
*                    decoding and writing EDR reports.
*                    
**************************************************************************/

#ifndef EdrReport_hh
#define EdrReport_hh

#include <stdio.h>

#include "Params.hh"
#include <rapformats/Edr.hh>
#include <toolsa/DateTime.hh>
#include <Spdb/DsSpdb.hh>
#include <math.h>
#include <mel_bufr/mel_bufr.h>
#include <InsituTurb/wfsXmlEdr.hh>

using namespace std;

class EdrReport
{

public:

  enum carrier_t   // Airline carrier type
  {
    UNITED,
    DELTA,
    SOUTHWEST
  };

  enum 
  {
    ENCODED_VERSION = 0,
    ENCODED_PEAK_EDR,
    ENCODED_MEDIAN_EDR,
    ENCODED_PEAK_EDR_CONF,
    ENCODED_MEDIAN_EDR_CONF,
    ENCODED_PEAK_LOCATION,
    ENCODED_NUM_GOOD_EDR,
    ENCODED_NUM_FIELDS
  };

  enum status_t 
  { 
       FAIL = -1,
       ALL_OK = 0,
       END_OF_FILE = 1,
       END_OF_DATA = 2,
       BAD_DATA = 3,
       BAD_INPUT_STREAM = 4,
       INCORRECT_SEQUENCE = 5
  };

  typedef struct {
    int day;
    int hour;
    int minute;
  } downlink_t;

  static const int NUM_CHAR;
  static const int MAX_FIELDS;
  static const float EDR_MAX_VAL;
  static const float CONF_MAX_VAL;
  static const int NUM_OUTPUT_CHAR;
  static const int MISSING_DATA;
  static const int MAX_DIM;
  static const unsigned int MAX_TOKENPTR_LEN;


 // Constructor.

  EdrReport(void);


 // Destructor

  virtual ~EdrReport(void);

  // Member functions

  virtual  int setAndWriteEdr(DateTime msgTime);
                           // Amount of processing is different depending on the carrier,
                           // some require interpolation for aircraft position.  Output
                           // is written to a SPDB data container.  The Edr.hh rapformat
                           // is used.

  virtual status_t decodeAscii(char* buffer);
                           // Each carrier has specific formats for downlinked EDR
			   // reports; turbulence data in all messages is encrypted.  The EDR
			   // report will have to be decoded up until one can determine
			   // what type of format is used before calling this method.  This 
                           // "preamble" contains ARINC Aircraft to ground to NCAR routing 
                           // information.  Sometimes, however, it is necessary to read and partially 
                           // decode part of the actual message to determine the format.
			   // When this happens the format argument is used to store the
			   // rest of that line.  The rest of the message is accessed by  
                           // the buffer pointer.  The decoded report is stored in the command line
                           // argument, report, using the "rapformat" defined in rapformats/Edr.hh

  virtual status_t decodeBufr(BUFR_Val_t &bv);
                           // Each carrier has specific formats for downlinked EDR
			   // reports; turbulence data in all messages is encrypted.  The EDR
			   // report will have to be decoded up until one can determine
			   // what type of format is used before calling this method.  
                           // The BUFR EDR message starts with the flight number
                           // We will keep processing fields until we find the flight
                           // number. Then we will assume it is an EDR message. If
                           // we fail to find the proper sequence of fields 
                           // we continue and then search for the flight number
                           // again.  After the flight number is found, we get the encoded 
                           // tail number, decode it, and use it to determine the airline.
                           // This code will need to be modified once we get the tailmaps
                           // for Delta and Southwest.  The decoded report is stored in the 
                           // command line argument, report, using the "rapformat" defined 
                           // in rapformats/Edr.hh


 void clearAll();

protected:

  void getRecordTime(DateTime msgTime, int hour, int min, int second);
  void getRecordTime(DateTime msgTime, int month, int day, int hour, int min, int second);


  //
  // Members
  //
  DsSpdb   _spdb;
  WfsXmlEdr _wfs;
  vector <Edr::edr_t> _edrBuffer;

  carrier_t carrier;

  downlink_t dnlnk_time;

  char     aircraft_registry_num[Edr::TAILNUM_NAME_LEN];
  char     encoded_aircraft_registry_num[Edr::TAILNUM_NAME_LEN];
  char     encoded_registry_num[Edr::TAILNUM_NAME_LEN];
  char     flight_id[Edr::FLIGHTNUM_NAME_LEN];

  fl32     edr_peak[4];
  fl32     edr_ave[4];

  char     rep_data[4];
  int      bufr_rep_data[4];
  si32     edr_qc_flag[4];
  char     format[Edr::SRC_FMT_LEN];

  // Report location
  fl32     alt;
  fl32     lat;
  fl32     lon;

  char     orig_airport[Edr::AIRPORT_NAME_LEN+1];
  char     dest_airport[Edr::AIRPORT_NAME_LEN+1];

  fl32     sat;
  fl32     wind_dir;
  fl32     wind_speed;
  fl32     edrAlgVersion;

  fl32     peakConf;
  fl32     meanConf;
  fl32     peakLocation;
  fl32     numGood;

  fl32     rms;      // Delta Heartbeat (MDCRS) specific info
  fl32     mach;     // Delta Heartbeat (MDCRS) specific info

  si32     computedAirspeed;      // Delta Heartbeat (MDCRS) specific info

  char     runningMinConf;
  char     runningNumBad;

  si32     maxNumBad;
  si32     QcDescriptionBitFlags;

  DateTime recordTime;

  char     sourceFmt[Edr::SRC_FMT_LEN];
  char     sourceName[Edr::SRC_NAME_LEN];

  char     tail_number[64];

  DateTime msgTime;
  Params *params;

//  EncodeTailnum encodedTails;
//  UalTailMap *tailMap;

 //
 // Protected Methods
 //

 void writeEdr (Edr::edr_t &edrData, bool addUniqueMode = true);

 void bufferEdrReport (Edr::edr_t &edrData);
 void writeWfsData (Edr::edr_t &edrData);

 status_t writeBufferedSpdbData();
 status_t writeBufferedVectorData();

 int change_basis (int in_num_dim, 
                   int *in_dim, 
                   int *in_val, 
                   int out_num_dim, 
                   int *out_dim, 
                   int *out_val);

  bool checkForDuplicateFile();
  bool checkForDuplicateEntry();
  bool checkForRepeatEntry(int repeatNum, time_t checkTime);
  bool checkForNonMinuteTime();

private:

 //
 // PrivateMethods
 //

 static const char *_className(void)
 {
    return (char *)"EdrReport";
 }

 status_t decodeAsciiMsg ( ui08* buffer, DateTime msgTime );
 status_t decodeBufrMsg ( ui08* buffer, DateTime msgTime );

};

#endif

