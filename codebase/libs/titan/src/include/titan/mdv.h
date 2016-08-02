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
 * titan/mdv.h
 *
 * Nancy Rehak, RAP, NCAR, Boulder, CO, Feb 1997
 *
 **********************************************************************/

#ifndef titan_mdv_h
#define titan_mdv_h

#ifdef __cplusplus
extern "C" {
#endif

#include <Mdv/mdv/mdv_file.h>
#include <titan/radar.h>


extern void RfCreateMdvData(vol_file_handle_t *vhandle,
			    void **plane_array,
			    int field,
			    const char *calling_routine);

extern MDV_chunk_header_t
  *RfCreateMdvElevationsChunk(vol_file_handle_t *vhandle,
			      void **data,
			      const char *calling_routine);

extern MDV_field_header_t *RfCreateMdvFieldHdr(vol_file_handle_t *vhandle,
					       int field_num,
					       const char *calling_routine);

extern MDV_master_header_t *RfCreateMdvMasterHdr(vol_file_handle_t *vhandle,
						 const char *calling_routine);

extern MDV_vlevel_header_t *RfCreateMdvVlevelHdr(vol_file_handle_t *vhandle,
						 const char *calling_routine);

extern MDV_chunk_header_t
  *RfCreateMdvVolParamsChunk(vol_file_handle_t *vhandle,
			     void **data,
			     const char *calling_routine);

extern int RfWriteVolumeMdv(vol_file_handle_t *vhandle,
			    int mdv_encoding_type,
			    const char *calling_routine);

#ifdef __cplusplus
}
#endif

#endif
