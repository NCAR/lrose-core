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

#include <Spdb/DsSpdb.hh>
#include <iostream>
#include <cmath>

#include <toolsa/pmu.h>
#include <toolsa/umisc.h>    
#include <toolsa/pjg_flat.h> 

#include <rapformats/VerGridRegion.hh> 

#include <Mdv/MdvxField.hh>

#include "Detect.hh"

////////////////////////////////
// Constructor. Does naught.
//
Detect::Detect(){


}

////////////////////////////////
// Destructor. Does nowt.
//
Detect::~Detect(){



}

///////////////////////////////////
// Threshold the two fields, detect
// storms, and send the output on.
//
int Detect::Threshold(MdvxField *truthField,
		      MdvxField *forecastField,
		      Params *P,
		      int UrlIndex,
		      time_t dataTime){

  VerGridRegion V;

  PMU_auto_register("Detecting...");

  Mdvx::field_header_t F = forecastField->getFieldHeader();
  float forecastMissing = F.missing_data_value;
  float forecastBad     = F.bad_data_value;

  Mdvx::field_header_t Fhdr = truthField->getFieldHeader();
  float truthMissing = Fhdr.missing_data_value;
  float truthBad     = Fhdr.bad_data_value;

  V.setHdr(P->Circles_n,
	   F.forecast_time,
	   P->_forecastLeadTime[UrlIndex],
	   0, // Forecast height
	   P->forecastLowerLimit,
	   P->forecastUpperLimit,
	   0, // Truth height
	   P->truthLowerLimit,
	   P->truthUpperLimit);
  
  for (int Circle = 0; Circle < P->Circles_n; Circle ++){

    PMU_auto_register("Dealing with circle");



    int minx,maxx,miny,maxy;
    int Cx, Cy;

    GetExtremes(P->_Circles[Circle].lat,
		P->_Circles[Circle].lon,
		P->_Circles[Circle].radius,
		Fhdr, &Cx, &Cy,
		&minx,&maxx,&miny,&maxy);

    // If the circle is outside our region, return.
    if ((minx == maxx) || (miny == maxy)) return 0;

    int truthNum = 0,forecastNum=0;
    int truthTotal=0, forecastTotal=0;

    float *truthData =    (float *)truthField->getVol();
    float *forecastData = (float *)forecastField->getVol();

    //
    // Actually do the thresholding.
    //
    for (int ix = minx; ix <= maxx; ix++){
      for (int iy = miny; iy <= maxy; iy++){
	if (GetDist(Cx,Cy,ix,iy,Fhdr) <= P->_Circles[Circle].radius){
	  
	  int index = iy * Fhdr.nx + ix; 

	  if (P->useAllPtsInCircle){
	    truthNum++;
	    if ((truthData[index] >= P->truthLowerLimit) &&
		(truthData[index] <= P->truthUpperLimit) &&
		(truthData[index] != truthMissing) &&
		(truthData[index] != truthBad)){
	      truthTotal++;
	    }
	  }
	  else if ((truthData[index]  != truthMissing) &&
		   (truthData[index]  != truthBad)){
	    truthNum++;
	    if ((truthData[index] >= P->truthLowerLimit) &&
		(truthData[index] <= P->truthUpperLimit)){
	      truthTotal++;
	    }
	  }

	  if (P->useAllPtsInCircle){
	    forecastNum++;
	    if ((forecastData[index] >= P->forecastLowerLimit) &&
		(forecastData[index] <= P->forecastUpperLimit) &&
		(forecastData[index] != forecastMissing) &&
		(forecastData[index] != forecastBad)){
	      forecastTotal++;
	    }
	  }
	  else if ((forecastData[index]  != forecastMissing) &&
		   (forecastData[index]  != forecastBad)){
	    forecastNum++;
	    if ((forecastData[index] >= P->forecastLowerLimit) &&
		(forecastData[index] <= P->forecastUpperLimit)){
	      forecastTotal++;
	    }
	  }
	}
      }
    }
    float truthPercent;
    if (truthNum == 0){
      truthPercent = 0.0;
    } else {
      truthPercent = 100.0 * float(truthTotal) / float(truthNum);
    }

    float forecastPercent;
    if (forecastNum == 0){
      forecastPercent = 0.0;
    } else {
      forecastPercent = 100.0 * float(forecastTotal) / float(forecastNum);
    }

    if (P->Debug) {

      cerr << forecastPercent <<  "\t";
      cerr << truthPercent << "\t";
      cerr << P->_Circles[Circle].percent_covered << "\t";
      cerr << "TIME : " << utimstr(dataTime) << "\t";
      cerr <<  P->_Circles[Circle].regionName << "\n";

    }

    V.setData(Circle,
	      P->_Circles[Circle].regionName,
	      P->_Circles[Circle].lat,
	      P->_Circles[Circle].lon,
	      P->_Circles[Circle].radius,
	      forecastPercent,
	      truthPercent);


  } // End of loop through the circles.
  //
  // Now, write the SPDB data.
  //




  V.to_BE();

  DsSpdb S;
  S.clearUrls(); S.addUrl( P->outputURL );
  S.clearPutChunks();
  //
  // I think we need the data time here.
  //
  S.addPutChunk(1, // Data type.
		dataTime,
		dataTime + P->expiryTime,
		V.chunkLen(),
		(void *) V.chunkPtr());
  
  S.put(SPDB_VERGRID_REGION_ID,SPDB_VERGRID_REGION_LABEL);

  return 0;

}
//
//
//
void Detect::GetExtremes(float lat, float lon,
			 float radius,
			 Mdvx::field_header_t Fhdr,
			 int *Cx, int *Cy,
			 int *minx, int *maxx, int *miny, int *maxy){


  //
  // At the moment only flat earth is supported.
  //
  switch( Fhdr.proj_type ){


  case Mdvx::PROJ_FLAT :

    double Vx,Vy, rCx, rCy;
    PJGLatLon2DxDy(Fhdr.proj_origin_lat,
		   Fhdr.proj_origin_lon,
		   lat,lon, &Vx, &Vy); 
    
    rCx = Vx - Fhdr.grid_minx;
    rCy = Vy - Fhdr.grid_miny;

    int X,Y;
    X = (int)rint(rCx/Fhdr.grid_dx);
    Y = (int)rint(rCy/Fhdr.grid_dy);

    *minx = X - (int)radius;
    *maxx = X + (int)radius;

    *miny = Y - (int)radius;
    *maxy = Y + (int)radius;
    *Cx = X; *Cy = Y;

    break;

    //
    // Insert code for other projections here
    //


  default :
    cerr << "Only flat earth projection supported at this time." << endl;
    exit(-1);
    break;

  }

  //
  // Do some QA - make sure it's all inside the grid.
  //
  if (*minx < 0) *minx = 0;
  if (*miny < 0) *miny = 0;
  if (*maxx < 0) *maxx = 0;
  if (*maxy < 0) *maxy = 0;

  if (*minx > Fhdr.nx-1) *minx = Fhdr.nx-1;
  if (*miny > Fhdr.ny-1) *miny = Fhdr.ny-1;
  if (*maxx > Fhdr.nx-1) *maxx = Fhdr.nx-1;
  if (*maxy > Fhdr.ny-1) *maxy = Fhdr.ny-1;

  if (*Cx > Fhdr.nx-1) *Cx = Fhdr.nx-1;
  if (*Cy > Fhdr.ny-1) *Cy = Fhdr.ny-1;
  if (*Cx < 0) *Cx = 0;
  if (*Cy < 0) *Cy = 0;

  return;

}

//////////////////////////////////////////////////
//
//
//
float Detect::GetDist(int Cx, int Cy, int ix, int iy,
		      Mdvx::field_header_t Fhdr){

  //
  // At the moment only flat earth supported.
  // Other projections should be trivial.
  //
  double Dx,Dy;
  switch( Fhdr.proj_type ){

  case Mdvx::PROJ_FLAT :

    Dx = double(ix-Cx)*Fhdr.grid_dx;
    Dy = double(iy-Cy)*Fhdr.grid_dy;

    break;

    //
    // Insert code for other projections here
    //

  default :
    cerr << "Only flat earth projection supported for distances." << endl;
    exit(-1);
    break;

  }

  return float(sqrt(Dx*Dx + Dy*Dy));

}










