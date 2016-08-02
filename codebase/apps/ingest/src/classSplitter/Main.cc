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
// This is a small program that splits one big
// class file with many soundings into many class
// files with one sounding each. The word "Release"
// is replaced by the word "Launch" in the output so that
// classIngest can read the resulting data.
//
// This was written somewhat hurriedly.
//
// Niles Oiuen December 2002.
//

#include <cstdio>
#include <string.h>
#include <cstdlib>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
using namespace std;

int main(int argc, char *argv[]){

  char inFileName[1024];
  if (argc < 2){
    fprintf(stdout,"Input file ->");
    fscanf(stdin,"%s",inFileName);
  } else {
    sprintf(inFileName,"%s", argv[1]);
  }

  char outDir[1024];
  if (argc < 3){
    fprintf(stdout,"Output directory ->");
    fscanf(stdin,"%s",outDir);
  } else {
    sprintf(outDir,"%s", argv[2]);
  }

  ta_makedir_recurse(outDir);

  FILE *ifp = fopen(inFileName,"r");
  if (ifp == NULL){
    fprintf(stderr,"Failed to open %s\n",inFileName);
    exit(-1);
  }

  fprintf(stderr,"Opened %s\n",inFileName);

  char Line[2048];
  char SiteName[1024];

  FILE *ofp = NULL;
  int gotTime = 0;
  int gotSite = 0;
  date_time_t T;
  int numLines = 0;

  while (fgets(Line, 2048, ifp) != NULL){

    numLines++;

    //
    // Do we have the end of a sounding here?
    //
    if ( NULL != strstr(Line,"Data Type")){
      if (ofp != NULL){
	fclose(ofp);

	if (!gotTime){
	  fprintf(stderr, "Failed to decode time.\n");
	  exit(-1);
	}
	char outFileName[1024];

	if (!gotSite){
	  sprintf(outFileName, "%s/%d%02d%02d%02d%02d%02d.class",
		  outDir, T.year, T.month, T.day, T.hour,
		  T.min, T.sec);
	} else {
	  //
	  // We have a site name, so make the directory and 
	  // put the file in there.
	  //
	  char dirToMake[2048];
	  sprintf(dirToMake,"%s/%s", outDir,SiteName);

	  ta_makedir_recurse(dirToMake ); 	  

	  sprintf(outFileName, "%s/%s/%d%02d%02d%02d%02d%02d.class",
		  outDir, SiteName, T.year, T.month, T.day, T.hour,
		  T.min, T.sec);
	}

	fprintf(stderr,"Moving to %s\n",outFileName);

	char com[2048];
	sprintf(com,"mv -f tmp.dat %s", outFileName);
	system( com );

      }

      gotTime = 0;
      gotSite = 0;
      fprintf(stderr,"Opening new file.\n");
      ofp = fopen("tmp.dat","w");
      if (ofp == NULL){
	fprintf(stderr, "Failed to create tmp.dat\n");
	exit(-1);
      }
    }
    //
    // Do we have the time here?
    //
    if (!(strncmp(Line, "Nominal Release Time (y,m,d,h,m,s)",
		  strlen("Nominal Release Time (y,m,d,h,m,s)")))){

      sscanf(Line,"Nominal Release Time (y,m,d,h,m,s):%d, %d, %d, %d:%d:%d",
	     &T.year, &T.month, &T.day, &T.hour, &T.min, &T.sec);
      gotTime=1;

    }
    //
    // Do we have the site here?
    //
    if (!(strncmp(Line, "Release Site Type/Site ID:",
		  strlen("Release Site Type/Site ID:")))){
      
      char *siteScan = Line + strlen("Release Site Type/Site ID:");
      //
      // Skip leading spaces.
      //
      int j=0;
      do {
	if (siteScan[j] == ' ') j++;
      } while (siteScan[j] == ' ');

      //
      // Then replace spaces with underscores.
      //
      int k=0;
      do {

	if (
	    (siteScan[j] != char(0)) && 
	    (siteScan[j] != ' ') &&
	    (siteScan[j] != ',')
	    ){
	  SiteName[k]=siteScan[j];
	  SiteName[k+1] = char(0);
	k++; j++;
	} else {
	  break;
	}

      } while ( 1 );

      if (strlen(SiteName) > 0){
	gotSite=1;
      }
      
    }


    //
    // Replace lines that start with "Release" so that they start
    // with "Launch".
    //
    int alreadyPrinted=0;

    if (!strncmp(Line, "Release ", strlen("Release "))){
      alreadyPrinted = 1;
      fprintf(ofp,"Launch %s", Line + strlen("Release "));
    }


    if (!alreadyPrinted){
      fprintf(ofp,"%s", Line);
    }

    if ((numLines % 500) == 0){
      fprintf(stderr,"%d Lines.\n",numLines);
    }

  }


  fclose(ifp);

  fprintf(stderr,"%d Lines.\n",numLines);


  return 0;

}
