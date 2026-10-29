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
/*********************************************************************
 * dist_spdb_data.cc: dist_spdb_data main program.  This program
 *                    receives SPDB data over a socket and pushes it
 *                    to all of the destinations given in the
 *                    parameter file.  The destinations may be either
 *                    databases on a disk mounted on the local
 *                    machine or sockets on remote (or local) machines.
 *
 * RAP, NCAR, Boulder CO
 *
 * Oct 1996
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <dataport/bigend.h>
#include <dataport/port_types.h>

#include <symprod/spdb_client.h>

#include <tdrp/tdrp.h>

#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <toolsa/procmap.h>
#include <toolsa/smu.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "dist_spdb_data.h"
#include "dist_spdb_data_tdrp.h"

/**********************************************************************
 * Forward declarations for static functions.
 */

static void parse_args(char *prog_name, int argc, char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       char **params_file_path_p,
		       tdrp_override_t *override);

static si32 extract_request_info(char *message,
				 ui32 *nchunks,
				 spdb_chunk_ref_t **chunk_hdrs,
				 void **chunk_data,
				 ui32 *chunk_data_len);

static void fill_servmap_info(si32 *data_start_time,
			      si32 *data_end_time,
			      si32 *last_data_time,
			      si32 *last_request_time,
			      si32 *n_data_requests);

static void handleClient(dist_spdb_data_tdrp_struct *params,
			 char *prog_name,
			 int clientFd);

/**********************************************************************
 * Global variables.
 */

static si32 _data_start_time = 0;
static si32 _data_end_time = 0;
static si32 _last_data_time = 0;

/**********************************************************************
 * Main program.
 */

int main(int argc, char **argv)
{

  // basic declarations

  path_parts_t progname_parts;
  char *prog_name;
  char *params_path_name;
  char *routine_name = "main";
  
  int check_params;
  int print_params;
  tdrp_override_t override;
  TDRPtable *table;
  dist_spdb_data_tdrp_struct params;
  
  int server_fd;
  int forever = TRUE;

  // set program name

  uparse_path(argv[0], &progname_parts);
  prog_name = STRdup(progname_parts.base);

  // display ucopyright message

  ucopyright(prog_name);

  // register function to trap termination and interrupts

  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGPIPE, handle_sigpipe);
  
  // parse the command line arguments, and open files as required

  parse_args(prog_name, argc, argv,
	     &check_params, &print_params,
	     &params_path_name, &override);

  // load up parameters

  table = dist_spdb_data_tdrp_init(&params);
  
  if (FALSE == TDRP_read(params_path_name,
			 table,
			 &params,
			 override.list)) {
    fprintf(stderr, "ERROR - %s:main\n", prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
	    params_path_name);
    tidy_and_exit(-1);
  }
  
  TDRP_free_override(&override);
  
  if (check_params)
  {
    TDRP_check_is_set(table, &params);
    tidy_and_exit(-1);
  }
  
  if (print_params)
  {
    TDRP_print_params(table, &params, prog_name, TRUE);
    tidy_and_exit(-1);
  }
  
  // Set the malloc debug level

  umalloc_debug(params.malloc_debug_level);
  
  // Initialize the PMU calls.

  PMU_auto_init(prog_name,
		params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // Initialize the SMU calls.

  SMU_auto_init(params.servmap_type,
		params.servmap_subtype,
		params.instance,
		params._destinations[0],
		params.input_port,
		TRUE,
		fill_servmap_info);
  
  SMU_auto_register();

  // Set up the server socket.  This is the socket where the
  // new SPDB data will be received.

  if ((server_fd = SKU_open_server(params.input_port)) <0)
  {
    fprintf(stderr, "%s:%s\n",
	    prog_name, routine_name);
    fprintf(stderr, "ERROR - Cannot open server socket\n");
    fprintf(stderr, "%d returned by SKU_open_server\n", server_fd);
    
    exit(-1);
  }
  
  // Operate forever.

  while (forever) {

    int clientFd = -1;
    
    // Register with the the process mapper

    PMU_auto_register("waiting for data");
    
    // Wait for a client connection

    if ((clientFd = SKU_get_client_timed(server_fd,
					 params.wait_msecs)) >= 0) {

      if (params.debug) {
	fprintf(stderr, "Got client\n");
      }

      // got client
      
      // Register with the the process mapper again,
      // just in case there was a long wait period
      
      PMU_auto_register("getting data from algorithm/server");
      
      // Register with the process mapper
  
      PMU_auto_register("handling client");
    
      // Handle the message from the client
      
      handleClient(&params, prog_name, clientFd);
	
      // close connection
	
      SKU_close(clientFd);
	
    } // if ((clientFd = SKU_get_client_timed ...

    // Reap any outstanding children

    SPDB_reap_children();
    
    // Register with the server mapper

    SMU_auto_register();
    
  } // while (forever)
  
  exit(0);
  
}

/////////////////////////////////////////////////////////
// handleClient() - Read the message from the client and
//                  respond to it.

static void handleClient(dist_spdb_data_tdrp_struct *params,
			 char *prog_name,
			 int clientFd)

{

  static char *routine_name = "handleClient";
  SKU_header_t sku_header;
  char *message;
  long message_len;
    
  int sku_status;
    
  si32 request_type;
    
  ui32 nchunks;
  spdb_chunk_ref_t *chunk_hdrs;
  void *chunk_data;
  ui32 chunk_data_len;
  
  // Read the message from the client

  if ((sku_status = SKU_read_message(clientFd,
				     &sku_header,
				     &message,
				     &message_len,
				     params->wait_msecs)) != 1)
    {
      fprintf(stderr, "%s:%s\n",
	      prog_name, routine_name);
      fprintf(stderr, "ERROR - Error %d reading message from client\n",
	      sku_status);
      return;
    }
	
  // Send a reply to the client so he knows we received the data
    
  si32 reply = BE_from_si32(SPDB_PUT_SUCCESSFUL);
    
  if (SKU_write_message(clientFd,
			sku_header.id,
			(char *)&reply,
			sizeof(si32)) != 1)
    {
      fprintf(stderr, "%s:%s\n",
	      prog_name, routine_name);
      fprintf(stderr,
	      "WARNING - Error sending acknowledgement back to algorithm\n");
    }
  
  // Extract the SPDB information so it can be passed on.

  request_type = extract_request_info(message,
				      &nchunks,
				      &chunk_hdrs,
				      &chunk_data,
				      &chunk_data_len);
    
  // Send the data to all of the destinations

  for (int i = 0; i < params->destinations.len; i++)
    {
      int put_return;
      
      switch(request_type)
	{
	case SPDB_PUT_DATA :
	  put_return = SPDB_put(params->destinations.val[i],
				sku_header.id,
				params->product_label,
				nchunks,
				chunk_hdrs,
				chunk_data,
				chunk_data_len);
	  break;
	  
	case SPDB_PUT_DATA_ADD :
	  put_return = SPDB_put_add(params->destinations.val[i],
				    sku_header.id,
				    params->product_label,
				    nchunks,
				    chunk_hdrs,
				    chunk_data,
				    chunk_data_len);
	  break;
	  
	case SPDB_PUT_DATA_OVER :
	  put_return = SPDB_put_over(params->destinations.val[i],
				     sku_header.id,
				     params->product_label,
				     nchunks,
				     chunk_hdrs,
				     chunk_data,
				     chunk_data_len);
	  break;
	  
	default:
	  fprintf(stderr, "%s:%s\n",
		  prog_name, routine_name);
	  fprintf(stderr,
		  "ERROR - Invalid request type %d received from client.\n",
		  request_type);
	  put_return = 0;
	  break;
	  
	} /* endswitch - params->put_type */
      
      if (put_return != 0)
	{
	  fprintf(stderr, "%s:%s\n",
		  prog_name, routine_name);
	  fprintf(stderr, "ERROR - Error sending data to "
		  "destination %s, put_return = %d\n",
		  params->destinations.val[i], put_return);
	}
      
    } // i
    
  // Update the data times for the server mapper
  
  for (ui32 i = 0; i < nchunks; i++)
  {
    if (_data_start_time == 0)
      _data_start_time = chunk_hdrs[i].valid_time;
    else if (_data_start_time > chunk_hdrs[i].valid_time)
      _data_start_time = chunk_hdrs[i].valid_time;

    if (_data_end_time == 0)
      _data_end_time = chunk_hdrs[i].valid_time;
    else if (_data_end_time < chunk_hdrs[i].valid_time)
      _data_end_time = chunk_hdrs[i].valid_time;
  }

  if (nchunks > 0) {
    _last_data_time = chunk_hdrs[nchunks-1].valid_time;
  }

  SMU_auto_register();
}
  
/*********************************************************************
 * handle_sigpipe() - Handle a SIGPIPE interrupt.
 */

void handle_sigpipe(int sig)
{

  // suppress compiler warning on unused arg

  int i;
  i = sig;

  // Let the user know we got a SIGPIPE

  fprintf(stderr, "dist_spdb_data:  Handling SIGPIPE\n");
  
  return;

}


/*********************************************************************
 * tidy_and_exit() - Clean up any memory, etc. and exit the program.
 */

void tidy_and_exit(int sig)
{
  // unregister the process

  PMU_auto_unregister();
  SMU_auto_unregister();

  // verify mallocs

  umalloc_map();
  umalloc_verify();

  // exit with code sig

  exit(sig);

}


/*******************************************************************
 * parse_args() - Parse the command line arguments.
 */

#define TMP_STR_LEN 8192

static void parse_args(char *prog_name,
		       int argc, char **argv,
		       int *check_params_p,
		       int *print_params_p,
		       char **params_file_path_p,
		       tdrp_override_t *override)
{

  int error_flag = 0;
  int warning_flag = 0;
  int i;

  char usage[BUFSIZ];
  char tmp_str[TMP_STR_LEN];
  
  // Initialize the returned values

  *check_params_p = FALSE;
  *print_params_p = FALSE;
  *params_file_path_p = NULL;
  TDRP_init_override(override);
  
  // load up usage string

  sprintf(usage, "%s%s%s",
	  "Usage:\n\n", prog_name, " [options] as below:\n\n"
	  "       [ -h, --, -help, -man] produce this list.\n"
	  "       [ -check_params] check parameter usage\n"
	  "       [ -debug ] debugging on\n"
	  "       [ -dest ? ? ?] destination list\n"
	  "       [ -mdebug level] set malloc debug level\n"
	  "       [ -params name] parameters file name\n"
	  "       [ -port port] input port number\n"
	  "       [ -print_params] print parameter usage\n"
	  "\n");

  // search for command options
  
  for (i = 1; i < argc; i++) {

    if (STRequal_exact(argv[i], "--") ||
	STRequal_exact(argv[i], "-h") ||
	STRequal_exact(argv[i], "-help") ||
	STRequal_exact(argv[i], "-man")) {

      printf("%s", usage);
      tidy_and_exit(1);

    } else if (STRequal_exact(argv[i], "-check_params")) {

      *check_params_p = TRUE;

    } else if (STRequal_exact(argv[i], "-print_params")) {

      *print_params_p = TRUE;
      
    } else if (STRequal_exact(argv[i], "-debug")) {

      sprintf(tmp_str, "debug = DEBUG_ALL;");
      TDRP_add_override(override, tmp_str);

    } else if (i < argc - 1) {

      if (STRequal_exact(argv[i], "-params")) {

	*params_file_path_p = argv[i+1];
	i++;

      } else if (STRequal_exact(argv[i], "-mdebug")) {

	sprintf(tmp_str, "malloc_debug_level = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
	i++;

      } else if (STRequal_exact(argv[i], "-port")) {

	sprintf(tmp_str, "input_port = %s;", argv[i+1]);
	TDRP_add_override(override, tmp_str);
	i++;

      }  else if (!strcmp(argv[i], "-dest")) {

	sprintf(tmp_str, "destinations = {");

	// move through args until we find one which starts with '-'
	
	int j;

	for (j = i + 1; j < argc; j++) {
	  
	  if (argv[j][0] == '-') {
	    break;
	  }

	  // concatenate destination onto the string
	  // making sure we leave space at the end
	  // for terminations
	  
	  STRconcat(tmp_str, "\"", TMP_STR_LEN - 3);
	  STRconcat(tmp_str, argv[j], TMP_STR_LEN - 3);
	  STRconcat(tmp_str, "\",", TMP_STR_LEN - 3);
	  
	} // j

	// terminate, overwriting last ',' with '};'

	int len = strlen(tmp_str);
	tmp_str[len - 1] = '\0';
	STRconcat(tmp_str, "};", TMP_STR_LEN);
	
	// add to override list

	TDRP_add_override(override, tmp_str);

	i = j - 1;
	
      } // if
      
    } // else if (i < argc - 1)

  } // i

  // print usage if there was an error

  if(error_flag || warning_flag)
  {
    fprintf(stderr, "%s\n", usage);
    fprintf(stderr, "Check the parameters file '%s'.\n\n",
	    *params_file_path_p);
  }

  if (error_flag)
    tidy_and_exit(1);

  return;
}


/*******************************************************************
 * fill_servmap_info() - Fill in the information for server mapper
 *                       registration.
 */

static void fill_servmap_info(si32 *data_start_time,
			      si32 *data_end_time,
			      si32 *last_data_time,
			      si32 *last_request_time,
			      si32 *n_data_requests)
{
  *data_start_time = _data_start_time;
  *data_end_time = _data_end_time;
  *last_data_time = _last_data_time;
  *last_request_time = 0;
  *n_data_requests = 0;
}


/*******************************************************************
 * extract_request_info() - Extract and byte-swap the request info
 *                          from the received message.
 */

static si32 extract_request_info(char *message,
				 ui32 *nchunks,
				 spdb_chunk_ref_t **chunk_hdrs,
				 void **chunk_data,
				 ui32 *chunk_data_len)
{
  char *msg_ptr = message;
  si32 request_type;
  
  /*
   * Initialize the returned values.
   */

  *nchunks = 0;
  *chunk_hdrs = NULL;
  *chunk_data = NULL;
  *chunk_data_len = 0;
  
  /*
   * Get the request type from the message.
   */

  request_type = *(si32 *)msg_ptr;
  request_type = BE_to_si32(request_type);
  msg_ptr += sizeof(si32);
  
  /*
   * Make sure this is a PUT request.  Otherwise we can't
   * process it.
   */

  if (request_type != SPDB_PUT_DATA &&
      request_type != SPDB_PUT_DATA_ADD &&
      request_type != SPDB_PUT_DATA_OVER)
    return(request_type);
  
  /*
   * Now get the data information.
   */

  // nchunks
  *nchunks = *(ui32 *)msg_ptr;
  *nchunks = BE_to_ui32(*nchunks);
  msg_ptr += sizeof(ui32);
  
  // data_length
  *chunk_data_len = *(ui32 *)msg_ptr;
  *chunk_data_len = BE_to_ui32(*chunk_data_len);
  msg_ptr += sizeof(ui32);
  
  // index_array
  *chunk_hdrs = (spdb_chunk_ref_t *)msg_ptr;
  BE_to_array_32(*chunk_hdrs, *nchunks * sizeof(spdb_chunk_ref_t));
  msg_ptr += *nchunks * sizeof(spdb_chunk_ref_t);
  
  // chunk_data
  *chunk_data = (void *)msg_ptr;
  
  return(request_type);
}
