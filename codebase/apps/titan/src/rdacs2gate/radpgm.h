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
/**************************************************************
 * radpgm.h
 *
 * RADTEK RDACS radar data aquisition
 *
 **************************************************************/

#ifndef RADPGM_H
#define RADPGM_H

/*
 * radar/antenna program format
 */

/*
  AOP_POINT
    az = (s.Az == AANGLE_NOCHANGE) ? s.Az : current_az;
    el = (s.El == AANGLE_NOCHANGE) ? s.El : current_el;
    The antenna is commanded to az, el via shortest path
    Wait until az, el reached (or internal timeout)
	if s.Duration is time
		Wait until s.Duration elapses
	else
	    wait until s.Duration rays processed

  AOP_AZFULL
    el = (s.El == AANGLE_NOCHANGE) ? s.El : current_el;
	command antenna to el
	command antenna to freerun az in specified dir (AOPMOD)
	Wait until el reached (or internal timeout)
	if s.Duration is time
		Wait until s.Duration elapses
	else
	    wait until count/4 revolutions have completed

  AOP_AZSCAN
    el = (s.El == AANGLE_NOCHANGE) ? s.El : current_el;
	command antenna to el
	if (s.Az ==  AANGLE_NOCHANGE || s.EndPt == AANG_NOCHANGE)
		exit
	az0 = s.Az
	az1 = s.EndPt
	if (s."aopmod" != SPECDIR && az1 closer to current_az)
		swap(az0,az1)
	command antenna to az0
	Wait until az0,el reached (or internal timeout)
	command antenna to az1

	when az1 is reached, command antenna to az0 and vice versa
	if s.Duration is time
		repeat until s.Duration elapses
	else
		repeat s.Duration times

    Note: handle the LONGHPATH situation

  AOP_ELSCAN
    Much like AOP_AZSCAN

	
*/

#define MAX_RADPGM_STEPS 30

typedef struct
{
  ui16 NumSteps;
  ui08 Title[40]; /* asciz */
} RADPGM_HDR;


typedef struct
{
  ui08 AntOp; /* AOP_xxx and AOPMOD_xxx */
  ui08 Mode; /* PMODE_xxx and PDATA_xxx */
  ui16 Range; /* microseconds (or ARANGE_NOCHANGE) */
  ui16 Skip; /* microseconds X 10 (or ASKIP_NOCHANGE) */
  ui16 Duration; /* repeat count (if hi bit set)
		  * or seconds X 10 */
  ui16 Az; /* 16384 * deg/360 (or AANGLE_NOCHANGE) */
  ui16 El; /* 16384 * deg/360 (or AANGLE_NOCHANGE) */
  ui16 EndPt; /* second az or el param (deg X 10) */
  ui16 Res[4]; /* future use load with 0 */
} RADPGM_STEP;

typedef struct
{
  RADPGM_HDR Hdr;
  RADPGM_STEP Step[MAX_RADPGM_STEPS];
} RADPGM;

typedef struct
{
  RADPGM_HDR Hdr;
  RADPGM_STEP Step[1];
} RADPGM_ONESTEP;

#define AMODE_NOCHANGE 0xffff
#define ARANGE_NOCHANGE 0xffff
#define ASKIP_NOCHANGE 0xffff
#define AANGLE_NOCHANGE 0xffff
#define ASPEED_NOCHANGE 0xffff
#define ATIME_FOREVER 0
#define ATIME_COUNTFLAG 0x8000
#define ATIME_MASK 0x7fff

#define AOP_MASK 0x0f
#define AOP_POINT 0
#define AOP_AZFULL 1
#define AOP_AZSCAN 2
#define AOP_ELSCAN 3

#define AOPMOD_MASK 0xf0
#define AOPMOD_AZFULL_CW 0x10
#define AOPMOD_AZFULL_CCW 0x20
#define AOPMOD_AZSCAN_SPECDIR 0x10 /* do not optimize direction */
#define AOPMOD_AZSCAN_LONGPATH 0x20 /* take the long direction */
#define AOPMOD_ELSCAN_SPECDIR 0x10 /* do not optimize direction */


#define PMODE_MASK 0x0f
#define PMODE_STANDBY 0
#define PMODE_LOG 1
#define PMODE_SINGLE 2
#define PMODE_DUAL 3
#define PMODE_POWERDOWN 4
#define PMODE_NOCHANGE 0x0f

#define PDATA_MASK 0xf0
#define PDATA_NOCHANGE 0x00
#define PDATA_LOGREFL 0x10
#define PDATA_LINREFL 0x20
#define PDATA_VELOCITY 0x30
#define PDATA_TURBULENCE 0x40

#define AZFULL_COUNTSPERREV 4 /* 4 counts for full rev */

#endif /* RADPGM_H */

