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

#include <toolsa/umisc.h>
#include <Spdb/SoundingGet.hh>
using namespace std;

int plug_missing(double *d, int Num, double Missing);

void ReadSounding(char *sounding_url,
		  long sounding_look_back,
		  double sounding_alt_min,
		  double sounding_alt_max,
		  float pres_li, float *tli,
		  float tli_default, float badsf,
		  float *Lat, float *Lon, 
		  date_time_t *SoundingTime,
		  date_time_t DataTime,
		  int debug){


  *tli = badsf;

  //
  // Instantiate the sounding class.
  //

  SoundingGet sounding;
  sounding.init( sounding_url,
		  sounding_look_back*60,
		  sounding_alt_min*1000.0,
		  sounding_alt_max*1000.0,
		  0.0, 0.0 );

  //
  // Set the time on it.
  //
  if ((sounding.readSounding( DataTime.unix_time ) < 0 ) )
    {
    if (debug) fprintf(stderr, 
		       "Unable to load sounding data from %s\n",
		       sounding_url);
    return;
    } 

  int Num =   sounding.getNumPoints();
  double *p = sounding.getPres();
  double *t = sounding.getTemp();

  if (p==NULL) fprintf(stderr,"NULL pressure!!\n");
  if (t==NULL) fprintf(stderr,"NULL temperature!!\n");

  if ((p==NULL) || (t==NULL)) {
    return;
  }

  double m =  sounding.getMissingValue();

  SoundingTime->unix_time = sounding.getLaunchTime();
  uconvert_from_utime( SoundingTime );

  *Lon = (float) sounding.getLon();
  *Lat = (float) sounding.getLat();


  if ( (plug_missing(p, Num, m)) || 
       (plug_missing(t, Num, m)) ){
    return;
  }

  int i=-1;
  int go=1;
  do{
    i++;

    if (((p[i] >= pres_li) && (p[i+1] <= pres_li)) ||
        ((p[i] <= pres_li) && (p[i+1] >= pres_li))) go=0;


  } while ((i < Num -1) && (go));
  if (i==Num -1) return; // Sounding never reached required prressure


  *tli = t[i]+(t[i+1]-t[i])*(p[i]-pres_li)/(p[i]-p[i+1]);


}

/////////////////////////////////
//
// This interpolates over any missing data.
//
int plug_missing(double *d, int Num, double Missing){

  //
  // Fill in any values missing at the start.
  //


  int i=-1;
  do{
    i++;
  } while ((i < Num) && (d[i]==Missing));
  if (i==Num) return -1; // No valid data.

  for (int j=i-1; j > -1; j--){
    d[j]=d[i];
  }

  //
  // Ditto at the end.
  //

  i=Num;
  do{
    i--;
  } while ((i > -1) && (d[i]==Missing));
  if (i== -1) return -1; 

  for (int j=i+1; j < Num; j++){
    d[j]=d[i];
  }

  //
  // Interpolate over the middle.
  //

  for (int k=1; k<Num-1; k++){
    if (d[k]==Missing){
      int f=k;
      do{
	if (d[f]==Missing) f++;
      } while(d[f]==Missing);
      int s=k-1;

      for (int j=s+1; j<f; j++){
	double t = double(j-s) / double(f-s);
	d[j]= t*d[f] + (1.0-t)*d[s];
      }

    }
  }

  return 0; // Success.
  
}



