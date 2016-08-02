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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/mtio.h>

#define NUM_RECS 3   /*  number of records to display in 'ufshow.out'
			 output file  */
 
main(argc, argv)   /* use Lassen file name and sweep number as arguments */
int argc;
char **argv;
{
    int fi;
    FILE   *fout, *fopen();
    short Buffer[65536], ashort;
    int ii,j;
    int sweep_fl, sweep_no;
    int dpt, pos1;
    int   recl, rlen, bpoint;
    char aaa[3];

    if(argc<3)
    {
        fprintf(stderr, "ufshow <uf1> <sweep_no>");
        exit(0);
    }

   fi = open(argv[1],0);
   sweep_no = atoi(argv[2]);
   printf("\n sweep_no = %d",sweep_no);
   if (fi < 0)
   {
      printf("\n !! Error opening file - may be the wrong file name !!\n");
      exit(0);
   }
 
   /* fout = fopen("ufshow.out","w"); */
   fout = stdout;
   ii = 0;
   int count = 0;
   for(;;)
   {
     if(read(fi,&rlen,4) <= 0) break;
     rlen = ntohl(rlen);
     if (rlen == 0) break;
     recl = rlen;
     if(read(fi,(char *)Buffer,recl) <= 0) break;
     if(read(fi,&bpoint,4) <= 0) break;
     bpoint = ntohl(bpoint);
     sweep_fl = (int)(ntohs(Buffer[9]));
     if (sweep_fl < sweep_no) continue;
     if (sweep_fl > sweep_no) break;
     ii++;
     if (ii>NUM_RECS) break;
     fprintf(fout,
       "sweep_no  recno_sweep  16bit_word_no  dec_rep char_rep\n");
     for (j=0;j<recl/2;j++) {
       strncpy(aaa,(char *)&Buffer[j],2);
       if (!isprint(aaa[0]) || !isprint(aaa[1])) {
	 aaa[0] = '.';
	 aaa[1] = '.';
       }
       aaa[2] = '\0';
       if(aaa[0] == 'U' && aaa[1] == 'F') {
	 count = 0;
       }
       ashort = ntohs(Buffer[j]);
       count++;
       fprintf(fout, "%5d    %2d          %1d         %4d         %6d     %2s\n",
	       count, sweep_fl,ii,j+1,ashort,aaa);
     }
     fprintf(fout,"\n");
   }
   close(fi);     /*  close UF1 file  */
   close(fout);     /*  close output file  */
}
