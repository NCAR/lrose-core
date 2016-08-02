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
#include <cstdio>
#include <cstdlib> 
#include <string.h>  

#include "GiniPDB.hh"
#include "mono_gif.hh"
using namespace std;

int main(int argc, char *argv[] ){

  char InFileName[256];

  if (argc > 1){
    sprintf(InFileName,"%s",argv[1]);
  } else {
    fprintf(stdout,"Input file name --->");
    fscanf(stdin, "%s", InFileName);
  }

  char OutFileName[256];
  if (argc > 2){
    sprintf(OutFileName,"%s",argv[2]);
  } else {
    fprintf(stdout,"Output GIF file name --->");
    fscanf(stdin, "%s", OutFileName);
  }

  int ReverseScale;
  if (argc > 3){
    ReverseScale = atoi(argv[3]);
  } else {
    fprintf(stdout,"Reverse scale [0=No (Visible), 1=Yes (IR)] --->");
    fscanf(stdin, "%d", &ReverseScale);
  }





  FILE *fp = fopen(InFileName,"rb");
  if (fp == NULL){
    fprintf(stderr,"%s not found.\n",InFileName);
    exit(-1);
  }

  fprintf(stderr,"Processing %s\n",InFileName);

  ///////////////////////////////////////

  char c;

  fread(&c,sizeof(char),1,fp);

  fprintf(stderr,"Data type : %c : ",c);

  if (c == 'T'){
    fprintf(stderr,"Satellite imagery.\n");
  } else {
    fprintf(stderr,"Unknown!\n");
  }

  ///////////////////////////////////////

  fread(&c,sizeof(char),1,fp);
  fprintf(stderr,"Data type : ");

  switch((int)c){

  case (int)'A' :
    fprintf(stderr,"Atmospheric parameters.\n");
    break;
    
  case (int)'D' :
    fprintf(stderr,"Data bases.\n");
    break;
  
  case (int)'I' :
    fprintf(stderr,"Images.\n");
    break;
  
  case (int)'O' :
    fprintf(stderr,"Ocean surface.\n");
    break;

  default :
    fprintf(stderr,"UNKNOWN : %c\n",c);
    break;
  
  }


  ///////////////////////////////////////

  char source;
  fread(&source,sizeof(char),1,fp);
  fprintf(stderr,"Data source : ");

  switch((int)source){

  case (int)'C' :
    fprintf(stderr,"Composite.\n");
    break;

  case (int)'D' :
    fprintf(stderr,"DMSP.\n");
    break;

  case (int)'E' :
    fprintf(stderr,"ERS.\n");
    break;

  case (int)'G' :
    fprintf(stderr,"GOES.\n");
    break;

  case (int)'J' :
    fprintf(stderr,"JERS.\n");
    break;

  case (int)'K' :
    fprintf(stderr,"GMS.\n");
    break;

  case (int)'M' :
    fprintf(stderr,"Meteosat.\n");
    break;

  case (int)'T' :
    fprintf(stderr,"TIROS (POES/NPOESS).\n");
    break;

  case (int)'Q' :
    fprintf(stderr,"Quickscat.\n");
    break;


  default :
    fprintf(stderr,"UNKNOWN : %c\n",c);
    break;
  
  }


  ///////////////////////////////////////
  fread(&c,sizeof(char),1,fp);
  fprintf(stderr,"Area/SubType : ");
  
  char AreaName[256];
 
  switch((int)c){

  case (int)'A' :
    sprintf(AreaName,"%s","AlaskaRegional");
    break;

  case (int)'B' :
    sprintf(AreaName,"%s","AlaskaNational");
    break;

  case (int)'C' :
    sprintf(AreaName,"%s","Altimeter");
    break;

  case (int)'D' :
    sprintf(AreaName,"%s","TropicalDiscussion");
    break;

  case (int)'E' :
    sprintf(AreaName,"%s","EastCONUS");
    break;

  case (int)'F' :
    sprintf(AreaName,"%s","NorthernHemisphere");
    break;

  case (int)'G' :
    sprintf(AreaName,"%s","GlobalAreaCoverage");
    break;

  case (int)'H' :
    sprintf(AreaName,"%s","HawaiiRegional");
    break;

  case (int)'I' :
    sprintf(AreaName,"%s","HawaiiNational");
    break;

  case (int)'J' :
    sprintf(AreaName,"%s","PrecipitationEstimates");
    break;

  case (int)'K' :
    sprintf(AreaName,"%s","Scatterometer");
    break;

  case (int)'L' :
    sprintf(AreaName,"%s","LocalAreaCoverage");
    break;

  case (int)'M' :
    sprintf(AreaName,"%s","SolarEnvironmentMonitor");
    break;

  case (int)'N' :
    sprintf(AreaName,"%s","SuperNational");
    break;

  case (int)'O' :
    sprintf(AreaName,"%s","OceanColor");
    break;

  case (int)'P' :
    sprintf(AreaName,"%s","PuertoRicoRegional");
    break;

  case (int)'Q' :
    sprintf(AreaName,"%s","PuertoRicoNational");
    break;

  case (int)'R' :
    sprintf(AreaName,"%s","HighResolutionPictureTransmission");
    break;

  case (int)'S' :
    sprintf(AreaName,"%s","Soundings");
    break;

  case (int)'T' :
    sprintf(AreaName,"%s","SeaSurfaceTemperatures");
    break;

  case (int)'U' :
    sprintf(AreaName,"%s","Winds");
    break;

  case (int)'V' :
    sprintf(AreaName,"%s","SSMI");
    break;

  case (int)'W' :
    sprintf(AreaName,"%s","WestCONUS");
    break;

  case (int)'X' :
    sprintf(AreaName,"%s","WeatherFAX");
    break;

  case (int)'Y' :
    sprintf(AreaName,"%s","ASOS_SupplementalCloud");
    break;

  case (int)'Z' :
    sprintf(AreaName,"%s","NotYetDefined");
    break;

  default :
    sprintf(AreaName,"%s","UNKNOWN");
    break;
  
  }

  fprintf(stderr,"%s\n",AreaName);

  ///////////////////////////////////////

  char FieldStr[3];

  fread(FieldStr,sizeof(char),2,fp);
  FieldStr[2]=(char)0;
  int FieldNum = atoi(FieldStr);

  fprintf(stderr,"Field : %d : ",FieldNum);
  
  char FieldName[256];
 

  if  (source == 'G'){
    switch(FieldNum){

    case 1 :
      sprintf(FieldName,"%s","Visible");
      break;

    case 2 :
      sprintf(FieldName,"%s","11_micron");
      break;

    case 3 :
      sprintf(FieldName,"%s","12_micron");
      break;

    case 4 :
      sprintf(FieldName,"%s","3.9_micron");
      break;

    case 5 :
      sprintf(FieldName,"%s","6.7_micron");
      break;

    case 6 :
      sprintf(FieldName,"%s","13_micron");
      break;

    case 7 :
      sprintf(FieldName,"%s","1.3_micron");
      break;

    case 13 :
      sprintf(FieldName,"%s","ImagerLI");
      break;

    case 14 :
      sprintf(FieldName,"%s","ImagerPrecipWater");
      break;

    case 15 :
      sprintf(FieldName,"%s","ImagerSurfSkinTemp");
      break;

    case 16 :
      sprintf(FieldName,"%s","SounderLI");
      break;

    case 17 :
      sprintf(FieldName,"%s","SounderPrecipWater");
      break;

    case 18 :
      sprintf(FieldName,"%s","SounderSurfSkinTemp");
      break;

    case 19 :
      sprintf(FieldName,"%s","CAPE");
      break;

    case 20 :
      sprintf(FieldName,"%s","LandSeaTemp");
      break;

    case 21 :
      sprintf(FieldName,"%s","WINDEX");
      break;

    case 22 :
      sprintf(FieldName,"%s","DMPI");
      break;

    case 23 :
      sprintf(FieldName,"%s","MDPI");
      break;

    case 24 :
      sprintf(FieldName,"%s","ConvectInhibit");
      break;

    case 25 :
      sprintf(FieldName,"%s","Volcano");
      break;

    case 27 :
      sprintf(FieldName,"%s","CloudTopPres");
      break;

    case 29 :
      sprintf(FieldName,"%s","ManualRainRate");
      break;

    case 30 :
      sprintf(FieldName,"%s","AutoRainRate");
      break;

    case 31 :
      sprintf(FieldName,"%s","SurfWetness");
      break;

    case 32 :
      sprintf(FieldName,"%s","IceConcentration");
      break;

    case 33 :
      sprintf(FieldName,"%s","IceType");
      break;

    case 34 :
      sprintf(FieldName,"%s","IceEdge");
      break;

    case 35 :
      sprintf(FieldName,"%s","CloudWaterContent");
      break;

    case 36 :
      sprintf(FieldName,"%s","SurfaceType");
      break;

    case 37 :
      sprintf(FieldName,"%s","SnowIndicator");
      break;

    case 38 :
      sprintf(FieldName,"%s","SnowWaterContent");
      break;

    case 39 :
      sprintf(FieldName,"%s","DerivedVolcano");
      break;

    case 41 :
      sprintf(FieldName,"%s","Sounder_14.71m");
      break;

    case 42 :
      sprintf(FieldName,"%s","Sounder_14.37m");
      break;

    case 43 :
      sprintf(FieldName,"%s","Sounder_14.06m");
      break;

    case 44 :
      sprintf(FieldName,"%s","Sounder_13.64m");
      break;

    case 45 :
      sprintf(FieldName,"%s","Sounder_13.37m");
      break;

    case 46 :
      sprintf(FieldName,"%s","Sounder_12.66m");
      break;

    case 47 :
      sprintf(FieldName,"%s","Sounder_12.02m");
      break;

    case 48 :
      sprintf(FieldName,"%s","Sounder_11.03m");
      break;

    case 49 :
      sprintf(FieldName,"%s","Sounder_9.71m");
      break;

    case 50 :
      sprintf(FieldName,"%s","Sounder_7.43m");
      break;

    case 51 :
      sprintf(FieldName,"%s","Sounder_7.02m");
      break;

    case 52 :
      sprintf(FieldName,"%s","Sounder_6.51m");
      break;

    case 53 :
      sprintf(FieldName,"%s","Sounder_4.57m");
      break;

    case 54 :
      sprintf(FieldName,"%s","Sounder_4.52m");
      break;

    case 55 :
      sprintf(FieldName,"%s","Sounder_4.45m");
      break;

    case 56 :
      sprintf(FieldName,"%s","Sounder_4.13m");
      break;

    case 57 :
      sprintf(FieldName,"%s","Sounder_3.98m");
      break;

    case 58 :
      sprintf(FieldName,"%s","Sounder_3.74m");
      break;

    case 59 :
      sprintf(FieldName,"%s","SounderVisible");
      break;


    default :
      sprintf(FieldName,"%s","UNKNOWN");
      break;
    
    }

  } else { // Not GOES.

    switch(FieldNum){

    case 1 :
      sprintf(FieldName,"%s","Visible");
      break;

    case 2 :
      sprintf(FieldName,"%s","3.9_micron");
      break;

    case 3 :
      sprintf(FieldName,"%s","6.7_micron");
      break;

    case 4 :
      sprintf(FieldName,"%s","11_micron");
      break;

    case 5 :
      sprintf(FieldName,"%s","12_micron");
      break;

    case 6 :
      sprintf(FieldName,"%s","13_micron");
      break;

    case 7 :
      sprintf(FieldName,"%s","1.3_micron");
      break;

    case 13 :
      sprintf(FieldName,"%s","ImagerLI");
      break;

    case 14 :
      sprintf(FieldName,"%s","ImagerPrecipWater");
      break;

    case 15 :
      sprintf(FieldName,"%s","ImagerSurfSkinTemp");
      break;

    case 16 :
      sprintf(FieldName,"%s","SounderLI");
      break;

    case 17 :
      sprintf(FieldName,"%s","SounderPrecipWater");
      break;

    case 18 :
      sprintf(FieldName,"%s","SounderSurfSkinTemp");
      break;

    case 26 :
      sprintf(FieldName,"%s","Scatterometer");
      break;

    case 27 :
      sprintf(FieldName,"%s","CloudTopPres");
      break;

    case 28 :
      sprintf(FieldName,"%s","CloudAmount");
      break;

    case 29 :
      sprintf(FieldName,"%s","RainRate");
      break;

    case 30 :
      sprintf(FieldName,"%s","SurfWinds");
      break;

    case 31 :
      sprintf(FieldName,"%s","SurfWetness");
      break;

    case 32 :
      sprintf(FieldName,"%s","IceConcentration");
      break;

    case 33 :
      sprintf(FieldName,"%s","IceType");
      break;

    case 34 :
      sprintf(FieldName,"%s","IceEdge");
      break;

    case 35 :
      sprintf(FieldName,"%s","CloudWaterContent");
      break;

    case 36 :
      sprintf(FieldName,"%s","SurfaceType");
      break;

    case 37 :
      sprintf(FieldName,"%s","SnowIndicator");
      break;

    case 38 :
      sprintf(FieldName,"%s","SnowWaterContent");
      break;

    case 39 :
      sprintf(FieldName,"%s","DerivedVolcano");
      break;

    default :
      sprintf(FieldName,"%s","UNKNOWN");
      break;
    
    }

  }

  fprintf(stderr,"%s\n",FieldName);

  ///////////////////////////////////////
  char Origin[6];
  fread(Origin,sizeof(char),5,fp);
  Origin[5]=(char)0;

  fprintf(stderr,"Originating station : ");
  if (strncmp(" KNES",Origin,5)) {
    fprintf(stderr,"%s\n",Origin);
  } else {
    fprintf(stderr,"NESDIS Interface, Camp Springs, Maryland\n");
  }


  ///////////////////////////////////////
  //
  // This 10 byte date string is not in the 
  // documentation that I have - Niles.
  //
  char DateStr[11];

  fread(DateStr,sizeof(unsigned char),10,fp);
  DateStr[10]=(char)0;
  fprintf(stderr,"Date string is : %s\n",DateStr);

  ///////////////////////////////////////

  unsigned  char pdb[512];
  fread(pdb,sizeof(unsigned char),512,fp);

  GiniPDB_t GiniPDB;

  DecodeGiniPDB(pdb, &GiniPDB);
  PrintGiniPDB(GiniPDB, stderr);     
  
  ///////////////////////////////////////

  unsigned char *buffer;

  buffer = (unsigned char *)malloc(sizeof(unsigned char) 
				   * GiniPDB.NumRecords
				   * GiniPDB.SizeRecords);

  if (buffer == NULL){
    fprintf(stderr,"Malloc failed.\n");
    exit(-1);
  }

  int p=fread(buffer,
	      sizeof(unsigned char),
	      GiniPDB.NumRecords*GiniPDB.SizeRecords,
	      fp);

  if (p != GiniPDB.NumRecords*GiniPDB.SizeRecords){
    fprintf(stderr,"Read error : %d vs. %d.\n",p,GiniPDB.NumRecords*GiniPDB.SizeRecords);
    exit(-1);
  }



  ///////////////////////////////////////

  unsigned char uc;

  fread(&uc,sizeof(unsigned char),1,fp);
  if ((int)uc != 255){
    fprintf(stderr,"\nDANGER! Last char was not correct.\n");
  }
  fclose(fp);

  mono_gif(buffer,GiniPDB.Nx,GiniPDB.Ny, OutFileName,ReverseScale,1); 

  free(buffer);

}

