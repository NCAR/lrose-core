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

/*
 * rdacs2gate.h
 *
 * Header file for rdacs2gate
 */

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
#include <titan/file_io.h>
#include <titan/radar.h>
#include <dataport/bigend.h>
#include <rapformats/gate_data.h>
#include <tdrp/tdrp.h>
#include <toolsa/sockutil.h>

#include "rdacsp.h"
#include "radpgm.h"

#include "rdacs2gate_tdrp.h"

#define DBZ_SCALE (0.5)
#define DBZ_BIAS (-30.0)
#define DBZ_FACTOR (10000)

/*
 * global struct
 */

typedef struct {

  char *prog_name;                /* program name */
  TDRPtable *table;               /* TDRP parsing table */
  rdacs2gate_tdrp_struct params;  /* parameter struct */

} global_t;

/*
 * declare the global structure locally in the main,
 * and as an extern in all other routines
 */

#ifdef MAIN
global_t *Glob = NULL;
#else
extern global_t *Glob;
#endif

/*
 * function prototypes
 */

extern int apply_params(void);

extern void handle_msg(RDP_HDR *hdr, RDP_ANY_R *msg);

extern void handle_step(RADPGM_STEP *s);

extern void init_param_buffer(void);

extern void load_beam(RDP_HDR *reply_hdr, RDP_GetRayData_R *rayd);

extern void parse_args(int argc,
		       char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       tdrp_override_t *override,
		       char **params_file_path_p);

extern int read_radar(void);

extern void send_buffer(void *outbuf, int outlen, int packet_code);

extern void send_params(void);

extern void store_ray_params(RDP_GetRayParms_R *rparams);

extern void store_site_params(SITECFG *site);

extern void SwapInit(void);
extern void SwapRDP_HDR(RDP_HDR *hdr);
extern void SwapRDP_Login_R(RDP_Login_R *r);
extern void SwapSITECFG(SITECFG *cfg);
extern void SwapRDP_SetPgm(RDP_SetPgm *p);
extern void SwapRDP_LoadCfg(RDP_LoadCfg *p);
extern void SwapRDP_SaveCfg(RDP_SaveCfg *p);
extern void SwapRDP_LoadMask(RDP_LoadMask *p);
extern void SwapRDP_SaveMask(RDP_SaveMask *p);
extern void SwapRDP_GetCurPgm_R(RDP_GetCurPgm_R *p);
extern void SwapRDP_GetMainPgm_R(RDP_GetMainPgm_R *p);
extern void SwapRDP_GetCurStep_R(RDP_GetCurStep_R *p);
extern void SwapRDP_GetRayParms(RDP_GetRayParms *p);
extern void SwapRDP_GetRayParms_R(RDP_GetRayParms_R *p);
extern void SwapRDP_GetRayData_R(RDP_GetRayData_R *p);
extern void SwapRDP_SelectAsync(RDP_SelectAsync *p);
extern void SwapRDP_Overflow_R(RDP_Overflow_R *p);
extern void SwapRDP_GetRawData_R(RDP_GetRawData_R *p);
extern void SwapRDP_ClearMask(RDP_ClearMask *p);
extern void SwapRADPGM_STEP(RADPGM_STEP *p);

extern void tidy_and_exit(int sig);

#ifdef __cplusplus
}
#endif
