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
#if defined(F_UNDERSCORE) || defined(F_UNDERSCORE2)
#define copen copen_
#define cclose cclose_
#define cread cread_
#define cwrite cwrite_
#define findgrib findgrib_
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <memory.h>
#include <signal.h>
#include <sys/stat.h>
/* #include <g2c.h> */

#define TRUE 1
#define FALSE 0
extern int errno;

/*---+----------------------------------------------------------------*/
FILE *copen(char *filename, char *faccess, int len1, int len2)

{
/*   printf("C_open - filename - %s \n",filename);   */
  return(fopen(filename,faccess));
}
/*---+----------------------------------------------------------------*/

int cclose(FILE **file)

{
/*  printf("C_close - filehandle\n");    */
  return(fclose(*file));
}
/*---+----------------------------------------------------------------*/

int cread(int *fbyte, int *numbytes, void *a, FILE **file)

{
  int retcode;

  retcode=fseek(*file,*fbyte,0);
  if(retcode != 0)
   {
     printf("C_read error\n");
     return(retcode);
   }
  retcode=fread((char*)a,1,*numbytes,*file);
  if(retcode != *numbytes)
   {
     return(errno);
   }
  else
   {
     return(0);
   }

}
/*---+----------------------------------------------------------------*/

int cwrite(int *fbyte, int *numbytes, void *a, FILE **file)

{
  int retcode;

  retcode=fseek(*file,*fbyte,2);
  if(retcode != 0)
   {
     printf("C_write error\n");
     return(retcode);
   }
  retcode=fwrite((char*)a,1,*numbytes,*file);
  if(retcode != *numbytes)
   {
     return(errno);
   }
  else
   {
     return(0);
   }
}
/*---+----------------------------------------------------------------*/

int findgrib(int *ibeg,FILE **file)
 
{
  int found, eof, grib_byte, buf_counter, num;
  char buf[8200],*place_ptr,*t_ptr;
  grib_byte=0;
  found=FALSE;
  eof=FALSE;
  buf_counter=0;
  rewind(*file);
 
/*  printf("Seeking to byte number %i\n",*ibeg); */
  fseek(*file, *ibeg, 0);
 
  while (!found & !eof)
  {
    errno = 0;
    if(fread(buf,sizeof(char),8192,*file) != 8192 && errno != EINTR ) eof=TRUE;
    buf[8192]=0;
/*         replace occurrences of null with a blank in buf
           because strstr will stop when it reaches a null
*/
    t_ptr = buf;
    num = 8192;
    while((t_ptr = memchr(t_ptr,0,num)) != NULL)
    {
      *t_ptr=' ';
      num = 8192 - (t_ptr-buf);
    }
    place_ptr = strstr(buf,"GRIB");
    if(place_ptr == NULL)
    {
      buf_counter+=8192;
    }
    else
    {
      found=TRUE;
      grib_byte=(place_ptr - buf) + buf_counter;
    }
  }
  return(grib_byte);
}
