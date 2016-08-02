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

#include "polyMaps.hh"
#include "workerDrone.hh"

#include <string>
#include <vector>
#include <ctime>

#include <euclid/boundary.h>
#include <toolsa/umisc.h>

using namespace std;
/**
 *
 * @file workerDrone.cc
 *
 * Implementation of workerDrone class.
 *
 * @author Niles Oien
 *
 */

workerDrone::workerDrone(fl32 *data,
			 fl32 bad_data_value,
			 fl32 missing_data_value,
			 int nx, int ny, int nz){
  //
  // Initialize, and make copies of everything.
  //
  _numMiss = _numHit = _numNon = _numFalse = _numConsidered =0;
  _validatedData = NULL;
  _thresholdedData = NULL;
  _nx = nx; _ny = ny; _nz = nz;
  _bad_data_value = bad_data_value;
  _missing_data_value = missing_data_value;

  _data = data;


  return;
}




void workerDrone::calcThresh( fl32 min, fl32 max, int badValueAction ){
  //
  // Allocate space, if needed.
  //
  if (_thresholdedData == NULL){
    _thresholdedData = (ui08 *)malloc(_nx*_ny*_nz*sizeof(ui08));
    if (_thresholdedData == NULL){
      cerr << "Threshold data allocation failed in workerDrone!" << endl;
      exit(-1);
    }
  }

  for (int i=0; i < _nx*_ny*_nz; i++){
    if (
	(_data[i] == _bad_data_value) ||
	(_data[i] == _missing_data_value)
	){
      switch (badValueAction) {

      case workerDrone::badValueActionStorm :
	_thresholdedData[i] = workerDrone::thresholdedStorm;
	break;
 
      case workerDrone::badValueActionNoStorm :
	_thresholdedData[i] = workerDrone::thresholdedNoStorm;
	break;

      default :
	_thresholdedData[i] = workerDrone::thresholdedIgnore;
	break;
      }
    } else {
      //
      // Not bad/missing data, do actual thresholding.
      //
      if ((_data[i] < min) || (_data[i] > max))
	_thresholdedData[i] = workerDrone::thresholdedNoStorm;
      else
	_thresholdedData[i] = workerDrone::thresholdedStorm;
    }
  }

  return;
}

//
// Return results of counts.
//
void workerDrone::getCounts(int *numMiss, 
			    int *numHit, 
			    int *numNon, 
			    int *numFalse, 
			    int *numConsidered){

  *numMiss = _numMiss;
  *numHit = _numHit;
  *numNon = _numNon;
  *numFalse = _numFalse;
  *numConsidered = _numConsidered;

  return;
}



void workerDrone::applyClump( int minClumpSize ){

  //
  // Allocate a plane of byte data.
  //
  ui08 *work = (ui08 *)malloc(_nx*_ny*sizeof(ui08));
  if (work == NULL){
    cerr << "Failed to allocate plane for clump operations." << endl;
    exit(-1);
  }

  //
  // Loop through the planes.
  //
  for (int iz=0; iz < _nz; iz++){

    //
    // For each plane, fill the working buffer with
    // the value 0 for no storm or ignore, 64 for storm
    // (arbitrary).
    //
    for (int iy=0; iy < _ny; iy++){
      for (int ix=0; ix < _nx; ix++){

	work[iy * _nx + ix] = 0;
	if (_thresholdedData[iz*_nx*_ny + iy*_nx + ix] == workerDrone::thresholdedStorm)
	  work[iy * _nx + ix] = 64
;
      }
    }

    //
    // Do clumping on this plane.
    //
    _clumpPlane(work, _nx, _ny, 32, minClumpSize, _nx*_ny);
    
    //
    // Now, for the points that used to be storms but the clumping operation
    // set to 0, set to thresholdedNoStorm.
    //
    for (int iy=0; iy < _ny; iy++){
      for (int ix=0; ix < _nx; ix++){  

	if (
	    (_thresholdedData[iz*_nx*_ny + iy*_nx + ix] == workerDrone::thresholdedStorm) &&
	    (work[iy * _nx + ix] != 64)
	    ){
	  _thresholdedData[iz*_nx*_ny + iy*_nx + ix] = workerDrone::thresholdedNoStorm;
	}
      }
    }
  }


  free(work);


  return;
}




void workerDrone::applyMapMask( ui08 *mapMask ){
  if (_thresholdedData == NULL){
    cerr << "WARNING - workerDrone::applyMapMask() called out of order!" << endl;
    return;
  }

  for (int iy=0; iy < _ny; iy++){
    for (int ix=0; ix < _nx; ix++){
      if (mapMask[iy * _nx + ix] == polyMaps::polyMapNotRelevant){
	for (int iz=0; iz < _nz; iz++){
	  _thresholdedData[iz*_nx*_ny + iy*_nx + ix] = workerDrone::thresholdedIgnore;
	}
      }
    }
  }

  return;
}


//
// Note - only invoke this on a workerDrone that has been
// loaded with forecast data - not truth data.
//
void workerDrone::validate( ui08 *otherThreshData ){

  //
  // Reset counts.
  //
  _numMiss = _numHit = _numNon = _numFalse = _numConsidered =0;

  if (_thresholdedData == NULL){
    cerr << "WARNING - workerDrone::validate() called out of order!" << endl;
    return;
  }

  if (_validatedData == NULL){
    _validatedData = (ui08 *)malloc(_nx*_ny*_nz*sizeof(ui08));
    if (_validatedData == NULL){
      cerr << "Validated data allocation failed in workerDrone!" << endl;
      exit(-1);
    }
  }

  for (int i=0; i < _nx*_ny*_nz; i++){

    //
    // If either one is marked as Ignore, ignore it.
    //
    if (
	(_thresholdedData[i] == workerDrone::thresholdedIgnore) ||
	(otherThreshData[i] == workerDrone::thresholdedIgnore)
	){
      _validatedData[i] = workerDrone::validatedIgnore;
      continue;
    }
    
    //
    // Is it a hit?
    //
    if (
	(_thresholdedData[i] == workerDrone::thresholdedStorm) &&
	(otherThreshData[i] == workerDrone::thresholdedStorm)
	){
      _validatedData[i] = workerDrone::validatedHit;
      _numHit++; _numConsidered++;
      continue;
    }

    //
    // Is it a nonEvent?
    //
    if (
	(_thresholdedData[i] == workerDrone::thresholdedNoStorm) &&
	(otherThreshData[i] == workerDrone::thresholdedNoStorm)
	){
      _validatedData[i] = workerDrone::validatedNonEvent;
      _numNon++; _numConsidered++;
      continue;
    }

    //
    // Is it a false alarm?
    //
    if (
	(_thresholdedData[i] == workerDrone::thresholdedStorm) &&
	(otherThreshData[i] == workerDrone::thresholdedNoStorm)
	){
      _validatedData[i] = workerDrone::validatedFalseAlarm;
      _numFalse++; _numConsidered++;
      continue;
    }

    //
    // Is it a miss?
    //
    if (
	(_thresholdedData[i] == workerDrone::thresholdedNoStorm) &&
	(otherThreshData[i] == workerDrone::thresholdedStorm)
	){
      _validatedData[i] = workerDrone::validatedMiss;
      _numMiss++; _numConsidered++;
      continue;
    }
  }

  return;
}




ui08 *workerDrone::getThresholdedData(){
  return _thresholdedData;
}

ui08 *workerDrone::getValidationData(){
  return _validatedData;
}


void workerDrone::applyHorizontalRelaxation(int halfWindowSize ){

  if (halfWindowSize == 0) return; // No action needed.

  if (NULL == _thresholdedData){
    cerr << "ERROR - workerDrone::applyHorizontalRelaxation() called out of order." << endl;
    return;
  }

  //
  // Allocate a working buffer.
  //
  ui08 *work = (ui08 *)malloc(_nx*_ny*_nz*sizeof(ui08));
  if (work == NULL){
    cerr << "Malloc failed in relaxation routine." << endl;
    exit(-1);
  }

  for (int iz=0; iz < _nz; iz++){

     for (int iy=0; iy < _ny; iy++){
       for (int ix=0; ix < _nx; ix++){

	 //
	 // Just copy workerDrone::thresholdedIgnore values across.
	 //
	 if (_thresholdedData[ix + iy*_nx + iz*_nx*_ny] == workerDrone::thresholdedIgnore){
	   work[ix + iy*_nx + iz*_nx*_ny] =  workerDrone::thresholdedIgnore;
	   continue;
	 }

	 //
	 // If we have a storm in the window, then set the working buffer
	 // value to storm - otherwise no storm.
	 //
	 work[ix + iy*_nx + iz*_nx*_ny] =  workerDrone::thresholdedNoStorm;

	 for (int ixx = ix-halfWindowSize; 
	      (ixx < ix+halfWindowSize+1) && (work[ix + iy*_nx + iz*_nx*_ny] == workerDrone::thresholdedNoStorm);
	      ixx++){
	   for (int iyy = iy-halfWindowSize;
		(iyy < iy+halfWindowSize+1) && (work[ix + iy*_nx + iz*_nx*_ny] == workerDrone::thresholdedNoStorm);
		iyy++){
	     //
	     // Make sure we're still in the grid
	     //
	     if ((ixx > -1) && (iyy > -1) && (ixx < _nx) && (iyy < _ny)){
	       if (_thresholdedData[ixx + iyy*_nx + iz*_nx*_ny] == workerDrone::thresholdedStorm){
		 work[ix + iy*_nx + iz*_nx*_ny] =  workerDrone::thresholdedStorm;
	       }
	     }
	   }
	 }
       }
     }
  }

  //
  // Copy the working buffer back into the thresholded data, free the buffer and return.
  //
  memcpy(_thresholdedData, work, _nx*_ny*_nz*sizeof(ui08));
  free(work);

  return;
}


///////////////////////////////////////////////////
void workerDrone::_clumpPlane( ui08 *plane, int nx, int ny,
			       int threshold_byte, int min_npoints,
			       int max_npoints)

{

  int i,j;		/* counters */
  int offset;		/* offset into data planes */
  int num_intervals = 0;
  int num_clumps = 0;
  Interval *intv_ptr = NULL;
  Clump_order *clump_ptr = NULL;

  static Row_hdr *Rowh = NULL;
  static int Nrows_alloc = 0;
  static Interval *Intervals = NULL;
  static int N_intervals_alloc = 0;

  static int N_ints_alloc_clump = 0;
  static Clump_order *Clumps = NULL;	/* a set of clumps */
  static Interval **Interval_order = NULL; 
  
  /*
   * Allocate Space for Row structs
   */

  EG_alloc_rowh(ny, &Nrows_alloc, &Rowh);
  
  /*
   * Find all intervals in each row
   */
  
  num_intervals = EG_find_intervals(ny,nx,plane,&Intervals,
				    &N_intervals_alloc,Rowh,
				    threshold_byte);
  
  /*
   * Allocate space for clump structs & make sure ID's are set to NULL_ID 
   */
  
  EG_alloc_clumps(num_intervals, &N_ints_alloc_clump,
		  &Clumps, &Interval_order);
  EG_reset_clump_id(Intervals, num_intervals);

  /*
   * Find all Clumps using the Intervals previously found
   */
  
  num_clumps = EG_rclump_2d(Rowh,ny,TRUE,1,Interval_order, Clumps);

  /*
   * 0th Clump is not used and Clumps array is actually num_clumps+1 big
   */

  clump_ptr = &(Clumps[1]);

  /*
   * Loop through all Clumps
   */
  
  int num_gone=0;
  for(i=0; i < num_clumps; i++, clump_ptr++) {
    
    if (
	(clump_ptr->pts < min_npoints) ||
	((max_npoints > 0) && (clump_ptr->pts > max_npoints))
	){
      num_gone++;
      /*
       * For small clumps set grid value to 
       * the threshold_byte
       */
      
      /*
       * Loop through each interval in clump
       */
      
      for(j=0; j < clump_ptr->size; j++) { 
	intv_ptr = clump_ptr->ptr[j];
	offset = (intv_ptr->row_in_plane * nx) + intv_ptr->begin;
	memset(plane + offset, threshold_byte, intv_ptr->len);
      }
    }

  } /* i */
  
}





workerDrone::~workerDrone(){
  if (NULL != _thresholdedData) free(_thresholdedData);
  if (NULL != _validatedData) free(_validatedData);
  return;
}
  


