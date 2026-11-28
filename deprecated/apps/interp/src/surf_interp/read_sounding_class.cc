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
#include <toolsa/umisc.h>
#include <string.h>
#include <math.h>
using namespace std;

void read_sounding_class(char *sounding_dir,
                         float pres_li, float *tli,
                         float tli_default, float badsf,
			 float *Lat, float *Lon, date_time_t *SoundingTime,
                         date_time_t DataTime, int debug)
{

  DIR *dirp;
  struct dirent *dp;
  date_time_t st; // Sounding time.
  double MinTimeDiff,TimeDiff;
  FILE *fp;
  int i,Num,go;
  float p[1000],t[1000],tm;
  char line[1024];
  char Filename[MAX_PATH_LEN];
  int first;

  MinTimeDiff=0; // Initialise or I get a compiler warning.

  *tli=badsf; // Start on a pessimistic note.
  float lat,lon;
  *Lon=-9000; *Lat=-9000;
  first=1;

  if ((dirp = opendir(sounding_dir)) == NULL) {
    fprintf(stderr,"Failed to open sounding directory %s\n",
	    sounding_dir);
    return;
  }

  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) { 
    //
    // See if the filename matches the YYYYMMDDhhmmss.class spec.
    //
    if ((strlen(dp->d_name)==20) && 
	(6==sscanf(dp->d_name,
		   "%4d%2d%2d%2d%2d%2d.class",
		   &st.year,&st.month,&st.day,
		   &st.hour,&st.min,&st.sec))){
      //
      if (debug) fprintf(stderr,"Considering %s\n",dp->d_name);
      //
      // See if this is the latest time.
      //
      st.unix_time=uconvert_to_utime(&st);

      TimeDiff=fabs(st.unix_time-DataTime.unix_time); 
      // Time error between data and sounding

      if (first){
	MinTimeDiff=TimeDiff;
      }

      if ((TimeDiff < MinTimeDiff) || (first)){ // This is the closest time yet.
	//
	if (debug) fprintf(stderr,"Closest time to data yet.\n");
	//
	// Loop through the file until pres_li is exceeded.
	//
	if (sounding_dir[strlen(sounding_dir)-1]=='/'){
	  sprintf(Filename,"%s%s",sounding_dir,dp->d_name);
	} else {
	  sprintf(Filename,"%s/%s",sounding_dir,dp->d_name);
	}
	fp = fopen(Filename,"ra");
	if (fp==NULL){ // Unlikely.
	  fprintf(stderr,"Failed to open %s\n",Filename);
	  exit(-1);
	}
	i=0;
	while((NULL!=fgets(line,1023,fp)) && (i < 1000)){
	  if (3==(sscanf(line,"%f %f %f",&tm,&p[i],&t[i]))){
	    i++;
	  }

	  if (2==(sscanf(line,
			 "Launch Location (lon,lat,alt): %*d %*f%*c, %*d %*f%*c, %f, %f",
			 &lon,&lat))){
	    if (debug) fprintf(stderr,"Location : %g,%g\n",*Lat,*Lon);
	  }

	}
	fclose(fp);
	Num=i;
	//
	// OK - filled arrays - see if we can get something.
	//
	i=0; go=1;
	do{
	  if ((p[i] >= pres_li) && (p[i+1]<= pres_li)){                       
	    go=0;
	    *tli=t[i]+(t[i+1]-t[i])*(p[i]-pres_li)/(p[i]-p[i+1]);
	    MinTimeDiff=TimeDiff; // Record time difference for this valid reading.
	    first=0; // No longer the first valid sounding we're looking for.
	    *SoundingTime=st; *Lon=lon; *Lat=lat;
	  }
	  i++;
	}while((i<Num-2) && (go));
	if (debug){
	  if (!(go)) fprintf(stderr,"Got %g from %s\n",*tli,Filename);
	  if (go) fprintf(stderr,"%s did not exceed pressure %g\n",
			  Filename,pres_li);
	}
      } // if latest time
    }
  }

  // End of directory reading loop.

  closedir(dirp);
  if (debug) fprintf(stderr,"Read soundings : returning %g\n",*tli);
  return;

}




