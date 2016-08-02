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
#ifndef _TXMOND_H_
#define _TXMOND_H_

#include <paths.h>

#define PID_FILE _PATH_VARRUN "txmond.pid"

/* Status byte 1 */

#define UNIT_ON         0x01
#define COOLDOWN        0x02
#define WARMUP          0x04
#define STANDBY         0x08
#define HV_RUNUP        0x10
#define FAULT_SUM       0x20

/* Status byte 2 */

#define REV_POWER       0x01
#define SAFETY_INLCK    0x02
#define REMOTE_MODE     0x04
#define HVPS_ON         0x08
#define BLOWER          0x10
#define MAG_AV_CUR      0x20

/* Status byte 3 */

#define HVPS_OV         0x01
#define HVPS_UV         0x02
#define WG_PRESSURE     0x04
#define HVPS_OC         0x08
#define PIP             0x20

#define STX (0x02)
#define ETX (0x03)

/* Txmon commands */
enum {
	CMD_INVALID,
	CMD_POWERON,
	CMD_POWEROFF,
	CMD_STANDBY,
	CMD_OPERATE,
	CMD_RESET,
	CMD_STATUS
};

const char TXMON_CMD_FIFO[16] = "/tmp/txmon_cmd";
const char TXMON_INFO_FIFO[16] = "/tmp/txmon_info";

#endif
