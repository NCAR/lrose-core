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
#include <Mdv/MdvxProj.hh>
#include <cmath>
#include <physics/physics.h>

using namespace std;

//
// Constructor
//
Process::Process(){
  return;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T){


  if (P->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }


  //
  // Set up for the new data.
  //
  DsMdvx New;


  New.setDebug( P->Debug);

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  New.addReadField(P->inputFieldNameList.pFieldName);
  New.addReadField(P->inputFieldNameList.tFieldName);
  New.addReadField(P->inputFieldNameList.zFieldName);
  New.addReadField(P->inputFieldNameList.speedFieldName);
  New.addReadField(P->inputFieldNameList.dirFieldName);
  New.addReadField(P->inputFieldNameList.dewFieldName);
  New.addReadField(P->inputFieldNameList.sfcTfieldName);
  New.addReadField(P->inputFieldNameList.sfcDewFieldName);
  New.addReadField(P->inputFieldNameList.sfcSpeedFieldName);
  New.addReadField(P->inputFieldNameList.sfcDirFieldName);
  New.addReadField(P->inputFieldNameList.sfcPresFieldName);
  New.addReadField(P->inputFieldNameList.terrainFieldName);

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     

  //
  //
  Mdvx::master_header_t InMhdr = New.getMasterHeader();

  MdvxField *pField = New.getFieldByName( P->inputFieldNameList.pFieldName );
  MdvxField *tField = New.getFieldByName( P->inputFieldNameList.tFieldName );
  MdvxField *zField = New.getFieldByName( P->inputFieldNameList.zFieldName );
  MdvxField *speedField = New.getFieldByName( P->inputFieldNameList.speedFieldName );
  MdvxField *dirField = New.getFieldByName( P->inputFieldNameList.dirFieldName );
  MdvxField *dewField = New.getFieldByName( P->inputFieldNameList.dewFieldName );

  MdvxField *sfcTfield = New.getFieldByName( P->inputFieldNameList.sfcTfieldName );
  MdvxField *sfcDewField = New.getFieldByName( P->inputFieldNameList.sfcDewFieldName );
  MdvxField *sfcSpeedField = New.getFieldByName( P->inputFieldNameList.sfcSpeedFieldName );
  MdvxField *sfcDirField = New.getFieldByName( P->inputFieldNameList.sfcDirFieldName );
  MdvxField *sfcPresField = New.getFieldByName( P->inputFieldNameList.sfcPresFieldName );
  MdvxField *terrainField = New.getFieldByName( P->inputFieldNameList.terrainFieldName );

  bool gotFields = true;
  if (pField==NULL){ cerr << "Could not find pressure field." << endl; gotFields = false; }
  if (tField==NULL){ cerr << "Could not find temperature field." << endl; gotFields = false; }
  if (zField==NULL){ cerr << "Could not find height field." << endl; gotFields = false; }
  if (speedField==NULL){ cerr << "Could not find wind speed field." << endl; gotFields = false; }
  if (dirField==NULL){ cerr << "Could not find wind direction field." << endl; gotFields = false; }
  if (dewField==NULL){ cerr << "Could not find dew point field." << endl; gotFields = false; }

  if (sfcTfield==NULL){ cerr << "Could not find surface temp field." << endl; gotFields = false; }
  if (sfcDewField==NULL){ cerr << "Could not find surface dew point field." << endl; gotFields = false; }
  if (sfcSpeedField==NULL){ cerr << "Could not find surface speed field." << endl; gotFields = false; }
  if (sfcDirField==NULL){ cerr << "Could not find surface direction field." << endl; gotFields = false; }
  if (sfcPresField==NULL){ cerr << "Could not find surface pressure field." << endl; gotFields = false; }
  if (terrainField==NULL){ cerr << "Could not find terrain field." << endl; gotFields = false; }

  if (!(gotFields)){
    cerr << "Necessary fields not found, exiting." << endl;
    exit(-1);
  }

  //
  // Assume all the fields have similar vlevel headers and
  // projection information, and use the vlevel header
  // and projection information from the pressure field.
  //
  Mdvx::field_header_t pFhdr = pField->getFieldHeader();
  Mdvx::vlevel_header_t pVhdr = pField->getVlevelHeader();

  MdvxProj Proj(InMhdr, pFhdr);

  Mdvx::field_header_t tFhdr = tField->getFieldHeader();
  Mdvx::field_header_t zFhdr = zField->getFieldHeader();
  Mdvx::field_header_t speedFhdr = speedField->getFieldHeader();
  Mdvx::field_header_t dirFhdr = dirField->getFieldHeader();
  Mdvx::field_header_t dewFhdr = dewField->getFieldHeader();

  Mdvx::field_header_t sfcTFhdr = sfcTfield->getFieldHeader();
  Mdvx::field_header_t sfcDewFhdr = sfcDewField->getFieldHeader();
  Mdvx::field_header_t sfcSpeedFhdr = sfcSpeedField->getFieldHeader();
  Mdvx::field_header_t sfcDirFhdr = sfcDirField->getFieldHeader();
  Mdvx::field_header_t sfcPresFhdr = sfcPresField->getFieldHeader();

  Mdvx::field_header_t terrainFhdr = terrainField->getFieldHeader();

  fl32 *pData = (fl32 *) pField->getVol();
  fl32 *tData = (fl32 *) tField->getVol();
  fl32 *zData = (fl32 *) zField->getVol();
  fl32 *speedData = (fl32 *) speedField->getVol();
  fl32 *dirData = (fl32 *) dirField->getVol();
  fl32 *dewData = (fl32 *) dewField->getVol();

  fl32 *sfcTData = (fl32 *) sfcTfield->getVol();
  fl32 *sfcDewData = (fl32 *) sfcDewField->getVol();
  fl32 *sfcSpeedData = (fl32 *) sfcSpeedField->getVol();
  fl32 *sfcDirData = (fl32 *) sfcDirField->getVol();
  fl32 *sfcPresData = (fl32 *) sfcPresField->getVol();

  fl32 *terrainData = (fl32 *) terrainField->getVol();

  const int badVal = 99999;

  //
  // Loop through locations.
  //
  for (int ip=0; ip < P->samplePoints_n; ip++){

    int ix,iy;
    if (Proj.latlon2xyIndex(P->_samplePoints[ip].lat,
			    P->_samplePoints[ip].lon,
			    ix, iy)){
      cerr << "WARNING :  Station " << P->_samplePoints[ip].id;
      cerr << " out of region, skipping.." << endl;
      continue;
    }

    const int numFields = 7; // Type, pressure, height, temp, dewPt, dir, speed
    int numLevels = pFhdr.nz + 1 + P->mandatedLevels_n; // NZ + surface level + mandated levels.
    
    int *dataBuf = (int *)malloc(sizeof(int)*numFields*numLevels);
    if (dataBuf == NULL){
      cerr << "Malloc failed!" << endl;
      exit(-1); // HIGHLY unlikely.
    }
    
    // Initialize everything to the missing value with type 5.
    for (int ilev = 0; ilev < numLevels; ilev++){
      for (int ifld = 0; ifld < numFields; ifld++){
	if (ifld == 0){
	  dataBuf[ilev * numFields + ifld] = 5;
	} else {
	  dataBuf[ilev * numFields + ifld] = badVal;
	}
      }
    }
    
    //
    // Put in surface line.
    //
    int index = pFhdr.nx * iy + ix;
    
    int dataLineNum = 0;
    dataBuf[0] = 9; // Appropriate type for surface.

    // Pressure in tenths of millibars. Convert from SLP to station pressure if needed.
    if (P->convertSfcPres){
      if (
	  (sfcPresData[index] != sfcPresFhdr.missing_data_value) &&
	  (sfcPresData[index] != sfcPresFhdr.bad_data_value) &&
	  (terrainData[index] != terrainFhdr.missing_data_value) &&
	  (terrainData[index] != terrainFhdr.bad_data_value)
	  ){
	double slPres = (sfcPresData[index] + P->inputFieldNameList.sfcPresBias)*P->inputFieldNameList.sfcPresScale;
	double h = ((terrainData[index] + P->inputFieldNameList.terrainBias)*P->inputFieldNameList.terrainScale) / 1000.0; 
	double stationPres = slPres * exp(-0.119*h - 0.0013*h*h);      
	dataBuf[1] = (int)rint(stationPres*10.0); // Input units are hPa
      }
    } else {
      if (
	  (sfcPresData[index] != sfcPresFhdr.missing_data_value) &&
	  (sfcPresData[index] != sfcPresFhdr.bad_data_value)
	  ){
	dataBuf[1] = (int)rint(10.0*((sfcPresData[index] + P->inputFieldNameList.sfcPresBias)*P->inputFieldNameList.sfcPresScale)); // Input units are hPa
      }
    }
  
    // Height in meters
    if ((terrainData[index] != terrainFhdr.missing_data_value) &&
	(terrainData[index] != terrainFhdr.bad_data_value)){
      dataBuf[2] = (int)rint((terrainData[index] + P->inputFieldNameList.terrainBias)*P->inputFieldNameList.terrainScale);
    }
    
    // Temperature, tenths of degrees C
    if ((sfcTData[index] != sfcTFhdr.missing_data_value) && (sfcTData[index] != sfcTFhdr.bad_data_value)){
      dataBuf[3] = (int)rint(10.0*((sfcTData[index] + P->inputFieldNameList.sfcTbias)*P->inputFieldNameList.sfcTscale));
    }
    
    // Dew point, tenths of deg C, must be less than temperature.
    if ((sfcDewData[index] != sfcDewFhdr.missing_data_value) && (sfcDewData[index] != sfcDewFhdr.bad_data_value)){
      dataBuf[4] = (int)rint(10.0*((sfcDewData[index] + P->inputFieldNameList.sfcDewBias)*P->inputFieldNameList.sfcDewScale));
      if (dataBuf[4] > dataBuf[3]) dataBuf[4]=dataBuf[3];
    }
    
    // Wind dir, degrees
    if ((sfcDirData[index] != sfcDirFhdr.missing_data_value) && (sfcDirData[index] != sfcDirFhdr.bad_data_value)){
      double sfcWdir = sfcDirData[index];
      if (sfcWdir < 0) sfcWdir += 360.0;
      dataBuf[5] = (int)rint(sfcWdir);
    }
    
    // Wind speed, knots
    if ((sfcSpeedData[index] != sfcSpeedFhdr.missing_data_value) && (sfcSpeedData[index] != sfcSpeedFhdr.bad_data_value)){
      dataBuf[6] = (int)rint(((sfcSpeedData[index] + P->inputFieldNameList.sfcSpeedBias)*P->inputFieldNameList.sfcSpeedScale)/0.5145); // Convert m/s to knots   
    }

    if ((P->sfcDewPtIsRH) && (dataBuf[4] != badVal) && (dataBuf[3] != badVal)){
      dataBuf[4] = (int)rint(10.0*PHYrhdp(dataBuf[3]/10.0, dataBuf[4]/10.0));
    }


    dataLineNum++;

    //
    // Now figure out the max, min pressures. We need these to determine where
    // manditory levels are outside of the range.
    //
    double minPres = -1.0; double maxPres = -1.0;
    for (int iz=0; iz < pFhdr.nz; iz++){
      index = iz*pFhdr.nx*pFhdr.ny + pFhdr.nx * iy + ix;
      if ((pData[index] != pFhdr.missing_data_value) && (pData[index] != pFhdr.bad_data_value)){
	if (minPres < 0.0){
	  minPres = pData[index];
	  maxPres = minPres;
	} else {
	  if (pData[index] < minPres) minPres = pData[index];
	  if (pData[index] > maxPres) maxPres = pData[index];
	}
      }
    }

    if (P->Debug){
      cerr << "Pressures aloft run from " << minPres << " to " << maxPres << endl;
    }

    //
    // Insert blank manditory pressure lines for pressures above the max pressure.
    //
    for (int im = 0; im < P->mandatedLevels_n; im++){
      if (P->_mandatedLevels[im] > maxPres){
	dataBuf[numFields * dataLineNum] = 4; // Code for manditory level
	dataBuf[numFields * dataLineNum + 1] = (int)rint(P->_mandatedLevels[im]*10.0);
	dataLineNum++;
      }
    }

    //
    // Loop through the data we have, putting it in the array, inserting manditory levels
    // in the right spots.
    //

    for (int iz=0; iz < pFhdr.nz; iz++){

      index = iz*pFhdr.nx*pFhdr.ny + pFhdr.nx * iy + ix;

      //
      // If we have no data here, skip it.
      //
      if (
          ((pData[index] == pFhdr.missing_data_value) || (pData[index] == pFhdr.missing_data_value)) &&
          ((zData[index] == zFhdr.missing_data_value) || (zData[index] == zFhdr.missing_data_value)) &&
          ((tData[index] == tFhdr.missing_data_value) || (tData[index] == tFhdr.missing_data_value)) &&
          ((dewData[index] == dewFhdr.missing_data_value) || (dewData[index] == dewFhdr.missing_data_value)) &&
          ((dirData[index] == dirFhdr.missing_data_value) || (dirData[index] == dirFhdr.missing_data_value)) &&
          ((speedData[index] == speedFhdr.missing_data_value) || (speedData[index] == speedFhdr.missing_data_value))
          ){
        continue;
      }

      
      // Pressure in tenths of millibars
      if ((pData[index] != pFhdr.missing_data_value) && (pData[index] != pFhdr.bad_data_value)){
	dataBuf[numFields * dataLineNum + 1] = (int)rint(10.0*((pData[index] + P->inputFieldNameList.pBias)*P->inputFieldNameList.pScale));
      }

      // Height in meters
      if ((zData[index] != zFhdr.missing_data_value) && (zData[index] != zFhdr.bad_data_value)){
	dataBuf[numFields * dataLineNum + 2] = (int)rint((zData[index] + P->inputFieldNameList.zBias)*P->inputFieldNameList.zScale);
      }

      // Temperature, tenths of degrees C
      if ((tData[index] != tFhdr.missing_data_value) && (tData[index] != tFhdr.bad_data_value)){
	dataBuf[numFields * dataLineNum + 3] = (int)rint(10.0*((tData[index] + P->inputFieldNameList.tBias)*P->inputFieldNameList.tScale));
      }

      // Dew point, tenths of deg C, must be less than temperature.
      if ((dewData[index] != dewFhdr.missing_data_value) && (dewData[index] != dewFhdr.bad_data_value)){
	double dewPoint = (dewData[index] + P->inputFieldNameList.dewBias)*P->inputFieldNameList.dewScale;
	if ((tData[index] != tFhdr.missing_data_value) && (tData[index] != tFhdr.bad_data_value)){
	  double temp = (tData[index] + P->inputFieldNameList.tBias)*P->inputFieldNameList.tScale;
	  if (dewPoint > temp) dewPoint = temp;
	  dataBuf[numFields * dataLineNum + 4] = (int)rint(dewPoint*10.0);
	}
      }

      // Wind dir, degrees
      if ((dirData[index] != dirFhdr.missing_data_value) && (dirData[index] != dirFhdr.bad_data_value)){
	double wdir = dirData[index];
	if (wdir < 0) wdir += 360.0;
	dataBuf[numFields * dataLineNum + 5] = (int)rint(wdir);
      }

      // Wind speed, knots
      if ((speedData[index] != speedFhdr.missing_data_value) && (speedData[index] != speedFhdr.bad_data_value)){
	dataBuf[numFields * dataLineNum + 6] = (int)rint(((speedData[index] + P->inputFieldNameList.speedBias)*P->inputFieldNameList.speedScale)/0.5145); // Convert m/s to knots
      }

      if ((P->aloftDewPtIsRH) && (dataBuf[numFields * dataLineNum + 4] != badVal) && (dataBuf[numFields * dataLineNum + 3] != badVal)){
	dataBuf[numFields * dataLineNum + 4] = (int)rint(10.0*PHYrhdp(dataBuf[numFields * dataLineNum + 3]/10.0, dataBuf[numFields * dataLineNum + 4]/10.0));
      }



      dataLineNum++;

      //
      // OK - If we had a good pressure, we need to look at the next good pressure.
      // If the next good pressure and this pressure bracket a
      // manditory pressure level, we need to put that pressure level in.
      //
      if ((pData[index] != pFhdr.missing_data_value) && (pData[index] != pFhdr.bad_data_value)){
	double thisPressure = (pData[index] + P->inputFieldNameList.pBias)*P->inputFieldNameList.pScale;
	double nextPressure = 0;
	if (iz < pFhdr.nz-1){
	  bool haveNextGoodP = false;
	  int izz = iz + 1;
	  do {
	    int nextIndex = izz*pFhdr.nx*pFhdr.ny + pFhdr.nx * iy + ix;
	    if ((pData[nextIndex] != pFhdr.missing_data_value) && (pData[nextIndex] != pFhdr.bad_data_value)){
	      nextPressure = (pData[nextIndex] + P->inputFieldNameList.pBias)*P->inputFieldNameList.pScale;
	      haveNextGoodP = true;
	      break;
	    }
	  } while(izz < pFhdr.nz);

	  if (haveNextGoodP){
	    int iThisP = (int)rint(thisPressure*10.0);
	    int iNextP = (int)rint(nextPressure*10.0);
	    for (int ip = 0; ip < P->mandatedLevels_n; ip++){
	      int iMandatedP = (int)rint(P->_mandatedLevels[ip]*10.0);
	      if ((iMandatedP > iNextP) && (iMandatedP < iThisP)) {
		//
		// We need to insert a manditory level here.
		//
		dataBuf[numFields * dataLineNum] = 4; // Code for manditory level.
		dataBuf[numFields * dataLineNum + 1] = iMandatedP;
		dataLineNum++;
	      }
	    }
	  }
	}
      }
    }


    //
    // Insert blank manditory pressure lines for pressures below the min pressure.
    //
    for (int im = 0; im < P->mandatedLevels_n; im++){
      if (P->_mandatedLevels[im] < minPres){
	dataBuf[numFields * dataLineNum] = 4; // Code for manditory level
	dataBuf[numFields * dataLineNum + 1] = (int)rint(P->_mandatedLevels[im]*10.0);
	dataLineNum++;
      }
    }


    //
    // The array is now full. Go through, and if any of the type 5 data have
    // happened to be on a manditory level, change it to type 4.
    //
    for (int dl=0; dl < dataLineNum; dl++){
      if (dataBuf[numFields * dataLineNum] == 5){
	for (int im = 0; im < P->mandatedLevels_n; im++){
	  if (dataBuf[numFields * dataLineNum+1] == (int)rint(P->_mandatedLevels[im]*10.0)){
	    dataBuf[numFields * dataLineNum] = 4;
	    break;
	  }
	}
      }
    }

    if (P->mandatedInterpolate){
      //
      // Interpolate over those type entries that are between two type 5 entries.
      // If we have two type 4 entries in a row, that's a bit much to interpolate.
      //
      for (int dl=1; dl < dataLineNum-1; dl++){
	if (
	    ((dataBuf[numFields * (dl-1)] == 5) || (dataBuf[numFields * (dl-1)] == 4)) && 
	    (dataBuf[numFields * dl] == 4) && 
	    ((dataBuf[numFields * (dl+1)] == 5) || (dataBuf[numFields * (dl+1)] == 4))
	    ){
	  
	  double t = double(dataBuf[numFields * dl + 1]- dataBuf[numFields * (dl+1) + 1]) / 
	    double(dataBuf[numFields * (dl-1) + 1] - dataBuf[numFields * (dl+1) + 1]);
	  
	  for (int ifld = 2; ifld < numFields; ifld++){
	    if (
		(dataBuf[numFields * (dl-1) + ifld] != 99999) &&
		(dataBuf[numFields * dl + ifld] == 99999) &&
		(dataBuf[numFields * (dl+1) + ifld] != 99999)
		){
	      
	      int a = 0;
	      int b = 0;
	      if (ifld == 5){
		//
		// Wind direction. Can't just interpolate, need to check if the 360 mark
		// is passed.
		//
		if (
		    (dataBuf[numFields * (dl-1) + ifld] < 180) && (dataBuf[numFields * (dl+1) + ifld] > 180) ||
		    (dataBuf[numFields * (dl-1) + ifld] > 180) && (dataBuf[numFields * (dl+1) + ifld] < 180)
		    ){
		  if (dataBuf[numFields * (dl-1) + ifld] < 180) a = 360;
		  if (dataBuf[numFields * (dl+1) + ifld] < 180) b = 360;
		}
		
	      }
	      
	      dataBuf[numFields * dl + ifld] = (int)rint(t*(a + dataBuf[numFields * (dl-1) + ifld]) + 
							 (1.0-t)*(b + dataBuf[numFields * (dl+1) + ifld]));

	      if (ifld == 5){
		if (dataBuf[numFields * dl + ifld] > 360) dataBuf[numFields * dl + ifld] -= 360;
	      }
	      
	    }
	  }
	}
      }
    }

    char lineTerm[8];
    sprintf(lineTerm, "%s", "\n");
    if (P->windowsEndOfLine) sprintf(lineTerm, "%s", "\r\n");
    
    //
    // Open the file and write it.
    //
    if (ta_makedir_recurse( P->OutDir )){
      cerr << "Failed to make directory " << P->OutDir << endl;
      exit(-1);
    }
    
    char outfileName[1024];
    date_time_t dataTime;
    dataTime.unix_time = InMhdr.time_centroid;
    uconvert_from_utime( &dataTime );
    
    sprintf(outfileName, "%s/%s_%04d%02d%02d_%02d%02d%02d.raob",
	    P->OutDir, P->_samplePoints[ip].id, dataTime.year,
	    dataTime.month, dataTime.day, dataTime.hour, dataTime.min, dataTime.sec);
    
    FILE *fp = fopen(outfileName, "w");
    if (fp == NULL){
      cerr << "Failed to create " << outfileName << ", skipping.." << endl;
      exit(-1);
    }

    if (P->Debug){
      cerr << "Writing " << outfileName << endl;
    }
    //
    // Output the first header lines.
    //
    fprintf(fp, "RUC2 analysis valid for grid point 0 nm / 0 deg from %s:%s",
	    P->_samplePoints[ip].id, lineTerm);
    
    char hourStr[16]; char dayStr[16];
    sprintf(hourStr, "%02d", dataTime.hour);
    sprintf(dayStr, "%02d", dataTime.day);
    
    fprintf(fp, "RUC2   %7s%7s%9s%8d%s", hourStr, dayStr, _monStr( dataTime.month ), dataTime.year, lineTerm);
    
    fprintf(fp, "   CAPE      0    CIN      0  Helic      0     PW      4%s", lineTerm);
    fprintf(fp, "%7d%7d%7d%7.2f%7.2f%7d  99999%s", 1,
	    P->_samplePoints[ip].wbanNum, P->_samplePoints[ip].wmoNum,
	    P->_samplePoints[ip].lat, -1.0*P->_samplePoints[ip].lon,
	    (int)rint(zData[ix + pFhdr.nx * iy]), lineTerm);
    fprintf(fp, "%7d  99999  99999  99999%7d  99999  99999%s", 2, dataLineNum + 4, lineTerm); // Count the first four header lines.
    fprintf(fp, "%7d%7s%7s%7s%7s%7d%7s%s", 3, " ",
	    P->_samplePoints[ip].id, " ", " ", 12, "kt", lineTerm );

    //
    // Now print out the actual data
    //
    for (int dl=0; dl < dataLineNum; dl++){
      for (int ifld = 0; ifld < numFields; ifld++){
	fprintf(fp,"%7d", dataBuf[dl * numFields + ifld]);
      }
      fprintf(fp,"%s", lineTerm);
    }

    fprintf(fp, "%s", lineTerm); // Trailing blank line seems to be needed

    fclose(fp);
    free(dataBuf);

  }

  if (P->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  return;
}


char *Process::_monStr(int mon){

  if (mon == 1) return "Jan";
  if (mon == 2) return "Feb";
  if (mon == 3) return "Mar";
  if (mon == 4) return "Apr";
  if (mon == 5) return "May";
  if (mon == 6) return "Jun";
  if (mon == 7) return "Jul";
  if (mon == 8) return "Aug";
  if (mon == 9) return "Sep";
  if (mon == 10) return "Oct";
  if (mon == 11) return "Nov";
  if (mon == 12) return "Dec";
  return "UNKNOWN MONTH";

}








