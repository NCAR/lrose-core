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

#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <toolsa/umisc.h>
#include <cmath>

using namespace std;

#include "SoundingMerge.hh"


////////////////////////////////////////////////
//
// Constructor. Makes copy of TDR parameters.
// Sets matching criteria. Initializes
// internal data to NULL.
//
SoundingMerge::SoundingMerge(const Params *P){
  _epsilon =10.0;       // The default.
  _params = P;           // Copy parameters.
  //
  // Init internals to be out of bounds.
  //
  _productIndex = -1;
  _id = -1;
  _dataTime = 0;
  
  return;

}
////////////////////////////////////////////////
//
// Destructor.
//
SoundingMerge::~SoundingMerge(){

  return;

}
////////////////////////////////////////////////
//
//
// Init. Client gives the class the sounding ID and the time,
// and it looks in the output SPDB database to see if there is
// already something there. If there is, it is sorted into ascending
// order with respect to altitude and stored internally.
//
int SoundingMerge::Init(time_t dataTime, int id){
  //
  // Make copies of input args.
  //
  _id = id; _dataTime = dataTime;
  //
  // See if we have data in the database for this time.
  //
  if (_params->debug){
    cerr << "Initializing sounding get with " << _params->OutUrl << endl;
  }
  _Sget.init( _params->OutUrl );
  if (_params->debug){
    cerr << "Initialized sounding get with " << _params->OutUrl << endl;
  }
  //
  //
  if (_params->debug){
    fprintf(stderr,"Looking for existing data in %s\n", _params->OutUrl );
    fprintf(stderr,"at time %s\n",utimstr( dataTime ));
    fprintf(stderr,"ID is %d\n",id);
  }
  //
  int numProds = _Sget.readSounding( dataTime, id );
  if (_params->debug){
    if (numProds < 1){
      fprintf(stderr,"No products found.\n");
    } else {
      fprintf(stderr,"%d products found.\n", numProds);
    }
  }
  if (numProds == -1)
    return 0; // Dismal failure.
  //
  // Loop through the products looking for our ID.
  // If we find it set _productIndex to record it.
  //
  //
  for (int i=0; i < numProds; i++){
    _Sget.loadProduct( i );
    if (id == _Sget.getSiteId()){
      _productIndex = i;
    }
  }
  return 1; // Did OK, even if we didn't find any data.
}
//////////////////////////////////////////////////
//
// Merge. The real woo-woo of the class.  You pass in the specs for
// a sounding and it is merged with whatever was stored internally
// by the init. This is then written out to the SPDB database.
//
int SoundingMerge::Merge(SoundingDataHolder toBeAdded){
  //
  // Make sure the client invoked Init first.
  //
  if (_id == -1) return 0;
  //
  // Sanity check - make sure there are no NaNs in the
  // height field.
  //
  for (int i=0; i < toBeAdded.numPoints; i++){
    if (std::isnan(toBeAdded.height[i])) return 0;
  }
  //
  // Sort the input data into ascending order with respect to height.
  //
  Sort( toBeAdded );
  //
  // Merge the data in with existing data, if we have any.
  // Otherwise just copy what we have across.
  // 
  SoundingDataHolder merged;
  if (_productIndex != -1){
    _merge( toBeAdded, merged );
  } else {
    merged = toBeAdded; // Starting anew.
  }
  //
  // Fill in any missing data points, if requested.
  //
  if ( _params->doInterp ) 
    SoundingMerge::_interpSounding(merged, _params->maxInterpDist);
  //
  // Print sounding out before saving, if requested.
  //
  if (_params->printBeforeSave)
    _printSounding( merged );
  //
  // Write the data out. Have to push back url to do so.
  //
  vector< string* > urlVec;
  string Url( _params->OutUrl );
  urlVec.push_back( &Url );

  // SoundingPut S;

  _Sput.init(urlVec, 
	     Sounding::DEFAULT_ID, 
	     merged.source, 
	     _id,
	     merged.siteName,
	     merged.lat, merged.lon, 
	     merged.alt, merged.badVal );

  //
  // If we are limiting wind speed, do that now.
  //
  if (_params->limit.limitWindSpeed){
    for (int k=0; k < merged.numPoints; k++){
      if (sqrt(merged.u[k]*merged.u[k] + merged.v[k]*merged.v[k]) > _params->limit.maxWindSpeed){
	if (_params->limit.acceptDirection){
	  double factor = _params->limit.maxWindSpeed / sqrt(merged.u[k]*merged.u[k] + merged.v[k]*merged.v[k]);
	  merged.u[k] *= factor; merged.v[k] *= factor; 
	} else {
	  merged.u[k] = merged.v[k] = merged.w[k] = merged.badVal;
	}
      }
    }
  }

  _Sput.set(_dataTime,
	    merged.numPoints,
	    merged.height,
	    merged.u,
	    merged.v,
	    merged.w,
	    merged.prs,
	    merged.relHum,
	    merged.temp);
  

  _Sput.writeSounding( _dataTime, _dataTime + _params->expiry );

  //
  // Free myself, if allocated.
  //
  if (_productIndex != -1)  SoundingMerge::FreeSoundingDataHolder( merged );

  return 1; // Did OK.
}
////////////////////////////////////////////////
//
// Soundings are merged by comparing heights. If the
// heights of two levels  match to within epsilon then they
// are taken to be the same level. Values of less than
// 0.5 meters are rejected.
//
void SoundingMerge::setMergeEpsilon(double epsilon ){
  if ((epsilon >= 0.5) && (epsilon <= 500.0)){
    _epsilon = epsilon;
  } else {
    fprintf(stderr,"Epsilon value %g not between 0.5 and 500.\n",epsilon);
    fprintf(stderr,"Using %g instead.\n",_epsilon);
  }
  return;
}
//////////////////////////////////////////////////////////
//
// Free up a SoundingDataHolder structure.
//
void SoundingMerge::FreeSoundingDataHolder( SoundingDataHolder S){
  //
  // Does not deallocate strings.
  //
  if (NULL!=S.height){
    free(S.height);
    S.height = NULL;
  }

  if (NULL!=S.u){
    free(S.u);
    S.u = NULL;
  }
  

  if (NULL!=S.v){
    free(S.v);
    S.v = NULL;
  }

  if (NULL!=S.w){
    free(S.w);
    S.w = NULL;
  }
 
  if (NULL!=S.prs){
    free(S.prs);
    S.prs = NULL;
  }

  if (NULL!=S.relHum){
    free(S.relHum);
    S.relHum = NULL;
  }

  if (NULL!=S.temp){
    free(S.temp);
    S.temp = NULL;
  }

  return;

}
//////////////////////////////////////////////////////////
//
// Allocate space for a SoundingDataHolder structure.
// Does not allocate strings, only doubles. Assumes that
// the SoundingGet object has been initialized and has
// its bad value set (why else would we be here if SoundingGet
// didn't have data, when you think about it?).
//
void SoundingMerge::AllocateSoundingDataHolder( SoundingDataHolder *S,
						int numPoints){

  S->height = (double *)malloc(numPoints * sizeof(double));
  S->u =      (double *)malloc(numPoints * sizeof(double));
  S->v =      (double *)malloc(numPoints * sizeof(double));
  S->w =      (double *)malloc(numPoints * sizeof(double));
  S->prs =    (double *)malloc(numPoints * sizeof(double));
  S->relHum = (double *)malloc(numPoints * sizeof(double));
  S->temp =   (double *)malloc(numPoints * sizeof(double));

 if (
     (S->height == NULL) ||
     (S->u == NULL) ||
     (S->v == NULL) ||
     (S->w == NULL) ||
     (S->prs == NULL) ||
     (S->relHum == NULL) ||
     (S->temp == NULL) 
     ){
   fprintf(stderr,"Malloc failed with %d points.\n",numPoints);
   exit(-1); // Not likely.
 }
 //
 // Set all the data to missing.
 //
 double inBadVal = _Sget.getMissingValue();
 //
 for (int i=0; i < numPoints; i++){
   S->height[i] = inBadVal;
   S->u[i] = inBadVal;
   S->v[i] = inBadVal;
   S->w[i] = inBadVal;
   S->prs[i] = inBadVal;
   S->relHum[i] = inBadVal;
   S->temp[i] = inBadVal;
 }
}

//////////////////////////////////////////////////////////////////
//
// Sort a SoundingDataHolder structure into ascending order wrt height.
//
void SoundingMerge::Sort( SoundingDataHolder S){
  //
  // Just use bubble sort. The quicksort method can't be easily
  // applied to this structure and the thing should be very close to being in
  // ascending order anyway. Assume no bad values in height field and
  // that the height array if there (ie height != NULL).
  //
  int interchanges;
  do {
    interchanges = 0;
    //
    for (int i=0; i < S.numPoints-1; i++ ){
      if (S.height[i] > S.height[i+1]){
	//
	interchanges = 1;
	double t;
	t = S.height[i]; S.height[i] = S.height[i+1]; S.height[i+1] = t;
	//
	// The other swappings are conditional on our having data.
	//
	if (S.u != NULL) {
	  t = S.u[i]; S.u[i] = S.u[i+1]; S.u[i+1] = t;
	}

	if (S.v != NULL) {
	  t = S.v[i]; S.v[i] = S.v[i+1]; S.v[i+1] = t;
	}
	
	if (S.w != NULL){
	  t = S.w[i]; S.w[i] = S.w[i+1]; S.w[i+1] = t;
	}

	if (S.prs !=NULL){
	  t = S.prs[i]; S.prs[i] = S.prs[i+1]; S.prs[i+1] = t;
	}

	if (S.relHum != NULL){
	  t = S.relHum[i]; S.relHum[i] = S.relHum[i+1]; S.relHum[i+1] = t;
	}

	if (S.temp != NULL){
	  t = S.temp[i]; S.temp[i] = S.temp[i+1]; S.temp[i+1] = t;
	}
      }
    }

  } while (interchanges);

  return;
}

/////////////////////////////////////////////////



void SoundingMerge::_merge(SoundingDataHolder toBeAdded,
			   SoundingDataHolder &merged){

  //
  // Copy the ancillary data across.
  //
  merged.lat = toBeAdded.lat;
  merged.lon = toBeAdded.lon;
  merged.alt = toBeAdded.alt;

  merged.source = toBeAdded.source;
  merged.siteName = toBeAdded.siteName;

  merged.badVal = toBeAdded.badVal;

  //
  // get the data out of the existing SoundingGet object.
  //
  double *exHt = _Sget.getAlts();
  double *exU = _Sget.getU();
  double *exV = _Sget.getV();
  double *exW = _Sget.getW();
  double *exPrs = _Sget.getPres();
  double *exRH = _Sget.getRH();
  double *exTemp = _Sget.getTemp();
  //
  double exBadVal = _Sget.getMissingValue();
  //
  // Allocate enough space to cope with the eventuality
  // that both soundings have data at distinct heights, ie. all
  // interleaving, no merging.
  //
  int numExistingPoints = _Sget.getNumPoints();
  int numMergedPoints = toBeAdded.numPoints + numExistingPoints;
  //
  //
  AllocateSoundingDataHolder( &merged, numMergedPoints);
  merged.badVal = exBadVal;
  //
  int getIndex = 0; int addIndex = 0; int outIndex = 0;
  do {
    //
    // See if we have two equal heights, within epsilon.
    //
    if (fabs(exHt[getIndex] - toBeAdded.height[addIndex]) <= _epsilon){

      // MERGE CODE
      //
      // Merge the individual points across. The toBeAdded points have
      // priority over the existing points.
      //
      if (_params->debug){
	fprintf(stderr,"Merging existing height %g with new height %g (D=%g)\n",
		exHt[getIndex], toBeAdded.height[addIndex],
		fabs(exHt[getIndex]-toBeAdded.height[addIndex]));
      }
      //
      // Set the new height to be the mean of the two mergees.
      //
      merged.height[outIndex] = 
	(toBeAdded.height[addIndex] + exHt[getIndex])/2.0;

      _mergeVal(toBeAdded.u, exU, merged.u,
		addIndex, getIndex, outIndex,
		toBeAdded.badVal, exBadVal);

      _mergeVal(toBeAdded.v, exV, merged.v,
		addIndex, getIndex, outIndex,
		toBeAdded.badVal, exBadVal);

      _mergeVal(toBeAdded.w, exW, merged.w,
		addIndex, getIndex, outIndex,
		toBeAdded.badVal, exBadVal);

      _mergeVal(toBeAdded.prs, exPrs, merged.prs,
		addIndex, getIndex, outIndex,
		toBeAdded.badVal, exBadVal);

      _mergeVal(toBeAdded.relHum, exRH, merged.relHum,
		addIndex, getIndex, outIndex,
		toBeAdded.badVal, exBadVal);

      _mergeVal(toBeAdded.temp, exTemp, merged.temp,
		addIndex, getIndex, outIndex,
		toBeAdded.badVal, exBadVal);

      //
      // The merge of this point is complete.
      // Increment both indicies, decrement the
      // total number of points, and move on.
      //
      getIndex++; addIndex++; numMergedPoints--; outIndex++; continue;
    }
    //
    // Heights are not equal - one must come into the array before the other.
    // See which one.
    //
    if (exHt[getIndex] < toBeAdded.height[addIndex]){
      //
      // The existing point goes in first.
      //
      merged.height[outIndex] = exHt[getIndex];
      _moveVal(exU, merged.u, getIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(exV, merged.v, getIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(exW, merged.w, getIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(exPrs, merged.prs, getIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(exRH, merged.relHum, getIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(exTemp, merged.temp, getIndex, outIndex, merged.badVal, exBadVal);

      getIndex++; outIndex++; continue;
      //
    } else {
      //
      // The toBeAdded point goes in first.
      //
      merged.height[outIndex] = toBeAdded.height[addIndex];
      _moveVal(toBeAdded.u, merged.u, addIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(toBeAdded.v, merged.v, addIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(toBeAdded.w, merged.w, addIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(toBeAdded.prs, merged.prs, addIndex, 
	       outIndex, merged.badVal, exBadVal);
      _moveVal(toBeAdded.relHum, merged.relHum, addIndex, outIndex, 
	       merged.badVal, exBadVal);
      _moveVal(toBeAdded.temp, merged.temp, addIndex, outIndex, 
	       merged.badVal, exBadVal);
      //
      addIndex++; outIndex++; continue;
      //      
    }

  } while (
	   (getIndex < numExistingPoints) &&
	   (addIndex < toBeAdded.numPoints)
	   );

  merged.numPoints=numMergedPoints;
  //
  // If we have no more points, return.
  //
  if (
      (getIndex >= numExistingPoints) &&
      (addIndex >= toBeAdded.numPoints)
      ){ 
    return;
  }
  //
  // We have more points - either in one structure or the other.
  //
  if (addIndex < toBeAdded.numPoints) {
    //
    // All through with existing points, only points
    // from the toBeAdded struct need to be added.
    //
    do {
      //
      merged.height[outIndex] = toBeAdded.height[addIndex];
      _moveVal(toBeAdded.u, merged.u, addIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(toBeAdded.v, merged.v, addIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(toBeAdded.w, merged.w, addIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(toBeAdded.prs, merged.prs, addIndex, 
	       outIndex, merged.badVal, exBadVal);
      _moveVal(toBeAdded.relHum, merged.relHum, addIndex, outIndex, 
	       merged.badVal, exBadVal);
      _moveVal(toBeAdded.temp, merged.temp, addIndex, outIndex, 
	       merged.badVal, exBadVal);
      //      
      addIndex++; outIndex++;
    } while (addIndex < toBeAdded.numPoints);
  } else {
    //
    // All through with the toBeAdded points, only
    // points from the existing struct need to be added.
    //
    do{
      //
      merged.height[outIndex] = exHt[getIndex];
      _moveVal(exU, merged.u, getIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(exV, merged.v, getIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(exW, merged.w, getIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(exPrs, merged.prs, getIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(exRH, merged.relHum, getIndex, outIndex, merged.badVal, exBadVal);
      _moveVal(exTemp, merged.temp, getIndex, outIndex, merged.badVal, exBadVal);
      //
      getIndex++; outIndex++;
    } while (getIndex < numExistingPoints);
  }

  merged.numPoints=numMergedPoints;
  return;

}

////////////////////////////////////////////////////
//
// Code to move sounding values around. Private to this class.
//
// If the destination array is NULL, nothing happens.
// If the dest array is good but the source array is NULL,
// the point is set to DestBadVal in the dest array.
//
// If there is a value in the source array euqal to
// SourceBadVal then the dest array point is set to DestBadVal.
//
// Finally, if there is a valid point in the SourceArray then it
// is moved across to the DestArray.
// 
void SoundingMerge::_moveVal(double *SourceArray,
			     double *DestArray,
			     int SourceIndex,
			     int DestIndex,
			     double DestBadVal,
			     double SourceBadVal){

  if (DestArray == NULL) return; // Nothing happens.

  if (SourceArray == NULL){ // Set dest point to bad.
    DestArray[DestIndex] = DestBadVal;
    return;
  }

  if (SourceArray[SourceIndex] == SourceBadVal){ // Set dest point to bad.
    DestArray[DestIndex] = DestBadVal;
    return;
  }
  //
  // Checks for bad situations are complete, do what we
  // came here to do.
  //
  DestArray[DestIndex] = SourceArray[SourceIndex];
  return;

}

//////////////////////////////////////////////////////
//
// Code to join two points together.
//
// If secondary and preferred are NULL, out[outIndex] is
// set to bad.
//
// If both preferred and secondary are either NULL or set to their bad
// values, then out[outIndex] is set to bad.
//
// After that, if the preferred value is OK, take that, otherwise
// take the secondary value.
//
// The preferred bad value is used to indacate bad values
// in the output array.
//
void SoundingMerge::_mergeVal(double *preferred,
			      double *secondary,
			      double *out,
			      int prefIndex,
			      int secondIndex,
			      int outIndex,
			      double prefBad,
			      double secBad){

  
  int gotPref = 0;
  int gotSec = 0;
  out[outIndex]=prefBad; // Start off pessimistic.

  if (preferred != NULL){
    if (preferred[prefIndex] != prefBad){
      gotPref = 1;
    }
  }

  if (secondary != NULL){
    if (secondary[secondIndex] != secBad){
      gotSec = 1;
    }
  }

  if (gotPref){
    out[outIndex]=preferred[prefIndex];
    return;
  }

  if (gotSec){
    out[outIndex]=secondary[secondIndex];
  }

  return;

}
////////////////////////////////////////////////////
//
// _interpSounding interpolates any missing points in a sounding.
//
void SoundingMerge::_interpSounding(SoundingDataHolder S, 
				    double maxInterpDist){

  _interpArray(S.height, S.u, S.badVal, S.numPoints, maxInterpDist);
  _interpArray(S.height, S.v, S.badVal, S.numPoints, maxInterpDist);
  _interpArray(S.height, S.w, S.badVal, S.numPoints, maxInterpDist);
  _interpArray(S.height, S.prs, S.badVal, S.numPoints, maxInterpDist);
  _interpArray(S.height, S.relHum, S.badVal, S.numPoints, maxInterpDist);
  _interpArray(S.height, S.temp, S.badVal, S.numPoints, maxInterpDist);


}


////////////////////////////////////////////////////
//
// _interpArray does linear interpolation on y to fill
// in gaps it the array y.
//
void SoundingMerge::_interpArray(double *x,
				 double *y,
				 double badVal,
				 int numPoints,
				 double maxInterpDist){
  //
  // Return if either input is NULL.
  //
  if ((y==NULL) || (x==NULL)) return;
  //
  // return if any of x, or all of y, are bad.
  //
  int numYgood = 0;
  for (int i=0; i < numPoints; i++){
    if (x[i] == badVal) return;
    if (y[i] != badVal) numYgood++;
  }
  if (numYgood == 0) return;
  //
  // Do the interp.
  //
  for (int i=0; i < numPoints; i++){
    if (y[i]==badVal){
      //
      // Find the index of the last good point.
      // Set to -1 if there is no last good point.
      //
      int ilg = i;
      do {
	ilg--;
      } while ((ilg > -1) && (y[ilg] == badVal));
      //
      // Find the index of the next good point.
      // Set to numPoints if there is no next good point.
      //
      int ing = i;
      do {
	ing++;
      } while ((ing < numPoints) && (y[ing] == badVal));
      //
      // Three possible cases : There is no last good
      // value (ilg == -1), or there is no next good point
      // (ing == numPoints), or we are between the good
      // points ilg and ing.
      //
      if (ilg == -1){
	for (int k=0; k < ing; k++){
	  if (fabs(x[k]-x[ing]) <= maxInterpDist){
	    y[k]=y[ing];
	  }
	}
      }
      //
      if (ing == numPoints) {
	for (int k=ilg+1; k < numPoints; k++){
	  if (fabs(x[k]-x[ilg]) <= maxInterpDist){
	    y[k]=y[ilg];
	  }
	}
      }
      //
      // Interp if between two good points.
      //
      if ((ing != numPoints) && (ilg != -1)){
	for (int k=ilg+1; k < ing; k++){
	  if (
	      (fabs(x[k]-x[ilg]) <= maxInterpDist) ||
	      (fabs(x[k]-x[ing]) <= maxInterpDist)
	      ){
	    double t = (x[k]-x[ilg])/(x[ing]-x[ilg]);
	    y[k]=(1.0-t)*y[ilg] + t*y[ing];
	  }
	}
      }
    }
  }
}
//
/////////////////////////////////////////////
//
//
//
void SoundingMerge::_printSounding(SoundingDataHolder S){

  fprintf(stderr,"Sounding data for station %d at %s\n",
	  _id, utimstr(_dataTime));

  fprintf(stderr,"Lat, lon, alt : %g, %g, %g\n",
	  S.lat, S.lon, S.alt);

  for (int i=0; i < S.numPoints; i++){
    fprintf(stderr,"%d\t",i);
    fprintf(stderr,"H=%g ",S.height[i]);
    fprintf(stderr,"U=%g ",S.u[i]);
    fprintf(stderr,"V=%g ",S.v[i]);
    fprintf(stderr,"T=%g ",S.temp[i]);
    fprintf(stderr,"RH=%g ",S.relHum[i]);
    fprintf(stderr,"P=%g ",S.prs[i]);
    fprintf(stderr,"\n");
  }

  fprintf(stderr,"\n");

}
