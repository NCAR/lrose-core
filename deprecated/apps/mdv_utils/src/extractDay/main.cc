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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <cstdio>
#include <cstring>
//
// Program to extract one day's worth of data from $RAP_DATA_DIR structure.
//
int main(int argc, char *argv[]){

  if (argc < 3){
    fprintf(stderr, "USAGE : %s <target> <source>\n",
	    argv[0]);
    fprintf(stderr,"EXAMPLE : cd $RAP_DATA_DIR\n");
    fprintf(stderr,"find rttc-anc -name 20100426\\* -exec %s ./tmp {} \\;\n", argv[0]);
    fprintf(stderr,"Niles Oien April 2010\n");
    return -1;
  }

  struct stat buf;

  if (stat(argv[2], &buf)){
    fprintf(stderr, "%s - could not stat %s\n", argv[0], argv[2]);
    return -1; // Unlikely as will run this with find
  }

  //
  // If it's a directory then we just have one copy command
  // after making the target directory
  //
  if (S_ISDIR(buf.st_mode)){
    char com[1024];
    sprintf(com,"/bin/mkdir -p %s/%s", argv[1], argv[2]);
    system(com);

    sprintf(com,"/bin/cp -rvf %s/* %s/%s/", argv[2], argv[1], argv[2]);
    system(com);

  }

  //
  // If it's a file then we need to make the target directory
  // and copt the file to it.
  //
  if (S_ISREG(buf.st_mode)){

    char dir[1024];
    sprintf(dir,"%s", argv[2]);

    //
    // Need to get directory name from file name
    //
    for (int i=strlen(dir); i > -1; i--){
      if (dir[i] == '/'){
	dir[i] = char(0);
	break;
      }
    }


    char com[1024];

    sprintf(com,"/bin/mkdir -p %s/%s", argv[1], dir);
    system(com);

    sprintf(com,"/bin/cp -rvf %s %s/%s", argv[2], argv[1], dir);
    system(com);

  }



  return 0;

}

