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
////////////////////////////////////////////////////////////////////
// TDWRadial class
//
// Class representing TDWR data
//
// Gary Blackburn
//
// April 2001
//
/////////////////////////////////////////////////////////////////////

#ifndef _TDWR_RADIAL_INC_
#define _TDWR_RADIAL_INC_

#include <cstdio>
#include "tdwr.h"
#include <iostream>
using namespace std;

class TDWRadial {

public:

  typedef struct {
	  int final_range_sample;
	  int rng_samples_per_dwell;
	  int dwell_flag;
	  int pri;
	  int message_id;
	  float elevation;
	  float target_elev;
	  int tilt_num;
	  int volume_num;
    } scan_description_t;

  typedef struct {
	  unsigned char   dbz;
	  unsigned char   snr;
	  unsigned short  vel;
	  unsigned char   width;
	  unsigned char   data_flag;
	  unsigned short  dealias_vel;
  } normal_prf_data_t;

	// Constructors
	TDWRadial(void);
	TDWRadial (const TDWRadial& inputParams); 
	TDWRadial (TDWR_data_header_t *inputParams); 

	// Destructor
	~TDWRadial(){};

	static const int    SAMPLES_PER_DWELL;

	TDWRadial& operator= (const TDWRadial &inputParams);

	void copy (const TDWRadial &inputParams);
	void copy (const TDWR_data_header_t *inputParams );
	void print (FILE *out=stdout);
	void print (ostream &out);

	const bool isLLWASData ();       // true if LLWAS data is detected
	const bool isLowPrf ();          // true if data is collected at a low prf
	const bool newTilt ();           // true if a new elevation is detected
	const bool newVolume ();         // true if a new Volume is detected
	const int getScanMode();         // returns current scan Mode 
	const int getScanType();         // returns current scan type 
	const void loadScanParams (scan_description_t& sParams);

	inline  int   getScanInfoFlag () {return _scanInfoFlag;}
	inline  float getAzimuth () {return _azimuth;}
	inline  int   getTimeStamp () {return _timeStamp;}

	static void BE_to_frame_hdr (Packet_hdr_t *hdr);
	static void BE_to_tdwr_data_hdr (TDWR_data_header_t *hdr);


private:

	unsigned short _messageId;
	unsigned short _messageLength;
	unsigned short _volumeCount;
	unsigned short _volumeFlag;   // first 8 bits - scan strategy, bit 14 - new vol
					              // bit 15 - vol end
	unsigned short _powerTrans;   // peak transmitter power
	unsigned short _playbackFlag; // flages for - playback, live, dummy record
	unsigned int   _scanInfoFlag; // flags for low prf, gust front, MB surface
	                              // low elevation, wind shift, precip and 
	                              // resolution, MB aloft, sector, velocity dealiasing,
	                              // spike removal, obscuration flagging, 8 bits
	                              // reserved.  flags for clutter map number,
	                              // start of elevation, end of elev and contains
	                                  // the scan number
	float          _currentElevation;
	float          _angularScanRate;
	unsigned short _PRI;               // pulse repetition Interval
	unsigned short _dwellFlag;
	unsigned short _finalRangeSample;  // last sample that contans a valid retrun
	unsigned short _rngSamplesPerDwell;
	float          _azimuth;
	float          _totalNoisePower;   // compensated for solar flux effects
	unsigned int   _timeStamp;
	unsigned short _baseDataType;      // unconitioned, edited, fully conditioned data types
	unsigned short _volElevStatusFlag; // indicates incomplete and restarted volumes 
                                      // and elevations
	unsigned short _intergerAzimuth;
	unsigned short _emptyShort;

	
};

#define PPI 1
#define RHI 2
#define SECTOR 3

#endif
