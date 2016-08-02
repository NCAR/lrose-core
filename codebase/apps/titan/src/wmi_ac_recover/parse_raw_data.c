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
/**********************************************************************
 * Function:       parse_raw_data                                          
 *
 * Description:                                                        
 *    Goes through a DataDirectory and fills in a StateDataStruct 
 *    Note that the order of the variables in the StateDataStruct are
 *    the same as in the origianal data.  Calculated fields are added
 *    as needed.
 *    Note: This routine calls all the calc routines and is really
 *    the most important routine of the whole bunch.
 *
 * Input:              
 *    a character buffer which is really a datadir struct             
 * Output:
 *    a filled in state data struct 
 **********************************************************************/

#include "wmi_ac_recover.h"

void parse_raw_data(ui08 *buf)

{

   static fl64 analog2volts[3]={3.051757813E-4,1.525878906E-4,7629394531E-5};

   si16 short_val;
   si32 i,ifld,icv;
   si32 data_tag, offset, data_size;
   si32 int_val;
   fl64 volts_val;

/* local variables used for calculations */
   fl64 static_pres = BAD_VAL;
   fl64 lat         = BAD_VAL;
   fl64 lon         = BAD_VAL;
   fl64 palt        = BAD_VAL;

   TimeStruct ts;
   date_time_t dt;
   DataDir *ddir;

/**********************************************************************/

   ddir= (DataDir *)buf;
   data_tag = (si32)ddir[0].tagNumber;

/* read each field in data dir directory and do necessary calculations */

   i = 0;
   while (data_tag!=LAST_TAG && data_tag!=NEXT_TAG && data_tag!=SAME_TAG) {

      offset    = (si32)ddir[i].dataOffset;
      data_size = (si32)ddir[i].numberBytes;

      /*if (Glob->params.debug) fprintf(stderr,"tag,offset,size are: %d %d %d\n",
                                     data_tag,offset,data_size); */

/*------------> time field so parse out data and put into time struct */
      if (data_tag == TIME_TAG) {

         memcpy(&ts,buf+offset,data_size);

      }

/*------------> Not time field so get out data into integer */

      /* 2 byte value */
      if (data_size == sizeof(si16))  {
         memcpy(&short_val,buf+offset,sizeof(si16));
         int_val = (si32)short_val;
      }

      /* 4 byte value */
      if (data_size == sizeof(si32))  {
         memcpy(&int_val,buf+offset,sizeof(si32));
      } 


      /* Go through fields and find out location in aquisition/calculated
       * variables table so that correct analog2volts may be applied.  
       * Then, do whatever calculations necessary to convert volts 
       * to engineering units. If these are not dependent on other fields! */

      for (ifld = 0; ifld < Glob->num_raw_fields; ifld++) {

         if (data_tag == Glob->acqtbl[ifld].tag) {

/*------------> Pressure variables SPRES*/

            if (strncmp(Glob->acqtbl[ifld].var_type,SPRES,5) == 0) {

               volts_val=(fl64)((fl64)(int_val)*analog2volts[Glob->acqtbl[ifld].para3]);
               static_pres = calc_pres(volts_val,
                                       Glob->fc[ifld].c1,
                                       Glob->fc[ifld].c2);
 
/* -----------> PALT Pressure Altitude */

               if ((icv = find_cvar(PALT,4)) != BAD_INDEX) {

                  palt= calc_palt(static_pres);

               } /* endif palt is to be calculated */

            }  /* endif SPRES/PPRES variable */

/* -----------> LAT/LON variables  */

            if (strncmp(Glob->acqtbl[ifld].var_type,LAT,3) == 0) {

               lat = calc_latlon(&int_val);

            }


            if (strncmp(Glob->acqtbl[ifld].var_type,LON,3) == 0) {

               lon = calc_latlon(&int_val);

            }

         } /* end looking through fields for computation */

      } /* end looking through tags */

      /* check next tag */
      i++;
      data_tag = (si32)ddir[i].tagNumber;

   }  /* end while reading through valid tags */

   if (lat != BAD_VAL && lon != BAD_VAL) {
     if (lat != 0.0 || lon != 0) {
       dt.year = ts.year;
       dt.month = ts.mon;
       dt.day = ts.day;
       dt.hour = ts.hour;
       dt.min = ts.min;
       dt.sec = ts.sec;
       uconvert_to_utime(&dt);
       store_pos(&dt,lat,lon,palt,Glob->params.callsign);
     }
   }
    
}
