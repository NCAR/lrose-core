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
/************************************************************************
*                                                                       *
*                              sio_index_file.cc
*                                                                       *
*************************************************************************

                         Thu Mar 16 14:50:08 1995

       Description:      Index file associated with "sio" data files.

       See Also:         ascii_shapeio.h, shapeio.c sio_util.c

       Author:           Dave Albo

       Contents:         SIO_rebuild_named_index_file(),
                          SIO_rebuild_index_file()

       Static Functions: init_index_list_to_empty(), index_file_name(),
                         touch_index_file(), file_stat(), write_index_record(),
                         write_index_data_len_record(), 
			 read_index_data_len_record(), read_index_record(),
			 write_new_index_file(), read_index_file(), 
			 index_data_status(), find_existing_index(), 
			 append_to_index_list(), setup_index_list(),
			 create_init(), create_build_index_list(), 
			 create_index(), check_data_file_pointers(),
			 append_to_index_file(), is_data_append(), 
			 rebuild_ifile_sub()
			
*/

/*
 * System include files
 */


#include <cstdlib>
#include <cstdio>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

/*
 * Local include files
 */

#include <rapformats/ascii_shapeio.h>
#include <toolsa/mem.h>
#include <toolsa/file_io.h>

#include "shapeio_int.h"
using namespace std;

/*
 * Definitions / macros / types
 */

/*
 * A list of SIO_index_data_t lists consists of an array of:
 */

typedef struct sio_index_list 
{
  char *index_file_name;   /* the index file name */
  int file_bytes;          /* size of data file (bytes)*/
  int nindex;              /* # of things in the list*/
  SIO_index_data_t *Ibase; /* index data array */
} SIO_index_list;


/*
 * External references / global variables 
 */

static SIO_index_list *Ilist = NULL;  /* the full index list list*/
static int Ilist_len = 0;             /* # of elements in current list*/
static int Debug = 1;
static char Data_file_name[200];
static char Index_file_name[200];
/* static char Open_app[1] = {'a'}; */
static char Open_read[1] = {'r'};
/* static char Open_write[1] = {'w'}; */

/*----------------------------------------------------------------*
 *
 * STATIC FUNCTIONS
 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*
 * Little routine to clear a list element to "empty", ready to
 * put in one data element to it.
 */

static void init_index_list_to_empty(SIO_index_list *L)
{
  if (L->Ibase != NULL)
    ufree(L->Ibase);
  L->Ibase = (SIO_index_data_t *)ucalloc(1, sizeof(SIO_index_data_t));
  L->nindex = 0;
  L->file_bytes = 0;
  L->index_file_name = 0;
}

/*----------------------------------------------------------------*/
/*
 * Form the index file name from inputs.
 */

static char *index_file_name(const char *directory, /* where */
			     const char *suffix,    /* what */
			     time_t time)     /* when */
{
  char *c;
    
  /*
   * Form the name of the data file.
   */

  c = SIO_file_name_full(directory, suffix, time, 0);

  /*
   * Append "_index"
   */

  strcat(c, "_index");

  /*
   * Thats it.
   */

  return c;
}

/*----------------------------------------------------------------*/
/*
 * TOuch the index file
 */

static void touch_index_file()
{
  char buf[300];
    
  sprintf(buf, "touch %s", Index_file_name);
  system(buf);
}

/*----------------------------------------------------------------*/
/*
 * Little routine..returns length of data file in bytes,
 *                 and time file last modified.
 */

static int file_stat(int is_data, int *nbytes, time_t *modtime) 
{
  char *c;
  struct stat bufd;
  int statd;
    
  if (is_data)
    c = Data_file_name;
  else
    c = Index_file_name;

  statd = ta_stat(c, &bufd);
  if (statd == -1)
  {
    if (is_data)
      printf("ERROR: Data file %s does not exist.\n", c);
    *nbytes = *modtime = 0;
    return 0;
  }

  *nbytes = (int)bufd.st_size;
  *modtime = bufd.st_mtime;

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Write one input index record to opened file.
 */

static void write_index_record(FILE *fp,
			       SIO_index_data_t *I) /* the record read in */
{
  fprintf(fp, "%d %d %d %s\n", (int)I->data_time, I->extrap_seconds,
	  I->file_index, I->type);
}

/*----------------------------------------------------------------*/

static void write_index_data_len_record(FILE *fp, SIO_index_list *L)
{
  fprintf(fp, "%d\n", L->file_bytes);
}

/*----------------------------------------------------------------*/
/*
 * read data length index record from opened file.
 * Return 1 if successful.
 */

static int read_index_data_len_record(FILE *fp,
				      SIO_index_list *L)
{
  if (fscanf(fp, "%d\n", &L->file_bytes) != 1)
    return 0;

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Read one index record from opened file.
 * Return 1 if was able to read properly.
 */

static int read_index_record(FILE *fp,
			     SIO_index_data_t *I) /* the record returned */
{
  char buf[20];
    
  if (fscanf(fp, "%ld %d %d %s\n", &I->data_time, &I->extrap_seconds,
	     &I->file_index, buf) == 4)
  {
    I->type = known_type_subtype_list(buf);
    return 1;
  }
  else
    return 0;
}

/*----------------------------------------------------------------*/
/*
 * Write entire input index data array to associated index file.
 */

static int write_new_index_file(SIO_index_list *L /* the list to write */)
{
  int i;
  FILE *fp;
    
  /*
   * Open the index file
   */

  fp = fopen(L->index_file_name, "w");
  if (fp == NULL)
  {
    /*
     * No luck
     */

    printf("ERROR opening index file %s for write\n", L->index_file_name);
    return 0;
  }    

  /*
   * Write special length record
   */

  write_index_data_len_record(fp, L);
    
  /*
   * Write each index record.
   */

  for (i=0; i<L->nindex; ++i)
    write_index_record(fp, &(L->Ibase[i]));

  /*
   * Close up & indicate good.
   */

  fclose(fp);

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Read in existing index file to build up index struct array.
 * Return 1 for o.k.
 */

static int read_index_file(SIO_index_list *L /* where to store the array*/)
{
  SIO_index_data_t *I;
  FILE *fd;
    
  /*
   * Open index file.
   */

  fd = fopen(L->index_file_name, "r");
  if (fd == NULL)
  {
    printf("ERROR opening %s for read\n", L->index_file_name);
    return 0;
  }
    
  /*
   * Read in index file, build up structs as we go.
   */

  init_index_list_to_empty(L);
  I = L->Ibase;

  /*
   * Start by reading the data file length..
   */

  if (read_index_data_len_record(fd, L) == 0)
  {
    printf("Error reading %s data len record\n", L->index_file_name);
    return 0;
  }

  while (read_index_record(fd, I) == 1)
  {
    ++ (L->nindex);
    L->Ibase = (SIO_index_data_t *)
      urealloc((char *)(L->Ibase),(L->nindex+1)*sizeof(SIO_index_data_t));
    I = &(L->Ibase[L->nindex]);
  }

  fclose(fd);

  if (L->nindex <= 0)
  {
    ufree(L->Ibase);
    L->Ibase = NULL;
  }

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Return status of index file compared to associated data file as follows:
 * -1   error of some sort in figuring out the status.
 *  0   current index file is good.
 *  1   need to make a new index file by reading entire data file.
 *  2   need to make a new index file maybe (or append to existing one)
 */

static int index_data_status(int  *data_bytes /* size of data file (ret)*/)
{
  time_t timed, timei;
  int index_bytes;
    
  /*
   * Get status of data file.
   */

  if (file_stat(1, data_bytes, &timed) != 1)
  {
    printf("ERROR no data file %s which invalidates index file status\n",
	   Data_file_name);
    *data_bytes = 0;
    return -1;
  }
    
  /*
   * Get index file status.
   */

  if (file_stat(0, &index_bytes, &timei) != 1)
  {
    /*
     * No index file.
     */

    if (Debug == 1)
      printf("Create index file..    there isn't one\n");
    return 1;
  }
  else
  {
    /*
     * Index file exists.
     */

    if (timei < timed)
    {
      /*
       * Index file older than data file..
       */

      if (Debug == 1)
	printf("remake index file (or append). data file newer\n");
      return 2;
    }
    else
      /*
       * Index file newer than data file.
       */
      return 0;
  }
}

/*----------------------------------------------------------------*/
/*
 * Look through the list of index lists and try to find a name
 * match..return pointer to matching element or NULL for none.
 */

static SIO_index_list *find_existing_index( )
{
  int i;

  /*
   * Start looking.
   */

  for (i = 0; i < Ilist_len; ++i)
  {
    if (strcmp(Index_file_name, Ilist[i].index_file_name) == 0)
      /*
       * Eureka.
       */
      return &Ilist[i];
  }

  return NULL;
}

/*----------------------------------------------------------------*/
/*
 * Append input index file specification as a new element in the 
 * SIO_index_list global array.  Initialize its values.
 *
 * Return pointer to new list element.
 */

static SIO_index_list *append_to_index_list()
{
  SIO_index_list *L;
  char *c;

  /*
   * Is this the first thing to put into the list?
   */

  if (Ilist == NULL)
  {
    /*
     * Yes it is..allocate space for exactly one element.
     */

    Ilist_len = 0;
    Ilist = (SIO_index_list *)ucalloc(1, sizeof(SIO_index_list));
  }
  else
    /*
     * Append by reallocating some more memory.
     */
    Ilist = (SIO_index_list *)
      urealloc((char *)Ilist, (Ilist_len+1)*sizeof(SIO_index_list));

  /*
   * Point to the newest thing, which becomes the last element in
   * a list of length Ilist_len.
   */

  L = &Ilist[Ilist_len++];
    
  /*
   * Form the name, then make a unique local copy of it to stuff
   * into L
   */

  c = Index_file_name;
  L->index_file_name = (char *)umalloc((strlen(c)+1)*sizeof(char));
  strcpy(L->index_file_name, c);
  
  /*
   * Init the other values to something reasonable.
   */

  L->nindex = 0;
  L->file_bytes = 0;
  L->Ibase = NULL;

  /*
   * Return that pointer.
   */

  return L;
}
    
/*----------------------------------------------------------------*/
/*
 * Return pointer to a SIO_index_list element that is ready for use,
 * based on the input specification of an index file, and knowledge of
 * need to rebuild the index file.
 */

static SIO_index_list *setup_index_list(int rebuild)
/* rebuild values:
 *   0  current index file is good
 *   1  need to make a new index file by reading entire data file
 *   2  need to either make a new index file or append to existing one
 */
{
  SIO_index_list *L;
    
  /*
   * Do we already have an element that matches?
   */

  L = find_existing_index();
  if (L != NULL)
  {
    /*
     * Yes..
     */

    if (rebuild == 1)
    {
      /*
       * Free the space associated with the list so as to force
       * rebuilding it.
       */

      ufree((char *)L->Ibase);
      L->Ibase = NULL;
      L->nindex = 0;
      L->file_bytes = 0;
    } // if (rebuild == 1)
  }
  else
  {
    /*
     * No entry yet..
     * Append a new entry to the list of index lists and use it.
     */

    L = append_to_index_list();
  } // if (L != NULL)

  return L;
}    

/*----------------------------------------------------------------*/
/*
 * Prepare to read in a data file to create an index file.
 * open the data file.
 * figure out position in data file.
 * Initialize the index_list struct for filling it in.
 * Return 1 if o.k.
 */

static int create_init(SIO_index_list *L, /* to be initialized on exit */
		       FILE **fp,         /* returned data file opened */
		       int *pos)          /* returned position in file*/
{
  /*
   * Form the data file name..and open it.
   */

  if ((*fp = SIO_open_named_data_file(Data_file_name, Open_read)) == NULL)
  {
    printf("ERROR opening data file to create index file\n");
    return 0;
  }

  /*
   * Clear out the current read buffer of anything that
   * might be in it, and set up re-building of Ibase.
   */

  SIO_clear_read_buf();
  init_index_list_to_empty(L);
    
  /*
   * Figure out where we are in file prior to starting (should
   * be beginning)
   */

  if ((*pos = ftell(*fp)) != 0)
  {
    printf("ERROR not at beginning of file..at %d\n", *pos);
    fclose(*fp);
    return 0;
  }

  return 1;
}

/*----------------------------------------------------------------*/

static int create_build_index_list(SIO_index_list *index_list)
{
  FILE *fp;
  SIO_shape_data_t shape;
  SIO_index_data_t *index;
  int pos, newpos;

  /*
   * Read in the file and fill in the index list.
   * Set up to start (open file).
   */

  if (create_init(index_list, &fp, &pos) == 0)
    return 0;

  /*
   * Initialize the shape structure before using it.
   */

  memset(&shape, 0, sizeof(shape));
  
  /*
   * Loop:  read a record from the file. (return position of
   * next record in the file, and the current data).
   */

  while (SIO_read_record(fp, &shape, &newpos) == 1)
  {
    /*
     * Got a record..Add in space for newest element in Ibase array.
     */

    index_list->Ibase =
      (SIO_index_data_t *)urealloc((char *)(index_list->Ibase),
				   ((index_list->nindex)+1) *
				   sizeof(SIO_index_data_t));

    /*
     * Copy in data.
     */

    index = &(index_list->Ibase[index_list->nindex]);
    index->data_time = shape.data_time;
    index->extrap_seconds = shape.P->nseconds;
    index->file_index = pos;
    index->type = shape.type;
	
    /*
     * Adjust variables for next time through loop.
     */

    pos = newpos;
    (index_list->nindex)++;
  }

  /*
   * Free space allocated within S, which is no longer needed.
   */

  SIO_free_shapes(&shape, 1);
	
  /*
   * Done reading data file.
   */

  fclose(fp);
  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Create new index struct array by reading in associated data file.
 * This is what is inefficient which is why I createed the index file.
 */

static int create_index(SIO_index_list *L /* to be filled in */ )
{
  int orig_len, final_len;
  time_t t;
  int counter;

  /*
   * File length right now is:...(might contain some growing records).
   */

  if (file_stat(1, &orig_len, &t) != 1)
    return 0;

  if (orig_len <= 0)
  {
    printf("Tried to create index file for empty data file..no can do\n");
    return 0;
  }

  counter = 0;
  while (1)
  {
    /*
     * Things getting out of hand?
     */

    if (++counter > 1000)
    {
      printf("ERROR looks like an infinite loop\n");
      printf("expect a data file that keeps growing and growing\n");
      return 0;
    }

    /*
     * Read the data file and fill in the index list with its
     * values.
     */

    if (create_build_index_list(L) == 0)
      return 0;

    /*
     * Get final file length after reading in all the data in the file
     */

    if (file_stat(1, &final_len, &t) == 0)
      return 0;

    if (orig_len > final_len)
    {
      printf("ERROR mysterious shrinking data file\n");
      return 0;
    }
    else if (orig_len < final_len)
      /*
       * The file grew since status last taken..
       * Reset orig_len and try to re-read it in..it grew.
       */
      orig_len = final_len;
    else
      /*
       * Same length before and after readin in the file.
       */
      break;
  }

  L->file_bytes = final_len;
  if (L->nindex == 0)
  {
    if (L->Ibase != NULL)
      ufree(L->Ibase);
    L->Ibase = NULL;
  }

  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Return 1 if all data file pointers point to places that are "nice."
 */

static int check_data_file_pointers(SIO_index_list *L)
{
  FILE *fd;
  int i;
    
  /*
   * Open the file.
   */

  fd = SIO_open_named_data_file(Data_file_name, Open_read);
  if (fd == NULL)
  {
    printf("ERROR opening the file..%s   BAD\n", Data_file_name);
    return 0;
  }	

  /*
   * For each index.
   */

  for (i = 0; i < L->nindex;++i)
  {
    /*
     * See if file aligned at alleged byte offset.
     */

    if (file_is_aligned(fd, &L->Ibase[i]) != 1)
    {
      /*
       * Nope
       */

      fclose(fd);
      return 0;
    }
  }
	
  /*
   * Its still aligned.
   */

  fclose(fd);
  return 1;
}

/*----------------------------------------------------------------*/
/*
 * Append new data to the existing index list.
 * Rewrite the index file.
 */

static int append_to_index_file(SIO_index_list *L, int data_bytes)
{
  FILE *fd;
  int pos, newpos, num_new;
  SIO_shape_data_t S;
  SIO_index_data_t *I;
    
  /*
   * Open data file.
   */

  if ((fd = SIO_open_named_data_file(Data_file_name, Open_read)) == NULL)
  {
    printf("ERROR opening data file %s to create index file\n",
	   Data_file_name);
    return 0;
  }

  /*
   * Clear out the current read buffer.
   */

  SIO_clear_read_buf();
    
  if (L->nindex > 0)
  {
    /*
     * SKip to previous last record in file..
     */

    if (fseek(fd, (long)L->Ibase[L->nindex-1].file_index, 0) != 0)
    {
      printf("ERROR seeking to %d to append to file\n",
	     L->Ibase[L->nindex-1].file_index);
      return 0;
    }
	
    /*
     * read in what should be the last record..returning
     * position of the start of the first new record.
     */

    memset( &S, 0, sizeof(S) );
    if (SIO_read_record(fd, &S, &newpos) != 1)
    {
      printf("ERROR reading previous last record in file\n");
      return 0;
    }
  }
  else
  {
    newpos = 0;
    init_index_list_to_empty(L);
  }
    
  /*
   * Now begin reading in records and appending them to the
   * index list.
   */

  num_new = 0;
  pos = newpos;
  while (SIO_read_record(fd, &S, &newpos) == 1)
  {
    ++num_new;
	
    /*
     * Got a record..Add in space for newest element in Ibase array.
     */

    L->Ibase = (SIO_index_data_t *)
      urealloc((char *)(L->Ibase),
	      ((L->nindex)+1)*sizeof(SIO_index_data_t));

    /*
     * Copy in data.
     */

    I = &(L->Ibase[L->nindex]);
    I->data_time = S.data_time;
    I->extrap_seconds = S.P->nseconds;
    I->file_index = pos;
    I->type = S.type;
	
    /*
     * Free space allocated within S, which is no longer needed.
     */

    SIO_free_shapes(&S, 1);

    /*
     * Adjust variables for next time through loop.
     */

    pos = newpos;
    (L->nindex)++;
  }

  /*
   * Done reading data file.
   */

  fclose(fd);
  if (Debug == 1)
    printf("%d new records found in data file\n", num_new);

  /*
   * Now ready to go, except should rewrite the index file
   * first.
   */

  L->file_bytes = data_bytes;

  return(write_new_index_file(L));
}

/*----------------------------------------------------------------*/
/*
 * Determine whether a data file has just been appended to.
 * If so,
 *    append new records to its index file and update index.
 *    return status = 1.
 * If not
 *    return status = 0.(need to reread entire file).
 *
 * On entry:The index file (as is) has been read into L.
 */

static int is_data_append(SIO_index_list *L, int data_bytes)
{
  if (L->file_bytes < data_bytes)
  {
    /*
     * File is bigger..it might have been 
     * appended to...first check the existing pointers
     */

    if (check_data_file_pointers(L) == 0)
    {
      /*
       * Nope..better do a complete re-read
       */

      if (Debug == 1)
	printf("Rebuild index...significant data file changes...\n");
      return 0;
    }
    else
    {
      /*
       * Looks like it was appended to.
       */

      if (Debug == 1)
	printf("....augment index due to data appended\n");
      return(append_to_index_file(L, data_bytes));
    }
  }
  else if (L->file_bytes == data_bytes)
  {
    /*
     * Maybe index file was just touched..
     * check out file pointers to build more confidence.
     */

    if (check_data_file_pointers(L) == 1)
    {
      /*
       * the file was just touched..well, touch the index file
       * too so this doesn't keep happening.
       */

      if (Debug == 1)
	printf("Touch index file ....\n");
      touch_index_file();
      return 1;
    }
    else
    {
      /*
       * The file IS different..reread it.
       */

      if (Debug == 1)
	printf("File more than just touched..its different\n");
      return 0;
    }
  }
  else
  {
    /*
     * Data file smaller than previously..
     * just give up and reread it.
     */

    if (Debug == 1)
      printf("Rebuild index file..data file has shrunk\n");
    return 0;
  }
}

/*----------------------------------------------------------------*/

static SIO_index_data_t *rebuild_ifile_sub(int *nindex)
{    
  int rebuild, data_bytes;
  SIO_index_list *L;

  /*
   * Check the file times in case of needing a rebuild.
   * Returned rebuild values:
   *   -1  error 
   *    0  current index file is good
   *    1  need to make a new index file by reading entire data file
   *    2  need to either make a new index file or append to existing one
   */

  if ((rebuild = index_data_status(&data_bytes)) == -1)
  {
    /*
     * Bad!
     */

    *nindex = 0;
    return NULL;
  } // if ((rebuild = index_data_status(&data_bytes)) == -1)
    
  /*
   * Prepare an entry on the list of index lists, maybe empty or new.
   */
  L = setup_index_list(rebuild);
  if (L->Ibase != NULL)
  {
    /*
     * The prepared entry has been filled in already.
     * (rebuild == 0)
     * The prepared entry needs to be examined some more
     * to see if it needs rebuilding
     * (rebuild == 2)
     */

    if (rebuild == 0)
    {
      *nindex = L->nindex;
      return L->Ibase;
    } // if (rebuild == 0)
  } // if (L->Ibase != NULL)
    
  /*
   * Need to fill in L with a list.
   */

  switch (rebuild)
  {
  case 2:
    /*
     * Need to either make a new index file or append to existing one.
     * See if this is a new index list.
     */

    if (L->Ibase == NULL)
      /*
       * Read in the CURRENT index file.
       * (which we've determined exists)
       */
      read_index_file(L);

    /*
     * Now compare file size, etc.
     */

    if (is_data_append(L, data_bytes) == 1)
      /*
       * Appended (hopefully) new data into the
       * index file and index record (L)
       */
      break;
	
    /*
     * Otherwise fallthrough and create new list.
     */

  case 1:
    /*
     * Need to make a new index file by reading entire data file.
     * Create a new index list.
     */

    if (Debug == 1)
      printf("\tReading data file to create index file....\n");
    if (create_index(L) == 0)
    {
      printf("ERROR creating index file\n");
      *nindex = 0;
      return NULL;
    } // if (create_index(L) == 0)

    /*
     * Write the list out to a file.
     */

    write_new_index_file(L);
    if (Debug == 1)
      printf("\tDone Reading data to create index file.%d records\n",
	     L->nindex);
    break;

  case 0:
    /*
     * Current index file is good.
     * Read in existing index file.
     */

    read_index_file(L);
    break;

  default:
    /*
     * Nothing..
     */

    L->Ibase = NULL;
    L->nindex = 0;
    L->file_bytes = 0;
  } // switch (rebuild)

  /*
   * Return interface:
   */

    *nindex = L->nindex;
    return (L->Ibase);
}

/*----------------------------------------------------------------*
 *
 * GLOBALLY EXPORTED FUNCTIONS
 *
 *----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*
 * THE ONE global routine for index files:
 *
 * If needed, recreate an index file.
 * 
 * On exit, index file has been made current and:
 *          Return pointer to array of structs containing index data.
 *
 *          Return NULL for no data or error making the index file or
 *          index list.
 */

SIO_index_data_t *SIO_rebuild_index_file(const char *directory,  /* where (bdry dir) */
					 const char *suffix,     /* what */
					 time_t time,      /* when */
					 int *nindex,      /* how many (returned)*/
					 const char *bdry_index_dir,
					 /* dir for bdry index file */
					 int yesterday_file,  /* flag to look at
							       * yesterday's bdry file */
					 char **bdry_file)  /* name of bdry file */
{
  char *c;
  time_t yesterday = 0;
    
  /*
   * FOrm the file name and the index file name.
   */

  if (yesterday_file)
  {
    yesterday = time - (24 * 60 * 60);
    c = SIO_file_name_full(directory, suffix, yesterday, 0);
  }
  else
    c = SIO_file_name_full(directory, suffix, time, 0);

  strcpy(Data_file_name, c);
  if (yesterday_file)
    *bdry_file = strcpy(new char[strlen(Data_file_name)+1], 
			Data_file_name);
  
  if (bdry_index_dir != NULL)
  {
    if (yesterday_file)
      c = index_file_name(bdry_index_dir, suffix, yesterday);
    else
      c = index_file_name(bdry_index_dir, suffix, time);
  }
  else
  {
    if (yesterday_file)
      c = index_file_name(directory, suffix, yesterday);
    else
      c = index_file_name(directory, suffix, time);
  } // if (bdry_index_dir != NULL)
  strcpy(Index_file_name, c);

  return(rebuild_ifile_sub(nindex));
}

/*----------------------------------------------------------------*/
/*
 * THE other ONE global routine for index files:
 *
 * If needed, recreate an index file.
 * 
 * On exit, index file has been made current and:
 *          Return pointer to array of structs containing index data.
 *
 *          Return NULL for no data or error making the index file or
 *          index list.
 */

SIO_index_data_t *SIO_rebuild_named_index_file(char *dir,  /* output dir */
					       char *name, /* data file name */
					       int *nindex) /* how many (returned)*/
{
  char *filename, str[BUFSIZ];
    
  /*
   * FOrm the file name and the index file name.
   */

  strcpy(Data_file_name, name);

  // get file name

  filename = strrchr(name, '/');
  sprintf(str, "%s/%s", dir, filename+1);
  strcpy(Index_file_name, str);
  strcat(Index_file_name, "_index");
  
  return(rebuild_ifile_sub(nindex));
}

/*----------------------------------------------------------------*/
