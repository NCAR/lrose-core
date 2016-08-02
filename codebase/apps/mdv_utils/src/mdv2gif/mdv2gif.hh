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
/////////////////////////////////////////
// mdv2gif.hh                          //
//                                     //
// Programmer: J.F.Stuart              //
/////////////////////////////////////////

#define EARTHR 6370

#include <toolsa/toolsa_macros.h>
#include "mdv_rdwr_utils.hh"

char *SYSPATH;
char *NOW_DATAPATH;
char *BACKGROUND_PATH;
char *OUTPUT_PATH;
char *ARCHIVE_OUTPUT;
char *WEB_PATH;
char *WEB_FILENAME;
int NEWFILE_CHECKTIME;
int ARCHIVE_HOURONLY;
int SCALESIZE;
int LABELSIZE;
//  int MDV_PROJ_LATLON;
//  int MDV_PROJ_FLAT;
int REALTIME = 0;
int ARCHIVE = 1;
int RUNTIME_MODE;
int IMAGEMAXNPIX;
float STATIONSIZE;
float LABELFONTSIZE;
char *PARAMSFILE;
int N_MASKS;
char **MASKFILE;
char **MASKNAME;
char **MASKBOUN;
char *MASKDIRECTORY;
int RINGRANGE;
float AZIMUTH_STEP;
int REFLON;
char STATIONFILE[100];
char RINGOUT_PATH[100];
int N_PROV;
char **PROVFILE;
char *PROVDIRECTORY;
char **PROVNAME;
int N_RINGS;
char **RINGNAMES;

char RGBOUTFILE[200];
char RINGOUTFILE[200];
char *Prog_Name;

// Declare all the flags
int BackGround_Flag;
int Stations_Flag;
int Orography_Flag;
int CatchmentBound_Flag;
int DistrictBound_Flag;
int RangeRings_Flag;
int MaskSet_Flag;
int Data_Flag;
int CatchTot_Flag;
int Gauges_Flag;
int MaskOnly_Flag;
int Help_Flag;
int Mode_Flag;
int Province_Flag;
int Zoom_Flag = FALSE;
int Print_district_Flag = FALSE;
int Drought_Flag;
int Lastrain_Flag;
int Print_catchment_Flag = FALSE;

// Create an instance of the mdv_float_handle
mdv_float_handle mdv_f;

// Create a struct and an instance for zooming
typedef struct
{
  float startlat;
  float startlon;
  float stoplat;
  float stoplon;
  int startx,starty,stopx,stopy;
  int nx,ny;
  float latlondx,latlondy;
  char *Rfield,*Gfield,*Bfield;
  int scalefactor;
  
} zoomstruct;
zoomstruct zoominfo;

typedef struct 
{
  char archive_filename[100];
  char previous_filename[100];
  int newfile;
  int Fieldnr;
  int Vertnr;
  char datatime[100];
  char datatype[100];
  char dataunits[100];
  float *data;
  float *orig_data;
  float beginlon,beginlat,endlon,endlat;
  float dx, dy;
  int nx, ny;
  int proj_type;
  int scalefactor;
  int *n_subcatch;
  //char **basin_symbols;
  char ***basin_numbers;
  float **basin_totals;
  int **basin_pix;
  float **basin_latpos;
  float **basin_lonpos;
  float **basin_av;

} datastruct;

datastruct datainfo;

typedef struct
{
  int  yr;
  int  mnth;
  int  day;
  int  hr;
  int  min;
  int  sec;
} dateinfo;

dateinfo date_info;

int Adjust_data_field();
int Create_Rangerings(char station[3],int range);
void invgauss(double Xlo,double Ylo, int meridian,
              double *Lat, double *Lon) ;
void gauss(double LAT, double LON, double *X, double *Y, int APPLO) ;
void DoZoomLabel();
void Zoom_Image();
void OutPut();
void Print_Usage();
void Free_Data();
void Free_Info();
void DoMasks();
void ReadParams();
void DrawDataScale(void);
void parse_command_line(int argc, char **argv);
void SetDataColorVal (float val);
void Scale_Image();
void DoLabel();
void DoData();
void SetImage(void);
int ReadMdv(void);
void DoRing_Station(char *code, int r, int g, int b);
void DoRadar(void);
void DoCatchment_Bound(void);
void DoRing(void);
void DoDate(void);
void DoSubcatch(void);
void DoDistrict_Bound(void);
void DoRing_Bet(void);
void DrawRadScale(void);
void SetRadColorVal (float val);
void PlotStations(void);
void DrawTopoScale(void);
void SetColorVal (double val);
void DoOrography(int seaonly);
void Readstation(void);

void ReadFloat(FILE *fp,float *val);
void ReadDouble(FILE *fp,double *val);

void ReadBackground(char *file);

void PlotGauges(datastruct *datainfo);
void Print_district_total_to_file(datastruct *datainfo,int count_mask);
extern void Add_day(dateinfo *date_info);

extern double Bsslint(float *p,float *ai,float *aj,int *m,int *n);


