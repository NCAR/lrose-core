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
/*********************************************************************
 * read_params.c
 *
 * Reads the parameters from the params data base into the
 * globals struct
 *
 * RAP, NCAR, Boulder CO
 *
 * December 1991
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "cart_slave.h"

void read_params(void)

{

  Glob->shmem_key = (key_t) uGetParamLong(Glob->prog_name,
					  "shmem_key", SHMEM_KEY);

  Glob->max_packet_length = uGetParamLong(Glob->prog_name,
					  "max_packet_length",
					  MAX_PACKET_LENGTH);

  Glob->slave_table_path = uGetParamString(Glob->prog_name,
					   "slave_table",
					   SLAVE_TABLE);

  Glob->servmap_host1 = uGetParamString(Glob->prog_name,
					"servmap_host1",
					SERVMAP_HOST1_STR);

  Glob->servmap_host2 = uGetParamString(Glob->prog_name,
					"servmap_host2",
					SERVMAP_HOST2_STR);

  Glob->cart_data_server_instance =
    uGetParamString(Glob->prog_name,
		    "cart_data_server_instance",
		    CART_DATA_SERVER_INSTANCE);

  Glob->cart_data_server_default_host =
    uGetParamString(Glob->prog_name,
		    "cart_data_server_default_host",
		    CART_DATA_SERVER_DEFAULT_HOST);
  
  Glob->cart_data_server_default_port =
    (int) uGetParamLong(Glob->prog_name,
			"cart_data_server_default_port",
			CART_DATA_SERVER_DEFAULT_PORT);

}
