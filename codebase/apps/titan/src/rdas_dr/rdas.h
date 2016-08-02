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
/*
 * rdas.h
 *
 * header file for RDAS driver
 */

#ifndef _RDAS_INCLUDED_
#define _RDAS_INCLUDED_

/*
 * typedefs for sized ints and floats
 */

typedef unsigned int ui32;
typedef unsigned short ui16;
typedef short si16;

#define _RDAS_DEFAULT_BASE_ADDR 0x300

/*
 * IOCTL commmand values
 */

#define RDAS_IOC_MAGIC 'R'

/* set the port base address */
#define _RDAS_SET_BASE_ADDR    _IO(RDAS_IOC_MAGIC,  1)

/* set the size of the program in 16-bit words */
#define _RDAS_SET_PROGRAM_SIZE  _IO(RDAS_IOC_MAGIC,  2)

/* set the irq number */
#define _RDAS_SET_IRQ_NUM       _IO(RDAS_IOC_MAGIC,  3)

/* reset the DSP */
#define _RDAS_RESET_DSP         _IO(RDAS_IOC_MAGIC,  4)

/* load the program into the DSP */
#define _RDAS_LOAD_PROGRAM      _IOW(RDAS_IOC_MAGIC,  5, int)

/* set addressing to program memory */
#define _RDAS_SET_ADDR_PROG     _IO(RDAS_IOC_MAGIC,  6)

/* set addressing to data memory */
#define _RDAS_SET_ADDR_DATA     _IO(RDAS_IOC_MAGIC,  7)

/* check status port for dav */
#define _RDAS_CHECK_DAV         _IO(RDAS_IOC_MAGIC,  8)

/* clear DAV flag */
#define _RDAS_CLEAR_DAV         _IO(RDAS_IOC_MAGIC,  9)

#endif

