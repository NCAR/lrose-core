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


#include "Process.hh"

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/file_io.h>
#include <Mdv/MdvxProj.hh>
#include <physics/physics.h>

using namespace std;

//
// Constructor, Destructor
//
Process::Process(Params *P){
  //
  // Make a copy of input parameters.
  //
  _params = P;

  //
  // Allocate space for the bins.
  // Use calloc to zero out the memory.
  //
  _dirHist = (double *) calloc(_params->numDirectionBins * _params->locations_n, sizeof(double));
  _weightedDirHist = (double *) calloc(_params->numDirectionBins * _params->locations_n, sizeof(double));
  _speedHist = (double *) calloc(_params->numSpeedBins * _params->locations_n, sizeof(double));

  _numEntries = (int *) calloc(_params->locations_n, sizeof(int));
  _numEntries2 = (int *) calloc(_params->locations_n * _params->numDirectionBins, sizeof(int));

  return;
}

Process::~Process(){

  free(_dirHist);
  free(_speedHist);
  free(_weightedDirHist);
  free(_numEntries);
  free(_numEntries2);

  return;
}


////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(time_t T){

  if (_params->Debug){
    cerr << "Processing at " << utimstr(T) << endl;
  }

  //
  // Set up for the new data.
  //
  DsMdvx New;


  New.setReadTime(Mdvx::READ_FIRST_BEFORE, _params->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  New.addReadField( _params->UfieldName );
  New.addReadField( _params->VfieldName );

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << _params->TriggerUrl  << endl;
    return -1;
  }     

  //
  // Get the U and V fields
  //
  MdvxField *UField = New.getFieldByName( _params->UfieldName );
  if (UField == NULL){
    cerr << "Unable to find U field " << _params->UfieldName << endl;
    return -1;
  }
  Mdvx::field_header_t UFhdr = UField->getFieldHeader();


  MdvxField *VField = New.getFieldByName( _params->VfieldName );
  if (VField == NULL){
    cerr << "Unable to find V field " << _params->VfieldName << endl;
    return -1;
  }
  Mdvx::field_header_t VFhdr = VField->getFieldHeader();

  //
  // Check that they are 2D fields - later this may change.
  //
  if ((UFhdr.nz !=1) || (VFhdr.nz != 1)){
    cerr << "3D data encountered - I cannot cope." << endl;
    return -1;
  }

  //
  // Set up a Proj object - I do this on the U field
  // and assume the V field is on the same projection.
  //
  Mdvx::master_header_t Mhdr = New.getMasterHeader();
  MdvxProj Proj(Mhdr, UFhdr);


  fl32 *Udata = (fl32 *) UField->getVol();
  fl32 *Vdata = (fl32 *) VField->getVol();


  //
  // Loop through the locations.
  //
  for (int il=0; il < _params->locations_n; il++){
    //
    // Get the indicies of the central lat/lon.
    //
    int ixc, iyc;
    if (Proj.latlon2xyIndex(_params->_locations[il].lat,
			    _params->_locations[il].lon,
			    ixc, iyc)){
      cerr << "Location " << _params->_locations[il].name << " is outside of grid." << endl;
      continue;
    }

    //
    // Loop through, do the average.
    //
    int numFound = 0;
    double totalU = 0.0;
    double totalV = 0.0;

    for (int ix = ixc - _params->_locations[il].numPointsPlusMinus;
	 ix < ixc + _params->_locations[il].numPointsPlusMinus + 1;
	 ix++){

      for (int iy = iyc - _params->_locations[il].numPointsPlusMinus;
	   iy < iyc + _params->_locations[il].numPointsPlusMinus + 1;
	   iy++){
    

	if ((iy > -1) && (ix > -1) && (iy < UFhdr.ny) && (ix < UFhdr.nx)){

	  int index = iy * UFhdr.nx + ix;

	  if (
	      ( Udata[index] != UFhdr.bad_data_value) &&
	      ( Udata[index] != UFhdr.missing_data_value) &&
	      ( Vdata[index] != VFhdr.bad_data_value) &&
	      ( Vdata[index] != VFhdr.missing_data_value)
	      ){

	    numFound++;
	    totalU += Udata[index];
	    totalV += Vdata[index];

	  }
	}
      }
    }

    if ((numFound == 0) || (numFound < _params->_locations[il].minNumGoodPoints)){
      continue;
    }

    double meanU = totalU / double(numFound);
    double meanV = totalV / double(numFound);

    //
    // Get speed and direction.
    //
    double speed =  PHYwind_speed( meanU, meanV );
    double dir =    PHYwind_dir( meanU, meanV );    // 0 to 360

    _insertIntoMatrix(speed, dir, il);

  }


  return 0;

 
}

void Process::_insertIntoMatrix(double speed, double dir, int locationIndex){

  if (_params->Debug){
    cerr << "Inserting speed=" << speed;
    cerr << " dir=" << dir;
    cerr << " at " << _params->_locations[locationIndex].name << endl;
  }

  //
  _numEntries[locationIndex]++;
  //
  // Get the directional index.
  //
  double dirStep = 360.0 / _params->numDirectionBins;
  int dirIndex = (int)floor(dir/dirStep);
  //
  // Increment the direction histogram, add the speed to the weighted one.
  //
  _dirHist[dirIndex * _params->locations_n + locationIndex] += 1.0;
  _weightedDirHist[dirIndex * _params->locations_n + locationIndex] += speed;
  _numEntries2[dirIndex * _params->locations_n + locationIndex]++;
  //
  // Similar for speed.
  //
  double maxSpeed = _params->speedBinSize * _params->numSpeedBins;
  if (speed >= maxSpeed) return;

  int speedIndex = (int)floor(speed/_params->speedBinSize);
  _speedHist[speedIndex * _params->locations_n + locationIndex] += 1.0;

  return;
}

void Process::writeOutput(){

  //
  // Make output directory.
  //
  if (ta_makedir_recurse(_params->OutDir)){
    cerr << "Failed to make directory " << _params->OutDir << endl;
  }

  //
  // Normalize the arrays.
  //
  for (int il=0; il < _params->locations_n; il++){
    
    if (_numEntries[il] == 0) continue;

    for (int id = 0; id < _params->numDirectionBins; id++){

      _dirHist[id * _params->locations_n + il] = 
	_dirHist[id * _params->locations_n + il] / double(_numEntries[il]);

      if (_numEntries2[id *_params->locations_n +il] > 0){
	_weightedDirHist[id * _params->locations_n + il] = 
	  _weightedDirHist[id * _params->locations_n + il] / double(_numEntries2[id *_params->locations_n +il]);
      }
    }

    for (int is = 0; is < _params->numSpeedBins; is++){
      _speedHist[is * _params->locations_n + il] =
	_speedHist[is * _params->locations_n + il] / double(_numEntries[il]);
    }
  }


  for (int il=0; il < _params->locations_n; il++){

    //
    // Directional histogram first.
    //
    char outFile[1024];
    sprintf(outFile,"%s/%s.dir", _params->OutDir, _params->_locations[il].name);

    FILE *fp = fopen(outFile, "w");
    if (fp == NULL){
      cerr << "Failed to create " << outFile << endl;
      exit(-1);
    }

    double dirStep = 360.0 / _params->numDirectionBins;

    for (int id = 0; id < _params->numDirectionBins; id++){
      fprintf(fp,"%g %g %g\n",
	      id * dirStep,
	      (id + 1) * dirStep,
	      _dirHist[id * _params->locations_n + il]);

    }

    fclose(fp);

    //
    // Then the weighted histogram.
    //
    sprintf(outFile,"%s/%s.dirWeighted", _params->OutDir, _params->_locations[il].name);

    fp = fopen(outFile, "w");
    if (fp == NULL){
      cerr << "Failed to create " << outFile << endl;
      exit(-1);
    }

    for (int id = 0; id < _params->numDirectionBins; id++){
      fprintf(fp,"%g %g %g\n",
	      id * dirStep,
	      (id + 1) * dirStep,
	      _weightedDirHist[id * _params->locations_n + il]);

    }

    fclose(fp);

    //
    // Finally, the speed histogram.
    //
    sprintf(outFile,"%s/%s.speed", _params->OutDir, _params->_locations[il].name);

    fp = fopen(outFile, "w");
    if (fp == NULL){
      cerr << "Failed to create " << outFile << endl;
      exit(-1);
    }


    for (int is = 0; is < _params->numSpeedBins; is++){
      fprintf(fp,"%g %g %g\n",
	      is * _params->speedBinSize,
	      (is + 1) * _params->speedBinSize,
	      _speedHist[is * _params->locations_n + il]);

    }

    fclose(fp);




  }


  return;
}








