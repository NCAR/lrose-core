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
/******************************************************************
 * swap_tdwr.c
 *
 * Byte swapping routines for TDWR structs
 *
 * Gary Blackburn, RAP, NCAR, P.O.Box 3000, Boulder,
 *             CO, 80303, USA
 *
 * Oct 1997
 *
 */

#include "tdwr2moments.h"
#include <dataport/bigend.h>

/*---------------------------------------------------------------------------*/

void Beam_hdr_to_BE (Beam_hdr *hdr)           

{
                                            
	BE_from_array_16(&hdr->cpi_seq_num, 22);
	BE_from_array_16(&hdr->prf, 18);
	BE_from_array_32(&hdr->latitude, 8);                   
	BE_from_array_16(&hdr->altitude, 12);                                 
	BE_from_array_16(&hdr->scale, 26);
	BE_from_array_16(&hdr->start_gate_first_pkt, 4);
}

/*----------------------------------------------------------------------------*/

void be_to_TDWR_data_header (TDWR_data_header *hdr)
{
	BE_to_array_16(&hdr->message_id,2);
	BE_to_array_16(&hdr->message_length,2);
	BE_to_array_16(&hdr->volume_count,2);
	BE_to_array_16(&hdr->volume_flag,2);
		
	BE_to_array_16(&hdr->power_trans,2);
	BE_to_array_16(&hdr->playback_flag,2);

    BE_to_array_32(&hdr->scan_info_flag,4);

	BE_to_array_32(&hdr->current_elevation,4);
	BE_to_array_32(&hdr->angular_scan_rate,4);

	BE_to_array_16(&hdr->pri,2);
	BE_to_array_16(&hdr->dwell_flag,2);
	BE_to_array_16(&hdr->final_range_sample,2);
	BE_to_array_16(&hdr->rng_samples_per_dwell,2);	

	BE_to_array_32(&hdr->azimuth,4);
	BE_to_array_32(&hdr->total_noise_power,4); 
	BE_to_array_32(&hdr->timestamp,4);
	BE_to_array_16(&hdr->base_data_type,2);
	BE_to_array_16(&hdr->vol_elev_status_flag,2);

	BE_to_array_16(&hdr->interger_azimuth,2);
	BE_to_array_16(&hdr->empty_short,2);

}

/*----------------------------------------------------------------------------*/

void be_from_TDWR_data_header (TDWR_data_header *hdr)
{
	BE_from_array_16(&hdr->message_id,2);
	BE_from_array_16(&hdr->message_length,2);
	BE_from_array_16(&hdr->volume_count,2);
	BE_from_array_16(&hdr->volume_flag,2);
		
	BE_from_array_16(&hdr->power_trans,2);
	BE_from_array_16(&hdr->playback_flag,2);

    BE_from_array_32(&hdr->scan_info_flag,4);

	BE_from_array_32(&hdr->current_elevation,4);

	BE_from_array_32(&hdr->angular_scan_rate,2);
	BE_from_array_16(&hdr->pri,2);
	BE_from_array_16(&hdr->dwell_flag,2);
	BE_from_array_16(&hdr->final_range_sample,2);
	BE_from_array_16(&hdr->rng_samples_per_dwell,2);	

	BE_from_array_32(&hdr->azimuth,4);
	BE_from_array_32(&hdr->total_noise_power,4); 
	BE_from_array_32(&hdr->timestamp,4);
	BE_from_array_16(&hdr->base_data_type,2);
	BE_from_array_16(&hdr->vol_elev_status_flag,2);

	BE_from_array_16(&hdr->interger_azimuth,2);
	BE_from_array_16(&hdr->empty_short,2);
}

/*----------------------------------------------------------------------------*/

