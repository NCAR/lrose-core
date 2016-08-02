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
// Small program to generate stats on lidar files.
//
#include <cstdio>
#include <toolsa/umisc.h>
#include <stdlib.h>
#include <ctime>

#include "Params.hh"

int main(int argc, char *argv[]){


  //
  // Define the header type.
  //
  typedef struct {
    unsigned int         unixTime;       ///< The beam time stamp, in seconds since jan 1, 1970.
    int                  gates;          ///< The number of gates in the beam.
    float                gateSpacing;    ///< The spacing between gates, in meters.
    float                azimuth;        ///< The azimuth pointing angle of the beam,  in compass 
                                         // coordinates. 0 = N, 90 = E, 180 = S, 270 = W
    float                elevation;      ///< The elevation pointing angle of the beam, in cartessian 
                                         // coordinates. 0 = horizontal, 90 = up, 180 = horizontal, 270 = down.
    float                minAngle;       ///< The minimum angle of the sweep.
    float                maxAngle;       ///< The maximum angle of the sweep.
    float                angleInc;       ///< The anticipated angle increment between beams.
    int                  mode;           ///< The instrument scanning mode. PPI == 0, RHI == 1, FIXED == 2
    int                  transect;       ///< A so-called transect number, analogous to a sweep number. 
                                         // It is used to indicate the change of a sweep.
    int                  headerVersion;  ///< The version of this header.
  } header_t;


  // Get the TDRP parameters.
  
  Params *P = new  Params(); 
  
  if (P->loadFromArgs(argc, argv, NULL, NULL)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  } 
  
  header_t h;

  time_t now = time(NULL);
 
  // Input filename needs to follow -f

  int fileNameIndex = -1;
  for (int i=1; i < argc; i++){
    if (0==strcmp("-f", argv[i])){
      fileNameIndex = i+1;
      break;
    }
  }

  if ((fileNameIndex > argc-1) || (fileNameIndex < 0)){
    fprintf(stderr, "ERROR : input filename must follow -f argument\n");
    return -1;
  }


  //
  // Open the input file, read the first header, use the time to name output file.
  //
  FILE *fp = fopen(argv[fileNameIndex], "r");
  if (fp == NULL) return -1;

  if (1 != fread(&h, sizeof(h), 1, fp)) return -1;

  //
  // Convert the time fields
  //
  date_time_t firstBeamTime;
  firstBeamTime.unix_time = h.unixTime;
  uconvert_from_utime( &firstBeamTime );

  //
  // extract the directory name from the input file name
  //
  char dirName[1024];
  sprintf(dirName, "%s", argv[fileNameIndex]);

  char *p = dirName + strlen(dirName) -1;
  do {
    if (p == dirName) break;
    if (*p == '/') break;
    p--;
  } while(1);

  if (*p == '/') 
    *(p+1) = char(0);
  else
    dirName[0] = char(0);

  //
  // Generate the output file name, open it.
  //
  char outFileName[1024];
  sprintf(outFileName, "%sREAL_%d%02d%02d_%02d%02d%02d.beamStats",
	  dirName,
	  firstBeamTime.year, firstBeamTime.month, firstBeamTime.day,
	  firstBeamTime.hour, firstBeamTime.min, firstBeamTime.sec);


  FILE *ofp = fopen(outFileName, "w");
  if (ofp == NULL){
    fprintf(stderr,"Failed to create %s\n", outFileName);
    return -1;
  }

  //
  // Loop through the file, reading beam data, writing stats to the output file.
  //

  unsigned long num = 0L;
  double total = 0.0;
  double totalTimes = 0.0;
  double max=0.0, min=0.0;
  int first = 1;
  unsigned long numTimes = 0L;



  date_time_t beamTime;

  do {

    beamTime.unix_time = h.unixTime;

    if (P->timeCheck.doCheck){
      if (fabs(beamTime.unix_time - now) > 86400*P->timeCheck.maxNumDaysDiff){
	fprintf(stderr, "WARNING : unix time %s in file differs from current time %s by more than %d days.\n",
		utimstr(beamTime.unix_time), utimstr(now), P->timeCheck.maxNumDaysDiff);
      }
    }

    uconvert_from_utime( &beamTime );

    unsigned long beamNum = 0L;
    double beamTotal = 0.0;
    double beamMax=0.0, beamMin=0.0;
    int beamFirst = 1;

    short int *beamData = (short int *)malloc(sizeof(short int)*h.gates);
    if (beamData == NULL){
      fclose(fp); fclose(ofp);
      fprintf(stderr, "Malloc failed!\n");
      return -1;
    }
    //
    // Read the beam data
    //
    if (h.gates != (int)fread(beamData, sizeof(short int), h.gates, fp)) break;

    //
    // Do some stats.
    //
    totalTimes += h.unixTime;
    numTimes++;


    int startGate = (int)rint(P->startRange/(double(h.gateSpacing)/1000.0));
    int endGate = (int)rint(P->endRange/(double(h.gateSpacing)/1000.0));
    
    if (endGate > h.gates-1) endGate = h.gates -1;
    if (startGate < 0) endGate = 0;

    for (int igate=startGate; igate <= endGate; igate++){

      if (
	  (beamData[igate] > P->minQcVal) &&
	  (beamData[igate] < P->maxQcVal)
	  ){
	beamTotal += beamData[igate];
	beamNum++;
	if (beamFirst){
	  beamMax = beamMin = beamData[igate];
	  beamFirst = 0;
	} else {
	  if (beamMax < beamData[igate]) beamMax = beamData[igate];
	  if (beamMin > beamData[igate]) beamMin = beamData[igate];
	}

      }
    }

    double beamMean = 0.0;
    if (beamNum > 0.0){
      beamMean = beamTotal / double(beamNum);
      if (first){
	first = 0; max = beamMax; min = beamMin;
      } else {
	if (max < beamMean) max = beamMean;
	if (min > beamMean) min = beamMean;
      }
    }

    num += beamNum;
    total += beamTotal;

    fprintf(ofp,"%ld %d %02d %02d %02d %02d %02d %ld %g %g %g %g %g\n",
	    beamTime.unix_time, beamTime.year, beamTime.month, beamTime.day,
	    beamTime.hour, beamTime.min, beamTime.sec, beamNum, beamMin, beamMean, beamMax,
	    h.azimuth, h.elevation );



    free(beamData);

    //
    // Read the next header.
    //
    if (1 != fread(&h, sizeof(h), 1, fp)) break;

  } while(1);

  fclose(fp); fclose(ofp);

  double mean = 0.0;
  date_time_t meanTime;
  meanTime.unix_time = 0L;

  if (num > 0){
    mean = total/double(num);
    meanTime.unix_time=(long)rint(totalTimes/double(numTimes));
  }
  uconvert_from_utime( &meanTime );

  sprintf(outFileName, "%sREAL_%d%02d%02d_%02d%02d%02d.FileStats",
	  dirName,
	  firstBeamTime.year, firstBeamTime.month, firstBeamTime.day,
	  firstBeamTime.hour, firstBeamTime.min, firstBeamTime.sec);


  ofp = fopen(outFileName, "w");
  if (ofp == NULL){
    fprintf(stderr,"Failed to create %s\n", outFileName);
    return -1;
  }


  fprintf(ofp,"%ld %d/%02d/%02d %02d:%02d:%02d\n",
	  firstBeamTime.unix_time, firstBeamTime.year, firstBeamTime.month, firstBeamTime.day,
	  firstBeamTime.hour, firstBeamTime.min, firstBeamTime.sec);

  fprintf(ofp,"%ld %d/%02d/%02d %02d:%02d:%02d\n",
	  meanTime.unix_time, meanTime.year, meanTime.month, meanTime.day,
	  meanTime.hour, meanTime.min, meanTime.sec);
  
  fprintf(ofp,"%ld %d/%02d/%02d %02d:%02d:%02d\n",
	  beamTime.unix_time, beamTime.year, beamTime.month, beamTime.day,
	  beamTime.hour, beamTime.min, beamTime.sec);
  

  fprintf(ofp, "%ld %g %g %g\n", num, min, mean, max);

  fclose(ofp);

  return 0;

}
