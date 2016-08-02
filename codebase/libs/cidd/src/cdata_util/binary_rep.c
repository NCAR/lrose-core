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
 *  BINARY_REP.C  Subroutines useful for dealing with differences in
 *   binary representations of numbers.
 *
 *  F. Hage.  Jan 1996. RAP
 */

#define BINARY_REP_C

#include <cidd/binary_rep.h>

/******************************************************************************
 * BR_host_is_big_endian(); Returns 1 if true, 0 otherwise
 */

int BR_host_is_big_endian(void)
{
  union {
      UShort16    sh_int;
      UByte       bytes[2];
  } int_union;

  int_union.sh_int = 1;
  if (int_union.bytes[1] != 0) return (1);
  else return (0);
}


/******************************************************************************
 * BR_host_is_little_endian(); Returns 1 if true, 0 otherwise
 */

int BR_host_is_little_endian(void)
{
  if (BR_host_is_big_endian()) return(0);
  else return (1);
}


/******************************************************************************
 * BR_Reverse_4byte_vals: 
 *
 */

void BR_Reverse_4byte_vals(UInt32* array, Int32 num)
{
    UInt32  value;
    while (num--) {
      value = *array;
      *array =  BR_REVERSE_INT(value);
      array++;
    }
}

/******************************************************************************
 * BR_Reverse_2byte_vals: 
 *
 */

void BR_Reverse_2byte_vals(UShort16* array, Int32 num)
{
    UShort16  value;
    while (num--) {
      value = *array;
      *array =  BR_REVERSE_SHORT(value);
      array++;
    }
}
