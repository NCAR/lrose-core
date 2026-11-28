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
////////////////////////////////////////////////////////////
// Program: mdv2gif_file.c                                //
//                                                        //
// This program creates a gif image from mdv files and    //
// also create subcatchment rainfall totals               //
// Same as mdv2gif, except a extra fubsction              //
// was addaed to print cathment totals to a file          //
// Added by P. Visser 2000-02-20                          //          
//                    2000-11-15                          //
// Programmer: J.F. Stuart                                //
// 1998/09                                                //
////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>

#include "mdv_rdwr_utils.hh"
#include "mdv2gif.hh"
#include "latlon.hh"
#include "drawlib.hh"
#include "bessel.hh"

int main(int argc,char **argv)
{
  char regions_file[100];
  char ringstat[3];

  int Next;

  // Set the input values
  parse_command_line(argc, argv);
  printf("/n n parse command klaar/n");
  // Check the usage
  if(Help_Flag)
  {
    Print_Usage();
    exit(1);
  }

  // Read the params file
  ReadParams();
  // Loop redoing image eveiry 5 minutes
/*  Next = TRUE;
  while(Next)
  {*/
    printf("\n In while loop\n");
    if(RUNTIME_MODE == ARCHIVE)
      Next = FALSE;
    if(RUNTIME_MODE == REALTIME)
      Next = TRUE;

    // Read the data file
    ReadMdv();
    printf("\n Read mdv");
    // Create a new image if a new file was detected
    if(((RUNTIME_MODE == REALTIME) && datainfo.newfile) || (RUNTIME_MODE == ARCHIVE))
    {

      // Set the image
      SetImage();
      printf("\n SetImage");
      Drawmode = SETmode; // Overwrite pixels in gif 
      SetLineType (0);

      // Draw the background orography if selected
      if(Orography_Flag)
      DoOrography(0);

      SetColor(0,0,0);
      // Draw a rectangle around the data
      DrawRect(0,0,GetChartXSize()-1,GetChartYSize()-1);

      SetLineType (0);
      DrawRect(0,0,GetChartXSize()-1,GetChartYSize()-1);

      //  Read the masks and maskout the data if the mask was selected, or
      // just do the subcatchments averages
      printf("\n Doing Mask\n");
      DoMasks();

      // Draw the data on the image if selected
      if(Data_Flag)
        DoData();

      SetColor(0,0,0);
  
      // Draw the catchment boundaries if selected
      if(CatchmentBound_Flag && !Zoom_Flag)
        DoCatchment_Bound();

      // Draw the range rings for the stations available if selected
      if(RangeRings_Flag && !Zoom_Flag)
      {
        int ringcnt;

        for(ringcnt = 0; ringcnt < N_RINGS; ringcnt++)
        {
          memset(ringstat,'\0',3);
          sprintf(ringstat,"%s",RINGNAMES[ringcnt]);
          DoRing_Station(ringstat,0,150,150);
        }
      }// if Rangeringsflag
  
      // Plot the stations if selected
      if(Stations_Flag && !Zoom_Flag)
        PlotStations();

      // Draw the topography scale if selected
      if(Orography_Flag)
      {
        DrawTopoScale();
      }

      // Draw the data scale if selected
      if(Data_Flag)
      {
        DrawDataScale(); 
      }

      // Read the background map if selected
      if(BackGround_Flag && !Zoom_Flag)
      {
        memset(regions_file,'\0',100);
        sprintf(regions_file,"%s%sregions.bkg",SYSPATH,BACKGROUND_PATH);
        ReadBackground(regions_file);
      }

      // Draw District boundaries if selected
      if(DistrictBound_Flag && !Zoom_Flag)
        DoDistrict_Bound();

      // Create and write the label
      // PlotGauges is also done in this part
      printf("\n Zoom flag  %d ",Zoom_Flag);
      if(!Zoom_Flag)
        DoLabel();
      printf("\n LABEL DONE\n");
      // Create the output
      OutPut();

      // Free all the data memory
      Free_Data();

    }// while (datainfo.newfile)
   
    /* Die while is uigehaal, program lopp net een keer deur en gaan uit*/
    /*if(RUNTIME_MODE == REALTIME)
    {
      sleep(NEWFILE_CHECKTIME);
    }*/
  /*} */    // While(Next)

  printf("Freeing\n");
  // Free all the memory
  Free_Data();
  Free_Info();

}//main


/************************************************************************/
/* Read the gauge rainfall and place on gif image                       */
/************************************************************************/
void PlotGauges(datastruct *data_info)
{

 char *char_lat, *char_lon, *char_alt, *char_tot, *station,
    gname[100], tmp[100];
 char month[8],day[8],year[8],hour[8],minute[8],sec[8];
 int imaand;
 char moonth[13][8] = {"Dum","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug",
                "Sep","Oct","Nov","Dec"};

  double lat,lon;
  int x1,y1;
  float latmin, lonmin;
  char No_rain[8];

  FILE *mf;
 
/* Extract month and day from date_line*/
 memset(month,'\0',4);
 memset(year,'\0',5);
 memset(day,'\0',3);
 memset(hour,'\0',3);
 memset(minute,'\0',3);
 memset(sec,'\0',3);
 strncpy(month,&data_info->datatime[4],3);
 strncpy(year,&data_info->datatime[20],4);
 strncpy(day,&data_info->datatime[8],2);
 strncpy(hour,&data_info->datatime[11],2);
 strncpy(minute,&data_info->datatime[14],2);
 strncpy(sec,&data_info->datatime[17],2);

 imaand = 1;
 while ( strncmp(moonth[imaand],month,3) != 0 && imaand < 13)
 {
  imaand++;
 }
 date_info.yr = atoi(year);
 date_info.mnth = imaand;
 date_info.day = atoi(day);
 date_info.hr = atoi(hour);
 date_info.min = atoi(minute);
 date_info.sec = atoi(sec);

  memset ( gname,'\0',100);
  sprintf ( gname, "%s/merge_data/rgauge_daily/g%04d%02d%02d.dat",SYSPATH, date_info.yr, date_info.mnth,date_info.day);

  printf (" gname is %s \n", gname);

  /*
   * Open and read ascii file
   */

  if (( mf = fopen ( gname, "r" )  ) == NULL ) {
    printf( "Can't open input file - %s \n", gname);
    exit(-1);
    }

  for (;;)
  {

    /*
     * unpack data string
     */

    memset ( tmp, '\0', 100);
    fgets ( tmp, 100, mf);

    /*
     * make sure there is new data, strtok keeps residual garbage which
     * can cause a SEGV at the feof
     */

    if ( strlen ( tmp ) <= 25 )
      break;


    /*
     * extract strings with | as delimiter
     */

    station = strtok ( tmp, "|");
    char_lat =  strtok ( NULL, "|");
    char_lon = strtok ( NULL, "|");
    char_alt = strtok ( NULL, "|");
    char_tot = strtok ( NULL, "|");

    lat =atof(char_lat)/100.;
    lon =atof(char_lon)/100.;

    latmin = (lat - (int)lat) * 1.66666;
    lonmin = (lon - (int)lon) * 1.66666;
    
    lat = (int)lat + latmin;
    lon = (int)lon + lonmin;

    //printf("StartLon %f StartLat %f StopLon  %f StopLat %f \n",zoominfo.startlon,zoominfo.startlat,zoominfo.stoplon,zoominfo.stoplat);
    if (feof(mf)) break;
    

    //if ((lon < zoominfo.startlon) || (lon > zoominfo.stoplon)) continue;
    //if ((lat < zoominfo.startlat) || (lat > zoominfo.stoplat)) continue;
    if ((lon < GetStartLon()) || (lon > GetStopLon())) continue;
    if ((lat < GetStartLat()) || (lat > GetStopLat())) continue;

    printf("\n In farm %s %s %s %s",char_lat,char_lon,char_tot,station);
    SetLineType(0);
    SetLineThick(0);
    WorldToScreen(lat,lon, &x1,&y1);
    SetColor(255,255,255);
    if(Zoom_Flag)
    {
      FillRect (x1-2,y1-2+LABELSIZE ,x1+2,y1+2+LABELSIZE );
      SetColor(0,0,0);
      DrawRect (x1-2,y1-2+LABELSIZE,x1+2,y1+2+LABELSIZE);
      SetColor(0,0,0);
    }
    else
    {
      FillRect (x1-2,y1-2+LABELSIZE,x1+2,y1+2+LABELSIZE);
      SetColor(0,0,0);
      DrawRect (x1-2,y1-2+LABELSIZE,x1+2,y1+2+LABELSIZE);
      SetColor(0,0,0);
    }
    strcpy(No_rain,"No Rain");
    if( strncmp(station,No_rain,7) == 0)
      strcpy(station,"  ");
    if(Zoom_Flag)
    {
      SetColor(0,0,0);
      pltsym(x1+5,y1+LABELSIZE,char_tot,1.6,0.0,0);
      SetColor(0,0,0);
    }
    else
    {
      SetColor(0,0,0);
      pltsym(x1+5,y1+LABELSIZE,char_tot,1.6,0.0,0);
      SetColor(0,0,0);
    }
  }
  fclose (mf);

}


/************************************************************************/
/* Read the station file and plot the station name and indicator        */
/************************************************************************/
void PlotStations()
{
  double lat,lon;
  int stasnum;
  int x1,y1;
  FILE *fp;
  char line[81];
  char stasname[81];
  char stasfile[100];

  sprintf (stasfile,"%s%s/staslist.dat",SYSPATH,BACKGROUND_PATH);
  if ((fp = fopen (stasfile,"r")) == NULL)
    return;
  
  for (;;)
  {
    fgets (line,80,fp);
    if (feof(fp)) break;

    sscanf (line,"%d %lf %lf",&stasnum,&lat,&lon);
    if ((lon < GetStartLon()) || (lon > GetStopLon())) continue;
    if ((lat < GetStartLat()) || (lat > GetStopLat())) continue;

    memcpy(stasname,&line[28],15);stasname[15] = 0;
	
    WorldToScreen(lat,lon, &x1,&y1);
    SetColor(255,255,255);
    if(Zoom_Flag)
    {
      FillRect (x1-2,y1-2 + LABELSIZE,x1+2,y1+2 + LABELSIZE);
      SetColor(0,0,0);
      DrawRect (x1-2,y1-2 + LABELSIZE,x1+2,y1+2 + LABELSIZE);
      SetColor(0,0,0);
    }
    else
    {
      FillRect (x1-2,y1-2,x1+2,y1+2);
      SetColor(0,0,0);
      DrawRect (x1-2,y1-2,x1+2,y1+2);
      SetColor(0,0,0);
    }
    if(Zoom_Flag)
    {
      pltsym(x1+5,y1 + LABELSIZE,stasname,STATIONSIZE,0.0,0);
    }
    else
    {
      pltsym(x1+5,y1,stasname,STATIONSIZE,0.0,0);
    }
  }
  fclose (fp);
}


void SetColorVal (double val)
{
  int r2,g2,b2;

  if (val <= 1.0000)
  {
    //r2=255;g2=255;b2=0;
    r2=255;g2=255;b2=255;
  }
  else if (val <= 25)   { r2=6;   g2=255; b2=0; }
  else if (val <= 50)   { r2=12;  g2=255; b2=0; }
  else if (val <= 100)  { r2=18;  g2=255; b2=0; }
  else if (val <= 150)  { r2=25;  g2=255; b2=0; }
  else if (val <= 200)  { r2=37;  g2=255; b2=0; }
  else if (val <= 250)  { r2=50;  g2=255; b2=0; }
  else if (val <= 310)  { r2=62;  g2=255; b2=0; }
  else if (val <= 375)  { r2=75;  g2=255; b2=0; }
  else if (val <= 437)  { r2=87;  g2=255; b2=0; }
  else if (val <= 500)  { r2=100; g2=255; b2=0; }
  else if (val <= 562)  { r2=112; g2=255; b2=0; }
  else if (val <= 625)  { r2=125; g2=255; b2=0; }
  else if (val <= 687)  { r2=137; g2=255; b2=0; }
  else if (val <= 750)  { r2=150; g2=255; b2=0; }
  else if (val <= 812)  { r2=162; g2=255; b2=0; }
  else if (val <= 875)  { r2=175; g2=255; b2=0; }
  else if (val <= 937)  { r2=187; g2=255; b2=0; }
  else if (val <= 1000) { r2=200; g2=255; b2=0; }
  else if (val <= 1067) { r2=213; g2=255; b2=0; }
  else if (val <= 1125) { r2=227; g2=255; b2=0; }
  else if (val <= 1250) { r2=255; g2=255; b2=0; }
  else if (val <= 1312) { r2=255; g2=238; b2=0; }
  else if (val <= 1375) { r2=255; g2=227; b2=0; }
  else if (val <= 1437) { r2=255; g2=213; b2=0; }
  else if (val <= 1500) { r2=255; g2=200; b2=0; }
  else if (val <= 1625) { r2=255; g2=187; b2=0; }
  else if (val <= 1750) { r2=255; g2=175; b2=0; }
  else if (val <= 1875) { r2=255; g2=167; b2=0; }
  else if (val <= 2000) { r2=255; g2=150; b2=0; }
  else if (val <= 2125) { r2=255; g2=137; b2=0; }
  else if (val <= 2250) { r2=255; g2=125; b2=0; }
  else if (val <= 2375) { r2=255; g2=113; b2=0; }
  else if (val <= 2500) { r2=255; g2=100; b2=0; }
  else if (val <= 2675) { r2=255; g2=87;  b2=0; }
  else if (val <= 2750) { r2=255; g2=75;  b2=0; }
  else if (val > 2750)  { r2=255; g2=50;  b2=0; }
  SetColor (r2,g2,b2);
}

void SetRadColorVal (float val)
{
  int r2,g2,b2;

  if(TRUE)
  {
    if (val < 1)
    {
      r2=0;g2=0;b2=0;
    }
    else if (1 < val && val <= 5)   { r2=0;  g2=0;   b2=50;}
    else if (5 < val && val <= 10 )   { r2=0;  g2=50;   b2=150;}
    else if (10 < val && val <= 15)   { r2=0;  g2=100;   b2=240;}
    else if (15 < val && val <= 20)   { r2=30;  g2=150;   b2=255;}
    else if (20 < val && val <= 25)   { r2=60;  g2=180; b2=255;}
    else if (25 < val && val <= 30)   { r2=0;  g2=215; b2=255; }
    else if (30 < val && val <= 40)   { r2=0;  g2=255; b2=255; }
    else if (40 < val && val <= 50)   { r2=110;g2=120; b2=255; }
    else if (50 < val && val <= 75)   { r2=185;g2=165; b2=255; }
    else if (75 < val && val <= 100)  { r2=240;g2=100; b2=255; }
    else if (100 < val && val <= 150)  { r2=255;g2=0;   b2=255; }
    else if (150 < val)  { r2=255;  g2=30; b2=200; }
    //else if (val == 93)  { r2=55;  g2=60; b2=250; }
    //else if (val == 94)  { r2=255;  g2=30; b2=200; }
  }
/*
  if(TRUE)
  {
    if (val < 1)
    {
      r2=0;g2=0;b2=0;
    }
    else if (1 == val)   { r2=0;  g2=0;   b2=50;}
    else if (2 == val)   { r2=0;  g2=50;   b2=150;}
    else if (2 < val && val <= 10)   { r2=0;  g2=100;   b2=240;}
    else if (10 < val && val <= 20)   { r2=30;  g2=150;   b2=255;}
    else if (20 < val && val <= 25)   { r2=60;  g2=180; b2=255;}
    else if (25 < val && val <= 30)   { r2=0;  g2=215; b2=255; }
    else if (30 < val && val <= 40)   { r2=0;  g2=255; b2=255; }
    else if (40 < val && val <= 45)   { r2=110;g2=120; b2=255; }
    else if (45 < val && val <= 50)   { r2=185;g2=165; b2=255; }
    else if (50 < val && val <= 61)  { r2=240;g2=100; b2=255; }
    else if (61 < val && val <= 66)  { r2=255;g2=0;   b2=255; }
    else if (66 < val)  { r2=255;  g2=30; b2=200; }
    else if (val == 93)  { r2=55;  g2=60; b2=250; }
    else if (val == 94)  { r2=255;  g2=30; b2=200; }
  }
*/
/*
  if(comptype == 0)
  {
    if (val < 1)
    {
      r2=0;g2=0;b2=0;
    }
    else if (1 == val)   { r2=0;  g2=0;   b2=50;}
    else if (2 == val)   { r2=0;  g2=50;   b2=150;}
    else if (3 == val)   { r2=0;  g2=100;   b2=240;}
    else if (4 == val)   { r2=30;  g2=150;   b2=255;}
    else if (5 == val)   { r2=60;  g2=180; b2=255;}
    else if (6 == val)   { r2=0;  g2=215; b2=255; }
    else if (7 == val)   { r2=0;  g2=255; b2=255; }
    else if (8 == val)   { r2=110;g2=120; b2=255; }
    else if (9 == val)   { r2=185;g2=165; b2=255; }
    else if (10 == val)  { r2=240;g2=100; b2=255; }
    else if (11 == val)  { r2=255;g2=0;   b2=255; }
    else if (12 == val)  { r2=255;  g2=30; b2=200; }
    else if (val == 93)  { r2=55;  g2=60; b2=250; }
    else if (val == 94)  { r2=255;  g2=30; b2=200; }
  }
*/  
  SetColor (r2,g2,b2);
}

void DrawTopoScale()
{
  int starty,dy;

  dy = (int)((((float)(GetChartYSize()))/2.0)/37.0);

  SetLineThick(0);
  SetColor (255,255,255);
  if(Zoom_Flag)
  {
    FillRect (GetChartXSize(),
	      LABELSIZE + (int) (GetChartYSize() / 2.0 + 0.5),
	      GetChartXSize()+SCALESIZE,
	      LABELSIZE + GetChartYSize());
    SetColor (0,0,0);
    DrawRect (GetChartXSize(),
	      LABELSIZE + (int) (GetChartYSize() / 2.0 + 0.5),
	      GetChartXSize()+SCALESIZE,
	      LABELSIZE + GetChartYSize());
  }
  else
  {
    FillRect (GetChartXSize(),
	       (int) (GetChartYSize() / 2.0 + 0.5),
	      GetChartXSize()+SCALESIZE,
	      GetChartYSize());
    SetColor (0,0,0);
    DrawRect (GetChartXSize(),
	      (int) (GetChartYSize() / 2.0 + 0.5),
	      GetChartXSize()+SCALESIZE,
	      GetChartYSize());
  }

  if(Zoom_Flag)
  {
    starty=GetChartYSize() - dy -2 + LABELSIZE;
  }
  else
  {
    starty=GetChartYSize() - dy -2 + LABELSIZE;
  }
  SetColorVal (25.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;
  SetColor (0,0,0);
  pltsym(GetChartXSize()+5,starty+dy,"  25",1.0,0.0,0);

  SetColorVal (50.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (100.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (150.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (200.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (250.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;
  SetColor (0,0,0);
  pltsym(GetChartXSize()+5,starty+dy," 250",1.0,0.0,0);

  SetColorVal (310.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (375.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (437.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (500.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;
  SetColor (0,0,0);
  pltsym(GetChartXSize()+5,starty+dy," 500",1.0,0.0,0);

  SetColorVal (562.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (625.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (687.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (750.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;
  SetColor (0,0,0);
  pltsym(GetChartXSize()+5,starty+dy," 750",1.0,0.0,0);

  SetColorVal (812.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (875.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (937.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (1000.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;
  SetColor (0,0,0);
  pltsym(GetChartXSize()+5,starty+dy,"1000",1.0,0.0,0);

  SetColorVal (1067.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (1125.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (1250.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;
  SetColor (0,0,0);
  pltsym(GetChartXSize()+5,starty+dy,"1250",1.0,0.0,0);

  SetColorVal (1312.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (1375.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (1437.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (1500.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;
  SetColor (0,0,0);
  pltsym(GetChartXSize()+5,starty+dy,"1500",1.0,0.0,0);

  SetColorVal (1625.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (1750.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;
  SetColor (0,0,0);
  pltsym(GetChartXSize()+5,starty+dy,"1750",1.0,0.0,0);

  SetColorVal (1875.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (2000.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;
  SetColor (0,0,0);
  pltsym(GetChartXSize()+5,starty+dy,"2000",1.0,0.0,0);

  SetColorVal (2125.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (2250.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;
  SetColor (0,0,0);
  pltsym(GetChartXSize()+5,starty+dy,"2250",1.0,0.0,0);

  SetColorVal (2375.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (2500.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;
  SetColor (0,0,0);
  pltsym(GetChartXSize()+5,starty+dy,"2500",1.0,0.0,0);

  SetColorVal (2675.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

  SetColorVal (2750.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;
  SetColor (0,0,0);
  pltsym(GetChartXSize()+5,starty+dy,"2750",1.0,0.0,0);

  SetColorVal (2755.0);
  FillRect (GetChartXSize()+35,starty,GetChartXSize()+SCALESIZE,starty+dy);starty = starty - dy;

}


/************************************************************************/
/* BLACK BOX FUNTIONS You should now have to change this                */
/************************************************************************/


/************************************************************************/
/* Plot the Topography data on the image                                */
/* If seaonly == 1 the only the sea areas will be drawn                 */
/************************************************************************/
void DoOrography(int seaonly)
{
  FILE *fp;
  int startx,stopx,starty,stopy;
  int X_dim,Y_dim; 
  //float stap = (float)1.0;
  int interp = 8;
  int val;
  int i,j,x,y;
  int x1,x2,y1,y2;
  double lon,lat ;
  double color;
  int nuwex,nuwey;
  float interpoleer1,interpoleer2;
  float aax,aay;
  float *datauit;
  float *dat;
  int oux,ouy;
  char fname[81];
  int max,min;
  double newstrtlon,newstplon,newstrtlat,newstplat;

  sprintf (fname,"%s%s/orog.asc",SYSPATH,BACKGROUND_PATH);
  if ((fp = fopen(fname,"r")) == NULL)
  {
    printf("Cannot open file %s",fname);
    return;
  }
  startx = (int)(GetStartLon()*6);
  stopx = (int)(GetStopLon()*6)+1;
  //stopx = (int)(GetStopLon()*6);
  starty = (int)((90 - GetStopLat())*6);
  stopy = ((int)((90 - GetStartLat())*6))+1;
  //stopy = ((int)((90 - GetStartLat())*6))  ;
  X_dim = stopx-startx+1;
  Y_dim = stopy-starty+1;

  newstrtlon = (double)startx/6.0;
  newstplon = (double)stopx / 6.0;
  newstrtlat = 90.0 -  (double)starty/6.0;
  newstplat = 90.0 - (double)stopy/6.0;

  max = 0;
  min = 99999;

  //Read the first lines to skip from the orog.asc file
  for (i=0;i<starty;++i)
    for (j=0;j<2160;++j)
      fscanf(fp,"%d",&val); 
  
  dat = (float *)malloc (X_dim * Y_dim * sizeof(float));
  for (i=0;i<Y_dim;++i)
  {
    for (j=0;j<startx;++j) fscanf (fp,"%d",&val);  // read the skippe columns
    for (j=0;j<X_dim;++j)
    {
       fscanf(fp,"%d",&val);
       dat[(i*X_dim)+j] = (float)val;
	   if (val > max)
		   max = val;
	   if (val < min)
		   min = val;
    }
    for (j=0;j<2160 - (startx + X_dim);++j) fscanf (fp,"%d",&val);  // read the skippe columns
  } 

  oux = X_dim;
  ouy = Y_dim;
  nuwex = ((X_dim - 1) * (interp + 1)) +1;
  //nuwey = ((X_dim - 1) * (interp + 1)) +1;
  nuwey = ((Y_dim - 1) * (interp + 1)) +1;
  interpoleer1 = (float)(1.0/(float)(interp + 1));
  interpoleer2 = (float)0.0;
  datauit = (float *)(malloc ((nuwex) * (nuwey) * sizeof(float)));
  memset (datauit,0,nuwex*nuwey*sizeof(float));

 
  for (y = 0,aay = interpoleer2;y <= (nuwey - 1);y ++,aay += interpoleer1)
    for (x = 0,aax = interpoleer2;x <= (nuwex - 1);x ++,aax += interpoleer1)
      datauit[y * nuwex + x]=(float)Bsslint(dat,&aax,&aay,&oux,&ouy);

  X_dim = nuwex ;
  Y_dim = nuwey ;
 
  for (i=0;i<Y_dim-1;++i)
  {
    for (j=0;j<X_dim-1;++j)
    {
      color = (double)(datauit[(i*X_dim)+j] + 
                       datauit[(i*X_dim)+j+1] +
                       datauit[((i+1)*X_dim)+j] + 
                       datauit[((i+1)*X_dim)+j+1]) / 4.0;
      SetColorVal (color);
      lon = newstrtlon+((double)j/54.0);
      lat = newstrtlat-((double)i/54.0);
      WorldToScreen(lat,lon, &x1,&y1);
//     printf("lon %f lat %f x1 %d y1 %d\n",lon,lat,x1,y1);
      lon = newstrtlon+((double)(j+1)/54.0);
      lat = newstrtlat-((double)(i+1)/54.0);
      WorldToScreen(lat,lon, &x2,&y2);
//     printf("lon %f lat %f x2 %d y2 %d\n",lon,lat,x2,y2);
      if (seaonly == 1)
      {
        if (color <= 1.0)
           FillRect(x1,y1,x2,y2);
      }
      else
        FillRect(x1,y1,x2,y2);
    }
  }
  free (dat);
  free (datauit);
  fclose(fp);
}

void ReadFloat(FILE *fp,float *val)
{
#ifdef UNIX
	char tmp[4],swap[4];
	fread (tmp,1,4,fp);
	swap[0] = tmp[3];
	swap[1] = tmp[2];
	swap[2] = tmp[1];
	swap[3] = tmp[0];
	*val = *((float *)swap);
#endif

#ifdef WIN32
    char tmp[4]
    fread (&val,1,sizeof(float),fp);
#endif

#ifdef LINUX
        char tmp[4];
        fread (tmp,1,4,fp);
        *val = *((double *)tmp);
#endif

/*#else
    fread (&xval,1,sizeof(float),fpin);
#endif
*/
}

void ReadDouble(FILE *fp,double *val)
{
#ifdef UNIX
	char tmp[8],swap[8];
	fread (tmp,1,8,fp);
	swap[0] = tmp[7];
	swap[1] = tmp[6];
	swap[2] = tmp[5];
	swap[3] = tmp[4];
	swap[4] = tmp[3];
	swap[5] = tmp[2];
	swap[6] = tmp[1];
	swap[7] = tmp[0];
	*val = *((double *)swap);
#endif

#ifdef WIN32
    fread (&val,1,sizeof(double),fp);
#endif

#ifdef LINUX
        char tmp[8];
        fread (tmp,1,8,fp);
        *val = *((double *)tmp);
#endif


}



void ReadBackground(char *fname)
{
  FILE *fpin;
  double xval,yval;
  int x1,y1,x2,y2;
  double xscale,yscale;
  
  xscale = (GetStopLon() - GetStartLon())/(double)GetChartXSize();
  yscale = (GetStopLat() - GetStartLat())/(double)GetChartYSize();

  if ((fpin = fopen (fname,"rb")) == NULL)
  {
    printf("Cannot open file %s",fname);
    return;
  }
  for (;;)
  {
    ReadDouble(fpin,&xval);
    ReadDouble(fpin,&yval);
    if (feof(fpin))
      break;

    if (xval < -7000.0)
    { 
       xval = xval+9000.0;
	
       x1 = (int)((xval + (0.0-GetStartLon())) / xscale);
       y1 = GetChartYSize() - 
           (int)((yval + (0.0-GetStartLat())) / yscale);
    }
    else
    {
	  
	  
       x2 = (int)((xval + (0.0-GetStartLon())) / xscale);
       y2 = GetChartYSize() - 
           (int)((yval + (0.0-GetStartLat())) / yscale);
       if ((x1 < 0) || (x1 > GetChartXSize()) || (y1<0) 
          || (y1 > GetChartYSize()))
       {
         x1 = x2;
         y1 = y2;
         continue;
       }
       if ((x2 < 0) || (x2 > GetChartXSize()) || (y2<0) 
          || (y2 > GetChartYSize()))
       {
         x1 = x2;
         y1 = y2;
         continue;
       }
      if(Zoom_Flag)
      {
        DrawLine (x1,y1 + LABELSIZE,x2,y2 + LABELSIZE);
      }
      else
      {
        DrawLine (x1,y1,x2,y2);
      }
      x1 = x2;
      y1 = y2;
    }

  }
  fclose (fpin);
}        


void DoCatchment_Bound()
{
  int i;
  int x1,x2,y1,y2;
  double lon,lat ;
  double newstrtlat;
  double newstrtlon;
  char catchfile[200];
  FILE *fpcatch;
  //float basin_bound[1240][2];
  float **basin_bound;
  int maskcnt;
  int n_lines;
  char buf[200];

  for(maskcnt = 0; maskcnt < N_MASKS; maskcnt++)
  {
   printf("maskcnt %d\n",maskcnt);
    SetLineThick(1);

    // Set up the filename and open it
    memset(catchfile,'\0',200);
    sprintf(catchfile,"%s%s%s",SYSPATH,BACKGROUND_PATH,MASKBOUN[maskcnt]); 
    if((fpcatch = fopen(catchfile,"r")) == NULL)
    {
      printf("Cannot open boundary file %s\n",catchfile);
      return;
    }
    printf("Na open\n");

    // Read the header and allocate memory
    memset(buf,'\0',200);
    fscanf(fpcatch,"%s %d",buf,&n_lines);
    basin_bound = (float **)malloc(n_lines*sizeof(float*));
    for(i = 0; i < n_lines; i++)
    {
      basin_bound[i] = (float *)malloc(2*sizeof(float));
    }
    

    n_lines = 0;
    i = 0;
    //while(i<n_lines)
    while(fscanf(fpcatch,"%f %f",&basin_bound[i][0], &basin_bound[i][1]) != EOF)
    {
  //   printf("lon %.3f lat %.3f\n",basin_bound[i][0], basin_bound[i][1]);
      i++;
    }
    n_lines = i;
    fclose(fpcatch);
    printf("na fscan\n");
 
 
    SetColor(255,0,0);
    newstrtlon = basin_bound[1][0];  
    newstrtlat = basin_bound[1][1];  
    for (i=0;i<n_lines-1;++i)
    {
      lon = basin_bound[i+1][0];
      lat = basin_bound[i+1][1];
  //    printf("lon %f lat %f\n",lon,lat);
      
      WorldToScreen(newstrtlat,newstrtlon, &x1,&y1);
      WorldToScreen(lat,lon, &x2,&y2);
  //    if(lon != 0 && newstrtlon !=0)
  //    if(lon != 0 && newstrtlon != 0 && x1 > 0 && x1 <= 600 && x2 > 0 && 
  //       x2 <= 600 && y1 > 0 && y1 <= 600 && y2 >0 && y2 <= 600)
      if(lon != 0.0 && newstrtlon != 0.0 && x1 > 0 && x1 <= GetChartXSize() 
         && x2 > 0 && x2 <= GetChartXSize() && y1 > 0 
         && y1 <= (GetChartYSize() + SCALESIZE) && y2 >0 
         && y2 <= (GetChartYSize() + SCALESIZE))
      {
        if(Zoom_Flag)
        {
          DrawLine(x1,y1 + LABELSIZE,x2,y2 + LABELSIZE);
        }
        else
        {
          DrawLine(x1,y1,x2,y2);
        }
      }
  //      printf("x1 %d y1 %d x2 %d y2 %d\n",x1,y1,x2,y2);
  
      newstrtlon = lon;
      newstrtlat = lat;
  
      // Reset the line thickness
      SetLineThick(0);
    }
  }// for maskcnt
}


/*********************************************************************
* This function reads the district boundaries and draw them on the   *
* image.                                                             *
**********************************************************************/

void DoDistrict_Bound()
{
  int i,j,counter;
  int x1,x2,y1,y2;
  double lon,lat ;
  double newstrtlat;
  double newstrtlon;
  char districtfile[100];
  FILE *fpdistrict;
  float district_bound[16906][2];
  char readfile[40000][30];

  memset(districtfile,'\0',100);
  sprintf(districtfile,"%s%smagis.gen",SYSPATH,BACKGROUND_PATH); 
  if((fpdistrict = fopen(districtfile,"r")) == NULL)
  {
    printf("Cannot open inputfile %s\n",districtfile);
    exit(1);
  }

  memset(readfile,'\0',1200000);
  i=0;
  while (fscanf(fpdistrict,"%s",readfile[i]) != EOF)
  {
//    printf("readfile %s\n",readfile[i]);
    i++;
  }
  fclose(fpdistrict);

  counter = 0;
  j = 1; 
  while(j<i)
  {
    if((strchr(readfile[j],'E')) == NULL)
    {
      district_bound[counter][0] = atof(readfile[j]);
      district_bound[counter][1] = atof(readfile[j+1]);
     // printf("district_bound[0] %f district_bound[1] %f\n",
     // district_bound[counter][0],district_bound[counter][1]);
      counter++;
    }
    else
    {
      district_bound[counter][0] = 0.0;
      district_bound[counter][1] = 0.0;
      counter++;
    }
    j += 2; 
//   printf("lon %.3f lat %.3f\n",district_bound[j][0], district_bound[j][1]);
  }
 
 
  SetColor(0,0,255);
  newstrtlon = district_bound[0][0];  
  newstrtlat = -1*district_bound[0][1];  
  for (i=0;i<counter;++i)
  {
    lon = district_bound[i+1][0];
    lat = district_bound[i+1][1];
//    printf("lon %f lat %f\n",lon,lat);
    
    WorldToScreen(newstrtlat,newstrtlon, &x1,&y1);
    WorldToScreen(lat,lon, &x2,&y2);
    if(lon != 0 && newstrtlon !=0)
      if(((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2))<230)
      {
        if(Zoom_Flag)
        {
          DrawLine(x1,y1+LABELSIZE,x2,y2+LABELSIZE);
        }
        else
        {
          DrawLine(x1,y1,x2,y2);
        }
      }
//      printf("x1 %d y1 %d x2 %d y2 %d\n",x1,y1,x2,y2);

    newstrtlon = lon;
    newstrtlat = lat;
  }
}

int ReadMdv()
{

  // Function to search for and read the newest mdv file
 
  int fileav;
  char fname[100];
  time_t asci_time;
  int fieldnr;
  int vertnr;
  FILE *latest_data_fp;
  char datafilestr[100];
  char command[200];
  typedef struct
  {
    int unixtime;
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
  } fileinfo;

  fileinfo latestdata;
  //int i,j;

/*
  // First scan the directory for a new file
  memset(dirname,'\0',100);
  sprintf(dirname,"%s%s",SYSPATH,NOW_DATAPATH);
  //strcpy(dirname,NOW_DATAPATH);
  dirp = opendir(dirname);
  if(dirp == NULL)
  {
    printf("\n Unable to open directory %s \n",dirname);
    exit(1);
  }

  fileav = 0;
  while((dp = readdir(dirp)) != NULL)
  {
    if(strlen(dp->d_name) > 2) // Exclude the . and ..
    {
      printf("File name is:... %s \n",dp->d_name);
      memset(fname,'\0',100);
      sprintf(fname,"%s%s",dirname,dp->d_name);
      //strcpy(fname,dp->d_name);
      fileav = 1;
    }// if
  }// while dp
*/

  // Read the mdv file according to the mode set
  if(RUNTIME_MODE == REALTIME)
  {
    // First read the latest_data_info for the newest file
    // Copy the file to another file to
    // prevent illegal access
    memset(command,'\0',200);
    sprintf(command,"cat %s%slatest_data_info > %s%smdv2gif_temp",
            SYSPATH,NOW_DATAPATH,SYSPATH,NOW_DATAPATH);
    system(command);
    memset(datafilestr,'\0',100);
    sprintf(datafilestr,"%s%smdv2gif_temp",SYSPATH,NOW_DATAPATH);
    if((latest_data_fp = fopen(datafilestr,"r")) == NULL)
    {
      printf("Cannot open latest_data_info file %s\n",datafilestr);
      exit(1);
    }

    
    fscanf(latest_data_fp,"%d %d %d %d %d %d %d",
           &latestdata.unixtime,&latestdata.year,&latestdata.month,
           &latestdata.day,&latestdata.hour,&latestdata.min,&latestdata.sec);

    //Close and remove the temp file
    fclose(latest_data_fp);
    sprintf(command,"rm %s%smdv2gif_temp",SYSPATH,NOW_DATAPATH);
    system(command);

    // Read the file
    memset(fname,'\0',100);

    //  The Gauges only use the 060000.mdv file while the other applications
    //  of this program uses the current available file
    //  Raingauge data lag the radar data and therefore this special case
    // all the "else" cases use the latest data info of hour minute and second
    if(Gauges_Flag) 
    {
     sprintf(fname,"%s%s%.4d%.2d%.2d/060000.mdv",SYSPATH,NOW_DATAPATH,
            latestdata.year,latestdata.month,latestdata.day);
    }
    else
    {
      sprintf(fname,"%s%s%.4d%.2d%.2d/%.2d%.2d%.2d.mdv",SYSPATH,NOW_DATAPATH,
            latestdata.year,latestdata.month,latestdata.day,latestdata.hour,
            latestdata.min,latestdata.sec);
    }

    // Test if a new file was received
    if(strcmp(datainfo.previous_filename,fname) == 0)
    {
      datainfo.newfile = FALSE;
      return 0;
    }
    else
    {
      datainfo.newfile = TRUE;
      strcpy(datainfo.previous_filename,fname);
      printf("New file found, creating image\n");
    }

    printf("File name is:... %s \n",fname);
    fileav = 1;

  }// if REALTIME

  if(RUNTIME_MODE == ARCHIVE)
  {
     if(datainfo.archive_filename == NULL)
    {
    printf("No file selected for ARCHIVE mode\n");
    exit(1);
    }
    strcpy(fname,datainfo.archive_filename);
    fileav = 1;
  }// if ARCHIVE

  //////////////////////////////////
  // if a file is available, read it
  //////////////////////////////////
  if(fileav != 1)
  {
    printf("No file available \n");
    exit(1);
  }

  // Initialize the MDV_handle_t element
  MDV_init_handle(&mdv_f.mdv);

  // Now read the MDV file
  if(MDV_read_all(&mdv_f.mdv,fname,MDV_INT8))
  {
    fprintf(stderr, "ERROR - readmdv \n");
    fprintf(stderr, "Cannot read mdvfile %s \n",fname);
    perror(fname);
    exit(-1);
  }

   // Set the field number to be extracted
  fieldnr = datainfo.Fieldnr;
  if(fieldnr > (mdv_f.mdv.master_hdr.n_fields - 1))
  {
    printf(" Field nr %d doesn't exist\n",fieldnr);
    exit(1);
  }

  // Set the vertical level to be extracted
  vertnr = datainfo.Vertnr;
  if(vertnr > (mdv_f.mdv.fld_hdrs[fieldnr].nz - 1))
  {
    printf(" Vertival plane nr %d doesn't exist\n",vertnr);
    exit(1);
  }

  // Allocate space for the data arrays
  alloc_arrays(&mdv_f);

  // Fill in the data from the mdv file into the data array
  fill_float_array(&mdv_f);

  // Copy to the Global float array
  datainfo.data = mdv_f.f_array[fieldnr][vertnr];

  // Fill in data info
  memset(datainfo.datatime,'\0',100);
  memset(datainfo.datatype,'\0',100);
  memset(datainfo.dataunits,'\0',100);
  // Fill in the headers
  strcpy(datainfo.datatype,
    mdv_f.mdv.fld_hdrs[fieldnr].field_name);
  strcpy(datainfo.dataunits,mdv_f.mdv.fld_hdrs[fieldnr].units);
  asci_time = mdv_f.mdv.master_hdr.time_centroid;
  sprintf(datainfo.datatime,"%s", asctime(gmtime(&asci_time)));

    //printf("hallo %s\n",fname);
    //exit(1);
  return 0;
}

void SetImage(void)
{
  // This function sets the image dimentions
  // according to the mdv header info

  int proj_flag = 0;
  int fieldnr;

  // Set the field number
  fieldnr = datainfo.Fieldnr;

  // Set the data size
  datainfo.nx = mdv_f.mdv.fld_hdrs[fieldnr].nx;
  datainfo.ny = mdv_f.mdv.fld_hdrs[fieldnr].ny;

  // Set the lat-lon range
  datainfo.dx = mdv_f.mdv.fld_hdrs[fieldnr].grid_dx;
  datainfo.dy = mdv_f.mdv.fld_hdrs[fieldnr].grid_dy;

  // Test for the projection type
  datainfo.proj_type = mdv_f.mdv.fld_hdrs[fieldnr].proj_type;

  // Projection LATLON
  if(datainfo.proj_type == MDV_PROJ_LATLON)
  {
    // Projection type is lat-lon
    printf("Projection type is MDV_PROJ_LATLON\n");
    proj_flag = 1;
    datainfo.beginlon = mdv_f.mdv.fld_hdrs[fieldnr].grid_minx;
    datainfo.beginlat = mdv_f.mdv.fld_hdrs[fieldnr].grid_miny;
    datainfo.endlon = datainfo.beginlon + 
        datainfo.nx * datainfo.dx;
    datainfo.endlat = datainfo.beginlat + 
        datainfo.ny * datainfo.dy;
    printf("beginlon = %f endlon = %f nx = %d dx = %f \n",
      datainfo.beginlon,datainfo.endlon,datainfo.nx,datainfo.dx);
    printf("beginlat = %f endlat = %f ny = %d dy = %f \n",
      datainfo.beginlat,datainfo.endlat,datainfo.ny,datainfo.dy);
  }

  // Projection FLAT
  if(datainfo.proj_type == MDV_PROJ_FLAT)
  {
    float r, dely, delx, dellat, dellon;
    // Projection type is lat-lon
    printf("Projection type is MDV_PROJ_FLAT\n");
    proj_flag = 1;
    dely = fabs(mdv_f.mdv.fld_hdrs[fieldnr].grid_miny);
    delx = fabs(mdv_f.mdv.fld_hdrs[fieldnr].grid_miny);
    dellat = (float)(dely*180.0/(EARTHR*PI));
    datainfo.beginlat = mdv_f.mdv.fld_hdrs[fieldnr].proj_origin_lat
       - dellat;
    r = EARTHR*cos(datainfo.beginlat*PI/180.0);
    dellon = delx*180.0/(r*PI);
    datainfo.beginlon = mdv_f.mdv.fld_hdrs[fieldnr].proj_origin_lon
       - dellon;
    datainfo.endlat = mdv_f.mdv.fld_hdrs[fieldnr].proj_origin_lat
       + dellat;
    datainfo.endlon = mdv_f.mdv.fld_hdrs[fieldnr].proj_origin_lon
       + dellon;
    //recalculate the dx and dy in latlon
    //datainfo.dy = (float)(((float)(datainfo.endlon - 
    //   datainfo.beginlon))/(float)(datainfo.ny));
    //datainfo.dx = (float)(((float)(datainfo.endlat - 
    //   datainfo.beginlat))/(float)(datainfo.nx));
    printf("beginlon = %f endlon = %f nx = %d dx = %f \n",
      datainfo.beginlon,datainfo.endlon,datainfo.nx,datainfo.dx);
    printf("beginlat = %f endlat = %f ny = %d dy = %f \n",
      datainfo.beginlat,datainfo.endlat,datainfo.ny,datainfo.dy);
  }

  if(proj_flag == 0)
  {
    // Other projection types are not yet supported
    printf("Projection type %d is not supported\n",
           datainfo.proj_type);
    exit(1);
  }

  // Set the image lat and lon range
  SetLonRange (datainfo.beginlon,datainfo.endlon);
  SetLatRange (datainfo.beginlat,datainfo.endlat);

  //Scale the image
    Scale_Image();


}

void DoRing_Station(char *code,int r,int g,int b)
{
  int i,counter;
  int x1,x2,y1,y2;
  double lon,lat ;
  double newstrtlat;
  double newstrtlon;
  //char ringfile[100];
  FILE *fp_ringfile;
  float ring_bound[5000][2];

  // Create the ringfile
  if((Create_Rangerings(code,RINGRANGE)) == -1)
  {
    printf("Cannot create rings for %s\n",code);
    return;
  }

  //memset(ringfile,'\0',100);
  //sprintf(ringfile,"%s%s%s_ring%d",SYSPATH,BACKGROUND_PATH,code,RINGRANGE); 
  //if((fp_ringfile = fopen(ringfile,"r")) == NULL)
  if((fp_ringfile = fopen(RINGOUTFILE,"r")) == NULL)
  {
    printf("Cannot open inputfile %s\n",RINGOUTFILE);
    exit(1);
  }

  SetColor(r,g,b);
  i=0;

  while (fscanf(fp_ringfile,"%f %f",&ring_bound[i][0], 
        &ring_bound[i][1]) != EOF)
  {
    i++;
  }

  counter = 0;
  newstrtlat = -1*ring_bound[0][0];  
  newstrtlon = ring_bound[0][1];  
  for (counter=0; counter<i ;++counter)
  {
    lat = -1*ring_bound[counter+1][0];
    lon = ring_bound[counter+1][1];
//    printf("lon %f lat %f\n",lon,lat);
    
    WorldToScreen(newstrtlat,newstrtlon, &x1,&y1);
    WorldToScreen(lat,lon, &x2,&y2);

//    if(lon != 0 && newstrtlon !=0)
    //if(x1 > 0 && x1 <= 600 && x2 > 0 && x2 <= 600 && y1 > 0 
    //  && y1 <= 600 && y2 >0 && y2 <= 600)
    if(x1 > 0 && x1 < (GetChartXSize()*datainfo.scalefactor) 
       && x2 > 0 && x2 < (GetChartXSize()*datainfo.scalefactor)
       && y1 > 0 && y1 < (GetChartYSize()*datainfo.scalefactor)
       && y2 > 0 && y2 < (GetChartYSize()*datainfo.scalefactor))
    {
      if(Zoom_Flag)
      {
        DrawLine(x1,y1+LABELSIZE,x2,y2+LABELSIZE);
      }
      else
      {
        DrawLine(x1,y1,x2,y2);
      }
    }

//      printf("x1 %d y1 %d x2 %d y2 %d\n",x1,y1,x2,y2);

    newstrtlon = lon;
    newstrtlat = lat;
  }
  fclose(fp_ringfile);

  return;
}

void DoData()
{
  // This function draws the data on the image

  int i,j;
  int x1,x2,y1,y2;
  float lon,lat ;

  for (i=0;i<datainfo.ny;++i)
  {
    for (j=0;j<datainfo.nx;++j)
    {
      if (datainfo.data[(i*datainfo.nx)+j] <= 1)
        continue;
      SetDataColorVal 
        ((float)datainfo.data[(i*datainfo.nx)+j]);

      // Test for projection type
      if(datainfo.proj_type == MDV_PROJ_FLAT)
      {
        FlatToLat(i,j, &lat,&lon);
        //printf("i=%d j=%d lat=%f lon=%f\n",i,j,lat,lon);
        WorldToScreen(lat,lon, &x1,&y1);
      }
      if(datainfo.proj_type == MDV_PROJ_LATLON)
      {
        WorldToScreen(i*datainfo.dy +datainfo.beginlat,
                      j*datainfo.dx +datainfo.beginlon,&x1,&y1);
      }

      // Test for projection type
      if(datainfo.proj_type == MDV_PROJ_FLAT)
      {
        FlatToLat((i+1), (j+1), &lat,&lon);
        WorldToScreen(lat,lon, &x2,&y2);
      }
      if(datainfo.proj_type == MDV_PROJ_LATLON)
      {
        WorldToScreen(i*datainfo.dy+datainfo.beginlat+datainfo.dy,
                      j*datainfo.dx+datainfo.beginlon+datainfo.dx,&x2,&y2);
      }

      FillRect(x1,y1,x2,y2);

    }
  }
}

void DoLabel()
{
  // This function creates and write the image labels

  char name_label[100];
  char date_label[100];
  char ***catch_label;
  char **basin_label;
  int i,j;
  int labelunits;
  int basinx,basiny;
  int rain_units;
  char compare[100];
  int subcnt, linecnt, strcnt;
  int extrasubcatchlines = 0;
  int extralines = 0;
  int namelen;

  // Test for data type
  rain_units = FALSE;
  memset(compare,'\0',100);
  sprintf(compare,"mm"); 
  if(strcmp(datainfo.dataunits,compare) == 0)
    rain_units = TRUE;

  // Clear the area for the label
  SetColor(255,255,255);
  FillRect(0,GetChartYSize()+1, GetChartXSize()+SCALESIZE,
           GetYSize());

  // Set the labels
  memset(name_label,'\0',100);
  if(rain_units)
  {
    sprintf(name_label,"%s, units: %s",datainfo.datatype, datainfo.dataunits);
  }
  else
  {
    sprintf(name_label,"%s, units: %s Vertical level %d",datainfo.datatype,
       datainfo.dataunits,datainfo.Vertnr);
  }
  printf("\n%s\n",name_label);

  memset(date_label,'\0',100);
  sprintf(date_label,"%s",datainfo.datatime);
  printf("%s\n",date_label);

  if(CatchTot_Flag)
  {
    printf("\n IN CATCHTOT\n");
    // Allocate space for the catchments
    if(rain_units)
      catch_label = (char ***)malloc(N_MASKS*sizeof(char **));
    basin_label = (char **)malloc(N_MASKS*sizeof(char *));
    for(i = 0; i < N_MASKS; i++)
    {
      if(rain_units)
      {
        // Test to make sure no more than 4 names are printed per line
        linecnt = (datainfo.n_subcatch[i]-1)/4 +1;
        catch_label[i] = (char **)malloc(linecnt*sizeof(char *));
        for(subcnt = 0; subcnt < linecnt; subcnt++)
        {
          catch_label[i][subcnt] = (char *)malloc(200*sizeof(char));
        }
      }
      basin_label[i] = (char *)malloc(200*sizeof(char));
    }
    if(rain_units)
    {
      for(i = 0; i < N_MASKS; i++)
      {
        memset(catch_label[i][0],'\0',200);
        sprintf(catch_label[i][0],"%s",MASKNAME[i]);
        strcnt = 0;
        linecnt = 0;
        for(j = 0; j < datainfo.n_subcatch[i]; j++)
        {
          //sprintf(catch_label[i],"%s %s%d %5.1f  ",catch_label[i], 
          //        datainfo.basin_symbols[i], datainfo.basin_numbers[i][j], 
          //        datainfo.basin_av[i][j]);

          if(strcnt < 4)
          {
            sprintf(catch_label[i][linecnt],"%s %s %5.1f  ",
                    catch_label[i][linecnt], datainfo.basin_numbers[i][j], 
                    datainfo.basin_av[i][j]);
            strcnt ++;
          }// if strcnt
          else
          {
            linecnt++;
            strcnt = 0;
            // Fill in spaces at beginning
            memset(catch_label[i][linecnt],'\0',200);
            for(namelen = 0; namelen < (int) strlen(MASKNAME[i]); namelen++)
            {
              sprintf(catch_label[i][linecnt],"%s ",catch_label[i][linecnt]);
            }
            sprintf(catch_label[i][linecnt],"%s %s %5.1f  ",
                    catch_label[i][linecnt], datainfo.basin_numbers[i][j], 
                    datainfo.basin_av[i][j]);
            strcnt++;
            extrasubcatchlines++;
          }// else if strcnt
        }// for j
      }// for i  
    }//if rain_units
  }// if(CatchTot_Flag)

  // Draw the labels
  labelunits = N_MASKS + 2 +extrasubcatchlines;
  SetColor(0,0,0);

  pltsym(3,datainfo.ny*datainfo.scalefactor + 
     LABELSIZE/labelunits -1,name_label,LABELFONTSIZE,0.0,0);

  pltsym(3,datainfo.ny*datainfo.scalefactor + 
     ((LABELSIZE/labelunits)*2) -1,date_label,LABELFONTSIZE,0.0,0);

  if(CatchTot_Flag)
  {
    // Draw labels for all the basins and label the basins as well
    for(i = 0; i < N_MASKS; i++)
    {
      if(rain_units)
      {
        linecnt = (datainfo.n_subcatch[i]-1)/4 +1;
        for(subcnt = 0; subcnt < linecnt; subcnt++)
        {
          pltsym(3,datainfo.ny*datainfo.scalefactor + 
             ((LABELSIZE/labelunits)*(extralines+subcnt+3)) -1,
             catch_label[i][subcnt], LABELFONTSIZE,0.0,0);
        }
        extralines += linecnt;
      }//if rain_units

      //SetColor(255,255,255);
      SetColor(255,0,0);
      for(j = 0; j < datainfo.n_subcatch[i]; j++)
      {
        WorldToScreen(datainfo.basin_latpos[i][j], 
                      datainfo.basin_lonpos[i][j], &basinx, &basiny);
        memset(basin_label[i],'\0',100);
        sprintf(basin_label[i],"%s %.1fmm",
                datainfo.basin_numbers[i][j], datainfo.basin_av[i][j]);
        pltsym(basinx,basiny,basin_label[i],STATIONSIZE,0.0,0);
      }
      SetColor(0,0,0);
    }
  }// if(CatchTot_Flag)

  

  SetColor(0,0,0);
  if(CatchTot_Flag)
  {
    if(rain_units)
    {
      for(i = 0; i < N_MASKS; i++)
      {
        linecnt = (datainfo.n_subcatch[i]-1)/4 +1;
        for(subcnt = 0; subcnt < linecnt; subcnt++)
        {
          free(catch_label[i][subcnt]);
        }// for subcnt
        free(catch_label[i]);
      }// for i
      free(catch_label);
    }// if rain_units
    for(i = 0; i < N_MASKS; i++)
    { 
      free(basin_label[i]);
    }// for i
    free(basin_label);
  }// if CatchTot_Flag

  //if(CatchTot_Flag)
  //{
  //  if(rain_units)
  //    free(catch_label);
  //  free(basin_label);
 // }
}

/*************************************************
 Print the daily average of rainfall per catchment
 to a monthly file

P. Visser 2000-02-21
**************************************************/
void Print_catchment_total_to_file(datastruct *data_info, int count_mask)
{

 FILE *catchment_totals_file;
 
 char datafile[100];
 char month[8],day[8],year[8],hour[8],minute[8],sec[8];
 int imaand,j,k;
 char moonth[13][8] = {"Dum","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug",
		       "Sep","Oct","Nov","Dec"};
 
 
/* Extract month and day from date_line*/
 memset(month,'\0',4);
 memset(year,'\0',5);
 memset(day,'\0',3);
 memset(hour,'\0',3);
 memset(minute,'\0',3);
 memset(sec,'\0',3);
 strncpy(month,&data_info->datatime[4],3);
 strncpy(year,&data_info->datatime[20],4);
 strncpy(day,&data_info->datatime[8],2);
 strncpy(hour,&data_info->datatime[11],2);
 strncpy(minute,&data_info->datatime[14],2);
 strncpy(sec,&data_info->datatime[17],2);

 imaand = 1;
 while ( strncmp(moonth[imaand],month,3) != 0 && imaand < 13)
 {
  imaand++;
 }
 date_info.yr = atoi(year);
 date_info.mnth = imaand;
 date_info.day = atoi(day);
 date_info.hr = atoi(hour);
 date_info.min = atoi(minute);
 date_info.sec = atoi(sec);

 // The filename is set according to rainfall reporting protocol
 // Rainday is reported at 80:00 of that day
 if ( date_info.hr > 6 || ( date_info.hr == 6 && date_info.min > 0))
   Add_day(&date_info);
 
 sprintf(datafile,"%s%scatchtotal/catch%02d%02d%02d",SYSPATH,OUTPUT_PATH,date_info.yr,date_info.mnth,date_info.day);
 printf("\n print to %s\n",datafile);
 if(( catchment_totals_file = fopen(datafile,"a+")) == NULL)
 {
   printf("\n cannot open the outputfile %s \n",datafile);
   exit(1);
 }
 if ( date_info.hr == 6 && date_info.min == 30)
 {
   fprintf(catchment_totals_file,"               ");
   for( j = 0; j < count_mask; j++)
    for( k = 0; k < data_info->n_subcatch[j]; k++)
     fprintf(catchment_totals_file," %s",data_info->basin_numbers[j][k]);
   fprintf(catchment_totals_file,"\n");

   fprintf(catchment_totals_file,"PIX SIZE       ");
   for( j = 0 ; j < count_mask; j++)
    for( k = 0; k < data_info->n_subcatch[j]; k++)
      fprintf(catchment_totals_file,"%5d",data_info->basin_pix[j][k]);
   fprintf(catchment_totals_file,"\n");
   

   fprintf(catchment_totals_file,"%s%02d%02d%02d%02d%02d ",year,imaand,atoi(day),atoi(hour),atoi(minute),atoi(sec));
   for( j = 0 ; j < count_mask; j++)
    for( k = 0; k < data_info->n_subcatch[j]; k++)
      fprintf(catchment_totals_file,"%7.1f",data_info->basin_av[j][k]);
   fprintf(catchment_totals_file,"\n");
 }
 else
 {
   fprintf(catchment_totals_file,"%s%02d%02d%02d%02d%02d ",year,imaand,atoi(day),atoi(hour),atoi(minute),atoi(sec));
   for( j = 0 ; j < count_mask; j++)
    for( k = 0; k < data_info->n_subcatch[j]; k++)
      fprintf(catchment_totals_file,"%7.1f",data_info->basin_av[j][k]);
    fprintf(catchment_totals_file,"\n");
  }
  fclose(catchment_totals_file);
}

/**********************************************************************
 Add one day to current date
**********************************************************************/
extern void Add_day(dateinfo *info)
{
  int maande[13] = {0 , 31, 28 ,31 ,30,31, 30 ,31, 31, 30, 31, 30, 31};

  if( (info->day + 1) > maande[info->mnth])
  {
    if( info->mnth == 2 && info->day == 28)
    {
       if(( info->yr % 4 == 0
        && info->yr % 100 != 0 )  ||  info->yr % 400 == 0 )
          info->day = 29;
    }
    else
    {
      if( info->mnth == 12)
      {
         info->yr = info->yr +1;
         info->mnth = 1;
      }
      else
        info->mnth = info->mnth + 1;
      info->day = 1;
    }
  }
  else
    info->day++;
}

void Scale_Image()
{
  // This function scales the image
  int factx,facty;
  // Initailize the scale
  datainfo.scalefactor = 1;
  
  if(!Zoom_Flag)
  {
    //Calculate the scale
    if((IMAGEMAXNPIX < (datainfo.nx+SCALESIZE)) || 
       (IMAGEMAXNPIX < (datainfo.ny+LABELSIZE)))
    {
      factx = 1;
      facty = 1;
    }
    else
    {
      factx = (int)(IMAGEMAXNPIX/(datainfo.nx+SCALESIZE));
      facty = (int)(IMAGEMAXNPIX/(datainfo.ny+LABELSIZE));
    }
    if(factx < facty)
    {
      datainfo.scalefactor = factx;
      printf("scalefactor = %d\n",datainfo.scalefactor);
    }
    else
    {
      datainfo.scalefactor = factx;
      printf("scalefactor = %d\n",datainfo.scalefactor);
    }
  }// if !Zoom_Flag

  // Set the chart size
  SetChartSize(datainfo.nx*datainfo.scalefactor,
      datainfo.ny*datainfo.scalefactor);

  // Size of the image 
  InitImage(datainfo.nx*datainfo.scalefactor + SCALESIZE,
      datainfo.ny*datainfo.scalefactor + LABELSIZE);

}

void SetDataColorVal (float val)
{
  int r2,g2,b2;
  char compare[100];;

  memset(compare,'\0',100);
  sprintf(compare,"Days");
  // Settings for rainfall
  if(strcmp(datainfo.dataunits,compare) == 0)
  {
    if (val < 1)
    {
      r2=255;g2=255;b2=255;
    }
    else if (val <= 3)    { r2=0;  g2=50; b2=150; }
    else if (val <= 7)    { r2=0;  g2=150; b2=255; }
    else if (val <= 14)   { r2=0;  g2=255; b2=200; }
    else if (val <= 21)   { r2=0;  g2=255; b2=0; }
    else if (val <= 28)   { r2=255;  g2=255; b2=0; }
    else if (val >  28)   { r2=255;  g2=100; b2=255; }
  }

  memset(compare,'\0',100);
  sprintf(compare,"mm"); 
  // Settings for rainfall
  if(strcmp(datainfo.dataunits,compare) == 0)
  {
    if (val < 1)
    {
      r2=0;g2=0;b2=0;
    }
    /*
    else if (1 < val && val <= 5)   { r2=0;  g2=0;   b2=50;}
    else if (5 < val && val <= 10 )   { r2=0;  g2=50;   b2=150;}
    else if (10 < val && val <= 15)   { r2=0;  g2=100;   b2=240;}
    else if (15 < val && val <= 20)   { r2=30;  g2=150;   b2=255;}
    else if (20 < val && val <= 25)   { r2=60;  g2=180; b2=255;}
    else if (25 < val && val <= 30)   { r2=0;  g2=215; b2=255; }
    else if (30 < val && val <= 40)   { r2=0;  g2=255; b2=255; }
    else if (40 < val && val <= 50)   { r2=110;g2=120; b2=255; }
    else if (50 < val && val <= 75)   { r2=185;g2=165; b2=255; }
    else if (75 < val && val <= 100)  { r2=240;g2=100; b2=255; }
    else if (100 < val && val <= 150)  { r2=255;g2=0;   b2=255; }
    else if (150 < val)  { r2=255;  g2=30; b2=200; }
    */

    else if (1 < val && val <= 5)   { r2=0;  g2=0;   b2=50;}
    else if (5 < val && val <= 10 )   { r2=0;  g2=50;   b2=150;}
    else if (10 < val && val <= 15)   { r2=0;  g2=100;   b2=240;}
    else if (15 < val && val <= 20)   { r2=30;  g2=150;   b2=255;}
    else if (20 < val && val <= 25)   { r2=0;  g2=255; b2=255; }
    else if (25 < val && val <= 30)   { r2=110;g2=120; b2=255; }
    else if (30 < val && val <= 40)   { r2=185;g2=165; b2=255; }
    else if (40 < val && val <= 50)  { r2=240;g2=100; b2=255; }
    else if (50 < val && val <= 75)  { r2=255;g2=0;   b2=255; }
    else if (75 < val && val <= 100)  { r2=255;  g2=30; b2=200; }
    else if (100 < val && val <= 150)  { r2=150;g2=150;   b2=150; }
    else if (150 < val)  { r2=205;  g2=205; b2=205; }

  }


  // Settings for reflectivity
  sprintf(compare,"dBZ"); 
  if(strcmp(datainfo.dataunits,compare) == 0)
  {
    if (val < 20)
    {
      r2=0;g2=0;b2=0;
    }
    else if (20 < val && val <= 25)   { r2=0;  g2=0;   b2=50;}
    else if (25 < val && val <= 30 )   { r2=0;  g2=50;   b2=150;}
    else if (30 < val && val <= 35)   { r2=0;  g2=100;   b2=240;}
    else if (35 < val && val <= 40)   { r2=30;  g2=150;   b2=255;}
    else if (40 < val && val <= 45)   { r2=60;  g2=180; b2=255;}
    else if (45 < val && val <= 50)   { r2=0;  g2=215; b2=255; }
    else if (50 < val && val <= 55)   { r2=0;  g2=255; b2=255; }
    else if (55 < val && val <= 60)   { r2=110;g2=120; b2=255; }
    else if (60 < val && val <= 65)   { r2=185;g2=165; b2=255; }
    else if (65 < val && val <= 70)  { r2=240;g2=100; b2=255; }
    else if (70 < val && val <= 75)  { r2=255;g2=0;   b2=255; }
    else if (75 < val)  { r2=255;  g2=30; b2=200; }
    //else if (val == 93)  { r2=55;  g2=60; b2=250; }
    //else if (val == 94)  { r2=255;  g2=30; b2=200; }
  }

  SetColor(r2,g2,b2);

}

/*****************************************************
 * parse_command_line()
 */

void parse_command_line(int argc, char **argv)
{
  int i;
  //int infile_found = FALSE;

  Prog_Name = argv[0];

  // Initialize the flags
  BackGround_Flag = FALSE;
  Data_Flag = FALSE;
  MaskSet_Flag = FALSE;
  Stations_Flag = FALSE;
  Orography_Flag = FALSE;
  CatchmentBound_Flag = FALSE;
  RangeRings_Flag = FALSE;
  CatchTot_Flag = FALSE;
  Help_Flag = FALSE;
  Print_catchment_Flag = FALSE;
  Gauges_Flag = FALSE;

  datainfo.Fieldnr = 0;
  datainfo.Vertnr = 0;

  // Check for valid arguments
  if(argc < 2)
  {
    Help_Flag = TRUE;
    return;
  }
  
  for (i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], "-data") == 0)
    {
      Data_Flag = TRUE;
    }

    if (strcmp(argv[i], "-maskout") == 0)
    {
      MaskSet_Flag = TRUE;
    }

    if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "-help") == 0)
        || (strcmp(argv[i], "-man") == 0))
    {
      Help_Flag = TRUE;
      return;
    }

    if (strcmp(argv[i], "-maskonly") == 0)
    {
      MaskOnly_Flag = TRUE;
    }

    if (strcmp(argv[i], "-catchtot") == 0)
    {
      CatchTot_Flag = TRUE;
    }

    if (strcmp(argv[i], "-print_vaal") == 0)
    {
      Print_catchment_Flag = TRUE;
    }
 
    if (strcmp(argv[i], "-district") == 0)
    {
      DistrictBound_Flag = TRUE;
    }

    if (strcmp(argv[i], "-bg") == 0)
    {
      BackGround_Flag = TRUE;
    }

    if (strcmp(argv[i], "-stations") == 0)
    {
      Stations_Flag = TRUE;
    }

    if (strcmp(argv[i], "-oro") == 0)
    {
      Orography_Flag = TRUE;
    }

    if (strcmp(argv[i], "-catchb") == 0)
    {
      CatchmentBound_Flag = TRUE;
    }

    if (strcmp(argv[i], "-rrings") == 0)
    {
      RangeRings_Flag = TRUE;
    }

    if (strcmp(argv[i], "Fieldnr") == 0)
    {
      datainfo.Fieldnr = atoi(argv[i+1]);
    }

    if (strcmp(argv[i], "Vertnr") == 0)
    {
      datainfo.Vertnr = atoi(argv[i+1]);
    }

    if (strcmp(argv[i], "-file") == 0)
    {
      strcpy(datainfo.archive_filename,argv[i+1]);
      printf("archive file: %s\n",datainfo.archive_filename);
    }

    if (strcmp(argv[i], "-params") == 0)
    {
      PARAMSFILE = argv[i+1];
    }

    if (strcmp(argv[i], "-gauges") == 0)
    {
      Gauges_Flag = TRUE;
      printf("\n THE GAUGE FLAG IS SET TO TRUE");
      sleep(2);
    }
  }

  return;
}

void DrawDataScale(void)
{
  int starty,dy;
  char compare[100];

   memset(compare,'\0',100);
  sprintf(compare,"Days");
  // Settings for rainfall
  if(strcmp(datainfo.dataunits,compare) == 0)
  {

    dy = (int)((((float)(GetChartYSize()))/2.0)/13.0);
    //dy = (int)((((float)(GetChartYSize()))/2.0)/14.0);
    //dy = 14;

    SetLineThick(0);
    SetColor (255,255,255);
    if(Zoom_Flag)
    {
      FillRect (GetChartXSize(),LABELSIZE,GetChartXSize()+SCALESIZE,
        (int)((((float)(GetChartYSize()))/2.0) - 1) + LABELSIZE);
      SetColor (0,0,0);
      DrawRect (GetChartXSize(),LABELSIZE,GetChartXSize()+SCALESIZE,
        (int)((((float)(GetChartYSize()))/2.0) - 1) + LABELSIZE);
      }
    else
    {
      FillRect (GetChartXSize(),0,GetChartXSize()+SCALESIZE,
        (int)((((float)(GetChartYSize()))/2.0) - 1));
      SetColor (0,0,0);
      DrawRect (GetChartXSize(),0,GetChartXSize()+SCALESIZE,
        (int)((((float)(GetChartYSize()))/2.0) - 1));
    }

    if(Zoom_Flag)
    {
      starty=LABELSIZE + 1;
    }
    else
    {
      starty=2;
    }

    SetDataColorVal (0.5);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"0",1.0,0.0,0);

    /* For drought color scales*/
    SetDataColorVal (2);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"1-3",1.0,0.0,0);

    SetDataColorVal (5);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"4-7",1.0,0.0,0);

    SetDataColorVal (9);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"8-14",1.0,0.0,0);

    SetDataColorVal (16);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"15-21",1.0,0.0,0);

    SetDataColorVal (23);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"22-28",1.0,0.0,0);

    SetDataColorVal (30);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"29+",1.0,0.0,0);

     /*SetDataColorVal (3);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"1-5",0.8,0.0,0);

    SetDataColorVal (16);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"5-25",0.8,0.0,0);

    SetDataColorVal (31);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"25-50",0.8,0.0,0);

    SetDataColorVal (75);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"50-100",0.8,0.0,0);

    SetDataColorVal (121);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"100-200",0.8,0.0,0);

    SetDataColorVal (551);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"200+",0.8,0.0,0);*/
  }

  memset(compare,'\0',100);
  sprintf(compare,"mm");
  // Settings for rainfall
  if(strcmp(datainfo.dataunits,compare) == 0)
  {

    dy = (int)((((float)(GetChartYSize()))/2.0)/13.0);
    //dy = (int)((((float)(GetChartYSize()))/2.0)/14.0);
    //dy = 14;

    SetLineThick(0);
    SetColor (255,255,255);
    if(Zoom_Flag)
    {
      FillRect (GetChartXSize(),LABELSIZE,GetChartXSize()+SCALESIZE,
        (int)((((float)(GetChartYSize()))/2.0) - 1) + LABELSIZE);
      SetColor (0,0,0);
      DrawRect (GetChartXSize(),LABELSIZE,GetChartXSize()+SCALESIZE,
        (int)((((float)(GetChartYSize()))/2.0) - 1) + LABELSIZE);
      }
    else
    {
      FillRect (GetChartXSize(),0,GetChartXSize()+SCALESIZE,
        (int)((((float)(GetChartYSize()))/2.0) - 1));
      SetColor (0,0,0);
      DrawRect (GetChartXSize(),0,GetChartXSize()+SCALESIZE,
        (int)((((float)(GetChartYSize()))/2.0) - 1));
    }

    if(Zoom_Flag)
    {
      starty=LABELSIZE + 1;
    }
    else
    {
      starty=2;
    }
/*
    SetDataColorVal (0.5);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"<1",0.8,0.0,0);
*/
  
    SetDataColorVal (2);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"1-5",0.8,0.0,0);
  
    SetDataColorVal (6);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"5-10",0.8,0.0,0);
 
    SetDataColorVal (11);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"10-15",0.8,0.0,0);
  
    SetDataColorVal (16);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"15-20",0.8,0.0,0);
  
    SetDataColorVal (21);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"20-25",0.8,0.0,0);
  
    SetDataColorVal (26);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"25-30",0.8,0.0,0);
  
    SetDataColorVal (31);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"30-40",0.8,0.0,0);
  
    SetDataColorVal (41);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"40-50",0.8,0.0,0);
  
    SetDataColorVal (51);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"50-75",0.8,0.0,0);
  
    SetDataColorVal (76);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"75-100",0.8,0.0,0);
  
    SetDataColorVal (101);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"100-150",0.8,0.0,0);
  
    SetDataColorVal (160);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,">150",0.8,0.0,0);
  }

  memset(compare,'\0',100);
  sprintf(compare,"dBZ");
  // Settings for reflectivity
  if(strcmp(datainfo.dataunits,compare) == 0)
  { 
    dy = (int)((((float)(GetChartYSize()))/2.0)/14.0);
    //dy = 14;

    SetLineThick(0);
    SetColor (255,255,255);
    FillRect (GetChartXSize(),0,GetChartXSize()+SCALESIZE,
      (int)((((float)(GetChartYSize()))/2.0) - 1));
    SetColor (0,0,0);
    DrawRect (GetChartXSize(),0,GetChartXSize()+SCALESIZE,
      (int)((((float)(GetChartYSize()))/2.0) - 1));

    starty=2;
    SetDataColorVal (21);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"20-25",0.8,0.0,0);

    SetDataColorVal (26);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"25-30",0.8,0.0,0);

    SetDataColorVal (31);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"30-35",0.8,0.0,0);

    SetDataColorVal (36);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"35-40",0.8,0.0,0);

    SetDataColorVal (41);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"40-45",0.8,0.0,0);

    SetDataColorVal (46);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"46-50",0.8,0.0,0);

    SetDataColorVal (51);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"50-55",0.8,0.0,0);

    SetDataColorVal (56);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"55-60",0.8,0.0,0);

    SetDataColorVal (61);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"60-65",0.8,0.0,0);

    SetDataColorVal (66);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"65-70",0.8,0.0,0);

    SetDataColorVal (71);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"70-75",0.8,0.0,0);

    SetDataColorVal (76);
    FillRect (GetChartXSize()+45,starty,
       GetChartXSize()+SCALESIZE,starty+dy);starty = starty + dy;
    SetColor (0,0,0);
    pltsym(GetChartXSize()+3,starty,"75-80",0.8,0.0,0);
  }


}

void ReadParams()
{
  // This function reads the params file and fills in the variables

  FILE *fp;
  char name[200];
  char eq[2];
  char val[100];
  char hash[2];
  char hashtest[2];
  char buf[1000];
  int maskcount;
  char *maskcmp;
  char namecmp[200];
  int zoomcount;
  int i;

  printf("in READPARAMS\n");
  // Set up the hash
  memset(hashtest,'\0',2);
  memset(hash,'\0',2);
  sprintf(hash,"#");

  // Open the params file 
  if ((fp = fopen (PARAMSFILE,"r")) == NULL)
  {
    printf("Cannot open params file %s\n",PARAMSFILE);
    return;
  }

  // Memset the name
  memset(name,'\0',200);

  // Read the params file and set the variables
  while(fscanf(fp,"%s",name) != EOF)
  {
    sprintf(hashtest,"%c",name[0]);
    if(strcmp(hashtest,hash) == 0)
    {
      fgets(buf,1000,fp);
    }
    else
    {
      //if(strcmp(name,"SYSPATH,OUTPUT_PATH,SYSPATH") == 0)
      if(strcmp(name,"SYSPATH") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        SYSPATH = (char *)malloc(strlen(val)*sizeof(char));
        memset(SYSPATH,'\0',strlen(val));
        strncpy(SYSPATH,&val[1],(strlen(val)-2));
        printf("SYSPATH %s\n",SYSPATH);
      }

      if(strcmp(name,"NOW_DATAPATH") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        NOW_DATAPATH = (char *)malloc(strlen(val)*sizeof(char));
        memset(NOW_DATAPATH,'\0',strlen(val));
        strncpy(NOW_DATAPATH,&val[1],(strlen(val)-2));
        printf("NOW_DATAPATH %s\n",NOW_DATAPATH);
      }

      if(strcmp(name,"BACKGROUND_PATH") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        BACKGROUND_PATH = (char *)malloc(strlen(val)*sizeof(char));
        memset(BACKGROUND_PATH,'\0',strlen(val));
        strncpy(BACKGROUND_PATH,&val[1],(strlen(val)-2));
        printf("BACKGROUND_PATH %s\n",BACKGROUND_PATH);
      }

      if(strcmp(name,"OUTPUT_PATH") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        OUTPUT_PATH = (char *)malloc(strlen(val)*sizeof(char));
        memset(OUTPUT_PATH,'\0',strlen(val));
        strncpy(OUTPUT_PATH,&val[1],(strlen(val)-2));
        printf("OUTPUT_PATH %s\n",OUTPUT_PATH);
      }

      if(strcmp(name,"ARCHIVE_OUTPUT") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        ARCHIVE_OUTPUT = (char *)malloc(strlen(val)*sizeof(char));
        memset(ARCHIVE_OUTPUT,'\0',strlen(val));
        strncpy(ARCHIVE_OUTPUT,&val[1],(strlen(val)-2));
        printf("ARCHIVE_OUTPUT %s\n",ARCHIVE_OUTPUT);
      }

      if(strcmp(name,"WEB_PATH") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        WEB_PATH = (char *)malloc(strlen(val)*sizeof(char));
        memset(WEB_PATH,'\0',strlen(val));
        strncpy(WEB_PATH,&val[1],(strlen(val)-2));
        printf("WEB_PATH %s\n",WEB_PATH);
      }

      if(strcmp(name,"WEB_FILENAME") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        WEB_FILENAME = (char *)malloc(strlen(val)*sizeof(char));
        memset(WEB_FILENAME,'\0',strlen(val));
        strncpy(WEB_FILENAME,&val[1],(strlen(val)-2));
        printf("WEB_FILENAME %s\n",WEB_FILENAME);
      }

//        if(strcmp(name,"MDV_PROJ_LATLON") == 0)
//        {
//          fscanf(fp,"%s",eq);
//          fscanf(fp,"%s",val);
//          PROJ_LATLON = atoi(val);
//          printf("MDV_PROJ_LATLON %d\n", PROJ_LATLON);
//        }

      if(strcmp(name,"NEWFILE_CHECKTIME") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        NEWFILE_CHECKTIME = atoi(val);
        printf("NEWFILE_CHECKTIME %d\n",NEWFILE_CHECKTIME);
      }

      if(strcmp(name,"ARCHIVE_HOURONLY") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        if(strcmp(val,"TRUE") == 0)
        {
          printf("ARCHIVE_HOURONLY TRUE\n");
          ARCHIVE_HOURONLY = TRUE;
        }
        if(strcmp(val,"FALSE") == 0)
        {
          printf("ARCHIVE_HOURONLY FALSE\n");
          ARCHIVE_HOURONLY = FALSE;
        }
      }

      if(strcmp(name,"RINGRANGE") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        RINGRANGE = atoi(val);
        printf("RINGRANGE %d\n",RINGRANGE);
      }

      if(strcmp(name,"REFLON") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        REFLON = atoi(val);
        printf("REFLON %d\n",REFLON);
      }

      if(strcmp(name,"AZIMUTH_STEP") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        AZIMUTH_STEP = atof(val);
        printf("AZIMUTH_STEP %f\n",AZIMUTH_STEP);
      }

      if(strcmp(name,"RINGOUT_PATH") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        //RINGOUT_PATH = (char *)malloc(strlen(val));
        memset(RINGOUT_PATH,'\0',100);
        strncpy(RINGOUT_PATH,&val[1],(strlen(val)-2));
        printf("RINGOUT_PATH %s\n",RINGOUT_PATH);
      }

      if(strcmp(name,"N_RINGS") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        N_RINGS = atoi(val);
        printf("N_RINGS %d\n",N_RINGS);
        if(N_RINGS != 0)
        {
          RINGNAMES = (char **)malloc(N_RINGS*sizeof(char *));

          for(i = 0; i < N_RINGS; i++)
          {
            fscanf(fp,"%s",name);
            memset(namecmp,'\0',200);
            sprintf(namecmp,"RINGNAMES[%d]",i+1);
            if(strcmp(name,namecmp) == 0)
            {
              RINGNAMES[i] = (char *)malloc(3*sizeof(char));
              fscanf(fp,"%s",eq);
              fscanf(fp,"%s",val);
              memset(RINGNAMES[i],'\0',3);
              strncpy(RINGNAMES[i],&val[1],(strlen(val)-2));
              printf("RINGNAMES[%d] %s\n",i,RINGNAMES[i]);
            }
          }// for i 
        }// if(N_RINGS != 0)
      }// if strcmp N_RINGS

      if(strcmp(name,"STATIONFILE") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        //STATION_FILE = (char *)malloc(strlen(val));
        memset(STATIONFILE,'\0',100);
        strncpy(STATIONFILE,&val[1],(strlen(val)-2));
        printf("STATIONFILE %s\n",STATIONFILE);
      }

//        if(strcmp(name,"MDV_PROJ_FLAT") == 0)
//        {
//          fscanf(fp,"%s",eq);
//          fscanf(fp,"%s",val);
//          MDV_PROJ_FLAT = atoi(val);
//          printf("MDV_PROJ_FLAT %d\n",MDV_PROJ_FLAT);
//        }

      if(strcmp(name,"SCALESIZE") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        SCALESIZE = atoi(val);
        printf("SCALESIZE %d\n",SCALESIZE);
      }

      if(strcmp(name,"LABELFONTSIZE") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        LABELFONTSIZE = atof(val);
        printf("LABELFONTSIZE %f\n",LABELFONTSIZE);
      }

      if(strcmp(name,"LABELSIZE") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        LABELSIZE = atoi(val);
        printf("LABELSIZE %d\n",LABELSIZE);
      }

      if(strcmp(name,"IMAGEMAXNPIX") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        IMAGEMAXNPIX = atoi(val);
        printf("IMAGEMAXNPIX %d\n",IMAGEMAXNPIX);
      }

      if(strcmp(name,"STATIONSIZE") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        STATIONSIZE = atof(val);
        printf("STATIONSIZE %f\n",STATIONSIZE);
      }

      if(strcmp(name,"MODE") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        if(strcmp(val,"REALTIME") == 0)
        {
          printf("RUNTIME_MODE REALTIME\n");
          RUNTIME_MODE = REALTIME;
        }
        if(strcmp(val,"ARCHIVE") == 0)
        {
          printf("RUNTIME_MODE ARCHIVE\n");
          RUNTIME_MODE = ARCHIVE;
        }
      }

       // Set up the Zoom if selected
      if(strcmp(name,"ZOOM") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        if(strcmp(val,"FALSE") == 0)
          Zoom_Flag = FALSE;
        if(strcmp(val,"TRUE") == 0)
        {
          Zoom_Flag = TRUE;

          // Read the zoom coordinates
          for(zoomcount = 0; zoomcount < 4; zoomcount++)
          {
            fscanf(fp,"%s",name);
            if(strcmp(name,"startlat") == 0)
            {
              fscanf(fp,"%s",eq);
              fscanf(fp,"%s",val);
              zoominfo.startlat = atof(val);
            }
            if(strcmp(name,"startlon") == 0)
            {
              fscanf(fp,"%s",eq);
              fscanf(fp,"%s",val);
              zoominfo.startlon = atof(val);
            }
            if(strcmp(name,"stoplat") == 0)
            {
              fscanf(fp,"%s",eq);
              fscanf(fp,"%s",val);
              zoominfo.stoplat = atof(val);
            }
            if(strcmp(name,"stoplon") == 0)
            {
              fscanf(fp,"%s",eq);
              fscanf(fp,"%s",val);
              zoominfo.stoplon = atof(val);
            }
          }//for zoomcount
          printf("zoom: startlat %f startlon %f stoplat %f stoplon %f\n",
                 zoominfo.startlat, zoominfo.startlon,zoominfo.stoplat,
                 zoominfo.stoplon);
        }// if ZOOM = TRUE

       }// if strcmp ZOOM


      // Test and set up the masks
      if(strcmp(name,"N_MASKS") == 0)
      {
        fscanf(fp,"%s",eq);
        fscanf(fp,"%s",val);
        N_MASKS = atoi(val);
        printf("N_MASKS %d\n",N_MASKS);
        
        // Set the mask directory
        if(fscanf(fp,"%s",name) != EOF)
        {
          if(strcmp(name,"MASKDIRECTORY") == 0)
          {
            fscanf(fp,"%s",eq);
            fscanf(fp,"%s",val);
            MASKDIRECTORY = (char *)malloc(strlen(val)*sizeof(char));
            memset(MASKDIRECTORY,'\0',strlen(val));
            strncpy(MASKDIRECTORY,&val[1],(strlen(val)-2));
            printf("MASKDIRECTORY %s\n",MASKDIRECTORY);
          }// if MASKDIRECTORY
          else
          {
            printf("Mask directory not in correct place\n");
            continue;
          }// else MASKDIRECTORY
        }// if !EOF
        
        // Set all the masks
        if(N_MASKS > 0)
        {
          // First allocate the outer dimention of the MASKFILES
          MASKFILE = (char **)malloc(N_MASKS*sizeof(char *));
          MASKNAME = (char **)malloc(N_MASKS*sizeof(char *));
          MASKBOUN = (char **)malloc(N_MASKS*sizeof(char *));
          for(maskcount = 0;  maskcount < N_MASKS; maskcount++)
          {
            // Allocate space for the names
            MASKNAME[maskcount] = (char *)malloc(200*sizeof(char ));
            //MASKBOUN[maskcount] = (char *)malloc(200*sizeof(char ));
            memset(MASKNAME[maskcount],'\0',200);

            if(fscanf(fp,"%s",name) != EOF)
            {
              maskcmp = (char *)malloc(strlen(name)*sizeof(char));
              memset(maskcmp,'\0',strlen(name));
              sprintf(maskcmp,"MASKFILE[%d]",maskcount+1);
              if(strcmp(name,maskcmp) == 0)
              {
                fscanf(fp,"%s",eq);
                fscanf(fp,"%s",val);
                MASKFILE[maskcount] = (char *)malloc(strlen(val)*sizeof(char));
                memset(MASKFILE[maskcount],'\0',strlen(val));
                strncpy(MASKFILE[maskcount],&val[1],(strlen(val)-2));
                printf("MASKFILE[%d] %s\n",maskcount,MASKFILE[maskcount]);
              }// if maskcmp
            }// if fscanf EOF
          /*********************************** 
            if(fscanf(fp,"%s",name) != EOF)
            {
              maskcmp = (char *)malloc(strlen(name)*sizeof(char));
              memset(maskcmp,'\0',strlen(name));
              sprintf(maskcmp,"MASKNAME[%d]",maskcount+1);
              if(strcmp(name,maskcmp) == 0)
              {
                fscanf(fp,"%s",eq);
                fscanf(fp,"%s",val);
                MASKNAME[maskcount] = (char *)malloc(strlen(val)*sizeof(char));
                memset(MASKNAME[maskcount],'\0',strlen(val));
                strncpy(MASKNAME[maskcount],&val[1],(strlen(val)-2));
                printf("MASKNAME[%d] %s\n",maskcount,MASKNAME[maskcount]);
              }// if maskcmp
            }// if fscanf EOF
           *****************************/
            if(fscanf(fp,"%s",name) != EOF)
            {
              maskcmp = (char *)malloc(strlen(name)*sizeof(char));
              memset(maskcmp,'\0',strlen(name));
              sprintf(maskcmp,"MASKBOUN[%d]",maskcount+1);
              
              if(strcmp(name,maskcmp) == 0)
              {
                fscanf(fp,"%s",eq);
                fscanf(fp,"%s",val);
                MASKBOUN[maskcount] = (char *)malloc(strlen(val)*sizeof(char));
                //MASKBOUN[maskcount] = (char *)malloc(20*sizeof(char));
                memset(MASKBOUN[maskcount],'\0',strlen(val));
                strncpy(MASKBOUN[maskcount],&val[1],(strlen(val)-2));
                printf("MASKBOUN[%d] %s\n",maskcount,MASKBOUN[maskcount]);
              }// if maskcmp

            }// if fscanf EOF

                printf("maskcmp %s\n",maskcmp);
                //exit(1);
             free(maskcmp);
          }// for maskcount 
        }//if N_MASKS > 0
      }// if strcmp N_MASKS 

    }// else strcmp hash

    // Memset the name
    memset(name,'\0',200);

  }//  while fscanf

  //free(maskcmp);
  // Close the params file
  fclose(fp);
  printf("finished with params\n");

}

void DoMasks()
{
  //  This function masks out the data according to the masks set
  int maskcount;
  FILE *fp;
  char *maskopenfile;
  int maskfilelen;
  float beginlat,beginlon;
  float endlat,endlon;
  int xdim,ydim;
  float xres,yres;
  int i,j,k;
  char buf[100];
  char ***mask;
  int datax,datay;
  int datastartx,dataendx;
  int datastarty,dataendy;
  float datalat,datalon;
  int maskx, masky;
  int datanx, datany;
  char compare[100];
  int rain_units;
  int subcnt;
  
  // First test to see if any mask flags were selected
  if((!MaskSet_Flag) && (!CatchTot_Flag))
    return;

  // Test for data type
  rain_units = FALSE;
  memset(compare,'\0',100);
  sprintf(compare,"mm"); 
  if(strcmp(datainfo.dataunits,compare) == 0)
    rain_units = TRUE;

  // Allocate space for the number of basins
  datainfo.n_subcatch = (int *)malloc(N_MASKS*sizeof(int));
  //datainfo.basin_symbols = (char **)malloc(N_MASKS*sizeof(char *));
  datainfo.basin_numbers = (char ***)malloc(N_MASKS*sizeof(char **));
  datainfo.basin_totals = (float **)malloc(N_MASKS*sizeof(float *));
  datainfo.basin_pix = (int **)malloc(N_MASKS*sizeof(int *));
  datainfo.basin_av = (float **)malloc(N_MASKS*sizeof(float *));
  datainfo.basin_latpos = (float **)malloc(N_MASKS*sizeof(float *));
  datainfo.basin_lonpos = (float **)malloc(N_MASKS*sizeof(float *));

  printf("\n maskcounts %d\n",N_MASKS);

  for(maskcount = 0; maskcount < N_MASKS; maskcount++)
  {
    printf("\n In FOR LOOP SYSPATH %s MASKDIRECTORY %s MASKFILE[maskcount] %s\n"
            ,SYSPATH,MASKDIRECTORY,MASKFILE[maskcount]);
    // First allocate the memory to the string and set the filename
    maskfilelen = strlen(SYSPATH) + strlen(MASKDIRECTORY) + 
                  strlen(MASKFILE[maskcount]);
    printf("\n %d \n",maskfilelen);
    maskopenfile = (char *)malloc(maskfilelen*sizeof(char)+1);
    memset(maskopenfile,'\0',maskfilelen+1);
    sprintf(maskopenfile,"%s%s%s",SYSPATH,MASKDIRECTORY,MASKFILE[maskcount]);
    printf("\n maskopenfile[%d]: %s\n\n",maskcount,maskopenfile);
    // Open the mask file
    if((fp = fopen(maskopenfile,"r")) == NULL)
    {
      printf("Cannot Open maskfile %s\n",maskopenfile);
      continue;
    }

    // Read the mask file
    // First read the header
    fscanf(fp,"%s",MASKNAME[maskcount]);
    fscanf(fp,"%s %f %s %f %s %d %s %d %s %f %s %f",buf,&beginlat,buf,&beginlon,
           buf,&xdim,buf,&ydim,buf,&xres,buf,&yres);
    endlat = beginlat + ydim*yres; 
    endlon = beginlon + xdim*xres;
    printf("\nbeginlat %f beginlon %f endlat %f endlon %f xdim %d ydim %d xres %f yres %f\n",
	   beginlat,beginlon,endlat,endlon,xdim,ydim,xres,yres);

    // Read the number of basins
    fscanf(fp,"%s %d",buf,&datainfo.n_subcatch[maskcount]);
    // Read the  basin symbols
    // Allocate space for the strings
    //datainfo.basin_symbols[maskcount] = (char *)malloc(100*sizeof(char ));
    //fscanf(fp,"%s %s",buf,datainfo.basin_symbols[maskcount]);
    //Allocate space for the basins
    fscanf(fp,"%s",buf);
    datainfo.basin_numbers[maskcount] = 
        (char **)malloc(datainfo.n_subcatch[maskcount]*sizeof(char *));
    for(subcnt = 0; subcnt < datainfo.n_subcatch[maskcount]; subcnt++)
    {
      datainfo.basin_numbers[maskcount][subcnt] =
        (char *)malloc(20*sizeof(char));
    }
    datainfo.basin_totals[maskcount] = 
        (float *)malloc(datainfo.n_subcatch[maskcount]*sizeof(float));
    datainfo.basin_pix[maskcount] = 
        (int *)malloc(datainfo.n_subcatch[maskcount]*sizeof(int));
    datainfo.basin_av[maskcount] = 
        (float *)malloc(datainfo.n_subcatch[maskcount]*sizeof(float));
    datainfo.basin_latpos[maskcount] = 
        (float *)malloc(datainfo.n_subcatch[maskcount]*sizeof(float));
    datainfo.basin_lonpos[maskcount] = 
        (float *)malloc(datainfo.n_subcatch[maskcount]*sizeof(float));

    for(i = 0; i < datainfo.n_subcatch[maskcount]; i++)
    {
      fscanf(fp,"%s %f %f",datainfo.basin_numbers[maskcount][i],
             &datainfo.basin_latpos[maskcount][i],
             &datainfo.basin_lonpos[maskcount][i]);
    }

    // Read the rest of the mask
    // Allocate space for the mask
    mask = (char ***)malloc(ydim*sizeof(char**));
    for(i = 0; i < ydim; i++)
    {
      mask[i] = (char **)malloc(xdim*sizeof(char *));
      for(j = 0; j < xdim; j++)
      {
        mask[i][j] = (char *)malloc(20*sizeof(char));
      }
    }
    
    // Do the read
    for(i = 0; i < ydim; i++)
    {
      for(j = 0; j < xdim; j++)
      {
        if((fscanf(fp,"%s",mask[ydim - i - 1][j])) == EOF)
        //if((fscanf(fp,"%3d",&mask[ydim - i -1][j])) == EOF)
        //if((fscanf(fp,"%3d",&mask[i][j])) == EOF)
        {
          printf("premature EOF reached in maskfile %s\n",maskopenfile);
          break;
        }
      }
    }

    fclose(fp);

    // Now maskout the data, and calculate the subcatchments
    WorldToScreen(beginlat,beginlon, &datastartx, &datastarty); 
    WorldToScreen(endlat,endlon, &dataendx, &dataendy); 
    printf("datastartx %d datastarty %d dataendx %d dataendy %d\n",
          datastartx,datastarty,dataendx,dataendy);
    //WorldToScreen(endlat,beginlon, &datastartx, &datastarty); 
    //WorldToScreen(beginlat,endlon, &dataendx, &dataendy); 
    datastartx = (int)(((float)(datastartx))/((float)(datainfo.scalefactor)) + 0.5);
    //datastarty = (int)(datastarty/datainfo.scalefactor + 0.5);
    datastarty = (int)(((float)(GetChartYSize() - datastarty -1))/
                 ((float)(datainfo.scalefactor)) + 0.5);
    dataendx = (int)(((float)(dataendx))/((float)(datainfo.scalefactor)) + 0.5);
    //dataendy = (int)(dataendy/datainfo.scalefactor + 0.5);
    dataendy = (int)(((float)(GetChartYSize() - dataendy -1))/
               ((float)(datainfo.scalefactor)) + 0.5);
    datanx = dataendx - datastartx;
    datany = dataendy - datastarty;
    printf("startx %d starty %d endx %d endy %d\n",
          datastartx,datastarty,dataendx,dataendy);
    
    printf("datanx %d datany %d\n",datanx,datany);

    //Create a backup of the original data
    //Allocate space for the original data
    datainfo.orig_data = (float *)malloc(datainfo.ny*datainfo.nx*sizeof(float));
    for(i = 0; i < datainfo.ny; i++)
    {
      for(j = 0; j < datainfo.nx; j++)
      {
        datainfo.orig_data[i*datainfo.nx+j] = datainfo.data[i*datainfo.nx+j];
      }
    }

    for(i = 0; i < datainfo.ny; i++)
    {
      for(j = 0; j < datainfo.nx; j++)
      {
        datainfo.data[i*datainfo.nx+j] = 0;
      }
    }
    // Do just the mask positions if selected
    if(MaskOnly_Flag)
    {
      printf("MaskOnly\n");
      for(i = datastarty; i < dataendy; i++)
      {
        for(j = datastartx; j < dataendx; j++)
        {
          if(j < ((float)(GetChartXSize())/((float)(datainfo.scalefactor))))
          {
            if(i < ((float)(GetChartYSize())/((float)(datainfo.scalefactor))))
            {
              datainfo.data[i*datainfo.nx + j] = 40;
            }// if i
          } // if j
        } // for j
      } // for i

      for(i = 0; i <  datany; i++)
      {
        for(j = 0; j < datanx; j++)
        {
          datay = i+datastarty;
          datax = j+datastartx;
          if(datax >= datainfo.nx)
            break;
          if(datax < 0)
            break;
          if(datay >= datainfo.ny)
          break;
          if(datay < 0)
            break;
          FlatToLat(datax,datay,&datalat,&datalon);
          masky = (int)((((float)(ydim))/((float)(datany)))*i+0.5);
          maskx = (int)((((float)(xdim))/((float)(datanx)))*j+0.5);

          // Prevent spillage at the edges
          if(masky >= ydim)
            continue;
          //  masky = ydim - 1;
          if(masky < 0)
            continue;
          if(maskx >= xdim)
            continue;
           // maskx = xdim - 1;
          if(maskx < 0)
            continue;

/*
          if(mask[masky][maskx] != 0)
          {
            if(mask[masky][maskx] == 11)
            datainfo.data[datay*datainfo.nx+datax]= 24;
            if(mask[masky][maskx] == 12)
            datainfo.data[datay*datainfo.nx+datax]= 30;
            if(mask[masky][maskx] == 13)
            datainfo.data[datay*datainfo.nx+datax]= 35;
            if(mask[masky][maskx] == 81)
            datainfo.data[datay*datainfo.nx+datax]= 50;
            if(mask[masky][maskx] == 82)
            datainfo.data[datay*datainfo.nx+datax]= 70;
            if(mask[masky][maskx] == 83)
            datainfo.data[datay*datainfo.nx+datax]= 76;

          } // if mask != 0
*/  // this only works for the old mask
        }// for j
      }// for i

      continue;
    }// if MaskOnly_flag

    // Initilize the basins
    for(k = 0; k < datainfo.n_subcatch[maskcount]; k++)
    {
      datainfo.basin_totals[maskcount][k] = 0;
      datainfo.basin_pix[maskcount][k] = 0;
    } 
    
    printf("mask the data\n");
    // Mask the data 
    for(i = 0; i <  datany; i++)
    {
      for(j = 0; j < datanx; j++)
      {
        datay = i+datastarty; 
        datax = j+datastartx; 
        if(datax >= datainfo.nx)
          break;
        if(datax < 0)
          break;
        if(datay >= datainfo.ny)
        break;
        if(datay < 0)
          break;
        FlatToLat(datax,datay,&datalat,&datalon);
        //printf("datax %d datay %d\n",datax,datay);
        //printf("datalat %f startlat %f datalon %f startlon %f\n",
        //        datalat,startlat,datalon,startlon);
        //masky = (int)(((datalat - startlat)/yres) + 0.5);
        //maskx = (int)(((datalon - startlon)/xres) + 0.5);
        masky = (int)((((float)(ydim))/((float)(datany)))*i+0.5);
        maskx = (int)((((float)(xdim))/((float)(datanx)))*j+0.5);

        // Prevent spillage at the edges
        if(masky >= ydim)
          continue;
        //  masky = ydim - 1;
        if(masky < 0)
          continue;
        if(maskx >= xdim)
          continue;
         // maskx = xdim - 1;
        if(maskx < 0)
          continue;

       // printf("i %d j %d n_subcatch[%d] %d\n",i,j,
               //  maskcount,datainfo.n_subcatch[maskcount]);
        for(k = 0; k < datainfo.n_subcatch[maskcount]; k++)
        {
          // Do the subcatch if selected
          if(CatchTot_Flag && rain_units)
          {
            //if(mask[masky][maskx] == datainfo.basin_numbers[maskcount][k])
            if(strcmp(mask[masky][maskx], datainfo.basin_numbers[maskcount][k])
                == 0)
            {
              //printf("inside catch %s :data = %f\n", mask[masky][maskx],
               //     datainfo.data[datay*datainfo.nx+datax]);
              datainfo.basin_pix[maskcount][k] += 1;
             /*
              if(datainfo.data[datay*datainfo.nx+datax] > 0.1) 
              {
                printf("Getting rainfall values\n");
                datainfo.basin_totals[maskcount][k] 
                         += datainfo.data[datay*datainfo.nx+datax] ; 
              }
             */
              if(datainfo.orig_data[datay*datainfo.nx+datax] > 0.1)
              {
                datainfo.basin_totals[maskcount][k] 
                         += datainfo.orig_data[datay*datainfo.nx+datax] ;
              }// if(datainfo > 1

            }// if strcmp
          }// if CatchTot
        }//for k

        // Do the maskout 
        memset(compare,'\0',100);
        sprintf(compare,"0");
        if(strcmp(mask[masky][maskx],compare) != 0)
        {
          datainfo.data[datay*datainfo.nx+datax] = 
          datainfo.orig_data[datay*datainfo.nx+datax];
         // printf("mask[%d][%d] %d\n",masky,maskx,mask[masky][maskx]);
        }
      }// for j
    }// for i

    for(k = 0; k < datainfo.n_subcatch[maskcount]; k++)
    {
      // Calculate the basin average if selected
      if(CatchTot_Flag && rain_units)
      {
        datainfo.basin_av[maskcount][k] = 
          ((datainfo.basin_totals[maskcount][k]))/
          ((datainfo.basin_pix[maskcount][k]));
        printf("%s basin_totals %g basin_pix %d basin_av %f\n",
               datainfo.basin_numbers[maskcount][k],
               datainfo.basin_totals[maskcount][k],
               datainfo.basin_pix[maskcount][k],
               datainfo.basin_av[maskcount][k]);
      }
    } // for k

    // reset the data if maskout was not selected
    if(!MaskSet_Flag)
    {
      for(i = 0; i < datainfo.ny; i++)
      {
        for(j = 0; j < datainfo.nx; j++)
        {
          datainfo.data[i*datainfo.nx+j] =
                   datainfo.orig_data[i*datainfo.nx+j];
        }
      }
    }// if(!MaskSet_Flag)

    for(i = 0; i < ydim; i++)
      free(mask[i]);
    free(mask);
  }// for maskcount
  printf("\before print stuff \n");
  if(Print_catchment_Flag)
  {
    printf("\n print catchment flag Ok\n");
    Print_catchment_total_to_file(&datainfo,maskcount);
    printf("\n print catchment flag Ok\n");
  }

  free(maskopenfile);

}

void Free_Data()
{
  // This function free the memory allocated
  // for the data

  int i;

  free(datainfo.data);
  free(datainfo.orig_data);
  free(datainfo.n_subcatch);

    for(i = 0; i < N_MASKS; i++)
  {
    //free(datainfo.basin_symbols[i]);
    //free(datainfo.basin_numbers[i]);
    free(datainfo.basin_totals[i]);
    free(datainfo.basin_pix[i]);
    free(datainfo.basin_latpos[i]);
    free(datainfo.basin_lonpos[i]);
    free(datainfo.basin_av[i]);
  }
  //free(datainfo.basin_symbols);
  //free(datainfo.basin_numbers);
  free(datainfo.basin_totals);
  free(datainfo.basin_pix);
  free(datainfo.basin_latpos);
  free(datainfo.basin_lonpos);
  free(datainfo.basin_av);

}


void Free_Info()
{
  // This function free the memory allocated
  // for mostly the params file

  int i;

  free(SYSPATH);
  free(NOW_DATAPATH);
  free(BACKGROUND_PATH);
  free(OUTPUT_PATH);
  free(ARCHIVE_OUTPUT);
  free(MASKDIRECTORY);
  free(WEB_PATH);
  free(WEB_FILENAME);

   // Free the masks
  for(i = 0; i < N_MASKS; i++)
  {
    free(MASKFILE[i]);
    free(MASKNAME[i]);
  }
  free(MASKFILE);
  free(MASKNAME);

  // Free the rings
  for(i = 0; i < N_RINGS; i++)
  {
    free(RINGNAMES[i]);
  }
  free(RINGNAMES);

  free(zoominfo.Rfield);
  free(zoominfo.Gfield);
  free(zoominfo.Bfield);

}

void Print_Usage()
{

  printf("\n"
	 "Usage: mdv2gif [options as below]\n"
	 "options:\n"
	 "       [ -h, -help, -man ] produce this list.\n"
	 "\n"
	 "       The following are manditory:\n"
	 "       [ -params ?] params file path\n"
	 "       [ Fieldnr ?] Field number to use \n"
	 "       [ Vertnr ?] Vertical level to use \n"
	 "       For ARCHIVE mode you need:\n"
	 "       [ -file ?] mdv file path\n"
	 "\n"
	 "       The following are optional:\n"
	 "       [ -data ] draw data on image\n"
	 "       [ -maskout ] mask the data\n"
	 "       [ -maskonly] draws only the mask positions\n"
	 "       [ -catchtot] computes catchment totals if rainfall file\n"
	 "       [ -district] draws the district boundaries\n"
	 "       [ -bg] draws the background map\n"
	 "       [ -stations] print the stations on the Image\n"
	 "       [ -oro ] draws the orography as background\n"
	 "       [ -catchb ] draws the catchment boundaries\n"
	 "       [ -rrings] draws the range rings for the radar stations\n"
	 "       [ -print_vaal] print catchment totals to a file\n"
	 "       [ -gauges] plot gauges on gif image\n"
	 "\n"
	 "NOTE: For non-rainfall fields the [-catchtot] will have no effect.\n"
	 "\n");

}

void OutPut()
{
  // This function creates the output names and write the files

  char command[200];
  time_t asci_time;
  struct tm *datestruct;
  char datedirstr[100];
  char timestring[100];

  //  First set up the date as a numerical string
  memset(timestring,'\0',100);
  asci_time = mdv_f.mdv.master_hdr.time_centroid;
  datestruct = gmtime(&asci_time);
  // Set up for year 2000 complient
  if(datestruct->tm_year < 1900)
  {
    if(datestruct->tm_year < 70)
    {
      datestruct->tm_year += 2000;
    }
    else
    {
      datestruct->tm_year += 1900;
    }
  }

  // Create the output directory if it doesnt exist
  // Create the date string
  memset(datedirstr,'\0',100);
  sprintf(datedirstr,
          "%.4d%.2d%.2d/",datestruct->tm_year,
           (datestruct->tm_mon+1), datestruct->tm_mday);
  memset(command,'\0',200);
  if(RUNTIME_MODE == REALTIME)
  {
    sprintf(command,
            "mkdir -p %s%s%s",SYSPATH,OUTPUT_PATH,datedirstr);
  }
  if(RUNTIME_MODE == ARCHIVE)
  {
    sprintf(command,
            "mkdir -p %s%s%s",SYSPATH,ARCHIVE_OUTPUT,datedirstr);
  }
  printf("command %s\n",command);
  system(command);

  // Create the filename string
  //sprintf(timestring,"%.4d%.2d%.2d_%.2d%.2d%.2d",datestruct->tm_year,
  //      datestruct->tm_mon, datestruct->tm_mday,datestruct->tm_hour,
  //      datestruct->tm_min, datestruct->tm_sec);
  // Create the time string
  sprintf(timestring,"%.2d%.2d%.2d",datestruct->tm_hour,
          datestruct->tm_min, datestruct->tm_sec);
  printf("date %s%s\n",datedirstr,timestring);

  ///////////////////////////
  // Write the rgb file
  //////////////////////////

  // Extract the zoomed part if selected
  if(Zoom_Flag)
  {
    Zoom_Image();

  } // if Zoom_Flag


  memset(RGBOUTFILE,'\0',100);
  if(RUNTIME_MODE == REALTIME)
  {
    //sprintf(RGBOUTFILE,"%s%s%sRAD_merge_%s",SYSPATH,OUTPUT_PATH,datedirstr,
    //        timestring);  
    sprintf(RGBOUTFILE,"%s%s%s%s",SYSPATH,OUTPUT_PATH,datedirstr,
            timestring);  
  }
  if(RUNTIME_MODE == ARCHIVE)
  {
    //sprintf(RGBOUTFILE,"%s%s%sRAD_merge_%s",SYSPATH,ARCHIVE_OUTPUT,datedirstr,
    //        timestring);
    sprintf(RGBOUTFILE,"%s%s%s%s",SYSPATH,ARCHIVE_OUTPUT,datedirstr,
            timestring);
  }

  printf("Output file:.... %s\n",RGBOUTFILE);
  WriteRGB(RGBOUTFILE);

  // Create the gif output file
  memset(command,'\0',200);
  if(RUNTIME_MODE == REALTIME)
  {
    //sprintf(command,
    // "/usr/bin/rawtoppm -rgb %d %d %s > %s%s%s/RAD_merge_%s.ppm",
    // GetXSize(),GetYSize(),RGBOUTFILE, SYSPATH,OUTPUT_PATH,
    // datedirstr,timestring);
    sprintf(command,
      "/usr/bin/rawtoppm -rgb %d %d %s > %s%s%s/%s.ppm",
      GetXSize(),GetYSize(),RGBOUTFILE, SYSPATH,OUTPUT_PATH,
      datedirstr,timestring);
  }
  if(RUNTIME_MODE == ARCHIVE)
  {
    //sprintf(command,
    // "/usr/bin/rawtoppm -rgb %d %d %s > %s%s%s/RAD_merge_%s.ppm",
    // GetXSize(),GetYSize(),RGBOUTFILE, SYSPATH,ARCHIVE_OUTPUT,
    // datedirstr,timestring);
    sprintf(command,
      "/usr/bin/rawtoppm -rgb %d %d %s > %s%s%s/%s.ppm",
      GetXSize(),GetYSize(),RGBOUTFILE, SYSPATH,ARCHIVE_OUTPUT,
      datedirstr,timestring);
  }

  printf("command %s\n",command);
  system(command);

  // Remove the raw rgb
  memset(command,'\0',200);
  sprintf(command, "rm  %s",RGBOUTFILE);
  printf("command %s\n",command);
  system(command);
  
  // Create a ppm
  memset(command,'\0',200);
  if(RUNTIME_MODE == REALTIME)
  {
    //sprintf(command,
    // "/usr/bin/ppmtogif %s%s%sRAD_merge_%s.ppm > %s%s%sRAD_merge_%s.gif",
    // SYSPATH,OUTPUT_PATH,datedirstr,timestring,SYSPATH,OUTPUT_PATH,
    // datedirstr,timestring);
    sprintf(command,
      "/usr/bin/ppmtogif %s%s%s%s.ppm > %s%s%s%s.gif",
      SYSPATH,OUTPUT_PATH,datedirstr,timestring,SYSPATH,OUTPUT_PATH,
      datedirstr,timestring);

  }
  if(RUNTIME_MODE == ARCHIVE)
  {
    //sprintf(command,
    // "/usr/bin/ppmtogif %s%s%sRAD_merge_%s.ppm > %s%s%sRAD_merge_%s.gif",
    // SYSPATH,ARCHIVE_OUTPUT,datedirstr,timestring,SYSPATH,ARCHIVE_OUTPUT,
    // datedirstr,timestring);
    sprintf(command,
      "/usr/bin/ppmtogif %s%s%s%s.ppm > %s%s%s%s.gif",
      SYSPATH,ARCHIVE_OUTPUT,datedirstr,timestring,SYSPATH,ARCHIVE_OUTPUT,
      datedirstr,timestring);
  }

  printf("command %s\n",command);
  system(command);

  if(RUNTIME_MODE == REALTIME)
  {
   // Copy the file to the web directory if ARCHIVE_HOURONLY is FALSE
   // or move the file if ARCHIVE_HOURONLY is TRUE, execpt for on the hour
    //memset(command,'\0',200);
    //sprintf(command,"cp %s%s%sRAD_merge_%s.gif %s%s%s",SYSPATH,OUTPUT_PATH,
    //        datedirstr,timestring,SYSPATH,WEB_PATH,WEB_FILENAME);
    //printf("command %s\n",command);
    // First test to see that file is not on the hour
    if(ARCHIVE_HOURONLY)
    {
      if((datestruct->tm_min == 0) && (datestruct->tm_sec == 0))
      {
        memset(command,'\0',200);
        sprintf(command,"cp %s%s%s%s.gif %s%s%s",SYSPATH,OUTPUT_PATH,
                datedirstr,timestring,SYSPATH,WEB_PATH,WEB_FILENAME);
      }
      else
      {
        memset(command,'\0',200);
        sprintf(command,"mv %s%s%s%s.gif %s%s%s",SYSPATH,OUTPUT_PATH,
                datedirstr,timestring,SYSPATH,WEB_PATH,WEB_FILENAME);
      }
    }
    else
    {
      memset(command,'\0',200);
      sprintf(command,"cp %s%s%s%s.gif %s%s%s",SYSPATH,OUTPUT_PATH,
              datedirstr,timestring,SYSPATH,WEB_PATH,WEB_FILENAME);
    }
    printf("command %s\n",command);
    system(command);
  }

  // Remove the ppm 
  memset(command,'\0',200);
  if(RUNTIME_MODE == REALTIME)
  {
    //sprintf(command,
    // "rm  %s%s%sRAD_merge_%s.ppm",SYSPATH,OUTPUT_PATH,datedirstr,timestring);
    sprintf(command,
      "rm  %s%s%s%s.ppm",SYSPATH,OUTPUT_PATH,datedirstr,timestring);
  }
  if(RUNTIME_MODE == ARCHIVE)
  {
    //sprintf(command,
    // "rm  %s%s%sRAD_merge_%s.ppm",SYSPATH,ARCHIVE_OUTPUT,
    //  datedirstr,timestring);
    sprintf(command,
      "rm  %s%s%s%s.ppm",SYSPATH,ARCHIVE_OUTPUT,
      datedirstr,timestring);
  }
  printf("command %s\n",command);
  system(command);

  return;

}

void Zoom_Image()
{
  // This function extracts the zoomed part of the image

  char regions_file[100];
  char ringstat[3];

  int xcount,ycount;
  int factx,facty;
  int scaleycnt,scalexcnt;

  //First test if zoom is within domain
  if(zoominfo.startlon < datainfo.beginlon || 
     zoominfo.startlon > datainfo.endlon)
  {
     zoominfo.startlon = datainfo.beginlon;
  }
  if(zoominfo.stoplon > datainfo.endlon || 
     zoominfo.stoplon < datainfo.beginlon)
  {
     zoominfo.stoplon = datainfo.endlon;
  }
  if(zoominfo.startlat < datainfo.beginlat || 
     zoominfo.startlat > datainfo.endlat)
  {
     zoominfo.startlat = datainfo.beginlat;
  }
  if(zoominfo.stoplat > datainfo.endlat || 
     zoominfo.stoplat < datainfo.beginlat)
  {
     zoominfo.stoplat = datainfo.endlat;
  }

  //////////////////////////////////////////////////////////
  // Determine the start and stop positions of the zoomed part
  /////////////////////////////////////////////////////////////
  if(datainfo.proj_type == MDV_PROJ_LATLON)
    {
      zoominfo.startx = (int)((zoominfo.startlon-datainfo.beginlon)/
                             datainfo.dx);
      zoominfo.starty = (int)((zoominfo.startlat-datainfo.beginlat)/
                             datainfo.dy);
      zoominfo.stopx = (int)((zoominfo.stoplon-datainfo.beginlon)/
                             datainfo.dx);
      zoominfo.stopy = (int)((zoominfo.stoplat-datainfo.beginlat)/
                             datainfo.dy);
      zoominfo.nx = zoominfo.stopx - zoominfo.startx;
      zoominfo.ny = zoominfo.stopy - zoominfo.starty;
      zoominfo.starty = datainfo.ny - zoominfo.stopy;
      zoominfo.stopy = datainfo.ny - zoominfo.starty;
    }// if PROJ_LATLON
    if(datainfo.proj_type == MDV_PROJ_FLAT)
    {
      float r;
      int fieldnr;

      // Set the field number
      fieldnr = datainfo.Fieldnr;

      zoominfo.latlondy = (float)(datainfo.dy*180.0/(EARTHR*PI));
      r = EARTHR*cos(mdv_f.mdv.fld_hdrs[fieldnr].proj_origin_lat*PI/180.0);
      zoominfo.latlondx = (float)(datainfo.dx*180.0/(r*PI));

      zoominfo.startx = (int)((zoominfo.startlon-datainfo.beginlon)/
                             zoominfo.latlondx);
      zoominfo.starty = (int)((zoominfo.startlat-datainfo.beginlat)/
                             zoominfo.latlondy);
      zoominfo.stopx = (int)((zoominfo.stoplon-datainfo.beginlon)/
                             zoominfo.latlondx);
      zoominfo.stopy = (int)((zoominfo.stoplat-datainfo.beginlat)/
                             zoominfo.latlondy);
      zoominfo.nx = zoominfo.stopx - zoominfo.startx;
      zoominfo.ny = zoominfo.stopy - zoominfo.starty;
      zoominfo.starty = datainfo.ny - zoominfo.stopy;
      zoominfo.stopy = datainfo.ny - zoominfo.starty;

    } // if PROJ_FLAT
      printf("dy %f dx %f\n",zoominfo.latlondy,zoominfo.latlondx);
    printf("zoom: startx %d starty %d stopx %d stopy %d nx %d ny %d\n",
          zoominfo.startx,zoominfo.starty,zoominfo.stopx,zoominfo.stopy,
          zoominfo.nx,zoominfo.ny);

  /////////////////////
  // Rescale the image
  /////////////////////

  // Initailize the scale
  zoominfo.scalefactor = 1;

  //Calculate the scale
  if(((IMAGEMAXNPIX-SCALESIZE) <= zoominfo.nx) ||
     ((IMAGEMAXNPIX-LABELSIZE) <= zoominfo.ny))
  {
    factx = 1;
    facty = 1;
  }
  else
  {
    factx = (int)((IMAGEMAXNPIX-SCALESIZE)/(zoominfo.nx));
    facty = (int)((IMAGEMAXNPIX-LABELSIZE)/(zoominfo.ny));
  }
  if(factx < facty)
  {
    zoominfo.scalefactor = factx;
    printf("zoom scalefactor = %d\n",zoominfo.scalefactor);
  }
  else
  {
    zoominfo.scalefactor = factx;
    printf("zoom scalefactor = %d\n",zoominfo.scalefactor);
  }


  //////////////////////////////////////////////////
  // Extract the zoomed part from the original image 
  //////////////////////////////////////////////////

  // Allocate space for the new image
  zoominfo.Rfield = (char *)malloc(((zoominfo.ny*zoominfo.scalefactor) +
                    LABELSIZE)* ((zoominfo.nx*zoominfo.scalefactor)+
                    SCALESIZE)*sizeof(char));
  zoominfo.Gfield = (char *)malloc(((zoominfo.ny*zoominfo.scalefactor) +
                    LABELSIZE)* ((zoominfo.nx*zoominfo.scalefactor)+
                    SCALESIZE)*sizeof(char));
  zoominfo.Bfield = (char *)malloc(((zoominfo.ny*zoominfo.scalefactor) +
                    LABELSIZE)* ((zoominfo.nx*zoominfo.scalefactor)+
                    SCALESIZE)*sizeof(char));
  memset(zoominfo.Rfield,255,((zoominfo.ny*zoominfo.scalefactor) +
                    LABELSIZE)* ((zoominfo.nx*zoominfo.scalefactor)+
                    SCALESIZE));
  memset(zoominfo.Gfield,255,((zoominfo.ny*zoominfo.scalefactor) +
                    LABELSIZE)* ((zoominfo.nx*zoominfo.scalefactor)+
                    SCALESIZE));
  memset(zoominfo.Bfield,255,((zoominfo.ny*zoominfo.scalefactor) +
                    LABELSIZE)* ((zoominfo.nx*zoominfo.scalefactor)+
                    SCALESIZE));
 
 
  // Extract the data without the label + scale
  for(ycount = 0; ycount < zoominfo.ny; ycount++)
  {
    for(xcount = 0; xcount < zoominfo.nx; xcount++)
    {
     /*
      zoominfo.Rfield[ycount*(zoominfo.nx*zoominfo.scalefactor + SCALESIZE) + 
                      xcount] =
              Rfield[(zoominfo.starty + ycount + LABELSIZE)*
                (datainfo.nx + SCALESIZE) + zoominfo.startx + xcount];
      zoominfo.Gfield[ycount*(zoominfo.nx*zoominfo.scalefactor + SCALESIZE) + 
                      xcount] =
              Gfield[(zoominfo.starty + ycount + LABELSIZE)*
                (datainfo.nx + SCALESIZE) + zoominfo.startx + xcount];
      zoominfo.Bfield[ycount*(zoominfo.nx*zoominfo.scalefactor + SCALESIZE) + 
                      xcount] =
              Bfield[(zoominfo.starty + ycount + LABELSIZE)*
                (datainfo.nx + SCALESIZE) + zoominfo.startx + xcount];
     */
     
      for(scaleycnt = 0; scaleycnt < zoominfo.scalefactor; scaleycnt++)
      {
        for(scalexcnt = 0; scalexcnt < zoominfo.scalefactor; scalexcnt++)
        {
         
          zoominfo.Rfield[(ycount*zoominfo.scalefactor + scaleycnt + 
                   LABELSIZE )* ((zoominfo.nx*zoominfo.scalefactor)+SCALESIZE) +
                   (xcount*zoominfo.scalefactor + scalexcnt)] = 
                          Rfield[(zoominfo.starty + ycount)* 
                          (datainfo.nx+SCALESIZE) + zoominfo.startx + xcount];
          zoominfo.Gfield[(ycount*zoominfo.scalefactor + scaleycnt + 
                   LABELSIZE )* ((zoominfo.nx*zoominfo.scalefactor)+SCALESIZE) +
                   (xcount*zoominfo.scalefactor + scalexcnt)] = 
                          Gfield[(zoominfo.starty + ycount)* 
                          (datainfo.nx+SCALESIZE) + zoominfo.startx + xcount];
          zoominfo.Bfield[(ycount*zoominfo.scalefactor + scaleycnt + 
                   LABELSIZE )* ((zoominfo.nx*zoominfo.scalefactor)+SCALESIZE) +
                   (xcount*zoominfo.scalefactor + scalexcnt)] = 
                          Bfield[(zoominfo.starty + ycount)* 
                          (datainfo.nx+SCALESIZE) + zoominfo.startx + xcount];
        
           //printf("pos %d\n",(ycount+scaleycnt)*zoominfo.scalefactor*
           //        ((zoominfo.nx* zoominfo.scalefactor)+SCALESIZE) +
           //        ((xcount+scalexcnt)*zoominfo.scalefactor));
        }//for scalexcnt
      }//for scaleycnt
       
    }// for xcount
  }// for ycount
  free(Rfield);
  free(Gfield);
  free(Bfield);


  ///////////////////////////////
  // Reset the image
  //////////////////////////////

  // Set the image lat and lon range
  SetLonRange (zoominfo.startlon,zoominfo.stoplon);
  SetLatRange (zoominfo.startlat,zoominfo.stoplat);

  // Set the chart size
  SetChartSize(zoominfo.nx*zoominfo.scalefactor,
      zoominfo.ny*zoominfo.scalefactor);

  // Size of the image
  InitImage((zoominfo.nx*zoominfo.scalefactor) + SCALESIZE,
      (zoominfo.ny*zoominfo.scalefactor) + LABELSIZE);

  Rfield = zoominfo.Rfield;
  Gfield = zoominfo.Gfield;
  Bfield = zoominfo.Bfield;

  // Draw a rectangle around the data
  SetColor(0,0,0);
  DrawRect(0,LABELSIZE,GetChartXSize()-1,GetChartYSize()+LABELSIZE);
  SetLineType (0);
  DrawRect(0,LABELSIZE,GetChartXSize()-1,GetChartYSize()+LABELSIZE);

  // Draw the topography scale
  if(Orography_Flag)
  {
    DrawTopoScale();
  }

  // Draw the data scale
  if(Data_Flag)
  {
    DrawDataScale(); 
  }

  if(CatchmentBound_Flag)
    DoCatchment_Bound();

  // Draw the range rings for the stations available if selected
  if(RangeRings_Flag)
  {
    int ringcnt;
  /* 
    memset(ringstat,'\0',3);
    sprintf(ringstat,"BM");
    DoRing_Station(ringstat,0,150,150);
    memset(ringstat,'\0',3);
    sprintf(ringstat,"DN");
    DoRing_Station(ringstat,0,150,150);
    memset(ringstat,'\0',3);
    sprintf(ringstat,"EO");
    DoRing_Station(ringstat,0,150,150);
    memset(ringstat,'\0',3);
    sprintf(ringstat,"BL");
    DoRing_Station(ringstat,0,150,150);
    memset(ringstat,'\0',3);
    sprintf(ringstat,"GB");
    DoRing_Station(ringstat,0,150,150);
    memset(ringstat,'\0',3);
    sprintf(ringstat,"IR");
    DoRing_Station(ringstat,0,150,150);
*/    
    for(ringcnt = 0; ringcnt < N_RINGS; ringcnt++)
    {
      memset(ringstat,'\0',3);
      sprintf(ringstat,"%s",RINGNAMES[ringcnt]);
      DoRing_Station(ringstat,0,150,150);
    }
  }

  // Plot the stations if selected
  if(Stations_Flag)
    PlotStations();

  // Read the background map if selected
  if(BackGround_Flag)
  {
    memset(regions_file,'\0',100);
    sprintf(regions_file,"%s%sregions.bkg",SYSPATH,BACKGROUND_PATH);
    ReadBackground(regions_file);
  }

  // Draw District boundaries if selected
  if(DistrictBound_Flag)
    DoDistrict_Bound();

  // Draw the label for the Zoomed Image
  DoZoomLabel();

  return;

}

void DoZoomLabel()
{
  // This function Recreates the label for the zoomed image

  char name_label[100];
  char date_label[100];
  char ***catch_label;
  char **basin_label;
  int i,j;
  int labelunits;
  int basinx,basiny;
  int rain_units;
  char compare[100];
  int subcnt, linecnt, strcnt;
  int extrasubcatchlines = 0;
  int extralines = 0;
  int namelen;

  //WorldToScreen(-26.1,29.9,&x,&y);
  //printf("x %d y %d\n",x,y);
  //exit(1);

  // Test for data type
  rain_units = FALSE;
  memset(compare,'\0',100);
  sprintf(compare,"mm");
  if(strcmp(datainfo.dataunits,compare) == 0)
    rain_units = TRUE;

  // Clear the area for the label and draw the border
  SetColor(255,255,255);
  FillRect(0,0, GetChartXSize()+SCALESIZE,
           LABELSIZE);
  SetColor(0,0,0);
  SetLineType (0);
  DrawRect(0,0,GetChartXSize()-1+SCALESIZE,LABELSIZE);

  // Set the labels
   memset(name_label,'\0',100);
  if(rain_units)
  {
    sprintf(name_label,"%s, units: %s",datainfo.datatype, datainfo.dataunits);
  }
  else
  {
    sprintf(name_label,"%s, units: %s Vertical level %d",datainfo.datatype,
       datainfo.dataunits,datainfo.Vertnr);
  }
  printf("\n%s\n",name_label);

  memset(date_label,'\0',100);
  sprintf(date_label,"%s",datainfo.datatime);
  printf("%s\n",date_label);

  if(CatchTot_Flag)
  {
    // Allocate space for the catchments
    if(rain_units)
      catch_label = (char ***)malloc(N_MASKS*sizeof(char **));
    basin_label = (char **)malloc(N_MASKS*sizeof(char *));
    for(i = 0; i < N_MASKS; i++)
    {
      if(rain_units)
      {
        // Test to make sure no more than 4 names are printed per line
        linecnt = (datainfo.n_subcatch[i]-1)/4 +1;
        catch_label[i] = (char **)malloc(linecnt*sizeof(char *));
        for(subcnt = 0; subcnt < linecnt; subcnt++)
        {
          catch_label[i][subcnt] = (char *)malloc(200*sizeof(char));
        }
      }
      basin_label[i] = (char *)malloc(200*sizeof(char));
    }
    if(rain_units)
    {
      for(i = 0; i < N_MASKS; i++)
      {
        memset(catch_label[i][0],'\0',200);
        sprintf(catch_label[i][0],"%s",MASKNAME[i]);
        strcnt = 0;
        linecnt = 0;
        for(j = 0; j < datainfo.n_subcatch[i]; j++)
        {
          //sprintf(catch_label[i],"%s %s%d %5.1f  ",catch_label[i],
          //        datainfo.basin_symbols[i], datainfo.basin_numbers[i][j],
          //        datainfo.basin_av[i][j]);
          if(strcnt < 4)
          {
            sprintf(catch_label[i][linecnt],"%s %s %5.1f  ",
                    catch_label[i][linecnt], datainfo.basin_numbers[i][j], 
                    datainfo.basin_av[i][j]);
            strcnt ++;
          }// if strcnt
           else
          {
            linecnt++;
            strcnt = 0;
            // Fill in spaces at beginning
            memset(catch_label[i][linecnt],'\0',200);
            for(namelen = 0; namelen < (int) strlen(MASKNAME[i]); namelen++)
            {
              sprintf(catch_label[i][linecnt],"%s ",catch_label[i][linecnt]);
            }
            sprintf(catch_label[i][linecnt],"%s %s %5.1f  ",
                    catch_label[i][linecnt], datainfo.basin_numbers[i][j], 
                    datainfo.basin_av[i][j]);
            strcnt++;
            extrasubcatchlines++;
          }// else if strcnt
        }// for j
      }// for i
    }//if rain_units
  }// if(CatchTot_Flag)

  // Draw the labels
  labelunits = N_MASKS + 2 +extrasubcatchlines;
  SetColor(0,0,0);

  pltsym(3, LABELSIZE/labelunits -1,name_label,LABELFONTSIZE,0.0,0);

  pltsym(3, ((LABELSIZE/labelunits)*2) -1,date_label,LABELFONTSIZE,0.0,0);
   if(CatchTot_Flag)
  {
    // Draw labels for all the basins and label the basins as well
    for(i = 0; i < N_MASKS; i++)
    {
      if(rain_units)
      {
        linecnt = (datainfo.n_subcatch[i]-1)/4 +1;
        for(subcnt = 0; subcnt < linecnt; subcnt++)
        {
          //pltsym(3, ((LABELSIZE/labelunits)*(i+extralines+subcnt+3)) -1,
          //   catch_label[i][subcnt], LABELFONTSIZE,0.0,0);
          pltsym(3, ((LABELSIZE/labelunits)*(extralines+subcnt+3)) -1,
             catch_label[i][subcnt], LABELFONTSIZE,0.0,0);
        }
        extralines += linecnt;
      }//if rain_units

      SetColor(255,0,0);
      for(j = 0; j < datainfo.n_subcatch[i]; j++)
      {
          WorldToScreen(datainfo.basin_latpos[i][j],
                        datainfo.basin_lonpos[i][j], &basinx, &basiny);
        memset(basin_label[i],'\0',100);
        //sprintf(basin_label[i],"%s%d %d",datainfo.basin_symbols[i],
        //        datainfo.basin_numbers[i][j], datainfo.basin_pix[i][j]);
        //sprintf(basin_label[i],"%s%d %.1fmm",datainfo.basin_symbols[i],
        //        datainfo.basin_numbers[i][j], datainfo.basin_av[i][j]);
        sprintf(basin_label[i],"%s %.1fmm",
                datainfo.basin_numbers[i][j], datainfo.basin_av[i][j]);
        pltsym(basinx,basiny+LABELSIZE,basin_label[i],STATIONSIZE,0.0,0);
      }
      SetColor(0,0,0);
    }
  }// if(CatchTot_Flag)

 if(Gauges_Flag)
    PlotGauges(&datainfo);

  SetColor(0,0,0);
  if(CatchTot_Flag)
  {
    if(rain_units)
    {
      for(i = 0; i < N_MASKS; i++)
      {
        linecnt = (datainfo.n_subcatch[i]-1)/4 +1;
        for(subcnt = 0; subcnt < linecnt; subcnt++)
        {
          free(catch_label[i][subcnt]);
        }// for subcnt
        free(catch_label[i]);
      }// for i
      free(catch_label);
    }// if rain_units
    for(i = 0; i < N_MASKS; i++)
    {
      free(basin_label[i]);
    }
    free(basin_label);
  }


}

/*    FUNCTION INVGAUSS                      BB INC       JULY 87         */
/*    Translated from the BASIC program written by E. B. Crawford,        */
/*    Nov 1984, which was obtained from S. Piper of Natal University.     */
/*                                                                        */
/*   Function to transform from Gauss Conform co-ordinates to Lat/LON.   */
/*   Parameters:                                                          */
/*      Xlo  (DBLE)     ===>  Gauss Conform X co-ordinate.                */
/*      Ylo  (DBLE)     ===>  Gauss Conform Y co-ordinate.                */
/*      Meridian  (INT) ===>  Central Meridian of the Gauss Conform       */
/*                            projection.                                 */
/*      Lat  (DBLE)     ===>  Transformed Latitude.                       */
/*      Lon  (DBLE)     ===>  Transformed LONitude.                      */

void gauss(double LAT, double LON, double *X, double *Y, int APPLO)

{

/* converted into a C program by super_seed_soft_where?? */

      double pi,CDR,D1,D2,D3,D4,D5,C,DEL,CON,FIR,ALOR        ;
      double cosa,cosb,VSQ,ETSQ,TOR,AN,F1,F2,F3,B,EIN,ZWEI   ;
      double DREI,XD,F4,F5,F6,VIER,FUNF,SECHS,YD,ALO,AL,AL2  ;
      double AL4,cosc, dLAT;

      pi = 3.141592653589793   ;
      //CDR = PI/180.              ;
      CDR = pi/180.              ;
      D1  =  29413.44944959   ;
      D2  =  -4314.31353082      ;
      D3 = 4.60197112          ;
      D4 = -0.00610875         ;
      D5 = 0.00000880          ;
      C = 1693913.586052       ;
      DEL = 0.006850085445147  ;
      CON = 3.28086933456/12.396 ;

      dLAT  =  -LAT ;
      FIR   =  dLAT*CDR ;
      ALOR  =  LON*CDR ;
      ALO   =  APPLO*CDR ;

      AL = ALO-ALOR ;
      AL2 = AL*AL   ;
      AL4 = AL2*AL2 ;
      cosc = cos(FIR)*cos(FIR) ;
      cosa = cosc*cos(FIR) ;
      cosb = cosc*cosa ;
      VSQ = 1.0+DEL*cosc ;
      ETSQ  =  VSQ-1.0 ;
      TOR  =  sin(FIR)/cos(FIR) ;
      AN = C/( sqrt( fabs(VSQ))) ;
      F1 = AL2*AN/2. ;
      F2 = F1*AL2*(5.- pow(TOR,2.0) + 9.*ETSQ+4. * pow(ETSQ,2.0) )/12. ;
      F3 = F1*AL4*(61.-58.* pow(TOR,2.0)+ pow(TOR,4.0)+270.*ETSQ-330.*ETSQ
         *pow(TOR,2.0)+445.* pow(ETSQ,2.0) )/360. ;
      B = D1*FIR/CDR+D2*sin(2.*FIR)+D3*sin(4.*FIR)+D4*sin(6.*FIR)+D5*
         sin(8.*FIR)   ;
      EIN = F1*sin(FIR)*cos(FIR) ;
      ZWEI = F2*sin(FIR)*cosa ;
      DREI = F3*sin(FIR)*cosb ;
      XD = B+EIN+ZWEI+DREI ;

      F4 = AL*AN ;
      F5 = F1*AL*(1.-pow(TOR,2.0) +ETSQ)/3.0 ;
      F6 = F1*AL*AL2*(5.-18. * pow(TOR,2.0) ) + pow(TOR,4.0)+14.*ETSQ-58.
         *ETSQ*pow(TOR,2.0) +13.* pow(ETSQ,2.0)/60. ;
      VIER = F4*cos(FIR) ;
      FUNF = F5*cosa ;
      SECHS = F6*cosb ;
      YD = VIER+FUNF+SECHS ;


      *X  =  -XD/CON ;  /* change from +ive north to +ive South as per maps */
      *Y  =  YD/CON ;
      return;
}

void invgauss(double Xlo,double Ylo, int meridian,
              double *Lat, double *Lon)
{
   int    i ;
   double a=6378249.145, chi=0.001706680847 ;
   double eccen=0.006803480882, eccen1=0.006850085306, eccen2=0.006850085249 ;
   double pi=3.141592653589793;
   double amult, a0, a1, a2, b1, b2, DeltaB, M, N ;
   double chisqr, gamma, gamsqr, etasqr, ksi ;
   double phi, DeltaPhi, temp ;

   Xlo    = -Xlo ;   /* Change from +ve south to +ve north              */

   chisqr = chi * chi ;
   amult  = a * (1. - chi) * (1. - chisqr) ;
   a0     = amult * (1. + chisqr * (2.25 + chisqr * 225. / 64.)) ;

/*  Now calculate and improve an estimate (phi) of the Lat                   */

   phi      = Xlo / a0 ;
   DeltaPhi = 1.0 ;                                 /* To get into the loop */
   for (i=1 ;  i < 10  &&  fabs(DeltaPhi) > 2.0E-10 ;  i++)
   {
      DeltaB  = phi * (1. + chisqr * (2.25 + chisqr * (225. / 64.))) ;
      DeltaB -= chi * (1.5 + chisqr * (45. / 16. + chisqr * (525. / 128.))) *
                                                           sin(2. * phi) ;
      DeltaB += 0.5 * chisqr * (15. / 8. + chisqr * (105. / 32.)) *
                                                           sin(4. * phi) ;
      DeltaB  = Xlo - (DeltaB * amult) ;
      a1      = chisqr * (1.5 + chisqr * (45. / 16. + chisqr * (525. / 128.))) ;
      a1     *= -2.0 * cos(2.0 * chi * phi) ;
      a2      = chi * chisqr * (15. / 8. + chisqr * (105. / 32.)) ;
      a2     *= cos(2.0 * chi * phi) ;
      DeltaPhi = DeltaB / (a0 + a1 + a2) ;
      phi    += DeltaPhi ;
   }
   gamma   = tan(phi) ;
   gamsqr  = gamma * gamma ;
   temp    = cos(phi) ;
   ksi     = eccen2 * temp * temp ;
   etasqr  = eccen1 * temp * temp ;

   temp    = sin(phi) ;
   temp    = 1. - eccen * temp * temp ;
   M       = a * (1. - eccen) / pow( temp, 1.5) ;
   N       = a / pow( temp, 0.5) ;

   a1      = -gamma / (2. * M * N) ;
   a2      = (5. + 3. * gamsqr + ksi - etasqr * (9. + etasqr * 4.)) ;
   a2      = gamma * a2 / (24. * M * N * N * N) ;
   b1      = 1. / (N * cos(phi)) ;
   b2      = -(1. + 2. * gamsqr + etasqr) / (6. * cos(phi) * N * N * N) ;

   temp    = phi + Ylo * Ylo * (a1 + Ylo * Ylo * a2) ;
   *Lat    = -temp * 180. / pi ;
   temp    = Ylo * (b1 + Ylo * Ylo * b2) ;
   *Lon    = meridian - temp * 180. / pi ;

   return;
}


int Create_Rangerings(char station[3],int range)
{
/*************************************************************************
   NOTE: the central meridian for the gauss projection is an odd
   number of degrees longitude in the South African mapping system.
   For the purposes of the Bethlehem radar, I will use the 28 degree
   longitude as a local mapping system. Also, x is positive in the
   SOUTH direction, and y positive in the EAST direction for the
   Gauss map projection.
**************************************************************************/

/*************************************************************************
 RAD   LAT    LON    MEDIAN

  BL  29.10  26.30   26 BLOEMFONTEIN
  BM  28.25  28.33   28 BETHLEHEM
  CT
  DN  29.97  30.95   30 DURBAN
  DY  26.66  24.00   27 DE-AAR
  EL
  EO  26.50  29.98   27 ERMELO
  IR  25.92  28.22   26 IRENE
  PE  33.98  25.62   34 PORT-ELIZABETH
************************************************************************/

  FILE *fp_stations,
     *fp_outfile;

  //char out_file[100];
  //*char in_file[100];
  //char station[3];
  char filestat[3];
  char stasfile[100];
  int statflag=0;

  int fileinfo;

  double statlat, statlon, filelat, filelon, statXref, statYref;

  float azimuth;
  int azcount;
  double ringx,ringy;
  double xdif,ydif;
  double ringlat,ringlon;
  int aznstep;

  //printf("Creating rings: %s %d\n",station,range);

  // Read the stations file and search for the station selected 
  memset(filestat,'\0',3);
  memset(stasfile,'\0',100);
  sprintf(stasfile,"%s%s%s",SYSPATH,BACKGROUND_PATH,STATIONFILE);
  statflag = 0;
  if((fp_stations=fopen(stasfile,"r")) == NULL)  //outputfile
  {
    printf(" CANNOT FIND %s\n",stasfile);
    return(-1);
  }

  while(fscanf(fp_stations,"%s%lf%lf%d",filestat,&filelat,
        &filelon,&fileinfo) !=  EOF)
  {
    //printf("filestation %s lat %.2f lon %.2f\n",
    //       filestat,filelat,filelon);
    if(strcmp(filestat,station) == 0)
    {
      statflag = 1;
      //statlat = -1*filelat;//
      statlat = filelat;
      statlon  =  filelon;
      printf("station %s found: lat %.3f lon %.3f\n",
             filestat,statlat,statlon);
    }
  }
  fclose(fp_stations);
  if(statflag == 0)
  {
    printf("Station %s not found\n",station);
    return(-1);
  }

  // Open the output file 

  // Clear the char first
  memset(RINGOUTFILE,'\0',200);
  sprintf(RINGOUTFILE,"%s%s%s%s_ring%.1f",SYSPATH,BACKGROUND_PATH,RINGOUT_PATH,
          station, ((float)range/1000.0));
  //printf("ringoutfile is %s\n",RINGOUTFILE);
  if((fp_outfile=fopen(RINGOUTFILE,"w")) == NULL)
  {
    printf(" Cannot open ringoutput file %s\n ",RINGOUTFILE);
    return(-1);
  }

  // First get the station refference X and Y 
  gauss( statlat,statlon, &statYref, &statXref, REFLON);
  //printf("\nstatlat %.3f statlon %.3f Xref %.3f Yref %.3f\n",
  //       statlat,statlon, statXref, statYref);

  // Create the rings and write it to a file

  // Calculate the number of steps for the azimuth 
  azimuth = 0.0;
  aznstep = (int)((2*PI)/(float)((AZIMUTH_STEP*PI)/180.0));
  for(azcount = 0; azcount < aznstep; azcount++)
  {

    // Calculate the x and y points
    xdif = range*sin(azimuth);
    ydif = range*cos(azimuth);

    ringx = statXref + xdif;
    ringy = statYref + ydif;

    // Calculate the lat and lon position 

    invgauss(ringy,ringx,REFLON,&ringlat,&ringlon);


    // Write the lat and lon positions to the outfile 
    fprintf(fp_outfile,"%f %f\n",ringlat,ringlon);

    azimuth += (AZIMUTH_STEP*PI/180.0);
  }

  fclose(fp_outfile);

  return(0);

}
