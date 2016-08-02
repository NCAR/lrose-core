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
 *  $Id: SeviriConverter.hh,v 1.5 2016/03/07 01:23:05 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	SeviriConverter
// 
// Author:	G M Cunning
// 
// Date:	Tue Aug 21 23:24:00 2007
// 
// Description:	The class handles the conversion from radiance or brightness 
//		counts to albedo and temperatures.
// 
// 


# ifndef    HEADER_NAME_H
# define    HEADER_NAME_H

// C++ include files
#include <string>

// System/RAP include files

// Local include files

using namespace std;

class SeviriConverter {
  
public:

  ////////////////////
  // public members //
  ////////////////////

  static const int NUM_BANDS = 12; // total number ofbands on SEVIRI 
  static const int START_IR_BANDS = 4;
  static const int END_IR_BANDS = 11;
  static const int MIN_COUNT_VALUE = 0;
  static const int MAX_COUNT_VALUE = 1023;

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  SeviriConverter();
  SeviriConverter(const SeviriConverter &);

  SeviriConverter  &operator=(const SeviriConverter &from);

  // destructor
  virtual ~SeviriConverter();

  void calculateRadiances(int band_num, int npts, float missing, float** data);
  void calculateBrightnessTemps(int band_num, int npts, float missing, float** data);
  void setMissing(float miss_val) { _missing = miss_val; }

protected:

  ///////////////////////
  // protected members //
  ///////////////////////
  

  ///////////////////////
  // protected methods //
  ///////////////////////

private:

  /////////////////////
  // private members //
  /////////////////////
  bool _isOk;
  string _errStr;
  static const string _className;

  static const float EPSILON = 0.000001;

  //
  // radaince calibration scale and bias
  //
  static const float _radainceCalScale[NUM_BANDS];
  static const float _radainceCalBias[NUM_BANDS];


  //
  // brightness temperature LUT coefficients
  //
  static const float _nuC[NUM_BANDS];
  static const float _alpha[NUM_BANDS];
  static const float _beta[NUM_BANDS];

  //
  // convert radiance to albedo (%)
  //
  static const float _toARad[NUM_BANDS];


  //
  // coeffiecients used in brightness temperature
  //
  // coeff1 = 2hc^2
  // coeff2 = hc/k
  //  
  // where h is Plank constant, c is the speed of light and
  // k is Boltzman constant
  //

  static const float _coeff1 = 1.19104e-5; // !mWm^-2sr^-1(cm^-1)^-4
  static const float _coeff2 = 1.43877;   // K(cm^-1)^-1


  float _missing;

  /////////////////////
  // private methods //
  /////////////////////

  void _copy(const SeviriConverter& from);

};

# endif     /* HEADER_NAME_H */
