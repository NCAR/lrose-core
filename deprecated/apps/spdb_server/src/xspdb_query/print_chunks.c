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
#include "xspdb_client.h"

/*****************************************************************
 * PRINT_CHUNK_HDR : Print a chunk header to the indicated stream.
 */

void print_chunk_hdr(FILE *stream, spdb_chunk_ref_t *hdr)
{
  fprintf(stream, "Product ID = %d\n", hdr->prod_id);
  fprintf(stream, "data_type = %d\n", hdr->data_type);
  
  fprintf(stream, "valid_time = %s\n", utimstr(hdr->valid_time));
  fprintf(stream, "expire_time = %s\n", utimstr(hdr->expire_time));
  
  fprintf(stream, "offset = %d\n", hdr->offset);
  
  fprintf(stream, "len = %d\n", hdr->len);
  
  return;
}

/*****************************************************************
 * PRINT_STN_REPORT : Print the station reports received from
 *                         the server.
 */

void print_stn_report(FILE *stream,
			     spdb_chunk_ref_t *hdr,
			     void *data)
{
  station_report_t *report = (station_report_t *)data;
  
  /*
   * Swap the report data
   */

  station_report_from_be(report);

  /*
   * Now print the fields.
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for station report:\n");

  print_station_report(stream, "   ", report);
    
  fprintf(stream, "\n");
  
  return;
}

/*****************************************************************
 * PRINT_AC_POSN_DATA : Print the aircraft position data received
 *                      from the server.
 */

void print_ac_posn_data(FILE *stream,
			       spdb_chunk_ref_t *hdr,
			       void *data)
{
  ac_posn_t *ac_posn_data = (ac_posn_t *)data;
  int num_posn = hdr->len / sizeof(ac_posn_t);
  int posn;
  
  /*
   * Print the aircraft data.
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for aircraft position (%d positions):\n", num_posn);

  /*
   * Check for errors
   */

  if (hdr->len % sizeof(ac_posn_t) != 0)
    fprintf(stream,
	    "  ERROR:  Chunk contains %d bytes, not an even multiple of structure size (%d bytes)\n",
	    hdr->len, (int)sizeof(ac_posn_t));
  
  for (posn = 0; posn < num_posn; posn++)
  {
    /*
     * Swap the aircraft data.
     */

    BE_to_ac_posn(&ac_posn_data[posn]);
  
    fprintf(stream, "  position %d\n", posn);
    fprintf(stream, "   lat = %f\n", ac_posn_data[posn].lat);
    fprintf(stream, "   lon = %f\n", ac_posn_data[posn].lon);
    fprintf(stream, "   alt = %f\n", ac_posn_data[posn].alt);
    fprintf(stream, "   callsign = <%s>\n", ac_posn_data[posn].callsign);
    fprintf(stream, "\n");
    
  } /* endfor - posn */
  
  fprintf(stream, "\n");

  return;
}

/*****************************************************************
 * PRINT_BDRY_DATA : Print the boundary data received from the server.
 */

void print_bdry_data(FILE *stream,
			    spdb_chunk_ref_t *hdr,
			    void *data)
{
  BDRY_spdb_product_t *bdry_data = (BDRY_spdb_product_t *)data;
  
  /*
   * Swap the boundary data.
   */

  BDRY_spdb_product_from_BE(bdry_data);
  
  /*
   * Print the boundary data.
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for boundary:\n");

  BDRY_print_spdb_product(stream, bdry_data, TRUE);
    
  fprintf(stream, "\n");

  return;
}

/*****************************************************************
 * PRINT_FLT_PATH_DATA : Print the flight path data received from
 *                       the server.
 */

void print_flt_path_data(FILE *stream,
				spdb_chunk_ref_t *hdr,
				void *data)
{
  FLTPATH_path_t *path_data = (FLTPATH_path_t *)data;
  
  /*
   * Swap the flight path data.
   */

  FLTPATH_path_from_BE(path_data);
  
  /*
   * Print the flight path data.
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for flight path:\n");

  FLTPATH_print_path(stream, path_data);
    
  fprintf(stream, "\n");

  return;
}

/*****************************************************************
 * PRINT_KAV_LTG_DATA : Print the Kavouras lightning data received
 *                      from the server.
 */

void print_kav_ltg_data(FILE *stream,
			       spdb_chunk_ref_t *hdr,
			       void *data)
{
  LTG_strike_t *ltg_data = (LTG_strike_t *)data;
  int strike;
  int num_strikes = hdr->len / sizeof(LTG_strike_t);
  
  /*
   * Process each strike received.
   */

  for (strike = 0; strike < num_strikes; strike++)
  {
    /*
     * Swap the lightning data.
     */

    LTG_from_BE(&(ltg_data[strike]));
  
    /*
     * Print the lightning data.
     */

    fprintf(stream, "\n");
    fprintf(stream, "Data for strike %d:\n", strike);

    LTG_print_strike(stream, &(ltg_data[strike]));
    
    fprintf(stream, "\n");
  } /* endfor - strike */
  
  return;
}

/*****************************************************************
 * PRINT_SYMPROD_DATA : Print the symbolic product data received
 *                      from the server.
 */

void print_symprod_data(FILE *stream,
			       spdb_chunk_ref_t *hdr,
			       void *data)
{
  char *prod_data = (char *)data;
  symprod_product_t *product;
  
  /*
   * Swap the SYMPROD data
   */

  SYMPRODproductBufferFromBE(prod_data);

  /*
   * Convert the data to internal product format.
   */

  product = SYMPRODbufferToProduct(prod_data);
    
  /*
   * Now print the product.
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for product:\n");

  SYMPRODprintProduct(stream, product);
    
  fprintf(stream, "\n");

  /*
   * Free the product memory.
   */

  SYMPRODfreeProduct(product);
    
  return;
}

/*****************************************************************
 * PRINT_SIGMET_DATA : Print the SIGMET data received from the
 *                     server.
 */

void print_sigmet_data(FILE *stream,
			      spdb_chunk_ref_t *hdr,
			      void *data)
{
  SIGMET_spdb_t *sigmet_data = (SIGMET_spdb_t *)data;
  
  /*
   * Swap the SIGMET data
   */

  SIGMET_spdb_from_BE(sigmet_data);

  /*
   * Print the SIGMET information.
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for SIGMET:\n");

  SIGMET_print_spdb(stream, sigmet_data);
    
  fprintf(stream, "\n");

  return;
}

/*****************************************************************
 * PRINT_TSTORMS_DATA : Print the TSTORMS data received from the
 *                     server.
 */

void print_tstorms_data(FILE *stream,
			       spdb_chunk_ref_t *hdr,
			       void *data)
{

  /*
   * swap the buffer data
   */

  tstorm_spdb_buffer_from_BE(data);

  /*
   * print the buffer
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for TSTORMS:\n");

  tstorm_spdb_print_buffer(stream, "  ", data);
  
  fprintf(stream, "\n");

  return;
}

/*****************************************************************
 * PRINT_TREC_GAUGE_DATA : Print the trec_gauge data received from the
 *                        server.
 */

void print_trec_gauge_data(FILE *stream,
				  spdb_chunk_ref_t *hdr,
				  void *data)
{

  trec_gauge_handle_t tgauge;

  /*
   * initialize handle
   */

  trec_gauge_init(&tgauge);

  /*
   * load data from chunk, swap from BE
   */
  
  trec_gauge_load_from_chunk(&tgauge, data, hdr->len);

  /*
   * print the buffer
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for TREC GAUGE:\n");
  trec_gauge_print(stream, "  ", &tgauge);
  fprintf(stream, "\n");

  /*
   * free up
   */

  trec_gauge_free(&tgauge);

  return;
}

/*****************************************************************
 * PRINT_ZR_PARAMS_DATA : Print the ZR params data received from the
 *                        server.
 */

void print_zr_params_data(FILE *stream,
				 spdb_chunk_ref_t *hdr,
				 void *data)
{

  zr_params_t *zrp = data;

  /*
   * swap
   */
  
  zr_params_from_BE(zrp);

  /*
   * print the buffer
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for ZR PARAMS:\n");
  zr_params_print(stream, "  ", zrp);
  fprintf(stream, "\n");

  return;
}

/*****************************************************************
 * PRINT_ZRPF_DATA : Print the zr point forecast data received from the
 *                   server.
 */

void print_zrpf_data(FILE *stream,
			    spdb_chunk_ref_t *hdr,
			    void *data)
{

  zrpf_handle_t handle;

  /*
   * initialize handle
   */

  zrpf_handle_init(&handle);

  /*
   * load data from chunk, swap from BE
   */
  
  zrpf_handle_load_from_chunk(&handle, data, hdr->len);

  /*
   * print the buffer
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for ZR POINT FORECAST DATA:\n");
  zrpf_handle_print(stream, "  ", &handle);
  fprintf(stream, "\n");

  /*
   * free up
   */

  zrpf_handle_free(&handle);

  return;
}


/*****************************************************************
 * PRINT_SOUNDING_DATA : 
 */

void print_sounding_data(FILE *stream,
			    spdb_chunk_ref_t *hdr,
			    void *data)
{

  SNDG_spdb_product_t *sounding = data;

  /*
   * swap from BE
   */
  
  SNDG_spdb_product_from_BE(sounding);

  /*
   * print 
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for Sounding :\n");
  SNDG_print_spdb_product(stream, sounding, sounding->nPoints);
  fprintf(stream, "\n");

  return;
}


/*****************************************************************
 * PRINT_ASCII_DATA : 
 */

void print_ascii_data(FILE *stream,
			    spdb_chunk_ref_t *hdr,
			    void *data)
{

  /*
   * print 
   */

  fprintf(stream, "\n");
  fprintf(stream, "ASCII Data:\n");
  fputs((char *)data,stream);
  fprintf(stream, "\n\n");

  return;
}

/*****************************************************************
 * PRINT_PIREP_DATA : 
 */

void print_pirep_data(FILE *stream,
			    spdb_chunk_ref_t *hdr,
			    void *data)
{

  pirep_t *pirep = data;

  /*
   * swap from BE
   */
  
  BE_to_pirep(pirep);

  /*
   * print 
   */

  fprintf(stream, "\n");
  fprintf(stream, "Pirep :\n");
  pirep_print(stream, "\t",pirep);
  fprintf(stream, "\n");

  return;
}
