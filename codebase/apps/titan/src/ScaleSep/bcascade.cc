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
// 	$Id: bcascade.cc,v 1.3 2016/03/04 01:28:13 dixon Exp $
/// \file bcascade.cpp
/// \brief Class to decompose a field into its spectral components and to calculate cascade parameters
/// \class BCascade
/// \brief Class to decompose a field into its spectral components and to calculate cascade parameters
/// <pre>
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// MODULE NAME: BCascade.cpp
///
/// TYPE: Class
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
////
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// PURPOSE:
///
///  This is the base class used to decompose a field into its spectral components and to calculate the cascade parameters
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// CHANGE HISTORY
///
/// VERSION NO.	AUTHOR		DATE		CHANGE DETAILS
///
///
/// 1.00	        Alan Seed       ??/??/??        Initial Version
/// 1.10          Alan Seed       04/05/2007      Added documentation
/// 1.11          Alan Seed       08/05/2010      Changed to 7 levels in cascade
///                                               Spatially varying standard deviation
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR	     DATE	   CHANGE REVEIWER    ACCEPTED
///
/// 0.00	        Neill Bowler 06/02/03
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// NOTES
///---------------------------------------------------------------------------
/// </pre>

#include "bcascade.hh"
// #include "fftw_util.h"
//#define DEBUG                   ///< Enable debug logging and identical noise between runs for the class
#define BUFFERSIZE 8*1024*1024  ///< More efficient file IO
//#define TEST_BCASCADE
//---------------------------------------------------------------------------

  const fl32 BCascade::CASCADE_MIN_KM = 4.0;
const fl32 BCascade::CASCADE_SCALE_BREAK = 4.0;

BCascade::BCascade(
  int i4NumberRowsPassed,        ///< IN The number of lines in a radar analysis
  int i4NumberColsPassed,        ///< IN The number of samples in each line of a radar analysis
  int i4NumberCascadesPassed,    ///< IN The number of cascades to be kept in memory
  int i4TimeStepPassed,          ///< IN The time step of the radar and NWP in minutes
  float r4PixelSizePassed,       ///< IN The size (in km) of a pixel
  float r4NoDataPassed,          ///< IN The "No-data" value (or outside value)
  char* cLogFileNamePassed,      ///< IN The name of the log file
  char* cBCascadeFileNamePassed, ///< IN The name of the BCascade output file
  int* i4ReturnCode              ///< OUT Whether this routine has been sucessful
){
/// \fn BCascade(int i4NumberRowsPassed,int i4NumberColsPassed,int i4NumberCascadesPassed,int i4TimeStepPassed,float r4PixelSizePassed,float r4NoDataPassed,char* cLogFileNamePassed,char* cBCascadeFileNamePassed,int* i4ReturnCode)
/// \brief Constructor for the BCascade class
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: BCascade.cpp
///
/// TYPE: Cascade Constructor
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// Constructor: Set up class-wide variables and define dynamic arrays.
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Alan Seed       27/03/05        Initial version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
/// 1. Set up some class-wide variables
/// 2. Initialise parameters
/// 3. Alocate memory
/// 4. Set up the tracking object
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// Returns 0 on success, -1 on failure
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
/// FILENAME: cLogFileName
/// NAME OF POINTER TO FILE: fLogFile
/// USAGE: output log
///
/// FILENAME: cBCascadeFileName
/// Name of File Handle-  i4BCascadeFile
///
/// FILENAME: cOpticalFileName
/// Passed to Optical Flow class
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// ARRAY NAME: r4CascadeMean
/// ARRAY TYPE: float**
/// DIMENSIONS: i4NumberCascades, i4NumberRowsPassed*i4NumberColsPassed
///
/// ARRAY NAME: r4CascadeStd
/// ARRAY TYPE: float**
/// DIMENSIONS: i4NumberCascades, i4NumberRowsPassed*i4NumberColsPassed
///
/// ARRAY NAME: r4Correlation
/// ARRAY TYPE: float*
/// DIMENSIONS: i4NumberCascades*i4NumberLevels
///
/// ARRAY NAME: r4Phi
/// ARRAY TYPE: float*
/// DIMENSIONS: i4NumberCascades*i4NumberLevels
///
/// ARRAY NAME: c8FFTout
/// ARRAY TYPE: fftw_complex*
/// DIMENSIONS: i4FFTSize*i4FFTSize
///
/// ARRAY NAME: r8FFTin
/// ARRAY TYPE: double*
/// DIMENSIONS: i4FFTSize*i4FFTSize
///
/// ARRAY NAME: r4TempArray
/// ARRAY TYPE: float*
/// DIMENSIONS: i4CascadeSize+1 * i4CascadeSize+1
///
/// ARRAY NAME: r4CascadeStack
/// ARRAY TYPE: float*
/// DIMENSIONS: i4NumberCascades, i4NumberLevels*i4CascadeSize*i4CascadeSize
///
/// ARRAY NAME: r4Maps
/// ARRAY TYPE: float**
/// DIMENSIONS: [i4NumberMaps][i4CascadeArraySize]
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// float r4NoData;                 // flag for non-valid data- must be < 0; -999 is good
/// FILE* fLogFile;                 // Pointer to the log file
/// float* r4FFT ;                  // temp arrays for the fft of the nids
/// float* r4TempArray ;            // Temporary array of image data
/// float* r4CascadeStack;       // cascade arrays
/// float* r4Phi;                   // ar(2) model parameters for updates
/// float* r4Correlation;           // auto correlations for the cascade
/// time_t sMapTimes[8] ;           // list of times for each map
/// float* r4CascadeMean ;          // mean for each level
/// float* r4CascadeStd ;           // standard deviation for each level
/// int i4CascadeSize;              // size of the cascade assumed to be square
/// int i4NumberRows;               // size of the input rain map
/// int i4NumberCols;               // size of the input rain map
/// int i4NumberCascades;           // number of cascades in memory
/// int i4TimeStep;                 // time step of radar and NWP in minutes
/// float r4PixelSize;              // pixel size in km
/// int i4ImageNumber;              // number of images since the cascade was initialised
/// int i4NumberLevels;             // image size and number of cascade levels
/// float* r4Maps;                  // array of previous maps
/// int i4FFTArraySize ;            // size of the zero packed array for the fft
/// float *r4FFTweights ;           // used by bandpass filter
///
/// STANDARD LIBRARIES USED:
///
/// stdio.h
/// time.h
/// iostream.h
/// string.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
/// </pre>
//
// Local variables
//
  size_t i4Loop;                   // Loop variable
  int i4MaxSize;                   // The maximum of i4NumberRowsPassed and i4NumberColsPassed

//
//-----------------------------------------------------------------------
//
// 1. Set up some class-wide variables
//
  idum = -time(NULL) ;

#ifdef DEBUG
  idum = -2007062100 ;
#endif

  for ( i4Loop = 0; i4Loop < 512; i4Loop++ ) cLogFileName[i4Loop] = 0 ;
  for ( i4Loop = 0; i4Loop < 512; i4Loop++ ) {
    cOpticalFileName[i4Loop] = 0 ;
    if ( cBCascadeFileNamePassed[i4Loop] == '.' ) break ;
    if ( cBCascadeFileNamePassed[i4Loop] == 0 )   break ;
    cOpticalFileName[i4Loop] = cBCascadeFileNamePassed[i4Loop] ;
  }
  strcpy(cBCascadeFileName, cBCascadeFileNamePassed);

  r4NoData          = r4NoDataPassed ;
  i4NumberRows      = i4NumberRowsPassed ;
  i4NumberCols      = i4NumberColsPassed ;
  i4NumberCascades  = i4NumberCascadesPassed ;
  i4TimeStep        = i4TimeStepPassed ;
  r4PixelSize       = r4PixelSizePassed ;

  i4CascadeMinLevel = (int)(CASCADE_MIN_KM/r4PixelSize) ;
  if ( i4CascadeMinLevel < 2 ) i4CascadeMinLevel = 2 ;
  r4CascadeScaleBreak = CASCADE_SCALE_BREAK/r4PixelSize ;  //Scale break in pixels

  i4ImageNumber   = 0 ;
  *i4ReturnCode   = 0;
  fLogFile        = NULL ;
  r4Phi           = NULL ;
  r4Correlation   = NULL ;
  inPlan          = NULL ;
  outPlan         = NULL ;
  c4FFTout        = NULL ;
  c4FFT           = NULL ;
  r8FFTin         = NULL ;
  r4TempArray     = NULL ;
  // sRadarAdvection = NULL ;
  r4CascadeStack  = NULL ;
  r4RainMap       = NULL ;
  i4DataMask      = NULL ;
  r4BandpassFilter = NULL ;
  // pMatch = new ProbMatch() ;
  // rand_gen = new gasRV();

// ACB 8/1/2010: Method to set random seed to specific number
//    ( this will allow generation of identical noise between runs)
#ifdef DEBUG
  rand_gen->set_seed(idum);
#endif

  for ( i4Loop = 0; i4Loop < 8; i4Loop++ ) sMapTimes[i4Loop] = 0 ;
  r4FieldMean  = 0 ;
  r4FieldStd   = 0 ;
  hasOpenedLogFile = false ;
  bIsDet = false ;

//
//-----------------------------------------------------------------------
//
// 2. Initialise parameters
//
  i4NumberMaps     = 2 ; // keep the previous map for tracking
  i4NumberLags     = 2 ; // default to the ar(2) model
// AWS 6 Sep 2010 Try increasing the edge of the map
//  i4MapOffset      = 16 ; // place a 16 row, 16 col buffer around the observed map in the cascade
  i4MapOffset      = 64 ; // place a 16 row, 16 col buffer around the observed map in the cascade
  i4MaxSize = std::max(i4NumberRows, i4NumberCols);
  i4CascadeSize  = i4MaxSize + 2*i4MapOffset ;
  i4NumberLevels = 7 ;
  r4ScaleRatio = pow( 2.0/(double)i4CascadeSize, 1.0/(double)(i4NumberLevels-1)) ;

  while ( r4ScaleRatio < 0.42 ) {
   i4NumberLevels++ ;
   r4ScaleRatio = pow( 2.0/(double)i4CascadeSize, 1.0/(double)(i4NumberLevels-1)) ;
  }

  // mDom.init(i4NumberRows, i4NumberCols) ;
  // cDom.init(i4CascadeSize, i4CascadeSize) ;

  i4InMapArraySize   = i4NumberRows*i4NumberCols  ;
  i4CascadeArraySize = i4CascadeSize*i4CascadeSize ;
//
//-----------------------------------------------------------------------
//
// 3. Allocate memory to dynamic arrays
//
//
  allocate_memory() ;
// set up the data mask
  int i4ColOffset = (cDom.ncols() - mDom.ncols())/2 ;
  int i4RowOffset = (cDom.nrows() - mDom.nrows())/2 ;
  int i4Row ;
  int i4Col ;
  int i4OutOffset ;
  for ( i4Loop = 0; i4Loop < (size_t)mDom.npoints(); i4Loop++ ) {
     i4Row = mDom.row(i4Loop)+i4RowOffset ;
     i4Col = mDom.col(i4Loop)+i4ColOffset ;
     i4OutOffset = cDom.point(i4Row, i4Col) ;
     i4DataMask[i4OutOffset] = 1 ;
  }

//
// 4. Set up the tracking object
//
  int i4InitOption        = 0 ;
  // Note that the tracking object is the size of the cascade not the input map
  sRadarAdvection = new Optical_Flow( i4CascadeSize, i4CascadeSize, r4PixelSize, i4InitOption, cOpticalFileName);

#ifdef VERBOSE_CASCADE
  if ( cLogFileNamePassed != NULL ) {
    strcpy(cLogFileName, cLogFileNamePassed);
    fLogFile = fopen(cLogFileNamePassed, "wt") ;
#ifdef DEBUG
    fprintf(fLogFile, "Cascade size = %d pixels, minimum scale = %d pixels, number of levels = %d\n", i4CascadeSize, i4CascadeMinLevel, i4NumberLevels) ;
#endif
    fprintf(fLogFile,"Random number seed idum = %d",idum);
    hasOpenedLogFile = true ;
  }
#endif
}

BCascade::BCascade(
  int i4UpscaleOptionPassed,     ///< IN The option to upscale the cascade
  int i4NumberRowsPassed,        ///< IN The number of lines in a radar analysis
  int i4NumberColsPassed,        ///< IN The number of samples in each line of a radar analysis
  int i4NumberCascadesPassed,    ///< IN The number of cascades to be kept in memory
  int i4TimeStepPassed,          ///< IN The time step of the radar and NWP in minutes
  float r4PixelSizePassed,       ///< IN The size (in km) of a pixel
  float r4NoDataPassed,          ///< IN The "No-data" value (or outside value)
  char* cLogFileNamePassed,      ///< IN The name of the log file
  char* cBCascadeFileNamePassed, ///< IN The name of the BCascade output file
  int* i4ReturnCode              ///< OUT Whether this routine has been sucessful
){
/// \fn BCascade::BCascade(int i4UpscaleOptionPassed,int i4NumberRowsPassed,int i4NumberColsPassed,int i4NumberCascadesPassed,int i4TimeStepPassed,float r4PixelSizePassed,float r4NoDataPassed,char* cLogFileNamePassed,char* cBCascadeFileNamePassed,int* i4ReturnCode)
/// \brief  Constructor for BCascade with upscale option
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: BCascade.cpp
///
/// TYPE: Cascade Constructor
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// Constructor: Set up class-wide variables and define dynamic arrays.
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Alan Seed       27/03/05        Initial version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
/// 1. Set up some class-wide variables
/// 2. Initialise parameters
/// 3. Alocate memory
/// 4. Set up the tracking object
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// Returns 0 on success, -1 on failure
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
/// FILENAME: cLogFileName
/// NAME OF POINTER TO FILE: fLogFile
/// USAGE: output log
///
/// FILENAME: cBCascadeFileName
/// Name of File Handle-  i4BCascadeFile
///
/// FILENAME: cOpticalFileName
/// Passed to Optical Flow class
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// ARRAY NAME: r4CascadeMean
/// ARRAY TYPE: float**
/// DIMENSIONS: i4NumberCascades, i4NumberRowsPassed*i4NumberColsPassed
///
/// ARRAY NAME: r4CascadeStd
/// ARRAY TYPE: float**
/// DIMENSIONS: i4NumberCascades, i4NumberRowsPassed*i4NumberColsPassed
///
/// ARRAY NAME: r4Correlation
/// ARRAY TYPE: float*
/// DIMENSIONS: i4NumberCascades*i4NumberLevels
///
/// ARRAY NAME: r4Phi
/// ARRAY TYPE: float*
/// DIMENSIONS: i4NumberCascades*i4NumberLevels
///
/// ARRAY NAME: c8FFTout
/// ARRAY TYPE: fftw_complex*
/// DIMENSIONS: i4FFTSize*i4FFTSize
///
/// ARRAY NAME: r8FFTin
/// ARRAY TYPE: double*
/// DIMENSIONS: i4FFTSize*i4FFTSize
///
///
/// ARRAY NAME: r4TempArray
/// ARRAY TYPE: float*
/// DIMENSIONS: i4CascadeSize+1 * i4CascadeSize+1
///
/// ARRAY NAME: r4CascadeStack
/// ARRAY TYPE: float*
/// DIMENSIONS: i4NumberCascades, i4NumberLevels*i4CascadeSize*i4CascadeSize
///
/// ARRAY NAME: r4Maps
/// ARRAY TYPE: float**
/// DIMENSIONS: [i4NumberMaps][i4CascadeArraySize]
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// float r4NoData;                 // flag for non-valid data- must be < 0; -999 is good
/// FILE* fLogFile;                 // Pointer to the log file
/// float* r4FFT ;                  // temp arrays for the fft of the nids
/// float* r4TempArray ;            // Temporary array of image data
/// float* r4CascadeStack;       // cascade arrays
/// float* r4Phi;                   // ar(2) model parameters for updates
/// float* r4Correlation;           // auto correlations for the cascade
/// time_t sMapTimes[8] ;           // list of times for each map
/// float* r4CascadeMean ;          // mean for each level
/// float* r4CascadeStd ;           // standard deviation for each level
/// int i4CascadeSize;              // size of the cascade assumed to be square
/// int i4NumberRows;               // size of the input rain map
/// int i4NumberCols;               // size of the input rain map
/// int i4NumberCascades;           // number of cascades in memory
/// int i4TimeStep;                 // time step of radar and NWP in minutes
/// float r4PixelSize;              // pixel size in km
/// int i4ImageNumber;              // number of images since the cascade was initialised
/// int i4NumberLevels;             // image size and number of cascade levels
/// float** r4Maps;                  // array of previous maps
/// int i4FFTArraySize ;            // size of the zero packed array for the fft
/// float *r4FFTweights ;           // used by bandpass filter
///
/// STANDARD LIBRARIES USED:
///
/// stdio.h
/// time.h
/// iostream.h
/// string.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
/// </pre>
//
// Local variables
//
  size_t i4Loop;                      // Loop variable
  int i4MaxSize;                   // The maximum of i4NumberRowsPassed and i4NumberColsPassed
//
//-----------------------------------------------------------------------
//
// 1. Set up some class-wide variables
//
  idum = -time(NULL) ;
#ifdef DEBUG
  idum = -2007062100 ;
#endif

  i4NumberRows      = i4NumberRowsPassed ;
  i4NumberCols      = i4NumberColsPassed ;
  i4TimeStep        = i4TimeStepPassed ;
  r4PixelSize       = r4PixelSizePassed ;

  if ( i4UpscaleOptionPassed == 1 ) {
    i4NumberRows /= 2 ;
    i4NumberCols /= 2 ;
    r4PixelSize *= 2 ;
  }

  for ( i4Loop = 0; i4Loop < 512; i4Loop++ ) {
    cOpticalFileName[i4Loop] = 0 ;
    if ( cBCascadeFileNamePassed[i4Loop] == '.' ) break ;
    if ( cBCascadeFileNamePassed[i4Loop] == 0 )   break ;
    cOpticalFileName[i4Loop] = cBCascadeFileNamePassed[i4Loop] ;
  }
  strcpy(cBCascadeFileName, cBCascadeFileNamePassed);
  r4CascadeScaleBreak = CASCADE_SCALE_BREAK/r4PixelSize ;  //Scale break in pixels
  i4NumberCascades  = i4NumberCascadesPassed ;
  r4NoData          = r4NoDataPassed ;
  i4CascadeMinLevel = (int)(CASCADE_MIN_KM/r4PixelSize) ;
  if ( i4CascadeMinLevel < 2 ) i4CascadeMinLevel = 2 ;

  i4ImageNumber   = 0 ;
  *i4ReturnCode   = 0;
  fLogFile        = NULL ;
  r4Phi           = NULL ;
  r4Correlation   = NULL ;
  inPlan          = NULL ;
  outPlan         = NULL ;
  c4FFTout        = NULL ;
  c4FFT           = NULL ;
  r8FFTin         = NULL ;
  r4TempArray     = NULL ;
  sRadarAdvection = NULL ;
  r4CascadeStack  = NULL ;
  r4RainMap       = NULL ;
  i4DataMask      = NULL ;
  r4BandpassFilter = NULL ;
  pMatch = new ProbMatch() ;
  rand_gen = new gasRV();

// ACB 8/1/2010: Method to set random seed to specific number
//    ( this will allow generation of identical noise between runs)
#ifdef DEBUG
  rand_gen->set_seed(idum);
#endif

  for ( i4Loop = 0; i4Loop < 8; i4Loop++ ) sMapTimes[i4Loop] = 0 ;
  r4FieldMean  = 0 ;
  r4FieldStd   = 0 ;
  hasOpenedLogFile = false ;
  bIsDet = false ;

//
//-----------------------------------------------------------------------
//
// 2. Initialise parameters
//
  i4NumberMaps     = 2 ; // keep the previous map for tracking
  i4NumberLags     = 2 ; // default to the ar(2) model
//  i4MapOffset      = 16 ; // place a 16 row, 16 col buffer around the observed map in the cascade
  i4MapOffset      = 64 ; // place a 16 row, 16 col buffer around the observed map in the cascade

  i4MaxSize = std::max(i4NumberRows, i4NumberCols);

  i4CascadeSize  = i4MaxSize + 2*i4MapOffset ;
  i4NumberLevels = 7 ;
  r4ScaleRatio = pow( 2.0/(double)i4CascadeSize, 1.0/(double)(i4NumberLevels-1)) ;
  while ( r4ScaleRatio < 0.42 ) {
   i4NumberLevels++ ;
   r4ScaleRatio = pow( 2.0/(double)i4CascadeSize, 1.0/(double)(i4NumberLevels-1)) ;
  }

  mDom.init(i4NumberRows, i4NumberCols) ;
  cDom.init(i4CascadeSize, i4CascadeSize) ;

  i4InMapArraySize   = i4NumberRows*i4NumberCols  ;
  i4CascadeArraySize = i4CascadeSize*i4CascadeSize ;
//
//-----------------------------------------------------------------------
//
// 3. Allocate memory to dynamic arrays
//
//
  allocate_memory() ;

  // set up the data mask
  int i4ColOffset = (cDom.ncols() - mDom.ncols())/2 ;
  int i4RowOffset = (cDom.nrows() - mDom.nrows())/2 ;
  int i4Row ;
  int i4Col ;
  int i4OutOffset ;
  for ( i4Loop = 0; i4Loop < (size_t)mDom.npoints(); i4Loop++ ) {
     i4Row = mDom.row(i4Loop)+i4RowOffset ;
     i4Col = mDom.col(i4Loop)+i4ColOffset ;
     i4OutOffset = cDom.point(i4Row, i4Col) ;
     i4DataMask[i4OutOffset] = 1 ;
  }

//
//-----------------------------------------------------------------------
//
// 4. Set up the tracking object
//
  int i4InitOption = 0 ; // start the tracking from scratch
  // Note that the tracking object is the size of the cascade not the input map
  sRadarAdvection = new Optical_Flow(i4CascadeSize, i4CascadeSize, r4PixelSize, i4InitOption, cOpticalFileName);

#ifdef VERBOSE_CASCADE
  if ( cLogFileNamePassed != NULL ) {
    strcpy(cLogFileName, cLogFileNamePassed);
    fLogFile = fopen(cLogFileNamePassed, "wt") ;
    fprintf(fLogFile, "Cascade size = %d pixels, minimum scale = %d pixels, number of levels = %d\n", i4CascadeSize, i4CascadeMinLevel, i4NumberLevels) ;
    fprintf(fLogFile,"Random number seed idum = %d",idum);
    hasOpenedLogFile = true ;
  }
#endif
}

BCascade::BCascade(
  char* cBCascadeFileNamePassed,  ///< IN The name of the BCascade input file
  int* i4ReturnCode         ///< OUT Whether this routine has been sucessful
){
/// /fn  BCascade::BCascade(char* cBCascadeFileNamePassed,int* i4ReturnCode)
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: BCascade.cpp
///
/// TYPE: Cascade Constructor
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// Constructor: Hot start the cascade using the files from the previous time step
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Alan Seed       27/03/05        Initial version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// Returns 0 on success, -1 on failure
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
/// FILENAME: cLogFileName
/// NAME OF POINTER TO FILE: fLogFile
/// USAGE: output log
///
/// FILENAME: cBCascadeFileName
/// Name of File Handle-  i4BCascadeFile
///
/// FILENAME: cOpticalFileName
/// Passed to Optical Flow class
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// ARRAY NAME: r4CascadeMean
/// ARRAY TYPE: float**
/// DIMENSIONS: i4NumberCascades, i4NumberRowsPassed*i4NumberColsPassed
///
/// ARRAY NAME: r4CascadeStd
/// ARRAY TYPE: float**
/// DIMENSIONS: i4NumberCascades, i4NumberRowsPassed*i4NumberColsPassed
///
/// ARRAY NAME: r4Correlation
/// ARRAY TYPE: float*
/// DIMENSIONS: i4NumberCascades*i4NumberLevels
///
/// ARRAY NAME: r4Phi
/// ARRAY TYPE: float*
/// DIMENSIONS: i4NumberCascades*i4NumberLevels
///
/// ARRAY NAME: c8FFTout
/// ARRAY TYPE: fftw_complex*
/// DIMENSIONS: i4FFTSize*i4FFTSize
///
/// ARRAY NAME: r8FFTin
/// ARRAY TYPE: double*
/// DIMENSIONS: i4FFTSize*i4FFTSize
///
///
/// ARRAY NAME: r4TempArray
/// ARRAY TYPE: float*
/// DIMENSIONS: i4CascadeSize+1 * i4CascadeSize+1
///
/// ARRAY NAME: r4CascadeStack
/// ARRAY TYPE: float*
/// DIMENSIONS: i4NumberCascades, i4NumberLevels*i4CascadeSize*i4CascadeSize
///
/// ARRAY NAME: r4Maps
/// ARRAY TYPE: float**
/// DIMENSIONS: [i4NumberMaps][i4CascadeArraySize]
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// float r4NoData;                 // flag for non-valid data- must be < 0; -999 is good
/// FILE* fLogFile;                 // Pointer to the log file
/// float* r4FFT ;                  // temp arrays for the fft of the nids
/// float* r4TempArray ;            // Temporary array of image data
/// float* r4CascadeStack;       // cascade arrays
/// float* r4Phi;                   // ar(2) model parameters for updates
/// float* r4Correlation;           // auto correlations for the cascade
/// time_t sMapTimes[8] ;           // list of times for each map
/// float* r4CascadeMean ;          // mean for each level
/// float* r4CascadeStd ;           // standard deviation for each level
/// int i4CascadeSize;              // size of the cascade assumed to be square
/// int i4NumberRows;               // size of the input rain map
/// int i4NumberCols;               // size of the input rain map
/// int i4NumberCascades;              // number of cascades in memory
/// float r4PixelSize;              // pixel size in km
/// int i4ImageNumber;              // number of images since the cascade was initialised
/// int i4NumberLevels;             // image size and number of cascade levels
/// float** r4Maps;                  // array of previous maps
/// int i4FFTArraySize ;            // size of the zero packed array for the fft
/// float *r4FFTweights ;           // used by bandpass filter
///
/// STANDARD LIBRARIES USED:
///
/// stdio.h
/// time.h
/// iostream.h
/// string.h
///
/// </pre>

//***99999999999999999999999999999999999999999999999999999999999999999999
//
// Local variables
//
  size_t i4Loop;                      // Loop variable
  bool status ;                    // Returned by read_cascade_parms
  int i4BCascadeFile ;             // Handle for the cascade file

  *i4ReturnCode   = 0;
  i4NumberLags    = 2 ;
//  i4MapOffset     = 16 ;
  i4MapOffset     = 64 ;
  fLogFile        = NULL ;
  r4Phi           = NULL ;
  r4Correlation   = NULL ;
  inPlan          = NULL ;
  outPlan         = NULL ;
  c4FFTout        = NULL ;
  c4FFT           = NULL ;
  r8FFTin         = NULL ;
  r4TempArray     = NULL ;
  sRadarAdvection = NULL ;
  r4CascadeStack  = NULL ;
  r4RainMap       = NULL ;
  i4DataMask      = NULL ;
  r4BandpassFilter = NULL ;
  pMatch = new ProbMatch() ;
  rand_gen = new gasRV();

  for ( i4Loop = 0; i4Loop < 8; i4Loop++ ) sMapTimes[i4Loop] = 0 ;
  hasOpenedLogFile = false ;
  bIsDet = false ;

  idum = -time(NULL) ;
#ifdef DEBUG
  idum = -2007062100 ;
#endif

// ACB 8/1/2010: Method to set random seed to specific number
//    ( this will allow generation of identical noise between runs)
#ifdef DEBUG
  rand_gen->set_seed(idum);
#endif

  strcpy(cBCascadeFileName, cBCascadeFileNamePassed);

// try to open the cascade class file
  i4BCascadeFile = open(cBCascadeFileName, O_RDONLY|O_BINARY) ;
  if ( i4BCascadeFile == -1 ){
    *i4ReturnCode = -1 ;       // return code for failure
    return ;
  }

// read in the cascade parameters
  status = read_cascade_parameters(i4BCascadeFile) ;
  if ( !status ){
    close (i4BCascadeFile) ;
    *i4ReturnCode = -1 ;  // flag for failure
    return ;
  }
  i4CascadeMinLevel = (int)(CASCADE_MIN_KM/r4PixelSize) ;
  if ( i4CascadeMinLevel < 2 ) i4CascadeMinLevel = 2 ;
  r4CascadeScaleBreak = CASCADE_SCALE_BREAK/r4PixelSize ;  //Scale break in pixels
  mDom.init(i4NumberRows, i4NumberCols) ;
  cDom.init(i4CascadeSize, i4CascadeSize) ;

// allocate the memory
  allocate_memory() ;

  // set up the data mask
  int i4ColOffset = (cDom.ncols() - mDom.ncols())/2 ;
  int i4RowOffset = (cDom.nrows() - mDom.nrows())/2 ;
  int i4Row ;
  int i4Col ;
  int i4OutOffset ;
  for ( i4Loop = 0; i4Loop < (size_t)mDom.npoints(); i4Loop++ ) {
     i4Row = mDom.row(i4Loop)+i4RowOffset ;
     i4Col = mDom.col(i4Loop)+i4ColOffset ;
     i4OutOffset = cDom.point(i4Row, i4Col) ;
     i4DataMask[i4OutOffset] = 1 ;
  }

// Set up the tracking object
  int i4InitOption = 0 ; // continue tracking from previous time step

// Note that the tracking object is the size of the cascade not the input map
  sRadarAdvection = new Optical_Flow( i4CascadeSize, i4CascadeSize, r4PixelSize, i4InitOption, cOpticalFileName);

  status = read_cascade_data(i4BCascadeFile) ;
  if ( !status ){
    close (i4BCascadeFile) ;
    deallocate_memory() ;
    *i4ReturnCode = -1 ;  // flag for failure
    return ;
  }
  close ( i4BCascadeFile ) ;

// Set the probability matching class
//  pMatch->set_reference(r4RainMap, i4InMapArraySize) ;

  // AWS 30 April 2010 Update the probability matching for this image
  int i4NumberValidPixels = 0 ;
  for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ) {
    if ( r4RainMap[i4Loop] > r4NoData+1) {
      r4TempArray[i4NumberValidPixels] = r4RainMap[i4Loop] ;
      i4NumberValidPixels++ ;
    }
  }
  pMatch->set_reference(r4TempArray, i4NumberValidPixels) ;

#ifdef VERBOSE_CASCADE
  if ( cLogFileName[0] != 0 ) {
    fLogFile = fopen(cLogFileName, "wt") ;
    fprintf(fLogFile, "Cascade size = %d pixels, minimum scale = %d pixels, number of levels = %d\n", i4CascadeSize, i4CascadeMinLevel, i4NumberLevels) ;
    fprintf(fLogFile,"Random number seed idum = %d",idum);
    hasOpenedLogFile = true ;
  }
#endif

}

void BCascade::allocate_memory() {
/// \fn void BCascade::allocate_memory()
/// \brief Method to allocate the memory
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: BCascade_110.cpp
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// Allocate the dynamic arrays for the cascade
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Alan Seed       27/03/05        Initial version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///  float *r4Phi           [(i4NumberLags+1)*i4NumberLevels]
///  float *r4Correlation   [i4NumberLags*i4NumberLevels]
///  float **r4Maps         [i4NumberMaps][i4CascadeArraySize]
///  float *r4RainMap       [i4InMapArraySize] ;
///  int   *i4DataMask      [i4CascadeArraySize] ;
///  float **cascadeStack   [i4NumberCascades][i4NumberLevels*i4CascadeSize*i4CascadeSize]
///  fftw_complex *c4FFTout [i4FFTArraySize]
///  fftw_complex *c4FFT    [i4FFTArraySize]
///  double *r8FFTIn        [i4FFTSize*i4FFTSize]
///  float *r4TempArray     [i4CascadeSize*i4CascadeSize]
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
/// int i4NumberCascades
/// int i4NumberLevels
/// int i4NumberMaps
/// int i4CascadeArraySize
/// int i4CascadeSize
/// fftw_plan inPlan ;            // forward fft
/// fftw_plan outPlan ;           // inverse fft
/// int i4FFTSize ;               // logical size of the fft array
/// int i4FFTArraySize ;          // Actual size of the fft array = FFTSize*(1+FFTSize/2)

///
/// </pre>
// Allocate memory for the correlation and phi arrays
//
  int i4Loop2 ;
  int i4Map ;
  size_t i4Loop ;
  int i4Casc ;
  size_t i4ArraySize ;

  // AWS 1 June 2009 Always assume AR(2) here so that estimating the nowcast skill in merged_cascade works for AR(1)

  //AWS 21 Dec 2010 - clean up memory allocation
  //  r4Phi = new float [3*i4NumberLevels];
  //  for ( i4Loop2=0; i4Loop2 < 3*i4NumberLevels; i4Loop2++ ) r4Phi[i4Loop2] = 0 ;
  r4Phi = new float [(i4NumberLags+1)*i4NumberLevels];
  for ( i4Loop2=0; i4Loop2 < (i4NumberLags+1)*i4NumberLevels; i4Loop2++ ) r4Phi[i4Loop2] = 0 ;

  r4Correlation = new float[2*i4NumberLevels];
  for ( i4Loop2=0; i4Loop2<2*i4NumberLevels; i4Loop2++ ) {
    r4Correlation[i4Loop2] = r4NoData ;
  }

// Allocate memory for the input map arrays
  for ( i4Map = 0; i4Map < 8; i4Map++ ){
    r4Maps[i4Map] = NULL ;
  }

  for ( i4Map = 0; i4Map < i4NumberMaps; i4Map++ ) {
    r4Maps[i4Map] = new float [i4CascadeArraySize] ; // put the input map into an array 2^n square
    for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ){
      r4Maps[i4Map][i4Loop] = r4NoData ;
    }
  }

  r4RainMap  = new float [i4InMapArraySize] ;
  r4RainMap1 = new float [i4InMapArraySize] ;
  i4DataMask = new int   [i4CascadeArraySize] ;

  for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ) r4RainMap[i4Loop] = 0 ;
  for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ) r4RainMap1[i4Loop] = 0 ;
  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ) i4DataMask[i4Loop] = 0 ;

// Allocate memory for the cascade arrays
  r4CascadeStack = new float *[i4NumberCascades] ;
  i4ArraySize = i4NumberLevels*i4CascadeArraySize ;
  for ( i4Casc = 0; i4Casc < i4NumberCascades; i4Casc++ ) {
    r4CascadeStack[i4Casc] = new float [i4ArraySize] ;
    for ( i4Loop = 0; i4Loop < i4ArraySize; i4Loop++ ) r4CascadeStack[i4Casc][i4Loop] = 0 ;
  }

// Allocate memory for the temporary array
  i4ArraySize = i4CascadeSize*i4CascadeSize ;
  r4TempArray = new float [i4ArraySize] ;
  for ( i4Loop = 0; i4Loop < i4ArraySize; i4Loop++ ) r4TempArray[i4Loop] = 0 ;

// Allocate the memory for the fft

// AWS START 12 Jan 2009 Increase the size of the FFT array to improve the FFT calculations
//  i4FFTSize = i4CascadeSize ;
  i4FFTSize = 2*i4CascadeSize ;
// AWS END 12 Jan 2009

  i4FFTArraySize = i4FFTSize*(1 + i4FFTSize/2)  ;
  c4FFTout  = (fftw_complex*) fftw_malloc (sizeof(fftw_complex)*i4FFTArraySize) ;
  c4FFT     = (fftw_complex*) fftw_malloc (sizeof(fftw_complex)*i4FFTArraySize) ;
  r8FFTin   = (double *) fftw_malloc ( sizeof(double)*i4FFTSize*i4FFTSize ) ;

  //#ifdef LINUX_HOST
  //inPlan  = fftw_plan_dft_r2c_2d(i4FFTSize, i4FFTSize, r8FFTin, c4FFTout, FFTW_MEASURE );
  //outPlan = fftw_plan_dft_c2r_2d(i4FFTSize, i4FFTSize, c4FFTout, r8FFTin, FFTW_MEASURE );
  //#else
  inPlan  = fftw_plan_dft_r2c_2d(i4FFTSize, i4FFTSize, r8FFTin, c4FFTout, fftw_util_get_plan_flags() );
  outPlan = fftw_plan_dft_c2r_2d(i4FFTSize, i4FFTSize, c4FFTout, r8FFTin, fftw_util_get_plan_flags() );
  //#endif

  // 2D bandpass filter
  int i4FFTStride = 1 + i4FFTSize / 2;
  r4BandpassFilter = new float* [i4NumberLevels];
  for (int i = 0; i < i4NumberLevels; ++i)
    r4BandpassFilter[i] = new float[i4FFTStride * i4FFTStride];

  calculate_bandpass_cascade_filter();
}

BCascade::~BCascade( ) {
/// \fn BCascade::~BCascade( )
/// \brief BCascade destructor
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: BCascade.cpp
///
/// TYPE: Class destructor
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To delete the class
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Alan Seed       27/03/05        Initial version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
/// NOTES:
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
///
/// STANDARD LIBRARIES USED:
///
/// </pre>
  bool status = write_cascade(cBCascadeFileName) ;
  if ( !status ) {
#ifdef VERBOSE_CASCADE
    if ( fLogFile != NULL )
		  fprintf(fLogFile, "ERROR: when opening the parms file %s\n", cBCascadeFileName) ;
#endif
  }
  deallocate_memory() ;
}

void BCascade::deallocate_memory() {
/// \fn  void BCascade::deallocate_memory()
/// \brief Deallocate the class memory
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: BCascade.cpp
///
/// TYPE: Class destructor
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To delete the memory allocated by the constructor
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Alan Seed       27/03/05        Initial version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
/// NOTES:
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
///
/// STANDARD LIBRARIES USED:
///
/// </pre>
  int i4Casc ;
  int i4Map ;

// Delete temporary data array
  if ( r4TempArray != NULL ) {
		delete [] r4TempArray;
    r4TempArray = NULL;
  }

// Delete the stack of cascade data
  if ( r4CascadeStack != NULL ) {
    for ( i4Casc = 0; i4Casc < i4NumberCascades; i4Casc++ ) {
      if ( r4CascadeStack[i4Casc] != NULL ){
	      delete [] r4CascadeStack[i4Casc]  ;
        r4CascadeStack[i4Casc] = NULL ;
      }
    }
    delete [] r4CascadeStack ;
    r4CascadeStack = NULL ;
  }

// Delete Fourier transform arrays and plans
  if ( inPlan != NULL ) {
    fftw_destroy_plan(inPlan) ;
    inPlan = NULL ;
  }

  if ( outPlan != NULL ) {
    fftw_destroy_plan(outPlan) ;
    outPlan = NULL ;
  }

  if ( c4FFTout != NULL ){
    fftw_free(c4FFTout) ;
    c4FFTout = NULL ;
  }

  if ( c4FFT != NULL ){
    fftw_free(c4FFT) ;
    c4FFT = NULL ;
  }

  if ( c4FFT != NULL ){
    fftw_free(c4FFT) ;
    c4FFT = NULL ;
  }

  if ( r8FFTin != NULL ) {
    fftw_free(r8FFTin) ;
    r8FFTin = NULL ;
  }

// Delete the arrays for rainfall and data mask
  if ( r4RainMap != NULL ) {
    delete [] r4RainMap ;
    r4RainMap = NULL ;
  }
  if ( r4RainMap1 != NULL ) {
    delete [] r4RainMap1 ;
    r4RainMap1 = NULL ;
  }

  if ( i4DataMask != NULL ) {
    delete [] i4DataMask ;
    i4DataMask = NULL ;
  }

// Delete the maps array
  for ( i4Map = 0; i4Map < i4NumberMaps; i4Map++ ){
    if ( r4Maps[i4Map] != NULL ) {
      delete [] r4Maps[i4Map] ;
      r4Maps[i4Map]= NULL ;
    }
  }

// Delete the AR(2) parameters array
  if ( r4Phi != NULL ) {
    delete [] r4Phi;
    r4Phi = NULL;
  }

// Delete correlation coefficients array
  if ( r4Correlation != NULL ) {
    delete [] r4Correlation;
    r4Correlation = NULL;
  }

// Delete the tracking object
  if ( sRadarAdvection != NULL ) {
    delete sRadarAdvection ;
    sRadarAdvection = NULL ;
  }

// Close the log file
  if ( hasOpenedLogFile ){
    if ( fLogFile != NULL ) {
      fclose(fLogFile) ;
      fLogFile = NULL ;
      hasOpenedLogFile = false ;
    }
  }

  if ( r4BandpassFilter != NULL )
  {
    for (int i = 0; i < i4NumberLevels; ++i)
      delete [] r4BandpassFilter[i];
  }
  delete [] r4BandpassFilter ;
  r4BandpassFilter = NULL ;

  if ( pMatch != NULL ) delete pMatch ;
	if ( rand_gen != NULL ) delete rand_gen ;
}

float BCascade::get_scaleRatio(){
  return r4ScaleRatio ;
}

bool BCascade::have_parameters() {
/// \fn bool BCascade::have_parameters()
/// \brief Returns true if there are valid AR(2) parameters
  if ( i4ImageNumber < 1 ) return false ;
  else return true ;
}

void BCascade::get_DataMask(
  int *inDataMask  ///< Mask of valid data, int [i4CascadeArraySize]. 1 = valid, 0 = not valid
) {
/// \fn void BCascade::get_DataMask( int *inDataMask )
/// \brief Get the mask for the area of valid data in the cascade
  size_t i4Loop ;
  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ )
    inDataMask[i4Loop] = i4DataMask[i4Loop]  ;
}

void BCascade::set_DataMask(
  int *inDataMask  ///< Mask of valid data, int [i4CascadeArraySize]. 1 = valid, 0 = not valid
) {
/// \fn void BCascade::set_DataMask( int *inDataMask )
/// \brief Set the mask for the area of valid data in the cascade
  size_t i4Loop ;
  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ )
    i4DataMask[i4Loop] = inDataMask[i4Loop] ;
}

float *BCascade::get_stackP(
  int cascadeNumber ///< Cascade number
){
  /// \fn float *BCascade::get_stackP(int cascadeNumber)
  /// \brief Return a float * pointer to the start of a cascade
	return r4CascadeStack[cascadeNumber] ;
}

float BCascade::get_RainMean( ) {
  /// \fn float BCascade::get_RainMean( )
  /// \brief Return the mean rain rate (mm/h) over the input map
  return r4RainMean ;
}

float BCascade::get_RainStd() {
  /// \fn float BCascade::get_RainStd()
  /// \brief Return the standard deviation of the rain rate (mm/h) over the input image
  return r4RainStd ;
}

FILE *BCascade::get_logFileP() {
  /// \fn FILE *BCascade::get_logFileP()
  /// \brief Retun the FILE * pointer to the file used for logging information
  return fLogFile ;
}

void BCascade::set_logFileP ( FILE *fileP ) {
  /// \fn void BCascade::set_logFileP ( FILE *fileP )
  /// \brief Set the FILE * pointer to a file that has been opened by the main program
  if ( fLogFile != NULL ) {
    fclose( fLogFile) ;
    fLogFile = NULL ;
  }
  fLogFile = fileP ;
  hasOpenedLogFile = false ;
}

void BCascade::get_stack(
  float *stackP     ///< OUT: Copy of cascade returned in array float[i4NumberCascades*i4CascadeSize]
) {
/// \fn void BCascade::get_stack( float *stackP )
/// \brief Copy the cascades into the array stackP
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: BCascade.cpp
///
/// TYPE: Class method
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To return a copy of the cascade
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Alan Seed       27/03/05        Initial version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
/// NOTES:
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///  float** r4CascadeStack;              // array of pointers to the cascade arrays
///  int i4CascadeArraySize ;
///  int i4NumberCascades ;
///  int i4NuberLevels ;
///
///
/// STANDARD LIBRARIES USED:
///
/// </pre>
  int i4Loop ;
  int i4Cascade ;
  int i4StackOffset = 0 ;
  for ( i4Cascade = 0; i4Cascade < i4NumberCascades; i4Cascade++ ) {
    for ( i4Loop = 0; i4Loop < (int)(i4CascadeArraySize*i4NumberLevels); i4Loop++, i4StackOffset++ ){
      stackP[i4StackOffset] = r4CascadeStack[i4Cascade][i4Loop] ;
    }
  }
}

void BCascade::set_stack(
  float *stackP ///< IN: Array of cascade values, float[i4NumberCascades*i4CascadeSize]
) {
/// \fn void BCascade::set_stack( float *stackP )
/// \brief Set the cascades using the array stackP
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: Class method
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To set the cascade
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Alan Seed       27/03/05        Initial version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
/// NOTES:
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///  float** r4CascadeStack;              // array of pointers to the cascade arrays
///  int i4CascadeArraySize ;
///  int i4NumberCascades ;
///  int i4NuberLevels ;
///
///
/// STANDARD LIBRARIES USED:
///
/// </pre>
  int i4Loop ;
  int i4Cascade ;
  int i4StackOffset = 0 ;
  for ( i4Cascade = 0; i4Cascade < i4NumberCascades; i4Cascade++ ) {
    for ( i4Loop = 0; i4Loop < (int)(i4CascadeArraySize*i4NumberLevels); i4Loop++, i4StackOffset++ ){
      r4CascadeStack[i4Cascade][i4Loop] = stackP[i4StackOffset]  ;
    }
  }
}

void BCascade::get_std(
  float *std  ///< OUT Array with cascade level standard deviations, float[i4NumberLevels]
){
/// \fn void BCascade::get_std(float *std)
/// \brief Get the standard deviation of each cascade level
  int i4Loop ;
  for ( i4Loop = 0; i4Loop < i4NumberLevels; i4Loop++ ) std[i4Loop] = r4CascadeLevelStd[i4Loop] ;
}

void BCascade::get_mean(float *mean){
/// \fn void BCascade::get_mean(float *mean)
/// \brief Get the mean of each cascade level
  int i4Loop ;
  for ( i4Loop = 0; i4Loop < i4NumberLevels; i4Loop++ ) mean[i4Loop] = r4CascadeLevelMean[i4Loop] ;
}

int BCascade::get_cascade_levels() {
/// \fn  int BCascade::get_cascade_levels()
/// \brief Return the number of levels in the cascade
	return i4NumberLevels ;
}

float BCascade::get_fieldMean() {
/// \fn float BCascade::get_fieldMean()
/// \brief Return the mean of the current field in 10log10(mm/h)
	return r4FieldMean ;
}

float BCascade::get_fieldStd() {
/// \fn float BCascade::get_fieldStd()
/// \brief Return the standard deviation of the current field in 10log10(mm/h)
	return r4FieldStd ;
}

float BCascade::get_rainFrac() {
/// \fn float BCascade::get_rainFrac()
/// \brief Return the fraction of the current field that exceeds 0.1 mm/h
  return r4RainFrac ;
}

float BCascade::get_condMean() {
/// \fn float BCascade::get_condMean()
/// \brief Return the mean of all pixels > 0.1 mm/h the current field
  return r4CondMean ;
}

int BCascade::get_cascade_size(){
/// \fn int BCascade::get_cascade_size()
/// \brief Returns the number of rows/columns in the cascade
  return i4CascadeSize ;
}

float BCascade::get_noData() {
/// \fn float BCascade::get_noData()
/// \brief Return the missing data flag
  return r4NoData ;
}

void BCascade::get_parameters(
  float *r4StdLevelPassed,  ///< OUT: Array of cascade level standard deviations, float[i4NumberLevels]
  float *r4PhiPassed,       ///< OUT: Array of AR(2) parameters, float[i4NumberLevels*(i4NumberLags+1)]
  float &r4FieldMeanPassed, ///< OUT: Mean, 10*log10(mm/h)
  float &r4FieldStdPassed,  ///< OUT: Standard Deviation, 10*log10(mm/h)
  float &r4BetaOnePassed,   ///< OUT: Slope of the power spectrum for scales > scale break
  float &r4BetaTwoPassed    ///< OUT: Slope of the power spectrum for scales < scale break
   ) {
/// \fn void BCascade::get_parameters(float *r4StdLevelPassed,float *r4PhiPassed,float &r4FieldMeanPassed,float &r4FieldStdPassed,float &r4BetaOnePassed,float &r4BetaTwoPassed)
/// \brief Returns the current cascade parameters
  int i4Loop ;
  for ( i4Loop = 0; i4Loop < i4NumberLevels*(i4NumberLags+1); i4Loop++ ) r4PhiPassed[i4Loop] = r4Phi[i4Loop] ;
  for ( i4Loop = 0; i4Loop < i4NumberLevels; i4Loop++ ) r4StdLevelPassed[i4Loop] = r4CascadeLevelStd[i4Loop] ;
  r4FieldMeanPassed = r4FieldMean ;
  r4FieldStdPassed  = r4FieldStd ;
  r4BetaOnePassed = r4BetaOne ;
  r4BetaTwoPassed = r4BetaTwo ;
}

void BCascade::set_parameters(
  float *r4StdLevelPassed,  ///< OUT: Array of cascade level standard deviations, float[i4NumberLevels]
  float *r4PhiPassed,       ///< OUT: Array of AR(2) parameters, float[i4NumberLevels*(i4NumberLags+1)]
  float r4FieldMeanPassed, ///< OUT: Mean, 10*log10(mm/h)
  float r4FieldStdPassed,  ///< OUT: Standard Deviation, 10*log10(mm/h)
  float r4BetaOnePassed,   ///< OUT: Slope of the power spectrum for scales > scale break
  float r4BetaTwoPassed    ///< OUT: Slope of the power spectrum for scales < scale break
   ) {
/// \fn void BCascade::set_parameters(float *r4StdLevelPassed,float *r4PhiPassed,float &r4FieldMeanPassed,float &r4FieldStdPassed,float &r4BetaOnePassed,float &r4BetaTwoPassed)
/// \brief Set the current cascade parameters
  int i4Loop ;
  for ( i4Loop = 0; i4Loop < i4NumberLevels*(i4NumberLags+1); i4Loop++ ) r4Phi[i4Loop] = r4PhiPassed[i4Loop]  ;
  for ( i4Loop = 0; i4Loop < i4NumberLevels; i4Loop++ )r4CascadeLevelStd[i4Loop] = r4StdLevelPassed[i4Loop] ;
  r4FieldMean = r4FieldMeanPassed ;
  r4FieldStd  = r4FieldStdPassed  ;
  r4BetaOne   = r4BetaOnePassed ;
  r4BetaTwo   = r4BetaTwoPassed ;
}

float BCascade::get_pixelSize() {
/// \fn float BCascade::get_pixelSize()
/// \brief Get the size of the input map pixel (km)
  return r4PixelSize ;
}

void BCascade::update_maps(
	float* r4InputMap,     ///< IN: The latest map data, float[i4NumberCols*i4NumberRows]
	time_t sThisTime,      ///< IN: The time of the latest data, seconds past 1970
	bool lReset            ///< IN: Whether the parameter estimation should be reset
) {
/// \fn  void BCascade::update_maps(float* r4InputMap,time_t sThisTime,bool lReset)
/// \brief Update the cascade class with a new rainfall field
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// Takes the latest data (in rainfall) and puts this on top of the stack of
/// maps.  Calculates the average (and variance) of the map
/// The map is converted to dbr and placed into an array that is a 2^n square
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Alan Seed       ??/??/??        Initial version
/// 1.10         Alan Seed       17/08/05        Calculate the spatial standard deviation
/// 1.20         Alan Seed       29/05/09        Keep the untransformed rainfal intensity map
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
/// Overloaded operator
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// float* r4RainMap ;     // untransformed rainfal intensity map
/// float* r4Maps[8];      // array of previous transformed rainfall maps
/// float r4FieldMean ;    // Mean value of the transformed rain rate
/// float r4FieldStd  ;    // standard deviation of the rain rate
/// int i4NumberMaps;      // number of rain fields in memory
/// int i4ImageNumber;     // number of images since the cascade was initialised
/// time_t sMapTimes[8] ;  // list of times for each map
/// size_t i4InMapArraySize ;      // Array size for input data
/// size_t i4CascadeArraySize ;    // Size of the cascade array
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>

// Local variables
//
  int i4Map;                // Loop variable, map number
  int i4Casc;               // Loop variable, the number of the cascade
  int i4NumberPixels;       // The pixels containing data
  int i4NumberRainPixels;   // The pixels containing rainfall
  float r4Variance;         // Variance of the rain image
  size_t i4Loop ;
  float tVal ;
  float *r4DbrMap = new float [i4CascadeArraySize] ;

// Set the image number
  if ( lReset ) {
      i4ImageNumber = 0;
  } else {
      i4ImageNumber++ ;
  }

// Move the current rainfall map back
  for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
    r4RainMap1[i4Loop] = r4RainMap[i4Loop] ;
  }

// Copy the current rainfall map into r4RainMap
  for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
    r4RainMap[i4Loop] = r4InputMap[i4Loop] ;
  }

// Calculate the mean and variance of the rain rates
  r4RainMean = 0 ;
  r4Variance  = 0 ;
  i4NumberPixels = 0 ;
  i4NumberRainPixels = 0 ;
  r4CondMean = 0 ;
  for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
    tVal = r4InputMap[i4Loop] ;
    if ( tVal >= 0.1 ) {
      r4CondMean += tVal ;
      i4NumberRainPixels++ ;
    }
    if ( tVal > r4NoData+1 ) {
      r4RainMean += tVal ;
      i4NumberPixels++;
    }
  }

  r4RainFrac = (float)i4NumberRainPixels / (float)i4NumberPixels ;
  r4RainMean /= (float)i4NumberPixels ;

// AWS 20 Jan 2011 Only do this if there is rain
  if ( r4RainFrac < 0.03 ) {
    r4CondMean = 0.0 ;
  } else {
    r4CondMean /= (float)i4NumberRainPixels ;
  }

  for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
    tVal = r4InputMap[i4Loop] ;
    if ( tVal > r4NoData+1 ) {
      r4Variance += (tVal- r4RainMean) * (tVal- r4RainMean) ;
    }
  }
  r4Variance /= (float)i4NumberPixels  ;
  r4RainStd = sqrt( r4Variance )  ;

// Normalise and then convert the rain intensity into dbr
  for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
    r4DbrMap[i4Loop] = r4InputMap[i4Loop] ;
	}
	convert_mmh_dbr(true, r4NoData, i4NumberRows, i4NumberCols, r4DbrMap) ;

// Calculate the mean and variance of the normalised dBr
  r4FieldMean = 0 ;
  r4Variance  = 0 ;
  i4NumberPixels = 0 ;
  for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
    tVal = r4DbrMap[i4Loop] ;
    if ( tVal > r4NoData+1 ) {
      r4FieldMean += tVal ;
      i4NumberPixels++;
    }
  }
  r4FieldMean /= (float)i4NumberPixels ;  // mean in dbr
  for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
    tVal = r4DbrMap[i4Loop] ;
    if ( tVal > r4NoData+1 ) {
      r4Variance += (tVal- r4FieldMean) * (tVal- r4FieldMean) ;
    }
  }
  r4Variance /= (float)i4NumberPixels  ;
  r4FieldStd = sqrt( r4Variance )  ;

// Move maps down the stack (newest sits on top)
  for ( i4Map = i4NumberMaps-1; i4Map > 0; i4Map-- )  {
    for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ )r4Maps[i4Map][i4Loop] = r4Maps[i4Map-1][i4Loop] ;
  }
  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++)r4Maps[0][i4Loop] = 0 ;

// Put the map into the class array and set the valid data mask
  to_cascade(r4DbrMap, r4Maps[0]) ;

// Update the array of map times
  for ( i4Casc=i4NumberMaps-1; i4Casc>=1; i4Casc-- )
    sMapTimes[i4Casc] = sMapTimes[i4Casc-1] ;
  sMapTimes[0] = sThisTime ;


  delete [] r4DbrMap ;
}

void BCascade::update_maps_dbz(
	float* r4InputMap,     ///< IN: The latest map data, float[i4NumberCols*i4NumberRows]
	time_t sThisTime,      ///< IN: The time of the latest data, seconds past 1970
	bool lReset            ///< IN: Whether the parameter estimation should be reset
) {
/// \fn  void BCascade::update_maps(float* r4InputMap,time_t sThisTime,bool lReset)
/// \brief Update the cascade class with a new rainfall field
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// Takes the latest data (in rainfall) and puts this on top of the stack of
/// maps.  Calculates the average (and variance) of the map
/// The map is converted to dbr and placed into an array that is a 2^n square
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Alan Seed       ??/??/??        Initial version
/// 1.10         Alan Seed       17/08/05        Calculate the spatial standard deviation
/// 1.20         Alan Seed       29/05/09        Keep the untransformed rainfal intensity map
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
/// Overloaded operator
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// float* r4RainMap ;     // untransformed rainfal intensity map
/// float* r4Maps[8];      // array of previous transformed rainfall maps
/// float r4FieldMean ;    // Mean value of the transformed rain rate
/// float r4FieldStd  ;    // standard deviation of the rain rate
/// int i4NumberMaps;      // number of rain fields in memory
/// int i4ImageNumber;     // number of images since the cascade was initialised
/// time_t sMapTimes[8] ;  // list of times for each map
/// size_t i4InMapArraySize ;      // Array size for input data
/// size_t i4CascadeArraySize ;    // Size of the cascade array
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>

// Local variables
//
	int i4Map;                // Loop variable, map number
	int i4Casc;               // Loop variable, the number of the cascade
	int i4NumberPixels;       // The pixels containing data
	int i4NumberRainPixels;   // The pixels containing rainfall
	float r4Variance;         // Variance of the rain image
	size_t i4Loop ;
	float tVal ;
	float *r4DbrMap = new float [i4CascadeArraySize] ;

// Set the image number
	if ( lReset ) {
			i4ImageNumber = 0;
	} else {
			i4ImageNumber++ ;
	}

// Move the current rainfall map back
	for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
		r4RainMap1[i4Loop] = r4RainMap[i4Loop] ;
	}

// Copy the current rainfall map into r4RainMap
	for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
		r4RainMap[i4Loop] = r4InputMap[i4Loop] ;
	}

// Calculate the mean and variance of the rain rates
	r4RainMean = 0 ;
	r4Variance  = 0 ;
	i4NumberPixels = 0 ;
	i4NumberRainPixels = 0 ;
	r4CondMean = 0 ;
	for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
		tVal = r4InputMap[i4Loop] ;
		if ( tVal >= 0.1 ) {
			r4CondMean += tVal ;
			i4NumberRainPixels++ ;
		}
		if ( tVal > r4NoData+1 ) {
			r4RainMean += tVal ;
			i4NumberPixels++;
		}
	}

	r4RainFrac = (float)i4NumberRainPixels / (float)i4NumberPixels ;
	r4RainMean /= (float)i4NumberPixels ;

// AWS 20 Jan 2011 Only do this if there is rain
	if ( r4RainFrac < 0.03 ) {
		r4CondMean = 0.0 ;
	} else {
		r4CondMean /= (float)i4NumberRainPixels ;
	}

	for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
		tVal = r4InputMap[i4Loop] ;
		if ( tVal > r4NoData+1 ) {
			r4Variance += (tVal- r4RainMean) * (tVal- r4RainMean) ;
		}
	}
	r4Variance /= (float)i4NumberPixels  ;
	r4RainStd = sqrt( r4Variance )  ;

// Normalise and then convert the rain intensity into dbr
	for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
		r4DbrMap[i4Loop] = r4InputMap[i4Loop] ;
	}

// Calculate the mean and variance of the normalised dBr
	r4FieldMean = 0 ;
	r4Variance  = 0 ;
	i4NumberPixels = 0 ;
  for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
    tVal = r4DbrMap[i4Loop] ;
    if ( tVal > r4NoData+1 ) {
      r4FieldMean += tVal ;
      i4NumberPixels++;
    }
  }
  r4FieldMean /= (float)i4NumberPixels ;  // mean in dbr
  for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ){
    tVal = r4DbrMap[i4Loop] ;
    if ( tVal > r4NoData+1 ) {
      r4Variance += (tVal- r4FieldMean) * (tVal- r4FieldMean) ;
    }
  }
  r4Variance /= (float)i4NumberPixels  ;
  r4FieldStd = sqrt( r4Variance )  ;

// Move maps down the stack (newest sits on top)
  for ( i4Map = i4NumberMaps-1; i4Map > 0; i4Map-- )  {
    for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ )r4Maps[i4Map][i4Loop] = r4Maps[i4Map-1][i4Loop] ;
  }
  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++)r4Maps[0][i4Loop] = 0 ;

// Put the map into the class array and set the valid data mask
  to_cascade(r4DbrMap, r4Maps[0]) ;

// Update the array of map times
  for ( i4Casc=i4NumberMaps-1; i4Casc>=1; i4Casc-- )
    sMapTimes[i4Casc] = sMapTimes[i4Casc-1] ;
  sMapTimes[0] = sThisTime ;


  delete [] r4DbrMap ;
}

void BCascade::update_parameters() {
/// \fn   void BCascade::update_parameters()
/// \brief Update the cascade parameters
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: Class method
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To calculate the tracking, AR(2), and power spectrum parameters
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Alan Seed       27/03/05        Initial version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
/// NOTES:
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///  float* r4CascadeStack[0]
///  int i4CascadeArraySize
///  int i4NumberCascades
///  int i4NuberLevels
///  float *r4Maps[0]
///  float r4CascadeLevelMean[16]
///  float r4CascadeLevelStd[16]
///  float r4BetaOne
///  float r4BetaTwo
///  Optical_Flow *sRadarAdvection
///  float *r4Correlation
///  float *r4Phi
///
///
/// STANDARD LIBRARIES USED:
///
/// </pre>
  int i4Lag ;
  int i4Loop ;
  char cMapDate[32] ;


  //Update the probability matching for this image
  int i4NumberValidPixels = 0 ;
  for ( i4Loop = 0; i4Loop < (int)i4InMapArraySize; i4Loop++ ) {
    if ( r4RainMap[i4Loop] > r4NoData+1) {
      r4TempArray[i4NumberValidPixels] = r4RainMap[i4Loop] ;
      i4NumberValidPixels++ ;
    }
  }
  pMatch->set_reference(r4TempArray, i4NumberValidPixels) ;

  for ( i4Lag=i4NumberCascades-1; i4Lag>=1; i4Lag-- ) {
    for ( i4Loop = 0; i4Loop < (int)(i4CascadeArraySize*i4NumberLevels); i4Loop++ )
       r4CascadeStack[i4Lag][i4Loop] = r4CascadeStack[i4Lag-1][i4Loop] ;
  }
  for ( i4Loop = 0; i4Loop < (int)(i4CascadeArraySize*i4NumberLevels); i4Loop++ )
       r4CascadeStack[0][i4Loop] = 0 ;

// Decompose the field and place at the top of the cascade stack
  decompose_mask(r4Maps[0], r4CascadeLevelMean, r4CascadeLevelStd, r4CascadeStack[0] ) ;

// Update the tracking information if we have 2 or more images
  if ( i4ImageNumber == 0 ){
    return ;
  }
  update_track(sRadarAdvection) ;

// Update the autocorrelation and AR parameters if we have enough data
  if ( i4ImageNumber < i4NumberCascades-2 ) return ;

// Use defaults if there is not enough rain
  if (r4RainFrac > 0.03) {
    update_autocor(sRadarAdvection) ;
    update_phi( r4Correlation, r4Phi ) ;
  }
  else {
    set_default_autocor() ;
  }

  if ( fLogFile != NULL ) {
    to_date(sMapTimes[0], cMapDate) ;
    fprintf(fLogFile, "\n %s, dbr mean = %f, dbr var = %f\n", cMapDate, r4FieldMean, r4FieldStd*r4FieldStd ) ;
    for ( i4Loop = 0; i4Loop < i4NumberLevels; i4Loop++ ) {
      fprintf(fLogFile, "level %d std = %f\n", i4Loop, r4CascadeLevelStd[i4Loop] ) ;
    }
    if ( i4NumberLags == 2 ) {
      for ( i4Loop = 0; i4Loop < i4NumberLevels; i4Loop++ ) {
        fprintf(fLogFile,"level = %d, lag 1 r = %f, lag 2 r = %f \n", i4Loop, r4Correlation[i4Loop], r4Correlation[i4Loop+i4NumberLevels] ) ;
      }
    }
    else {
      for ( i4Loop = 0; i4Loop < i4NumberLevels; i4Loop++ ) {
        fprintf(fLogFile,"level = %d, lag 1 r = %f \n", i4Loop, r4Correlation[i4Loop] ) ;
      }
    }
  }

// AWS 26 August 2011 Remove this message
//  else {
//    cout << "Log File not open" << endl ;
//  }

}

void BCascade::decompose_fftw(
  float*  r4InputMap,            ///< IN: The data to be decomposed, float[i4NumberCols*i4NumberRows]
  float*  r4CascadeLevelMeanOut, ///< OUT: The mean value of the cascade, at each level and every point
  float*  r4CascadeLevelStdOut,  ///< OUT: The standard deviation of the cascade, at each level and every point
  float*  r4Cascade              ///< OUT: The decomposed cascade, float[i4CascadeArraySize*i4NumberLevels]
) {
/// \fn void BCascade::decompose_fftw(float *r4InputMap,float *r4CascadeLevelMeanOut,float *r4CascadeLevelStdOut,float *r4Cascade )
/// \brief Decompose the rainfall field into spectral components
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To decompose the rainfall field into its spectral components
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed       21/02/05     Initial version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
/// 1. Remove the no-data flag and subtract the field mean
/// 2. Take the Fourier transform using fftw
/// 3. For each level, filter the transform and place the data into the
///    cascade using bandpass_filter_fftw
/// 4. Calculate the mean and standard deviation of each level, not using
///    variable blocks this time
///  5. Reduce the standard deviation of the lowest level and censor to +- 3
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
/// None
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// ARRAY NAME: c4FFT alocated using fftw_malloc
/// ARRAY TYPE: fftw_complex*
///
/// ARRAY NAME: i4Mask
/// ARRAY TYPE: float*
/// DIMENSIONS: i4NumberRows*i4NumberCols
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// float r4NoData;          // flag for non-valid data
/// int i4NumberRows;        // size of the input rain map
/// int i4NumberCols;        // size of the input rain map
/// int i4NumberLevels;      // image size and number of cascade levels
/// fftw_complex* c4FFTout;  // output array for fftw
/// double r8FFTin ;         // input array for fftw
/// int i4FFTSize;           // size of the fft arrays for spatial smoothing
/// int i4FFTArraySize ;     // size of the zero packed array for the fft
/// int i4InMapArraySize ;   // size of the input maps
/// int i4CascadeArraySize ; // size of one level of the cascade
/// float* r4TempArray ;     // Temporary array of image data
/// FILE* fCascadeFile[8];   // Pointer to the cascade weights files
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
/// </pre>
  int i4Level;                                       // Loop counter, level in the cascade
  int i4Row;                                         // Loop counter, horizontal position in analyses
  int i4Col;                                         // Loop counter, vertical position in analyses
  int i4NumberValidData ;                            // Number of valid pixels in the input map

  size_t i4Loop ;                                    // Loop counter, over the array
  size_t i4FFTOffset;                                // Offset into the FFT array
  size_t i4CascadeOffset ;                           // Offset into the cascade

  float startMask  ;                                 // Largest scale to be kept by filter
  float centreMask ;                                 // Middle scale to be kept by filter
  float endMask    ;                                 // Smallest scale to be kept by filter
  float r4FFTFactor;                                 // Need to multiply by this factor to regain data
  float r4CascadeSum ;                               // Sum of the cascade levels
  float r4Residual ;                                 // Residual of input field and cascade sum

//
//-----------------------------------------------------------------------
//
// 1. Remove the no data flag and subtract the field mean
//
  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ){
    r4TempArray[i4Loop] = i4DataMask[i4Loop]*(r4InputMap[i4Loop])  ;
  }

  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ){
    if ( r4TempArray[i4Loop] < r4NoData+1 ) r4TempArray[i4Loop] = 0 ;
  }

  domain2d fDom ;
  fDom.init(i4FFTSize, i4FFTSize) ;

  r4FFTFactor    = 1.0 / (float)(i4FFTSize*i4FFTSize) ;	// inverse fft multiplies by this factor
  startMask      = i4FFTSize/r4ScaleRatio   ;
  centreMask     = i4FFTSize ;
  endMask        = i4FFTSize*r4ScaleRatio ;
//
//-----------------------------------------------------------------------
//
// 2. Calculate the FFT
//
  i4Row = 0 ;
  i4NumberValidData = 0 ;
  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ) i4NumberValidData += i4DataMask[i4Loop] ;
  for ( i4Loop = 0; i4Loop < (size_t)(i4FFTSize*i4FFTSize); i4Loop++ ) r8FFTin[i4Loop] = 0 ;

// load the image into the top left corner of the input array
  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ) {
    i4Row = cDom.row(i4Loop) ;
    i4Col = cDom.col(i4Loop) ;
    i4FFTOffset = fDom.point(i4Row,i4Col) ;
    r8FFTin[i4FFTOffset] = r4TempArray[i4Loop]  ;
	}

// Take the Fourier transform
  fftw_execute(inPlan) ;

// and load the output into an array so that we can work on it
  for ( i4Loop = 0; i4Loop < i4FFTArraySize; i4Loop++ ){
   c4FFT[i4Loop][0] = c4FFTout[i4Loop][0] ;
   c4FFT[i4Loop][1] = c4FFTout[i4Loop][1] ;
  }

// Calculate the power spectrum while we have the FFT handy
  calc_power_exponent(c4FFT, r4BetaOne, r4BetaTwo) ;
  if ( fLogFile != NULL ) fprintf( fLogFile,
        "\nInformation: BCascade; r4BetaOne = %f r4BetaTwo = %f\n", r4BetaOne, r4BetaTwo);

//
//-------------------------------------------------------------------------
//
// 3. For each level, filter the transform and place the data into the
// cascade
//
	for ( i4Level=0; i4Level<i4NumberLevels; i4Level++ ) {
		bandpass_filter_fftw(c4FFT, i4FFTSize, startMask, centreMask, endMask, c4FFTout) ;

// Calculate the inverse fft, and load the result into r4Cascade
    fftw_execute(outPlan) ;

    i4CascadeOffset = i4Level*i4CascadeArraySize ;
    for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++) {
      i4Row = cDom.row(i4Loop) ;
      i4Col = cDom.col(i4Loop) ;
      i4FFTOffset = fDom.point(i4Row, i4Col) ;
      r4Cascade[i4CascadeOffset+i4Loop] = r8FFTin[i4FFTOffset]*r4FFTFactor ;
		}

    r4CascadeLevelMeanOut[i4Level] = 0 ;
    for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++) {
      r4CascadeLevelMeanOut[i4Level] += i4DataMask[i4Loop]*r4Cascade[i4CascadeOffset+i4Loop] ;
		}
    r4CascadeLevelMeanOut[i4Level] /= (float)i4NumberValidData ;
//
//-----------------------------------------------------------------------
//
// 4. Subtract the mean and calculate the standard deviation of this level
//
    r4CascadeLevelStdOut[i4Level] = 0 ;
    for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++) {
      r4Cascade[i4CascadeOffset+i4Loop] -= r4CascadeLevelMeanOut[i4Level] ;
      r4CascadeLevelStdOut[i4Level] += i4DataMask[i4Loop]*r4Cascade[i4CascadeOffset+i4Loop]*r4Cascade[i4CascadeOffset+i4Loop] ;
		}
    r4CascadeLevelStdOut[i4Level] =  sqrt(r4CascadeLevelStdOut[i4Level]/(float)i4NumberValidData) ;

    for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++) {
      r4Cascade[i4CascadeOffset+i4Loop] /= r4CascadeLevelStdOut[i4Level] ;
		}

// Reduce the scale of rain objects we are looking at, and proceed to next level
    startMask  *= r4ScaleRatio ;
    centreMask *= r4ScaleRatio ;
    endMask    *= r4ScaleRatio ;
  }

// AWS 15 May 2010 Add the residual to the lowest cascade level
  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ) {
    if ( i4DataMask[i4Loop] == 1 ) {
      r4CascadeSum = r4FieldMean ;
      for ( i4Level = 0; i4Level < i4NumberLevels; i4Level++ ) {
        i4CascadeOffset = i4Level * i4CascadeArraySize ;
        r4CascadeSum += r4CascadeLevelStdOut[i4Level]*r4Cascade[i4CascadeOffset+i4Loop] ;
      }
      r4Residual =  r4InputMap[i4Loop] - r4CascadeSum ;
      i4CascadeOffset = (i4NumberLevels-1)*i4CascadeArraySize ;
      r4Cascade[i4CascadeOffset+i4Loop] += r4Residual ;
    }
  }
}

void BCascade::update_track(
	Optical_Flow* sOpticalFlowPassed   ///< IN The optical flow object for which to calculate advection
) {
/// \fn   void BCascade::update_track(Optical_Flow* sOpticalFlowPassed)
/// \brief Upate the optical flow tracking class
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To call Optical_Flow with the new radar data, to generate a new set
/// of advection velocities
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Neill Bowler    06/02/03     Initial Version
/// 1.1          Alan Seed       10/10/04     Changed to 1D vectors
/// 2.0          Alan Seed       12/01/07     Changed the tracking so that the input rectangular map is tracked rather than the square cascade array
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.2          Alan Seed       12/01/2007   A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
/// 1.Create a tracking class for rectangular input map array
/// 2.Create two rectangular arrays and fill with the input map data
/// 3.Track these arrays
/// 4. Fill the cascade array with the 1d displacement in the areas with no data
/// and with the 2d advections in the area with valid data
/// 5. Set the cascade optical flow velocities
///
///
/// NOTES:
/// There is no temporal averaging with the tracking so no need to
/// maintain the data from one call to the next
///
/// The tracking object is the size the input map not the cascade array
///
/// REFERENCES:
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///  float *r4Map1 = new float [i4InMapArraySize] ;        array of the lag one input map
///  float *r4Map0 = new float [i4InMapArraySize] ;        array of the lag zero input map
///  float *r4MapAdvE = new float [i4InMapArraySize] ;     array of input map east advection
///  float *r4MapAdvS = new float [i4InMapArraySize] ;     array of input map south advection
///  float *r4CasAdvE = new float [ i4CascadeArraySize] ;  array of advection for cascade east
///  float *r4CasAdvS = new float [ i4CascadeArraySize] ;  array of advection for cascade south
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// float r4NoData;           flag for non-valid data
/// float** r4Maps;           array of previous maps
/// time_t sMapTimes[8] ;     list of times for each map
///
/// STANDARD LIBRARIES USED:
///
/// None
///
///***99999999999999999999999999999999999999999999999999999999999999999999
/// </pre>
//
//-------------------------------------------------------------------------
//

// Local variables
//
//-----------------------------------------------------------------------
//
// 1.Create a tracking class for rectangular array
//
  size_t i4Loop ;
  Optical_Flow *sMapAdvection ;
  int i4InitOption = i4ImageNumber-1  ;  // Initialise the first time we have two images to track
  char cMapOpticalFileName[512] ;
  sprintf(cMapOpticalFileName, "%s_temp",  cOpticalFileName ) ;
  sMapAdvection = new Optical_Flow( i4NumberRows, i4NumberCols, r4PixelSize, i4InitOption, cMapOpticalFileName);

  short int i2DeltaTime = (short int) ((sMapTimes[0] - sMapTimes[1]) / 60) ;
  float *r4MapAdvE = new float [i4InMapArraySize] ; // array of input map east advection
  float *r4MapAdvS = new float [i4InMapArraySize] ; // array of input map south advection
  float *r4CasAdvE = new float [ i4CascadeArraySize] ; // array of advection for cascade east
  float *r4CasAdvS = new float [ i4CascadeArraySize] ; // array of advection for cascade south
  float r4EastAdv  ; // 1d advection east
  float r4SouthAdv ; // 1d advection south
//
//-----------------------------------------------------------------------
//
// 3.Track these arrays
//
  sMapAdvection->Find_Velocities( r4RainMap1, r4RainMap, r4NoData, i2DeltaTime, i4InitOption ) ;
//
//-----------------------------------------------------------------------
//
// 4. Fill the cascade array with the 1d displacement in the areas with no data
// and with the 2d advections in the area with valid data
//
  sMapAdvection->Get_Velocities(r4MapAdvE, r4MapAdvS) ;

  r4EastAdv = 0 ;
  r4SouthAdv = 0 ;
  for ( i4Loop = 0; (int)i4Loop < i4NumberRows*i4NumberCols; i4Loop++) r4EastAdv += r4MapAdvE[i4Loop] ;
  r4EastAdv /= (i4NumberRows*i4NumberCols) ;
  for ( i4Loop = 0; (int)i4Loop < i4NumberRows*i4NumberCols; i4Loop++) r4SouthAdv += r4MapAdvS[i4Loop] ;
  r4SouthAdv /= (i4NumberRows*i4NumberCols) ;

  to_cascade(r4MapAdvE, r4CasAdvE) ;
  to_cascade(r4MapAdvS, r4CasAdvS) ;
  for ( i4Loop = 0 ; i4Loop < i4CascadeArraySize; i4Loop++ ) {
    if ( i4DataMask[i4Loop] == 0 ) {
      r4CasAdvE[i4Loop] = r4EastAdv ;
      r4CasAdvS[i4Loop] = r4SouthAdv ;
    }
  }
//
//-----------------------------------------------------------------------
//
// 5. Set the cascade optical flow velocities
//
  sOpticalFlowPassed->Set_Velocities(r4CasAdvE, r4CasAdvS) ;

// delete local memory
  delete [] r4MapAdvE ;
  delete [] r4MapAdvS ;
  delete [] r4CasAdvE ;
  delete [] r4CasAdvS ;
  delete sMapAdvection ;

}

void BCascade::update_autocor(
	Optical_Flow* sOpticalFlowPassed   ///< IN: The optical flow object to use for the advection
) {
/// \fn  void BCascade::update_autocor(Optical_Flow* sOpticalFlowPassed )
/// \brief Calculate the lag 1 and 2 autocorrelations for each level in the cascade
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To find the correlation coefficients between the cascade levels at lag 1 and lag 2
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Alan Seed       ??/??/??     Initial version
/// 1.10	        Neill Bowler    06/02/03     Large changes.
/// 1.20         Alan Seed       10/10/04     Changed 2d arrays to vectors
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// ARRAY NAME: r4TempWeight
/// ARRAY TYPE: float*
/// DIMENSIONS: i4NumberRows*i4NumberCols
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// int i4InMapArraySize ;          Size of the input map array
/// int i4NumberRows;               Size of the input rain map
/// int i4NumberCols;               Size of the input rain map
/// int i4NumberLevels;             Image size and number of cascade levels
/// float* r4CascadeStack[3];       Array of pointers to the cascade arrays
/// int i4NumberCascades;           Maximum number of cascades in memory
/// float* r4TempArray ;            Temporary array of image data
/// float r4NoData;                 Flag for non-valid data
/// float* r4Correlation;           Auto correlations for the cascade
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>
// Local variables
//
  int i4Lag ;                  // Loop variable, cascade lag
  int i4Level;                 // Loop variable, level in cascade
  int i4Loop2 ;                // Loop variable
  size_t i4Loop ;              // Loop variable
  size_t i4CascadeOffset ;     // Offset into the cascade array
	size_t i4Total ;             // Number of pixels with data
	int i4LoopCascade ;          // Loop over the levels and lags

// Variables for calculating correlation
	float r4x;                   // Used for calculating mean value of current analysis
	float r4xx;                  // Used for calculating variance of current analysis
	float r4y;                   // Used for calculating mean value of old analysis
	float r4yy;                  // Used for calculating variance of old analysis
	float r4xy;                  // Used for calculating cross-product of old and current analysis
	double r8Denominator;         // Value of denominator of correlation
	float r4TempCorrelation;     // Value of correlation

// Dynamically allocated arrays
	float* r4TempWeight = new float [i4CascadeArraySize];  // Temporary array for holding advected data

	for ( i4LoopCascade=0; i4LoopCascade<i4NumberLags*i4NumberLevels; i4LoopCascade++ ) {
		i4Lag   = 1 + i4LoopCascade/i4NumberLevels ;
		i4Level = i4LoopCascade - (i4Lag-1)*i4NumberLevels ;
		i4CascadeOffset = i4Level*i4CascadeArraySize ;

// Load old set of weights into a temporary array
		for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ) {
			r4TempWeight[i4Loop] = r4CascadeStack[i4Lag][i4CascadeOffset+i4Loop] ;
		}

// Advect this image to the present time
		if ( i4Lag == 1 ) {
			advect_cascade(1, (int)r4NoData,  r4TempWeight, r4TempArray) ;
		}
		else if ( i4Lag == 2 ) {
			advect_cascade(1, (int)r4NoData,  r4TempWeight, r4TempArray) ;

			for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ) r4TempWeight[i4Loop] = r4TempArray[i4Loop];
			advect_cascade(1, (int)r4NoData,  r4TempWeight, r4TempArray) ;
		}
		else {
			advect_cascade(i4Lag, (int) r4NoData, r4TempWeight, r4TempArray) ;
		}

//  Calculate the correlation
		r4x  = 0 ;            // Mean value of the first map
		r4xx = 0 ;            // Variance of the first map
		r4y  = 0 ;            // Mean value of the second map
		r4yy = 0 ;            // Variance of the second map
		r4xy = 0 ;            // Cross-product of first and second maps
		i4Total = 0;            // The number of pixels with data
		r4TempCorrelation = 0 ;

		for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ) {
			if ( r4TempArray[i4Loop] > r4NoData+1 && i4DataMask[i4Loop] == 1 ) {
				r4x += r4TempArray[i4Loop] ;
				r4y += r4CascadeStack[0][i4CascadeOffset+i4Loop] ;
				i4Total++ ;
			}
		}

		if ( i4Total > 1 ) {
			r4x /= (float)(i4Total) ;
			r4y /= (float)(i4Total) ;

			for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ) {
				if ( r4TempArray[i4Loop] > r4NoData+1 && i4DataMask[i4Loop] == 1 ) {
					r4xx += (r4TempArray[i4Loop]-r4x)*(r4TempArray[i4Loop]-r4x) ;
					r4yy += (r4CascadeStack[0][i4CascadeOffset+i4Loop]-r4y)*(r4CascadeStack[0][i4CascadeOffset+i4Loop]-r4y) ;
					r4xy += (r4TempArray[i4Loop]-r4x)*(r4CascadeStack[0][i4CascadeOffset+i4Loop]-r4y) ;
				}
			}
			r4xx /= (float)(i4Total) ;
			r4yy /= (float)(i4Total) ;
			r4xy /= (float)(i4Total) ;

			r8Denominator = r4xx*r4yy ;
			if ( r8Denominator > 0.0 ) {
				r4Correlation[i4LoopCascade] = r4xy / sqrt(r8Denominator) ;
			}
			else
				r4Correlation[i4LoopCascade] = 0 ;
		} else {

// All of the data is marked by the "no-data" flag, so set the correlation to an error code
			r4Correlation[i4LoopCascade] = r4NoData;
		}
	}

// Delete the dynamic array
	delete [] r4TempWeight;

#ifdef VERBOSE_CASCADE
	if ( fLogFile != NULL ) {
		fprintf(fLogFile, "\nInformation: BCascade:update_autocor Correlations\n" ) ;
    if (i4NumberLags == 2) {
      for ( i4Loop2 = 0; i4Loop2 < i4NumberLevels; i4Loop2++ )
        fprintf(fLogFile, "Level = %d, lag1 = %f, lag2 = %f\n", i4Loop2, r4Correlation[i4Loop2],r4Correlation[i4Loop2+i4NumberLevels] ) ;
    }
    else {
      for ( i4Loop2 = 0; i4Loop2 < i4NumberLevels; i4Loop2++ )
        fprintf(fLogFile, "Level = %d, lag1 = %f\n", i4Loop2, r4Correlation[i4Loop2] ) ;
    }
  }
#endif
}

void BCascade::update_phi( float* r4LocalCorr) {
/// \fn void BCascade::update_phi(float* r4LocalCorr)
/// \brief Used by Merged Cascade
	update_phi(r4LocalCorr, r4Phi) ;
}

void BCascade::update_phi(
	float* r4LocalCorr,  ///< IN: The value of the auto-correlation
	float* r4LocalPhi    ///< OUT: The value of the AR(2) parameters
) {
/// \fn void BCascade::update_phi(float* r4LocalCorr,float* r4LocalPhi)
/// \brief Calculate the AR(2) parameters using the Yule Walker equations
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: cascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// Given the new values of the correlation coefficient, calculate the
/// AR(2) parameters
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Alan Seed       ??/??/??        Initial version
/// 1.10	        Neill Bowler    06/02/03        Later version
/// 1.2          Alan Seed       10/10/04        Convert 2d arrays into vectors
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
/// None
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// None
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// int i4NumberCascades;       // maximum number of cascades in memory
/// int i4NumberLevels;      // image size and number of cascade levels
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>
// Local variables
//

	int i4Cor1;       // Location of lag 1 correlation
	int i4Cor2 ;      // Location of lag 2 correlation
	int i4Phi0 ;      // Location of phi0
	int i4Phi1 ;      // Location of phi1
	int i4Phi2 ;      // Location of phi2
	int i4Level ;     // Level in the cascade

	float r4Temp1;    // Temporary accumulator when calculating r4Phi[0] for AR(2)
	float r4Temp2;    // Temporary accumulator when calculating r4Phi[0] for AR(2)

	for (i4Level = 0; i4Level < (i4NumberLags+1)*i4NumberLevels; i4Level++ ) r4LocalPhi[i4Level] = 0 ;

// Perform consistency checks on the autocorrelations
		for ( i4Level =0; i4Level<i4NumberLevels; i4Level++ ) {
			i4Cor1 = i4Level ;
			i4Cor2 = i4Level+i4NumberLevels ;
			if ( i4NumberLags == 1 ) {
				if ( r4LocalCorr[i4Cor1] < 0.5 ) {
					r4LocalCorr[i4Cor1] = 0.5*(r4LocalCorr[i4Cor1]+0.5) ;
				}
			}
			else
			{

// Adjust for the error in tracking at small scales
			if ( r4LocalCorr[i4Cor1] < 0.5 ) {
				r4LocalCorr[i4Cor1] = 0.5*(r4LocalCorr[i4Cor1]+0.5) ;
				r4LocalCorr[i4Cor2] = pow(r4LocalCorr[i4Cor1],(float)1.7) ;
			}

// Make sure that the correlation lies in the proper solution space
			r4LocalCorr[i4Cor2] = std::max( (double)r4LocalCorr[i4Cor2],
				2.0 * r4LocalCorr[i4Cor1] * r4LocalCorr[i4Cor2] - 1 );
			float r4TempA = pow( (float)r4LocalCorr[i4Cor1], (float)2.0 ) ;
			r4LocalCorr[i4Cor2] = std::max( (double)r4LocalCorr[i4Cor2],
				(3.0 * r4TempA - 2.0 + 2.0 * pow((float)(1.0 - r4TempA ),(float)1.5)) /
				r4TempA ) ;
		}
	}

// Calculate the values of phi, using either an AR(1) or AR(2) model
	if ( i4NumberLags == 1 ) { // AR(1) model
		for ( i4Level=0; i4Level<i4NumberLevels; i4Level++ ) {
			r4LocalPhi[i4NumberLevels + i4Level] = r4LocalCorr[i4Level] ;
			r4LocalPhi[i4Level] = sqrt(1-r4LocalCorr[i4Level]*r4LocalCorr[i4Level]);
		}
	} else if ( i4NumberLags == 2 ) { // AR(2) model
		for ( i4Level=0; i4Level<i4NumberLevels; i4Level++ ) {
			i4Phi0 = i4Level ;
			i4Phi1 = i4Level + i4NumberLevels ;
			i4Phi2 = i4Level + 2*i4NumberLevels ;
			i4Cor1 = i4Level ;
			i4Cor2 = i4Level + i4NumberLevels ;

			r4LocalPhi[i4Phi1] = r4LocalCorr[i4Cor1]*(1-r4LocalCorr[i4Cor2])/
				(1-r4LocalCorr[i4Cor1]*r4LocalCorr[i4Cor1]);
			r4LocalPhi[i4Phi2] = (r4LocalCorr[i4Cor2]-
				r4LocalCorr[i4Cor1]*r4LocalCorr[i4Cor1])/
				(1-r4LocalCorr[i4Cor1]*r4LocalCorr[i4Cor1]);

			if (  r4LocalPhi[i4Phi2] +  r4LocalPhi[i4Phi1] >= 1.0 ||
						r4LocalPhi[i4Phi2] -  r4LocalPhi[i4Phi1] >= 1.0 ||
						fabs(r4LocalPhi[i4Phi2]) >= 1.0 ) {
#ifdef VERBOSE_CASCADE
				if ( fLogFile != NULL ) {
					fprintf(fLogFile,"Information BCascade::update_phi(float*, float*) Invalid Phi values found Phi1 = %f, Phi2 = %f\n",
					 r4LocalPhi[i4Phi1], r4LocalPhi[i4Phi2]) ;
				}
#endif
			}

			r4Temp1 = (1+r4LocalPhi[i4Phi2])/(1-r4LocalPhi[i4Phi2]) ;
			r4Temp2 = ((1-r4LocalPhi[i4Phi2])*(1-r4LocalPhi[i4Phi2])-
				r4LocalPhi[i4Phi1]*r4LocalPhi[i4Phi1]) ;
			if ( r4Temp1*r4Temp2 > 0 )
				r4LocalPhi[i4Phi0] = sqrt(r4Temp1*r4Temp2) ;
			else
				r4LocalPhi[i4Phi0] = 0 ;

#ifdef VERBOSE_CASCADE
			if ( fLogFile != NULL ) {
				if ( i4NumberLags == 2 )
					fprintf(fLogFile,"Information BCascade::update_phi(float*, float*) Level = %d, Phi0 = %5.2f, Phi1 = %5.2f, Phi2 = %5.2f\n",
					 i4Level,r4LocalPhi[i4Phi0],r4LocalPhi[i4Phi1], r4LocalPhi[i4Phi2]) ;
				else
					fprintf(fLogFile,"Information BCascade::update_phi(float*, float*) Level = %d, Phi0 = %5.2f, Phi1 = %5.2f\n",
					 i4Level,r4LocalPhi[i4Phi0],r4LocalPhi[i4Phi1]) ;
			}
#endif

		}
	}
}

void BCascade::get_phi(
	float *r4LocalPhi ///< AR parameters, float[i4NumberLevels*(i4NumberLags+1)]
){
/// \fn  void BCascade::get_phi(float *r4LocalPhi)
/// \brief Return the AR parameters
  int i4Loop ;
  for ( i4Loop = 0; i4Loop < i4NumberLevels*(i4NumberLags+1); i4Loop++ ) r4LocalPhi[i4Loop] = r4Phi[i4Loop] ;
}

void BCascade::get_correlation(
  float *r4LocalCor   ///< IN: Lag 1 & 2 correlations, float[i4NumberLevels*i4NumberLags]
){
/// \fn  void BCascade::get_correlation(float *r4LocalCor)
/// \brief Return the lag 1 & 2 correlations
  int i4Loop ;
  for ( i4Loop = 0; i4Loop < i4NumberLevels*i4NumberLags; i4Loop++ ) r4LocalCor[i4Loop] = r4Correlation[i4Loop] ;
}


void BCascade::update_weights ( ) {
/// \fn void BCascade::update_weights ( )
/// \brief Update the cascade weights using the AR model
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To update the cacade weights using the AR2 model and the current advection
/// Used by the merged_cascade class
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00          Alan Seed       15/11/04     Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
/// 1. Rotate the cascade stack for the next iteration of the AR(2) model
/// 2. Update each level in the cascade
/// 3. Delete all dymanic arrays
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
/// None
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// ARRAY NAME: r4TempMap 1, 1a, 2, 2a
/// ARRAY TYPE: float*
/// DIMENSIONS: i4CascadeArraySize
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// int i4NumberRows;             Size of the input rain map
/// int i4NumberCols;             Size of the input rain map
/// int i4NumberLevels;           Image size and number of cascade levels
/// int i4CascadeArraySize;       Size of cascade level
/// float*** r4CascadeStack[3];   Array of pointers to the cascade arrays
/// float* r4Phi;                 AR(2) model parameters for updates
/// float** r4CascadeLevelMean[16];    Mean for each level
/// float** r4CascadeLevelStd[16];     Standard deviation for each level
/// float r4NoData;               Flag for non-valid data
///
/// STANDARD LIBRARIES USED:
///
///
///***99999999999999999999999999999999999999999999999999999999999999999999
/// </pre>
//
// Local variables
//
  int i4Level;                // Loop counter, level in cascade
  size_t i4Loop;                 // Loop counter
  size_t i4CascadeLevelOffset ;  // Start of cascade level
  int i4NoData  ;                // Invalid data flag
  float r4Phi1  ;                // Value of phi1
  float r4Phi2  ;                // Value of phi2
  float r4Tlag1 ;
  float r4Tlag2 ;
  float r4Tlag0 ;

#ifdef DEBUG
  int i4BadPoint ;
  int i4StartRow ;
  int i4EndRow ;
  int i4StartCol ;
  int i4EndCol ;
#endif


// Dynamically allocate arrays
  float* r4TempMap1    = new float [i4CascadeArraySize] ;
  float* r4TempMap2    = new float [i4CascadeArraySize] ;
  float* r4TempMap1a   = new float [i4CascadeArraySize] ;
  float* r4TempMap2a   = new float [i4CascadeArraySize] ;

//
//-----------------------------------------------------------------------
//
// 1. Rotate the cascade stack for the next iteration of the AR(2) model
//
  for ( i4Loop=0; i4Loop < i4NumberLevels*i4CascadeArraySize;i4Loop++) r4CascadeStack[2][i4Loop] = r4CascadeStack[1][i4Loop] ;
  for ( i4Loop=0; i4Loop < i4NumberLevels*i4CascadeArraySize;i4Loop++) r4CascadeStack[1][i4Loop] = r4CascadeStack[0][i4Loop] ;
  for ( i4Loop=0; i4Loop < i4NumberLevels*i4CascadeArraySize;i4Loop++) r4CascadeStack[0][i4Loop] = 0.0 ;
  i4NoData = (int)r4NoData ;
//
//-----------------------------------------------------------------------
//
// 2. Update each level in the cascade
//
  for ( i4Level = 0; i4Level < i4NumberLevels; i4Level++ ) {

// First load up the cascade levels and advect
    i4CascadeLevelOffset = i4Level * i4CascadeArraySize ;
    for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ) {
      r4TempMap1[i4Loop] = r4CascadeStack[1][i4CascadeLevelOffset+i4Loop] ;
      r4TempMap2[i4Loop] = r4CascadeStack[2][i4CascadeLevelOffset+i4Loop] ;
      r4TempMap2a[i4Loop] = 0 ;
      r4TempMap1a[i4Loop] = 0 ;
    }
    advect_cascade(1, i4NoData,  r4TempMap1, r4TempMap1a);
    advect_cascade(2, i4NoData,  r4TempMap2, r4TempMap2a);

// Now calculate update the cascade level using the AR(2) paramaters
    r4Phi1 = r4Phi[i4NumberLevels + i4Level ];
    r4Phi2 = r4Phi[2*i4NumberLevels + i4Level] ;

    for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++) {
      r4Tlag1 = r4TempMap1a[i4Loop] ;
      r4Tlag2 = r4TempMap2a[i4Loop] ;

      if ( r4Tlag1 > r4NoData+1 && r4Tlag2 > r4NoData+1 ){
        r4Tlag0 = r4Phi1*r4Tlag1 + r4Phi2*r4Tlag2 ;
        r4CascadeStack[0][i4CascadeLevelOffset+i4Loop] = r4Tlag0 ;
      }
      else
        r4CascadeStack[0][i4CascadeLevelOffset+i4Loop] = r4NoData ;

#ifdef DEBUG
      if ( fabs (r4Tlag0) > 10.0  ) {
        int i4Row = cDom.row(i4Loop) ;
        int i4Col = cDom.col(i4Loop) ;
        fprintf(fLogFile,"Information BCascade::update_weights() Weight out of bounds Row = %d, Col = %d, Level = %d, Weight = %f\n",
        i4Row, i4Col, i4Level, r4Tlag0) ;
        if ( i4BadPoint == 0 ) {
          i4BadPoint = 1 ;

          i4StartRow = i4Row-5 ;
          if ( i4StartRow < 0 ) i4StartRow = 0 ;
          i4EndRow = i4StartRow+11 ;
          if ( i4EndRow > i4CascadeSize-1 ) i4EndRow = i4CascadeSize-1 ;

          i4StartCol = i4Col-5 ;
          if ( i4StartCol < 0 ) i4StartCol = 0 ;
          i4EndCol = i4StartCol+11 ;
          if ( i4EndCol > i4CascadeSize-1 ) i4EndCol = i4CascadeSize-1 ;

          fprintf(fLogFile,"Lag1a,") ;
          for ( int i4Col = i4StartCol; i4Col <= i4EndCol; i4Col++ )
            fprintf(fLogFile,"%05d,", i4Col) ;
          fprintf(fLogFile,"\n") ;

          for ( int i4Row = i4StartRow; i4Row <= i4EndRow; i4Row++ ) {
            fprintf(fLogFile,"%05d,", i4Row) ;
            for ( int i4Col = i4StartCol; i4Col <= i4EndCol; i4Col++ )
              fprintf(fLogFile,"%5.2f,",r4TempMap1a[i4Row*i4CascadeSize+i4Col]) ;
            fprintf(fLogFile,"\n") ;
          }

          fprintf(fLogFile,"Lag2a,") ;
          for ( int i4Col = i4StartCol; i4Col <= i4EndCol; i4Col++ ) fprintf(fLogFile,"%05d,", i4Col) ;
          fprintf(fLogFile,"\n") ;

          for ( int i4Row = i4StartRow; i4Row <= i4EndRow; i4Row++ ) {
            fprintf(fLogFile,"%05d,", i4Row) ;
            for ( int i4Col = i4StartCol; i4Col <= i4EndCol; i4Col++ )
              fprintf(fLogFile,"%5.2f,",r4TempMap2a[i4Row*i4CascadeSize+i4Col]) ;
            fprintf(fLogFile,"\n") ;
          }
        }
      }
#endif
    }
  }
//
//-----------------------------------------------------------------------
//
// 3. Delete all dymanic arrays
//
  delete [] r4TempMap1 ;
  r4TempMap1 = NULL ;
  delete [] r4TempMap1a ;
  r4TempMap1a = NULL ;

  delete [] r4TempMap2 ;
  r4TempMap2 = NULL ;
  delete [] r4TempMap2a ;
  r4TempMap2a = NULL ;
}

void BCascade::smooth_forecast_rain(
	int i4NumberForecasts,       ///< IN: Number of forecast to make
	float* r4OutputForecast     ///< OUT: Forecast rainfall, float[i4NumberForecasts*i4NumberCols*i4NumberRows]
) {
/// \fn void BCascade::smooth_forecast_rain(int i4NumberForecasts,float* r4OutputForecast)
/// \brief Generate rainfall forecasts that smooth in time
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To produce a deterministic forecast, using my optical flow and the
/// AR(2) model to smooth the field with increasing lead time.
/// Builds the forecasts iteratively
/// This version assumes that we have an AR(2) model
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00          Alan Seed       15/11/04     Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
/// 1. Dynamically allocate arrays
/// 2. Rotate the cascade stack for the next iteration of the AR(2) model
/// 3. Advect and update each level in the cascade
/// 4. Calculate the forecast mean and variance
/// 5. Convert back into rain intensity
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
/// None
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// ARRAY NAME: r4TempMap 1, 1a, 2, 2a
/// ARRAY TYPE: float*
/// DIMENSIONS: i4CascadeArraySize
///
/// ARRAY NAME: r4TempCascadeStack[3]
/// ARRAY TYPE: float*
/// DIMENSIONS: i4CascadeArraySize*i4NumberLevels
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// int i4NumberRows;             // size of the input rain map
/// int i4NumberCols;             // size of the input rain map
/// int i4NumberLevels;           // image size and number of cascade levels
/// int i4CascadeArraySize;       // size of cascade level
/// float*** r4CascadeStack[3];   // array of pointers to the cascade arrays
/// float* r4Phi;                 // ar(2) model parameters for updates
/// float** r4CascadeLevelMean[16];    // mean for each level
/// float** r4CascadeLevelStd[16];     // standard deviation for each level
/// float r4NoData;               // flag for non-valid data
///
/// STANDARD LIBRARIES USED:
///
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
///-----------------------------------------------------------------------
///
/// </pre>

/// Declare variables and dynamic arrays
//
// Local variables
//
  int i4Level;                   // Loop counter, level in cascade
  int i4Row;                     // Loop counter, column position in output image
  int i4Col;                     // Loop counter, row position in output image
  size_t i4Loop;                 // Loop counter
  size_t i4CascadeLevelOffset ;  // Start of cascade level
  size_t i4Offset ;              // Offset in the output map
  size_t i4CascadeStackSize ;    // Size of the cascade stack (number levels*size*size)
  int i4Forecast;                // Loop counter, number of current forecast
  float *r4TempP ;               // Temp pointer to cascade
  float r4Phi1  ;                // Value of phi1
  float r4Phi2  ;                // Value of phi2

//
//-----------------------------------------------------------------------
//
// 1. Dynamically allocate arrays
  float* r4TempMap1    = new float [i4CascadeArraySize] ;
  float* r4TempMap2    = new float [i4CascadeArraySize] ;
  float* r4TempMap1a   = new float [i4CascadeArraySize] ;
  float* r4TempMap2a   = new float [i4CascadeArraySize] ;

  float* r4TempCascadeStack[3] ;
  i4CascadeStackSize = i4CascadeArraySize*i4NumberLevels ;
  r4TempCascadeStack[0] = new float [i4CascadeStackSize] ;
  r4TempCascadeStack[1] = new float [i4CascadeStackSize] ;
  r4TempCascadeStack[2] = new float [i4CascadeStackSize] ;

  for ( i4Loop = 0; i4Loop < i4CascadeStackSize; i4Loop++ ){
    r4TempCascadeStack[0][i4Loop] = r4CascadeStack[0][i4Loop] ;
    r4TempCascadeStack[1][i4Loop] = r4CascadeStack[1][i4Loop] ;
    r4TempCascadeStack[2][i4Loop] = r4CascadeStack[2][i4Loop] ;
  }

	for ( i4Forecast=0; i4Forecast<i4NumberForecasts; i4Forecast++ ) {
//
//-----------------------------------------------------------------------
//
// 2. Rotate the cascade stack for the next iteration of the AR(2) model
//
    float *r4OutputForecastP = r4OutputForecast +  i4Forecast*i4InMapArraySize ;
    r4TempP = r4TempCascadeStack[2] ;
    r4TempCascadeStack[2] = r4TempCascadeStack[1] ;
    r4TempCascadeStack[1] = r4TempCascadeStack[0] ;
    r4TempCascadeStack[0] = r4TempP ;
//
//-----------------------------------------------------------------------
//
// 3. Advect and update each level in the cascade
//
    for ( i4Level = 0; i4Level < i4NumberLevels; i4Level++ ) {

// First load up the cascade levels and advect
      i4CascadeLevelOffset = i4Level * i4CascadeArraySize ;
      for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ) {
        r4TempMap1[i4Loop] = r4TempCascadeStack[1][i4CascadeLevelOffset+i4Loop] ;
        r4TempMap2[i4Loop] = r4TempCascadeStack[2][i4CascadeLevelOffset+i4Loop] ;
      }
	    advect_cascade(1, (int) r4NoData,  r4TempMap1, r4TempMap1a);
	    advect_cascade(2, (int) r4NoData,  r4TempMap2, r4TempMap2a);

// Now calculate update the cascade level using the AR(2) paramaters
      r4Phi1 = r4Phi[i4NumberLevels + i4Level ];
      r4Phi2 = r4Phi[2*i4NumberLevels + i4Level] ;

      for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++) {
        if ( r4TempMap1a[i4Loop] > r4NoData+1 && r4TempMap2a[i4Loop] > r4NoData+1 ) {
          r4TempCascadeStack[0][i4CascadeLevelOffset+i4Loop] = r4Phi1*r4TempMap1a[i4Loop] + r4Phi2*r4TempMap2a[i4Loop] ;
        }
        else {
          r4TempCascadeStack[0][i4CascadeLevelOffset+i4Loop] = r4NoData ;
        }

// And accumulate in the output array
        i4Row = i4Loop / i4CascadeSize ;
        i4Col = i4Loop - i4Row*i4CascadeSize ;
        if ( i4Row < i4NumberRows && i4Col < i4NumberCols ) {
          i4Offset = i4Row*i4NumberCols + i4Col ;
          if ( r4TempCascadeStack[0][i4CascadeLevelOffset+i4Loop] > r4NoData+1 ) {
            if (i4Level == 0) r4OutputForecastP[i4Offset] = r4FieldMean ;
            r4OutputForecastP[i4Offset] += (r4CascadeLevelStd[i4Level]*r4TempCascadeStack[0][i4CascadeLevelOffset+i4Loop])  ;
          }
          else
            r4OutputForecastP[i4Offset] = r4NoData ;
        }
      }
    }
//
//-----------------------------------------------------------------------
//
// 4. Calculate the forecast mean and variance
//
    float r4FxMean = 0 ; // mean of the forecast
    float r4FxVariance = 0 ; // variance of the forecast
    int i4NumberValidPixels = 0 ; // Number of valid pixels in forecast
    float r4TempBias ; // correction factor

    for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ) {
      if ( r4OutputForecastP[i4Loop] > r4NoData+1 ) {

        // Assume that the field is dbr so the minimum is -10 (0.1 mm/h)
        if ( r4OutputForecastP[i4Loop] < -10.0 ) r4OutputForecastP[i4Loop] = -10.0 ;
        r4FxMean += r4OutputForecastP[i4Loop]  ;
        i4NumberValidPixels++ ;
      }
    }
    if ( i4NumberValidPixels > 0 ) r4FxMean /= i4NumberValidPixels ;

    for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ) {
      if ( r4OutputForecastP[i4Loop] > r4NoData+1 ) {

        // Assume that the field is dbr so the minimum is -10 (0.1 mm/h)
        if ( r4OutputForecastP[i4Loop] < -10.0 ) r4OutputForecastP[i4Loop] = -10.0 ;
        r4FxVariance += (r4OutputForecastP[i4Loop]-r4FxMean)*(r4OutputForecastP[i4Loop]-r4FxMean) ;   ;
      }
    }
    if ( i4NumberValidPixels > 0 ) r4FxVariance /= i4NumberValidPixels ;

// Calculate the bias due to the loss of variance and add to the forecast
    r4TempBias = (r4FieldMean -  r4FxMean) + 0.05*(r4FieldStd*r4FieldStd - r4FxVariance) ;
    for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++ ) {
      if ( r4OutputForecastP[i4Loop] > r4NoData+1 ) {
        r4OutputForecastP[i4Loop] += r4TempBias ;
      }
    }

//
//-----------------------------------------------------------------------
//
// 5. Convert back into rain intensity
//
    convert_mmh_dbr(false, r4NoData, i4NumberRows, i4NumberCols, r4OutputForecastP ) ;
  }

// Delete all dymanic arrays
  delete [] r4TempCascadeStack[0];
  delete [] r4TempCascadeStack[1];
  delete [] r4TempCascadeStack[2];

  delete [] r4TempMap1 ;
  r4TempMap1 = NULL ;
  delete [] r4TempMap1a ;
  r4TempMap1a = NULL ;

  delete [] r4TempMap2 ;
  r4TempMap2 = NULL ;
  delete [] r4TempMap2a ;
  r4TempMap2a = NULL ;
}

void BCascade::bandpass_filter_fftw(
  fftw_complex* c4FFTPassedIn,      ///< IN: The Fourier transform to be filtered
  int    i4FFTSize,           ///< IN: The size of (the 2D version of) the Fourier tranform
  float  r4StartWaveLength,   ///< IN: The largest wavelength to be passed by the filter
  float  r4CentreWaveLength,  ///< IN: The middle wavelength to be passed by the filter
  float  r4EndWaveLength,     ///< IN: The smallest wavelength to be passed by the filter
  fftw_complex* c4FFTPassedOut      ///< OUT: The filtered Fourier transform
){
/// \fn  void BCascade::bandpass_filter_fftw(fftw_complex* c4FFTPassedIn,int i4FFTSize,float r4StartWaveLength,float r4CentreWaveLength,float r4EndWaveLength,fftw_complex* c4FFTPassedOut)
/// \brief Bandpass filter of an FFT array
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To filter a Fourier transform that has been generated by the FFTW library.
/// This filter multiplies the Fourier tranform array by a smoothly
/// varying factor to minimise the Gibbs effect
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.o          Alan Seed       14/06/05        Version 1
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
/// 1. Calculate the wave numbers for the start, centre, end of the filter
/// 2. Calculate the filter for each point in the FFT
/// 3. Apply the filter to the FFT
///
///
/// NOTES:
///
/// method to apply a Hamming Window filter centred on r4CentreWaveLength
/// assumes that the input field has zero mean
/// r4StartWaveLength : large scale wave length in pixels
/// r4EndWaveLength   : small scale wave length in pixels
/// i4FFTSize    : the number of col and rows in the FFT array
///
/// REFERENCES:
/// FFTW documentation
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
/// None
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///   float *r4Factor = new float [(i4Nyquest+1)*i4FFTStride];
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// i4FFTArraySize                                /// The physical size of the array
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
/// </pre>

// Local variables
  int i4StartWaveNumber;                            // The smallest frequency to allow through
  int i4CentreWaveNumber;                           // The frequency to allow through unfiltered
  int i4EndWaveNumber;                              // The largest frequency to allow through
  int i4CentreStartDistance  ;                      // Difference between centre wave number and start wave number
  int i4CentreEndDistance;                          // Difference between centre wave number and end wave number
  int i4CentreDistance;                             // Difference between centre wave number and either the end or start wave number
  float r4CurrentDistance ;                         // Difference between the current wave number and current centre wave number
  float r4CurrentWaveNumber;                        // Current wave number
//  float r4SmoothingFactor = 4.75 ;                  // Factor used in the weighting calculation
  float r4SmoothingFactor = 2.0 ;                  // Factor used in the weighting calculation
  int i4Offset;                                     // Loop variable, position in the 1D array
  int i4Row ;
  int i4Col;                                        // Loop variable, sample in the 2D array
  size_t i4Loop ;                                      // Loop variable
  int i4Nyquest = i4FFTSize/2 ;                     // Nyquest frequency
  int i4FFTStride = 1 + i4FFTSize/2 ;               // The stride in the complex array
  int i4LoopEnd;                                    // The location of the end of the loop given the symmetry

// Dynamically allocate local arrays
  float *r4Factor = new float [(i4Nyquest+1)*i4FFTStride]; // The filter applied to the FFT

// Initialize arrays
	for ( i4Loop=0; i4Loop < i4FFTArraySize; i4Loop++ ){
            c4FFTPassedOut[i4Loop][0] = 0 ;
            c4FFTPassedOut[i4Loop][1] = 0 ;
  }
  for ( i4Offset=0; i4Offset < (i4Nyquest+1)*i4FFTStride; i4Offset++ )
          r4Factor[i4Offset] = 0.0;

//
//-----------------------------------------------------------------------
//
// 1. Calculate the wave numbers for the start, centre, end of the filter
//
// Since we are working on an array, which is a discrete grid, we convert the passed wave
// numbers to integer values and perform some checks
// Calculate the start and end wave numbers for the notch
//
	i4StartWaveNumber = (int) (i4FFTSize/r4StartWaveLength); // get the nearest wave number
	if ( i4StartWaveNumber < 1 ) i4StartWaveNumber = 1 ;
	if ( i4StartWaveNumber >= i4FFTSize/2 ) return ;

	i4EndWaveNumber = (int) (i4FFTSize/r4EndWaveLength);
	if ( i4EndWaveNumber > i4FFTSize/2 ) i4EndWaveNumber = i4FFTSize/2 ;
	if ( i4EndWaveNumber < 1 ) return ;

	i4CentreWaveNumber = (int) (i4FFTSize/r4CentreWaveLength);
  if ( i4CentreWaveNumber < 1 ) i4CentreWaveNumber = 1 ;
	if ( i4CentreWaveNumber > i4FFTSize/2 ) i4CentreWaveNumber = i4FFTSize/2 ;

	i4CentreStartDistance = i4CentreWaveNumber - i4StartWaveNumber ;
  if ( i4CentreStartDistance == 0 ) i4CentreStartDistance = 1 ;

	i4CentreEndDistance   = i4EndWaveNumber    - i4CentreWaveNumber ;
  if ( i4CentreEndDistance == 0 ) i4CentreEndDistance = 1 ;
//
//-----------------------------------------------------------------------
//
// 2. Calculate the filter for each point in the FFT
//
// symmetrical around row i4Nyquest, and below i4Nyquest only rows < i4EndWaveNumber
// will have an element with wave number < i4EndWaveNumber (sqrt(irow^2 + anything))
// is always greater than row). So we can stop looping fairly quickly in most cases
// Also, the highest column number will never be more than i4EndWaveNumber+1 for the
// same reason
  i4LoopEnd = min(i4EndWaveNumber+1,i4FFTStride); // precalc end of loop to allow vectorisation

  for (i4Row=0; i4Row<=min(i4EndWaveNumber,i4Nyquest); i4Row++) {
    for (i4Col=0; i4Col<i4LoopEnd; i4Col++) {
      r4CurrentWaveNumber = sqrt((float)(i4Row*i4Row + i4Col*i4Col)) ;
      if ( r4CurrentWaveNumber >= i4StartWaveNumber && r4CurrentWaveNumber < i4EndWaveNumber ) {
        if ( r4CurrentWaveNumber < i4CentreWaveNumber ) {
          i4CentreDistance  = i4CentreStartDistance;
          r4CurrentDistance = r4CurrentWaveNumber - i4StartWaveNumber;
        } else {
          i4CentreDistance  = i4CentreEndDistance;
          r4CurrentDistance = r4CurrentWaveNumber - i4CentreWaveNumber;
        }
        r4Factor[i4Row*i4FFTStride+i4Col] =
          exp(-1*r4SmoothingFactor*(r4CurrentDistance*r4CurrentDistance)/(i4CentreDistance*i4CentreDistance));

// various special cases:
        if ( r4CurrentWaveNumber < i4CentreWaveNumber ) {
          r4Factor[i4Row*i4FFTStride+i4Col] = 1.0 - r4Factor[i4Row*i4FFTStride+i4Col];
          if ( r4CurrentWaveNumber == i4StartWaveNumber)  r4Factor[i4Row*i4FFTStride+i4Col] = 0.0 ;
        }
        if ( i4CentreWaveNumber == 1 && r4CurrentWaveNumber == 1 )
          r4Factor[i4Row*i4FFTStride+i4Col] = 1.0 ;
      }
    }
  }

// more special cases:
  r4Factor[0] = 1.0;
//
//-----------------------------------------------------------------------
//
// 3. Apply the filter to the FFT
//
  for ( i4Offset=0; i4Offset < (i4Nyquest+1)*i4FFTStride; i4Offset++ ) {

// first half:
    c4FFTPassedOut[i4Offset][0] = r4Factor[i4Offset] * c4FFTPassedIn[i4Offset][0] ;
    c4FFTPassedOut[i4Offset][1] = r4Factor[i4Offset] * c4FFTPassedIn[i4Offset][1] ;
  }

// second half:
  for ( i4Row=i4Nyquest+1; i4Row<i4FFTSize; i4Row++) {
    for ( i4Col=0; i4Col<i4FFTStride; i4Col++ ) {
      c4FFTPassedOut[i4Row*i4FFTStride + i4Col][0]
        = r4Factor[(i4FFTSize-i4Row)*i4FFTStride + i4Col]*c4FFTPassedIn[i4Row*i4FFTStride + i4Col][0];
      c4FFTPassedOut[i4Row*i4FFTStride + i4Col][1]
        = r4Factor[(i4FFTSize-i4Row)*i4FFTStride + i4Col]*c4FFTPassedIn[i4Row*i4FFTStride + i4Col][1];
    }
  }
  delete [] r4Factor;
}

void BCascade::convert_mmh_dbr(
	bool lDirection,  ///< IN: True for mm/h to normal, false for normal to mm/h
	float r4NoData,   ///< IN: The no-data flag
	int i4NumberRowsPassed, ///< IN: The number of rows in the image
	int i4NumberColsPassed, ///< IN: The number of columns in the image
	float* r4Data     ///< IN+OUT: The data to be converted, float[i4NumberCols*i4NumberRows]
) {
/// \fn void convert_mmh_dbr(bool lDirection,float r4NoData,int i4NumberRows,int i4NumberCols,float* r4Data)
/// \brief Transform the field from rain rate to dbr or inverse
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To convert an image from normal to rain or vice versa using probability matching
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed     19/05/09       Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
/// lDirection defines the direction of the transformation
/// lDirection = true gives conversion rain to Normal
/// lDirection = false gives conversion Normal to rain
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// None
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>

// Local parameters
	int i4Loop ; // Loop variable
  int i4InMapArraySize = i4NumberRowsPassed*i4NumberColsPassed ;
  //int i4Loop2 ;
	float r4Rain ;
  float r4Dbr ;

	// convert from rainfall to dbr
  if (lDirection) {
    for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++) {
			r4Rain = r4Data[i4Loop] ;
      if ( r4Rain < r4NoData+1 ) r4Rain = 0 ;
      r4TempArray[i4Loop] =  10*log10(r4Rain+0.03) ;
		}

    // Write the dbr back into r4Data
		for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++) {
      if ( r4Data[i4Loop] > r4NoData+1 )
        r4Data[i4Loop] = r4TempArray[i4Loop] ;
		}
  }
  // convert from dbr to rainfall
	else {
    for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++) {
      r4Dbr = r4Data[i4Loop] ;
			if ( r4Dbr > r4NoData+1 ) {
        r4Rain = pow(10.0, r4Dbr/10.0 ) - 0.03;
        if ( r4Rain < 0.1 ) r4Rain = 0.0 ;
				r4Data[i4Loop] = r4Rain ;
      }
    }
	}
}

void BCascade::convert_dbz_norm(
	bool lDirection,  ///< IN: True for mm/h to normal, false for normal to mm/h
	float r4NoData,   ///< IN: The no-data flag
	int i4NumberRowsPassed, ///< IN: The number of rows in the image
	int i4NumberColsPassed, ///< IN: The number of columns in the image
	float* r4Data     ///< IN+OUT: The data to be converted, float[i4NumberCols*i4NumberRows]
) {
/// \fn void convert_dbz_norm(bool lDirection,float r4NoData,int i4NumberRows,int i4NumberCols,float* r4Data)
/// \brief Transform the field from dbz to normal or inverse

// Local parameters
	int i4Loop ; // Loop variable
	int i4InMapArraySize = i4NumberRowsPassed*i4NumberColsPassed ;
	float r4Dbzt ;
	float r4Dbz ;
	float alpha = 0.95 ;
  float alphaInv = 1.0/alpha ;

	// convert from dbz to norm
	if (lDirection) {
		for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++) {
			r4Dbz = r4Data[i4Loop] ;
			if ( r4Dbz > 0 ) {
				r4Data[i4Loop] =  pow(r4Dbz,alpha) ;
			}
		}
	}

	// convert from norm to dbz, set negative dbzt to zero
	else {
		for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++) {
			r4Dbzt = r4Data[i4Loop] ;
			if ( r4Dbzt > r4NoData+1 && r4Dbzt < 0 ) r4Data[i4Loop] = 0.0;
			if ( r4Dbzt > 0 ) {
				r4Dbz = pow(r4Dbzt,alphaInv) ;
				r4Data[i4Loop] = r4Dbz ;
			}
		}
	}
}
void BCascade::convert_mmh_dbz(
	bool lDirection,  ///< IN: True for mm/h to dbz, false for dbz to mm/h
	float r4NoData,   ///< IN: The no-data flag
	int i4NumberRowsPassed, ///< IN: The number of rows in the image
	int i4NumberColsPassed, ///< IN: The number of columns in the image
	float* r4Data     ///< IN+OUT: The data to be converted, float[i4NumberCols*i4NumberRows]
) {
/// \fn void convert_mmh_dbz(bool lDirection,float r4NoData,int i4NumberRows,int i4NumberCols,float* r4Data)
/// \brief Transform the field from rain rate to dbz or inverse

// Local parameters
	int i4Loop ; // Loop variable
	int i4InMapArraySize = i4NumberRowsPassed*i4NumberColsPassed ;
	float r4Rain ;
	float r4Dbz ;
	float r4Zed ;
	float zrA = 200.0 ;
	float zrB = 1.6 ;

	// convert from rainfall to dbzt
	if (lDirection) {
		for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++) {
			r4Rain = r4Data[i4Loop] ;
			if ( r4Rain > 0 ) {
				r4Zed = zrA*pow(r4Rain,zrB) ;
				r4Dbz =   10*log10(r4Zed) ;
				r4Data[i4Loop] =  r4Dbz ;
			}
		}
	}

	// convert from dbz to rainfall, set negative dbz to zero
	else {
		for ( i4Loop = 0; i4Loop < i4InMapArraySize; i4Loop++) {
			r4Dbz = r4Data[i4Loop] ;
			if ( r4Dbz > r4NoData+1 && r4Dbz < 0 ) r4Data[i4Loop] = 0.0;
			if ( r4Dbz > 0 ) {
				r4Zed = pow(10.0, r4Dbz/10.0 ) ;
				r4Rain = pow((float)(r4Zed/zrA), (float)(1.0/zrB)) ;
				if ( r4Rain < 0.03 ) r4Rain = 0.0 ;
				r4Data[i4Loop] = r4Rain ;
			}
		}
	}
}


bool BCascade::write_cascade(
  char *cOutFileName   ///< Name of the file for the cascade data, char[512]
) {
/// \fn  bool BCascade::write_cascade(char *cOutFileName)
/// \brief Write the cascade data to a file
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To write the cascade parameters and data into a file for use later
///
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed     19/03/05       Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
/// The file name must include the entire path
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
/// false if the write to the file failed else
/// true
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///   FILE *fFile; The file stream used as output
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///  float *xVel = new float [i4CascadeArraySize] ; // Array of e-w velocities
///  float *yVel = new float [i4CascadeArraySize] ; // Array of n-s velocities
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// char cLogFileName[512] ;             Name of error log file
/// char cOpticalFileName[512] ;         Name of the optical flow file name
/// time_t sMapTimes[8] ;                List of times for each map
/// size_t i4CascadeArraySize ;          Size of the cascade array
/// int i4CascadeSize;                   Size of the cascade assumed to be square
/// int i4FFTArraySize ;                 Actual size of the fft array = FFTSize*(1+FFTSize/2)
/// int i4FFTSize ;                      Logical size of the fft array
/// size_t i4InMapArraySize ;            Array size for input data
/// int i4NumberCascades;                Number of cascades to keep
/// int i4NumberCols;                    Size of the input rain map
/// int i4NumberLevels;                  Image size and number of cascade levels
/// int i4NumberMaps;                    Number of maps to keep
/// int i4NumberRows;                    Size of the input rain map
/// int i4NumberLags;                    Order of the ar model
/// int i4TimeStep ;                     Time step of the radar and NWP in minutes
/// int i4ImageNumber;                   Number of images since the cascade was initialised
/// float r4FieldMean;                   Mean value of the rain rate (in dBr )
/// float r4FieldStd;                    Standard deviation of the rain rate
/// float r4NoData;                      Flag for non-valid data
/// float r4PixelSize;                   Pixel size in km
/// float r4RainFrac;                    Fraction of image > 0.2 mm/h
/// float r4CondMean ;                   Conditional mean rain rate in mm/h
/// float r4RainMean;                    Mean in mm/h
/// float r4ScaleRatio ;                 Ratio of scales between level n+1 and level n in the cascade: < 1
/// float r4RainStd;                     Std in mm/h
/// float r4CascadeLevelMean[16] ;       The mean of the cascade level
/// float r4CascadeLevelStd[16] ;        The standard deviation of the cascade level
/// float* r4Correlation;                Auto-correlations for the cascade lag 1 in [0] to [i4NumberLevels-1],
/// float* r4Phi;                        AR(2) model parameters for updates
/// float** r4CascadeStack;              Array of pointers to the cascade arrays
/// float* r4Maps[8] ;                   Array of previous maps
/// Optical_Flow* sRadarAdvection;       The velocities determined from the radar analyses
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>

// Make sure that the PC version can open the files
  FILE *fFile;                // File stream for output
  fFile = fopen (cOutFileName, "wb") ;

  int i4NoBytes ;             // Number of bytes to write to the file
  int i4IntParameters[32] ;   // Array for the integer parameters
  float r4FltParameters[32] ; // Array for the float parameters
  int i4Loop ;                // Loop variable
  int i4WriteStatus ;         // Status of the write to the file

// allocate the dynamic memory
  float *xVel = new float [i4CascadeArraySize] ; // Array of e-w velocities
  float *yVel = new float [i4CascadeArraySize] ; // Array of n-s velocities

  for ( i4Loop = 0; i4Loop < 32; i4Loop++ ) {
    i4IntParameters[i4Loop] = 0 ;
    r4FltParameters[i4Loop] = 0 ;
  }
  i4NoBytes = 512 ;

  i4WriteStatus = fwrite((char*) cLogFileName, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    fclose (fFile) ;
    return false ;
  }

  i4WriteStatus = fwrite((char*) cOpticalFileName, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    fclose (fFile) ;
    return false ;
  }

  i4NoBytes = 8 * sizeof (time_t) ;
  i4WriteStatus = fwrite((char*) sMapTimes, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    fclose (fFile) ;
    return false ;
  }

  i4IntParameters[0]  = i4CascadeArraySize ;
  i4IntParameters[1]  = i4CascadeSize ;
  i4IntParameters[2]  = i4FFTArraySize ;
  i4IntParameters[3]  = i4FFTSize ;
  i4IntParameters[4]  = i4ImageNumber ;
  i4IntParameters[5]  = i4InMapArraySize ;
  i4IntParameters[6]  = i4NumberCascades ;
  i4IntParameters[7]  = i4NumberCols ;
  i4IntParameters[8]  = i4NumberLevels ;
  i4IntParameters[9]  = i4NumberMaps ;
  i4IntParameters[10] = i4NumberRows ;
  i4IntParameters[11] = i4NumberLags ;
  i4IntParameters[12] = i4TimeStep ;

  i4NoBytes = 32 * sizeof(int) ;
  i4WriteStatus = fwrite((char*) i4IntParameters, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    fclose (fFile) ;
    return false ;
  }

  // collect up the float parameters
  r4FltParameters[0] = r4FieldMean ;
  r4FltParameters[1] = r4FieldStd ;
  r4FltParameters[2] = r4NoData ;
  r4FltParameters[3] = r4PixelSize ;
  r4FltParameters[4] = r4RainFrac ;
  r4FltParameters[5] = r4CondMean ;
  r4FltParameters[6] = r4RainMean;
  r4FltParameters[7] = r4ScaleRatio;
	r4FltParameters[8] = r4RainStd ;

	// AWS 13 October 2009 Fixed bug
	r4FltParameters[9] = r4BetaOne ;
	r4FltParameters[10] = r4BetaTwo ;

	i4NoBytes = 32 * sizeof(float) ;
	i4WriteStatus = fwrite((char*) r4FltParameters, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    fclose (fFile) ;
    return false ;
  }

  i4NoBytes = i4NumberLevels*sizeof ( float ) ;
  i4WriteStatus = fwrite((char*) r4CascadeLevelMean, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    fclose (fFile) ;
    return false ;
  }

  i4WriteStatus = fwrite((char*) r4CascadeLevelStd, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    fclose (fFile) ;
    return false ;
  }

  i4NoBytes = i4NumberLags*i4NumberLevels * sizeof ( float )  ;
  i4WriteStatus = fwrite((char*) r4Correlation, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    fclose (fFile) ;
    return false ;
  }

  i4NoBytes = (i4NumberLags+1)*i4NumberLevels * sizeof ( float )  ;
  i4WriteStatus = fwrite((char*) r4Phi, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    fclose (fFile) ;
    return false ;
  }

// now write out the cascade arrays
  i4NoBytes = i4NumberLevels*i4CascadeArraySize * sizeof ( float ) ;
  for ( i4Loop = 0; i4Loop < i4NumberCascades; i4Loop++ ) {
    i4WriteStatus = fwrite((char*) r4CascadeStack[i4Loop], 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    fclose (fFile) ;
    return false ;
  }
  }

// Write the transformed input rainfall maps
  i4NoBytes = i4CascadeArraySize * sizeof ( float ) ;
  for ( i4Loop = 0; i4Loop < i4NumberMaps; i4Loop++ ) {
    i4WriteStatus = fwrite((char*) r4Maps[i4Loop], 1, i4NoBytes, fFile);
    if ( i4WriteStatus != i4NoBytes ){
      fclose (fFile) ;
      return false ;
    }
  }

// the curent advection vectors
  sRadarAdvection->Get_Velocities( xVel, yVel ) ;

	i4NoBytes = i4CascadeArraySize*sizeof(float) ;
  i4WriteStatus = fwrite((char*) xVel, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    fclose (fFile) ;
    return false ;
  }

  i4WriteStatus = fwrite((char*) yVel, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    fclose (fFile) ;
    return false ;
  }

// Write the untransformed rainfall map
  i4NoBytes = i4InMapArraySize*sizeof(float) ;
  i4WriteStatus = fwrite((char*) r4RainMap, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    fclose (fFile) ;
    return false ;
  }

  delete [] xVel ;
  delete [] yVel ;
  if (fclose(fFile)) {
    cout << "Error closing file " << cOutFileName << endl;
    return false;
  }
  return true ;
}

// AWS Move the original write_cascade aside to test borland codeguard
bool BCascade::write_cascade_old(
  char *cOutFileName   ///< Name of the file for the cascade data, char[512]
) {
/// \fn  bool BCascade::write_cascade(char *cOutFileName)
/// \brief Write the cascade data to a file
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To write the cascade parameters and data into a file for use later
///
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed     19/03/05       Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
/// The file name must include the entire path
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
/// false if the write to the file failed else
/// true
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///   FILE *fFile; The file stream used as output
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///  float *xVel = new float [i4CascadeArraySize] ; // Array of e-w velocities
///  float *yVel = new float [i4CascadeArraySize] ; // Array of n-s velocities
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// char cLogFileName[512] ;             Name of error log file
/// char cOpticalFileName[512] ;         Name of the optical flow file name
/// time_t sMapTimes[8] ;                List of times for each map
/// size_t i4CascadeArraySize ;          Size of the cascade array
/// int i4CascadeSize;                   Size of the cascade assumed to be square
/// int i4FFTArraySize ;                 Actual size of the fft array = FFTSize*(1+FFTSize/2)
/// int i4FFTSize ;                      Logical size of the fft array
/// size_t i4InMapArraySize ;            Array size for input data
/// int i4NumberCascades;                Number of cascades to keep
/// int i4NumberCols;                    Size of the input rain map
/// int i4NumberLevels;                  Image size and number of cascade levels
/// int i4NumberMaps;                    Number of maps to keep
/// int i4NumberRows;                    Size of the input rain map
/// int i4NumberLags;                    Order of the ar model
/// int i4TimeStep ;                     Time step of the radar and NWP in minutes
/// int i4ImageNumber;                   Number of images since the cascade was initialised
/// float r4FieldMean;                   Mean value of the rain rate (in dBr )
/// float r4FieldStd;                    Standard deviation of the rain rate
/// float r4NoData;                      Flag for non-valid data
/// float r4PixelSize;                   Pixel size in km
/// float r4RainFrac;                    Fraction of image > 0.2 mm/h
/// float r4CondMean ;                   Conditional mean rain rate in mm/h
/// float r4RainMean;                    Mean in mm/h
/// float r4ScaleRatio ;                 Ratio of scales between level n+1 and level n in the cascade: < 1
/// float r4RainStd;                     Std in mm/h
/// float r4CascadeLevelMean[16] ;       The mean of the cascade level
/// float r4CascadeLevelStd[16] ;        The standard deviation of the cascade level
/// float* r4Correlation;                Auto-correlations for the cascade lag 1 in [0] to [i4NumberLevels-1],
/// float* r4Phi;                        AR(2) model parameters for updates
/// float** r4CascadeStack;              Array of pointers to the cascade arrays
/// float* r4Maps[8] ;                   Array of previous maps
/// Optical_Flow* sRadarAdvection;       The velocities determined from the radar analyses
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>

// Make sure that the PC version can open the files
#ifdef PC_HOST
  int i4OutFile = open ( cOutFileName, O_CREAT|O_TRUNC|O_BINARY|O_WRONLY,
                         S_IWRITE|S_IREAD ) ;
#else
  int i4OutFile = open ( cOutFileName, O_CREAT|O_TRUNC|O_BINARY|O_WRONLY,
                         S_IWRITE|S_IREAD|S_IRGRP|S_IROTH) ;
#endif

  if ( i4OutFile == -1 ) return false ;

  int i4NoBytes ;             // Number of bytes to write to the file
  int i4IntParameters[32] ;   // Array for the integer parameters
  float r4FltParameters[32] ; // Array for the float parameters
  int i4Loop ;                // Loop variable
  int i4WriteStatus ;         // Status of the write to the file
  FILE *fFile;                // File stream for output

// allocate the dynamic memory
  float *xVel = new float [i4CascadeArraySize] ; // Array of e-w velocities
  float *yVel = new float [i4CascadeArraySize] ; // Array of n-s velocities

// attach a stream and use streamio calls to buffer the output
  fFile = fdopen(i4OutFile, "wb");
  if (setvbuf(fFile, NULL, _IOFBF, BUFFERSIZE))
    cout << "Warning: setting a buffer failed for file " << cOutFileName << endl;

  for ( i4Loop = 0; i4Loop < 32; i4Loop++ ) {
    i4IntParameters[i4Loop] = 0 ;
    r4FltParameters[i4Loop] = 0 ;
  }
  i4NoBytes = 512 ;

  i4WriteStatus = fwrite((char*) cLogFileName, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

  i4WriteStatus = fwrite((char*) cOpticalFileName, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

  i4NoBytes = 8 * sizeof (time_t) ;
  i4WriteStatus = fwrite((char*) sMapTimes, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

  i4IntParameters[0]  = i4CascadeArraySize ;
  i4IntParameters[1]  = i4CascadeSize ;
  i4IntParameters[2]  = i4FFTArraySize ;
  i4IntParameters[3]  = i4FFTSize ;
  i4IntParameters[4]  = i4ImageNumber ;
  i4IntParameters[5]  = i4InMapArraySize ;
  i4IntParameters[6]  = i4NumberCascades ;
  i4IntParameters[7]  = i4NumberCols ;
  i4IntParameters[8]  = i4NumberLevels ;
  i4IntParameters[9]  = i4NumberMaps ;
  i4IntParameters[10] = i4NumberRows ;
  i4IntParameters[11] = i4NumberLags ;
  i4IntParameters[12] = i4TimeStep ;

  i4NoBytes = 32 * sizeof(int) ;
  i4WriteStatus = fwrite((char*) i4IntParameters, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

  // collect up the float parameters
  r4FltParameters[0] = r4FieldMean ;
  r4FltParameters[1] = r4FieldStd ;
  r4FltParameters[2] = r4NoData ;
  r4FltParameters[3] = r4PixelSize ;
  r4FltParameters[4] = r4RainFrac ;
  r4FltParameters[5] = r4CondMean ;
  r4FltParameters[6] = r4RainMean;
  r4FltParameters[7] = r4ScaleRatio;
	r4FltParameters[8] = r4RainStd ;

	// AWS 13 October 2009 Fixed bug
	r4FltParameters[9] = r4BetaOne ;
	r4FltParameters[10] = r4BetaTwo ;


	i4NoBytes = 32 * sizeof(float) ;
	i4WriteStatus = fwrite((char*) r4FltParameters, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

  i4NoBytes = i4NumberLevels*sizeof ( float ) ;
  i4WriteStatus = fwrite((char*) r4CascadeLevelMean, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

  i4WriteStatus = fwrite((char*) r4CascadeLevelStd, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

  i4NoBytes = i4NumberLags*i4NumberLevels * sizeof ( float )  ;
  i4WriteStatus = fwrite((char*) r4Correlation, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

  i4NoBytes = (i4NumberLags+1)*i4NumberLevels * sizeof ( float )  ;
  i4WriteStatus = fwrite((char*) r4Phi, 1, i4NoBytes, fFile);
	if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

// now write out the cascade arrays
  i4NoBytes = i4NumberLevels*i4CascadeArraySize * sizeof ( float ) ;
  for ( i4Loop = 0; i4Loop < i4NumberCascades; i4Loop++ ) {
    i4WriteStatus = fwrite((char*) r4CascadeStack[i4Loop], 1, i4NoBytes, fFile);
    if ( i4WriteStatus != i4NoBytes ){
      close ( i4OutFile) ;
      return false ;
    }
  }

// Write the transformed input rainfall maps
  i4NoBytes = i4CascadeArraySize * sizeof ( float ) ;
  for ( i4Loop = 0; i4Loop < i4NumberMaps; i4Loop++ ) {
    i4WriteStatus = fwrite((char*) r4Maps[i4Loop], 1, i4NoBytes, fFile);
    if ( i4WriteStatus != i4NoBytes ){
      close ( i4OutFile) ;
      return false ;
    }
  }

// the curent advection vectors
  sRadarAdvection->Get_Velocities( xVel, yVel ) ;

	i4NoBytes = i4CascadeArraySize*sizeof(float) ;
  i4WriteStatus = fwrite((char*) xVel, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

  i4WriteStatus = fwrite((char*) yVel, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

// Write the untransformed rainfall map
  i4NoBytes = i4InMapArraySize*sizeof(float) ;
  i4WriteStatus = fwrite((char*) r4RainMap, 1, i4NoBytes, fFile);
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

  delete [] xVel ;
  delete [] yVel ;
  if (fclose(fFile)) {
    cout << "Error closing file " << cOutFileName << endl;
    return false;
  }
  return true ;
}

bool BCascade::read_cascade(
  char *cInCascadeFileName   ///< Name of the cascade file to be read, char[512]
){
/// \fn  bool BCascade::read_cascade(char *cInCascadeFileName)
/// \brief Read a cacade back from a file generated by write_cascade
  int i4InFile = open(cInCascadeFileName, O_BINARY|O_RDONLY) ;
  if ( i4InFile == -1 ) return false ;
  read_cascade_parameters(i4InFile) ;
  read_cascade_data(i4InFile) ;
  close (i4InFile) ;
  return true ;
}

bool BCascade::read_cascade_parameters(
  int i4OutFile  ///< Handle of file to be read
) {
/// \fn bool BCascade::read_cascade_parameters(int i4OutFile)
/// \brief Read in the cascade parameters from a file generated by write_cascade
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To read the cascade parameters and data from a file
///
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed     19/03/05       Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// true on sucess
/// false on failure
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
/// char cLogFileName[512] ;             Name of error log file
/// char cOpticalFileName[512] ;         Name of the optical flow file name
/// time_t sMapTimes[8] ;                List of times for each map
/// size_t i4CascadeArraySize ;          Size of the cascade array
/// int i4CascadeSize;                   Size of the cascade assumed to be square
/// int i4FFTArraySize ;                 Actual size of the fft array = FFTSize*(1+FFTSize/2)
/// int i4FFTSize ;                      Logical size of the fft array
/// size_t i4InMapArraySize ;            Array size for input data
/// int i4NumberCascades;                Number of cascades to keep
/// int i4NumberCols;                    Size of the input rain map
/// int i4NumberLevels;                  Image size and number of cascade levels
/// int i4NumberMaps;                    Number of maps to keep
/// int i4NumberRows;                    Size of the input rain map
/// int i4NumberLags;                    Order of the ar model
/// int i4TimeStep ;                     Time step of the radar and NWP in minutes
/// int i4ImageNumber;                   Number of images since the cascade was initialised
/// float r4FieldMean;                   Mean value of the rain rate (in dBr )
/// float r4FieldStd;                    Standard deviation of the rain rate
/// float r4NoData;                      Flag for non-valid data
/// float r4PixelSize;                   Pixel size in km
/// float r4RainFrac;                    Fraction of image > 0.2 mm/h
/// float r4CondMean ;                   Conditional mean rain rate in mm/h
/// float r4RainMean;                    Mean in mm/h
/// float r4ScaleRatio ;                 Ratio of scales between level n+1 and level n in the cascade: < 1
/// float r4RainStd;                     Std in mm/h
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>

// Local parameters
  int i4NoBytes ;             // number of bytes to write to the file
  int i4IntParameters[32] ;   // all the integer parameters
  float r4FltParameters[32] ; // all the float parameters
  int i4Loop ;                // loop variable
  int i4WriteStatus ;         // status of the write to the file

  for ( i4Loop = 0; i4Loop < 32; i4Loop++ ) {
    i4IntParameters[i4Loop] = 0 ;
    r4FltParameters[i4Loop] = 0 ;
  }

  i4NoBytes = 512 ;

  i4WriteStatus = read( i4OutFile, cLogFileName, i4NoBytes ) ;
  if ( i4WriteStatus != i4NoBytes ){
    return false ;
	}

  i4WriteStatus = read( i4OutFile, cOpticalFileName, i4NoBytes ) ;
  if ( i4WriteStatus != i4NoBytes ){
    return false ;
  }

  i4NoBytes = 8 * sizeof (time_t) ;
  i4WriteStatus = read( i4OutFile, sMapTimes, i4NoBytes ) ;
  if ( i4WriteStatus != i4NoBytes ){
    return false ;
  }

  i4NoBytes = 32 * sizeof(int) ;
  i4WriteStatus = read( i4OutFile, i4IntParameters, i4NoBytes ) ;
  if ( i4WriteStatus != i4NoBytes ){
    return false ;
  }

  i4CascadeArraySize = i4IntParameters[0] ;
  i4CascadeSize = i4IntParameters[1]   ;
  i4FFTArraySize = i4IntParameters[2]  ;
  i4FFTSize = i4IntParameters[3]  ;
  i4ImageNumber = i4IntParameters[4]  ;
  i4InMapArraySize = i4IntParameters[5]  ;
  i4NumberCascades = i4IntParameters[6]  ;
  i4NumberCols = i4IntParameters[7]  ;
  i4NumberLevels = i4IntParameters[8]  ;
	i4NumberMaps = i4IntParameters[9] ;
  i4NumberRows = i4IntParameters[10] ;
  i4NumberLags = i4IntParameters[11] ;
  i4TimeStep   = i4IntParameters[12] ;

  i4NoBytes = 32 * sizeof(float) ;
  i4WriteStatus = read( i4OutFile, r4FltParameters, i4NoBytes ) ;
  if ( i4WriteStatus != i4NoBytes ){
    return false ;
  }
  r4FieldMean = r4FltParameters[0] ;
  r4FieldStd  = r4FltParameters[1] ;
  r4NoData    = r4FltParameters[2] ;
  r4PixelSize = r4FltParameters[3] ;
  r4RainFrac  = r4FltParameters[4] ;
  r4CondMean  = r4FltParameters[5] ;
  r4RainMean  = r4FltParameters[6];
  r4ScaleRatio = r4FltParameters[7];
	r4RainStd    = r4FltParameters[8];

		// AWS 13 October 2009 Fixed bug
	r4BetaOne = r4FltParameters[9] ;
	r4BetaTwo = r4FltParameters[10]   ;

	return true ;
}

bool BCascade::read_cascade_data(
  int i4OutFile   ///< Handle of file to be read
) {
/// \fn bool BCascade::read_cascade_data(int i4OutFile)
/// \brief Read in the cascade arrays from a file
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To read the cascade arrays from a file
///
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed     19/03/05       Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
/// Assumes that read_cascade_parameters has been called followed by
/// allocate_memory before read_cascade_data is called
///
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// true on success
/// false on failure
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///  float *xVel = new float [i4CascadeArraySize] ;
///  float *yVel = new float [i4CascadeArraySize] ;
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
///  float r4CascadeLevelMean[16] ;       The mean of the cascade level
///  float r4CascadeLevelStd[16] ;        The standard deviation of the cascade level
///  float* r4Correlation;                Auto-correlations for the cascade lag 1 in [0] to [i4NumberLevels-1],
///  float* r4Phi;                        AR(2) model parameters for updates
///  float** r4CascadeStack;              Array of pointers to the cascade arrays
///  float* r4Maps[8] ;                   Array of previous maps
///  float* r4RainMap ;               Array of untransformed rainfall intensities
///  Optical_Flow* sRadarAdvection;       The velocities determined from the radar analyses
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>

// Local parameters
  int i4NoBytes ;
  int i4Loop ;
  int i4WriteStatus ;

  i4NoBytes = i4NumberLevels*sizeof(float) ;
  i4WriteStatus = read( i4OutFile, r4CascadeLevelMean, i4NoBytes ) ;
  if ( i4WriteStatus != i4NoBytes ){
    return false ;
  }
  i4WriteStatus = read( i4OutFile, r4CascadeLevelStd, i4NoBytes ) ;
  if ( i4WriteStatus != i4NoBytes ){
    return false ;
  }
  i4NoBytes = i4NumberLags*i4NumberLevels*sizeof(float) ;
  i4WriteStatus = read( i4OutFile, r4Correlation, i4NoBytes ) ;
  if ( i4WriteStatus != i4NoBytes ){
    return false ;
  }
  i4NoBytes = (i4NumberLags+1)*i4NumberLevels*sizeof(float) ;
  i4WriteStatus = read( i4OutFile, r4Phi, i4NoBytes ) ;
  if ( i4WriteStatus != i4NoBytes ){
    return false ;
  }
  i4NoBytes = i4NumberLevels*i4CascadeArraySize * sizeof ( float ) ;
  for ( i4Loop = 0; i4Loop < i4NumberCascades; i4Loop++ ) {
    i4WriteStatus = read( i4OutFile, r4CascadeStack[i4Loop], i4NoBytes ) ;
    if ( i4WriteStatus != i4NoBytes ){
       return false ;
    }
  }
  i4NoBytes = i4CascadeArraySize * sizeof ( float ) ;
  for ( i4Loop = 0; i4Loop < i4NumberMaps; i4Loop++ ) {
    i4WriteStatus = read( i4OutFile, r4Maps[i4Loop], i4NoBytes ) ;
    if ( i4WriteStatus != i4NoBytes ){
      return false ;
    }
  }

// the curent advection vectors
  float *xVel = new float [i4CascadeArraySize] ;
  float *yVel = new float [i4CascadeArraySize] ;

  i4NoBytes = i4CascadeArraySize*sizeof(float) ;
  i4WriteStatus = read( i4OutFile, xVel, i4NoBytes ) ;
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

  i4WriteStatus = read( i4OutFile, yVel, i4NoBytes ) ;
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }
  sRadarAdvection->Set_Velocities( xVel, yVel ) ;

  delete [] xVel ;
  delete [] yVel ;

// read in the untransformed rainfall intensities
  i4NoBytes = i4InMapArraySize*sizeof(float) ;
  i4WriteStatus = read( i4OutFile, r4RainMap, i4NoBytes ) ;
  if ( i4WriteStatus != i4NoBytes ){
    close ( i4OutFile) ;
    return false ;
  }

  return true ;
}

void BCascade::decompose_and_track(  ) {
/// \fn  void BCascade::decompose_and_track()
/// \brief Track the rainfall field and decompose into spectral components
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To call the routines to decompose the latest input file, and calculate
/// the advection velocities.
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00	        Neill Bowler    13/06/03     Inital Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// None
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// float r4CascadeLevelMean[16] ;     The mean of the cascade level
/// float r4CascadeLevelStd[16] ;      The standard deviation of the cascade level
/// float** r4CascadeStack;            Array of pointers to the cascade arrays
/// Optical_Flow* sRadarAdvection;     The velocities determined from the radar analyses
///
/// STANDARD LIBRARIES USED:
///
/// None
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>
//	decompose_fftw( r4Maps[0], r4CascadeLevelMean, r4CascadeLevelStd, r4CascadeStack[0] ) ;
	decompose_mask( r4Maps[0], r4CascadeLevelMean, r4CascadeLevelStd, r4CascadeStack[0] ) ;
	if ( i4ImageNumber > 1 ){
		update_track( sRadarAdvection ) ; // update the field advection
	}
}

void BCascade::decompose( ) {
/// \fn  void BCascade::decompose()
/// \brief Wrapper to call decompose_fftw
//	decompose_fftw( r4Maps[0], r4CascadeLevelMean, r4CascadeLevelStd, r4CascadeStack[0] ) ;
	decompose_mask( r4Maps[0], r4CascadeLevelMean, r4CascadeLevelStd, r4CascadeStack[0] ) ;

}

void BCascade::advect_weights(
  int i4NoSteps,   ///< Number of steps in the advection
  int i4CascadeNo  ///< Number of the cascade to be advected
) {
/// \fn void BCascade::advect_weights(int i4NoSteps,int i4CascadeNo)
/// \brief Advect a cascade forward by a number of time steps
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To advect the cacade weights in the top cascade by the number of steps passed
/// Used by the Make_Forecast method in the merged_cascade class
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00          Alan Seed       26/05/05     Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
/// None
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// ARRAY NAME: r4TempMap 1,  2,
/// ARRAY TYPE: float*
/// DIMENSIONS: i4CascadeArraySize
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// int i4NumberLevels;           Image size and number of cascade levels
/// int i4CascadeArraySize;       Size of cascade level
/// float*** r4CascadeStack[3];   Array of pointers to the cascade arrays
///
/// STANDARD LIBRARIES USED:
///
///  None
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>

// Local variables
  int i4Level;                  // Loop counter, level in cascade
  size_t i4Loop;                // Loop counter
  size_t i4CascadeLevelOffset ; // Start of cascade level
  float r4TempVal ;

// Dynamically allocate arrays
  float* r4TempMap2    = new float [i4CascadeArraySize] ;

// Loop over the levels in the cascade
  for ( i4Level = 0; i4Level < i4NumberLevels; i4Level++ )
  {
// First load up the cascade levels and advect
    i4CascadeLevelOffset = i4Level * i4CascadeArraySize ;
    const float *r4TempMap1 = &r4CascadeStack[i4CascadeNo][i4CascadeLevelOffset];

    advect_cascade(i4NoSteps, (int) r4NoData,  r4TempMap1, r4TempMap2);

    for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++) {
      if ( i4DataMask[i4Loop] == 1 )
        r4TempVal = r4TempMap2[i4Loop] ;
      else
        r4TempVal = r4NoData ;
      r4CascadeStack[i4CascadeNo][i4CascadeLevelOffset+i4Loop] = r4TempVal ;
    }
  }

// Delete all dymanic arrays
  delete [] r4TempMap2 ;
  r4TempMap2 = NULL ;
}

void normalize_ms_rain (
  float r4NoData,       ///< IN: Missing data flag
  int i4ArrayLength,    ///< IN: Length of input array
  float r4ReqMean,      ///< IN: Required mean
  float r4ReqStd,       ///< IN: Required std
  int   *i4DataMask,    ///< IN: Mask for the area with data in the input image
  float *r4Data         ///< IN/OUT: Data array [i4ArrayLength] in size
) {
/// \fn void normalize_ms_rain(float r4NoData,int i4ArrayLength,float r4ReqMean,float r4ReqStd,int *i4DataMask,float *r4Data)
/// \brief Adjust a field of rainfall rates to a desired mean and standard deviation
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To normalise a field of rainfall intensities to a required mean and standard deviation
///
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed     19/03/05       Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         AWS Take out the check on the max rain
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// None
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// None
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>

  float r4ThisMean = 0 ;    // Actual mean for the input field
  float r4ThisSS = 0;       // Accumulator for calculating the variance
  int n = 0 ;               // Number of valid pixels
  float r4ThisStd ;         // Actual standard deviation of the input field
  int i4Loop ;              // Loop variable
  float r4Val ;             // Scratch variable
  float r4MaxRain = 100.0 ; // Max value to use in the calculation of stats

  for ( i4Loop = 0; i4Loop < i4ArrayLength; i4Loop++ ) {
    r4Val =  r4Data[i4Loop] ;
    if (i4DataMask[i4Loop] == 1 && r4Val > r4NoData+1 && r4Val < r4MaxRain) {
      n++ ;
      r4ThisMean += r4Val ;
    }
  }
  if ( n == 0 ){
    cout << "WARNING: normalize_ms_rain No valid data pixels found in the image, normalization not done" << endl ;
     return ;
  }
  r4ThisMean /= (float)n ;

  for ( i4Loop = 0; i4Loop < i4ArrayLength; i4Loop++ ) {
    r4Val =  r4Data[i4Loop] ;
    if (i4DataMask[i4Loop] == 1 && r4Val > r4NoData+1 && r4Val < r4MaxRain ) {
      r4ThisSS += (r4Val-r4ThisMean)*(r4Val-r4ThisMean) ;
    }
  }
  if ( n > 0 )
    r4ThisStd = sqrt(r4ThisSS/(float)n ) ;
  else
    r4ThisStd = 1.0 ;

  if ( r4ReqStd < 0.000001 ){
    cout << "WARNING: normalize_ms_rain Standard deviation of rain field < 0.000001, normalization not done" << endl ;
    return  ;
  }
  float r4Factor = r4ReqStd/r4ThisStd ;

  if ( r4Factor > 1.5 ) r4Factor = 1.5 ;
  if ( r4Factor < 0.75 ) r4Factor = 0.75 ;
  cout << " INFORMATION: normalize_ms_rain r4ThisMean " << r4ThisMean << " r4ReqMean " << r4ReqMean << " r4ThisStd " << r4ThisStd << " r4ReqStd " << r4ReqStd << "adj factor " << r4Factor <<  endl;

  for ( i4Loop = 0; i4Loop < i4ArrayLength; i4Loop++) {
    r4Val = r4Data[i4Loop] ;
    if ( r4Val > r4NoData+1 && i4DataMask[i4Loop] == 1 )
      r4Val *= r4Factor ;
    else r4Val = r4NoData ;
    r4Data[i4Loop] = r4Val ;
  }

// Apply a constraint on the magnitude of the adjustment to the variance
// to prevent loss of variance in the merged forecast when the variance of the noise blows up
//  if ( (r4ThisStd/r4ReqStd) > 1.8 ) {
//    r4ReqStd = r4ThisStd/1.8 ;
//  }
//  for ( i4Loop = 0; i4Loop < i4ArrayLength; i4Loop++ ) {
//    r4Val =  r4Data[i4Loop] ;
//    if (i4DataMask[i4Loop] == 1 && r4Val > r4NoData+1 ) {
//      if ( r4Val > 0 ) {
//        z = (r4Val-r4ThisMean)/r4ThisStd ;
//        r4Rain =  z*r4ReqStd + r4ReqMean ;
//        if ( r4Rain < 0 ) r4Rain = 0 ;
//      }
//      else
//        r4Rain = 0 ;
//      r4Data[i4Loop] = r4Rain ;
//    }
//    else r4Data[i4Loop] = r4NoData ;
//  }
}

void BCascade::normalize_ms (
  float r4NoData,       ///< Missing data flag
  int i4ArrayLength,    ///< Length of input array
  float r4ReqMean,      ///< Required mean
  float r4ReqStd,       ///< Required std
  int   *i4DataMask,    ///< Mask for the area with data in the input image
  float *r4Data         ///< Data array [i4ArrayLength] in size
) {
/// \fn void normalize_ms(float r4NoData,int i4ArrayLength,float r4ReqMean,float r4ReqStd,int *i4DataMask,float *r4Data)
/// \brief Adjust a field of dbr values to a desired mean and standard deviation
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To normalise a field of dbr values to a required mean and standard deviation
///
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed     19/03/05       Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// None
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// None
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>

  float r4ThisMean = 0 ;  // Mean of the input field
  float r4ThisSS = 0;     // Accumulator for the calculation of the variance
  int n = 0 ;             // Number of valid points in the field
  float r4ThisStd ;       // Standard deviation of the input field
  int i4Loop ;            // Loop counter
  float r4Val ;           // Scratch variable
  float z ;               // Standard variate (mean=0, var = 1)
  float r4AdjStd ;        // Adjust to this standard deviation
  float r4MaxDbr = 10*log10(150.0) ; // Threshold at the max value

  for ( i4Loop = 0; i4Loop < i4ArrayLength; i4Loop++ ) {
    r4Val =  r4Data[i4Loop] ;
    if ( r4Val > r4MaxDbr ) r4Val = r4MaxDbr ;
    if (i4DataMask[i4Loop] == 1 && r4Val > r4NoData+1 ) {
      n++ ;
      r4ThisMean += r4Val ;
    }
  }
  if ( n == 0 ){
    cout << "WARNING: BCascade::normalize_ms  No valid data pixels found in the image, normalization not done" << endl ;
    return ;
  }
  r4ThisMean /= (float)n ;

  for ( i4Loop = 0; i4Loop < i4ArrayLength; i4Loop++ ) {
    r4Val =  r4Data[i4Loop] ;
    if ( r4Val > r4MaxDbr ) r4Val = r4MaxDbr ;
    if (i4DataMask[i4Loop] == 1 && r4Val > r4NoData+1 ) {
      r4ThisSS += (r4Val-r4ThisMean)*(r4Val-r4ThisMean) ;
    }
  }
  r4ThisStd = sqrt(r4ThisSS/(float)n ) ;
  if ( r4ThisStd < 0.000001 ){
    cout << "WARNING: BCascade::normalize_ms Standard deviation of rain field < 0.000001, normalization not done" << endl ;
    return ;
  }

// Work out the target standard deviation
  //  if ( r4AdjStd > 1.25*r4ThisStd ) r4AdjStd = 1.25*r4ThisStd ;
  //  if ( r4AdjStd < 0.75*r4ThisStd ) r4AdjStd = 0.75*r4ThisStd ;

  float r4AdjustRatio = r4ReqStd/r4ThisStd ;
  float r4MaxAdjustRatio = 1.25 ;
  float r4MinAdjustRatio = 0.75 ;
  r4AdjStd = r4ReqStd ; // Target standard deviation adjustment

  if ( r4AdjustRatio > r4MaxAdjustRatio ){

#ifdef DEBUG
    if ( fLogFile != NULL ) {
      fprintf(fLogFile,
        "Information: BCascade:normalize_ms Adjustment factor = %f exceeds the maximum limit %f\n",
        r4AdjustRatio, r4MaxAdjustRatio ) ;
    }
#endif
    r4AdjStd = r4MaxAdjustRatio*r4ThisStd ;
  }

  if ( r4AdjustRatio < r4MinAdjustRatio ){

#ifdef DEBUG
    if ( fLogFile != NULL ) {
      fprintf(fLogFile,
        "Information: BCascade:normalize_ms Adjustment factor = %f is less than the minimum limit %f\n",
        r4AdjustRatio, r4MinAdjustRatio ) ;
    }
#endif
    r4AdjStd = r4MinAdjustRatio*r4ThisStd ;
  }

// Now adjust the field to the required mean and target standard deviation
  for ( i4Loop = 0; i4Loop < i4ArrayLength; i4Loop++ ) {
    r4Val =  r4Data[i4Loop] ;
    if (i4DataMask[i4Loop] == 1 && r4Val > r4NoData+1 ) {
     z = (r4Val-r4ThisMean)/r4ThisStd ;
     r4Data[i4Loop] = z*r4AdjStd + r4ReqMean ;
    }
    else r4Data[i4Loop] = r4NoData ;
  }
}


void  BCascade::calc_power_exponent(
  fftw_complex *c4Temp,   ///< IN:  FFT calculated using FFTW
  float& r4Beta1,         ///< OUT: Slope of the power spectrum above the scale break
  float& r4Beta2          ///< OUT: Slope of the power spectrum below the scale break
) {
/// \fn void  BCascade::calc_power_exponent( fftw_complex *c4Temp, float& r4Beta1, float& r4Beta2 )
/// \brief Calculate the power law exponent for the FFT above and below the scaling break
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To calculate the slope of the power spectrum above and below the scaling break
///
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed     19/03/05       Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///  float* r4PowerSpec = new float [i4Nyquest] ;    // array for the 1-d power spectrum
///  float* r4FreqDB    = new float [i4Nyquest] ;    // array for the frequencies in db
///  int* i4PowerNumber = new int [i4Nyquest] ;      // array for number of points in 1-d power spectrum
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///   float r4CascadeScaleBreak ;          // Location of scale break (pixels)
///   int i4FFTSize ;                      // Logical size of the fft array
///   float r4PixelSize;                   // Pixel size in km
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>

  int i4Loop ;               // Loop counter
  int i4Row;                 // Loop counter over the FFT rows
   int i4Col ;                // Loop counter over the FFT columns
  int i4LZeroWaveNumber ;    // The wave number at the start of the scaling regime
  int i4CurrentWaveNumber ;  // The wave number at a location in the FFT array
  float r4LOneWaveNumber ;   // The wave number at the scale break
  float r4LOneFreq ;         // The frequency at the scale break
  int i4ScaleBreak ;         // The wave number at the scale break
  int i4FFTStride ;          // Step through the FFT
  int i4Nyquest ;            // FFT Nyquest frequency
  float r4MaxFreq ;          // Maximum frequency
  int i4BetaOneStart ;       // Start wave number for the calculation of Beta1
  int i4BetaOneEnd ;         // End wave number for the calculation of Beta1
  int i4BetaTwoStart ;       // Start wave number for the calculation of Beta2
  int i4BetaTwoEnd;          // End wave number for the calculation of Beta2

  i4LZeroWaveNumber = i4FFTSize/256 ; // Avoid the very low frequency part of the spectrum

  // AWS 2 Dec 2012 Dont use the first two wave numbers because of a low bias
  if ( i4LZeroWaveNumber < 2 ) i4LZeroWaveNumber = 2 ;

  // Calculate the wave number and frequency of the scale break
  r4LOneWaveNumber  = (float)i4FFTSize/r4CascadeScaleBreak ; // r4CascadeScaleBreak in pixel units
  r4LOneFreq        = 1.0 /r4CascadeScaleBreak ;
  i4ScaleBreak      = (int)r4LOneWaveNumber ;

  i4FFTStride = 1 + i4FFTSize/2 ;    // The stride in the complex array
  i4Nyquest   = i4FFTSize/2 ;        // Limit of the FFT
  r4MaxFreq = (float)(i4Nyquest-1)/(float)(i4FFTSize)   ;

  if ( r4LOneWaveNumber > i4Nyquest ) r4LOneWaveNumber = i4Nyquest ;
  if ( i4ScaleBreak > i4Nyquest ) i4ScaleBreak = i4Nyquest ;

 // AWS 25 Jan 2007 Bug Fix
 // if( r4LOneFreq < r4MaxFreq ) r4LOneFreq = r4MaxFreq ;
  if( r4LOneFreq > r4MaxFreq ) r4LOneFreq = r4MaxFreq ;

  float* r4PowerSpec = new float [i4Nyquest+2] ;    // array for the 1-d power spectrum
  float* r4FreqDB    = new float [i4Nyquest+2] ;    // array for the frequencies in db
  int* i4PowerNumber = new int   [i4Nyquest+2] ;      // array for number of points in 1-d power spectrum

  for ( i4Loop = 0; i4Loop < i4Nyquest+2; i4Loop++ ) r4PowerSpec[i4Loop] = 0 ;
  for ( i4Loop = 0; i4Loop < i4Nyquest+2; i4Loop++ ) i4PowerNumber[i4Loop] = 0 ;

// Calculate the power spectrum for this fft
  for ( i4Row=0; i4Row<=i4Nyquest+1; i4Row++) {
    for (i4Col=0; i4Col<i4FFTStride; i4Col++) {
      i4CurrentWaveNumber = (int)sqrt( (double)(i4Row*i4Row + i4Col*i4Col) ) ;
      i4Loop = i4Row*i4FFTStride + i4Col;
      if (i4CurrentWaveNumber <= i4Nyquest) {
        r4PowerSpec[i4CurrentWaveNumber] += (c4Temp[i4Loop][0]*c4Temp[i4Loop][0]+c4Temp[i4Loop][1]*c4Temp[i4Loop][1]) ;
        i4PowerNumber[i4CurrentWaveNumber]++ ;
      }
    }
  }

  // Calculate the db mean power and frequency
  for ( i4Loop = 1; i4Loop < i4Nyquest; i4Loop++ ) {
    r4PowerSpec[i4Loop] = 10*log10( (double)(r4PowerSpec[i4Loop]/i4PowerNumber[i4Loop]) ) ;
    r4FreqDB[i4Loop] = 10*log10( (double)i4Loop/(double)i4FFTSize/r4PixelSize )  ;
  }

  i4BetaOneStart = i4LZeroWaveNumber ;
  i4BetaOneEnd   = i4ScaleBreak ;
  i4BetaTwoStart = i4ScaleBreak ;

// AWS 2 Dec 2012 Dont use the very high frequencies due to a high bias
//  i4BetaTwoEnd   = i4Nyquest-1 ;
  i4BetaTwoEnd   = i4FFTSize/3 ;

  if ( i4BetaOneEnd-i4BetaOneStart+1 > 4 )
    r4Beta1 = get_beta( r4FreqDB+i4BetaOneStart, r4PowerSpec+i4BetaOneStart, i4BetaOneEnd-i4BetaOneStart+1) ;
  else
    r4Beta1 = -2.3 ;

  if ( i4BetaTwoEnd-i4BetaTwoStart+1 > 4 )
    r4Beta2 = get_beta( r4FreqDB+i4BetaTwoStart, r4PowerSpec+i4BetaTwoStart, i4BetaTwoEnd-i4BetaTwoStart+1) ;
  else
    r4Beta2 = -2.6 ;

  r4Beta1 *= -1.0 ;
  r4Beta2 *= -1.0 ;

// AWS 26 August 2011 Need to know the true value of the beta
//  r4Beta2 += 0.2 ; // Account for a bias due to artefacts at the high frequency end
//  r4Beta1 += 0.3 ; // Account for a bias due to artefacts at the low frequency end

#ifdef DEBUG
  if ( fLogFile != NULL ) {
    fprintf(fLogFile,"\nInformation: BCascade:calc_power_exponent Power Spectrum\n" ) ;
    fprintf(fLogFile,"FFTSize = %d, LZeroWaveNumber = %d, ScaleBreak = %d, Nyquest = %d\n", i4FFTSize, i4LZeroWaveNumber, i4ScaleBreak, i4Nyquest) ;
    fprintf(fLogFile,"Start wave number for beta1 = %d, Number of points = %d\n",i4BetaOneStart,i4BetaOneEnd-i4BetaOneStart+1) ;
    fprintf(fLogFile,"Start wave number for beta2 = %d, Number of points = %d\n",i4BetaTwoStart,i4BetaTwoEnd-i4BetaTwoStart+1) ;
    fprintf(fLogFile,"Beta1 = %f, Beta2 = %f\n", r4Beta1, r4Beta2) ;
#ifdef TEST_BCASCADE
    for ( i4Loop = 0; i4Loop < i4Nyquest; i4Loop++ ) {
      fprintf(fLogFile,"%d,%f,%f\n", i4Loop, r4FreqDB[i4Loop], r4PowerSpec[i4Loop]) ;
    }
#endif
  }
#endif

  delete [] r4PowerSpec ;
  delete [] r4FreqDB ;
  delete [] i4PowerNumber ;
}

float BCascade::get_beta(
  float* x,   ///< IN: Array of x values
  float* y,   ///< IN: Array of y values
  int n       ///< IN: Number of points
) {
/// \fn float BCascade::get_beta( float* x, float* y, int n )
/// \brief Returns the least squares fit to a set of points
  float sum_x=0;
  float sum_x2=0;
  float sum_y=0;
  float sum_xy=0 ;

  for ( int ia = 0; ia < n; ia++ ){
    sum_x  += x[ia] ;
    sum_x2 += x[ia]*x[ia] ;
    sum_y  += y[ia] ;
    sum_xy += x[ia]*y[ia] ;
  }

  float bottom =  n*sum_x2 - sum_x*sum_x ;

  float slope ;
  if ( bottom > 0 )
    slope = (float)(n*sum_xy - sum_x*sum_y)/bottom ;
  else
    slope = 0 ;
  return slope ;

}

void BCascade::to_cascade(
  float *r4InMap,   ///< IN: Input radar array
  float *r4OutMap   ///< OUT: Output cascade array
) {
/// \fn void BCascade::to_cascade(float *r4InMap,float *r4OutMap)
/// \brief Insert the input array into the cascade array
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// Insert the input array into the cascade array
///
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed     19/03/05       Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// None
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
///  domain2d mDom ;                    // Manage the input map coordinate system after (possible) upscaling
///  domain2d cDom ;                    // Manage the cascade array coordinate system
///  int* i4DataMask ;                  // Mask for valid data in the cascade array
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>

  int i4Loop ;        // Loop counter over input array
  int i4Row ;         // Row in input array
  int i4Col ;         // Column in input array
  int i4ColOffset ;   // Start column of radar map in the cascade array
  int i4RowOffset ;   // Start row of radar map in the cascade array
  int i4OutOffset ;   // Location of the pixel in the cascade array

  for ( i4Loop = 0; i4Loop < cDom.npoints(); i4Loop++ ) r4OutMap[i4Loop]   = r4NoData ;
  for ( i4Loop = 0; i4Loop < cDom.npoints(); i4Loop++ ) i4DataMask[i4Loop] = 0 ; // missing = 0

  i4ColOffset = (cDom.ncols() - mDom.ncols())/2 ;
  i4RowOffset = (cDom.nrows() - mDom.nrows())/2 ;

  for ( i4Loop = 0; i4Loop < mDom.npoints(); i4Loop++ ) {
    i4Row = mDom.row(i4Loop)+i4RowOffset ;
    i4Col = mDom.col(i4Loop)+i4ColOffset ;
    i4OutOffset = cDom.point(i4Row, i4Col) ;
    r4OutMap[i4OutOffset] = r4InMap[i4Loop] ;
    if ( r4InMap[i4Loop] > r4NoData+1 )
      i4DataMask[i4OutOffset] = 1 ;
  }
}

void BCascade::from_cascade(
  float *r4InMap,   ///< IN: Input cascade array
  float *r4OutMap   ///< OUT: Output radar array
) {
/// \fn void BCascade::to_cascade(float *r4InMap,float *r4OutMap)
/// \brief Insert the cascade array into the radar array
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// Insert the input array into the cascade array
///
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed     19/03/05       Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// None
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
///  domain2d mDom ;                    // Manage the input map coordinate system after (possible) upscaling
///  domain2d cDom ;                    // Manage the cascade array coordinate system
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>

  int i4Loop ;        // Loop counter over radar array
  int i4Row ;         // Row in cascade array
  int i4Col ;         // Column in cascade array
  int i4ColOffset ;   // Start column of radar map in the cascade array
  int i4RowOffset ;   // Start row of radar map in the cascade array
  int i4InOffset ;    // Location of the pixel in the cascade array

  i4ColOffset = (cDom.ncols() - mDom.ncols())/2 ;
  i4RowOffset = (cDom.nrows() - mDom.nrows())/2 ;

  for ( i4Loop = 0; i4Loop < mDom.npoints(); i4Loop++ ) r4OutMap[i4Loop] = 0 ;

  for ( i4Loop = 0; i4Loop < mDom.npoints(); i4Loop++ ) {
    i4Row = mDom.row(i4Loop)+i4RowOffset ;
    i4Col = mDom.col(i4Loop)+i4ColOffset ;
    i4InOffset = cDom.point(i4Row, i4Col) ;
    r4OutMap[i4Loop] = r4InMap[i4InOffset] ;
  }
}

void BCascade::get_velocities(
  float *r4eVelPassed,  ///< OUT: The east velocities mapped into cascadeSize x cascadeSize array
  float *r4sVelPassed   ///< OUT: The south velocities mapped into cascadeSize x cascadeSize array
) {
/// \fn void BCascade::get_velocities(float *r4eVel,float *r4sVel)
/// \brief Get the velocities back mapped into the cascadeSize*cascadeSize arrays
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// Get the velocities in cascade arrays
///
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed     19/03/05       Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
///  domain2d mDom ;      // Manage the input map coordinate system after (possible) upscaling
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>
  sRadarAdvection->Get_Velocities(r4eVelPassed, r4sVelPassed) ;
}

void BCascade::set_velocities(
  float *r4eVelPassed,  ///< OUT: The east velocities mapped into cascadeSize x cascadeSize array
  float *r4sVelPassed   ///< OUT: The south velocities mapped into cascadeSize x cascadeSize array
) {
/// \fn void BCascade::set_velocities(float *r4eVel,float *r4sVel)
/// \brief Set the velocities in the optical flow class
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// Get the velocities in cascade arrays
///
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed     19/03/05       Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///  float *r4sVelMap = new float [mDom.npoints] ; // The advection vectors for the map domain
///  float *r4eVelMap = new float [mDom.npoints] ; // The advection vectors for the map domain
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
///  domain2d mDom ;      // Manage the input map coordinate system after (possible) upscaling
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>
  sRadarAdvection->Set_Velocities(r4eVelPassed, r4sVelPassed) ;

}

void BCascade::advect_cascade(
  int i4NoIterations, ///< IN Number of time steps to advect through
  int i4NoData,       ///< IN Missing data flag as an int
  const float *r4InputCascade, ///< IN Cascade sized array to advect
  float *r4OutputCascade ///< OUT Cascade size array to return
) {
/// \fn void BCascade::advect_cascade(int i4NoIterations, int i4NoData, float *r4InputCascade, float *r4OutputCascade)
/// \brief Advect cascade forwards by a number of iterations
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// Advect cascade array
///
///
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed     19/03/05       Initial Version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///  float *r4InputMap  = new float [cDom.npoints()] ; // The advection vectors for the cascade domain
///  float *r4OutputMap = new float [cDom.npoints()] ; // The advection vectors for the cascade domain
///
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
///
/// </pre>
  sRadarAdvection->Advect_Image(i4NoIterations, i4NoData, r4InputCascade, r4OutputCascade) ;

}

void BCascade::calculate_bandpass_cascade_filter() {
/// \fn void BCascade::calculate_bandpass_filter()
/// \brief Method to construct the band pass filters needed for each level in the cascade
  int i4Loop ;
  int i4WaveNumber ; // wave number in the fft
  float r4Freq ; // frequency at a wave number in the fft
  float r4CentreFreq ; // Centre frequency for the filter
  float r4CentreWaveLength ; // Centre wavelength for the filter
  int i4Level ;
  int i4Nyquest = i4FFTSize/2 ;

  float *r4BandpassFilter1D = new float[i4NumberLevels * (i4Nyquest + 1)];
  float *r4FilterSum = new float [i4Nyquest+1] ; // Sum of the filters
  float *r4NormFactor  = new float [i4Nyquest+1] ; // Normalisation to sum  = 1

  float r4Filter ; // Filter
  float r4Width ; // Width factor for the filter
  float r4RelFrequency ; // Scaled frequency for exponential decay
  r4CentreWaveLength = i4CascadeSize ;

  // Zero the array for the sum of the filters
  for ( i4Loop = 1; i4Loop <= i4Nyquest; i4Loop++ ) r4FilterSum[i4Loop] = 0.0 ;
  for ( i4Loop = 0; i4Loop < i4NumberLevels*(i4Nyquest+1); i4Loop++ ) r4BandpassFilter1D[i4Loop] = 0.0 ;

  r4Width = 2.0 ; // Filter width
  // Start the loop over the levels in the cascade
  for ( i4Level = 0; i4Level < i4NumberLevels; i4Level++ ){
    r4CentreFreq = 1.0/ r4CentreWaveLength ;
    for ( i4WaveNumber = 1; i4WaveNumber <= i4Nyquest; i4WaveNumber++ ) {
      r4Freq = (float)i4WaveNumber/(float)i4FFTSize ;
      if ( r4Freq > r4CentreFreq )
       r4RelFrequency = r4Freq/r4CentreFreq ;
      else
       r4RelFrequency = r4CentreFreq/r4Freq  ;

      r4Filter = exp(-r4Width*r4RelFrequency) ;
      r4BandpassFilter1D[i4WaveNumber+i4Level*(i4Nyquest+1)] = r4Filter ;
      r4FilterSum[i4WaveNumber] += r4Filter ;
    }
    r4CentreWaveLength *= r4ScaleRatio ;
  }

  // loop over the wave numbers and calculate the normalisation factor
  for ( i4WaveNumber = 1; i4WaveNumber <= i4Nyquest; i4WaveNumber++ ) {
    r4NormFactor[i4WaveNumber] = 1.0 /r4FilterSum[i4WaveNumber] ;
  }

  // Normalise the filters so that each wave number sums to one
  for ( i4Level = 0; i4Level < i4NumberLevels; i4Level++ ) {
    for ( i4WaveNumber = 1; i4WaveNumber <= i4Nyquest; i4WaveNumber++) {
      r4Filter =  r4BandpassFilter1D[i4WaveNumber+i4Level*(i4Nyquest+1)]*r4NormFactor[i4WaveNumber];
      if ( r4Filter < 0.001 ) r4Filter = 0.0 ;
      r4BandpassFilter1D[i4WaveNumber+i4Level*(i4Nyquest+1)] = r4Filter ;
    }
  }

// Write out the filters if in debug mode
#ifdef DEBUG
  fprintf(fLogFile,"Information Stochastic_Cascade::calculate_bandpass_filter() Bandpass filters\n") ;
  for ( i4WaveNumber = 1; i4WaveNumber <= i4Nyquest; i4WaveNumber++) {
    fprintf(fLogFile,"%d,", i4WaveNumber) ;
    for ( i4Level = 0; i4Level < i4NumberLevels; i4Level++ ) {
      fprintf(fLogFile,"%6.3f,",r4BandpassFilter1D[i4WaveNumber+i4Level*(i4Nyquest+1)]) ;
    }
    fprintf(fLogFile,"\n") ;
  }
#endif

  // Okay, we've generated a 1D filter, now predetermine the 2D filter mask for each level
  int i4FFTStride = i4Nyquest + 1;
  for (int nLevel = 0; nLevel < i4NumberLevels; ++nLevel)
  {
    float * r4Factor = r4BandpassFilter[nLevel];
    int i4FilterOffset = nLevel * i4FFTStride;

    for (int i4Row = 0; i4Row <= i4Nyquest; ++i4Row)
    {
      int nRowIdx = i4Row * i4FFTStride;
      int nRowSqr = i4Row * i4Row;
      for (int i4Col = 0; i4Col < i4FFTStride; ++i4Col)
      {
        float r4CurrentWaveNumber = sqrtf((float)(nRowSqr + i4Col*i4Col)) ;
        r4Factor[nRowIdx + i4Col] = (r4CurrentWaveNumber > i4Nyquest)
          ? 0.0
          : r4BandpassFilter1D[i4FilterOffset + (int) r4CurrentWaveNumber];
      }
    }
    r4Factor[0] = 1.0;
  }

  delete [] r4FilterSum ;
  delete [] r4NormFactor ;
  delete [] r4BandpassFilter1D ;
}

void BCascade::bandpass_filter_mask(
  fftw_complex* c4FFTPassedIn,      ///< IN: The Fourier transform to be filtered
  int i4CascadeLevelNumber,         ///< IN: The cascade level to be filtered
  fftw_complex* c4FFTPassedOut      ///< OUT: The filtered Fourier transform
){
///\fn void BCascade::bandpass_filter_mask(fftw_complex* c4FFTPassedIn,int i4CascadeLevelNumber,fftw_complex* c4FFTPassedOut) ;
/// \brief Bandpass filter based on a pre-computed filter
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// Exponential filter used to filter the FFT. The filter used
/// is calculated through a call to calculate_bandpass_filter
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.o          Alan Seed       14/06/05        Version 1
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
///
/// NOTES:
///
///
/// REFERENCES:
/// FFTW documentation
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
/// None
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// i4FFTArraySize     The physical size of the array
/// r4BandpassFilter   The pre-computed filter for decomposing field
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
/// </pre>

// Local variables
  int i4Nyquest = i4FFTSize/2 ;                     // Nyquest frequency
  int i4FFTStride = 1 + i4FFTSize/2 ;               // The stride in the complex array

  // Pointer to the 2D bandpass filter mask
  float *r4Filter = r4BandpassFilter[i4CascadeLevelNumber];

  // Initialize arrays
  memset(c4FFTPassedOut, 0, sizeof(fftw_complex) * i4FFTArraySize);

  // Apply the filter to the FFT
  // first half:
  for (int i4Row = 0; i4Row <= i4Nyquest; ++i4Row)
  {
    int nRowIdx = i4Row * i4FFTStride;
    for (int i4Col = 0; i4Col < i4FFTStride; ++i4Col)
    {
      int nIdx = nRowIdx + i4Col;
      c4FFTPassedOut[nIdx][0] = r4Filter[nIdx] * c4FFTPassedIn[nIdx][0];
      c4FFTPassedOut[nIdx][1] = r4Filter[nIdx] * c4FFTPassedIn[nIdx][1];
    }
  }

  // second half:
  for (int i4Row = i4Nyquest + 1; i4Row < i4FFTSize; ++i4Row)
  {
    int nRowIdx1 = i4Row * i4FFTStride;
    int nRowIdx2 = (i4FFTSize - i4Row) * i4FFTStride;
    for (int i4Col = 0; i4Col < i4FFTStride; ++i4Col)
    {
      int nIdx1 = nRowIdx1 + i4Col;
      int nIdx2 = nRowIdx2 + i4Col;
      c4FFTPassedOut[nIdx1][0] = r4Filter[nIdx2] * c4FFTPassedIn[nIdx1][0];
      c4FFTPassedOut[nIdx1][1] = r4Filter[nIdx2] * c4FFTPassedIn[nIdx1][1];
    }
  }
}

void BCascade::decompose_mask(
  float*  r4InputMap,            ///< IN: The data to be decomposed, float[i4NumberCols*i4NumberRows]
  float*  r4CascadeLevelMeanOut, ///< OUT: The mean value of the cascade, at each level and every point
  float*  r4CascadeLevelStdOut,  ///< OUT: The standard deviation of the cascade, at each level and every point
  float*  r4Cascade              ///< OUT: The decomposed cascade, float[i4CascadeArraySize*i4NumberLevels]
) {
/// \fn void BCascade::decompose_fftw(float *r4InputMap,float *r4CascadeLevelMeanOut,float *r4CascadeLevelStdOut,float *r4Cascade )
/// \brief Decompose the rainfall field into spectral components
/// <pre>
///***00000000000000000000000000000000000000000000000000000000000000000000
///
/// MODULE NAME: bcascade.cpp
///
/// TYPE: void
///
/// LANGUAGE: c++
///
/// COMPILER OPTIONS:
///
///***11111111111111111111111111111111111111111111111111111111111111111111
///
/// PURPOSE:
///
/// To decompose the rainfall field into its spectral components
///
///***22222222222222222222222222222222222222222222222222222222222222222222
///
/// CHANGE HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE DETAILS
///
/// 1.00         Alan Seed       21/02/05     Initial version
///
///***33333333333333333333333333333333333333333333333333333333333333333333
///
/// REVIEW HISTORY
///
/// VERSION NO.  AUTHOR          DATE         CHANGE REVEIWER    ACCEPTED
///
/// 1.00         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.01         A Programmer    dd/mm/yy     A Reviewer         dd/mm/yy
/// 1.02         etc....
///
///***44444444444444444444444444444444444444444444444444444444444444444444
///
/// STRUCTURE:
///
/// 1. Remove the no-data flag and subtract the field mean
/// 2. Take the Fourier transform using fftw
/// 3. For each level, filter the transform and place the data into the
///    cascade using bandpass_filter_fftw
/// 4. Calculate the mean and standard deviation of each level, not using
///    variable blocks this time
///  5. Reduce the standard deviation of the lowest level and censor to +- 3
///
/// NOTES:
///
///
/// REFERENCES:
///
///
///***55555555555555555555555555555555555555555555555555555555555555555555
///
/// RETURN CODES
///
/// None
///
///***66666666666666666666666666666666666666666666666666666666666666666666
///
/// FILES USED:
///
/// None
///
///***77777777777777777777777777777777777777777777777777777777777777777777
///
/// DYNAMIC ARRAYS ALLOCATED
///
/// ARRAY NAME: c4FFT alocated using fftw_malloc
/// ARRAY TYPE: fftw_complex*
///
/// ARRAY NAME: i4Mask
/// ARRAY TYPE: float*
/// DIMENSIONS: i4NumberRows*i4NumberCols
///
///***88888888888888888888888888888888888888888888888888888888888888888888
///
/// CLASS-WIDE VARIABLES USED:
///
/// float r4NoData;          // flag for non-valid data
/// int i4NumberRows;        // size of the input rain map
/// int i4NumberCols;        // size of the input rain map
/// int i4NumberLevels;      // image size and number of cascade levels
/// fftw_complex* c4FFTout;  // output array for fftw
/// double r8FFTin ;         // input array for fftw
/// int i4FFTSize;           // size of the fft arrays for spatial smoothing
/// int i4FFTArraySize ;     // size of the zero packed array for the fft
/// int i4InMapArraySize ;   // size of the input maps
/// int i4CascadeArraySize ; // size of one level of the cascade
/// float* r4TempArray ;     // Temporary array of image data
/// FILE* fCascadeFile[8];   // Pointer to the cascade weights files
///
/// STANDARD LIBRARIES USED:
///
/// math.h
///
///***99999999999999999999999999999999999999999999999999999999999999999999
/// </pre>
  int i4Level;                                       // Loop counter, level in the cascade
  int i4Row;                                         // Loop counter, horizontal position in analyses
  int i4Col;                                         // Loop counter, vertical position in analyses
  int i4NumberValidData ;                            // Number of valid pixels in the input map

  size_t i4Loop ;                                    // Loop counter, over the array
  size_t i4FFTOffset;                                // Offset into the FFT array
  size_t i4CascadeOffset ;                           // Offset into the cascade

  float startMask  ;                                 // Largest scale to be kept by filter
  float centreMask ;                                 // Middle scale to be kept by filter
  float endMask    ;                                 // Smallest scale to be kept by filter
  float r4FFTFactor;                                 // Need to multiply by this factor to regain data
//
//-----------------------------------------------------------------------
//
// 1. Remove the no data flag and subtract the field mean
//
  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ){
        r4TempArray[i4Loop] = i4DataMask[i4Loop]*(r4InputMap[i4Loop] - r4FieldMean)  ;
  }

  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ){
      if ( r4TempArray[i4Loop] < r4NoData+1 ) r4TempArray[i4Loop] = 0 ;
  }

  domain2d fDom ;
  fDom.init(i4FFTSize, i4FFTSize) ;

  r4FFTFactor    = 1.0 / (float)(i4FFTSize*i4FFTSize) ;	// inverse fft multiplies by this factor
  startMask      = i4FFTSize/r4ScaleRatio   ;
  centreMask     = i4FFTSize ;
  endMask        = i4FFTSize*r4ScaleRatio ;
//
//-----------------------------------------------------------------------
//
// 2. Calculate the FFT
//
  i4Row = 0 ;
  i4NumberValidData = 0 ;
  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ) i4NumberValidData += i4DataMask[i4Loop] ;
  for ( i4Loop = 0; (int)i4Loop < i4FFTSize*i4FFTSize; i4Loop++ ) r8FFTin[i4Loop] = 0 ;

// load the image into the top left corner of the input array
  for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ) {
    i4Row = cDom.row(i4Loop) ;
    i4Col = cDom.col(i4Loop) ;
    i4FFTOffset = fDom.point(i4Row,i4Col) ;
    r8FFTin[i4FFTOffset] = r4TempArray[i4Loop]  ;
  }

// Take the Fourier transform
  fftw_execute(inPlan) ;

// and load the output into an array so that we can work on it
  for ( i4Loop = 0; i4Loop < i4FFTArraySize; i4Loop++ ){
   c4FFT[i4Loop][0] = c4FFTout[i4Loop][0] ;
   c4FFT[i4Loop][1] = c4FFTout[i4Loop][1] ;
  }

// Calculate the power spectrum while we have the FFT handy
// START AWS 22 April 2009 Use default slopes if not enough rain
//  calc_power_exponent(c4FFT, r4BetaOne, r4BetaTwo) ;
  if (r4RainFrac > 0.03) {
    calc_power_exponent(c4FFT, r4BetaOne, r4BetaTwo) ;
  }
  else {
    r4BetaOne = 2.2 ;
    r4BetaTwo = 2.5 ;
  }
// END AWS 22 April 2009

  if ( fLogFile != NULL ) fprintf( fLogFile,
        "\nInformation: BCascade; r4BetaOne = %f r4BetaTwo = %f\n", r4BetaOne, r4BetaTwo);

//
//-------------------------------------------------------------------------
//
// 3. For each level, filter the transform and place the data into the
// cascade
//
  i4CascadeOffset = 0;
  for ( i4Level=0; i4Level<i4NumberLevels; i4Level++ ) {
    if ( i4Level < i4NumberLevels-1 ) {

// Calculate the inverse fft, and load the result into r4Cascade
      bandpass_filter_mask(c4FFT, i4Level, c4FFTout) ;
      fftw_execute(outPlan) ;
      i4CascadeOffset = i4Level*i4CascadeArraySize ;
      for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++) {
        i4Row = cDom.row(i4Loop) ;
        i4Col = cDom.col(i4Loop) ;
        i4FFTOffset = fDom.point(i4Row, i4Col) ;
        r4Cascade[i4CascadeOffset+i4Loop] = r8FFTin[i4FFTOffset]*r4FFTFactor ;
      }
    }
    else {
      // AWS 15 May 2010 The lower level is the residual
      float r4CascadeSum ;
      float r4Residual ;
      int i4LevelA ;
      for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++ ) {
        if ( i4DataMask[i4Loop] == 1 ) {
          r4CascadeSum = r4FieldMean ;
          for ( i4LevelA = 0; i4LevelA < i4NumberLevels-1; i4LevelA++ ) {
            i4CascadeOffset = i4LevelA * i4CascadeArraySize ;
            r4CascadeSum += r4CascadeLevelStdOut[i4LevelA]*r4Cascade[i4CascadeOffset+i4Loop] ;
          }
          r4Residual =  r4InputMap[i4Loop] - r4CascadeSum ;
          i4CascadeOffset = (i4NumberLevels-1)*i4CascadeArraySize ;
          r4Cascade[i4CascadeOffset+i4Loop] = r4Residual ;
        }
        else {
          r4Cascade[i4CascadeOffset+i4Loop] = 0 ;
        }
      }
    }

    r4CascadeLevelMeanOut[i4Level] = 0 ;
    for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++) {
      r4CascadeLevelMeanOut[i4Level] += i4DataMask[i4Loop]*r4Cascade[i4CascadeOffset+i4Loop] ;
	  }
    r4CascadeLevelMeanOut[i4Level] /= (float)i4NumberValidData ;
//
//-----------------------------------------------------------------------
//
// 4. Subtract the mean and calculate the standard deviation of this level
//
    r4CascadeLevelStdOut[i4Level] = 0 ;
    for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++) {
      r4Cascade[i4CascadeOffset+i4Loop] -= r4CascadeLevelMeanOut[i4Level] ;
      r4CascadeLevelStdOut[i4Level] += i4DataMask[i4Loop]*r4Cascade[i4CascadeOffset+i4Loop]*r4Cascade[i4CascadeOffset+i4Loop] ;
	  }
    r4CascadeLevelStdOut[i4Level] =  sqrt(r4CascadeLevelStdOut[i4Level]/(float)i4NumberValidData) ;

    // AWS 20 Jan 2011 - only normalise if std > 0.1
    if ( r4CascadeLevelStdOut[i4Level] > 0.1 ) {
      for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++) {
        r4Cascade[i4CascadeOffset+i4Loop] /= r4CascadeLevelStdOut[i4Level] ;
      }
    } else {
      for ( i4Loop = 0; i4Loop < i4CascadeArraySize; i4Loop++) {
        r4Cascade[i4CascadeOffset+i4Loop] = 0.0 ;
      }
    }

// Reduce the scale of rain objects we are looking at, and proceed to next level
		startMask  *= r4ScaleRatio ;
		centreMask *= r4ScaleRatio ;
		endMask    *= r4ScaleRatio ;
	}
}

void BCascade::set_default_autocor() {
	float r4CorB = 1.8 ;
	float r4CorA = 0.14 ;
	float r4CorC = 1.7 ;
	set_default_autocor(r4CorA,r4CorB,r4CorC) ;
}

void BCascade::set_default_autocor(float r4CorA, float r4CorB, float r4CorC) {
	int i4Level ;
	float r4CorrelTime ;

// Calculate the autocorrelations at the top of this cascade
	float r4Scale = i4CascadeSize*r4PixelSize ;

// Set the default auto-correlations for the levels in the cascade using the scaling exponent
	for ( i4Level = 0; i4Level < i4NumberLevels; i4Level++ ) {
		r4CorrelTime =  r4CorA*pow(r4Scale,r4CorB) ;
		r4Correlation[i4Level]  =  exp( -(double)(i4TimeStep/r4CorrelTime) ) ;
    if (r4Correlation[i4Level] < 0.5) {
       r4Correlation[i4Level] = 0.5*(r4Correlation[i4Level]+0.5) ;
    }

		if ( i4NumberLags == 2 )
			r4Correlation[i4Level+i4NumberLevels] = pow(r4Correlation[i4Level],r4CorC)  ;

		r4Scale *= r4ScaleRatio ;
	}

// Update the phi parameters using these correlations
	update_phi(r4Correlation, r4Phi) ;

// Set the slopes of the power spectra to default values
	r4BetaOne = 2.2 ;
	r4BetaTwo = 2.7 ;

}


// Get the input map of rainfall intensities
void BCascade::get_rainMap( float *r4rainMapPassed ){
	int i4Loop ;
	for ( i4Loop = 0; i4Loop < i4NumberCols*i4NumberRows; i4Loop++)
		r4rainMapPassed[i4Loop] = r4RainMap[i4Loop] ;
}
