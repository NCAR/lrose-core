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
/*
 *  $Id: BufrDecoder.hh,v 1.12 2016/03/07 01:22:59 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	BufrDecoder
// 
// Author:	G. M. cunning
// 
// Date:	Mon Mar 19 10:35 2012
// 
// Description:	Decoder subclass that handles BUFR-formatted messages 
//		following FM 95. See WMO AMDAR reference manual.
// 
// 


# ifndef    BUFR_DECODER_H
# define    BUFR_DECODER_H

// C++ include files

// System/RAP include files

// Local include files
#include "Decoder.hh"

class BufrDecoder : public Decoder {
  
public:

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  BufrDecoder();
  BufrDecoder(const Params *params);
  BufrDecoder(const BufrDecoder& from);

  // destructor
  virtual ~BufrDecoder();

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	BufrDecoder::checkFormat
  //
  // Description:	checks to see if message is FM42 (ASCII) or FM94
  //			(BUFR)  
  //
  // Returns:		a value from msg_type_t
  //
  // Notes:	
  //
  msg_type_t checkFormat(const string &hdr, const string &msg); 

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	BufrDecoder::process
  //
  // Description:	decode the message 
  //
  // Returns:		returns 0 for succes, and -1 otherwise
  //
  // Notes:	
  //
  int process(const std::string& amdar_str, std::vector<Amdar*>& amdars);

  BufrDecoder &operator=(const BufrDecoder &from);

protected:

  ///////////////////////
  // protected members //
  ///////////////////////
  

  ///////////////////////
  // protected methods //
  ///////////////////////

  // set the WMO and type IDs
  void _setID();


private:

  /////////////////////
  // private members //
  /////////////////////

  static const std::string _className;

  static const std::string _endSectionID;

  int _amdarCounter; // used for debugging purposes

  static const int _linesReport1;
  static const int _linesReport2;
  static const int _linesReport3;

  static const int _dataWidthOnRight;

  // a bunch of strings matching FXY values is decbufr output

  static const std::string _missingStr;

  // different report types
  static const std::string _reportType1FXY;
  static const std::string _reportType2FXY; 
  static const std::string _reportType3FXY; 

  // FXY associated with report support
  static const std::string _obsSequenceNumFXY; // goes with _reportType1FXY
  static const std::string _aircraftNavSystemFXY; // goes with _reportType2FXY
  static const std::string _tailNumberFXY; // goes with _reportType2FXY

  // FXY associated with date & time -- used in all 3 report types
  static const std::string _yearFXY;
  static const std::string _monthFXY;
  static const std::string _dayFXY;
  static const std::string _hourFXY;
  static const std::string _minuteFXY;
  static const std::string _secondFXY;

  // FXY associated with position
  static const std::string _latitudeFXY; // goes in all 3
  static const std::string _longitudeFXY;  // goes in all 3
  static const std::string _flightLevelFXY; 
  static const std::string _altitudeFXY; // goes with _reportType2FXY
  static const std::string _detailedPhaseOfFlightFXY; // goes with _reportType1FXY
  static const std::string _phaseOfFlightFXY;

  // FXY associated with weather 
  static const std::string _windDirFXY;
  static const std::string _windspeedFXY;
  static const std::string _temperatureFXY;
  static const std::string _airTemperatureFXY;
  static const std::string _dewpointTemperatureFXY;

  // FXY associated with turbulence
  static const std::string _degreeOfTubulenceFXY; // goes with _reportType1FXY
  static const std::string _maxVertGustFXY; // goes with _reportType1FXY
  static const std::string _degreeOfTurbulenceFXY;
  static const std::string _turbBaseHeightFXY;
  static const std::string _turbTopHeightFXY;
  static const std::string _peakTurbulenceIntensityFXY;

  // FXY associated with airframe icing 
  static const std::string _airframeIcingFXY;

  // FXY associated with naviagtion system
  static const std::string _rollAngleQualityFXY;
  static const std::string _temperaturePrecisionFXY;
  static const std::string _aircraftDataRelaySysFXY;
  static const std::string _aircraftNavSysFXY;

  /////////////////////
  // private methods //
  /////////////////////

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	BufrDecoder::_parseReportType1
  //
  // Description:	parses vector of strings containing deoded
  //			AMDAR in report format 1
  //
  // Returns:		returns 1 for success, otherwise 0
  //
  // Notes:	
  //
  int _parseReportType1(const std::vector< std::string >& decoded_strings, 
			vector< Amdar::bufr_std_amdar_bulletin_t >& bufr_amdars);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	BufrDecoder::_parseReportType2
  //
  // Description:	parses vector of strings containing deoded
  //			AMDAR in report format 2
  //
  // Returns:		returns 1 for success, otherwise 0
  //
  // Notes:	
  //
  int _parseReportType2(const std::vector< std::string >& decoded_strings, 
			vector< Amdar::bufr_std_amdar_bulletin_t >& bufr_amdars);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	BufrDecoder::_parseReportType3
  //
  // Description:	parses vector of strings containing deoded
  //			AMDAR in report format 3
  //
  // Returns:		returns 1 for success, otherwise 0
  //
  // Notes:	
  //
  int _parseReportType3(const std::vector< std::string >& decoded_strings, 
			vector< Amdar::bufr_std_amdar_bulletin_t >& bufr_amdars);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	BufrDecoder::_extractFloat
  //
  // Description:	tries to extract a float from a string
  //
  // Returns:		returns 0 for success, otherwise 1
  //
  // Notes:		looks for 'missing.' will return 1 if missing
  //
  //			left_side option is control searching for val
  //			to the left or right of key in decoded_string
  //
  int _extractFloat(const std::string decoded_string, const std::string key, float& val,
		    bool left_side = true);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	BufrDecoder::_setDetailedPhaseOfFlight
  //
  // Description:	convert numerical value to strings
  //
  // Returns:		returns the string
  //
  // Notes:		there are 15 values. Mapping all to
  //			6 tags: UNS, LVR, LVW, ASC, DES, and '///'
  //
  string _setDetailedPhaseOfFlight(int val);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	BufrDecoder::_setDetailedPhaseOfFlight
  //
  // Description:	convert numerical value to strings
  //
  // Returns:		returns the string
  //
  // Notes:		there are 6 values that map to
  //			UNS, LVR, LVW, ASC, DES, and '///'
  //
  string _setPhaseOfFlight(int val);

  /////////////////////////////////////////////////////////////////////////
  //
  // Method Name:	BufrDecoder::_copy
  //
  // Description:	performs the deep copy
  //
  // Returns:		none
  //
  // Notes:	
  //
  void _copy(const BufrDecoder& from);


};

# endif     /* BUFR_DECODER_H */
