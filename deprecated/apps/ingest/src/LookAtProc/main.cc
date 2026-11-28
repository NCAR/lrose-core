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

//
// Program to look at the environment variables that a system
// is running with on a linux system. Niles Oien.
//

#include <cstdio>
#include <cstring>

int main(int argc, char *argv[]){

  if ((argc < 2) || (0==strcmp(argv[1], "-h"))){
    fprintf(stderr,"USAGE : LookAtProc <pid>\n");
    return -1;
  }

  char filename[1024];
  sprintf(filename,"/proc/%s/cmdline", argv[1]);

  FILE *fp = fopen(filename,"r");
  if (fp == NULL){
    fprintf(stderr,"Failed to open %s\n", filename);
    return -1;
  }

  fprintf(stdout,"Command line : ");
  char c;

  while (1) {

    int numRead = fread(&c, sizeof(char), 1, fp);
    if (numRead != 1) break;

    if (c == char(0)){
      fprintf(stdout," ");
    } else {
      fprintf(stdout, "%c", c);
    }
  }

  fclose(fp);
  fprintf(stdout,"\n\n");



  sprintf(filename,"/proc/%s/environ", argv[1]);

  fp = fopen(filename,"r");
  if (fp == NULL){
    fprintf(stderr,"Failed to open %s\n", filename);
    return -1;
  }

  fprintf(stdout,"Environment : \n");

  while (1) {

    int numRead = fread(&c, sizeof(char), 1, fp);
    if (numRead != 1) break;

    if (c == char(0)){
      fprintf(stdout,"\n");
    } else {
      fprintf(stdout, "%c", c);
    }
  }

  fprintf(stdout,"\n");



  return 0;

}
