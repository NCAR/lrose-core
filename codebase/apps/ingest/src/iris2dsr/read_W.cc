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


#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <dataport/bigend.h>
#include <dataport/swap.h>

#include "read_W.hh"
#include "aBeam.hh"

using namespace std;


//
// Constructor. Loads the file, gets beam data in
// memory as floats.
//
read_W::read_W(Params *P, char *baseName, int tiltNum, double nyquistVel,
	       double origGateSpacingKm, double origFirstGateKm){

  _nyquistVel = nyquistVel;
  _allBeams.clear();

  //
  // Construe the filename for this tilt.
  //
  char fullFilename[MAX_PATH_LEN];

  if (baseName[strlen(baseName)-1] == '.'){
    sprintf(fullFilename,"%s/%s%02dW", P->inputDir, baseName, tiltNum+1);
  } else {
    sprintf(fullFilename,"%s/%s.%02dW", P->inputDir, baseName, tiltNum+1);
  }


  FILE *fp;
  int fcount = 0;
  do {
    fp = fopen(fullFilename, "r");
    if (fp != NULL) break;
    if (P->mode == Params::ARCHIVE) return;

    if (P->debug){
      cerr << "Could not open " << fullFilename << " at " << utimstr(time(NULL)) << endl;
      perror("The reason given");
    }

    fcount++;
    for (int q=0; q < P->endOfVolTimeout.sleepBetweenTries; q++){
      sleep(1);
      PMU_auto_register("Waiting for input data");
    }
  } while (fcount < P->endOfVolTimeout.numTries);

  if (fp == NULL){
    cerr << "Failed to open " << fullFilename << endl;
    return;
  }

  fseek(fp, 30, SEEK_SET);
  si16 numRaysExpected;
  fread(&numRaysExpected, sizeof(si16), 1, fp);
  if (P->byteSwap) SWAP_array_16((ui16 *) &numRaysExpected, 2);

  fseek(fp, 36, SEEK_SET);
  si16 bitsPerBin;
  fread(&bitsPerBin, sizeof(si16), 1, fp);
  if (P->byteSwap) SWAP_array_16((ui16 *) &bitsPerBin, 2);

  if (P->debug){
    cerr << "In " << fullFilename;
    cerr << " there are " << bitsPerBin << " bits per bin." << endl;
    cerr << "Number of rays expected : " << numRaysExpected << endl;
  }


  vector <int> fileOffsets;

  fseek(fp, 76, SEEK_SET);
  for (int i=0; i < numRaysExpected; i++){
    ui32 pointer;
    fread(&pointer, sizeof(ui32), 1, fp);
    if (P->byteSwap) SWAP_array_32((ui32 *) &pointer, 4);

    int fileOffset = 0;
    if (pointer)
      fileOffset = 75 + 4*numRaysExpected + pointer;

    fileOffsets.push_back(fileOffset);
  }




  for (unsigned i=0; i < fileOffsets.size(); i++){

    aBeam beam;
    beam.az = MISSING;
    beam.el = MISSING;
    beam.beamValues.clear();

    if (!(fileOffsets[i])){
      _allBeams.push_back( beam );
      continue;
    }

    fseek(fp, fileOffsets[i], SEEK_SET);

    si16 azStart;
    fread(&azStart, sizeof(si16), 1, fp);
    if (P->byteSwap) SWAP_array_16((ui16 *) &azStart, 2);

    si16 elStart;
    fread(&elStart, sizeof(si16), 1, fp);
    if (P->byteSwap) SWAP_array_16((ui16 *) &elStart, 2);
    
    si16 azEnd;
    fread(&azEnd, sizeof(si16), 1, fp);
    if (P->byteSwap) SWAP_array_16((ui16 *) &azEnd, 2);

    si16 elEnd;
    fread(&elEnd, sizeof(si16), 1, fp);
    if (P->byteSwap) SWAP_array_16((ui16 *) &elEnd, 2);

    si16 nBins;
    fread(&nBins, sizeof(si16), 1, fp);
    if (P->byteSwap) SWAP_array_16((ui16 *) &nBins, 2);

    double el1 = _getAngle(elStart);
    double az1 = _getAngle(azStart);
    double el2 = _getAngle(elEnd);
    double az2 = _getAngle(azEnd);

    double pi = acos(-1.0);
    double cosEl1 = cos( pi * el1 / 180.0);
    double sinEl1 = sin( pi * el1 / 180.0);
    double cosEl2 = cos( pi * el2 / 180.0);
    double sinEl2 = sin( pi * el2 / 180.0);

    double cosAz1 = cos( pi * az1 / 180.0);
    double sinAz1 = sin( pi * az1 / 180.0);
    double cosAz2 = cos( pi * az2 / 180.0);
    double sinAz2 = sin( pi * az2 / 180.0);

    double cosEl = (cosEl1 + cosEl2)/2.0;
    double sinEl = (sinEl1 + sinEl2)/2.0;

    double cosAz = (cosAz1 + cosAz2)/2.0;
    double sinAz = (sinAz1 + sinAz2)/2.0;

    double el = 180.0*atan2(sinEl, cosEl)/pi;
    double az = 180.0*atan2(sinAz, cosAz)/pi;

    //if (P->debug){
    //  cerr << "Beam " << i+1 << " at " << az << ", " << el;
    //  cerr << " with " << nBins << " bins." << endl;
    //}

    beam.az = az; beam.el = el;
    
    fseek(fp, 2, SEEK_CUR); // OK, now pointing at the start of data.

    for (int i=0; i < nBins; i++){
      fl32 wVal = MISSING;
      ui08 bte;
      ui16 word;
      switch (bitsPerBin){

      case 8 : // This is all I have at time of writing - niles
	fread(&bte, sizeof(bte), 1, fp);
	if (
	    (bte != 0) && 
	    (bte != 255)){ // These being bad data.
	  wVal = double(bte)/256.0;
	  wVal = wVal * _nyquistVel;
	}
	break;

      case 16 :
	fread(&word, sizeof(ui16), 1, fp);
	if (P->byteSwap) SWAP_array_16((ui16 *) &word, 2);
	if ((word !=0) && (word != 65535)){ // These being bad data.
	  wVal = double(word)/100.0;
	}

	break;

      default :
	cerr << "Unsupported number of bits per bin : ";
	cerr << bitsPerBin << endl;
	exit(-1);
	break;

      }
      // cerr << "W at gate " << i+1 << " is " << wVal << endl;
      beam.beamValues.push_back( wVal );
    }

    if (P->resample.doResample){

      aBeam rBeam; // Resampled beam
      rBeam.az = beam.az;
      rBeam.el = beam.el;
      rBeam.beamValues.clear();

      double dist = P->resample.firstGateKm;

      do {

	int oldIndex = (int)rint( (dist - origFirstGateKm)/origGateSpacingKm );

	if ((oldIndex < 0) || (oldIndex > (int) beam.beamValues.size()-1)){
	  rBeam.beamValues.push_back(MISSING);
	} else {
	  rBeam.beamValues.push_back( beam.beamValues[oldIndex]); 
	}

	dist +=  P->resample.incKm;
      } while (dist <= P->resample.lastGateKm);

      _allBeams.push_back( rBeam );

    } else {
      //
      // Not resampling, save beam as is.
      //
      _allBeams.push_back( beam );
    }


  }


  fclose(fp);


  return;
}

int read_W::getNBeams(){
  return _allBeams.size();
}

aBeam read_W::getBeam(int beamNum ){
  return _allBeams[beamNum];
}

double read_W::_getAngle(si16 bin2val){
  return 180.0*double(bin2val)/32768.0;
}



//
// Destructor. Does little, but avoids default destructor.
//
read_W::~read_W(){

  for (unsigned i=0; i < _allBeams.size(); i++){
    _allBeams[i].beamValues.clear();
  }

  _allBeams.clear();

  return;
}



