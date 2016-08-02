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
#include <iostream>
#include <vector>

#include <cstdio>

#include <toolsa/pmu.h> 
#include <toolsa/umisc.h>
#include <toolsa/pjg_flat.h>

#include <math.h>
#include <string.h> // For strdup 
#include <stdlib.h> // For malloc

#include "read_sounding_class.hh"
#include "pints.hh"
#include "nints.hh"
#include "calc_li.hh"
#include "Output.hh"
#include "Input.hh"
#include "Params.hh"
#include "Control.hh"
#include "Terrain.hh"
#include "Filt2DMedian.hh"
#include "ProjType.hh"
#include "ReadSounding.hh"

using namespace std;

void GetDataAndWrite(time_t t_end, const Params &params)

{

  int nx = params.nx;
  int ny = params.ny;
  int npoints = nx * ny;

  float lon,lat;

  const float M2Ft = 3.280833; // Meters to feet.
  const float Km2Ft = 3280.833; // Kilometers to feet.

  bool debug = params.debug;

  float pres_li,tli=0.0;
  float tliK;
  float bad;

  float _rscale = params.Rscale;

  time_t t_start;

  float Rscale,Dmax,Dclose,Rfac;

  int i;

  char name[33],units[33];
 
  const float badsf=-999.99;
  ProjType Proj;
  
  // parameters for pints.
         
  int ip = 1;

  // special parameters for pints.
		  					  
  float gamma = 1.0;
  float rmxpints = -1.0;
  int ifg   = 0;
  float arcmax = 0.0;
  float rclose = -1.0;

  						    
  // The user used to be able to set these, but it wound up
  // being more trouble than it was worth - now set to 0.			    
  float xmin, ymin;
  xmin=0.0;
  ymin=0.0;


  float xmax,ymax,x0,y0;
  float dx = params.dx;
  float dy = params.dy;

  float lat_origin = params.lat_origin;
  float lon_origin = params.lon_origin;

 
  int ifield;

  int j;

  // set a default value for the temperature at pres_li
    
  float tli_default = -20.0;


  //
  // Fields have the following significance.
  //
  //
  // Index     | GRIB code |   Significance
  //
  //*	0	  33		U wind
  //*	1	  34		V wind
  //*	2	  11		Temperature, in C not K
  //*	3	  17		Dew point temp, C not K
  //	4	  131		Lifted index - requires soundings.
  //	5	  52		Relative humidity
  //	6	  118		Wind gust - grib code is undefined
  //*	7	  1		Pressure
  //              13            Potential temperature
  //	8	  263		liquid_accum
  //	9	  59		precip_rate
  //*	10	  20		visibility
  //*	11	  153		rvr - Runway visual Range - grib undefined.
  //*	12	  154		Ceiling - grib code is undefined.
  //*	13	  170		Convergance - calculated from U and V.
  //    14        185           Sea Level corrected ceiling - requires
  //                            terrain file and needs ceiling to be
  //                            specified.
  //    15        186           Flight vis rule, 0=LIFR, 1=IFR, 2=MVFR, 3=VFR
  //    16          5           Terrain from DEM
  //    17        171           Terrain from weather stations.
  //    18         18           Dew point depression
  // 
  // A star "*" indicates that this field is included by default.
  // Units are those used by the reporting station.
  //

  const int AllowedGribCodes[] = {33, 34, 11, 17, 131, 52, 118,
				  1, 13, 263, 59, 20, 153, 154, 170,
                                  185, 186, 5, 171, 18} ;

  const int NumAllowedGribCodes = 20;

  bad = badsf;

  xmax = xmin + (nx-1)*dx;
  ymax = ymin + (ny-1)*dy;

  if (params.flat){
    x0=-nx*params.dx/2.0; 
    y0=-ny*params.dy/2.0;
  } else {
    x0=params.lon_origin; 
    y0=params.lat_origin;
  }


  // Set up Proj structure for pints routine.
  Proj.Nx=params.nx;  Proj.Ny=params.ny;
  Proj.Dx=params.dx;  Proj.Dy=params.dy;
  Proj.Origin_lat=params.lat_origin;
  Proj.Origin_lon=params.lon_origin;    
  Proj.x0 = x0;        Proj.y0 = y0;

  if (params.flat){
    Proj.flat = 1;
  } else {
    Proj.flat = 0;
  }




  if (params.VisThresh_n != params.CeilThresh_n){
    fprintf(stderr,
	    "Ceiling and visibility threshold arrays must be the same length.\n");
    exit(-1);
  }


  PMU_auto_register("Reading surface and sounding data");

  pres_li = params.PresLi;

  date_time_t SoundTime;
  float SoundLat,SoundLon;

  date_time_t DataTime;

  DataTime.unix_time=t_end;
  uconvert_from_utime(&DataTime);



  // Check that the grib codes are all allowed - exit
  // if not, since this would cause severe problems
  // otherwise (the MDV file would be set up for
  // N fields and would then have fewer than N
  // written to it). Also see if we are calculating the lifted
  // index - if so, we need soundings.

  bool OK; bool NeedSoundings; bool GotSoundings;

  NeedSoundings = false; GotSoundings = false;
  for (i=0; i<params.desired_fields_n;i++){
    OK = false;
    for (j=0; j<NumAllowedGribCodes; j++){
      if (params._desired_fields[i] == AllowedGribCodes[j]){
	OK = true; 
	//
	// Field code 131 (lifted index) means we need to
	// look for soundings.
	//
	if (params._desired_fields[i] == 131) NeedSoundings = true;
	j=NumAllowedGribCodes;
      }
    }
    if (!(OK)) {
      fprintf(stderr,"Grib code %d is not allowed - I cannot cope.\n",
	      params._desired_fields[i]);
      exit(-1);
    }
  }



  if (NeedSoundings){
    if (params.ReadSoundingClassFiles){
      //
      // Read sounding from class files in sounding_dir
      //
      read_sounding_class(params.sounding_dir,
			  pres_li,&tli,tli_default,badsf,
			  &SoundLat, &SoundLon, &SoundTime,
			  DataTime,debug);

    } else {
      //
      // Read soundings from URL.
      //
      ReadSounding(params.sounding_url,
		   params.sounding_look_back,
		   params.sounding_alt_min,
		   params.sounding_alt_max,
		   pres_li, &tli,
		   tli_default,  badsf,
		   &SoundLat, &SoundLon,
		   &SoundTime, DataTime,
		   debug);            
    
    }

    if (tli==badsf){
      GotSoundings = false;
      if (debug) fprintf(stderr,"No soundings found.\n");
    } else {
      GotSoundings = true;
      if (debug){
	fprintf(stderr,"Read sounding class got %g at %g,%g ",
		tli,SoundLat,SoundLon);
      
	fprintf(stderr,"for time : %4d/%02d/%02d %02d:%02d:%02d\n",
		SoundTime.year,SoundTime.month,SoundTime.day,
		SoundTime.hour,SoundTime.min,SoundTime.sec);
                                              
      }
	double TimeDiff=fabs(double(SoundTime.unix_time-DataTime.unix_time));

	if (TimeDiff > params.MaxSoundTimeDiff*86400){

	  //
	  // The following WARNINGs should be 
	  // printed out regardless
	  // of if we are in debug mode.
	  //
	  fprintf(stderr,
		  "WARNING : Sounding time is significantly different to data time!\n");
	  if (params.ExitOnSoundingError) GotSoundings = false;
	  fprintf(stderr,"Performing calculation anyway...\n");
	}

	double SpaceDiff, Direction;

	PJGLatLon2RTheta(SoundLat,SoundLon,lat_origin,lon_origin,
			 &SpaceDiff, &Direction);   

	if (SpaceDiff > params.MaxSoundDist){
	  fprintf(stderr,
		  "WARNING : Sounding position is significantly different to origin!\n");
	  if (params.ExitOnSoundingError) GotSoundings = false;
	  fprintf(stderr,"Performing calculation anyway...\n");
	}
	
    }
    

  } // End of if we need soundings.

  if (!(GotSoundings)) tli = badsf; // Which effectivley leaves blank the LI field.

  t_start = t_end - params.duration; 


  // Read from the data source.

  // Read from the data source.

  Input II;
  if (II.read(params.input_source,
              t_start, t_end, bad,
              params.MaxVis,
              params.MaxCeiling)) {
    if (debug) fprintf(stderr,"No data found - returning.\n");
    return;
  }

  if (params.debug) {
    fprintf(stderr,"Number of stations reporting from %s : %d\n",
	    params.input_source,II.num_found);
  }


  if (II.num_found ==0){
    if (debug) fprintf(stderr,"No data found - returning.\n");
    return;
  }


  if (II.num_found < params.MinStations){
    if (debug) fprintf(stderr,"Only %d stations reported - need %d.\n",
		      II.num_found,  params.MinStations );
    return;
  }


  //
  // Allocate working space. This used to be done as an
  // automatic (ie. "float zint[npoints];") and this worked for
  // a while but then seemed to have problems. So for the moment
  // a malloc is being used.
  //

  float *zint = (float *)malloc(sizeof(float)*npoints);
  float *zu = (float *)malloc(sizeof(float)*npoints);
  float *zv = (float *)malloc(sizeof(float)*npoints);
  float *zvis = (float *)malloc(sizeof(float)*npoints);
  float *zvfr = (float *)malloc(sizeof(float)*npoints);
  float *zcld_height = (float *)malloc(sizeof(float)*npoints);
  float *zslceil = (float *)malloc(sizeof(float)*npoints);
  float *zalt = (float *)malloc(sizeof(float)*npoints);
  float *zterrain = (float *)malloc(sizeof(float)*npoints);

  float *datatemps = (float *)malloc(sizeof(float) * II.num_found);
  float *xsta = (float *)malloc(sizeof(float) * II.num_found);
  float *ysta = (float *)malloc(sizeof(float) * II.num_found);

  if (
      ( zint == NULL ) ||
      ( zu == NULL ) ||
      ( zv == NULL ) ||
      ( zvis == NULL ) ||
      ( zvfr == NULL ) ||
      ( zcld_height == NULL ) ||
      ( zslceil == NULL ) ||
      ( zalt == NULL ) ||
      ( zterrain == NULL ) ||
      ( datatemps == NULL ) ||
      ( xsta == NULL ) ||
      ( ysta == NULL )
      ){

    fprintf(stderr, "Malloc failed.\n");

  }

  //
  // Replace values of ceiling that indicate clear sky, if desired.
  //
  if (params.ReplaceCeiling){
    for (i=0; i<II.num_found; i++){
      if (II.reports[i].ceiling != bad){
	if (II.reports[i].ceiling >= params.ReplaceCeilingThreshold){
	  II.reports[i].ceiling = params.ReplaceCeilingValue;
	}
      }
    }
  }
  /* ------------------------------------

     // Debugging. Overwrite actual data with test values.
     // Test values are in file test.dat. Uncomment this
     // section of the code to make this debugging take place.

     FILE *ifp;
     ifp = fopen("test.dat","ra");

     fscanf(ifp,"%d",&II.num_found);
     for (i=0; i<II.num_found; i++){
     fscanf(ifp,"%f %f %f %f %f %f %f %f",
     &II.reports[i].lat,
     &II.reports[i].lon,
     &II.u[i],
     &II.v[i],
     &II.reports[i].rvr,
     &II.reports[i].visibility,
     &II.reports[i].ceiling,
     &II.reports[i].alt
     );

     }
     fclose(ifp);

     ------------------------------------ */


    if (params.flat) {
      for (i=0;i<II.num_found;i++){

	//  calculate station locations in kilometers relative to origin

	double xx, yy;
	
	PJGLatLon2DxDy(lat_origin, lon_origin,
		       II.reports[i].lat, II.reports[i].lon,
		       &xx, &yy);
	
// 	xx = (II.reports[i].lon - 
// 	      lon_origin)*111.12*cos(3.14159*lat_origin/180.0);
// 	yy = (II.reports[i].lat - lat_origin)*111.12;
   
	//  now calculate positions in index space.
      
	//
	// Niles : Modified the following lines after
	// advice from Jim Wilson in the field, May 21 1999.
	//

	//xsta[i] = (xx - xmin)/dx;
	//ysta[i] = (yy - ymin)/dy;

	xsta[i] = (xx - x0)/dx;
	ysta[i] = (yy - y0)/dy;

	// Modifications end.


      }
    } else { // Lat/Lon projection.

      for(i=0; i<II.num_found; i++){
	xsta[i]=(II.reports[i].lon-lon_origin)/dx;
	ysta[i]=(II.reports[i].lat-lat_origin)/dy;
      }
    }

    int NumIn =0;
    for(i=0; i<II.num_found; i++){
      if (xsta[i] > nx-1) continue;
      if (xsta[i]<0) continue;
      if (ysta[i] > ny-1) continue;
      if (ysta[i]<0) continue;
      NumIn++;

    }

    if (params.debug) {
      fprintf(stderr,"%d Stations in area.\n",NumIn);
    }

    if (debug) {
      fprintf(stderr,
	      "stat. no\tlat \tlon \tCeiling,KM \t Ceiling, Ft \tAlt,M \tVis, Km\n"); 

      for (i=0;i<II.num_found;i++){
 
	if (II.reports[i].lat != bad){
	  fprintf(stderr,
		  "%d %7.2f %7.2f %7.2f %7.2f  %7.2f %7.2f\n",
		  i+1,
		  II.reports[i].lat,II.reports[i].lon,
		  II.reports[i].ceiling,II.reports[i].ceiling*Km2Ft,
		  II.reports[i].alt,II.reports[i].visibility);
	}
      }


      /*
      fprintf(stderr,
	      "stat. no \tuwind \tvwind \ttemp \tdewpoint \tpress \tlat \tlon\n"); 

      for (i=0;i<II.num_found;i++){
 
	fprintf(stderr,
		"%d\t %7.2f\t %7.2f\t %7.2f\t %7.2f\t %7.2f\t %7.2f\t %7.2f\n",
		i+1,II.u[i],II.v[i],II.reports[i].temp,
		II.reports[i].dew_point,
		II.reports[i].pres,
		II.reports[i].lat,II.reports[i].lon);
 
      }

      */



    }



    // OK - we have some data - prepare to write it out. Init the
    // Output class.

    string fieldType = "analysis field";
    Output QQ(DataTime,
	      params.duration,
	      params.desired_fields_n, 
	      nx, 
	      ny, 
	      1,
	      lon_origin,
	      lat_origin, 
	      params.altitude,       
	      const_cast<char*>(fieldType.c_str()), 
	      params.dataset_name, 
	      params.input_source,
	      params.flat);

    bool GotU, GotV; // For convergance calc.
    bool GotCeil, GotVis, GotSLCeil;
    bool GotTerrain,GotAlt;

    GotTerrain=false; GotAlt=false;

    GotU=false; GotV=false; GotCeil=false; GotVis=false;
    GotSLCeil=false;


    for (ifield=0;ifield<params.desired_fields_n;ifield++){

      PMU_auto_register("Interpolating fields");

      if ((params._desired_fields[ifield] != 170) &&
	  (params._desired_fields[ifield] != 185) &&
	  (params._desired_fields[ifield] != 5) &&
	  (params._desired_fields[ifield] != 186)) { 
	// 170 being the grib code for convergance, 185
	// being for sea level corrected ceiling and
	// 186 being for the vis rules, and 5 is terrain. These are derived
	// fields and are not dealt with in the
	// switch statement.

	switch (params._desired_fields[ifield]){
	case 33:
	  if (params.debug) {
	    fprintf(stderr," interpolating uwind\n");
	  }
	  sprintf(name,"%s","Uwind"); sprintf(units,"%s","m/s");
	  for (i=0;i<II.num_found;i++) datatemps[i] = II.reports[i].uu;
	  GotU = true;
	  break;
	case 34:
	  if (params.debug) {
	    fprintf(stderr," interpolating vwind\n");
	  }
	  sprintf(name,"%s","Vwind"); sprintf(units,"%s","m/s");
	  for (i=0;i<II.num_found;i++) datatemps[i] = II.reports[i].vv;
	  GotV = true;
	  break;
	case 11:
	  if (params.debug) {
	    fprintf(stderr," interpolating temp\n");
	  }
	  sprintf(name,"%s","Temp"); sprintf(units,"%s","C");
	  for (i=0;i<II.num_found;i++)  datatemps[i] = II.reports[i].temp;
	  break;
	case 13 :
	  if (params.debug) {
	    fprintf(stderr," calculating potentialtemp\n");
	  }
	  sprintf(name,"%s","PotentialTemp"); sprintf(units,"%s","K-300");
	  for (i=0;i<II.num_found;i++){
	    if ((II.reports[i].temp == bad) || (II.reports[i].pres == bad)){
	      datatemps[i]=bad;
	    } else {
	      //
	      // Units are Kelvin - 300 degrees.
	      //
	      float tk = 273.15 + II.reports[i].temp;
	      float relP = 1000.0 / II.reports[i].pres;
	      datatemps[i] = double(tk) * pow(double(relP),0.287) - 300.0;
	    }
	  }

	  break;
	case 17:
	  if (params.debug) {
	    fprintf(stderr," interpolating dewpoint\n");
	  }
	  sprintf(name,"%s","DewPoint"); sprintf(units,"%s","C");
	  for (i=0;i<II.num_found;i++) datatemps[i] = II.reports[i].dew_point;
	  break;
	case 131:
	  if (params.debug) {
	    fprintf(stderr," interpolating lifted index\n");
	  }
	  sprintf(name,"%s","LiftedIndex"); sprintf(units,"%s","C");

	  // this is calculated rather than obtained directly.

	  if (tli == badsf)
	    tliK = badsf;
	  else
	    tliK = tli + 273.15;
                
	  for (i=0;i<II.num_found;i++){
	    if ((tli == badsf) ||
		(II.reports[i].pres==bad) ||
		(II.reports[i].temp==bad) ||
		(II.reports[i].dew_point==bad)){
	      datatemps[i]=bad;
	    } else {

	      datatemps[i] = calc_li(II.reports[i].pres,
				     II.reports[i].temp,
				     II.reports[i].dew_point,
				     tliK,
				     pres_li,bad);
	    }
	  }
	  break;
	case 52:
	  if (params.debug) {
	    fprintf(stderr," interpolating humidity\n");
	  }
	  sprintf(name,"%s","RelHum"); sprintf(units,"%s","Percent");
	  for (i=0;i<II.num_found;i++) datatemps[i] = II.reports[i].relhum;
	  break;
	case 118:
	  if (params.debug) {
	    fprintf(stderr," interpolating wind gust\n");
	  }
	  sprintf(name,"%s","Gust"); sprintf(units,"%s","m/s");
	  for (i=0;i<II.num_found;i++) datatemps[i] = II.reports[i].windgust;
	  break;
	case 1:
	  if (params.debug) {
	    fprintf(stderr," interpolating pressure\n");
	  }
	  sprintf(name,"%s","Pressure"); sprintf(units,"%s","mb");
	  for (i=0;i<II.num_found;i++) datatemps[i] = II.reports[i].pres;
	  break;
	case 263:
	  if (params.debug) {
	    fprintf(stderr," interpolating liquid accum\n");
	  }
	  sprintf(name,"%s","LiquidAccum"); sprintf(units,"%s","mm");
	  for (i=0;i<II.num_found;i++) datatemps[i] = II.reports[i].liquid_accum;
	  break;
	case 59:
	  if (params.debug) {
	    fprintf(stderr," interpolating precip_rate\n");
	  }
	  sprintf(name,"%s","PrecipRate"); sprintf(units,"%s","mm/hr");
	  for (i=0;i<II.num_found;i++) datatemps[i] = II.reports[i].precip_rate;
	  break;
	case 20:
	  if (params.debug) {
	    fprintf(stderr," interpolating visibility\n");
	  }
	  sprintf(name,"%s","Visibility"); sprintf(units,"%s","Km");
	  //
	  // I used to convert to Nautical Miles here - not now.
	  //
	  for (i=0;i<II.num_found;i++){ 
	    datatemps[i] = II.reports[i].visibility;
	  }

	  GotVis = true;
	  break;
	case 153:
	  if (params.debug) {
	    fprintf(stderr," interpolating runway visual range\n");
	  }
	  sprintf(name,"%s","RVR"); sprintf(units,"%s","Km");
	  for (i=0;i<II.num_found;i++) datatemps[i] = II.reports[i].rvr;
	  break;

	case 154:
	  if (params.debug) {
	    fprintf(stderr," interpolating cloud height\n");
	  }
	  sprintf(name,"%s","SeaLevelCeiling"); sprintf(units,"%s","Ft");
	  //
	  // A factor converts from Km to Feet at this point.
	  //
	  for (i=0;i<II.num_found;i++){ 
	    // if ceiling is bad but vis is good, set ceiling to high value
	    if (II.reports[i].ceiling == bad && II.reports[i].visibility != bad) {
	      II.reports[i].ceiling = 10.0;
	    }
	    if ((II.reports[i].ceiling == bad) || (II.reports[i].alt == bad)){
	      datatemps[i]=bad;
	      } else {
		datatemps[i] = II.reports[i].ceiling*Km2Ft +
		  II.reports[i].alt*M2Ft;
	      }
	  }
	  GotCeil=true;
	  break;

	case 171:
	  if (params.debug) {
	    fprintf(stderr," interpolating altitude\n");
	  }
	  sprintf(name,"%s","Altitude"); sprintf(units,"%s","Ft");
	  //
	  // A factor converts from Km to Feet at this point.
	  //
	  for (i=0;i<II.num_found;i++){ 
	    if (II.reports[i].alt == bad){
	      datatemps[i]=bad;
	      } else {
		datatemps[i] = II.reports[i].alt*M2Ft;
	      }
	  }
	  GotAlt=true;
	  break;


	case 18:
	  if (params.debug) {
	    fprintf(stderr," Interpolating DP depression\n");
	  }
	  sprintf(name,"%s","DP_depress"); sprintf(units,"%s","C");
	  for (i=0;i<II.num_found;i++){ 
	    if ((II.reports[i].temp == bad) || (II.reports[i].dew_point == bad)) {
	      datatemps[i]=bad;
	      } else {
		datatemps[i] = II.reports[i].temp -  II.reports[i].dew_point;
	      }
	  }
	  break;
		 

	} // End of switch.

	// At this point the field we want to interpolate is in datatemps.


	Rscale=0.0;
	Dmax=0.0;
	Dclose=0.0;
	Rfac=0.0;

	int Goodies = 0;
	for (i=0; i< II.num_found; i++){
	  if (datatemps[i] == bad) continue;
	  if (xsta[i] > nx-1) continue;
	  if (xsta[i]<0) continue;
	  if (ysta[i] > ny-1) continue;
	  if (ysta[i]<0) continue;
	  Goodies++;
	}


	if (!(params.UseOutsideRegion)){
	  for (i=0; i< II.num_found; i++){
	    if (datatemps[i] == bad) continue;
	    if (xsta[i] > nx-1) datatemps[i]=bad;
	    if (xsta[i]<0) datatemps[i]=bad;
	    if (ysta[i] > ny-1) datatemps[i]=bad;;
	    if (ysta[i]<0) datatemps[i]=bad;
	  }
	}

	if (params.debug) {
	  fprintf(stderr,"%d good readings for this field.\n",Goodies);
	}

	float qmin,qmax;
	int pp,NumBad;
	
	if (debug){ // Print extremities before and after interp.
	  qmax=-10000000.0;
	  qmin=10000000.0;
	  NumBad=0;
	  for(pp=0; pp<II.num_found; pp++){
	    if (datatemps[pp]!=bad){
	      if (qmax < datatemps[pp]) qmax=datatemps[pp];
	      if (qmin>datatemps[pp]) qmin=datatemps[pp];
	    } else {
	      NumBad++;
	    }
	  }

	  fprintf(stderr,"On input range is %g -> %g for %s\n",
		  qmin,qmax,name);
       
	  fprintf(stderr,"%d of %d were bad for %s\n",
		  NumBad,II.num_found,name);
	}


    

	if (params.desired_fields_n != params.InterpMethod_n){
	  //
	  // The InterpMethod array has not been set up.
	  // Use Barnes interpolation by default.
	  //
	  pints(
		zint,&nx,&ny,xsta,ysta,datatemps,&II.num_found,
		&_rscale,&rmxpints,&gamma,&ip,&ifg,&arcmax,
		&rclose,&bad,&debug,
		&Rscale,&Dmax,&Dclose,&Rfac,
		params.MinWeight,
		params.MaxInterpDist, Proj);
	} else {
	  //
	  // The InterpMethod array has been set up,
	  // so use this to determine interpolation scheme.
	  //
	  switch (params._InterpMethod[ifield]){

	  case Params::INTERP_BARNES :
	    if (debug) {
	      fprintf(stderr,"Using Barnes interpolation.\n");
	    }
	    pints(
		  zint,&nx,&ny,xsta,ysta,datatemps,&II.num_found,
		  &_rscale,&rmxpints,&gamma,&ip,&ifg,&arcmax,
		  &rclose,&bad,&debug,
		  &Rscale,&Dmax,&Dclose,&Rfac,
		  params.MinWeight,
		  params.MaxInterpDist, Proj);
	    break;

	  case Params::INTERP_NEAREST :
	    if (debug) fprintf(stderr,"Using nearest point interpolation.\n");
	    nints(zint,xsta,ysta,datatemps,
		  II.num_found,bad,
		  params.MaxInterpDist, Proj);
	    break;
	  
	  default:

	    fprintf(stderr,"Unsupported interpolation scheme.\n");
	    exit(-1);
	    break;

	  }
	}

	if (debug){
	  qmax=-10000000.0;
	  qmin=10000000.0;
	  NumBad=0;
	  for(pp=0; pp < npoints; pp++){
	    if (zint[pp]!=bad){
	      if (qmax < zint[pp]) qmax=zint[pp];
	      if (qmin > zint[pp]) qmin=zint[pp];
	    } else {
	      NumBad++;
	    }
	  }
	  
	  fprintf(stderr,"On Output range is %g -> %g for %s\n",
		  qmin,qmax,name);

	  fprintf(stderr,"%d of %d were bad for %s\n",
		  NumBad,npoints,name);

	}


	// Save the U and V fields for convergance calculation.

	if (params._desired_fields[ifield] == 33) {
	  for (i=0;i<npoints;i++){
	    zu[i] = zint[i];
	  }
	}

	if (params._desired_fields[ifield] == 34) { 
	  for (i=0;i<npoints;i++){
	    zv[i] = zint[i];
	  }
	}

	// Save Ceiling and vis for VFR rules.

	if (params._desired_fields[ifield] == 20) { 
	  for (i=0;i<npoints;i++){
	    zvis[i] = zint[i];
	  }
	}

	if (params._desired_fields[ifield] == 154) { 
	  for (i=0;i<npoints;i++){
	    zcld_height[i] = zint[i];
	  }
	}

	if (params._desired_fields[ifield] == 171) { // Alt
	  for (i=0;i<npoints;i++){
	    zalt[i] = zint[i];
	  }
	}


      } // End of if not a derived field.


      ////////////////////////// Calculation of derived fields. ////

      // Convergance - grib code 170
      if (params._desired_fields[ifield] == 170){
	if (!((GotU) && (GotV))){
	  fprintf(stderr,"Must specify U and V to get convergance.\n");
	  fprintf(stderr,"I cannot cope.\n"); exit(-1);
	}

	if (debug) {
	  fprintf(stderr," calculating convergence\n");
	}
	sprintf(name,"%s","Convergance"); sprintf(units,"%s","10**-6s-1");
      
	// calculate convergence.
       

	if (params.debug) {
	  PMU_auto_register("Calculating convergance");
	}

	// Get dx and dy in kilometers for convergance calculation.
	float dxkm, dykm;
	if (params.flat) {
	  dxkm=dx; dykm = dy; // Already in correct units.
	} else { // units are in degrees - convert to km.
	  // Only really correct at SW corner but OK.
	  //
	  dxkm = dx*111.12*cos(3.14159*lat_origin/180.0);
	  dykm = dy*111.12;
	  //
	}

	//
	// Set zint to all bad.
	//
	for (int l=0; l < nx*ny; l++){
	  zint[l]=bad;
	}

	int step = params.converganceCalcPoints;
	for (j=step; j<ny-step; j++){ /* do 60 j = 2,ny-1 */
	  for (i=step; i<nx-step; i++){ /* do 60 i = 2,nx-1 */
 
	    
	    if ((zu[i-step+nx*j] == bad) ||
		(zu[i+step+nx*j] == bad) ||
		(zv[i+nx*(j-step)] ==  bad) ||
		(zv[i+nx*(j+step)] ==  bad)) {
 
	      zint[i+nx*j] = bad;
 
	    } else {
 
	      //	      zint[i+nx*j]= -((zu[i+step+nx*j]-zu[i-step+nx*j])/(step*2.0*dxkm*1000.0)
	      //	      + (zv[i+nx*(j+step)]-zv[i+nx*(j-step)])/(step*2.0*dykm*1000.0))*1.0e6;
 
	      zint[i+nx*j]= ((zu[i+step+nx*j]-zu[i-step+nx*j])/(step*2.0*dxkm)
			      + (zv[i+nx*(j+step)]-zv[i+nx*(j-step)])/(step*2.0*dykm));
 


	    }
 
	  }
	}
     



	// Fill in convergance matrix around the edges.

	for (i=1;i<nx-1;i++){
	  //
	  // Fill in the top and bottom.
	  //
	  float topFillValue = zint[i+nx*step];
	  float bottomFillValue = zint[i+nx*(ny - 1 - step)];
	  //
	  for (int l=0; l < step; l++){
	    zint[i + nx*l] = topFillValue;
	    zint[i + nx*(ny - 1 - l)] = bottomFillValue;
	  }
	}

	for (j=0; j < ny; j++){
	  //
	  // Fill in the left and the right.
	  //
	  float leftFillValue = zint[step + nx*j];
	  float rightFillValue = zint[nx - 1 - step + nx*j];
	  //
	  for (int l=0; l < step; l++){
	    zint[l + nx*j] = leftFillValue;
	    zint[nx - 1 - l + nx*j] = rightFillValue;
	  }

	}

      } 
    
      //////////////// End of convergance - Sea Level Corrected ceiling calc.


     float x,y;

      if (params._desired_fields[ifield] == 185){
	PMU_auto_register("Calculating SL corrected ceiling");

	if (params.debug) {
	  fprintf(stderr," Calculating SL corrected ceiling\n");
	}

	sprintf(name,"%s","CloudHt"); sprintf(units,"%s","Ft");

	if (!GotCeil){
	  fprintf(stderr,"Sea level corrected ceiling listed before ceiling.\n");
	  fprintf(stderr,"I cannot cope.\n");
	  exit(-1);
	}

	// Get the terrain data.

	Terrain *T = new Terrain(params.terrain_file);

	if (T->Nx == -1){
	  fprintf(stderr,"Failed to open terrain file %s\n",params.terrain_file);
	  exit(-1);
	}       


	// loop through, getting the lat/lon of each point
	// and subtracting the elevation from the ceiling.

	int OutCount = 0;
	for (i=0; i<nx; i++){
	  for (j=0; j<ny; j++){

	    if (!(params.flat)){

	      lon = params.lon_origin + i * params.dx;
	      lat = params.lat_origin + j * params.dy;

	    } else {
	      
	      double lat2,lon2;
	      
	      x=x0+i * params.dx; 
	      y=y0+j * params.dy;


	      PJGLatLonPlusDxDy(double(params.lat_origin), 
				double(params.lon_origin),
				double(x), double(y),
				&lat2, &lon2);   


	      lat=float(lat2); lon=float(lon2);
	      
	    }

	    if (zcld_height[i+j*nx] == bad){
	      zslceil[i+j*nx]=bad;
	    } else {
	      zslceil[i+j*nx] = zcld_height[i+j*nx] - M2Ft*T->Elevation(lat,lon);

	      if (T->OutsideGrid){
		if (params.AllowOutsideTerrain){
		  OutCount++;
		} else {
		  fprintf(stderr,"Terrain file %s does not cover point (%f, %f).\n",
			  params.terrain_file,lat,lon);
		  exit(-1);
		}
		
	      }
	    }
	    //
	    // Put it into zint as well for writing.
	    //
	    zint[i+j*nx]=zslceil[i+j*nx];
	  }

	}


	// Free the terrain data.

	delete T;

	GotSLCeil = true;

	if (OutCount) {
	  if (debug) {
	    fprintf (stderr,"%d of %d points were outside of region.\n",
		     OutCount, npoints);
	  }
	}


      }




      //////////////// End of Sea Level Corrected ceiling - VFR calc.

      if (params._desired_fields[ifield] == 186){
	PMU_auto_register("Calculating Flight Cat");

	if (params.debug) {
	  fprintf(stderr," Calculating Flight Cat\n");
	}

	sprintf(name,"%s","Flight_Cat"); sprintf(units,"%s","None");

	if ( (!GotSLCeil) || (!GotVis)){
	  fprintf(stderr,"VFR listed before sea level ceiling or visibility.\n");
	  fprintf(stderr,"I cannot cope.\n");
	  exit(-1);
	}


	if ((params.MaxAltError > 0.0) && ((!(GotAlt)) || (!(GotTerrain))) ){
	  fprintf(stderr,"VFR listed before alt or terrain and check requested.\n");
	  fprintf(stderr,"I cannot cope.\n");
	  exit(-1);
	}

	for (i=0; i<npoints; i++){

	  zvfr[i]=0.0;

	  float Ceiling;

	  if (zslceil[i] == bad) {
	    if (params.BadCeilingValue < 0.0 ) {
	      Ceiling = bad;
	    } else {
	      Ceiling = params.BadCeilingValue;
	    }
	  } else {
	    Ceiling = zslceil[i];
	  }

   	  if (Ceiling == bad) zvfr[i]=bad;
	  if (zvis[i] == bad) zvfr[i]=bad;

	  if (params.MaxAltError > 0.0){ // check terrain vs. alt
	    if (fabs(zalt[i]-zterrain[i]) > params.MaxAltError){
	      zvfr[i]=bad; // Failed terrain check, mark it bad.
	    }
	  }

	  if (zvfr[i]!=bad){ // Not marked bad, put correct value in.
	    zvfr[i]=10.0;

	    for (int k=0; k < params.VisThresh_n; k++){
	      if ((Ceiling >= params._CeilThresh[k]) && 
		  (zvis[i] >= params._VisThresh[k] )){
		zvfr[i]=zvfr[i]+10.0;
	      }
	    }
	  }
	}

	// Invoke median filter if desired.
	if ((params.WindowSize > 0) && (params.NumPasses > 0)){
	  Filt2DMedian(zvfr, nx, ny, params.WindowSize, params.NumPasses,
		       bad);
 	}

	for (i=0; i<npoints; i++) zint[i]=zvfr[i];

      }


      //////////////// End of Sea Level Corrected ceiling - Terrain.
      // This is really a debugging exercise - but it may be nice to
      // check the terrain data used.

      if (params._desired_fields[ifield] == 5){
	PMU_auto_register("Getting terrain");

	if (debug) {
	  fprintf(stderr," Getting Terrain\n");
	}

	sprintf(name,"%s","Terrain"); sprintf(units,"%s","Ft");

	// Get the terrain data.

	Terrain TT(params.terrain_file);

	if (TT.Nx == -1){
	  fprintf(stderr,"Failed to open terrain file %s\n",params.terrain_file);
	  exit(-1);
	}       


	// loop through, getting the lat/lon of each point
	// and subtracting the elevation from the ceiling.

	for (i=0; i<nx; i++){
	  for (j=0; j<ny; j++){

	    if (!(params.flat)){

	      lon = params.lon_origin + i * params.dx;
	      lat = params.lat_origin + j * params.dy;

	    } else {

	      double lat2,lon2;

	      x=x0+i * params.dx; 
	      y=y0+j * params.dy;


	      PJGLatLonPlusDxDy(double(params.lat_origin), 
				double(params.lon_origin),
				double(x), double(y),
				&lat2, &lon2);   


	      lat=float(lat2); lon=float(lon2);

	    }

	    zint[i+j*nx] = M2Ft*TT.Elevation(lat,lon);
	    // Store for VFR calc
	    zterrain[i+j*nx] = M2Ft*TT.Elevation(lat,lon);

	  }

	}

	GotTerrain=true;
      }

      //////////////// End of derived fields.

      PMU_auto_register("Writing to mdv file");

      QQ.AddField(name,name, units,
		  ifield, params.desired_fields_n,
		  zint, bad,0, // Note that missing is a byte values.
		  dx,dy,0.0,
		  x0,y0,0.0,
		  params._desired_fields[ifield]);

    }

    PMU_auto_register("Writing output file.");

    // Write the file.

    if (QQ.write(params.output_url)) {
      cerr << "ERROR - GetDataAndWrite" << endl;
    }


    free( zint );
    free( zu );
    free( zv );
    free( zvis );
    free( zvfr );
    free( zcld_height );
    free( zslceil );
    free( zalt );
    free( zterrain );
    
    free( datatemps );
    free( xsta );
    free( ysta );
    


    return;
    

}














