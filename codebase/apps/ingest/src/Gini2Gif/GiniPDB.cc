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

#include "GiniPDB.hh"
using namespace std;

void DecodeGiniPDB(unsigned char pdb[512], GiniPDB_t *GiniPDB){


  /////////////////////////////////////////////////

  if (pdb[0] == 1){
    sprintf(GiniPDB->Source,"%s","NESDIS");
  } else {
    sprintf(GiniPDB->Source,"%s_%d","UNKNOWN",(int)pdb[0]);
  }

  /////////////////////////////////////////////////

  switch ((int)pdb[1]){

  case 6 :
    sprintf(GiniPDB->Creator,"%s","Composite");
    break;

  case 7 :
    sprintf(GiniPDB->Creator,"%s","DMSP");
    break;

  case 8 :
    sprintf(GiniPDB->Creator,"%s","GMS");
    break;

  case 9 :
    sprintf(GiniPDB->Creator,"%s","METEOSAT");
    break;

  case 10 :
    sprintf(GiniPDB->Creator,"%s","GOES-7");
    break;

  case 11 :
    sprintf(GiniPDB->Creator,"%s","GOES-8");
    break;

  case 12 :
    sprintf(GiniPDB->Creator,"%s","GOES-9");
    break;

  case 13 :
    sprintf(GiniPDB->Creator,"%s","GOES-10");
    break;

  case 14 :
    sprintf(GiniPDB->Creator,"%s","GOES-11");
    break;

  case 15 :
    sprintf(GiniPDB->Creator,"%s","GOES-12");
    break;

  default :
    sprintf(GiniPDB->Creator,"%s_%d","UNKNOWN",(int)pdb[1]);
    break;

  }

  /////////////////////////////////////////////////

  switch ((int)pdb[2]){

  case 0 :
    sprintf(GiniPDB->Sector,"%s","NorthHemComposite");
    break;

  case 1 :
    sprintf(GiniPDB->Sector,"%s","EastCONUS");
    break;

  case 2 :
    sprintf(GiniPDB->Sector,"%s","WestCONUS");
    break;

  case 3 :
    sprintf(GiniPDB->Sector,"%s","AlaskaRegional");
    break;

  case 4 :
    sprintf(GiniPDB->Sector,"%s","AlaskaNational");
    break;

  case 5 :
    sprintf(GiniPDB->Sector,"%s","HawaiiRegional");
    break;

  case 6 :
    sprintf(GiniPDB->Sector,"%s","HawaiiNational");
    break;

  case 7 :
    sprintf(GiniPDB->Sector,"%s","PuertoRicoRegional");
    break;

  case 8 :
    sprintf(GiniPDB->Sector,"%s","PuertoRicoNational");
    break;

  case 9 :
    sprintf(GiniPDB->Sector,"%s","SuperNational");
    break;

  case 10 :
    sprintf(GiniPDB->Sector,"%s","GeoNorthHemComp");
    break;

  default :
    sprintf(GiniPDB->Sector,"%s_%d","UNKNOWN",(int)pdb[2]);
    break;

  }

  /////////////////////////////////////////////////
    switch((int)pdb[3]){

    case 1 :
      sprintf(GiniPDB->Channel,"%s","Visible");
      break;

    case 2 :
      sprintf(GiniPDB->Channel,"%s","3.9_micron");
      break;

    case 3 :
      sprintf(GiniPDB->Channel,"%s","6.7_micron");
      break;

    case 4 :
      sprintf(GiniPDB->Channel,"%s","11_micron");
      break;

    case 5 :
      sprintf(GiniPDB->Channel,"%s","12_micron");
      break;

    case 6 :
      sprintf(GiniPDB->Channel,"%s","13_micron");
      break;

    case 7 :
      sprintf(GiniPDB->Channel,"%s","1.3_micron");
      break;

    case 13 :
      sprintf(GiniPDB->Channel,"%s","ImagerLI");
      break;

    case 14 :
      sprintf(GiniPDB->Channel,"%s","ImagerPrecipWater");
      break;

    case 15 :
      sprintf(GiniPDB->Channel,"%s","ImagerSurfSkinTemp");
      break;

    case 16 :
      sprintf(GiniPDB->Channel,"%s","SounderLI");
      break;

    case 17 :
      sprintf(GiniPDB->Channel,"%s","SounderPrecipWater");
      break;

    case 18 :
      sprintf(GiniPDB->Channel,"%s","SounderSurfSkinTemp");
      break;

    case 26 :
      sprintf(GiniPDB->Channel,"%s","Scatterometer");
      break;

    case 27 :
      sprintf(GiniPDB->Channel,"%s","CloudTopPres");
      break;

    case 28 :
      sprintf(GiniPDB->Channel,"%s","CloudAmount");
      break;

    case 29 :
      sprintf(GiniPDB->Channel,"%s","RainRate");
      break;

    case 30 :
      sprintf(GiniPDB->Channel,"%s","SurfWinds");
      break;

    case 31 :
      sprintf(GiniPDB->Channel,"%s","SurfWetness");
      break;

    case 32 :
      sprintf(GiniPDB->Channel,"%s","IceConcentration");
      break;

    case 33 :
      sprintf(GiniPDB->Channel,"%s","IceType");
      break;

    case 34 :
      sprintf(GiniPDB->Channel,"%s","IceEdge");
      break;

    case 35 :
      sprintf(GiniPDB->Channel,"%s","CloudWaterContent");
      break;

    case 36 :
      sprintf(GiniPDB->Channel,"%s","SurfaceType");
      break;

    case 37 :
      sprintf(GiniPDB->Channel,"%s","SnowIndicator");
      break;

    case 38 :
      sprintf(GiniPDB->Channel,"%s","SnowWaterContent");
      break;

    case 39 :
      sprintf(GiniPDB->Channel,"%s","DerivedVolcano");
      break;

    case 41 :
      sprintf(GiniPDB->Channel,"%s","Sounder_14.71m");
      break;

    case 42 :
      sprintf(GiniPDB->Channel,"%s","Sounder_14.37m");
      break;

    case 43 :
      sprintf(GiniPDB->Channel,"%s","Sounder_14.06m");
      break;

    case 44 :
      sprintf(GiniPDB->Channel,"%s","Sounder_13.64m");
      break;

    case 45 :
      sprintf(GiniPDB->Channel,"%s","Sounder_13.37m");
      break;

    case 46 :
      sprintf(GiniPDB->Channel,"%s","Sounder_12.66m");
      break;

    case 47 :
      sprintf(GiniPDB->Channel,"%s","Sounder_12.02m");
      break;

    case 48 :
      sprintf(GiniPDB->Channel,"%s","Sounder_11.03m");
      break;

    case 49 :
      sprintf(GiniPDB->Channel,"%s","Sounder_9.71m");
      break;

    case 50 :
      sprintf(GiniPDB->Channel,"%s","Sounder_7.43m");
      break;

    case 51 :
      sprintf(GiniPDB->Channel,"%s","Sounder_7.02m");
      break;

    case 52 :
      sprintf(GiniPDB->Channel,"%s","Sounder_6.51m");
      break;

    case 53 :
      sprintf(GiniPDB->Channel,"%s","Sounder_4.57m");
      break;

    case 54 :
      sprintf(GiniPDB->Channel,"%s","Sounder_4.52m");
      break;

    case 55 :
      sprintf(GiniPDB->Channel,"%s","Sounder_4.45m");
      break;

    case 56 :
      sprintf(GiniPDB->Channel,"%s","Sounder_4.13m");
      break;

    case 57 :
      sprintf(GiniPDB->Channel,"%s","Sounder_3.98m");
      break;

    case 58 :
      sprintf(GiniPDB->Channel,"%s","Sounder_3.74m");
      break;

    case 59 :
      sprintf(GiniPDB->Channel,"%s","SounderVisible");
      break;

    default :
      sprintf(GiniPDB->Channel,"%s","UNKNOWN");
      break;
    
    }

  GiniPDB->NumRecords = pdb[4]*256 + pdb[5];
  GiniPDB->SizeRecords = pdb[6]*256 + pdb[7];
 
  GiniPDB->Year = (int)pdb[8] + 1900;
  GiniPDB->Month = (int)pdb[9];
  GiniPDB->Day = (int)pdb[10];
  GiniPDB->Hour = (int)pdb[11];
  GiniPDB->Min = (int)pdb[12];
  GiniPDB->Sec = (int)pdb[13];
  GiniPDB->HSec = (int)pdb[14];

  GiniPDB->Projection = (int)pdb[15];

  GiniPDB->Nx = pdb[16]*256 + pdb[17];
  GiniPDB->Ny = pdb[18]*256 + pdb[19];

  double LatDiv, LonDiv;
  if (pdb[20] > 127){
    pdb[20] = pdb[20] - 128;
    LatDiv = -10000.0;
  } else {
    LatDiv = 10000.0;
  }

  if (pdb[23] > 127){
    pdb[23] = pdb[23] - 128;
    LonDiv = -10000.0;
  } else {
    LonDiv = 10000.0;
  }


  GiniPDB->Lat1 = double(pdb[20]*256*256 + pdb[21]*256 + pdb[22])/LatDiv;
  GiniPDB->Lon1 = double(pdb[23]*256*256 + pdb[24]*256 + pdb[25])/LonDiv;

  if (pdb[27] > 127){
    pdb[27] = pdb[27] - 128;
    LonDiv = -10000.0;
  } else {
    LonDiv = 10000.0;
  }

  GiniPDB->Lov = double(pdb[27]*256*256 + pdb[28]*256 + pdb[29])/LonDiv;

  GiniPDB->Dx = double(pdb[30]*256*256 + pdb[31]*256 + pdb[32])/10.0;
  GiniPDB->Dy = double(pdb[33]*256*256 + pdb[34]*256 + pdb[35])/10.0;

  GiniPDB->SouthPole = pdb[36];

  if (pdb[37] > 127){
    pdb[37]=pdb[37]-128;
    GiniPDB->ScanLeftToRight = 0;
  } else {
    GiniPDB->ScanLeftToRight = 1;
  } 

  if (pdb[37] > 63){
    pdb[37]=pdb[37]-64;
    GiniPDB->ScanTopToBottom = 0;
  } else {
    GiniPDB->ScanTopToBottom = 1;
  } 

  if (pdb[37] > 31){
    pdb[37]=pdb[37]-32;
    GiniPDB->ScanInXfirst = 0;
  } else {
    GiniPDB->ScanInXfirst = 1;
  } 


  if (pdb[38] > 127){
    pdb[38] = pdb[38] - 128;
    LatDiv = -10000.0;
  } else {
    LatDiv = 10000.0;
  }
  
  GiniPDB->Latin = double(pdb[38]*256*256 + pdb[39]*256 + pdb[40])/LatDiv;

  GiniPDB->ImageRes =   (int)pdb[41];
  GiniPDB->Compression= (int)pdb[42];
  GiniPDB->CreatorVersion=(int)pdb[43];
  GiniPDB->PDBSize =  pdb[44]*256 + pdb[45];

  if (
      (pdb[46] == 1) ||
      (pdb[46] == 2)
      ){
    GiniPDB->NavIncluded = 1;
  } else {
    GiniPDB->NavIncluded = 0;
  }

  if (
      (pdb[46] == 1) ||
      (pdb[46] == 3)
      ){
    GiniPDB->CalIncluded = 1;
  } else {
    GiniPDB->CalIncluded = 0;
  }


}



void PrintGiniPDB(GiniPDB_t GiniPDB, FILE *out){

  fprintf(out,"PRODUCT DEFINITION BLOCK : \n");
  
  fprintf(out,"SOURCE : %s\n", GiniPDB.Source);
  fprintf(out,"CREATOR : %s\n", GiniPDB.Creator);
  fprintf(out,"SECTOR : %s\n", GiniPDB.Sector);
  fprintf(out,"CHANNEL : %s\n", GiniPDB.Channel);
  
  fprintf(out,"NUMBER OF RECORDS : %d\n", GiniPDB.NumRecords);
  fprintf(out,"SIZE OF RECORDS : %d\n", GiniPDB.SizeRecords);
  
  fprintf(out,"TIME : %02d/%02d/%02d %02d:%02d:%02d.%02d\n",
	  GiniPDB.Year,
	  GiniPDB.Month,
	  GiniPDB.Day,
	  GiniPDB.Hour,
	  GiniPDB.Min,
	  GiniPDB.Sec,
	  GiniPDB.HSec);

  fprintf(out,"PROJECTION : ");
  switch (GiniPDB.Projection){

  case GiniMercatorProj :
    fprintf(out,"Mercator\n");
    break;

  case GiniLambertProj :
    fprintf(out,"Lambert conformal\n");
    break;

  case GiniPolarStereoProj :
    fprintf(out,"Polar Stereographic\n");
    break;

  default :
    fprintf(out,"UNKNOWN! CODE %d\n",GiniPDB.Projection);
    break;
   
  }

  fprintf(out,"IMAGE DIMENSIONS : %d BY %d\n",GiniPDB.Nx, GiniPDB.Ny);
  fprintf(out,"LATITUDE, LONGITUDE OF FIRST GRID POINT : %g, %g\n",
	  GiniPDB.Lat1, GiniPDB.Lon1);

  fprintf(out,"ORIENTATION LONGITUDE : %g\n", GiniPDB.Lov);
  fprintf(out,"GRID INCREMENT IN X,Y : %g, %g\n", GiniPDB.Dx, GiniPDB.Dy);

  fprintf(out,"POLE : ");
  if (GiniPDB.SouthPole){
    fprintf(out,"SOUTH\n");
  } else {
    fprintf(out,"NORTH\n");
  }

  fprintf(out,"TRUE LATITUDE : %g\n", GiniPDB.Latin);

  fprintf(out,"SCAN LEFT TO RIGHT : ");
  if (GiniPDB.ScanLeftToRight){
    fprintf(out,"YES\n");
  } else {
    fprintf(out,"NO\n");
  }

  fprintf(out,"SCAN TOP TO BOTTOM : ");
  if (GiniPDB.ScanTopToBottom){
    fprintf(out,"YES\n");
  } else {
    fprintf(out,"NO\n");
  }

  fprintf(out,"SCAN IN X FIRST, IE. X CHANGES FASTEST IN MEMORY : ");
  if (GiniPDB.ScanInXfirst){
    fprintf(out,"YES\n");
  } else {
    fprintf(out,"NO\n");
  }

  fprintf(out,"IMAGE RESOLUTION CODE : %d\n", GiniPDB.ImageRes);

  fprintf(out,"COMPRESSION : ");
  if (GiniPDB.Compression){
    fprintf(out,"ON\n");
  } else {
    fprintf(out,"OFF\n");
  }

  fprintf(out,"CREATOR VERSION : %d\n", GiniPDB.CreatorVersion);
  fprintf(out,"PRODUCT DEFINITION BLOCK SIZE : %d\n", 
	  GiniPDB.PDBSize);

  fprintf(out,"NAVIGATION : ");
  if (GiniPDB.NavIncluded){
    fprintf(out,"Included\n");
  } else {
    fprintf(out,"Not included\n");
  }

  fprintf(out,"CALIBRATION : ");
  if (GiniPDB.CalIncluded){
    fprintf(out,"Included\n");
  } else {
    fprintf(out,"Not included\n");
  }


}




















