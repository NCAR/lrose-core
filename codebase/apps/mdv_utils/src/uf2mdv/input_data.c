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
#include <stdio.h>
#include <sys/file.h>
#include <input.h>
#include <input_data.h>
#include <uf_data.h>
#include <ctetwws/smem.h>

/*
 * 1 zero length recod in a row implies sweep complete
 * 2 zero length records in a row implies end of data
 */
#define EOF_SWEEP_COMPLETE 0
#define EOF_DONE 1
#define EOF_NO_FLAG 2

static int zero = 0;  /* number of zero length records in a row */
static int sweep_number = 0;
static int record_count = 0;
/*----------------------------------------------------------------*/
/*
 * Return flag value based on inputs
 */
static int eof_adjust(input_data_t *d)
{
    int eof;

    eof = 0;
    if (d == NULL)
        eof = 1;
    else
    {
	if (d->nbytes <= 0)
	    eof = 1;
    }
    if (eof == 1)
    {
	if (params.debug_level >= 3)
	    printf("flag:  EOF (zero bytes) read from the file\n");
	zero++;
    }
    else
	zero = 0;
	
    if (zero == 1)
    {
	if (params.debug_level >= 3)
	    printf("flag:  setting sweep complete flag\n");
	return EOF_SWEEP_COMPLETE; 
    }       
    if (zero >= 2)
    {
	if (params.debug_level >= 3)
	    printf("flag:  2 zero byte reads..end of file\n");
	return EOF_DONE;
    }
    if (params.debug_level >= 10)
	printf("flag:  normal read\n");
    return EOF_NO_FLAG;
}

/*----------------------------------------------------------------*/
void input_data_init(void)
{
    /* init the state values */
    zero = 0;  /* number of zero length records in a row */
    sweep_number = 0;
    uf_data_init();
    record_count = 0;
}

/*----------------------------------------------------------------*/
void input_data_rewind(void)
{
    zero = 0;  /* number of zero length records in a row */
    sweep_number = 0;
    uf_data_rewind();
    record_count = 0;
}

/*----------------------------------------------------------------*/
/*
 * Return pointer to next data, or NULL.
 */
input_data_t *input_data_get_next(void)
{
    input_data_t *d;
    int flag;

    for (;;)
    {
	/*
	 * Read the data into local storage.
	 */
        d = uf_data_get_next();
	flag = eof_adjust(d);
	switch (flag)
	{
	case EOF_SWEEP_COMPLETE:
	    /*
	     * Finished a sweep due to info found in file.
	     */
	    ++sweep_number;
	    break;
	case EOF_DONE:
	    /*
	     * Done with current file
	     */
	    if (uf_data_open_next_file()== 0)
		/*
		 * NO more files
		 */
  	        return NULL;
	    break;
	case EOF_NO_FLAG:
	    /*
	     * This data is good to return as is
	     * Make an object to return
	     */
   	    d->init_state.sweep = sweep_number;
	    ++record_count;
	    return d;
	default:
	    /*
	     * ERROR
	     */
	    printf("ERROR bad flag %d\n", flag);
	    return NULL;
	}
    }
}

/*----------------------------------------------------------------*/
void input_data_free(input_data_t *input)
{
    MEM_FREE(input->data);
    MEM_FREE(input);
}

/*----------------------------------------------------------------*/
/*
 * Browse input data.
 */
void input_browse_data(input_data_t *uf)
{
    switch (params.browse_level)
    {
    case BROWSE_1LINE:
	ufdump(uf, 1);
	break;
    case BROWSE_MULTILINE:
	ufdump(uf, 0);
	break;
    case BROWSE_MULTILINE_AND_DATA:
	ufdump_all(uf);
	break;
    case BROWSE_NONE:
    default:
	break;
    }
}

/*----------------------------------------------------------------*/
int input_data_num_records(void)
{
    return record_count;
}
