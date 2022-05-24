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
#include <ctime>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <stdlib.h>
#include <cmath>

#include <rapformats/ltg.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/umisc.h>

#include "Params.hh"

#define LINE_LENGTH 1 
#define UNIX_TIME_DIFF 320630400
#define LON_CONST 0.001068115234
#define LAT_CONST 0.001281738
#define UNIX_DAY 86400
#define FILE_HEADER 30
#define VERSION 1.0

void usage();
void backUp();
void tossHeader();
int parseDate();
int get2Bytes();
int getByte();

FILE           *source_fd;
FILE           *logfile;
char           buffer[LINE_LENGTH];
char           program_name[128];


int ltg_decrypt(char *inFilename, Params *P){

  date_time_t fileNameTime;

  if (
      (strlen(inFilename) < strlen("20070216_1612.ltg")) ||
      (5 != sscanf(inFilename + strlen(inFilename) - strlen("20070216_1612.ltg"),
                   "%4d%2d%2d_%2d%2d",
                   &fileNameTime.year, &fileNameTime.month, &fileNameTime.day,
                   &fileNameTime.hour, &fileNameTime.min))
      ){
    fprintf(stderr, "I need a valid file name, like 20070216_1612.ltg\n");
    return -1;
  }

  uconvert_to_utime( &fileNameTime );


  if ((source_fd = fopen(inFilename, "r")) == NULL)
  {
    fprintf (stderr,"Could not open input file %s\n",inFilename);
    return (1);
  }


  // Init the SPDB database for output.
  DsSpdb OutSpdb;
  OutSpdb.clearPutChunks();
  OutSpdb.clearUrls();
  OutSpdb.addUrl(P->OutputUrl);



  int            multiplicity, 
    duration, 
    kiloamps, 
    x,
    log = 0,
    quality_flag;

  float          latitude, 
    longitude;

  unsigned short num_days = 5,
    byte0, 
    byte1,
    byte2,
    two_bytes,
    flash_count,
    prevByte = 0,
    flash_type;

  unsigned long  calc_time,
    curr_time,
    too_old,
    big_byte,
    position = 0;

  char    *inputfile = inFilename;

  //time_t time(time_t *tloc);
  STRncopy(program_name, "noaaportLtg2Spdb", 128);

//---------------------------------------------------------------------------
// process command line arguments

 
 

 log = 0;
 num_days = 50;
 

  /* Get the current time */
  curr_time = time(NULL);

  /* how may days old can the data be? */ 
  too_old = num_days * UNIX_DAY;

  if (log == 1)
  { 
    fprintf(logfile, "Processing file %s with a max data age of %d days\n", inputfile, num_days);
  }
 
  //------------------------------------------------------------------------
  // this is where we start processing the file



  /* Toss out the file header */
  tossHeader();

  /* Read out the lightning data. */
  while (fgets(buffer, 2, source_fd) != NULL)
  {
    byte0 = 0;
    memcpy((void *)&byte0, buffer, 1);

    if ((0x96 == byte0) || (0x97 == byte0))
    {

      //
      // Type 0x97 is an experimental type.
      // The default is not to ingest this.
      //
      if ((0x97 == byte0) && (!(P->ingestExperimental))) continue;

      // skip 0x96 or 0x97 bytes if they are the first byte of the file
      // after the header as legitimate 0x96 and 0x97 bytes are preceeded 
      // by a flash/count byte
      if (position < 1)
      {
         continue;
      }

      flash_type = byte0;
      flash_count = prevByte; 

      calc_time = parseDate(); 
      calc_time += UNIX_TIME_DIFF;

      if (log == 1)
      { 
         fprintf(logfile, "Flash message is %d\n", flash_type);
         fprintf(logfile, "Number of flashes is %d\n", flash_count);
         fprintf(logfile, "Timestamp is %d\n", calc_time);
      }

      // if the timestamp is bad, then we want to skip back three
      // bytes (back to the start of the timestamp) and continue 
      // looking for flash messages 
      if (fabs(double(calc_time) - double(fileNameTime.unix_time)) > P->MaxRealtimeFileTimeDiff)
      {
         if (log == 1)
         {
            fprintf(logfile, "   bad timestamp\n");
         }
         backUp();
         continue; 
      }
      else
      {
         for(x=1; x<= flash_count; x++)
         {
            if (0x96 == flash_type) 
            {
               two_bytes = get2Bytes(); 
               longitude = (two_bytes * LON_CONST) - 130;

               two_bytes = get2Bytes();
               quality_flag = two_bytes >> 15;

               two_bytes = two_bytes & 0x7FFF;
               latitude = (two_bytes * LAT_CONST) + 18;

               byte0 = getByte();
               kiloamps = (byte0 & 0X7F)*2;
               if (byte0 >> 7) { kiloamps = -kiloamps; }
          
               byte0 = getByte();
               duration = (byte0 >> 4);
               multiplicity = byte0 & 0x0F;
            }
            else
            {  // flash_type = 0x97

               // the original C# code said: get 4 bytes (not 3) and then use
               // the bottom 24 bits.  This doesn't work :(  But getting 3 
               // bytes and dropping the bottom bit does 
        
               big_byte = get2Bytes(); 
               byte2 = getByte();
               big_byte = big_byte | (byte2 << 16);
               longitude = ( (big_byte & 0x7FFFFF) / 16777216.0 * 360.0 ) - 180.0;

               // based on the above, I *think* kiloamps must be the next 
               // byte plus the last bit of the previous byte - but this is 
               // guesswork on my part!!! Original C# code: next byte only 

               byte0 = getByte();
               byte2 = byte2 >> 7;
               kiloamps = (byte0 + byte2);
               if (kiloamps > 127) { kiloamps = (128 - kiloamps)*2; }
               else { kiloamps *= 2; } 

               // latitude is almost the same... 

               byte0 = getByte();
               byte1 = getByte();        
               byte2 = getByte();
               big_byte = byte0 | (byte1 << 8);
               big_byte = big_byte | (byte2 << 16);
               latitude = ( (big_byte & 0x7FFFFF) / 16777216.0 * 360.0 ) - 90.0;

               // then this last byte + a bit of the previous byte must be 
               // the multiplicity...which I don't know much about.
               // Per GT's database, it should be between 1 and 10.
               // But perhaps it should be more?  0 to 19?
               // This needs more research

               byte0 = (getByte() >> 4);
               byte2 = byte2 >> 7;
               multiplicity = (byte2 * 10) + byte0;
            } 
            if (P->Debug)
	      fprintf (stderr, "%d %f %f %d %d %x %d\n", calc_time, longitude, latitude, kiloamps, multiplicity, flash_type, flash_count);


	    if( (longitude > P->MinLon) && (longitude < P->MaxLon) &&
		(latitude > P->MinLat) && (latitude < P->MaxLat))
	      { 
		
		LTG_strike_t strk;
		memset(&strk, 0, sizeof(LTG_strike_t));
		
		strk.type =  LTG_TYPE_UNKNOWN;
		strk.time = calc_time;
		strk.longitude = longitude;
		strk.latitude = latitude;
		strk.amplitude = kiloamps;
		
		
		// Assign ID's based on Location 
		int dataType, dataType2;
		dataType = (int) rint( (strk.latitude + 90.0) * 100.0);
		dataType2 = (int) rint( (strk.longitude + 180.0) * 100.0);

		LTG_to_BE( &strk );
	    
		OutSpdb.addPutChunk( dataType, calc_time, 
				     calc_time + P->ExpireSecs, sizeof(strk),  
				     &strk, dataType2);
                
        
	      } // Is within the bounding box 
         }
      }
    }
    else            /* this wasn't a flash message; save the byte and go on */ 
    { prevByte = byte0; } 
    position++;

  } /* While there is still data */

  if (source_fd) fclose(source_fd);

  //
  // Commit the data to the database, close the file, and exit
  //
  OutSpdb.put( SPDB_LTG_ID, SPDB_LTG_LABEL);


  return 0;

}
/****************************************************************************/
/* some useful functions                                                    */

//--------------------------------------------------------------------------
void tossHeader()
{
   int x = 0;
   unsigned short byte;
   while (x <= FILE_HEADER)
   {
     byte = getByte();
     x++;
   }
}
//--------------------------------------------------------------------------
int getByte()
{
   unsigned short byte = 0;
   fgets(buffer, 2, source_fd);
   memcpy((void *)&byte, buffer, 1);
   return byte;
}
//--------------------------------------------------------------------------
int get2Bytes()
{
   unsigned short byte0,
                  byte1;

   byte0 = getByte();
   byte1 = getByte();
   byte0 = (byte1 << 8) | byte0;
   return byte0;
}
//--------------------------------------------------------------------------
int parseDate ()
{
   unsigned short byte0 = 0,
                  byte1 = 0,
                  two_bytes,
                  days,
                  hours,
                  mins,  
                  secs;
   unsigned long  raw_time;

   byte0 = getByte();
   byte1 = getByte();
   two_bytes = get2Bytes();

   days  = (two_bytes & 0xFFFE) >> 1;

   hours = (two_bytes & 0x0001) << 4;
   hours = hours | (((byte1 & 0xF0) >> 4) & 0x0F);

   mins  = (byte1 & 0x0F) << 2;
   mins  = mins | (byte0 & 0xC0) >> 6;

   secs  = byte0 & 0x3F;
   raw_time = (((((days * 24) + hours) * 60) + mins) * 60) + secs;
   return raw_time;
}
//--------------------------------------------------------------------------
void backUp ()
{
   int position = 0;
   position = ftell(source_fd);
   position -= 4;
   if (fseek(source_fd, position, SEEK_SET))
   {
      /* I don't think this can happen, but just in case. */
      printf("File pointer position error in input file.\n");
      if (source_fd) fclose(source_fd);
      if (logfile) fclose(logfile);
      exit(2);
   } 
}
//--------------------------------------------------------------------------
void usage()
{
  printf ("Usage: %s [-v] [-h] [-i input] [-n days] [-l log]\n", program_name);
  printf ("-v                : print program version\n");
  printf ("-h                : print help message\n");
  printf ("-i input_file     : input filename\n");
  printf ("-n max_days_old   : max age of data in days\n");
  printf ("-l logfile        : logfile\n");
  printf ("\n");
  printf ("Default max_days_old is 5 days\n");
  printf ("Include the path if not '.'\n");
  exit(0);
}
