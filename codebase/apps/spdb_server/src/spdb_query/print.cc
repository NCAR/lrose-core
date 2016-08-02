// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/******************************************************************************
 * PRINT.C - print routines
 */

#include "spdb_query.h"

#include <Spdb/FltRoute.hh>
#include <Spdb/PosnRpt.hh>
#include <Spdb/sounding.h>
#include <Spdb/WxHazardBuffer.hh>

#include <rapformats/hist_fore.h>
#include <rapformats/ac_data.h>
#include <rapformats/ac_posn.h>
#include <rapformats/bdry.h>
#include <rapformats/flt_path.h>
#include <rapformats/fos.h>
#include <rapformats/GenPt.hh>
#include <rapformats/GenPoly.hh>
#include <rapformats/HydroStation.hh>
#include <rapformats/ltg.h>
#include <rapformats/pirep.h>
#include <rapformats/station_reports.h>
#include <rapformats/trec_gauge.h>
#include <rapformats/zr.h>
#include <rapformats/zrpf.h>
#include <toolsa/udatetime.h>
#include <rapformats/tstorm_spdb.h>
#include <toolsa/membuf.h>
#include <toolsa/port.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>


/*****************************************************************
 * PRINT_CHUNK_HDR : Print a chunk header to the indicated stream.
 */

void print_chunk_hdr(FILE *stream, spdb_chunk_ref_t *hdr)
{
  fprintf(stream, "data_type = %d\n", hdr->data_type);
  
  fprintf(stream, "valid_time = %s\n", utimstr(hdr->valid_time));
  fprintf(stream, "expire_time = %s\n", utimstr(hdr->expire_time));
  
  fprintf(stream, "offset = %d\n", hdr->offset);
  
  fprintf(stream, "len = %d\n", hdr->len);

  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_ASCII_DATA: Print ascii string
 */

void print_ascii_data(FILE *stream,
		      spdb_chunk_ref_t *hdr,
		      void *data)
{

  /*
   * make sure we have a null-terminated string
   */
  
  char *str = (char *)data;
  int len = hdr->len;
  str[len-1] = '\0';

  /*
   * print it
   */

  fprintf(stream, "%s", str);
  fprintf(stream, "\n");
  
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_STN_REPORT : Print the station reports received from
 *                         the server.
 */

void print_stn_report(FILE *stream, void *data)
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
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_PIREP_DATA : Print the aircraft position data received
 *                      from the server.
 */

void print_pirep_data(FILE *stream,
		      spdb_chunk_ref_t *hdr,
		      void *data)
{
  pirep_t *pirep_data = (pirep_t *)data;
  int num = hdr->len / sizeof(pirep_t);
  int posn;
  
  /*
   * Check for errors
   */

  if (hdr->len % sizeof(pirep_t) != 0)
    fprintf(stream,
	    "  ERROR:  Chunk contains %d bytes, not an even "
	    "multiple of structure size (%d bytes)\n",
	    hdr->len, (int)sizeof(pirep_t));
  
  for (posn = 0; posn < num; posn++)
  {
    BE_to_pirep(&pirep_data[posn]);
    pirep_print(stream, "  ", &pirep_data[posn]);
  } /* endfor - posn */
  
  fprintf(stream, "\n");
  fflush(stream);
  
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
	    "  ERROR:  Chunk contains %d bytes, not an even "
	    "multiple of structure size (%d bytes)\n",
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
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_AC_POSN_WMOD_DATA : Print the wmod aircraft position data
 */

void print_ac_posn_wmod_data(FILE *stream,
			     spdb_chunk_ref_t *hdr,
			     void *data)
{
  ac_posn_wmod_t *ac_posn_data = (ac_posn_wmod_t *)data;
  int num_posn = hdr->len / sizeof(ac_posn_wmod_t);
  int posn;
  
  /*
   * Print the aircraft data.
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for aircraft position (%d positions):\n", num_posn);

  /*
   * Check for errors
   */

  if (hdr->len % sizeof(ac_posn_wmod_t) != 0)
    fprintf(stream,
	    "  ERROR:  Chunk contains %d bytes, not an even "
	    "multiple of structure size (%d bytes)\n",
	    hdr->len, (int)sizeof(ac_posn_wmod_t));
  
  for (posn = 0; posn < num_posn; posn++)
  {
    /*
     * Swap the aircraft data.
     */

    BE_to_ac_posn_wmod(&ac_posn_data[posn]);
    ac_posn_wmod_print(stream, "  ", &ac_posn_data[posn]);
    
  } /* endfor - posn */
  
  fprintf(stream, "\n");
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_AC_DATA_DATA : Print the aircraft position data received
 *                      from the server.
 */

void print_ac_data_data(FILE *stream,
			spdb_chunk_ref_t *hdr,
			void *data)
{
  ac_data_t *ac_data = (ac_data_t *)data;
  
  /*
   * Print the aircraft data.
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for aircraft:\n");

  /*
   * Swap the aircraft data.
   */

  ac_data_from_BE(ac_data);
  
  ac_data_print(stream, "   ", ac_data);
  
  fprintf(stream, "\n");
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_BDRY_DATA : Print the boundary data received from the server.
 */

void print_bdry_data(FILE *stream, void *data)
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

  BDRY_print_spdb_product(stream, bdry_data, FALSE);
    
  fprintf(stream, "\n");
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_FLT_PATH_DATA : Print the flight path data received from
 *                       the server.
 */

void print_flt_path_data(FILE *stream, void *data)
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
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_HISTORY_FORECAST_DATA : Print the history forecast data
 *                               received from the server.
 */

void print_history_forecast_data(FILE *stream,
				 spdb_chunk_ref_t *hdr,
				 void *data)
{   
  int ier;

  fprintf(stream, " Printing history forecast data:\n");
  ier = hf_chunk_native((hf_chunk_t **) &data);
  print_hf_chunk(stream, (hf_chunk_t *) data, hdr->len);

  fflush(stream);
}

/*****************************************************************
 * PRINT_LTG_DATA : Print the lightning data received from the server.
 */

void print_ltg_data(FILE *stream,
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
  
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_SYMPROD_DATA : Print the symbolic product data received
 *                      from the server.
 */

void print_symprod_data(FILE *stream, void *data)
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
    
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_SIGMET_DATA : Print the SIGMET data received from the
 *                     server.
 */

void print_sigmet_data(FILE *stream, void *data)
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
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_TSTORMS_DATA : Print the TSTORMS data received from the
 *                     server.
 */

void print_tstorms_data(FILE *stream, void *data)
{

  /*
   * swap the buffer data
   */

  tstorm_spdb_buffer_from_BE((ui08 *) data);

  /*
   * print the buffer
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for TSTORMS:\n");

  tstorm_spdb_print_buffer(stream, "  ", (ui08 *) data);
  
  fprintf(stream, "\n");
  fflush(stream);
  
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

  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_ZR_PARAMS_DATA : Print the ZR params data received from the
 *                        server.
 */

void print_zr_params_data(FILE *stream, void *data)
{

  zr_params_t *zrp = (zr_params_t *) data;

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
  fflush(stream);
  
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

  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_SNDG_DATA : Print the sounding data received from the server.
 */

void print_sndg_data(FILE *stream, void *data)
{
  SNDG_spdb_product_t *sndg_data = (SNDG_spdb_product_t *)data;
  
  /*
   * Swap the boundary data.
   */

  SNDG_spdb_product_from_BE(sndg_data);
  
  /*
   * Print the boundary data.
   */

  fprintf(stream, "\n");
  fprintf(stream, "Data for sounding:\n");

  SNDG_print_spdb_product(stream, sndg_data, FALSE);
    
  fprintf(stream, "\n");
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_WX_HAZARD_DATA : Print the weather hazard data received
 *                        from the server.
 */

void print_wx_hazard_data(FILE *stream,
			  spdb_chunk_ref_t *hdr,
			  void *data)
{
  Spdb::chunk_ref_t spdb_hdr;
  
  spdb_hdr.valid_time = hdr->valid_time;
  spdb_hdr.expire_time = hdr->expire_time;
  spdb_hdr.data_type = hdr->data_type;
  spdb_hdr.data_type2 = hdr->prod_id;
  spdb_hdr.offset = hdr->offset;
  spdb_hdr.len = hdr->len;
  
  WxHazardBuffer *hazards = new WxHazardBuffer(&spdb_hdr, data);
  hazards->print(stream);

  delete hazards;

  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_FLT_ROUTE_DATA : Print the flight route data received
 *                        from the server.
 */

void print_flt_route_data(FILE *stream,
			  spdb_chunk_ref_t *hdr,
			  void *data)
{
  FltRoute *route = new FltRoute(data, true);
  route->print(stream);

  delete route;

  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_GENPT_DATA : Print the generic point data received
 *                    from the server.
 */

void print_GenPt_data(FILE *stream,
		      spdb_chunk_ref_t *hdr,
		      void *data)
{
  GenPt point;
  
  point.disassemble(data, hdr->len);
  
  point.print(stream);
  
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_GENPOLY_DATA : Print the generic polyline data received
 *                      from the server.
 */

void print_GenPoly_data(FILE *stream,
			spdb_chunk_ref_t *hdr,
			void *data)
{
  GenPoly polygon;
  
  polygon.disassemble(data, hdr->len);
  
  polygon.print(stream);
  
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_HYDROSTATION_DATA : Print the hydrology station data received
 *                           from the server.
 */

void print_HydroStation_data(FILE *stream,
			     spdb_chunk_ref_t *hdr,
			     void *data)
{
  HydroStation station;
  
  station.disassemble(data, hdr->len);
  
  station.print(stream);
  
  fflush(stream);
  
  return;
}

/*****************************************************************
 * PRINT_POSN_RPT_DATA : Print the aircraft position report data
 *                       received from the server.
 */

void print_posn_rpt_data(FILE *stream,
			 spdb_chunk_ref_t *hdr,
			 void *data)
{
  PosnRpt *posn_rpt = new PosnRpt(data, true);
  posn_rpt->print(stream);

  delete posn_rpt;

  fflush(stream);
  
  return;
}
