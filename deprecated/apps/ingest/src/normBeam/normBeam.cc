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

#include <math.h>
#include <stdlib.h>
#include <cstdio>

//
// Small routine to perpetrate normalization on a lidar beam
// according to ITT's specifications. Niles Oien December 2006.
//
// Returns 0 if OK, -1 if error.
//

static int compare(const void *v1, const void *v2);

int normBeam(float *oldData,       // Pointer to the input beam.
	     float *newData,       // Pointer to the (already allocated) space for the new beam.
	     double gateSpacing,   // Gate spacing, meters.
	     double startRange,    // Start range, Km
	     double endRange,      // End range, Km
	     int percentToTake,    // Percent to take, percent.
	     double displayOffset, // Display offset
	     int numGatesPerBeam,  // Number of gates in a beam.
	     int debug,            // 0 => silent, 1 => some messages, 2 => lots of messages on stderr
	     double minVal,        // Lower limit on useful data values.
	     double maxVal,        // Upper limit on useful data values.
	     double firstGateRange,  // Range to first gate, Km.
	     double minPercentGood){ // Min percent good over normalization range
  //
  // Figure out what gates the start and end ranges correspond to.
  //
  int startGate = (int)rint((startRange - firstGateRange)*1000.0/gateSpacing);
  if (startGate < 0) startGate = 0;
  if (startGate > numGatesPerBeam-1){
    if (debug) fprintf(stderr, "normBeam : Start range is beyond total range, aborting\n");
    return -1;
  }

  int endGate = (int)rint((endRange-firstGateRange)*1000.0/gateSpacing);
  if (endGate > numGatesPerBeam-1) endGate = numGatesPerBeam-1;
  if (endGate < 0) {
    if (debug) fprintf(stderr, "normBeam : End range is negative, aborting\n");
    return -1;
  }
  
  if (endGate < startGate) {
    if (debug) fprintf(stderr, "normBeam : nonsensical start end end ranges specified, aborting.\n");
    return -1;
  }

  int numPossibleSamples = endGate - startGate +1;

  if (debug) fprintf(stderr, "normBeam : Start, end gates for ranges %g Km to %g Km : %d to %d (%d possible samples)\n",
		     startRange, endRange, startGate, endGate, numPossibleSamples);


  float *samples = (float *)malloc(sizeof(float)*numPossibleSamples);
  if (samples == NULL){
    if (debug) fprintf(stderr, "normBeam : Malloc failure, aborting.\n");
    return -1;
  }

  //
  // See how many samples we actually have - do not consider missing values.
  //
  int j=0;
  int numNonMissingFound = 0;
  for (int i=startGate; i < endGate+1; i++){
    if ((oldData[i] < minVal) || (oldData[i] > maxVal)) continue;
    samples[j] = oldData[i]; 
    j++;
    numNonMissingFound++;
  }

  double percentGood = 100.0*double(numNonMissingFound)/double(endGate - startGate +1);

  if (debug) fprintf(stderr, "normBeam : %d of %d samples were non-missing (%g percent good).\n",
		     numNonMissingFound, numPossibleSamples, percentGood);

  if (percentGood < minPercentGood){
    //
    // In this instance we mark the whole beam as missing.
    // The way things are set up, it's likely that the lidar data have issues.
    //
    for (int i=0; i < numGatesPerBeam; i++){
      newData[i] = -9999;
    }
    fprintf(stderr,"Beam set to missing due to only having %g percent good point in normalization range.\n",
	    percentGood);

    return -1;
  }


  if (numNonMissingFound == 0){
    //
    // Although there may be non-missing data in this beam
    // outside of the start and end range specified for normalization,
    // we can't normalize, so we fail. This is a problem for
    // data sets that have missing data (the lidar, in fact, does not).
    //
    free(samples);
    return -1;
  }


  //
  // Sort the samples into ascending order.
  //
  qsort(samples, numNonMissingFound, sizeof(float), compare);

  if (debug > 1){
    for(int i=0; i < numNonMissingFound; i++){
      fprintf(stderr, "normBeam : sorted sample number %d is %g\n", i+1, samples[i]);
    }
  }

  //
  // Compute the average of the lower percentToTake sorted sample values. Reject missing values.
  //
  int numToTake = (int)rint(double(numNonMissingFound)*double(percentToTake)/100.0);

  if (debug) fprintf(stderr, "normBeam : Taking lower %d percent of %d sorted non-missing samples (total of %d considered)\n",
		    percentToTake, numNonMissingFound, numToTake);

  if (numToTake < 1){
    free(samples);
    return -1; // Unlikely.
  }

  float total = 0.0;
  int numAdded = 0;

  for (int i=0; i < numToTake; i++){
    total += samples[i]; numAdded++;
    if (debug > 1) fprintf(stderr,"normBeam : Sample value %g brings total to %g\n",
			     samples[i], total);
  }

  float mean=0.0;
  if (numAdded){
    mean = total / float(numAdded);
  } else {
    //
    // No non-missing data found.
    //
    free(samples);
    return -1;
    if (debug) fprintf(stderr, "normBeam : No non-missing data found for normalization, returning input beam.\n");
    return 0;
  }
  if (debug) fprintf(stderr,"normBeam : The mean of selected samples is %g\n", mean);

  //
  // Use this mean to generate the output filtered beam.
  //
  for (int i=0; i < numGatesPerBeam; i++){
    newData[i] = oldData[i] - mean + displayOffset;
  }

  free(samples);

  return 0;
}

 static int compare(const void *v1, const void *v2) {
 
   float *f1 = (float *) v1;
   float *f2 = (float *) v2;
   
   if (*f1 < *f2) return -1;
   if (*f1 == *f2) return 0;
   return 1;

 }

