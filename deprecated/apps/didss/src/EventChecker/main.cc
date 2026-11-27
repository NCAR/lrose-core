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
#include<toolsa/umisc.h>
#include <string.h>
using namespace std;

int main(int argc, char *argv[]){


  FILE *fp = fopen("events.list.inField","rt");

  if (fp == NULL){
    fprintf(stderr,"No.\n");
    exit(-1);
  }


  double DaysCovered = 0;
  int first = 1;

  date_time_t S,E, now;
  time_t LastStart;
  
  ugmtime (&now);
  LastStart = now.unix_time;
  time_t AbsEnd, AbsStart;

  char Line[2048];
  while(fgets(Line,2048,fp) != NULL){


    if (
	12 ==
	sscanf(Line,
	       "start %4d/%2d/%2d %2d:%2d:%2d end %4d/%2d/%2d %2d:%2d:%2d",
	       &S.year,&S.month,&S.day,
	       &S.hour, &S.min, &S.sec,
	       &E.year,&E.month,&E.day,
	       &E.hour, &E.min, &E.sec)

	){
      uconvert_to_utime( &E );
      uconvert_to_utime( &S );

      if (first){
	first = 0;
	AbsEnd = E.unix_time;
      }

      int OverLap;
      if (LastStart <= E.unix_time) {
	OverLap = 1;
      } else {
	OverLap = 0;
      }

      fprintf(stderr,"%s to %s (%g days)\t%d\n",
	      utimstr( S.unix_time),
	      utimstr( E.unix_time),
	      double(E.unix_time-S.unix_time)/86400.0,
	      OverLap);

      DaysCovered = DaysCovered + double(E.unix_time-S.unix_time)/86400.0;

      if (E.unix_time <= S.unix_time){
	fprintf(stderr,"Order : %s",Line);
      }

      if ((E.unix_time - S.unix_time) > 3*86400){
	fprintf(stderr,"Long : %s",Line);
      }


      LastStart = S.unix_time;


    } else {

      if (
	  (strncmp(Line,"#",1)) && 
	   (strlen(Line) > 10)
	   ){
	fprintf(stderr,"Problem : %s",Line);
      }

    }


  }


  fclose(fp);

  AbsStart = S.unix_time;

  double days = (AbsEnd - AbsStart) / 86400.0;


fprintf(stderr,"\n%g of %g days covered.\n",
	DaysCovered,days);


  return 0;

}



