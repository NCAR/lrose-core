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
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <math.h>
using namespace std;

int main()
{
  const int maxline = 80;
//  char fname[] = "./gage_data/20000808/120000.txt";
  char fname[] = "dat.dat";

  char line[80];
  FILE *fp;
  char comma;
  fp = fopen(fname,"r");
  int id,nc;
  float five_min, one_hour, six_hour, one_day;
  char date[9];
  char time[9];
  char SHEF[6];
  char  *input;
  fgets(line, maxline, fp);
  fgets(line, maxline, fp);

   while(fgets(line,maxline,fp) != NULL)
    {
     if ( (nc = sscanf(line,"%d %c %8s %8s%c %f %c %f %c %f %c %f %c %5s",
                       &id, &comma, date, time, &comma,
                       &five_min, &comma, &one_hour, &comma,
                       &six_hour, &comma, &one_day, &comma, 
                       SHEF)) == 14) 
     {
      printf("%d\n",id) ; 
      printf("%s\n",date); 
      printf("%s\n",time); 
      printf("%f\n",five_min); 
      printf("%f\n",one_hour); 
      printf("%f\n",six_hour); 
      printf("%f\n",one_day); 
      printf("%s\n",SHEF); 
     }
    }  // end while


  fclose(fp);
} // end main
