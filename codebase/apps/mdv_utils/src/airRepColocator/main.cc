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
#include <signal.h>
#include <toolsa/port.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>

#include <Spdb/DsSpdb.hh>

#include <rapformats/pirep.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxProj.hh>


#include "Params.hh"

using namespace std;

//
// Function needed for clean exit.
//
void cleanExit( int signal );
void getExpandedCubes(Params *P,
		      int numCubes,
		      double *expandedCube, time_t airmetTime,
		      double lat,  double lon, double alt,
		      int *t_error);
void usage();

int main( int argc, char **argv )
{

  //
  // Process the CLI args.
  //
  int startDay=0, startMonth=0, startYear=0;
  //
  for (int i=1; i < argc; i++){

    if (!(strcmp("-h", argv[i]))){
      usage();
      exit(0);
    }
    
    if (!(strcmp("-start", argv[i]))){
      if (i == argc-1){
	usage();
	exit(-1);
      }

      if (3 != sscanf(argv[i+1], "%d %d %d", &startYear, &startMonth, &startDay)){
	usage();
	exit(-1);
      }
      
    }

  }

   //
   // Trap signals for a clean exit
   //
   PORTsignal( SIGINT,  cleanExit );
   PORTsignal( SIGTERM, cleanExit );
   PORTsignal( SIGQUIT, cleanExit );
   PORTsignal( SIGKILL, cleanExit );

   Params P;

   if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  
  PMU_auto_init("airRepColocator", P.Instance,
                PROCMAP_REGISTER_INTERVAL);     

  //
  // Make sure the output directory exists.
  //
  if (ta_makedir_recurse( P.OutDir )){
    cerr << "Failed to create directory " << P.OutDir << endl;
  }
  //
  // Get the start and end times - yesterday, unless a start
  // time was specified.
  //
  time_t now;
  if (startYear == 0){
    now = time(NULL) - 86400;
  } else {
    date_time_t S;
    S.sec=0; S.min=0; S.hour = 0;
    S.year = startYear; S.month = startMonth; S.day = startDay;
    uconvert_to_utime( &S );
    now = S.unix_time;
  }

  date_time_t T;
  T.unix_time = now;
  uconvert_from_utime( &T );

  T.hour = 0; T.min = 0; T.sec = 0;
  uconvert_to_utime( &T );

  time_t startTime = T.unix_time;
  time_t endTime = startTime + 86399;

  T.unix_time = startTime;
  uconvert_from_utime( &T );

  if (P.Debug){
    cerr << "Looking at time interval " << utimstr(startTime);
    cerr << " to " << utimstr(endTime) << endl;
  }

  DsSpdb airRepMgr;

  airRepMgr.getInterval(P.airRepUrl, startTime, endTime);

  int nChunks = airRepMgr.getNChunks();

  char outFileName[1024];
  
  sprintf(outFileName,"%s/airRepColocator_%04d%02d%02d.dat",
	  P.OutDir, T.year, T.month, T.day);

  double *expandedCubes = (double *)malloc(P.numCubes*sizeof(double));
  if (NULL==expandedCubes){
    cerr << "malloc failed!" << endl;
    exit(-1);
  }
  

  FILE *ofp = fopen(outFileName, "w");
  if (ofp == NULL){
    cerr << "Failed to create " << outFileName << endl;
    exit(-1);
  }

  if (P.Debug){
    cerr << "Writing to " << outFileName << endl;
  }

  for (int ic=0; ic < nChunks; ic++){

    pirep_t *PR = ( pirep_t *) airRepMgr.getChunks()[ic].data;

    BE_to_pirep( PR );

     if (P.Debug){
       cerr << " Point " << ic << " ";
       cerr << utimstr( PR->time ) << " : ";
       cerr << " " << PR->lat;
       cerr << ", " << PR->lon;
       cerr << " " << PR->alt;
       cerr << " feet. Comments : " << PR->text << endl;
     }

     //
     // Now look up the expanded cube values for the lat, lon alt.
     //

     int t_error;
     getExpandedCubes(&P, P.numCubes, expandedCubes, PR->time,
		       PR->lat,  PR->lon,  PR->alt, &t_error);
     
     date_time_t amt;
     amt.unix_time = PR->time;
     uconvert_from_utime( &amt );

     fprintf(ofp,"%04d %02d %02d %02d %02d %02d ",
	     amt.year, amt.month, amt.day,
	     amt.hour, amt.min,   amt.sec);

     fprintf(ofp,"%g\t%g\t%g\t",
	     PR->lat,  PR->lon,  PR->alt);

     fprintf(ofp,"%d\t", t_error );

     for (int i=0; i < P.numCubes; i++){
       fprintf(ofp,"%g\t", expandedCubes[i]);
       fprintf(ofp,"\t");
     }

     if (!(P.numericOnly)){
       fprintf(ofp,"%s\n", PR->text);
     }
  }

  fclose (ofp);

  free(expandedCubes);

  cleanExit(0);
  return 0;
  
}

//
/////////////////////////////////////////////////////
//

void cleanExit( int signal )
{
   //
   // Unregister with process mapper and exit
   //
   PMU_auto_unregister();
   exit( signal );
}              
//
/////////////////////////////////////////////////////
// 
// Look up data for MDV
//
void getExpandedCubes(Params *P,
		      int numCubes,
		      double *expandedCube, time_t airmetTime,
		      double lat,  double lon, double alt,
		      int *t_error){

  //
  // Init all cubes to bad value.
  //
  for (int i=0; i < numCubes; i++){
    expandedCube[i] = -999.0;
  }
  *t_error = -999;

  //
  // Read the MDV data.
  //
  DsMdvx New;

  New.setReadTime(Mdvx::READ_CLOSEST, P->turbUrl, P->maxTimeDiff, 
		  airmetTime );
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  New.addReadField(P->fieldName);
  
  if (New.readVolume()){
    if (P->Debug){
      cerr << "Read failed at " << utimstr(airmetTime) << " from ";
      cerr << P->turbUrl  << endl;
    }
    return;
  }     
  
  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  
  *t_error = airmetTime - InMhdr.time_centroid;
  
  MdvxField *InField = New.getFieldByName( P->fieldName );
  if (InField == NULL){
    cerr << "Input field " << P->fieldName << " not found." << endl;
    return;
  }

  Mdvx::field_header_t InFhdr = InField->getFieldHeader();
  Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();
  
  MdvxProj Proj(InMhdr, InFhdr);

  //
  // Get the horizontal indicies.
  //
  int ixc, iyc;
  if (Proj.latlon2xyIndex(lat, lon, ixc, iyc)){
     if (P->Debug) {
       cerr << "Point " << lat << ", " << lon << " outside of grid." << endl;
       return;
     }
  }
  
  //
  // Find the correct vertical level.
  //
  int first = 1;
  double minVertOff = 0.0;
  int izc = 0;

  for (int iz = 0; iz < InFhdr.nz; iz++){
    if (first){
      izc = iz;
      minVertOff = fabs(InVhdr.level[iz]*100 - alt); // Factor of 100 between FL and feet
      first = 0;
    } else {
      if (minVertOff > fabs(InVhdr.level[iz]*100 - alt)){
	 izc = iz;
	 minVertOff = fabs(InVhdr.level[iz]*100 - alt);
      }
    }
  }
  
  if (P->Debug){
    cerr << "Using MDV level " << InVhdr.level[izc] << " for airmet level " << alt << endl;
  }

  fl32 *InData = (fl32 *) InField->getVol();

  //
  // Fill in the cubes.
  //

  for (int icube=0; icube < numCubes; icube++){

    int startX = ixc - icube; int endX = ixc + icube;
    int startY = iyc - icube; int endY = iyc + icube;
    int startZ = izc - icube; int endZ = izc + icube;


    first = 1;
    for (int ix=startX; ix < endX+1; ix++){
      for (int iy=startY; iy < endY+1; iy++){
	for (int iz=startZ; iz < endZ+1; iz++){
	  if (
	      (ix < 0) || (iy < 0) || (iz < 0) ||
	      (ix > InFhdr.nx-1) ||
	      (iy > InFhdr.ny-1) ||
	      (iz > InFhdr.nz-1)
	      ){
	    continue; // Box is outside grid.
	  }

	  int index = ix + iy*InFhdr.nx + iz*InFhdr.nx*InFhdr.ny;

	  if (
                (InData[index] == InFhdr.bad_data_value) ||
                (InData[index] == InFhdr.missing_data_value)
                ){
	    continue; // Data point is bad.
	  }

	  if (first){
	    expandedCube[icube] = InData[index];
	    first = 0;
	  } else {
	    if (InData[index] > expandedCube[icube]){
	      expandedCube[icube] = InData[index]; // Take the maximum from the cube.
	    }
	  }
	}
      }
    }
  }

  return;

}

 void usage(){
   cerr << "USAGE : airRepColocator [-h] [-start \"YYYY MM DD\"]" << endl;
   cerr << "Default is to process yesterday's data. Niles Oien." << endl;
   cerr << "The -print_params option also can be helpful (and describes";
   cerr << " the file fomat)." << endl;
   return;
 }







