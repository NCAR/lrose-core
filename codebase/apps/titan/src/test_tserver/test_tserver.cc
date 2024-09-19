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
/************************************************************************
 * test_tserver.s
 *
 * tests the track server access routines
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * May 1993
 *
 *************************************************************************/


#include <toolsa/os_config.h>
#include <toolsa/port.h>
#include <toolsa/pjg.h>
#include <toolsa/sockutil.h>
#include <titan/tdata_index.h>
#include <dataport/bigend.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <signal.h>

#define BOOL_STR(a) (a == 0? "false" : "true")

static int handle_tserver_sigpipe(int sig);

static void read_product(char *host,
			 int port);

static tdata_index_t Tdata_index;
  

int main()

{

  char *prog_name;
  char *servmap_host1;
  char *servmap_host2;
  char *server_subtype;
  char *server_instance;
  char *default_host;

  int request_notify;
  int runs_included;
  int mode;
  int source;
  int target_entries;
  int default_port;

  long track_set;
  long max_message_len;
  long request_time;
  long time_margin;
  long ientry;

  date_time_t dtime;

  tdata_basic_without_params_index_t *bindex;

  /*
   * set up server details
   */

  prog_name = (char *) "test_tserver";

  servmap_host1 = (char *) "kodiak";
  servmap_host2 = (char *) "none";

  server_subtype = (char *) "Dobson";

  server_instance = (char *) "Test";

  default_host = (char *) "kodiak";

  default_port = 44000;

  request_notify = FALSE;

  runs_included = FALSE;

  max_message_len = 100000;

  /*
   * initialize the server
   */

  tserver_init(prog_name,
	       servmap_host1,
	       servmap_host2,
	       server_subtype,
	       server_instance,
	       default_host,
	       default_port,
	       request_notify,
	       runs_included,
	       TRUE, TRUE,
	       max_message_len,
	       &Tdata_index);

  /*
   * set up sigpipe trap
   */
  
  PORTsignal(SIGPIPE, (PORTsigfunc) handle_tserver_sigpipe);

  /*
   * set up the request type - unused options are set first
   */

  mode = TDATA_BASIC_WITHOUT_PARAMS;
  mode = TDATA_COMPLETE;
  mode = TDATA_PRODUCT;
  mode = TDATA_BASIC_WITH_PARAMS;

  source = TDATA_REALTIME;
  source = TDATA_LATEST;
  source = TDATA_ARCHIVE;

  track_set = TDATA_ALL_IN_FILE;
  track_set = TDATA_ALL_AT_TIME;

  target_entries = TDATA_ENTRIES_IN_TIME_WINDOW;
  target_entries = TDATA_CURRENT_ENTRIES_ONLY;
  target_entries = TDATA_ALL_ENTRIES;

  dtime.year = 1990;
  dtime.month = 7;
  dtime.day = 10;
  dtime.hour = 03;
  dtime.min = 03;
  dtime.sec = 0;

  request_time = uunix_time(&dtime);
  time_margin = 200;

  if (mode == TDATA_PRODUCT) {

    read_product (default_host, default_port);

  } else {

    if (tserver_read(mode,
		     source,
		     track_set,
		     target_entries,
		     request_time,
		     time_margin,
		     (si32) 0, (si32) 0,
		     &Tdata_index)) {
    
      fprintf(stderr, "tserver read failed\n");
      exit (-1);

    } else {
      
      fprintf(stderr, "tserver read successful\n");
      
    }

    if (mode == TDATA_BASIC_WITHOUT_PARAMS) {
      
      bindex = &Tdata_index.basic;
      
      if (bindex->header.grid.proj_type == TITAN_PROJ_FLAT)
	printf("Grid type : flat\n");
      else
	printf("Grid type : latlon\n");
      
      printf("time : %s\n", utimstr(bindex->header.time));
      
      printf("n_entries : %ld\n", (long) bindex->header.n_entries);
      
      for (ientry = 0; ientry < bindex->header.n_entries; ientry++) {
	
	printf("entry , x, y : %ld, %g, %g\n",
	       ientry,
	       bindex->track_entry[ientry].refl_centroid_x,
	       bindex->track_entry[ientry].refl_centroid_y);

      } /* ientry */

    } /* if (mode == TDATA_BASIC_WITHOUT_PARAMS) */

  } /* if (mode == TDATA_PRODUCT) */

  tserver_clear(&Tdata_index);

  return (0);

}

/*************************************************************
 * handle_tserver_sigpipe()
 *
 * Routine to handle sigpipe from track server
 */

static int handle_tserver_sigpipe(int sig)

{

  tserver_handle_disconnect(&Tdata_index);

  return (sig);

}

/*************************************************************
 * read_product()
 *
 * Wait for product data, read in and print
 */

static void read_product(char *host,
			 int port)

{

  char *buf;
  char *mess_ptr;

  int sockfd;
  int forever = TRUE;
  int polygon_size;

  long ientry;
  long data_len;

  SKU_header_t mess_header;
  tdata_product_header_t *header;
  tdata_product_entry_t *entry;
  tdata_product_ellipse_t *ellipse;
  tdata_product_polygon_t *polygon;
  
  /*
   * connect to server
   */

  sockfd = SKU_open_client(host, port);

  if (sockfd < 0) {
    fprintf(stderr,
	    "read_product: could not open server, host %s, port %d\n",
	    host, port);
    perror ("");
    exit(-1);
  }

  while (forever) {

    if (SKU_read_message(sockfd,
			 &mess_header,
			 &buf,
			 &data_len,
			 -1L) < 0) {
      fprintf(stderr,
	      "read_product: could not read data\n");
      perror ("");
      exit(-1);
    }

    /*
     * set pointers
     */
    
    mess_ptr = buf;
    header = (tdata_product_header_t *) mess_ptr;
    mess_ptr += sizeof(tdata_product_header_t);
  
    BE_to_array_32((ui32 *) header,
		   (ui32) sizeof(tdata_product_header_t));
    
    polygon_size = sizeof(si32) + header->n_poly_sides * sizeof(ui08);

    fprintf(stdout, "########################\n");

    fprintf(stdout, "Product ID : %ld\n", (long) mess_header.id);

    fprintf(stdout, "Data time : %s\n", utimstr(header->time));
    
    if (header->grid_type == PJG_FLAT)
      fprintf(stdout, "Grid type: flat\n");
    else
      fprintf(stdout, "Grid type: latlon\n");
    
    fprintf(stdout, "Dbz threshold : %ld\n", (long) header->dbz_threshold);
    fprintf(stdout, "Min storm size : %ld\n",
	    (long) header->min_storm_size);
    
    fprintf(stdout, "forecast lead time : %ld\n",
	    (long) header->forecast_lead_time);

    fprintf(stdout, "n complex tracks : %ld\n",
	    (long) header->n_complex_tracks);
    fprintf(stdout, "n entries : %ld\n", (long) header->n_entries);


    /*
     * entries
     */

    entry = (tdata_product_entry_t *)
      (buf + sizeof(tdata_product_header_t));

    for (ientry = 0; ientry < header->n_entries; ientry++) {

      /*
       * set pointers into buffer, incrementing buffer 
       * pointer accordingly
       */
      
      entry = (tdata_product_entry_t *) mess_ptr;
      mess_ptr += sizeof(tdata_product_entry_t);
      
      BE_to_array_32((ui32 *) &entry->longitude,
		     (ui32) sizeof(si32));
      BE_to_array_32((ui32 *) &entry->latitude,
		     (ui32) sizeof(si32));
      BE_to_array_16((ui16 *) &entry->direction,
		     (ui16) sizeof(ui16));
      BE_to_array_16((ui16 *) &entry->speed,
		     (ui16) sizeof(ui16));
      
      ellipse = (tdata_product_ellipse_t *) NULL;
      if (header->ellipse_included) {
	ellipse = (tdata_product_ellipse_t *) mess_ptr;
	mess_ptr += sizeof(tdata_product_ellipse_t);
	BE_to_array_16((ui16 *) &ellipse->norm_darea_dt,
		       (ui16) sizeof(ui16));
	BE_to_array_16((ui16 *) &ellipse->orientation,
		       (ui16) sizeof(ui16));
	BE_to_array_16((ui16 *) &ellipse->minor_radius,
		       (ui16) sizeof(ui16));
	BE_to_array_16((ui16 *) &ellipse->major_radius,
		       (ui16) sizeof(ui16));
      }
      
      polygon = (tdata_product_polygon_t *) NULL;
      if (header->polygon_included) {
	polygon = (tdata_product_polygon_t *) mess_ptr;
	mess_ptr += polygon_size;
	BE_to_array_32((ui32 *) &polygon->radial_multiplier,
		  (ui32) sizeof(si32));
      }
      
      /*
       * print
       */

      fprintf(stdout, "***********\nentry num: %ld\n", ientry);

      fprintf(stdout, "longitude : %g\n",
	      (double) entry->longitude / (double) header->angle_scale);

      fprintf(stdout, "latitude : %g\n",
	      (double) entry->latitude / (double) header->angle_scale);
      
      fprintf(stdout, "speed : %g\n",
	      (double) entry->speed / (double) header->speed_scale);
      
      fprintf(stdout, "direction : %g\n",
	      (double) entry->direction /
	      (double) header->short_angle_scale);
      
      fprintf(stdout, "intensity_trend : %d\n",
	      (int) entry->intensity_trend);
      
      fprintf(stdout, "size_trend : %d\n",
	      (int) entry->size_trend);
      
      fprintf(stdout, "valid forecast? : %s\n",
	      BOOL_STR(entry->forecast_valid));

      entry++;

    } /* ientry */

    fflush (stdout);

  } /* while (forever) */

}
