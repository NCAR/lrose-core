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

/******************************************************************************
 * HTTPgetURL: Return the resource data specified by the input url.
 * Returns -1 bad url, -2 failure to connect,  -3 read reply failure,
 * -4 alloc failure, -5 read resource data failure. Values > 0
 * are the transaction status as returned by the http server. This
 * routine allocates space for the resource data. It is the caller's
 * responsibility for freeing this memory.  On success: sets **resource
 * to point to the allocated data and *resource_len * to its length.
 *
  * F. Hage Feb 1999.
 */

int
HTTPgetURL(const char *url, size_t timeout_msec,       /* INPUT */
           char **resource, int * resource_len); /* RETURN */
int
HTTPgetURL_via_proxy(const char *proxy_url, const char *url, size_t timeout_msec,   /* INPUT */
           char **resource, int * resource_len); /* RETURN */


#ifdef __cplusplus
}
#endif
