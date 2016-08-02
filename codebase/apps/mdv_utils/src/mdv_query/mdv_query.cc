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
 *  MDV_QUERY.CC Query program for the MDV data servers 
 */

#include <ctype.h>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <sys/param.h>

#include <toolsa/os_config.h>
#include <mdv/mdv_client.h>
#include <mdv/mdv_handle.h>
#include <mdv/mdv_print.h>
#include <toolsa/port.h>
#include <toolsa/str.h>
using namespace std;

#define MAX_INPUT_LEN     1024
#define SERVICE_NAME_LEN    64


/************** STATIC FUNCTION PROTOTYPES **************/

static void signal_trap(int sig);

static char get_main_command(void);

static char get_retrieve_command(void);

static void send_get_command(int cmd_type);

static MDV_handle_t *request_data_get_closest(char *source);


/************** GLOBAL DATA *****************************/

char	*SourceString;

/******************************************************************************
 * MAIN
 *
 */

int main(int argc, char *argv[])
{
  int done = FALSE;
  
  if (argc < 2)
  {
    fprintf(stderr,"Standard Usage: mdv_query source_string\n"); 
    exit(0);
  }

  SourceString  = argv[1];

  PORTsignal(SIGINT, signal_trap);
  PORTsignal(SIGTERM, signal_trap);

  while (!done)
  {
    switch(get_main_command())
    {
    case 'c' :
      send_get_command(MDV_GET_CLOSEST);
      break;
      
    case 'b' :
      send_get_command(MDV_GET_FIRST_BEFORE);
      break;
      
    case 'a' :
      send_get_command(MDV_GET_FIRST_AFTER);
      break;
      
    case 'n' :
      send_get_command(MDV_GET_NEW);
      break;
      
    case 'l' :
      send_get_command(MDV_GET_LATEST);
      break;
      
    case 'q' :
      done = TRUE;
      break;
      
    default:
      fprintf(stdout, "\n");
      fprintf(stdout, "Invalid command -- try again\n");
      fprintf(stdout, "\n");
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
  fprintf(stdout, "   C = get Closest data\n");
  fprintf(stdout, "   B = get first data Before\n");
  fprintf(stdout, "   A = get first data After\n");
  fprintf(stdout, "   N = get New data\n");
  fprintf(stdout, "   L = get Latest data\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "   Q = Quit\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Desired command: ");
  
  fgets(command_string, MAX_INPUT_LEN, stdin);
  
  return(tolower(command_string[0]));
}
 
/*****************************************************************
 * GET_RETRIEVE_COMMAND : Get the desired data retrieval command
 *                        from the user.
 */

static char get_retrieve_command(void)
{
  char command_string[MAX_INPUT_LEN];
  
  fprintf(stdout, "Choose a command:\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "   C = get Closest data\n");
  fprintf(stdout, "   B = get first data Before\n");
  fprintf(stdout, "   A = get first data After\n");
  fprintf(stdout, "   N = get New data\n");
  fprintf(stdout, "   L = get Latest data\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "   Q = Quit\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "Desired command: ");
  
  fgets(command_string, MAX_INPUT_LEN, stdin);
  
  return(tolower(command_string[0]));
}
 
/*****************************************************************
 * SEND_GET_COMMAND : Send the indicated get command to the server
 *                    and see what we get back.
 */

static void send_get_command(int cmd_type)
{
  MDV_handle_t *mdv_handle;
  
  /*
   * Get the data.
   */

  switch (cmd_type)
  {
  case MDV_GET_CLOSEST :
    mdv_handle = request_data_get_closest(SourceString);
    break;
    
  case MDV_GET_FIRST_BEFORE :
    fprintf(stdout, "*** Get first before not yet implemented\n");
    break;
    
  case MDV_GET_FIRST_AFTER :
    fprintf(stdout, "*** Get first after not yet implemented\n");
    break;
    
  case MDV_GET_LATEST :
    fprintf(stdout, "*** Get latest not yet implemented\n");
    break;
    
  case MDV_GET_NEW :
    fprintf(stdout, "*** Get new not yet implemented\n");
    break;
    
  } /* endswitch - cmd_type */
  
  /*
   * Print the reply.
   */

  if (mdv_handle != (MDV_handle_t *)NULL)
  {
    MDV_print_master_header_full(&mdv_handle->master_hdr, stdout);
    
    for (int ifield = 0; ifield < mdv_handle->master_hdr.n_fields; ifield++)
      MDV_print_field_header_full(&mdv_handle->fld_hdrs[ifield], stdout);
    
    if (mdv_handle->master_hdr.vlevel_included)
    {
      for (int ifield = 0; ifield < mdv_handle->master_hdr.n_fields;
	   ifield++)
	MDV_print_vlevel_header_full(&mdv_handle->vlv_hdrs[ifield],
				     mdv_handle->fld_hdrs[ifield].nz,
				     mdv_handle->fld_hdrs[ifield].field_name_long,
				     stdout);
    }
    
    for (int ichunk = 0; ichunk < mdv_handle->master_hdr.n_chunks; ichunk++)
    {
      MDV_print_chunk_header_full(&mdv_handle->chunk_hdrs[ichunk], stdout);
      MDV_print_chunk_data_full(mdv_handle->chunk_data[ichunk],
				mdv_handle->chunk_hdrs[ichunk].chunk_id,
				mdv_handle->chunk_hdrs[ichunk].size,
				stdout);
    }
    
    fprintf(stdout, "\n\n");
  }
  
  return;
}


/*****************************************************************
 * REQUEST_DATA_GET_CLOSEST : Request data from the server using a
 *                            get closest command.
 *
 * Returns the MDV handle on success, NULL on error.
 */

static MDV_handle_t *request_data_get_closest(char *source)
{
  char input_string[MAX_INPUT_LEN];
  date_time_t request_time_struct;
  si32 request_time;
  
  si32 time_margin;
  
  MDV_handle_t *mdv_handle;
  
  /*
   * Get the request time from the user
   */

  fprintf(stdout, "\n");
  fprintf(stdout, "Enter request time (dd/mm/yy hh:mm:ss, RET for now): ");
  
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

      return((MDV_handle_t *)NULL);
    }
    
    if (request_time_struct.year < 1900)
      request_time_struct.year += 1900;
    
    request_time = uconvert_to_utime(&request_time_struct);
  }
  
  /*
   * Get the time margin from the user.
   */

  fprintf(stdout, "Enter time margin (secs), RET for unlimited: ");
  fgets(input_string, MAX_INPUT_LEN, stdin);
  
  if (STRequal_exact(input_string, "\n"))
    time_margin = -1;
  else
    time_margin = atoi(input_string);
  
  /*
   * Try to get the data.
   */

  if (MDV_get_closest(source,
		      request_time,
		      time_margin,
		      &mdv_handle) != 0)
  {
    fprintf(stdout, "\n");
    fprintf(stdout, "ERROR - SPDB_get_closest failed on source <%s>\n",
	    source);
    fprintf(stdout, "\n");
    return((MDV_handle_t *)NULL);
  }

  return(mdv_handle);
}
