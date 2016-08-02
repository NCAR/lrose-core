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
/****************************************************************************
 * tape_input.c - tape read module
 *
 *      this routine reads tape data, seqments the data into logical records,
 *      and divides each record into simulated ethernet packets.  The 
 *	justification for this methodology is to provide a mechanism for 
 *	testing modifications made to the formatter without requiring the radar
 *	to be on line.  
 *
 * RAP, NCAR, Boulder CO
 *
 * Feb 1991
 *
 * Gary Blackburn
 *
 ****************************************************************************/

#include "ridds2mom.h"
#include <dataport/bigend.h>
#include <toolsa/ttape.h>
#include <toolsa/sockutil.h>
using namespace std;

#define MAX_TAPE_READ 65536

/*
 * file scope
 */

static int Debug = FALSE;
static int Remote_tape;
static int Tape_fd = -1;
static int Wait_usecs;
static int Init_done = FALSE;

static int read_logical_short (ui08 **logical_rec);
static int read_logical (ui08 **logical_rec);
static int get_physical_rec (ui08 *buffer);

/**************************************************************************
 * open_input_tape()
 *
 * Open tape device, wait until ready
 */
  
int open_input_tape(char *device, int wait_msecs, int debug)

{

  Debug = debug;
  Wait_usecs = wait_msecs * 1000;

  if (strchr(device, ':') == NULL) {

    Remote_tape = FALSE;
    if ((Tape_fd = open (device, O_RDONLY)) < 0) {
      perror (device);
      return (-1);
    }
    TTAPE_set_var(Tape_fd);

    if (Debug) {
      fprintf (stderr, "Successfully opened tape %s, fd = %d\n",
	       device, Tape_fd);
    }
    
  } else {

    Remote_tape = TRUE;
    if (RMT_open (device, O_RDONLY, 0666) < 0) {
      perror (device);
      return (-1);
    }
    
    if (Debug) {
      fprintf (stderr, "Successfully opened remote tape %s\n",
	       device);
    }
    
  }

  Init_done = TRUE;
  return (0);

}

/******************************************************************
 * Read a tape record. Checks for the correct record type. If
 * correct, setns number of bytes read and returns. Otherwise
 * keeps reading for good record.
 *
 * Returns:
 *   0 on success, -1 on error.
 *
 */

int read_input_tape (ui08 **buf_p, int *len_p)
     
{
     
  ui08 *buffer;

  int forever = TRUE;
  int log_rec_size;
  int offset;
  RIDDS_id_rec *id_rec;
  RIDDS_vol_title *vol_title;
  RIDDS_msg_hdr *msg_hdr;

  if (!Init_done) {
    fprintf(stderr, "ERROR - read_input_tape - init not done\n");
    return (-1);
  }

  while (forever) {

    if (Glob->params.short_tape) {
      
      if ((log_rec_size = read_logical_short (&buffer)) <= 0) {
	return (-1);
      }
      uusleep(Wait_usecs);

      *buf_p = buffer;
      *len_p = log_rec_size;
      return (0);

    } else {

      if ((log_rec_size = read_logical (&buffer)) <= 0) {
	return (-1);
      }
      uusleep(Wait_usecs);

      /*
       * deduce the type of record
       */
    
      if (log_rec_size == sizeof (RIDDS_id_rec)) {
      
	if (Debug) {
	  id_rec = (RIDDS_id_rec *) buffer;
	  fprintf (stderr, "RIDDS id %s\n\n\n", id_rec->filename);
	}

      } else if (log_rec_size == sizeof (RIDDS_vol_title)) {

	if (Debug) {
	  vol_title = (RIDDS_vol_title *) buffer;
	  fprintf(stderr, "\n****************************************\n\n");
	  fprintf(stderr, "volume title\n");
	  fprintf(stderr, "	- %s.%s\n\n",
		  vol_title->filename, 
		  vol_title->extension);
	}
	
      } else if (log_rec_size == NEX_PACKET_SIZE) {
	
	msg_hdr =  (RIDDS_msg_hdr *) (buffer + sizeof (RIDDS_ctm_info));
	
	if (msg_hdr->message_type == DIGITAL_RADAR_DATA) {
	  offset = sizeof(RIDDS_ctm_info) + sizeof(RIDDS_msg_hdr);
	  *buf_p = buffer + offset;
	  *len_p = log_rec_size - offset;
	  return (0);
	} 
	
      } else {
	
	if (Debug) {
	  fprintf(stderr, "Unknown tape record size %d\n", log_rec_size);
	}
      }
      
    } /* if (rap_tape) */

  } /* while (forever) */
    
  return (-1); /* suppress compiler warning */
    
}

/******************************************************************
 * close_input_tape()
 *
 */

void close_input_tape(void)

{
  if (Init_done) {
    if (Tape_fd >= 0) {
      close(Tape_fd);
      Tape_fd = -1;
      if (Debug) {
	fprintf(stderr, "Closing tape\n");
      }
    }
  }
}

/*-------------------------------------------------------------------------*/

/*
 * read_logical: reads a logical record from tape and returns its 
 * size and a pointer to it.
 *
 * returns number of bytes in record, -1 on error.
 */

static int read_logical (ui08 **logical_rec)

{

  static ui08 physical_rec[MAX_TAPE_READ];
  static int offset = 0;   /* Start of a logical record */ 
  int log_rec_size, nread;
  static int left = 0;     /* Portion of a physical record
			    * yet to be processed */
  static int firstime = TRUE; /* nexrad tapes contain an
			       * initial 8 byte id */
  int counter = 0;
  RIDDS_vol_title *vol_title;

  PMU_auto_register("Reading tape record");

  if (left == 0) {
    nread = get_physical_rec (physical_rec);
    if (nread < 0) {
      return (-1);
    }
    if (nread == 0) {
      if (firstime) {
	/* at the beginning nexrad tapes sometimes have 0 sized records */
	while (counter < 5) {
	  nread = get_physical_rec (physical_rec);
	  if (nread > 0)
	    break;
	  counter++;
	}
      } else {
	nread = get_physical_rec (physical_rec);
	if (nread < 0) {
	  return (-1);
	}
      }
    }
    
    offset = 0;
    left = nread;
  }

  if (left == 0) {
    if (Debug) {
      fprintf(stderr, "End of tape\n");
    }
    return (left);
  }

  if (left < 0) {
    fprintf(stderr, "tape read error = %d\n", nread);
    *logical_rec = (ui08 *) 0;
    log_rec_size = left;
  } else {
    *logical_rec = physical_rec + offset;
    if (firstime && nread == sizeof(RIDDS_id_rec)) {
      log_rec_size = nread;
    } else {
      /* determine which type of record is being read */
      vol_title = (RIDDS_vol_title *) *logical_rec;
      if (strncmp (vol_title->filename, "ARCHIVE2", 8) == 0) {
	log_rec_size = sizeof (RIDDS_vol_title);
      } else {
	log_rec_size =  NEX_PACKET_SIZE;
      }
    }

    offset += log_rec_size;
    left -= log_rec_size;

    if (left < log_rec_size) {
      left = 0;
    }

  }

  firstime = FALSE;
  return (log_rec_size);	

}

/*-------------------------------------------------------------------------*/

/*
 * read_logical_short: reads a logical record from tape recorded with
 * short blocks.
 *
 * returns number of bytes in record, -1 on error.
 */

static int read_logical_short (ui08 **logical_rec)

{

  static ui08 physical_rec[MAX_TAPE_READ];
  static int offset = 0;   /* Start of a logical record */ 
  static int left = 0;     /* Portion of a physical record
			    * yet to be processed */
  int nread;

  PMU_auto_register("Reading RAP tape record");

  if (left < NEX_RAP_TAPE_REC_SIZE) {

    offset = 0;
    nread = get_physical_rec (physical_rec);

    if (nread < 0) {
      return (-1);
    } else if (nread < NEX_RAP_TAPE_REC_SIZE) {
      if (Debug) {
	fprintf(stderr,
		"Not enough bytes in read - read %d bytes\n", nread);
      }
      return (0);
    }

    left = nread;

  }

  *logical_rec = physical_rec + offset;
  offset += NEX_RAP_TAPE_REC_SIZE;
  left -= NEX_RAP_TAPE_REC_SIZE;

  return (NEX_RAP_TAPE_REC_SIZE);	

}

/*------------------------------------------------------------------------
 * get_physical_rec()
 *
 * Gets a physical tape record.
 *
 * If successful, returns number of bytes read.
 * If failure, returns -1.
 */


static int get_physical_rec (ui08 *buffer)
     
{

  int nread;
  int nerr;

  PMU_auto_register("Reading physical tape record");

  if (Remote_tape) {

    return (RMT_read ((char *) buffer, MAX_TAPE_READ));

  } else {

    PMU_auto_register("Reading tape");

    nerr = 0;
    while (nerr < 50) {

      errno = EINTR;
      while (errno == EINTR) {
	errno = 0;
	nread = read (Tape_fd, (char *) buffer, MAX_TAPE_READ);
      }
      
      if (nread > 0) {
	return (nread);
      } else {
	nerr++;
      }

      uusleep(1000);

    }

    fprintf(stderr, "Tape not ready, or no data left\n");
    return (-1);

  }

}



