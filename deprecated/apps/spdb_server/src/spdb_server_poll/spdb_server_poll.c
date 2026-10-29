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
/******************************************************************************
 *  SPDB_SERVER_POLL.C Test program for the spdb data servers 
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <sys/param.h>

#ifdef SUNOS4
extern int tolower(int c);
#endif

#include <toolsa/os_config.h>

#include <rapformats/station_reports.h>

#include <rapformats/hist_fore.h>

#include <toolsa/udatetime.h>

#include <symprod/spdb_client.h>
#include <symprod/spdb_products.h>

#include <toolsa/membuf.h>
#include <toolsa/port.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <toolsa/utim.h>


#define MAX_INPUT_LEN     1024


/************** STATIC FUNCTION PROTOTYPES **************/

static void signal_trap(int sig);

static char get_poll_request_type(void);

static int choose_poll_server(void);

static void init_polling(void);

static void delete_polling(void);

static void poll_for_data(void);

static void print_chunk_hdr(FILE *stream, spdb_chunk_ref_t *chunk_hdr);

static void print_station_reports(FILE *stream, int nchunks,
				  spdb_chunk_ref_t *chunk_hdrs,
				  void *chunk_data);

static void print_history_forecast_data(FILE *stream, int nchunks,
				  spdb_chunk_ref_t *chunk_hdrs,
				  void *chunk_data);

/************** GLOBAL DATA *****************************/

char    InputBuffer[MAX_INPUT_LEN];

spdb_poll_t **PollArray = NULL;
int           PollSize = 0;
  

/******************************************************************************
 * MAIN
 *
 */

int main(int argc, char *argv[])
{
  int done = FALSE;
  char request_type;
  
  PORTsignal(SIGINT, signal_trap);
  PORTsignal(SIGTERM, signal_trap);

  while (!done)
  {
    request_type = get_poll_request_type();
  
    switch(request_type)
    {
    case 'i' :
      init_polling();
      break;
    
    case 'd' :
      delete_polling();
      break;
    
    case 'p' :
      poll_for_data();
      break;
    
    case 'q' :
      done = TRUE;
      break;
      
    default:
      fprintf(stdout, "Invalid menu choice -- try again\n");
      break;
    } /* endswitch - request_type */
    
  } /* endwhile - !done */

  exit(0);
}

/*****************************************************************
 * SIGNAL_TRAP : Traps Signals so as to die gracefully
 */

static void signal_trap(int sig)
{
  exit(0);
}
 
/*****************************************************************
 * GET_POLL_REQUEST_TYPE : Get the type of poll request from the
 *                         user.
 */

static char get_poll_request_type(void)
{
  fprintf(stdout, "Choose a poll action:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "   I = Initialize the poll operation\n");
  fprintf(stdout, "   D = Delete the poll operation\n");
  fprintf(stdout, "   P = Poll the server for data\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "   Q = Quit\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Desired command: ");
  
  fgets(InputBuffer, MAX_INPUT_LEN, stdin);
  
  return(tolower(InputBuffer[0]));
}
 
/*****************************************************************
 * CHOOSE_POLL_SERVER : Choose a poll server from the current list.
 */

static int choose_poll_server(void)
{
  int i;
  int index;
  
  fprintf(stdout, "Choose a server:\n\n");
  for (i = 0; i < PollSize; i++)
    fprintf(stdout, "  %2d = %-10s %-10s %-10s (product %d, type %d)\n",
	    i,
	    PollArray[i]->request.server_type,
	    PollArray[i]->request.server_subtype,
	    PollArray[i]->request.instance,
	    PollArray[i]->product_id,
	    PollArray[i]->data_type);
  fprintf(stdout, "\nEnter choice: ");
  
  fgets(InputBuffer, MAX_INPUT_LEN, stdin);
  
  index = atoi(InputBuffer);
  
  if (index < 0 || index >= PollSize)
    return(-1);
  
  return(index);
}
 
/*****************************************************************
 * INIT_POLLING() - Initialize the polling process.  This must be
 *                  done before polling can begin.  There can be
 *                  several polling processes going at the same
 *                  time.
 */

static void init_polling(void)
{
  int i;
  
  char type[SERVMAP_NAME_MAX];
  char subtype[SERVMAP_NAME_MAX];
  char instance[SERVMAP_INSTANCE_MAX];
  si32 product_id;
  si32 data_type;
  int get_latest_data_only;
  
  spdb_poll_t *poll_ptr;
  
  if (PollSize > 0)
  {
    fprintf(stdout, "Current poll array:\n");
    for (i = 0; i < PollSize; i++)
      fprintf(stdout, "  %-10s %-10s %-10s (product %d, type %d)\n",
	      PollArray[i]->request.server_type,
	      PollArray[i]->request.server_subtype,
	      PollArray[i]->request.instance,
	      PollArray[i]->product_id,
	      PollArray[i]->data_type);
  }
  
  /*
   * Get the polling information from the user.
   */

  fprintf(stdout, "\n\nEnter new server type: ");
  fgets(InputBuffer, MAX_INPUT_LEN, stdin);
  InputBuffer[strlen(InputBuffer)-1] = '\0';
  STRcopy(type, InputBuffer, SERVMAP_NAME_MAX);
  
  fprintf(stdout, "Enter new server subtype: ");
  fgets(InputBuffer, MAX_INPUT_LEN, stdin);
  InputBuffer[strlen(InputBuffer)-1] = '\0';
  STRcopy(subtype, InputBuffer, SERVMAP_NAME_MAX);
  
  fprintf(stdout, "Enter new server instance: ");
  fgets(InputBuffer, MAX_INPUT_LEN, stdin);
  InputBuffer[strlen(InputBuffer)-1] = '\0';
  STRcopy(instance, InputBuffer, SERVMAP_INSTANCE_MAX);
  
  fprintf(stdout, "Enter product id: ");
  fgets(InputBuffer, MAX_INPUT_LEN, stdin);
  product_id = atoi(InputBuffer);
  
  fprintf(stdout, "Enter data type (0 = all data): ");
  fgets(InputBuffer, MAX_INPUT_LEN, stdin);
  data_type = atoi(InputBuffer);
  
  fprintf(stdout, "Get latest data only? (1 = TRUE, 0 = FALSE): ");
  fgets(InputBuffer, MAX_INPUT_LEN, stdin);
  get_latest_data_only = atoi(InputBuffer);
  
  /*
   * Create the polling structure.
   */

  poll_ptr = SPDB_poll_init(type,
			    subtype,
			    instance,
			    product_id,
			    data_type,
			    get_latest_data_only);
  
  if (PollSize <= 0)
  {
    PollSize = 1;
    PollArray = (spdb_poll_t **)umalloc(sizeof(spdb_poll_t *));
    PollArray[0] = poll_ptr;
  }
  else
  {
    PollSize++;
    PollArray = (spdb_poll_t **)urealloc(PollArray,
					 PollSize * sizeof(spdb_poll_t *));
    PollArray[PollSize-1] = poll_ptr;
  }
  
}


/*****************************************************************
 * DELETE_POLLING() - Remove a poll structure from the list.
 */

static void delete_polling(void)
{
  int poll_index;
  spdb_poll_t *temp_poll;
  int i;
  
  /*
   * Make sure we currently have a poll structure.
   */

  if (PollSize <= 0)
  {
    fprintf(stdout, "\nNo polling structures currently exist\n\n");
    return;
  }
  
  /*
   * Have the user choose the structure to delete.
   */

  poll_index = choose_poll_server();
  
  if (poll_index < 0)
  {
    fprintf(stdout, "Invalid poll index -- try again\n");
    return;
  }
  
  /*
   * Delete the poll structure and remove it from the list.
   */

  temp_poll = PollArray[poll_index];
  
  for (i = poll_index + 1; i < PollSize; i++)
    PollArray[i-1] = PollArray[i];
  PollSize--;
  
  if (PollSize <= 0)
    ufree(PollArray);
  
  SPDB_poll_delete(&temp_poll);
  
  return;
}


/*****************************************************************
 * POLL_FOR_DATA : Poll a chosen server for any new data.
 */

static void poll_for_data(void)
{
  int poll_index;
  int spdb_status;
  
  si32 nchunks;
  spdb_chunk_ref_t *chunk_hdrs;
  void *chunk_data;
  
  int i;
  
  /*
   * Ask the user which server to poll.
   */

  if ((poll_index = choose_poll_server()) < 0)
  {
    fprintf(stdout,
	    "Invalid server chosen -- try again\n");
    return;
  }
  
  /*
   * Poll the server.
   */

  if ((spdb_status = SPDB_poll(PollArray[poll_index],
			       &nchunks,
			       &chunk_hdrs,
			       &chunk_data)) != SPDB_SUCCESS)
  {
    if (spdb_status == SPDB_NO_DATA_AVAIL)
      fprintf(stdout,
	      "\nNo new data for server\n");
    else
      fprintf(stdout,
	      "Error %d returned by SPDB_poll\n",
	      spdb_status);

    return;
  } /* endif - spdb_status != SPDB_SUCCESS */
  
  /*
   * Print the data returned by the server.
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "%d chunks received from server.\n", nchunks);
  fprintf(stdout, "\n");
  
  for (i = 0; i < nchunks; i++)
  {
    fprintf(stdout, "Header for chunk %d:\n", i);
    fprintf(stdout, "\n");
    print_chunk_hdr(stdout, &chunk_hdrs[i]);
    fprintf(stdout, "\n");
  }
  
  switch (PollArray[poll_index]->product_id)
  {
  case SPDB_STATION_REPORT_ID :
    print_station_reports(stdout, nchunks, chunk_hdrs, chunk_data);
    break;
      
  case SPDB_TREC_PT_FORECAST_ID :
    print_history_forecast_data(stdout, nchunks, chunk_hdrs, chunk_data);
    break;
       
  } /* endswitch - ProductId */ 

  return;
}


/*****************************************************************
 * PRINT_CHUNK_HDR : Print a chunk header to the indicated stream.
 */

static void print_chunk_hdr(FILE *stream, spdb_chunk_ref_t *hdr)
{
  fprintf(stream, "data_type = %d\n", hdr->data_type);
  
  fprintf(stream, "valid_time = %s\n", utimstr(hdr->valid_time));
  fprintf(stream, "expire_time = %s\n", utimstr(hdr->expire_time));
  
  fprintf(stream, "offset = %d\n", hdr->offset);
  
  fprintf(stream, "len = %d\n", hdr->len);
  
  return;
}

/*****************************************************************
 * PRINT_STATION_REPORTS : Print the station reports received from
 *                         the server.
 */

static void print_station_reports(FILE *stream, int nchunks,
				  spdb_chunk_ref_t *hdrs,
				  void *data)
{
  int i;
  station_report_t report;
  
  for (i = 0; i < nchunks; i++)
  {
    report = *(station_report_t *)((char *)data + hdrs[i].offset);
    
    /*
     * Swap the report data
     */

    station_report_from_be(&report);

    /*
     * Now print the fields.
     */

    fprintf(stream, "\n");
    fprintf(stream, "Data for station report %d:\n", i);

    print_station_report(stream, "   ", &report);
    
    fprintf(stream, "\n");
  }
  
  return;
}

/*****************************************************************
 * PRINT_HISTORY_FORECAST_DATA : Print the history forecast data
 *                               received from the server.
 */

static void print_history_forecast_data(FILE *stream, int nchunks,
				  spdb_chunk_ref_t *hdrs,
				  void *data)
{   
   int ier;
   fprintf(stream, "\n");
   fprintf(stream, " Will print history_forecast chunk\n");
   fprintf(stream, " nchunks is %d \n",nchunks);
   fprintf(stream, " chunk length is %d \n",hdrs->len);
   ier = hf_chunk_native((hf_chunk_t **) &data);
   print_hf_chunk(stream, data,hdrs->len);
   fprintf(stream, "\nDone printing\n");
   fprintf(stream, "\n");
}
