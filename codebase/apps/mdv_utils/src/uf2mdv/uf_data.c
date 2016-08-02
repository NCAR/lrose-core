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
#include <ctetwws/smem.h>
#include <input.h>
#include <input_data.h>
#include <uf_data.h>

/* max size of buffers for uf data */
#define UF_MAXBUF 64000

static int fd;
static char temp_buf[UF_MAXBUF];  // the data buffer
static int current_pos=0;         // 0th byte in temp_buf for writing
static int match1 = -1;           // position of start of next available 
                                  // uf buffer in temp_buf.
static int current_rec;

static int file_number = -1;/* set index to before 0th file*/

/*------------------------------------------------------------------------ */
static input_data_t *fb_read(void)
{
    /* 
     * fortran-binary read routine
     */
    long int size_rec=0, rlen1, rlen2=0;
    long int size_rec2;
    long data_bytes;
    input_data_t *d;
    char *temp;

    /* For bad returns...*/
    d = NULL;

    /* read the record size */
    rlen1 = read (fd, &size_rec, sizeof(size_rec));
    if( rlen1 < sizeof(size_rec))
    {
	if (rlen1 != 0)
	    printf("ERROR reading rec size %d bytes gotten\n", rlen1);
	return d;
    }
    byte_swap((unsigned char *)&size_rec, rlen1);
    if( size_rec <= 0 )
    {
	printf("%d bytes called for, which is wierd.\n", size_rec);
	rlen1 = read (fd, &size_rec, sizeof(size_rec));
	if( rlen1 < sizeof(size_rec))
	{
	    if (rlen1 != 0)
		printf("ERROR reading rec size %d bytes gotten\n", rlen1);
	    return d;
	}
	byte_swap((unsigned char *)&size_rec, rlen1);
	if (size_rec > 0)
	    printf("NOW I'm REALLY confused %d\n", size_rec);
	return d;
    }

    /*
     * read the record
     * (the read may be less than the size of the record)
     */
    if (size_rec <= UF_MAXBUF)
	rlen2 = size_rec;
    else
    {
	printf("BUFFER %d bytes not big enough wanted %d truncate\n",
	       UF_MAXBUF, size_rec);
	rlen2 = UF_MAXBUF;
    }

    temp = MEM_CALLOC(rlen2, char);
    data_bytes = read (fd, temp, rlen2);	/* read it! */
    if( data_bytes < rlen2)
    {
	printf("ERROR read failed asked for %d got %d\n", rlen2, data_bytes);
	MEM_FREE(temp);
	return d;
    }
    
    /*
     * Decide how much more to skip.
     */
    if (data_bytes < size_rec)
	rlen2 = lseek(fd, size_rec-data_bytes, 1);
    
    /*
     * Doublecheck stuff.
     */
    rlen1 = read (fd, &size_rec2, sizeof(size_rec2));
    if( rlen1 < sizeof(size_rec2))
    {
	if (rlen1 != 0)
	    printf("ERROR reading rec size %d bytes gotten\n", rlen1);
	MEM_FREE(temp);
	return d;
    }
    byte_swap((unsigned char *)&size_rec2, rlen1);
    if( size_rec2 != size_rec)
    {
	printf("inconsistant %d bytes  vs %d\n", size_rec, size_rec2);
	MEM_FREE(temp);
	return d;
    }
    if (data_bytes == 0)
	printf("ZERO BYTES READ\n");

    /*
     * Good return
     */
    d = MEM_CALLOC(1, input_data_t);
    d->nbytes = data_bytes;
    d->data = temp;
    uf_byteswap(d->data, d->nbytes);
    return d;
}

/*----------------------------------------------------------------*/
// return true if input is start of a UF buffer.
static int is_uf(char *c)
{
    short *shortData;
   
    if (*c == 'U' && *(c+1) == 'F')
    {
        //
        // Check to see that the third two byte word
        // is the length of the mandatory header + 1
        // which is 46. If the third word is not 46, then
        // The UF found in the data stream is merely a
        // a data sequence. 
        // 
        shortData = (short*)(c+4);  
	if (*shortData == 46)
	    return 1;
	else
	    return 0;
    }
    else
        return 0;
}

/*----------------------------------------------------------------*/
// return position of 'UF' past match1 up to current_pos.
// -1 for none.
static int find_next_match(void)
{
    char *c;
    int i;
    
    for (i=match1+2, c=&temp_buf[match1+2]; i<current_pos; ++c, ++i)
    {
        if (is_uf(c) == 1)
	    return i;
    }
    return -1;
}

/*----------------------------------------------------------------*/
// return position of 0th and 1st 'UF' in temp_buf up to current_pos.
// false if match1 never found.
static int find_match(int *match2)
{
    char *c;
    int i;
    
    match1 = -1;
    *match2 = -1;
    for (i=0, c=temp_buf; i<current_pos; ++c, ++i)
    {
        if (is_uf(c) == 1)
	{
	    if (match1 == -1)
	        match1 = i;
	    else
	    {
		*match2 = i;
		return 1;
	    }
	}
    }
    if (match1 == -1)
        return 0;
    else
        return 1;
}


/*----------------------------------------------------------------*/
// the data from match1 to current_pos in temp_buf is a uf buffer.
static input_data_t *fill_input_data_t(int p0, int p1)
{
    input_data_t *d;
    int rlen;
    
    d = MEM_ALLOC(input_data_t);
    rlen = p1 - p0;
    d->data = MEM_CALLOC(rlen, char);
    memcpy(d->data, &temp_buf[p0], rlen);
    d->nbytes = rlen;
    return d;
}

/*----------------------------------------------------------------*/
// rlen bytes have been read into temp buf starting at current_pos
// match1 = starting position of next available UF buffer in temp_buf.
// fill in input_data_t and adjust current_pos and match1
static input_data_t *process_incomplete_buf(int rlen)

{
    int match2;
    input_data_t *d;

    // adjust current_pos
    current_pos += rlen;

    if (match1 >= current_pos)
        // no more to do.
        return NULL;
    
    if (match1 != -1)
        // what we have is some number of UF beams in temp, starting
        // at match1.
        match2 = find_next_match();
    else
    {
        // temp has some UF beams, but no known starting point.
        if (find_match(&match2) == 0)
	    // no u.f. data in temp_buf at all.
	    return NULL;
    }
    
    // here match1 is set, and match2 may or may not be set.
    if (match2 == -1)
    {
        // expect what is in temp is the final UF data.
        d = fill_input_data_t(match1, current_pos);
	match1 = current_pos;
    }
    else
    {
        d = fill_input_data_t(match1, match2);
	match1 = match2;
    }
    return d;
}

/*----------------------------------------------------------------*/
// rlen bytes have been read into temp buf starting at current_pos
// match1 = starting position of next available UF buffer in temp_buf.
// fill in input_data_t and adjust current_pos and match1
static input_data_t *process_buf(int rlen)
				 
{
    int match2;
    input_data_t *d;
    
    current_pos += rlen;
    
    if (match1 != -1)
        // what we have is some number of UF beams in temp, starting
        // at match1.
        match2 = find_next_match();
    else
    {
        // temp has some UF beams, but no known starting point.
        if (find_match(&match2) == 0)
{
	    // no u.f. data in temp_buf at all...problematic.
	    printf("ERROR reading uf buffer, no 'uf'\n");
	    return NULL;
	}
	
    }
    
    // here match1 is set, and match2 may or may not be set.
    if (match2 == -1)
    {
        // expect what is in temp is exactly 1 UF buffer.
        d = fill_input_data_t(match1, current_pos);
	current_pos = 0;
	match1 = -1;
    }
    else
    {
        int i, ii;
      
        d = fill_input_data_t(match1, match2);
	// copy data from match1 position back to 0th position.
	for (i=match2,ii=0; i<UF_MAXBUF; ++i,++ii)
	    temp_buf[ii] = temp_buf[i];
	current_pos = ii;
	match1 = 0;
    }
    
    return d;
}

/*----------------------------------------------------------------*/
static int fill_buf(char *buf, int max, int *num)
{
  
    *num = read (fd, buf, max);
    if (*num < max)
        return 0;
    else
        return 1;
}

/*----------------------------------------------------------------*/
// normal binary file read, just the data one beam after another,
// no seperators.
static input_data_t *vax_read(void)
{
    int rlen;
    
    // byte swap not easy for this data!
    if (params.byte_swap)
    {
        if (byteswap_needed() == 1)
	{
	    printf("non-fortran binary read not implemented with byteswapping\n");
	    printf("If byteswapping needed, uf read may be impossible\n");
	    return NULL;
	}
    }
    
    // read into temp buf as much as possible.
    if (fill_buf(&temp_buf[current_pos], UF_MAXBUF-current_pos, &rlen) == 0)
        return process_incomplete_buf(rlen);
    else
        return process_buf(rlen);
}

/*----------------------------------------------------------------*/
static input_data_t *uf_read(void)
{
    /*
     * Decide whether it's "fortran binary" or just data
     */
    if (params.input_data_format == UF_FORTRAN_BINARY)
	return (fb_read());
    else if (params.input_data_format == UF_PURE_DATA)
	return (vax_read());
    else
	return NULL;
}

/*----------------------------------------------------------------*/
void uf_data_init(void)
{
    fd = -1;
    uf_data_rewind();
}

/*----------------------------------------------------------------*/
void uf_data_rewind(void)
{
    current_pos = 0;  /* available at position 0 */
    match1 = -1;      /* no previous uf */
    current_rec = -1; /* most recently returned record from list.*/
    file_number = -1; /* set index to before 0th file*/

    if (fd >= 0)
	close(fd);
    fd = -1;
}

/*----------------------------------------------------------------*/
/*
 * Return pointer to next data in currently opened file, or NULL.
 */
input_data_t *uf_data_get_next(void)
{
    input_data_t *uf;

    if (fd < 0)
	return NULL;
   
    /*
     * Read the data into local storage.
     */
    uf = uf_read();
    if (uf == NULL)
	return uf;

    uf_get_header_values(uf->data, &uf->init_state.azimuth,
			 &uf->init_state.elevation, &uf->init_state.sweep);
    uf->init_state.wanted = 1;
    uf->init_state.prf = 1;
    uf->init_state.range = 1;
    uf->init_state.time = 1;
    uf->init_state.file = file_number;
    uf->init_state.record = ++current_rec;
    uf->init_state.volume = -1;
    uf->init_state.az_index = -1;
    uf->init_state.elev_index = -1;
    uf->init_state.sticky_azimuth = 0;
    return uf;
}

/*----------------------------------------------------------------*/
/*
 * Close  currently opened file, and open the next file
 * Return 1 or 0
 */ 
int uf_data_open_next_file(void)
{
    if (fd >= 0)
	close(fd);
    fd = -1;
    
    if (++file_number >= params.input_file.len)
	return 0;

    fd = open(params.input_file.val[file_number], O_RDONLY);
    if (fd < 0)
    {
	printf("ERROR opening input file %s\n", 
	       params.input_file.val[file_number]);
	return 0;
    }
    return 1;
}

