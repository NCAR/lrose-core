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
/* 	$Id: bcascade.hh,v 1.3 2016/03/04 01:28:13 dixon Exp $	 */
///
/// \file bcascade.h
///

// Base cascade class for use at Met Office and BOM
//---------------------------------------------------------------------------
#ifndef BCascade_HH
#define BCascade_HH

#include <cmath>
#include <time.h>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <cstring>
#include <iostream>

#include <sys/stat.h>
#define O_BINARY 0
#include <unistd.h>
#include <fftw3.h>

#include <dataport/port_types.h>

typedef int __int32 ;

using namespace std;

/* depends on : steps_utilities.o */
/* depends on : optical_flow.o */
/* depends on : fftw_wrapper.o */
/* depends on : smoothing.o */
/* depends on : pmatch.o */
/* depends on : gasrv_2.o */

// Normalize the rain data under the mask
/// \fn normalize_ms_rain (fl32 r4NoData,int i4ArrayLength,fl32 r4ReqMean,fl32 r4ReqStd,int   *i4DataMask,fl32 *r4Data)
/// \brief Normalize the field to a known mean and standard deviation in rain rate (mm/h)
void normalize_ms_rain (
  fl32 r4NoData,       ///<  Missing data flag
  int i4ArrayLength,    ///<  Length of input array
  fl32 r4ReqMean,      ///<  Required mean
  fl32 r4ReqStd,       ///<  Required std
  int   *i4DataMask,    ///<  Mask for the area with data in the input image
  fl32 *r4Data         ///<  Data array [i4ArrayLength] in size
) ;

class BCascade {
protected:
  static const fl32 CASCADE_MIN_KM;
  static const fl32 CASCADE_SCALE_BREAK;

  FILE* fLogFile;                      ///<  Pointer to the log file
  __int32 idum ;                       ///<  Random number seed for gasdev(),ran3()
  time_t sMapTimes[8] ;                ///<  List of times for each map
  bool hasOpenedLogFile ;              ///<  Flag to indicate that the class has opened a log file
  bool bIsDet ;                        ///<  Flag to indicate that the class is using the deterministic options

  char cLogFileName[512] ;             ///<  Name of error log file
  char cOpticalFileName[512] ;         ///<  Name of the optical flow file name
  char cBCascadeFileName[512] ;        ///<  Name for the BCascade file

  int i4CascadeSize;                   ///<  Size of the cascade assumed to be square
  int i4NumberRows;                    ///<  Size of the input rain map
  int i4NumberCols;                    ///<  Size of the input rain map
  int i4NumberLevels;                  ///<  Image size and number of cascade levels
  int i4NumberCascades;                ///<  Number of cascades to keep
  int i4NumberMaps;                    ///<  Number of maps to keep
  int i4NumberLags;                    ///<  Order of the ar model
  int i4CascadeMinLevel ;              ///<  Minimum scale for decomposition in pixels
  int i4TimeStep ;                     ///<  Time step of the radar and NWP in minutes
  int i4MapOffset ;                    ///<  Offset of the radar map in the cascade array
  int i4ImageNumber;                   ///<  Number of images since the cascade was initialised
  int i4FFTSize ;                      ///<  Logical size of the fft array

  size_t i4FFTArraySize ;              ///<  Actual size of the fft array = FFTSize*(1+FFTSize/2)
  size_t i4InMapArraySize ;            ///<  Array size for input data
  size_t i4CascadeArraySize ;          ///<  Size of the cascade array

  fl32 r4PixelSize;                   ///<  Pixel size in km
  fl32 r4NoData;                      ///<  Flag for non-valid data
  fl32 r4CondMean ;                   ///<  Conditional mean rain rate in mm/h
  fl32 r4RainFrac;                    ///<  Fraction of image > 0.2 mm/h
  fl32 r4RainMean;                    ///<  Mean in mm/h
  fl32 r4RainStd;                     ///<  Std in mm/h
  fl32 r4FieldMean;                   ///<  Mean value of the rain rate (in dBr )
  fl32 r4FieldStd;                    ///<  Standard deviation of the rain rate
  fl32 r4BetaOne ;                    ///<  Slope of the power spectrum above scale break
  fl32 r4BetaTwo ;                    ///<  Slope of the power spectrun below the scale break
  fl32 r4ScaleRatio ;                 ///<  Ratio of scales between level n+1 and level n in the cascade: < 1
  fl32 r4CascadeScaleBreak ;          ///<  Location of scale break (pixels)

  fl32* r4RainMap ;                   ///<  Array of the current rainfall intensity map
  fl32* r4RainMap1 ;                  ///<  Array of the previous rainfall intensity map

  fl32* r4Maps[8] ;                   ///<  Array of previous maps normalized to N(0,1)
  fl32 *r4TempArray ;                 ///<  Temporary array of image data

  fl32 **r4CascadeStack;              ///<  Array of pointers to the cascade arrays
  fl32 *r4Phi;                        ///<  AR(2) model parameters for updates
                                       ///<  Level 0 from [0] to [2], level 1 from [3] to [5] etc
  fl32 *r4Correlation;                ///<  Auto-correlations for the cascade lag 1 in [0] to [i4NumberLevels-1],
                                       ///<  Lag 2 in [i4NumberLevels] to [2*i4NumberLevels-1]
  fl32 r4CascadeLevelMean[16] ;       ///<  The mean of the cascade level
  fl32 r4CascadeLevelStd[16] ;        ///<  The standard deviation of the cascade level
  fl32 *r4Velocity ;                  ///<  Array of field velocities
  fl32 **r4BandpassFilter;            ///<  Array of 2D masks for the bandpass filters
  double *r8FFTin ;                    ///<  Array for input to fftw
  fftw_complex *c4FFTout ;             ///<  Array for output from fftw
  fftw_complex *c4FFT ;                ///<  Array needed by decompose function

  fftw_plan inPlan ;                   ///<  Forward fft
  fftw_plan outPlan ;                  ///<  Inverse fft
  // domain2d mDom ;                      ///<  Manage the input map coordinate system after (possible) upscaling
  // domain2d cDom ;                      ///<  Manage the cascade array coordinate system

  // gasRV  *rand_gen ;                   ///< Random number generator class

// Functions

// Update the optical flow tracker
  // void update_track(
  //   Optical_Flow* sOpticalFlowPassed);        // IN The optical flow object for which to calculate advection

// Calculate the autocorrelations
  // void update_autocor(
  //   Optical_Flow* sOpticalFlowPassed);        // IN The optical flow object which to use for the advection

// Calculate AR parameters
  void update_phi(
    fl32* r4LocalCorr,     // IN The value of the auto-correlation
    fl32* r4LocalPhi       // OUT The value of the AR(2) parameters
  ) ;


// Decompose field into spectral components
  void decompose_fftw(
    fl32* r4InputMap,               // IN The data to be decomposed, has no-data point removed
    fl32* r4TransformMean,          // OUT The mean value of the cascade, at each level and every point
    fl32* r4TransformStd,           // OUT The standard deviation of the cascade, at each level and every point
    fl32* r4Cascade                 // OUT The decomposed cascade
  );

  void  decompose_mask(
    fl32*  r4InputMap,            // IN: The data to be decomposed, fl32[i4NumberCols*i4NumberRows]
    fl32*  r4CascadeLevelMeanOut, // OUT: The mean value of the cascade, at each level and every point
    fl32*  r4CascadeLevelStdOut,  // OUT: The standard deviation of the cascade, at each level and every point
    fl32*  r4Cascade              // OUT: The decomposed cascade, fl32[i4CascadeArraySize*i4NumberLevels]
  ) ;

// Band pass filter FFT
  void bandpass_filter_fftw(
    fftw_complex* c4FFTIn,      // IN The Fourier transform to be filtered
    int    i4FFTSize,           // IN The size of (the 2D version of) the Fourier tranform
    fl32  r4StartWaveLength,   // IN The largest wavelength to be passed by the filter
    fl32  r4CentreWaveLength,  // IN The middle wavelength to be passed by the filter
    fl32  r4EndWaveLength,     // IN The smallest wavelength to be passed by the filter
    fftw_complex* c4FFTOut      // OUT The filtered Fourier transform
  ) ;

  void allocate_memory() ;  // Allocate class memory
  void deallocate_memory() ;  // Delete class memory

// Read a cascade off a file
  bool read_cascade_parameters( int i4InFile ) ;

// Read the data off a file
  bool read_cascade_data( int i4InFile ) ;


// Calculate the slope of the power spectrum above and below the scale break
  void calc_power_exponent( fftw_complex *c4Temp, fl32& r4Beta1, fl32& r4Beta2 ) ;

// Least squares fit to straight line
  fl32 get_beta( fl32* x, fl32* y, int n ) ;

// Calculate the masks for bandpass filters
  void calculate_bandpass_cascade_filter() ;

// Bandpass filter using pre-computed mask
  void bandpass_filter_mask(
    fftw_complex* c4FFTPassedIn,      // IN: The Fourier transform to be filtered
    int i4CascadeLevelNumber,         // IN: The cascade level to be filtered
    fftw_complex* c4FFTPassedOut      // OUT: The filtered Fourier transform
	) ;

public:
// AWS 19 May 2009
  // ProbMatch *pMatch ;                  ///< Probability matching class for Normal transformations

  int* i4DataMask ;                  ///<  Mask for valid data in the cascade array
  bool hasObsError ;                 ///<  Flag for the use of the observation error class
  // Optical_Flow* sRadarAdvection;     ///<  The velocities determined from the radar analyses

  BCascade(
    int i4NumberRowsPassed,          // IN The number of lines in a radar analysis
    int i4NumberColsPassed,          // IN The number of samples in each line of a radar analysis
    int i4NumberCascadesPassed,      // IN The number of cascades that are kept in memory
    int i4TimeStepPassed,            // IN The time step of the radar and NWP forecasts
    fl32 r4PixelSizePassed,         // IN The size (in km) of a pixel
    fl32 r4NoDataPassed,            // IN The "No-data" value (or outside value)
    char* cLogFileNamePassed,        // IN The name of the log file
    char* cBCascadeFileNamePassed,   // IN The name of the BCascade output file
    int* i4ReturnCode                // OUT Whether this routine has been sucessful
  );

  BCascade(
    int i4UpscaleOptionPassed,     // IN The upscale option
    int i4NumberRowsPassed,        // IN The number of lines in a radar analysis
    int i4NumberColsPassed,        // IN The number of samples in each line of a radar analysis
    int i4NumberCascadesPassed,    // IN The number of cascades that are kept in memory
    int i4TimeStepPassed,          // IN The time step of the radar and NWP in minutes
    fl32 r4PixelSizePassed,       // IN The size (in km) of a pixel
    fl32 r4NoDataPassed,          // IN The "No-data" value (or outside value)
    char* cLogFileNamePassed,      // IN The name of the log file
    char* cBCascadeFileNamePassed, // IN The name of the BCascade output file
    int* i4ReturnCode              // OUT Whether this routine has been sucessful
  );

  BCascade(
    char* cBCascadeFileNamePassed, // IN The name of the BCascade input file
	  int* i4ReturnCode              // OUT Whether this routine has been sucessful
  ) ;

  ~BCascade( );

// Convert rain into dbr and load into array, call this before update_parameters
  void update_maps(
          fl32* r4InputMap,         // IN The latest map data
          time_t sThisTime,          // IN The time of the latest data
          bool lReset                // IN Whether the cascade parameters need to be reset
                   );
  
// Load into array, call this before update_parameters
  void update_maps_dbz(
          fl32* r4InputMap,         // IN The latest map data
          time_t sThisTime,          // IN The time of the latest data
          bool lReset                // IN Whether the cascade parameters need to be reset
                       );

// Used by Merged_Cascade to advect the top cascade when making deterministic forecast
  void advect_weights(
          int i4NoSteps,             // IN Number of advection steps
          int i4CascadeNo            // IN Cascade to be advected
                      ) ;
  
// Generate a set of forecasts of rainfall
  void smooth_forecast_rain(
    int i4NumberForecasts,     // IN Number of forecast to make
    fl32* r4OutputForecast    // OUT Forecast image
  );

// Calculate AR parameters
  void update_phi(
    fl32* r4LocalCorr      // IN The value of the auto-correlation
  ) ;

// Get a copy of the phi parameters
  void get_phi(
    fl32* r4LocalPhi          // Out The phi values
  ) ;

// Get a copy of the phi parameters
  void get_correlation(
    fl32* r4LocalCorr         // Out The corelations
  ) ;

// Get the cacasde parameters
  void get_parameters(
    fl32 *r4StdLevelPassed,
    fl32 *r4PhiPassed,
    fl32 &r4FieldMeanPassed,
    fl32 &FieldStdPassed,
    fl32 &r4BetaOnePassed,
    fl32 &r4BetaTwoPassed
  ) ;

// Set the cascade parameters
  void set_parameters(
    fl32 *r4StdLevelPassed,  // OUT: Array of cascade level standard deviations, fl32[i4NumberLevels]
    fl32 *r4PhiPassed,       // OUT: Array of AR(2) parameters, fl32[i4NumberLevels*(i4NumberLags+1)]
    fl32 r4FieldMeanPassed, // OUT: Mean, 10*log10(mm/h)
    fl32 r4FieldStdPassed,  // OUT: Standard Deviation, 10*log10(mm/h)
    fl32 r4BetaOnePassed,   // OUT: Slope of the power spectrum for scales > scale break
    fl32 r4BetaTwoPassed    // OUT: Slope of the power spectrum for scales < scale break
  ) ;

// Copy an array from the input radar domain to the cascade domain
  void to_cascade(
    fl32 *r4InMap,  // Radar array to be re-mapped to the cascade
    fl32 *r4OutMap  // Output cascade array
  )   ;

// Copy an array from the cascade domain to the radar domain
  void from_cascade(
    fl32 *r4InMap,  // Cascade array to be re-mapped to the radar
    fl32 *r4OutMap  // Output radar array
  )   ;

// Advect the cascade array
  void advect_cascade(
    int i4NoIterations, // IN Number of time steps to advect through
    int i4NoData,       // IN Missing data flag as an int
    const fl32 *r4InputCascade, // IN Cascade sized array to advect
    fl32 *r4OutputCascade // OUT Cascade size array to return
  ) ;

// AWS 19 May 2009 Convert rain into normal distribution
  void convert_mmh_dbr(
    bool lDirection,   ///< IN True for mm/h to dbz, false for dbz to mm/h
    fl32 r4NoData,    ///< IN The no-data flag
    int i4NumberRows,  ///< IN The number of rows in the image
    int i4NumberCols,  ///< IN The number of columns in the image
    fl32* r4Data     ///< IN+OUT The data to be converted
	) ;

// AWS 13 Nov 2012 Convert rain to dbz using z = 200r^1.6
	void convert_mmh_dbz(
    bool lDirection,   ///< IN True for mm/h to dbz, false for dbz to mm/h
    fl32 r4NoData,    ///< IN The no-data flag
    int i4NumberRows,  ///< IN The number of rows in the image
    int i4NumberCols,  ///< IN The number of columns in the image
    fl32* r4Data     ///< IN+OUT The data to be converted
  ) ;

	void convert_dbz_norm(
		bool lDirection,  ///< IN: True for mm/h to normal, false for normal to mm/h
		fl32 r4NoData,   ///< IN: The no-data flag
		int i4NumberRowsPassed, ///< IN: The number of rows in the image
		int i4NumberColsPassed, ///< IN: The number of columns in the image
		fl32* r4Data     ///< IN+OUT: The data to be converted, fl32[i4NumberCols*i4NumberRows]
	) ;

  bool read_cascade( char *cInFileName) ; // Read a cascade off a file
  bool write_cascade( char *cOutFileName ) ; // Write the cascade to a file
  bool write_cascade_old( char *cOutFileName ) ; // Write the cascade to a file
  void decompose(  ) ;   // Decompose rainfall fields
  void decompose_and_track(  ) ;  // Decompose and track, used with NWP rainfall fields
  fl32* get_stackP(int cascadeNumber); // Return the pointer to a cascade
  void get_stack( fl32 *stackP )  ;    // Method to copy the cascade arrays
  void get_std(fl32 *std);             // Return the array of level standard deviations
  void get_mean(fl32 *mean);           // Return the array of level means
  fl32 get_RainMean( );                // Return the mean rain rate in mm/h
  fl32 get_RainStd( );                 // Return the standard deviation of rain in mm/h
  int get_cascade_levels();             // Return the number of levels in the cascade
  fl32 get_pixelSize() ;               // Return the size of the pixel in km
  fl32 get_noData() ;                  // Return the missing data flag
  fl32 get_fieldMean();                // Return the mean of the input field in dBr
  fl32 get_fieldStd();                 // Return the standard deviation of the input field in dBr
  fl32 get_rainFrac() ;                // Return the fraction of the field with rain rate >= 0.2 mm/h
  fl32 get_condMean() ;                // Return the mean rain rate conditioned on rain >= 0.2 mm/h
  int get_cascade_size() ;              // Return the size of the square used for the cascade
  FILE *get_logFileP() ;                // Return pointer to log file
  bool have_parameters()  ;             // Return flag if the cascade has valid parameters
  void set_logFileP ( FILE *fileP ) ; // Set the pointer to the log file
  void set_stack( fl32 *stackP ) ;   // Set the cascade arrays
  void set_DataMask( int *r4InMask ); // Set the valid data mask
  void update_parameters() ;  // Decompose field, track, estimate AR(2) parameters
  void update_weights ( ) ;  // Use AR(2) model to update the forecast cascade
  void get_velocities( fl32 *eVel, fl32 *sVel ) ;  // Get the velocity arrays mapped into cascadeSize x cascadeSize arrays
  void set_velocities( fl32 *eVel, fl32 *sVel ) ;  // Set the velocity arrays mapped into cascadeSize x cascadeSize arrays
  void get_rainMap( fl32 *r4rainMapPassed ) ; // Get the input map of rainfall intensities

/// \fn BCascade::get_scaleRatio()
/// \brief Get the ratio of the scales between the levels in the cascade
  fl32 get_scaleRatio() ;

/// \fn BCascade::get_rainMean()
/// \brief Return rain mean (mm/h)
  fl32 get_rainMean() { return r4RainMean; } ;

/// \fn BCascade::get_rainStd()
/// \brief Return rain standard deviation (mm/h)
  fl32 get_rainStd() { return r4RainStd; } ;

/// \fn  BCascade::set_deterministic_flag(bool detFlag)
/// \brief Set the deterministic forecast flag to true
  void set_deterministic_flag(bool detFlag) {
    bIsDet = detFlag ;
  } ;

/// \fn  BCascade::set_mean_std(fl32 *r4CascadeLevelStdPassed,fl32 r4FieldMeanPassed, fl32 r4FieldStdPassed)
/// \brief Set the cascade level standard deviations, field mean, field std
  void set_mean_std(fl32 *r4CascadeLevelStdPassed, fl32 r4FieldMeanPassed, fl32 r4FieldStdPassed) {
	int i4Loop ;
	for ( i4Loop = 0; i4Loop < i4NumberLevels; i4Loop++ )
	  r4CascadeLevelStd[i4Loop] = r4CascadeLevelStdPassed[i4Loop] ;
	r4FieldMean = r4FieldMeanPassed ;
	r4FieldStd  = r4FieldStdPassed ;
  } ;

/// \fn BCascade::set_rainFraction( fl32 r4RainFractionPassed)
///\brief Set the raining fraction
  void  set_rainFrac( fl32 r4RainFractionPassed){
	  r4RainFrac = r4RainFractionPassed ;
  }  ;

/// \fn BCascade::normalize_ms (fl32 r4NoData,int i4ArrayLength,fl32 r4ReqMean,fl32 r4ReqStd,int   *i4DataMask,fl32 *r4Data)
/// \brief Normalize the dbr data under the mask
  void normalize_ms (
	fl32 r4NoData,       ///< IN Missing data flag
	int i4ArrayLength,    ///< IN Length of input array
	fl32 r4ReqMean,      ///< IN Required mean
	fl32 r4ReqStd,       ///< IN Required std
	int   *i4DataMask,    ///< IN Mask for the area with data in the input image
	fl32 *r4Data         ///< IN+OUT Data array [i4ArrayLength] in size
  ) ;

/// \fn BCascade::get_map(fl32 *r4RadarMapReturned)
/// \brief Copy the current map (in units of dbr)
  void get_map( fl32 *r4RadarMapReturned ){
	int i4Loop ;
	for (i4Loop = 0; i4Loop < (int)i4CascadeArraySize; i4Loop++) {
	  r4RadarMapReturned[i4Loop] = r4Maps[0][i4Loop] ;
	}
  }   ;

// Set default auto-correlations
	void set_default_autocor() ;
	void set_default_autocor(fl32 r4CorA, fl32 r4CorB, fl32 r4CorC) ;
  void get_DataMask( int *) ; 

};

#endif


