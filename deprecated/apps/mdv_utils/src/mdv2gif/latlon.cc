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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <toolsa/toolsa_macros.h>

#define LATLON_MAIN
#include "latlon.hh"

double GetStartLon()
{
  return (startlon);
}
double GetStopLon()
{
  return (stoplon);
}
double GetStartLat()
{
  return (startlat);
}
double GetStopLat()
{
  return (stoplat);
}
int GetChartXSize()
{
  return (CHARTX);
}
int GetChartYSize()
{
  return (CHARTY);
}
void SetLonRange(double start,double stop)
{
  startlon = start;
  stoplon = stop;
}
void SetLatRange(double start,double stop)
{
  startlat = start;
  stoplat = stop;
}

void SetChartSize(int xs,int ys)
{
  CHARTX = xs;
  CHARTY = ys;
}

int WorldToScreen(double lat, double lon, int *x, int *y)
{
  double xscale,yscale;

  xscale = (stoplon - startlon)/(double)CHARTX;
  yscale = (stoplat - startlat)/(double)CHARTY;

  *x = (int)((lon + (0.0-startlon)) / xscale);
  *y = CHARTY - (int)((lat + (0.0-startlat)) / yscale);
  if ( *x < 0 || *x > CHARTX)
    return (0);
  if ( *y < 0 || *y > CHARTY)
    return (0);
  return (1);
}      

int FlatToLat(int y, int x, float *lat, float *lon)
{
  float dellat, dellon;
  float r;

  dellat = 180.0/(EARTHR*PI);
  *lat = startlat + dellat*y;

  r = EARTHR*cos(*lat*PI/180.0);
  dellon = 180.0/(r*PI);
  *lon = startlon + dellon*x;
//  printf("startlat=%f startlon=%f y=%d dellat=%f lat=%f x=%d dellon=%f lon=%f\n",startlat,startlon,y,dellat,*lat,x,dellon,*lon);
  
  return(1);
}
