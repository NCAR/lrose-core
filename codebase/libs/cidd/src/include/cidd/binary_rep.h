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
#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 * BINARY_REP.H Macros and function prototypes that assist in 
 *   portabilly representiong binary data
 *  F. Hage 8/95 - 
 */

#ifndef BINARY_REP_H
#define BINARY_REP_H

#include <sys/types.h>

/* The following type defs should resolve to the following sizes :
 * Byte     -> 1 byte (8 bits)
 * Short16  -> 2 bytes
 * Int32    -> 4 bytes
 * Long64   -> 8 bytes 
 *
 * Float32  -> 4 bytes   IEEE single precision floating point
 * Double64 -> 8 bytes   IEEE double precision floating point
 */
  
  typedef signed char Byte;
  typedef unsigned char	UByte;
  
  typedef int16_t Short16;
  typedef u_int16_t UShort16;
  
  typedef int32_t Int32;
  typedef u_int32_t UInt32;
  
  typedef float Float32; 
  typedef double Double64; 

  typedef int64_t Long64;
  typedef u_int64_t ULong64; 
 
#define BR_Reverse_Ints(arg,n) BR_Reverse_4byte_vals((UInt32 *)(arg),(n));
#define BR_Reverse_Floats(arg,n) BR_Reverse_4byte_vals((UInt32 *)(arg),(n));
#define BR_Reverse_Shorts(arg,n) BR_Reverse_2byte_vals((UShort16 *)(arg),(n));

/* These functions return 1 if True, 0 otherwize */
  int BR_host_is_big_endian(void);
  int BR_host_is_little_endian(void);
  
  void BR_Reverse_4byte_vals(UInt32* array, Int32 num);
  void BR_Reverse_2byte_vals(UShort16* array, Int32 num);

#define BR_REVERSE_SHORT(arg) (((arg)  << 8) | ((arg) >> 8))
#define BR_REVERSE_INT(arg) (((arg) << 24) | (((arg) & 0xFF00) << 8) | (((arg) >> 8) & 0xFF00) | ((arg) >> 24))

#endif /* BINARY_REP_H */

#ifdef __cplusplus
}
#endif
