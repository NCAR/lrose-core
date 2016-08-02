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
#ifndef LATLON_HH
#define LATLON_HH

#ifdef LATLON_MAIN
  double startlon;
  double stoplon;
  double startlat;
  double stoplat;
  int CHARTX,CHARTY;
#else
  extern double startlon;
  extern double stoplon;
  extern double startlat;
  extern double stoplat;
  extern int CHARTX;
  extern int CHARTY;
#endif

#define EARTHR 6370

int WorldToScreen(double lat, double lon, int *x, int *y);
int FlatToLat(int x, int y, float *lat, float *lon);
void rect_to_polar (double xdiff, double ydiff, double *angle, double *radius);
void SetLonRange(double start,double stop);
void SetLatRange(double start,double stop);
void SetChartSize(int xs,int ys);
int GetChartXSize(void);
int GetChartYSize(void);
double GetStartLon(void);
double GetStopLon(void);
double GetStartLat(void);
double GetStopLat(void);

#endif

