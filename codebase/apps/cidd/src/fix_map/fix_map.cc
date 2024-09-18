/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/******************************************************************************
 * FIX_MAP
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <toolsa/str.h>

/******************************************************************************
 * MAIN :   Open files and send the output to STDOUT
 *
 */
int main(int argc, const char **argv)
{
  int  i,num_fields;
  char buf[256];
  FILE *infile;
  double lon,lat;
  char *cfield[8];

  for(i=0; i < 8; i++) {
    cfield[i] = (char *) calloc(1,64);
  }
          

  if(argc != 4) { 	/* take the input from stdin */
    fprintf(stderr,"Usage: fix_map lat_offset lon_offset file > newfile\n");
    exit(-1);
  }
  double lat_offset = atof(argv[1]);
  double lon_offset = atof(argv[2]);

  if((infile = fopen(argv[3],"r")) == NULL) {
    fprintf(stderr," fix_map: can't open %s\n",argv[3]);
    fprintf(stderr,"Usage: fix_map file > newfile\n");
    exit(-1);
  }

  while(fgets(buf,256,infile) != NULL) {
    num_fields = STRparse(buf,cfield,256,8,64); 
    if(num_fields != 2 ||  *buf < '-') {
      fputs(buf,stdout);
    } else {
      lat = atof(cfield[0]);
      lon = atof(cfield[1]);
      if(lon <= -1000.0) {
        fputs(buf,stdout);
      } else {
        lat += lat_offset;
        lon += lon_offset;
        printf("%.4f %.4f\n",lat,lon);
      }


    }
  }


}
