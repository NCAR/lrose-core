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
 * Function:       swap_ddir                                                
 * 
 * Description:                                                        
 *    Swaps a data directory struct  (headers first, then data)
 *
 *    NOTE: Does not swap data of unknown lengths -- right now just
 *    swaps shorts,ints and the time struct.
 *    DOES NOT SWAP fssp information or image info.
 * Input:
 *    a data dir in some format
 * Output:
 *    a swapped data dir
 **********************************************************************/

#include "ac_tape_recover.h"

si32 swap_ddir(DataDir *ddir)
{
   si16 sdata_tag;
   si32 i;
   si32 data_tag;
   si32 nbytes;
   si32 errflg;
   DataDir *ddir_beg;

/**********************************************************************/

/* set pointer to beginning of data so swap_ddir_data knows 
 * where to start offset */
   ddir_beg = ddir;

   sdata_tag = ddir[0].tagNumber;
   nbytes = SWAP_array_16((ui16 *) &sdata_tag,2);
   data_tag = (si32)sdata_tag;

   i = 0;
   while  (data_tag!=LAST_TAG && data_tag!=NEXT_TAG && data_tag!= SAME_TAG) {

      errflg = swap_ddir_header(&ddir[i]);
      if (errflg != OKAY) {
        fprintf(stderr,"Error swapping data dir header\n");
        return(ERROR);
      }

      errflg = swap_ddir_data(&ddir[i],ddir_beg);
      if (errflg != OKAY) {
        fprintf(stderr,"Error swapping data dir data\n");
        return(ERROR);
      }

      i++;
      sdata_tag = ddir[i].tagNumber;
      nbytes = SWAP_array_16((ui16 *) &sdata_tag,2);
      data_tag = (si32)sdata_tag;

   }  /* end while data tag's are valid */

/* make sure and swap the special data tag!? */
    ddir[i].tagNumber = (short)data_tag;

   return(OKAY);
}

/**********************************************************************
 * a data dir header is always 16 bytes -- 5 shorts, 4 chars and 1 short
 **********************************************************************/

si32 swap_ddir_header(DataDir *ddir)

{

  si32 nbytes;

   nbytes = SWAP_array_16((ui16 *) &ddir->tagNumber,10);
   if (nbytes != 10) {
      fprintf(stderr,"Error swapping 10 bytes!\n");
      return(ERROR);
   }

   nbytes = SWAP_array_16((ui16 *) &ddir->Address,2);

   if (nbytes != 2) {
      fprintf(stderr,"Error swapping 2 bytes!\n");
      return(ERROR);
   }

   return(OKAY);
}


/**********************************************************************
 * only swaps 2,4 byte data values and a time struct of 18 bytes
 **********************************************************************/

si32 swap_ddir_data(DataDir *ddir,DataDir *ddir_beg)

{

   si32 nbytes,offset;
   si32 nswap;
   si32 data_tag;
   si32 temp_si32;
   si16 temp_si16;
   ui08 *data_loc;
   ui08 temp_ui08[24];
  
/**********************************************************************/

   data_tag = (si32)ddir->tagNumber;
   offset = (si32)ddir->dataOffset;
   nbytes = (si32)ddir->numberBytes;

   memset(temp_ui08,0,sizeof(temp_ui08));

   data_loc = (ui08 *)ddir_beg;
   data_loc += offset;

/* swap according to size -- 
 * if it can't swap, just fake it and return nbytes */

   switch (nbytes) {

      case 2:  memcpy(&temp_si16,data_loc,nbytes); 
               nswap = SWAP_array_16((ui16 *) &temp_si16,nbytes); 
               memcpy(data_loc,&temp_si16,nbytes); 
               break;

      case 4:  memcpy((char *)&temp_si32,data_loc,nbytes); 
               nswap = SWAP_array_32((ui32 *) &temp_si32,nbytes); 
               memcpy(data_loc,(char *)&temp_si32,nbytes); 
               break;

      case 18: memcpy(&temp_ui08,data_loc,nbytes); 
               nswap = SWAP_array_16((ui16 *)&temp_ui08,nbytes);
               memcpy(data_loc,temp_ui08,nbytes); 
               break;

      default: nswap=nbytes;

   }


   if (nswap != nbytes) {
      fprintf(stderr,"Error swapping %d bytes!\n",nbytes);
      return(ERROR);
   }

   return(OKAY);
}

