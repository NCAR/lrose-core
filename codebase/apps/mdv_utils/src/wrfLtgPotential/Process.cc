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
int Process::Derive(Params *P, time_t T, int leadTime){


  if (P->Debug) cerr << "  Data at " << utimstr(T) << endl;

  //
  // Set up for the new data.
  //
  DsMdvx New;

  // Only read in the fields we need (the volume is quite large).
  New.addReadField( P->inFieldNames.graupelFieldName );
  New.addReadField( P->inFieldNames.pressureFieldName );
  New.addReadField( P->inFieldNames.tempFieldName );
  New.addReadField( P->inFieldNames.snowFieldName );
  New.addReadField( P->inFieldNames.wFieldName );

  if (P->dbz.doCalc){
    New.addReadField( P->dbz.rainFieldName );
  }

  if (P->remap.doRemap){
    New.setReadRemapFlat(P->remap.nx, P->remap.ny,
                         P->remap.minx, P->remap.miny,
                         P->remap.dx, P->remap.dy,
                         P->remap.originLat, P->remap.originLon,
                         0.0);
  }

  if ((P->Mode == Params::ARCHIVE) && (P->writeAsForecast)){
    New.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, P->TriggerUrl, 86400, T, leadTime );
  } else {
    New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  }
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     

  if (P->Debug)
    cerr << "  Data read from " << New.getPathInUse() << endl;

  Mdvx::master_header_t InMhdr = New.getMasterHeader();

  //
  // Get the input fields.
  //
  MdvxField *graupelField = New.getFieldByName( P->inFieldNames.graupelFieldName );
  if ( graupelField == NULL ){
    cerr << "Graupel field " <<  P->inFieldNames.graupelFieldName << " not found, skipping." << endl;
    return -1;
  }

  MdvxField *pressureField = New.getFieldByName( P->inFieldNames.pressureFieldName );
  if ( pressureField == NULL ){
    cerr << "Pressure field " <<  P->inFieldNames.pressureFieldName << " not found, skipping." << endl;
    return -1;
  }

  MdvxField *tempField = New.getFieldByName( P->inFieldNames.tempFieldName );
  if ( tempField == NULL ){
    cerr << "Temp field " <<  P->inFieldNames.tempFieldName << " not found, skipping." << endl;
    return -1;
  }

  MdvxField *snowField = New.getFieldByName( P->inFieldNames.snowFieldName );
  if ( snowField == NULL ){
    cerr << "Snow field " <<  P->inFieldNames.snowFieldName << " not found, skipping." << endl;
    return -1;
  }


  MdvxField *wField = New.getFieldByName( P->inFieldNames.wFieldName );
  if ( wField == NULL ){
    cerr << "W field " <<  P->inFieldNames.wFieldName << " not found, skipping." << endl;
    return -1;
  }

  //
  // get the field headers, data.
  //
  fl32 *graupelData = (fl32 *)graupelField->getVol();
  Mdvx::field_header_t graupelFhdr = graupelField->getFieldHeader();

  if (graupelFhdr.proj_type != Mdvx::PROJ_FLAT){
    cerr << "Data are not on flat projection, unable to process without remapping data first." << endl;
    return -1;
  }

  fl32 *pressureData = (fl32 *)pressureField->getVol();
  Mdvx::field_header_t pressureFhdr = pressureField->getFieldHeader();

  fl32 *tempData = (fl32 *)tempField->getVol();
  Mdvx::field_header_t tempFhdr = tempField->getFieldHeader();

  fl32 *snowData = (fl32 *)snowField->getVol();
  Mdvx::field_header_t snowFhdr = snowField->getFieldHeader();
  Mdvx::vlevel_header_t vhdr3D = snowField->getVlevelHeader();


  fl32 *wData = (fl32 *)wField->getVol();
  Mdvx::field_header_t wFhdr = wField->getFieldHeader();

  int nx = wFhdr.nx; int ny = wFhdr.ny;  int nz = wFhdr.nz; // Assume all fields the same size.

  fl32 *w_all = (fl32 *)malloc(nx*ny*sizeof(fl32));
  if (w_all == NULL){
    cerr << "W malloc failed!" << endl;
    exit(-1);
  }

  fl32 *ice_all = (fl32 *)malloc(nx*ny*sizeof(fl32));
  if (ice_all == NULL){
    cerr << "Ice malloc failed!" << endl;
    exit(-1);
  }

  fl32 *ltg_pot_all = (fl32 *)malloc(nx*ny*sizeof(fl32));
  if (ltg_pot_all == NULL){
    cerr << "Ltg pot malloc failed!" << endl;
    exit(-1);
  }

  const double R = 278.05;


  for (int ix=0; ix < nx; ix++){
    for (int iy=0; iy < ny; iy++){

      int index2D = iy*nx+ix;
      w_all[index2D]=0.0;
      ice_all[index2D]=0.0;

      for (int iz=0; iz < nz; iz++){
	int index3D = iz*nx*ny + iy*nx+ix;

	// Figure out contribution to w_all

	if (
	    (wData[index3D] != wFhdr.bad_data_value) &&
	    (wData[index3D] != wFhdr.missing_data_value) &&
	    (tempData[index3D] != tempFhdr.bad_data_value) &&
	    (tempData[index3D] != tempFhdr.missing_data_value) 
	    ){
	  if (
	      (wData[index3D] >= P->iceThresh.minVertSpeedIce) &&
	      (_rightTemp(tempData[index3D]) <= P->iceThresh.maxTempIce)
	      ){
	    w_all[index2D]+=1.0;
	  }
	}

	// Figure out contribution to ice_all
	
	if (
	    (snowData[index3D] != snowFhdr.bad_data_value) &&
	    (snowData[index3D] != snowFhdr.missing_data_value) &&
	    (tempData[index3D] != tempFhdr.bad_data_value) &&
	    (tempData[index3D] != tempFhdr.missing_data_value) &&
	    (graupelData[index3D] != graupelFhdr.bad_data_value) &&
	    (graupelData[index3D] != graupelFhdr.missing_data_value) &&
	    (pressureData[index3D] != pressureFhdr.bad_data_value) &&
	    (pressureData[index3D] != pressureFhdr.missing_data_value)
	    ){

	  //
	  // Get rho from pressure, multiply by 100 to get SI units
	  //
	  double rho = 100.0*pressureData[index3D]/(R*(273.15+_rightTemp(tempData[index3D])));

	  if (_rightTemp(tempData[index3D]) <= P->iceThresh.maxTempIceAll) {
	    ice_all[index2D] += wFhdr.grid_dx*wFhdr.grid_dy*rho*(_nonNeg(snowData[index3D]) + _nonNeg(graupelData[index3D]));
	  }
	}
      } // End of loop in height

      w_all[index2D] *= wFhdr.grid_dx*wFhdr.grid_dy; // Only works for flat, lambert projections.
      ice_all[index2D] = P->iceScaleBias.iceScale*(ice_all[index2D] - P->iceScaleBias.iceBias);
      if (ice_all[index2D] <0.0) ice_all[index2D] = 0.0;


      double steig_w=w_all[index2D]/P->steigvarThresh.steigvar_w;
      if (steig_w > 1.0) steig_w = 1.0;
      if (w_all[index2D] == 0) steig_w = 0.0;

      double weight_w = steig_w*0.5;


      double steig_pice=ice_all[index2D]/P->steigvarThresh.steigvar_pice;
      if (steig_pice > 1.0) steig_pice = 1.0;

      double weight_pice = steig_pice*0.5;

      ltg_pot_all[index2D] = weight_w + weight_pice;

    }
  }

  //
  // Now output these to MDV.
  //
  Mdvx::master_header_t Mhdr = New.getMasterHeader();
  DsMdvx Out;

  Out.setMasterHeader(Mhdr);

  Mdvx::vlevel_header_t vhdr;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0.0;

  Mdvx::field_header_t fhdr = snowFhdr;
  fhdr.nz = 1;

  if (P->useZeroForBad){
    fhdr.bad_data_value = 0.0;
    fhdr.missing_data_value = 0.0;
  } else {
    fhdr.bad_data_value = -1.0;
    fhdr.missing_data_value = -1.0;
  }
  fhdr.volume_size = fhdr.nz*fhdr.ny*fhdr.nx*sizeof(fl32);

  MdvxField *wFld = new MdvxField(fhdr, vhdr, w_all);

  wFld->setFieldName("ltgPotW");
  wFld->setUnits("none");
  wFld->setFieldNameLong("ltg potl W");

  if (wFld->convertRounded(Mdvx::ENCODING_INT16,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }

  Out.addField(wFld);


  MdvxField *iceFld = new MdvxField(fhdr, vhdr, ice_all);

  iceFld->setFieldName("ltgPotIce");
  iceFld->setUnits("none");
  iceFld->setFieldNameLong("ltg potl ice");

  if (iceFld->convertRounded(Mdvx::ENCODING_INT16,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }

  Out.addField(iceFld);


  MdvxField *totalFld = new MdvxField(fhdr, vhdr, ltg_pot_all);

  totalFld->setFieldName("ltgPot");
  totalFld->setUnits("none");
  totalFld->setFieldNameLong("ltg potential");

  if (totalFld->convertRounded(Mdvx::ENCODING_INT16,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }

  Out.addField(totalFld);

  if (P->dbz.doCalc){

    MdvxField *rainField = New.getFieldByName( P->dbz.rainFieldName );
    if ( rainField == NULL ){
      cerr << "Rain field " <<  P->dbz.rainFieldName << " not found, can't do dbz calc " << endl;
    } else {

      //
      // Allocate space
      //
      fl32 *dbz = (fl32 *)malloc(sizeof(fl32)*nx*ny*nz);
      if (dbz == NULL){
	cerr << "DBZ malloc failed!" << endl;
	exit(-1);
      }

      //
      // get the field headers, data.
      //
      fl32 *rainData = (fl32 *)rainField->getVol();
      Mdvx::field_header_t rainFhdr = rainField->getFieldHeader();
      
      for (int ix=0; ix < nx; ix++){
	for (int iy=0; iy < ny; iy++){
	  for (int iz=0; iz < nz; iz++){

	    int index3D = iz*nx*ny + iy*nx+ix;
	    dbz[index3D] = P->dbz.minVal;

	    if (
		(snowData[index3D] != snowFhdr.bad_data_value) &&
		(snowData[index3D] != snowFhdr.missing_data_value) &&
		(tempData[index3D] != tempFhdr.bad_data_value) &&
		(tempData[index3D] != tempFhdr.missing_data_value) &&
		(graupelData[index3D] != graupelFhdr.bad_data_value) &&
		(graupelData[index3D] != graupelFhdr.missing_data_value) &&
		(pressureData[index3D] != pressureFhdr.bad_data_value) &&
		(pressureData[index3D] != pressureFhdr.missing_data_value) &&
		(rainData[index3D] != rainFhdr.bad_data_value) &&
		(rainData[index3D] != rainFhdr.missing_data_value)
		){

	      //
	      // Get rho from pressure, multiply by 100 to get SI units
	      //
	      double rho = 100.0*pressureData[index3D]/(R*(273.15+_rightTemp(tempData[index3D])));

	      dbz[index3D] = _compute_reflec( rho, tempData[index3D],
					      rainData[index3D], snowData[index3D], graupelData[index3D],
					      P->dbz.minVal);
	    }
	  }
	}
      }


      fhdr.bad_data_value = P->dbz.minVal;
      fhdr.missing_data_value = fhdr.bad_data_value;
      fhdr.nz = nz;
      fhdr.volume_size = fhdr.nz*fhdr.ny*fhdr.nx*sizeof(fl32);
      
      MdvxField *dbzFld = new MdvxField(fhdr, vhdr3D, dbz);

      free(dbz);
      
      dbzFld->setFieldName("DBZ");
      dbzFld->setUnits("dbz");
      dbzFld->setFieldNameLong("derived reflectivity");
      
      if (dbzFld->convertRounded(Mdvx::ENCODING_INT16,
				 Mdvx::COMPRESSION_ZLIB)){
	cerr << "convertRounded failed - I cannot go on." << endl;
	exit(-1);
      }
      
      Out.addField(dbzFld);
    }
  }

  free(w_all); free(ice_all); free(ltg_pot_all);


  if (P->writeAsForecast)
    Out.setWriteAsForecast();

  if (Out.writeToDir(P->OutUrl)) {
    cerr << "Failed to wite to " << P->OutUrl << endl;
    return -1;
  }

  if (P->Debug){
    cerr << "  Output written to " << Out.getPathInUse() << endl << endl;
  }

  return 0;

}

double Process::_nonNeg(double x){
  if (x > 0.0) return x;
  return 0.0;
}

// Return T in deg C, even if input is in K
double Process::_rightTemp(double t){
  if (t > 120.0) return t-273.15;
  return t;
}

double Process::_compute_reflec( double den, double temp,
				 double qr, double qs, double qh,
				 double minVal){




  double rn0_r = 8.e6; // m^-4
  double rn0_s = 2.e7;
  double rn0_g = 4.e6;

  
  double celkel = 273.15;
  double rhowat = 1000.0;
  
  
  // Other constants

  double gamma_seven = 720.0;
  double rho_r = rhowat; // 1000. kg m^-3
  double rho_s = 100.0;  // kg m^-3
  double rho_g = 400.0;  // kg m^-3
  double alpha = 0.224;
  double factor_r = gamma_seven * 1.e18 * pow((1./(M_PI*rho_r)),1.75);
  double factor_s = gamma_seven * 1.e18 * pow((1./(M_PI*rho_s)),1.75)
    * pow((rho_s/rhowat),2.0) * alpha;
  double  factor_g = gamma_seven * 1.e18 * pow((1./(M_PI*rho_g)),1.75)
    * pow((rho_g/rhowat),2.0) * alpha;

  double rhoair = den;

  double tempK = _rightTemp(temp)+celkel;


  double factorb_s, factorb_g;

  if(tempK > celkel) {
    factorb_s=factor_s/alpha;
    factorb_g=factor_g/alpha;
  } else {
    factorb_s=factor_s;
    factorb_g=factor_g;
  }

  // Scheme without Ice physics.
  double ronv = rn0_r;
  double sonv = rn0_s;
  double gonv = rn0_g;

  // Total equivalent reflectivity factor (z_e, in mm^6 m^-3) is
  double z_e = 0.0;

  if (tempK > celkel){

    z_e = factor_r * pow((rhoair * qr),1.75) / pow(ronv,0.75);

    z_e += factorb_s * pow((rhoair * qs),1.75) / pow(sonv,0.75);
  
    z_e += factorb_g * pow((rhoair * qh),1.75) / pow(gonv,0.75);
  
  } else {
    
    // Below freezing, take qra value as qsn
 
    z_e = factorb_s * pow((rhoair * qr),1.75) / pow(sonv,0.75);
    
    z_e += factorb_s * pow((rhoair * qs),1.75) / pow(sonv,0.75);
  
    z_e += factorb_g * pow((rhoair * qh),1.75) / pow(gonv,0.75);
    
  }

  if(z_e < 0.0001) z_e = 0.0001;
  
  double dbz = 10.0 * log10(z_e);

  if (dbz < minVal) 
    dbz = minVal;

  return dbz;

}





Process::~Process(){
  return;
}









