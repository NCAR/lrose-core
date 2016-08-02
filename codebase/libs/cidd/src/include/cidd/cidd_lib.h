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
 * CIDD_LIB.H: Defines for routines for CIDD LIBS
 *
 */

#ifndef CIDD_LIB_H
#define CIDD_LIB_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <time.h>

extern void daemonize(void);
extern void latlong_to_xy(double center_lat, double center_long, double other_lat, double other_long, double *px, double *py);

extern void register_server(char   **dir,
			    int    num_dirs,
			    char   *suffix,
			    char   *instance,
			    char   *service_id,
			    char   *user_info,
			    int    port,
			    int    realtime_flag,
			    time_t last_request_time,
			    char   *host1,
			    char   *host2,
			    int    n_requests);
   
extern void to_netl();
extern void to_hostl();

extern void ieeei_vaxi(int *dp, int n);
extern void ieeef_vaxf(float *dp, int n);
extern void vaxf_ieeef(float *dp, int n);
extern void ieeed_vaxd(double *dp, int n);
extern void vaxd_ieeed(double *dp, int n);

extern unsigned char *RLDecode7(unsigned char *coded_data, 
                                unsigned int *nbytes_full);
extern unsigned char *RLDecode8(unsigned char *coded_data,
                                unsigned int *nbytes_full);
extern unsigned char *RLEncode7(unsigned char *full_data, 
                                unsigned int nbytes_full,
                                unsigned int *nbytes_array);
extern unsigned char *RLEncode8(unsigned char *full_data, 
                                unsigned int nbytes_full,
                                unsigned int key, 
                                unsigned int *nbytes_array);


#ifdef __cplusplus
}
#endif

#endif
