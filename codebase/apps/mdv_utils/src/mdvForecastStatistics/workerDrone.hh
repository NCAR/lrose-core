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
/////////////////////////////////////////////////////////////
//
// Niles Oien, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2006
//
/////////////////////////////////////////////////////////////

#ifndef WORKER_DRONE_HH
#define WORKER_DRONE_HH

#include <string>
#include <vector>
#include <ctime>


/**
 *
 * @file workerDrone.hh
 *
 * @class workerDrone
 *
 * This is the class that does most of the work for this application
 * (aside from reading the data, which is done in Main.cc). Applies
 * a map mask, applies minimum clump size criteria, generates
 * a thresholded table of storm/noStorm, generates an output
 * validated data table using a thresholded table from another
 * workerDrone class.
 *
 * @author Niles Oien
 *
 */
using namespace std;

class workerDrone {
  
public:
  
  //
  // Possible outcomes for thresholding operation.
  //
  const static int thresholdedIgnore = 0;   
  const static int thresholdedNoStorm = 1;   
  const static int thresholdedStorm = 2;   

  //
  // Possible outcomes of validation.
  //
  const static int validatedIgnore = 0;
  const static int validatedNonEvent = 1;
  const static int validatedFalseAlarm = 2;
  const static int validatedMiss = 3;
  const static int validatedHit = 4;

  //
  // Possible actions when encountering bad/missing data when thresholding.
  //
  const static int badValueActionStorm = 0;
  const static int badValueActionNoStorm = 1;
  const static int badValueActionIgnore = 2;


/**
 * The constructor for the workerDrone class.
 *
 * @param data A pointer to the data, we make an internal copy.
 * @param bad_data_value The MDV bad data value for these data.
 * @param missing_data_value The MDV missing data value.
 * @param nx Number of points in X for these data. X changes
 *           fastest in memory.
 * @param ny Number of points in Y for these data.
 * @param nx Number of points in Z for these data. Z changes
 *           slowest in memory.
 *
 * @return  No return value.
 *
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  workerDrone(fl32 *data,
	      fl32 bad_data_value,
	      fl32 missing_data_value,
	      int nx, int ny, int nz);



/**
 * Allocate space for, and calculate, a table of thresholded
 * values for the data.
 *
 * @param min The threshold minimum
 * @param max The threshold maximum
 * @param badValueAction Which should be one of 
 * workerDrone::badValueActionStorm,
 * workerDrone::badValueActionNoStorm or
 * workerDrone::badValueActionIgnore depending on if we should
 * treat bad/missing values as storms, treat them as not storms, or
 * ignore them completely.
 *
 * @return  None.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void calcThresh( fl32 min, fl32 max, int badValueAction );


/**
 * Apply a minimum clump size criterion to the thresholded data.
 * calcThresh() should be invoked before using this.
 *
 * @param minClumpSize minimum clup size, grid points.
 *
 * @todo Cope with polar projection MDV data which "wraps"
 * in the azimuth. apps/mdv_utils/src/MdvClump does this,
 * or at least claims to.
 *
 * @return  None.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void applyClump( int minClumpSize );




/**
 * Apply a map mask to the thresholded data.
 * calcThresh() should be invoked before using this.
 *
 * @param mapMask pointer to nx*ny ui08 used to mask thresholded data.
 *
 * @return  None.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void applyMapMask( ui08 *mapMask );


/**
 * Allocate space for, and calculate, a validated table from another
 * workerDrone's thresholded data.
 * calcThresh() should be invoked before using this.
 *
 * @note For this to work correctly, it should be invoked on
 * the workerDrone that is loaded with the forecast data. Othewise
 * misses and false alarms will be transposed.
 *
 * @param otherThreshData Pointer to nx*ny*nz ui08 table of threshold
 * results from another workerDrone.
 *
 * @return  None.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void validate( ui08 *otherThreshData );



/**
 *
 * Get a pointer to the nx*ny*nz ui08 threshold data.
 * calcThresh() should be invoked before using this.
 *
 * @return Pointer to data, or NULL if not ready yet.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
 ui08 *getThresholdedData();

/**
 *
 * Get a pointer to the nx*ny*nz ui08 validation data.
 * calcThresh() and validate() should be invoked before using this.
 *
 * @return Pointer to data, or NULL if not ready yet.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
 ui08 *getValidationData();

/**
 *
 * Relax the thresholded data in the horizontal.
 * calcThresh() should be invoked before using this.
 *
 * @param halfWindowSize Integer defining the window size. Actual
 * window size is 2*halfWindowSize+1 square.
 *
 * @return None
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void applyHorizontalRelaxation(int halfWindowSize );


/**
 * Get the results of the call to validate().
 *
 * @return  None. Data is passed back in pointers.
 *
 * @note because you should only invoke validate() on a workerDrone
 * loaded with forecast data, and you need to call validate() before
 * calling this, this should only be invoked on a workerDrone loaded
 * with forecast data - NOT truth data.
 *
 * @param numMiss Pointer to the number of misses, ie. the number
 *                of grid points where we did not predict a storm
 *                but one occurred.
 *
 * @param numHit Pointer to the number of hits, ie the number of
 *               grid points where we said there would be a storm
 *               and there was a storm
 *
 * @param numNon Pointer to the number of non events, grid points
 *               where no storm was forecast and none occurred.
 *
 * @param numFalse Pointer to the number of false alarms - points
 *                 where a storm was predicted but none occurred.
 *
 * @param numConsidered Pointer to the total number of points
 *                      considered.
 *
 * 
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void getCounts(int *numMiss, 
		 int *numHit, 
		 int *numNon, 
		 int *numFalse, 
		 int *numConsidered);
  
/**
 * Destructor. Frees up memory as needed.
 *
 * @return  None.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  ~workerDrone();
  
protected:
  
private:

/**
 *
 * Internal method to act as front end to euclid routine that
 * does clumping.
 *
 * @param plane Pointer to two dimensional byte array to
 *              do clumping on.
 *
 * @param nx Number of points in X
 * @param ny Number of points in Y
 * @param threshold_byte The minimum byte value for a point to be "on"
 * @param min_npoints Minimum number of points in a clump for it to survive
 * @param max_npoints Maximum number of points in a clump for it to survive
 *
 * @return None
 *
 * @author Niles Oien oien@ucar.edu
 *
 */ 
  void _clumpPlane( ui08 *plane, int nx, int ny,
		    int threshold_byte, int min_npoints,
		    int max_npoints);


  fl32 *_data;
  fl32 _bad_data_value, _missing_data_value;

  int _nx, _ny, _nz;

  ui08 *_thresholdedData;
  ui08 *_validatedData;

  int _numMiss, _numHit, _numNon, _numFalse, _numConsidered;

};

#endif
