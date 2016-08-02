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
/***************************************************************************
 * read_disk_record.c
 *
 * reads a physical record from the tape
 *
 * On success, returns the size of the record read, and loads the data
 * into buffer
 *
 * On failure returns -1: this is either a read failure, or logical
 * end of tape.
 *
 * Gary Blackburn / Mike Dixon RAP NCAR September 1990
 *
 **************************************************************************/

#include "polar2gate.h"
static FILE * fdin;
static int first_call = TRUE;
static lass_params_t lass_header;
static lass_beam_params_t lass_beam;
static int volCounter = 0;
#define min(x,y) (x < y ? x: y)
#define max(x,y) (x > y ? x: y)

static int veryFirstTime = 1;
static char fileNames[10][80] ; 
/*
static char *fileNames[] = 
{
  "/home/sanjiv/titan/darwin/2D2060EC.vol",
  "/home/sanjiv/titan/darwin/2D206344.vol",
  "/home/sanjiv/titan/darwin/2D20659D.vol",
  "/home/sanjiv/titan/darwin/2D206A4C.vol"
};
*/
static int readChar(FILE *fd, char *data)

{

    si32 sz = 0,size = 0;
    int ret = 0;

    fread((char *)&size,4,1,fd);
    sz  = size;
    if ( sz == 0 )
    {
         *data = '\0';
         return 0;
    }

    if ( sz % 4 )
         size = sz + ( 4 - ( sz % 4));
    ret = fread(data,size,1,fd);
    data[size] = '\0';
    return ret;
 
}

static int getLassHeader(FILE *fd)
{
  si32 size;
  char data[MAXDATASIZE+1];
  int i;
  lass_params_t *ptr;
   
   ptr = &lass_header;

   readChar(fd,ptr->magic_data);
   if ( strcmp(ptr->magic_data,MAGICDATA) )
   {
       fprintf(stderr,"Input file is not of Lassen Data\n");
       return (-2);
   }

   fread((char *)&ptr->sTime,sizeof(struct lass_time_structure),1,fd);
   fread((char *)&ptr->fTime,sizeof(struct lass_time_structure),1,fd);
   fread((char *)&size,4,1,fd);
   readChar(fd,data);
   readChar(fd,data);
   fread((char *)&size,4,1,fd);
   fread((char *)&size,4,1,fd);
   readChar(fd,data);
   fread((char *)&size,4,1,fd);

   for ( i = 0; i < 12; i++ )
       fread((char *)&size,4,1,fd);

   fread((char *)&ptr->volSummary,sizeof(struct lass_volume_summary),1,fd);

   readChar(fd,ptr->radinfo.radar_name);
   readChar(fd,ptr->radinfo.site_name);
   fread((char *)&ptr->radinfo.antenna_height,4,1,fd);
   fread((char *)&ptr->radinfo.latdegree,4,1,fd);
   fread((char *)&ptr->radinfo.latminute,4,1,fd);
   fread((char *)&ptr->radinfo.latsecond,4,1,fd);
   fread((char *)&ptr->radinfo.londegree,4,1,fd);
   fread((char *)&ptr->radinfo.lonminute,4,1,fd);
   fread((char *)&ptr->radinfo.lonsecond,4,1,fd);
   
   return sizeof(ptr);
}

static int getLassBeam(FILE *fd, int *eof_flag)
{

lass_beam_params_t *beam;
int early,i,j,kk;
double vU,tV;
double dbConst1,dbConst2;
double velCons1,velCons2;
si32 size = 0;
ui08 echoData[LASS_NFIELDS*LASS_MAXGATES +1];
double vel,dbz;
static double dbzBias  = -32.0;
static double dbzScale = 0.25;
static double velBias  = -64.0;
static double velScale = 0.5;
double minDbz = 9999,minVel = 9999;
double maxDbz = -999,maxVel = -999;

  beam = &lass_beam;
  fread((char *)&beam->rays,sizeof(struct lass_ray_header ),1,fd);
 
  if ( !feof(fd) )
  {
       vU = 299792500.0 * (lass_header.volSummary.prf / 10.0) /
                          (4.0 * lass_header.volSummary.freq * 100000.0);
       early = 1;
       if ( lass_header.volSummary.year > 93 )
       {
            early = 0;
       }
       if ( ( lass_header.volSummary.year  == 93 ) &&
            ( lass_header.volSummary.month == 12 ) )
       {
             early = 0;
       }
       if ( early )
       {
            dbConst1 = 56;
            dbConst2 = 2;
            velCons1 = 128.0;
            velCons2 = 128.0;
       }
       else
       {
            dbConst1 = 64;
            dbConst2 = 2;
            velCons1 = 128.0;
            velCons2 = 127.0;
       }
       for ( i = 0; i < LASS_NUMOFFSETS; i++ )
       {
           size = size + ( lass_beam.rays.numgates * (lass_beam.rays.offset[i] != 0));
       }
       readChar(fd,(char *)echoData);
       lass_header.scaleFactor[0] =  dbzScale;
       lass_header.scaleFactor[1] =  velScale;
       lass_header. biasFactor[0] = dbzBias;
       lass_header. biasFactor[1] = velBias;
       for ( kk = 0, j = 0; j < size /2; j +=1, kk += 2)
       {
             dbz =   (( echoData[j] - dbConst1 ) / dbConst2) ;
             dbz =   (dbz - dbzBias)/dbzScale;
             if ( dbz < 0 ) dbz = 0;
             if ( dbz > 255 ) dbz = 255;
             minDbz = min(dbz,minDbz);
             maxDbz = max(dbz,maxDbz);
             lass_header.fieldValues[kk] = (ui08 )dbz; 
             tV = vU * ( (double )echoData[j+size/2] - velCons1) / velCons2;
             vel = tV ;
             vel = (vel - velBias)/velScale;
             if ( vel < 0 ) vel = 0;
             if ( vel > 255 ) vel = 255;
             minVel = min(vel,minVel);
             maxVel = max(vel,maxVel);
             lass_header.fieldValues[kk+1] = (ui08  )( vel );
   
       }
  }
  *eof_flag = 0;
  if ( feof(fd) )
     *eof_flag = 1;
  if ( Glob->debug )
  {
  	printf("minimum Dbz %f maxDbz %f\n",minDbz,maxDbz);
  	printf("minimum Vel %f maxVel %f\n",minVel,maxVel);
  }
  return (sizeof(lass_beam_params_t)+sizeof(lass_field_params_t));

}
static int maxCount = 0;

si32 read_lass_record (char *output_buffer)

{

  si32 eof_flag = 0;              /* end of file flag */
  si32 nread,count;
  int errcnt;
  int ii,oldsweep;

  /*
   * on first call, open device
   */

  if ( veryFirstTime )
  {
       veryFirstTime = 0;
       maxCount = 0;
       fdin = fopen(Glob->device_name,"r");
       if ( fdin == NULL )
   	   {
         fprintf(stderr, "\nERROR - %s:read_disk_record.\n",
	         Glob->prog_name);
         fprintf(stderr, "Opening Disk File\n");
         perror(Glob->device_name);
         tidy_and_exit(-1);
       }
       while ( !feof(fdin) )
       {
          fscanf(fdin,"%s",fileNames[maxCount]);
          maxCount++;
       }
       fclose(fdin);
       maxCount--;
  }
  
  start:
  nread = 0;
  if (first_call) {

  /**  if((fdin = fopen(Glob->device_name, "r")) == NULL) **/
    printf("fileName %s\n",fileNames[volCounter]);
    if((fdin = fopen(fileNames[volCounter], "r")) == NULL)
    {
      fprintf(stderr, "\nERROR - %s:read_disk_record.\n",
	      Glob->prog_name);
      fprintf(stderr, "Opening Disk File number %d\n",volCounter);
      fprintf(stderr, "Maximum Volumes are      %d\n",maxCount  );
      perror(Glob->device_name);
      tidy_and_exit(-1);
    }

    count = getLassHeader(fdin);
    /*lass_header.volSummary.volume += volCounter;*/
    first_call = FALSE;

  }

  count = sizeof(lass_params_t);
  errcnt = 0;

  do_read:

  nread = getLassBeam(fdin,&eof_flag);

  if ( !eof_flag )
  {
       if ( Glob->debug )
       for ( ii = 0; ii < 20; ii += 2 )
       {
             printf("input db %d vel %d \n",lass_header.fieldValues[ii],
                                            lass_header.fieldValues[ii+1]);
       }
       lass_header.rays        = lass_beam.rays;
       lass_header.myAzymuth   = lass_beam.rays.vangle * 360 / 16384.0;
       oldsweep    = lass_beam.rays.sweep;
       lass_header.targetAngle = lass_header.volSummary.fangles[oldsweep-1]*360.0 / 16384; 
       lass_header.myElevation  = lass_beam.rays.fanglea * 360 / 16384.0;
       lass_header.sweep       = lass_beam.rays.sweep;
       lass_header.rangeTo1stGate = lass_header.volSummary.gatewid / 2;
       switch ( lass_header.volSummary.sweep_type )
       {
             case LASS_SWEEP_POINT: /* 0 */
                  lass_header.volSummary.sweep_type = GATE_DATA_UNKNOWN_MODE;
	              break;
             case LASS_SWEEP_PPI:
                  lass_header.volSummary.sweep_type = GATE_DATA_SURVEILLANCE_MODE;
                  break;
             case LASS_SWEEP_RHI:
                  lass_header.volSummary.sweep_type = GATE_DATA_RHI_MODE;
                  break;
             case LASS_SWEEP_SEC:
                  lass_header.volSummary.sweep_type = GATE_DATA_SURVEILLANCE_MODE;
                  break;
       }
       if (nread < 0) {

           fprintf(stderr, "ERROR - %s:read_disk_record\n",
	               Glob->prog_name);
           perror(Glob->device_name);
           if(++errcnt > 20)
               return (-1L);  /* quit if many errors */
           goto do_read;

       }

  }
  else 
  {
      first_call = TRUE;
      volCounter++;
      if ( volCounter >= maxCount ) volCounter = 0;
      fclose(fdin);
      goto start;
  }
  count = sizeof(lass_params_t);
  memcpy((void *)output_buffer,(void *)&lass_header,count);
  return count;
}

