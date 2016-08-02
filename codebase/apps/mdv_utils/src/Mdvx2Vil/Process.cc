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
#include <physics/vil.h>

//
// Macros used for SSS index calculations
//
#define MASS_Z(mass, dbz) if ((dbz) > VIL_THRESHOLD) \
               mass = (VILCONST * pow(10.0, (dbz) * FOURBYSEVENTY) * 1000.0)

using namespace std;

//
// Constructor
//
Process::Process(){
  OutputUrl = (char *)NULL;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T){

  fl32 mass = 0.0;

  if (P->debug){
    cerr << "Data at " << utimstr(T) << endl;
  }


  //
  // Set up for the new data.
  //
  DsMdvx New;

  New.addReadField( P->dbz_field );

  New.setDebug( P->debug);

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);


  if (P->RemapGrid){
    switch ( P->grid_projection){

    case Params::FLAT:
      New.setReadRemapFlat(P->grid_nx, P->grid_ny,
			 P->grid_minx, P->grid_miny,
			 P->grid_dx, P->grid_dy,
			 P->grid_origin_lat, P->grid_origin_lon,
			 P->grid_rotation);

      break;
                                   
    case Params::LATLON:
      New.setReadRemapLatlon(P->grid_nx, P->grid_ny,
			   P->grid_minx, P->grid_miny,
			   P->grid_dx, P->grid_dy);

      break;            

    case Params::LAMBERT:
      New.setReadRemapLc2(P->grid_nx, P->grid_ny,
			P->grid_minx, P->grid_miny,
			P->grid_dx, P->grid_dy,
			P->grid_origin_lat, 
			P->grid_origin_lon,
			P->grid_lat1,  P->grid_lat2);
      
      break;
      
    default:
      cerr << "Unsupported projection." << endl;
      return -1;
      break;
      
    }               
  }

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     

  //
  // Set up the output.
  //
  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  Mdvx::master_header_t OutMhdr = InMhdr;


  Out.setMasterHeader(OutMhdr);
  Out.clearFields();

  bool  calcTotalVil, calcDiffVil, calcSSSIndex;
  calcTotalVil = calcDiffVil = calcSSSIndex = false;

  if ( P->output_totalVil ) calcTotalVil = true;
  if ( P->output_dVil )  calcDiffVil = true;
  if ( P->output_SSS_index )  calcSSSIndex = true;

  if (
      (calcTotalVil == false) &&
      (calcDiffVil == false) &&
      (calcSSSIndex == false)
      ){
    cerr << "There are no output fields requested. No point in going on." << endl;
    exit(-1);
  }

  //
  // Get the DBZ field.
  //
  MdvxField *InField = New.getFieldByName( P->dbz_field );

  if (InField == NULL){
    cerr << "Field name " << P->dbz_field << " not found - skipping ..." << endl;
    return -1;
  }

  Mdvx::field_header_t fhdr = InField->getFieldHeader();

  Mdvx::vlevel_header_t vhdr = InField->getVlevelHeader();

  int npoints = fhdr.nx * fhdr.ny;

  //
  // allocate data arrays
  //
  fl32 *totalVilArray   = (fl32 *) ucalloc (npoints, sizeof(fl32));
  fl32 *lowerVilArray = (fl32 *) ucalloc (npoints, sizeof(fl32));
  fl32 *diffVilArray    = (fl32 *) ucalloc (npoints, sizeof(fl32));
  fl32 *sssIndexArray   = (fl32 *) ucalloc (npoints, sizeof(fl32));

  fl32 totalVilMin, diffVilMin, sssIndexMin;
  totalVilMin = diffVilMin = sssIndexMin =  9999999.0;

  fl32 totalVilMax, diffVilMax, sssIndexMax;
  totalVilMax = diffVilMax = sssIndexMax = -9999999.0;

  fl32 lowerVilSum, totalVilSum, upperVilVal, centerMass, stdDeviation;

  int nz = fhdr.nz;

  // load up ht and delta ht arrays
  fl32 *ht = (fl32 *) umalloc(nz * sizeof(fl32));
  fl32 *dht = (fl32 *) umalloc(nz * sizeof(fl32));
  for (int iz = 0; iz < nz; iz++) {
    ht[iz] = vhdr.level[iz];
    
    
    fl32 base_ht, top_ht;
    if ( nz == 1 ) {
      base_ht = vhdr.level[0] - fhdr.grid_dz / 2.0;
      top_ht  = vhdr.level[0] + fhdr.grid_dz / 2.0;
    }
    else {
      if (iz == 0) {
        base_ht =  vhdr.level[0] -
	  (vhdr.level[1] -
	   vhdr.level[0]) / 2.0;
      }
      else {
        base_ht = (vhdr.level[iz] +
                   vhdr.level[iz - 1]) / 2.0;
      }
      if (iz == nz-1) {
        top_ht  =  vhdr.level[nz - 1] +
	  (vhdr.level[nz - 1] -
	   vhdr.level[nz - 2]) / 2.0;
      }
      else {
        top_ht  = (vhdr.level[iz] +
                   vhdr.level[iz + 1]) / 2.0;
      }
    }
    dht[iz] = top_ht - base_ht;
  }
  
  
  //
  // Initialize mass for SSS index calculations
  //
  fl32 *verticalMass = (fl32 *) ucalloc (nz, sizeof(fl32));

  fl32 *dbzData = (fl32 *)InField->getVol();

  //
  // loop through the points
  //
  for (int iy = 0; iy < fhdr.ny; iy++) {
    for (int ix = 0; ix < fhdr.nx; ix++) {

      int ipoint = ix + iy * fhdr.nx; // 2D index
    
      fl32 *totalVilPtr = totalVilArray + ipoint;
      fl32 *lowerVilPtr = lowerVilArray + ipoint;
      fl32 *diffVilPtr  = diffVilArray + ipoint;
      fl32 *sssIndexPtr = sssIndexArray + ipoint;
      
      VIL_INIT( totalVilSum );
      VIL_INIT( lowerVilSum );
      
      fl32 maxDbz = 0.0;
      fl32 maxDbzHeight = 0.0;

      //
      // loop through planes
      //
      for (int iz = 0; iz < fhdr.nz; iz++) {
	//
	// get plane and height data
	// skip over this plane if it is below the specified min.
	//
	fl32 planeHeight = ht[iz];
	if ( planeHeight < P->min_height ) {
	  continue;
	}
 
	int index3D = ix + iy * fhdr.nx + iz * fhdr.nx * fhdr.ny;
	
	fl32 dbz = dbzData[index3D];

	if (
	    (dbz == fhdr.missing_data_value) ||
	    (dbz == fhdr.bad_data_value)
	    ){
	  continue;
	}

	fl32 deltaHeight = dht[iz];
 
	if ( dbz > maxDbz ) {
	  maxDbz = dbz;
	  maxDbzHeight = planeHeight;
	}

	//
	// Accumulate the total vil
	//
	VIL_ADD ( totalVilSum, dbz, deltaHeight );

	//
	// Accumulate the lower vil
	// ONLY IF we are below the specified difference_height
	//
	if ( planeHeight <= P->difference_height ) { 
	  VIL_ADD( lowerVilSum, dbz, deltaHeight );
	}

	//
	// Get mass for SSS index calculation
	//
	MASS_Z( mass, dbz );

	verticalMass[iz] = mass;

      } /* iz */
 
      //
      // Compute the total vil for this point
      //
      VIL_COMPUTE(totalVilSum );
      *totalVilPtr = totalVilSum;
      totalVilMin  = MIN(*totalVilPtr, totalVilMin);
      totalVilMax  = MAX(*totalVilPtr, totalVilMax);

    //
    // Compute the lower vil for this point 
    // Don't need the min/max since we're not writing this out to mdv
    //
    VIL_COMPUTE(lowerVilSum );
    *lowerVilPtr = lowerVilSum;

    //
    // Compute the vil difference for this point
    //
    upperVilVal = *totalVilPtr - *lowerVilPtr;
    *diffVilPtr = upperVilVal - *lowerVilPtr;
    diffVilMin  = MIN(*diffVilPtr, diffVilMin);
    diffVilMax  = MAX(*diffVilPtr, diffVilMax);

    //
    // Compute the sss index for this point
    //
    if ( calcSSSIndex ) {
      centerMass = stdDeviation = 0.0;
      if ( totalVilSum > 0.1 && maxDbz > 30.0 ) {
	for (int iz = 0; iz < nz; iz++) {
	  centerMass += ht[iz] 
	    * verticalMass[iz] / totalVilSum;
	}
	for (int iz = 0; iz < nz; iz++) {
	  stdDeviation += pow(ht[iz] - centerMass, 2) 
	    * verticalMass[iz] / totalVilSum;
	}
      }
      if ( maxDbz >= 3.0 ) {
	/* cerr << iy*fhdr.nx + ix << ", ";

	for (int iz = 0; iz < nz; iz++) {
	  cerr << verticalMass[iz] << ", ";
	}
	cerr << " --->> "; */

	*sssIndexPtr = calcSSS( stdDeviation, centerMass, 
				maxDbzHeight, maxDbz, P );
      }
      sssIndexMin  = MIN(*sssIndexPtr, sssIndexMin);
      sssIndexMax  = MAX(*sssIndexPtr, sssIndexMax);
    }
    
    } /* iy */
  } /* ix */

 // free up ht arrays

  ufree(ht);
  ufree(dht);
  ufree(verticalMass);

  // Set up a vlevel header for the surface.

  Mdvx::vlevel_header_t surfVhdr;
  surfVhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  surfVhdr.level[0] = 0.0;

  fhdr.bad_data_value = 0.0;
  fhdr.missing_data_value = 0.0;
  fhdr.nz = 1;
  fhdr.volume_size = fhdr.nx*fhdr.ny*fhdr.nz*fhdr.data_element_nbytes;

  if ( calcTotalVil ) {

    MdvxField *fld = new MdvxField(fhdr, surfVhdr, totalVilArray);

    fld->setFieldName("vil");
    fld->setUnits("Kg/m2");
    fld->setFieldNameLong("vil");
    fld->setTransform("none");

    if (fld->convertDynamic(Mdvx::ENCODING_INT16,
			    Mdvx::COMPRESSION_ZLIB)){
      cerr << "convert failed - I cannot go on." << endl;
      exit(-1);
    }  
  
    Out.addField(fld);
  }



  if ( calcDiffVil ) {

    MdvxField *fld = new MdvxField(fhdr, surfVhdr, diffVilArray);

    fld->setFieldName("dVil");
    fld->setUnits("Kg/m2");
    fld->setFieldNameLong("dVil");
    fld->setTransform("none");

    if (fld->convertDynamic(Mdvx::ENCODING_INT16,
			    Mdvx::COMPRESSION_ZLIB)){
      cerr << "convert failed - I cannot go on." << endl;
      exit(-1);
    }  
  
    Out.addField(fld);
  }


  if ( calcSSSIndex ) {

    MdvxField *fld = new MdvxField(fhdr, surfVhdr, sssIndexArray);

    fld->setFieldName("SSS");
    fld->setUnits("Kg/m2");
    fld->setFieldNameLong("SSS Index");
    fld->setTransform("none");

    if (fld->convertIntegral(Mdvx::ENCODING_INT8,
			     Mdvx::COMPRESSION_ZLIB)){
      cerr << "convert failed - I cannot go on." << endl;
      exit(-1);
    }  
  
    Out.addField(fld);
  }

  //
  // free resources
  //
  ufree((char *) totalVilArray);
  ufree((char *) lowerVilArray);
  ufree((char *) diffVilArray);
  ufree((char *) sssIndexArray);
  
  //
  // Write the output data.
  //  
  if (Out.writeToDir(P->OutUrl)) {
    cerr << "Failed to wite to " << P->OutUrl << endl;
    exit(-1);
  }      

  if (P->debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  return 0;
}

Process::~Process(){
  return;
}

fl32 Process::calcSSS( fl32 stdDeviation, fl32 centerMass,
			 fl32 maxDbzHeight, fl32 maxDbz,
			 Params *P)
{


  const int  WB =1;   /* Weak Base 30-45 dBz*/
  const int  WV =2;   /* Weak volume 30-45 dBz*/
  const int  WT =3;   /* Weak Top  30-45 dBz*/
  const int  SB =4;   /* Severe Base  45 -55 dBz*/
  const int  SV =5;   /* Severe Volume 45-55 dBz*/
  const int  ST =6;   /* Severe TOP  45-55 dBz*/
  const int  VSB =7;  /* Very Severe Base  55+ dBz*/
  const int  VSV =8;  /* Very Severe Volume  55+ dBz*/
  const int  VST =9;  /* Very Severe Top 55+ dBz*/


  fl32 sss = 0.0;
  
  if( maxDbz < P->sss_weak_dbz_max) {
    if( centerMass < P->sss_base_center_mass_max ) {
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight < P->sss_base_height_max )
	sss = WB;
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight >= P->sss_base_height_max )
	sss = WV;
      if( stdDeviation < P->sss_std_deviation_limit)
	sss = WB;
    }
    else if( centerMass < P->sss_top_center_mass_min) {
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight < P->sss_base_height_max)
	sss = WB;
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight == P->sss_base_height_max)
	sss = WV;
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight >= P->sss_top_height_min)
	sss = WT;
      if( stdDeviation < P->sss_std_deviation_limit) 
	sss = WV;
    }
    else {
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight < P->sss_top_height_min)
	sss = WV;
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight >= P->sss_top_height_min)
	sss = WT;
      if( stdDeviation < P->sss_std_deviation_limit) 
	sss = WT;
    }
  }

  if( maxDbz >= P->sss_weak_dbz_max  && maxDbz < P->sss_severe_dbz_max) {
    if( centerMass < P->sss_base_center_mass_max) {
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight < P->sss_base_height_max)
	sss = SB;
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight >= P->sss_base_height_max)
	sss = SV;
      if( stdDeviation < P->sss_std_deviation_limit) 
	sss = SB;
    }
    else if( centerMass < P->sss_top_center_mass_min) {
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight < P->sss_base_height_max)
	sss = SB;
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight == P->sss_base_height_max)
	sss = SV;
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight >= P->sss_top_height_min)
	sss = ST;
      if( stdDeviation < P->sss_std_deviation_limit)
	sss = SV;
    }
    else {
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight < P->sss_top_height_min)
	sss = SV;
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight >= P->sss_top_height_min)
	sss = ST;
      if( stdDeviation < P->sss_std_deviation_limit) 
	sss = ST;
    }
  }

  if( maxDbz > P->sss_severe_dbz_max) {
    if( centerMass < P->sss_base_center_mass_max) {
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight < P->sss_base_height_max)
	sss = VSB;
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight >= P->sss_base_height_max)
	sss = VSV;
      if( stdDeviation < P->sss_std_deviation_limit) 
	sss = VSB;
    }
    else if( centerMass < P->sss_top_center_mass_min) {
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight < P->sss_base_height_max)
	sss = VSB;
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight == P->sss_base_height_max)
	sss = VSV;
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight >= P->sss_top_height_min)
	sss = VST;
      if( stdDeviation < P->sss_std_deviation_limit) 
	sss = VSV;
    }
    else {
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight < P->sss_top_height_min)
	sss = VSV;
      if( stdDeviation >= P->sss_std_deviation_limit &&
	  maxDbzHeight >= P->sss_top_height_min)
	sss = VST;
      if( stdDeviation < P->sss_std_deviation_limit) 
	sss = VST;
    }
  }

  /*
  cerr << stdDeviation << ", ";
  cerr << centerMass << ", ";
  cerr <<  maxDbzHeight << ", ";
  cerr << maxDbz << ", ";
  cerr << sss << endl;
  */

  return sss;
}

