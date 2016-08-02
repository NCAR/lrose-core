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
 *  SPDB_QUERY.CC Query program for the spdb data servers 
 */

#include "spdb_query.h"

#include <rapformats/ac_data.h>
#include <toolsa/str.h>
#include <toolsa/port.h>
#include <csignal>

#define MAX_INPUT_LEN     1024
#define SERVICE_NAME_LEN    64

/*
 * Local typedefs
 */

typedef enum
{
  GET,
  GET_CLOSEST,
  GET_INTERVAL,
  GET_VALID,
  GET_FIRST_BEFORE,
  GET_FIRST_AFTER,
  GET_CMD_ERROR
} get_request_t;


/************** STATIC FUNCTION PROTOTYPES **************/

static void signal_trap(int sig);

static char get_main_command(void);

static get_request_t get_get_request_type(void);

static void send_get_command(int unique_flag);

static void send_put_command(void);

static int request_data_get(char *source,
			    ui32 *nchunks,
			    spdb_chunk_ref_t **chunk_hdrs,
			    void ** chunk_data);

static int request_data_get_closest(char *source,
				    ui32 *nchunks,
				    spdb_chunk_ref_t **chunk_hdrs,
				    void ** chunk_data);

static int request_data_get_interval(char *source,
				     ui32 *nchunks,
				     spdb_chunk_ref_t **chunk_hdrs,
				     void ** chunk_data);

static int request_data_get_valid(char *source,
				  ui32 *nchunks,
				  spdb_chunk_ref_t **chunk_hdrs,
				  void ** chunk_data);

static int request_data_get_first_before(char *source,
					 ui32 *nchunks,
					 spdb_chunk_ref_t **chunk_hdrs,
					 void ** chunk_data);

static int request_data_get_first_after(char *source,
					ui32 *nchunks,
					spdb_chunk_ref_t **chunk_hdrs,
					void ** chunk_data);

static int get_data_type(void);

/************** GLOBAL DATA *****************************/

char	*SourceString;
int     ProductId;
char   *ProductLabel = (char *) "TEST PRODUCT DATA";

/******************************************************************************
 * MAIN
 *
 */

int main(int argc, char *argv[])
{
  int done = FALSE;
  
  if (argc < 3) {
    fprintf(stderr,"Standard Usage: spdb_query source_string product_id \n"); 
    exit(0);
  }

  SourceString  = argv[1];
  ProductId  = atoi(argv[2]);

  PORTsignal(SIGINT, signal_trap);
  PORTsignal(SIGTERM, signal_trap);

  while (!done)
  {
    switch(get_main_command())
    {
    case 'g' :
      send_get_command(FALSE);
      break;
      
    case 'u' :
      send_get_command(TRUE);
      break;
      
    case 'p' :
      send_put_command();
      break;
      
    case 'q' :
      done = TRUE;
      break;
      
    default:
      fprintf(stdout, "\n");
      fprintf(stdout, "Invalid command -- try again\n");
      fprintf(stdout, "\n");
      fflush(stdout);
      break;
    } /* endswitch - get_main_command */
    
  } /* endwhile - !done */

  exit(0);
}

/*****************************************************************
 * SIGNAL_TRAP : Traps Signals so as to die gracefully
 */

static void signal_trap(int sig)
{
  exit(sig);
}
 
/*****************************************************************
 * GET_MAIN_COMMAND : Get the desired main command from the user.
 */

static char get_main_command(void)
{
  char command_string[MAX_INPUT_LEN];
  
  fprintf(stdout, "Choose a command:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "   G = Get data\n");
  fprintf(stdout, "   U = get Unique data\n");
  fprintf(stdout, "   P = Put data\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "   Q = Quit\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Desired command: ");
  fflush(stdout);
  
  fgets(command_string, MAX_INPUT_LEN, stdin);
  
  return(tolower(command_string[0]));
}
 
/*****************************************************************
 * GET_GET_REQUEST_TYPE : Get the type of get request from the user.
 */

static get_request_t get_get_request_type(void)
{
  char command_string[MAX_INPUT_LEN];
  
  fprintf(stdout, "Choose a get type:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "   G = Regular get command\n");
  fprintf(stdout, "   C = Get closest\n");
  fprintf(stdout, "   I = Get interval\n");
  fprintf(stdout, "   V = Get valid\n");
  fprintf(stdout, "   B = Get first before\n");
  fprintf(stdout, "   A = Get first after\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Desired command: ");
  fflush(stdout);
  
  fgets(command_string, MAX_INPUT_LEN, stdin);
  
  switch(command_string[0])
  {
  case 'G' :
  case 'g' :
    return(GET);
    
  case 'C' :
  case 'c' :
    return(GET_CLOSEST);
    
  case 'I' :
  case 'i' :
    return(GET_INTERVAL);
    
  case 'V' :
  case 'v' :
    return(GET_VALID);
    
  case 'B' :
  case 'b' :
    return(GET_FIRST_BEFORE);
    
  case 'A' :
  case 'a' :
    return(GET_FIRST_AFTER);
    
  default:
    return(GET_CMD_ERROR);
  }
}
 
/*****************************************************************
 * GET_PUT_REQUEST_TYPE : Get the type of put request from the user.
 */

static spdb_request_t get_put_request_type(void)
{
  char command_string[MAX_INPUT_LEN];
  
  fprintf(stdout, "Choose a put type:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "   P = Regular put command\n");
  fprintf(stdout, "   A = Put add\n");
  fprintf(stdout, "   O = Put over\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Desired command: ");
  fflush(stdout);
  
  fgets(command_string, MAX_INPUT_LEN, stdin);
  
  switch(command_string[0])
  {
  case 'P' :
  case 'p' :
    return(SPDB_PUT_DATA);
    
  case 'A' :
  case 'a' :
    return(SPDB_PUT_DATA_ADD);
    
  case 'O' :
  case 'o' :
    return(SPDB_PUT_DATA_OVER);
    
  default:
    return((spdb_request_t)-1);
  }
}
 
/*****************************************************************
 * GET_PUT_REQUEST_DATA : Get the data for the put command from the
 *                        user.
 *
 * Returns 0 on success, -1 on error.
 */

static int get_put_request_data(si32 *nchunks,
				spdb_chunk_ref_t **chunk_hdrs,
				void **chunk_data,
				si32 *chunk_data_len)
{
  static int first_call = TRUE;
  static MEMbuf *hdr_buf;
  static MEMbuf *data_buf;
  
  char input_line[MAX_INPUT_LEN];
  int i;
  spdb_chunk_ref_t hdr;
  
  date_time_t valid_time_struct;
  date_time_t expire_time_struct;
  int expire_secs;
  
  /*
   * Initialize the static buffers.
   */

  if (first_call)
  {
    hdr_buf = MEMbufCreate();
    data_buf = MEMbufCreate();
    first_call = FALSE;
  }
  else
  {
    MEMbufReset(hdr_buf);
    MEMbufReset(data_buf);
  }
  
  /*
   * Initialize returned values.
   */

  *nchunks = 0;
  *chunk_hdrs = NULL;
  *chunk_data = NULL;
  *chunk_data_len = 0;
  
  /*
   * Get the number of chunks from the user.
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "Enter the number of chunks to put in the database: ");
  fflush(stdout);
  
  fgets(input_line, MAX_INPUT_LEN, stdin);
  
  *nchunks = atoi(input_line);
  
  if (*nchunks <= 0)
  {
    *nchunks = 0;
    return(-1);
  }
  
  /*
   * Get the data type from the user.
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "Enter the data type (integer, 0 = ignore): ");
  fflush(stdout);
  
  fgets(input_line, MAX_INPUT_LEN, stdin);
  
  hdr.data_type = atoi(input_line);
  
  /*
   * Get the valid time and expire time from the user.  We will use
   * the same times for all of the chunks entered in this group.
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "Enter the valid time (dd/mm/yy hh:mm:ss, RET for now): ");
  fflush(stdout);
  
  fgets(input_line, MAX_INPUT_LEN, stdin);
  
  if (STRequal_exact(input_line, "\n"))
  {
    hdr.valid_time = time(NULL);
    valid_time_struct = *udate_time(hdr.valid_time);
  }
  else if (sscanf(input_line, "%d/%d/%d %d:%d:%d",
		  &valid_time_struct.day,
		  &valid_time_struct.month,
		  &valid_time_struct.year,
		  &valid_time_struct.hour,
		  &valid_time_struct.min,
		  &valid_time_struct.sec) == 6)
  {
    if (valid_time_struct.year < 1900)
      valid_time_struct.year += 1900;
    
    hdr.valid_time = uconvert_to_utime(&valid_time_struct);
  }
  else
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "ERROR - Invalid time entry -- could not parse\n");
    fprintf(stdout, "\n");
    fflush(stdout);
    
    return(-1);
  }
    

  fprintf(stdout, "\n");
  fprintf(stdout, "Enter the expire time (dd/mm/yy hh:mm:ss, nnn secs, RET for now): ");
  fflush(stdout);
  
  fgets(input_line, MAX_INPUT_LEN, stdin);
  
  if (STRequal_exact(input_line, "\n"))
    hdr.expire_time = time(NULL);
  else if (sscanf(input_line, "%d/%d/%d %d:%d:%d",
		  &expire_time_struct.day,
		  &expire_time_struct.month,
		  &expire_time_struct.year,
		  &expire_time_struct.hour,
		  &expire_time_struct.min,
		  &expire_time_struct.sec) == 6)
  {
    if (expire_time_struct.year < 1900)
      expire_time_struct.year += 1900;
    
    hdr.expire_time = uconvert_to_utime(&expire_time_struct);
  }
  else if (sscanf(input_line, "%d",
		  &expire_secs) == 1)
  {
    hdr.expire_time = hdr.valid_time + expire_secs;
  }
  else
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "ERROR - Invalid time entry -- could not parse\n");
    fprintf(stdout, "\n");
    fflush(stdout);
    
    return(-1);
  }
    
  /*
   * Get the chunk data from the user.
   */

  for (i = 0; i < *nchunks; i++)
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "Data for chunk %d:\n", i);
    
    fprintf(stdout, "  Enter a string for the data: ");
    fflush(stdout);
    fgets(input_line, MAX_INPUT_LEN, stdin);
    input_line[MAX_INPUT_LEN-1] = '\0';
      
    /*
     * Fill in the header information.
     */

    hdr.offset = MEMbufLen(data_buf);
    hdr.len = strlen(input_line) + 1;
      
    /*
     * Add the data to the returned buffer.
     */

    MEMbufAdd(hdr_buf, &hdr, sizeof(hdr));
    MEMbufAdd(data_buf, input_line, hdr.len);

    *chunk_data_len += hdr.len;
  }
  
  /*
   * Set the returned pointers.
   */

  *chunk_hdrs = (spdb_chunk_ref_t *) MEMbufPtr(hdr_buf);
  *chunk_data = (void **) MEMbufPtr(data_buf);
  
  return(0);
}
 
/*****************************************************************
 * SEND_GET_COMMAND : Send a get command to the server and see what
 *                    we get back.
 */

static void send_get_command(int unique_flag)
{
  get_request_t request_type;
  
  ui32 nchunks;
  si32 i;

  spdb_chunk_ref_t *chunk_hdrs;
  void *chunk_data;
  
  int prod_id;
  char input_buffer[MAX_INPUT_LEN];
  
  /*
   * Set up the request based on the type of request desired
   * by the user.
   */

  request_type = get_get_request_type();
  
  switch(request_type)
  {
  case GET :
    if (request_data_get(SourceString,
			 &nchunks,
			 &chunk_hdrs,
			 &chunk_data) != 0)
      return;
    break;
    
  case GET_CLOSEST :
    if (request_data_get_closest(SourceString,
				 &nchunks,
				 &chunk_hdrs,
				 &chunk_data) != 0)
      return;
    break;
    
  case GET_INTERVAL :
    if (request_data_get_interval(SourceString,
				  &nchunks,
				  &chunk_hdrs,
				  &chunk_data) != 0)
      return;
    break;
    
  case GET_VALID :
    if (request_data_get_valid(SourceString,
			       &nchunks,
			       &chunk_hdrs,
			       &chunk_data) != 0)
      return;
    break;
    
  case GET_FIRST_BEFORE :
    if (request_data_get_first_before(SourceString,
				      &nchunks,
				      &chunk_hdrs,
				      &chunk_data) != 0)
      return;
    break;
    
  case GET_FIRST_AFTER :
    if (request_data_get_first_after(SourceString,
				     &nchunks,
				     &chunk_hdrs,
				     &chunk_data) != 0)
      return;
    break;
    
  case GET_CMD_ERROR :
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "Invalid get command type -- no request sent\n");
    fprintf(stdout, "\n");
    fflush(stdout);
    break;
  } /* endcase - GET_CMD_ERROR */
    
  } /* endswitch - request_type */
  
  /*
   * Get rid of the duplicates, if requested.
   */

  si32 nchunks_print;
  spdb_chunk_ref_t *chunk_hdrs_print;
  void *chunk_data_print;
  
  if (unique_flag)
  {
    SPDB_last_unique(nchunks, chunk_hdrs, chunk_data,
		     &nchunks_print, &chunk_hdrs_print, &chunk_data_print);
  }
  else
  {
    nchunks_print = nchunks;
    chunk_hdrs_print = chunk_hdrs;
    chunk_data_print = chunk_data;
  }
  
  /*
   * Print out the results.
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "%d chunks received from server.\n", nchunks_print);
  fprintf(stdout, "\n");
  fflush(stdout);
  
  for (i = 0; i < nchunks_print; i++)
  {
    void *chunk_data_ptr;
    
    fprintf(stdout, "Header for chunk %d:\n", i);
    fprintf(stdout, "\n");
    print_chunk_hdr(stdout, &chunk_hdrs_print[i]);
    fprintf(stdout, "\n");
    fflush(stdout);
    
    chunk_data_ptr = (void *)((char *)chunk_data_print +
			      chunk_hdrs_print[i].offset);

    if (ProductId != 0) {
      prod_id = ProductId;
    } else {
      prod_id = chunk_hdrs_print[i].prod_id;
    }
    switch (prod_id)
    {
    case SPDB_ASCII_ID :
      print_ascii_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;
      
    case SPDB_AC_DATA_ID :
      print_ac_data_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;
      
    case SPDB_AC_POSN_ID :
      print_ac_posn_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;
      
    case SPDB_AC_POSN_WMOD_ID :
      print_ac_posn_wmod_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;
      
    case SPDB_BDRY_ID :
      print_bdry_data(stdout, chunk_data_ptr);
      break;
    
    case SPDB_FLT_PATH_ID :
      print_flt_path_data(stdout, chunk_data_ptr);
      break;
    
    case SPDB_FLT_ROUTE_ID :
      print_flt_route_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;

    case SPDB_GENERIC_POINT_ID :
      print_GenPt_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;

    case SPDB_GENERIC_POLYLINE_ID :
      print_GenPoly_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;

    case SPDB_HYDRO_STATION_ID :
      print_HydroStation_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;

    case SPDB_LTG_ID :
      print_ltg_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;
      
    case SPDB_PIREP_ID :
      print_pirep_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;
      
    case SPDB_POSN_RPT_ID :
      print_posn_rpt_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;

    case SPDB_SIGMET_ID :
      print_sigmet_data(stdout, chunk_data_ptr);
      break;
    
    case SPDB_SNDG_ID:
       print_sndg_data(stdout, chunk_data_ptr);
       break;
    
    case SPDB_STATION_REPORT_ID :
      print_stn_report(stdout, chunk_data_ptr);
      break;
      
    case SPDB_SYMPROD_ID :
      print_symprod_data(stdout, chunk_data_ptr);
      break;
    
    case SPDB_TREC_GAUGE_ID :
      print_trec_gauge_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;
    
    case SPDB_TREC_PT_FORECAST_ID :
      print_history_forecast_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;
       
    case SPDB_TSTORMS_ID :
      print_tstorms_data(stdout, chunk_data_ptr);
      break;
    
    case SPDB_VERGRID_REGION_ID :
      print_vergrid_region(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;

    case SPDB_WX_HAZARDS_ID :
      print_wx_hazard_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;

    case SPDB_ZR_PARAMS_ID :
      print_zr_params_data(stdout, chunk_data_ptr);
      break;
    
    case SPDB_ZRPF_ID :
      print_zrpf_data(stdout, &chunk_hdrs[i], chunk_data_ptr);
      break;
    
    } /* endswitch - ProductId */ 

    fprintf(stdout, "\nPress RETURN for next chunk, \"Q\" to quit: ");
    fflush(stdout);
    
    fgets(input_buffer, MAX_INPUT_LEN, stdin);

    if (input_buffer[0] == 'q' || input_buffer[0] == 'Q')
      break;
  }
  
  return;
}
 
/*****************************************************************
 * SEND_PUT_COMMAND : Send a put command to the server and see what
 *                    we get back.
 */

static void send_put_command(void)
{
  
  spdb_request_t request_type;
  
  si32 nchunks;
  spdb_chunk_ref_t *chunk_hdrs;
  void *chunk_data;
  int chunk_data_len;
  
  /*
   * Set up the request based on the type of request desired
   * by the user.
   */

  request_type = get_put_request_type();
  
  if (request_type != SPDB_PUT_DATA &&
      request_type != SPDB_PUT_DATA_ADD &&
      request_type != SPDB_PUT_DATA_OVER)
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "Invalid put command type -- no request sent\n");
    fprintf(stdout, "\n");
    fflush(stdout);
    
    return;
  }

  if (get_put_request_data(&nchunks,
			   &chunk_hdrs,
			   &chunk_data,
			   &chunk_data_len) != 0)
    return;
  
  switch (request_type)
  {
  case SPDB_PUT_DATA :
    if (SPDB_put(SourceString,
		 ProductId,
		 ProductLabel,
		 nchunks,
		 chunk_hdrs,
		 chunk_data,
		 chunk_data_len) != 0)
      return;
    break;
    
  case SPDB_PUT_DATA_ADD :
    if (SPDB_put_add(SourceString,
		     ProductId,
		     ProductLabel,
		     nchunks,
		     chunk_hdrs,
		     chunk_data,
		     chunk_data_len) != 0)
      return;
    break;
    
  case SPDB_PUT_DATA_OVER :
    if (SPDB_put_over(SourceString,
		      ProductId,
		      ProductLabel,
		      nchunks,
		      chunk_hdrs,
		      chunk_data,
		      chunk_data_len) != 0)
      return;
    break;

  default:
    fprintf(stderr, "Invalid request type %d\n", request_type);
    fflush(stderr);
    return;
    
  } /* endswitch - request_type */
  
  return;
}
 
/*****************************************************************
 * REQUEST_DATA_GET : Request data from the server using a get
 *                    command.
 *
 * Returns 0 on success, -1 on error.
 */

static int request_data_get(char *source,
			    ui32 *nchunks,
			    spdb_chunk_ref_t **chunk_hdrs,
			    void ** chunk_data)
{
  char input_string[MAX_INPUT_LEN];
  date_time_t request_time_struct;
  si32 request_time;
  si32 data_type;
  
  /*
   * Get the request time from the user
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "Enter request time (dd/mm/yy hh:mm:ss, RET for now): ");
  fflush(stdout);
  
  fgets(input_string, MAX_INPUT_LEN, stdin);
  
  if (STRequal_exact(input_string, "\n"))
    request_time = time(NULL);
  else
  {
    if (sscanf(input_string, "%d/%d/%d %d:%d:%d",
	       &request_time_struct.day,
	       &request_time_struct.month,
	       &request_time_struct.year,
	       &request_time_struct.hour,
	       &request_time_struct.min,
	       &request_time_struct.sec) != 6)
    {
      fprintf(stdout, "\n");
      fprintf(stdout, "ERROR - Invalid time entry -- could not parse\n");
      fprintf(stdout, "\n");
      fflush(stdout);
      
      return(-1);
    }
    
    if (request_time_struct.year < 1900)
      request_time_struct.year += 1900;
    
    request_time = uconvert_to_utime(&request_time_struct);
  }
  
  /*
   * Get the data type from the user.
   */

  data_type = get_data_type();
  
  /*
   * Try to get the data.
   */

  if (SPDB_get(source,
	       ProductId,
	       data_type,
	       request_time,
	       nchunks,
	       chunk_hdrs,
	       chunk_data) != 0)
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "ERROR - SPDB_get failed on source <%s>\n",
	    source);
    fprintf(stdout, "\n");
    fflush(stdout);
    return(-1);
  }

  return(0);
}

/*****************************************************************
 * REQUEST_DATA_GET_CLOSEST : Request data from the server using a
 *                            get closest command.
 *
 * Returns 0 on success, -1 on error.
 */

static int request_data_get_closest(char *source,
				    ui32 *nchunks,
				    spdb_chunk_ref_t **chunk_hdrs,
				    void ** chunk_data)
{
  char input_string[MAX_INPUT_LEN];
  date_time_t request_time_struct;
  si32 request_time;
  
  si32 time_margin;
  
  si32 data_type;
  
  /*
   * Get the request time from the user
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "Enter request time (dd/mm/yy hh:mm:ss, RET for now): ");
  fflush(stdout);
  
  fgets(input_string, MAX_INPUT_LEN, stdin);
  
  if (STRequal_exact(input_string, "\n"))
    request_time = time(NULL);
  else
  {
    if (sscanf(input_string, "%d/%d/%d %d:%d:%d",
	       &request_time_struct.day,
	       &request_time_struct.month,
	       &request_time_struct.year,
	       &request_time_struct.hour,
	       &request_time_struct.min,
	       &request_time_struct.sec) != 6)
    {
      fprintf(stdout, "\n");
      fprintf(stdout, "ERROR - Invalid time entry -- could not parse\n");
      fprintf(stdout, "\n");
      fflush(stdout);
      
      return(-1);
    }
    
    if (request_time_struct.year < 1900)
      request_time_struct.year += 1900;
    
    request_time = uconvert_to_utime(&request_time_struct);
  }
  
  /*
   * Get the time margin from the user.
   */

  fprintf(stdout, "Enter time margin (secs), RET for unlimited: ");
  fflush(stdout);
  
  fgets(input_string, MAX_INPUT_LEN, stdin);
  
  if (STRequal_exact(input_string, "\n")) {
    time_margin = -1;
  } else {
    time_margin = atoi(input_string);
  }
  
  /*
   * Get the data type from the user.
   */

  data_type = get_data_type();
  
  /*
   * Try to get the data.
   */

  if (SPDB_get_closest(source,
		       ProductId,
		       data_type,
		       request_time,
		       time_margin,
		       nchunks,
		       chunk_hdrs,
		       chunk_data) != 0)
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "ERROR - SPDB_get_closest failed on source <%s>\n",
	    source);
    fprintf(stdout, "\n");
    fflush(stdout);
    
    return(-1);
  }

  return(0);
}

/*****************************************************************
 * REQUEST_DATA_GET_INTERVAL : Request data from the server using
 *                             a get interval command.
 *
 * Returns 0 on success, -1 on error.
 */

static int request_data_get_interval(char *source,
				     ui32 *nchunks,
				     spdb_chunk_ref_t **chunk_hdrs,
				     void ** chunk_data)
{
  char input_string[MAX_INPUT_LEN];
  date_time_t time_struct;
  si32 start_time;
  si32 end_time;
  si32 data_type;
  
  /*
   * Get the start time from the user
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "Enter start time (dd/mm/yy hh:mm:ss): ");
  fflush(stdout);
  
  fgets(input_string, MAX_INPUT_LEN, stdin);
  
  if (sscanf(input_string, "%d/%d/%d %d:%d:%d",
	     &time_struct.day,
	     &time_struct.month,
	     &time_struct.year,
	     &time_struct.hour,
	     &time_struct.min,
	     &time_struct.sec) != 6)
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "ERROR - Invalid time entry -- could not parse\n");
    fprintf(stdout, "\n");
    fflush(stdout);
    
    return(-1);
  }
    
  if (time_struct.year < 1900)
    time_struct.year += 1900;
    
  start_time = uconvert_to_utime(&time_struct);
  
  /*
   * Get the end time from the user
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "Enter end time (dd/mm/yy hh:mm:ss, RET for now): ");
  fflush(stdout);
  
  fgets(input_string, MAX_INPUT_LEN, stdin);
  
  if (STRequal_exact(input_string, "\n"))
    end_time = time(NULL);
  else
  {
    if (sscanf(input_string, "%d/%d/%d %d:%d:%d",
	       &time_struct.day,
	       &time_struct.month,
	       &time_struct.year,
	       &time_struct.hour,
	       &time_struct.min,
	       &time_struct.sec) != 6)
    {
      fprintf(stdout, "\n");
      fprintf(stdout, "ERROR - Invalid time entry -- could not parse\n");
      fprintf(stdout, "\n");
      fflush(stdout);
      
      return(-1);
    }
    
    if (time_struct.year < 1900)
      time_struct.year += 1900;
    
    end_time = uconvert_to_utime(&time_struct);
  }
  
  /*
   * Get the data type from the user.
   */

  data_type = get_data_type();
  
  /*
   * Try to get the data.
   */

  fprintf(stdout, "Getting data between %s and %s\n",
	  utimstr(start_time), utimstr(end_time));
  
  if (SPDB_get_interval(source,
			ProductId,
			data_type,
			start_time,
			end_time,
			nchunks,
			chunk_hdrs,
			chunk_data) != 0)
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "ERROR - SPDB_get_interval failed on source <%s>\n",
	    source);
    fprintf(stdout, "\n");
    fflush(stdout);
    
    return(-1);
  }

  return(0);
}

/*****************************************************************
 * REQUEST_DATA_GET_VALID : Request data from the server using a
 *                          get valid command.
 *
 * Returns 0 on success, -1 on error.
 */

static int request_data_get_valid(char *source,
				  ui32 *nchunks,
				  spdb_chunk_ref_t **chunk_hdrs,
				  void ** chunk_data)
{
  char input_string[MAX_INPUT_LEN];
  date_time_t search_time_struct;
  si32 search_time;
  si32 data_type;
  
  /*
   * Get the search time from the user
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "Enter search time (dd/mm/yy hh:mm:ss, RET for now): ");
  fflush(stdout);
  
  fgets(input_string, MAX_INPUT_LEN, stdin);
  
  if (STRequal_exact(input_string, "\n"))
    search_time = time(NULL);
  else
  {
    if (sscanf(input_string, "%d/%d/%d %d:%d:%d",
	       &search_time_struct.day,
	       &search_time_struct.month,
	       &search_time_struct.year,
	       &search_time_struct.hour,
	       &search_time_struct.min,
	       &search_time_struct.sec) != 6)
    {
      fprintf(stdout, "\n");
      fprintf(stdout, "ERROR - Invalid time entry -- could not parse\n");
      fprintf(stdout, "\n");
      fflush(stdout);
      
      return(-1);
    }
    
    if (search_time_struct.year < 1900)
      search_time_struct.year += 1900;
    
    search_time = uconvert_to_utime(&search_time_struct);
  }
  
  /*
   * Get the data type from the user.
   */

  data_type = get_data_type();
  
  /*
   * Try to get the data.
   */

  if (SPDB_get_valid(source,
		     ProductId,
		     data_type,
		     search_time,
		     nchunks,
		     chunk_hdrs,
		     chunk_data) != 0)
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "ERROR - SPDB_get_valid failed on source <%s>\n",
	    source);
    fprintf(stdout, "\n");
    fflush(stdout);
    
    return(-1);
  }

  return(0);
}


/*****************************************************************
 * REQUEST_DATA_GET_FIRST_BEFORE : Request data from the server
 *                                 using a get first before command.
 *
 * Returns 0 on success, -1 on error.
 */

static int request_data_get_first_before(char *source,
					 ui32 *nchunks,
					 spdb_chunk_ref_t **chunk_hdrs,
					 void ** chunk_data)
{
  char input_string[MAX_INPUT_LEN];
  date_time_t request_time_struct;
  si32 request_time;
  
  si32 time_margin;
  
  si32 data_type;
  
  /*
   * Get the request time from the user
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "Enter request time (dd/mm/yy hh:mm:ss, RET for now): ");
  fflush(stdout);
  
  fgets(input_string, MAX_INPUT_LEN, stdin);
  
  if (STRequal_exact(input_string, "\n"))
    request_time = time(NULL);
  else
  {
    if (sscanf(input_string, "%d/%d/%d %d:%d:%d",
	       &request_time_struct.day,
	       &request_time_struct.month,
	       &request_time_struct.year,
	       &request_time_struct.hour,
	       &request_time_struct.min,
	       &request_time_struct.sec) != 6)
    {
      fprintf(stdout, "\n");
      fprintf(stdout, "ERROR - Invalid time entry -- could not parse\n");
      fprintf(stdout, "\n");
      fflush(stdout);
      
      return(-1);
    }
    
    if (request_time_struct.year < 1900)
      request_time_struct.year += 1900;
    
    request_time = uconvert_to_utime(&request_time_struct);
  }
  
  /*
   * Get the time margin from the user.
   */

  fprintf(stdout, "Enter time margin (secs), RET for unlimited: ");
  fflush(stdout);
  
  fgets(input_string, MAX_INPUT_LEN, stdin);
  
  if (STRequal_exact(input_string, "\n")) {
    time_margin = -1;
  } else {
    time_margin = atoi(input_string);
  }
  
  /*
   * Get the data type from the user.
   */

  data_type = get_data_type();
  
  /*
   * Try to get the data.
   */

  if (SPDB_get_first_before(source,
			    ProductId,
			    data_type,
			    request_time,
			    time_margin,
			    nchunks,
			    chunk_hdrs,
			    chunk_data) != 0)
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "ERROR - SPDB_get_first_before failed on source <%s>\n",
	    source);
    fprintf(stdout, "\n");
    fflush(stdout);
    
    return(-1);
  }

  return(0);
}


/*****************************************************************
 * REQUEST_DATA_GET_FIRST_AFTER : Request data from the server
 *                                using a get first after command.
 *
 * Returns 0 on success, -1 on error.
 */

static int request_data_get_first_after(char *source,
					ui32 *nchunks,
					spdb_chunk_ref_t **chunk_hdrs,
					void ** chunk_data)
{
  char input_string[MAX_INPUT_LEN];
  date_time_t request_time_struct;
  si32 request_time;
  
  si32 time_margin;
  
  si32 data_type;
  
  /*
   * Get the request time from the user
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "Enter request time (dd/mm/yy hh:mm:ss, RET for now): ");
  fflush(stdout);
  
  fgets(input_string, MAX_INPUT_LEN, stdin);
  
  if (STRequal_exact(input_string, "\n"))
    request_time = time(NULL);
  else
  {
    if (sscanf(input_string, "%d/%d/%d %d:%d:%d",
	       &request_time_struct.day,
	       &request_time_struct.month,
	       &request_time_struct.year,
	       &request_time_struct.hour,
	       &request_time_struct.min,
	       &request_time_struct.sec) != 6)
    {
      fprintf(stdout, "\n");
      fprintf(stdout, "ERROR - Invalid time entry -- could not parse\n");
      fprintf(stdout, "\n");
      fflush(stdout);
      
      return(-1);
    }
    
    if (request_time_struct.year < 1900)
      request_time_struct.year += 1900;
    
    request_time = uconvert_to_utime(&request_time_struct);
  }
  
  /*
   * Get the time margin from the user.
   */

  fprintf(stdout, "Enter time margin (secs), RET for unlimited: ");
  fflush(stdout);
  
  fgets(input_string, MAX_INPUT_LEN, stdin);
  
  if (STRequal_exact(input_string, "\n")) {
    time_margin = -1;
  } else {
    time_margin = atoi(input_string);
  }
  
  /*
   * Get the data type from the user.
   */

  data_type = get_data_type();
  
  /*
   * Try to get the data.
   */

  if (SPDB_get_first_after(source,
			   ProductId,
			   data_type,
			   request_time,
			   time_margin,
			   nchunks,
			   chunk_hdrs,
			   chunk_data) != 0)
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "ERROR - SPDB_get_first_after failed on source <%s>\n",
	    source);
    fprintf(stdout, "\n");
    fflush(stdout);
    
    return(-1);
  }

  return(0);
}


/*****************************************************************
 * GET_DATA_TYPE : Gets the desired data_type from the user.
 */

int get_data_type(void)
{
  char input_string[MAX_INPUT_LEN];
  int data_type;
  
  fprintf(stdout, "\n");
  fflush(stdout);
  
  if (ProductId == SPDB_AC_DATA_ID ||
      ProductId == SPDB_AC_POSN_ID)
  {
    fprintf(stdout, "Enter the desired callsign (CR = ignore): ");
    fflush(stdout);
    
    fgets(input_string, MAX_INPUT_LEN, stdin);
    
    if (input_string[0] == '\n')
      data_type = 0;
    else
    {
      input_string[strlen(input_string)-1] = '\0';
      data_type = ac_data_callsign_hash(input_string);
    }
  }
  else
  {
    fprintf(stdout, "Enter the desired data type (0 = ignore): ");
    fflush(stdout);
    
    fgets(input_string, MAX_INPUT_LEN, stdin);
  
    data_type = atoi(input_string);
  }

  return(data_type);
}
