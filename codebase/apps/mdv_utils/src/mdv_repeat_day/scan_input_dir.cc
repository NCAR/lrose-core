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
using namespace std;

int scan_input_dir(char *InDir, date_time_t *Time,
		    int MaxEntries, int *NTimes){

  DIR *dirp;
  struct dirent *dp;

  if ((dirp = opendir(InDir)) == NULL) {
    fprintf(stderr,
            "Cannot open directory '%s'\n",InDir);
    exit(-1);
  }

  int Entry=0,hour,min,sec;
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {

    if ((strlen(dp->d_name)==10) &&
        (3==sscanf(dp->d_name,"%2d%2d%2d.mdv",
		   &hour, &min,&sec))){
      Time[Entry].year=1970;
      Time[Entry].month=1;
      Time[Entry].day=1;
      Time[Entry].hour=hour;
      Time[Entry].min=min;
      Time[Entry].sec=sec;

      Time[Entry].unix_time=uconvert_to_utime(Time + Entry);
      Entry++;
      if (Entry==MaxEntries){
	*NTimes=Entry;
	return 1;
      }

    }
  }

  closedir(dirp);

  *NTimes=Entry;

  //
  // Sort the times into ascending order.
  //

  int ic;
  date_time_t Q;
  do {
    ic=0;
    for (int k=0; k < *NTimes - 1; k++){
      if (Time[k].unix_time > Time[k+1].unix_time){
        Q=Time[k]; Time[k] = Time[k+1]; Time[k+1]=Q; ic=1;
      }
    }
  } while(ic);


  return 0;

}
