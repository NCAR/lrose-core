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
// Program PrintSST prints Sea Surface Temperature data from a file
//
// Author: Curtis Caravone
// Date:   1/31/2003
//

#include "SSTData.hh"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 3 && argc != 7) {
        fprintf(stderr, "Usage: %s SST_file record_number [xmin xmax ymin ymax]\n", argv[0]);
        fprintf(stderr, "Record number 1 = Sea Surface Temperature\n");
        fprintf(stderr, "              2 = Number of point per bin\n");
        fprintf(stderr, "              3 = Sea Surface Temperature Anomaly\n");
        fprintf(stderr, "              4 = Interpolated Sea Surface Temperature\n");
        fprintf(stderr, "              5 = Interpolated SST Anomaly\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "              6 = Raw Sea Surface Temperature\n");
        fprintf(stderr, "              7 = Raw Number of point per bin\n");
        fprintf(stderr, "              8 = Raw Sea Surface Temperature Anomaly\n");
        fprintf(stderr, "              9 = Raw Interpolated Sea Surface Temperature\n");
        fprintf(stderr, "              10 = Raw Interpolated SST Anomaly\n");
        return 2;
    }
    FILE *f = NULL;
    f = fopen(argv[1], "r");
    if (f == NULL) {
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        return 1;
    }
    SSTData data(f);
    fclose(f);
    int rec_num = atoi(argv[2]);

    int xmin = 0;
    int xmax = SST_COLUMNS;
    int ymin = 0;
    int ymax = SST_ROWS;

    if (argc > 3) {
        xmin = atoi(argv[3]);
        xmax = atoi(argv[4]);
        ymin = atoi(argv[5]);
        ymax = atoi(argv[6]);
    }

    switch (rec_num) {
      case 1: {
          float *sst = data.getSST();
          for (int i = ymin; i < ymax; i++) {
              for (int j = xmin; j < xmax; j++) {
                  printf("%10.2f", sst[i * SST_COLUMNS + j]);
              }
              printf("\n");
          }
          break;
      }
      case 6: {
          ui08 *sst_raw = data.getRawSST();
          for (int i = ymin; i < ymax; i++) {
              for (int j = xmin; j < xmax; j++) {
                  printf("%5d", sst_raw[i * SST_COLUMNS + j]);
              }
              printf("\n");
          }
          break;
      }

      case 2: {
          int *npoints = data.getNumPoints();
          for (int i = ymin; i < ymax; i++) {
              for (int j = xmin; j < xmax; j++) {
                  printf("%5d", npoints[i * SST_COLUMNS + j]);
              }
              printf("\n");
          }
          break;
      }
      case 7: {
          ui08 *npoints_raw = data.getRawNumPoints();
          for (int i = ymin; i < ymax; i++) {
              for (int j = xmin; j < xmax; j++) {
                  printf("%5d", npoints_raw[i * SST_COLUMNS + j]);
              }
              printf("\n");
          }
          break;
      }

      case 3: {
          float *anomaly = data.getSSTAnomaly();
          for (int i = ymin; i < ymax; i++) {
              for (int j = xmin; j < xmax; j++) {
                  printf("%10.2f", anomaly[i * SST_COLUMNS + j]);
              }
              printf("\n");
          }
          break;
      }
      case 8: {
          ui08 *anomaly_raw = data.getRawSSTAnomaly();
          for (int i = ymin; i < ymax; i++) {
              for (int j = xmin; j < xmax; j++) {
                  printf("%5d", anomaly_raw[i * SST_COLUMNS + j]);
              }
              printf("\n");
          }
          break;
      }

      case 4: {
          if (!data.interpAvailable()) {
              printf("Interpolated data not available\n");
          } else {
              float *interpSST = data.getInterpSST();
              for (int i = ymin; i < ymax; i++) {
                  for (int j = xmin; j < xmax; j++) {
                      printf("%10.2f", interpSST[i * SST_COLUMNS + j]);
                  }
                  printf("\n");
              }
          }
          break;
      }
      case 9: {
          if (!data.interpAvailable()) {
              printf("Interpolated data not available\n");
          } else {
              ui08 *interpSST_raw = data.getRawInterpSST();
              for (int i = ymin; i < ymax; i++) {
                  for (int j = xmin; j < xmax; j++) {
                      printf("%5d", interpSST_raw[i * SST_COLUMNS + j]);
                  }
                  printf("\n");
              }
          }
          break;
      }

      case 5: {
          if (!data.interpAvailable()) {
              printf("Interpolated data not available\n");
          } else {
              float *interpAnomaly = data.getInterpSSTAnomaly();
              for (int i = ymin; i < ymax; i++) {
                  for (int j = xmin; j < xmax; j++) {
                      printf("%10.2f", interpAnomaly[i * SST_COLUMNS + j]);
                  }
                  printf("\n");
              }
          }
          break;
      }
      case 10: {
          if (!data.interpAvailable()) {
              printf("Interpolated data not available\n");
          } else {
              ui08 *interpAnomaly_raw = data.getRawInterpSSTAnomaly();
              for (int i = ymin; i < ymax; i++) {
                  for (int j = xmin; j < xmax; j++) {
                      printf("%5d", interpAnomaly_raw[i * SST_COLUMNS + j]);
                  }
                  printf("\n");
              }
          }
          break;
      }

      default:
          fprintf(stderr, "Invalid record number\n");
          return 1;
    }
}
