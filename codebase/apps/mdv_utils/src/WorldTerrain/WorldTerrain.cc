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

#include <Mdv/MdvxField.hh>
#include <Mdv/DsMdvx.hh>
#include <toolsa/umisc.h>
#include <cstdio>
#include <math.h>
using namespace std;


void GetFilename(double Lat, double Lon, 
		 int *Nx, int *Ny,
		 double *Dx, double *Dy,
		 double *ULX, double *ULY,
		 char *FileName);

int GetValue(char *TopDir, char *FileName, int offset);

int main(int argc, char *argv[])
{

  double Lat,Lon;
  int Nx,Ny;
  double Dx,Dy,ULX,ULY;

  double DistLat,DistLon;
  int idx,idy, offset;
  int j,k;


  char FileName[MAX_PATH_LEN];
  char TopDir[MAX_PATH_LEN];
  char OutFileName[MAX_PATH_LEN];

  double OutLat,OutLon,OutDx,OutDy;
  int OutNx,OutNy;

  float *data;

  if (argc < 9){
    fprintf(stderr,"USAGE : WorldTerrain TerrainDataDir LowerLeftLat "
                   "LowerLeftLon Nx Ny Dx Dy OutURL\n\n");
    exit(-1);
  } 

  sprintf(TopDir,"%s",argv[1]);
  OutLat = atof(argv[2]);  OutLon = atof(argv[3]);
  OutNx = atoi(argv[4]);   OutNy = atoi(argv[5]);
  OutDx = atof(argv[6]);   OutDy = atof(argv[7]);
  sprintf(OutFileName,"%s",argv[8]);


  data = (float *) malloc(OutNx * OutNy * sizeof(float));

  for (j=0; j<OutNy; j++){
    Lat = OutLat + j * OutDy;
    fprintf(stdout,"Latitude : %g\n",Lat);

    for (k=0; k<OutNx; k++){
      Lon = OutLon + k * OutDx;

      GetFilename(Lat,Lon,
		  &Nx, &Ny,
		  &Dx, &Dy,
		  &ULX, &ULY,
		  FileName);

      DistLat = ULY - Lat;
      DistLon = Lon - ULX;

      idx= (int)rint(DistLon / Dx);
      idy= (int)rint(DistLat / Dy);

      if (idx > Nx-1) idx = Nx-1;
      if (idy > Ny-1) idy = Ny-1;
      if (idx < 0) idx = 0;
      if (idy < 0) idy = 0;

      offset = idy * Nx + idx;
      
      data[j*OutNx + k] = GetValue(TopDir,FileName,offset);

    }
  }


  DsMdvx Out;   

  Mdvx::master_header_t OutMhdr;

  time_t Now;
  Now=time(NULL);;

  OutMhdr.time_gen = Now;
  OutMhdr.user_time = Now;
  OutMhdr.time_begin = Now;
  OutMhdr.time_end = Now;
  OutMhdr.time_centroid = Now;


  OutMhdr.time_expire = Now;


  OutMhdr.data_collection_type = Mdvx::DATA_MEASURED;
  
  OutMhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  OutMhdr.vlevel_included = 1;
               

  OutMhdr.n_fields = 1;
  OutMhdr.max_nx = OutNx;
  OutMhdr.max_ny = OutNy;
  OutMhdr.max_nz = 1; 


  OutMhdr.sensor_lon = OutLon; 
  OutMhdr.sensor_lat = OutLat;   
  OutMhdr.sensor_alt = 0.0; 
      
  sprintf(OutMhdr.data_set_info,"%s","Elevation");
  sprintf(OutMhdr.data_set_name,"%s","Elevation");
  sprintf(OutMhdr.data_set_source,"%s","USGS");


  Out.setMasterHeader(OutMhdr);

  Out.clearFields();     

  Mdvx::field_header_t Fhdr;
  MEM_zero(Fhdr);

  Mdvx::vlevel_header_t Vhdr;
  MEM_zero(Vhdr);               


  Fhdr.struct_id = Mdvx::FIELD_HEAD_MAGIC_COOKIE_32;
  Fhdr.field_code = 5;

  Fhdr.user_time1 =     Now;
  Fhdr.forecast_delta = 0; 
  Fhdr.user_time2 =     Now;
  Fhdr.user_time3 =     Now;
  Fhdr.forecast_time =  Now;
  Fhdr.user_time4 =     Now; 
  Fhdr.nx =  OutNx;
  Fhdr.ny =  OutNy;
  Fhdr.nz =  1;

  Fhdr.proj_type = Mdvx::PROJ_LATLON;


  // Then reals
  Fhdr.proj_origin_lat =  OutLat;
  Fhdr.proj_origin_lon =  OutLon;
  Fhdr.proj_param[0] = 0.0;
  Fhdr.proj_param[1] = 0.0;
  Fhdr.proj_param[2] = 0.0;
  Fhdr.proj_param[3] = 0.0;
  Fhdr.proj_param[4] = 0.0;
  Fhdr.proj_param[5] = 0.0;
  Fhdr.proj_param[6] = 0.0;
  Fhdr.proj_param[7] = 0.0;
  Fhdr.vert_reference = 0.0;

  Fhdr.grid_dx =  OutDx;
  Fhdr.grid_dy =  OutDy;
  Fhdr.grid_dz =  1;

  Fhdr.grid_minx =  OutLon;
  Fhdr.grid_miny =  OutLat;
  Fhdr.grid_minz =  0;

  Fhdr.bad_data_value = -100.0;
  Fhdr.missing_data_value = -100.0;
  Fhdr.proj_rotation = 0.0;

  // Then characters
  sprintf(Fhdr.field_name_long,"%s","Elevation");
  sprintf(Fhdr.field_name,"%s","Elevation");
  sprintf(Fhdr.units,"%s","m");
  sprintf(Fhdr.transform,"%s"," ");

  Fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  Fhdr.data_element_nbytes = sizeof(fl32);
  Fhdr.volume_size = Fhdr.nx * Fhdr.ny * Fhdr.nz * sizeof(fl32);
  Fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  Fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  Fhdr.scaling_type = Mdvx::SCALING_NONE;     
  
  for (int iz = 0; iz < Fhdr.nz; iz++) {
    Vhdr.type[iz] = Mdvx::VERT_TYPE_SURFACE;
    Vhdr.level[iz] = Fhdr.grid_minz + iz * Fhdr.grid_dz;
  }
  
  // create field

  MdvxField *fld = new MdvxField(Fhdr, Vhdr, (void *)data);
    
  fld->setFieldName("Elevation");
  fld->setUnits("m");
  fld->setFieldNameLong("Elevation");
  
  if (fld->convertRounded(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  
    
  Out.addField(fld);

  cerr << "Writing to " << argv[8] << endl;
  
  Out.writeToPath(argv[8]);


  free(data);

  //
  // Try to zip the DEM files we have used.
  //
  char com[1024];
  sprintf(com,"gzip -f %s/*.DEM", TopDir);
  fprintf(stderr,"Issuing command %s\n", com);
  system(com);
  sleep(1);

  return 0;

}


void GetFilename(double Lat, double Lon, 
		 int *Nx, int *Ny,
		 double *Dx, double *Dy,
		 double *ULX, double *ULY,
		 char *FileName){

  char LatChar, LonChar;
  int LatDeg=0, LonDeg=0;
  int i;


  if ((fabs(Lat) > 90.0) || (fabs(Lon) > 180.0)) {
    sprintf(FileName,"NONE");
    return;
  }

  *Dx=0.00833333333333;
  *Dy=0.00833333333333;
 
  if (Lat > -60.0) {

   *Nx=4800; *Ny=6000;

    if (Lat <= -10.0) 
      LatChar = 'S';
    else
      LatChar = 'N';
    
    if (Lon < 20.0)
      LonChar = 'W';
    else
      LonChar = 'E';

    if (Lat <= -10.0) LatDeg = -10;
    if ((Lat > -10.0) && (Lat < 40.0)) LatDeg = 40;
    if (Lat >= 40.0) LatDeg = 90;
    
    *ULY = (double) LatDeg - *Dy/2.0;
    
    i = (int)floor( (Lon + 180.0) / 40.0 );
    LonDeg = -180 + i*40;

    *ULX = (double) LonDeg + *Dx/2.0;

    if (LonDeg < 0) LonDeg = -LonDeg;
    if (LatDeg < 0) LatDeg = -LatDeg;

  } else {

    *Nx=7200; *Ny=3600;

    *ULY = -60.00416666666667;

    LatChar = 'S';
    LatDeg = 60;

    if (Lon < 60.0)
      LonChar = 'W';
    else
      LonChar = 'E';

    i = (int)floor( (Lon + 180.0) / 60.0 );
    LonDeg = -180 + i*60;

    *ULX = (double) LonDeg + *Dx/2.0;

    if (LonDeg < 0) LonDeg = -LonDeg;

  }


  sprintf(FileName,"%c%03d%c%02d.DEM",
	  LonChar, LonDeg, LatChar, LatDeg);
  return;

}

int GetValue(char *TopDir, char *FileName, int offset){

  char Wholename[MAX_PATH_LEN];
  FILE *ifp;

  short Val;
  unsigned char temp, *b;

  sprintf(Wholename,"%s/%s",TopDir, FileName);
  ifp = fopen(Wholename,"rb");

  if (ifp == NULL){
    //
    // Try to unzip the file - then fail if that
    // does not work.
    //
    char zippedName[MAX_PATH_LEN];
    char com[MAX_PATH_LEN];

    sprintf(zippedName,"%s/%s.gz",TopDir, FileName);

    sprintf(com,"gunzip -f %s", zippedName);

    fprintf(stderr,"Issuing command %s\n", com);

    system(com); sleep(1);
  }

  if (ifp == NULL){
    ifp = fopen(Wholename,"rb");
    if (ifp == NULL){
      fprintf(stderr,"Failed to open %s\n", Wholename);
      exit(-1);
    }
  }

  fseek(ifp,2*offset,SEEK_SET);

  fread(&Val, 1, 2, ifp);
  fclose(ifp);

  b = (unsigned char *) &Val;
  temp = *b;
  *b = *(b+1);
  *(b+1) = temp;

  if (Val == -9999) Val = 0;

  return Val;

}









