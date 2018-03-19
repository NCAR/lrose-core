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
/*************************************************************************
 *
 * RfZr.c
 *
 * part of the rfutil library - radar file access
 *
 * Z-R file routines
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * Feb 1996
 *
 **************************************************************************/

#include <titan/zr.h>
#include <dataport/bigend.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define ALLOC_INC 10

/*
 * file scope
 */

static char Label[R_FILE_LABEL_LEN];
static long N_entries_alloc = 0;
static zr_entry_t *Entries;
static zr_header_t Header;
static FILE *Zr_file;

/*
 * local prototypes
 */

static void add_entry(zr_entry_t *entry);
static void alloc_entries(long n_entries_needed);
static void close_file(void);
static int compare_entries(const void *, const void *);
static long entry_index(si32 time);
static int open_and_read_file(char *file_path, char *mode);
static int open_new_file(char *file_path);
static void write_and_close_file(char *file_path);

/*************************************************************************
 *
 * RfAddZrEntry()
 *
 * Add a Zr entry
 *
 * returns 0 on success, -1 on failure
 *
 **************************************************************************/

int RfAddZrEntry(char *zr_dir,
		 zr_entry_t *entry)
     
{
  
  char file_path[MAX_PATH_LEN];
  date_time_t dtime;

  /*
   * fill out time struct
   */
  
  dtime.unix_time = entry->time;
  uconvert_from_utime(&dtime);

  /*
   * compute file path
   */
  
  sprintf(file_path, "%s%s%.4d%.2d%.2d.zr",
	  zr_dir, PATH_DELIM,
	  dtime.year, dtime.month, dtime.day);
  
  /*
   * open and read file
   */

  if (open_and_read_file(file_path, "r+")) {
    return (-1);
  }

  /*
   * add entry
   */

  add_entry(entry);

  /*
   * write and close file
   */

  write_and_close_file(file_path);

  return (0);

}

/*************************************************************************
 *
 * RfGetZrClosest()
 *
 * Get Zr closest in time to requested time, within the margin
 *
 * returns 0 on success, -1 on failure
 *
 **************************************************************************/

int RfGetZrClosest(char *zr_dir,
		   time_t req_time, int time_margin,
		   double *coeff_p, double *expon_p)
     
{

  char file_path[MAX_PATH_LEN];
  int i, index = 0;
  si32 time_diff;
  si32 min_diff = 1000000000;
  date_time_t dtime;
  zr_entry_t *entry;

  /*
   * fill out time struct
   */
  
  dtime.unix_time = req_time;
  uconvert_from_utime(&dtime);

  /*
   * compute dbm path for this time
   */
  
  sprintf(file_path, "%s%s%.4d%.2d%.2d.zr",
	  zr_dir, PATH_DELIM,
	  dtime.year, dtime.month, dtime.day);

  /*
   * open and read file
   */

  if (open_and_read_file(file_path, "r")) {
    return (-1);
  }
  close_file();
  
  entry = Entries;
  for (i = 0; i < Header.n_entries; i++, entry++) {
    time_diff = labs(req_time - entry->time);
    if (time_diff < min_diff) {
      min_diff = time_diff;
      index = i;
    }
  }

  if (min_diff > time_margin) {
    return (-1);
  } else {
    *coeff_p = Entries[index].coeff;
    *expon_p = Entries[index].expon;
    return (0);
  }

}

/*************************************************************************
 *
 * RfGetZrEntry()
 *
 * Get a Zr entry for a given time, load into struct ptr provided
 *
 * returns 0 on success, -1 on failure
 *
 **************************************************************************/

int RfGetZrEntry(char *zr_dir,
		 si32 entry_time,
		 zr_entry_t *entry)
     
{

  char file_path[MAX_PATH_LEN];
  long index;
  date_time_t dtime;

  /*
   * fill out time struct
   */
  
  dtime.unix_time = entry_time;
  uconvert_from_utime(&dtime);

  /*
   * compute dbm path for this time
   */
  
  sprintf(file_path, "%s%s%.4d%.2d%.2d.zr",
	  zr_dir, PATH_DELIM,
	  dtime.year, dtime.month, dtime.day);

  /*
   * open and read file
   */

  if (open_and_read_file(file_path, "r")) {
    return (-1);
  }

  /*
   * get index
   */

  index = entry_index(entry_time);

  if (index < 0) {
    return (-1);
  }

  /*
   * copy in entry
   */

  *entry = Entries[index];
  
  return (0);

}

/*************************************************************************
 *
 * RfPrintZrFile()
 *
 * Print the contents of a zr file
 *
 * returns 0 on success, -1 on failure
 *
 **************************************************************************/

int RfPrintZrFile(char *file_path)
     
{

  long i;
  zr_entry_t *entry;
  
  /*
   * open and read
   */

  if (open_and_read_file(file_path, "r")) {
    return (-1);
  }

  fprintf(stdout, "Contents of ZR file '%s'\n\n", file_path);
  fprintf(stdout, "%19s %10s %10s %10s\n",
	  "Time", "Coeff", "Expon", "Correction");
  fprintf(stdout, "%19s %10s %10s %10s\n",
	  "====", "=====", "=====", "==========");

  entry = Entries;
  for (i = 0; i < Header.n_entries; i++, entry++) {
    fprintf(stdout, "%19s %10.0f %10.2f %10.2f\n",
	    utimstr(entry->time),
	    entry->coeff, entry->expon, entry->correction);
  }

  close_file();

  return (0);

}

/*************
 * add_entry()
 */

static void add_entry(zr_entry_t *entry)

{

  long index;
  
  /*
   * get index of entry in array
   */
  
  index = entry_index(entry->time);

  if (index >= 0) {

    /*
     * copy over old entry
     */

    Entries[index] = *entry;

  } else {

    /*
     * realloc
     */

    alloc_entries(Header.n_entries + 1);

    /*
     * put into last slot
     */

    Entries[Header.n_entries] = *entry;
    Header.n_entries++;

    /*
     * sort array
     */

    qsort((char *) Entries, (int) (Header.n_entries),
	  sizeof(zr_entry_t), compare_entries);

    /*
     * set header
     */

    Header.start_time = Entries[0].time;
    Header.end_time = Entries[Header.n_entries - 1].time;
    
  } /* if (index >= 0) */

  return;

}

/******************
 * alloc_entries()
 *
 * Alloc entry array
 */

static void alloc_entries(long n_entries_needed)

{

  if (N_entries_alloc < n_entries_needed) {

    /*
     * allocate the required space plus a buffer so that 
     * we do not do too many reallocs
     */
      
    if (Entries == NULL) {
      
      N_entries_alloc = n_entries_needed + ALLOC_INC;
      
      Entries = (zr_entry_t *) umalloc
	(N_entries_alloc * sizeof(zr_entry_t));

    } else {
      
      N_entries_alloc = n_entries_needed + ALLOC_INC;
      
      Entries = (zr_entry_t *) urealloc
	((char *) Entries,
	 N_entries_alloc * sizeof(zr_entry_t));
      
    } /* if (N_entries_alloc == 0) */

  } /* if (Entries == NULL) */

  return;
  
}

/**************
 * close_file()
 */

static void close_file(void)

{
  fclose(Zr_file);
}

/*****************************************************************************
 * define function to be used for sorting (lowest to highest)
 */

static int compare_entries(const void *v1, const void *v2)


{

  zr_entry_t *e1, *e2;

  e1 = (zr_entry_t *) v1;
  e2 = (zr_entry_t *) v2;

  return (e1->time - e2->time);

}

/***************
 * entry_index()
 *
 * Searches for entry index in array
 *
 * returns -1 if not found
 */

static long entry_index(si32 time)

{
  
  long i;
  zr_entry_t *entry = Entries;

  if (Header.start_time == 0 && Header.end_time == 0) {
    return (-1L);
  } else if (time < Header.start_time) {
    return (-1L);
  } else if (time > Header.end_time) {
    return (-1L);
  } else {
    for (i = 0; i < Header.n_entries; i++, entry++) {
      if (time == entry->time) {
	return (i);
      }
    }
  }

  return (-1L);

}


/*****************
 * open_new_file()
 *
 * Opens a new file, sets header
 */

static int open_new_file(char *file_path)

{

  struct stat stat_buf;

  if (stat(file_path, &stat_buf) != 0) {

    /*
     * file does not exist, so open it
     */

    if ((Zr_file = fopen(file_path, "w")) == NULL) {
      return (-1);
    }

  }
  
  /*
   * set header
   */

  Header.n_entries = 0;
  Header.start_time = 0;
  Header.end_time = 0;
  memset(Label, 0, R_LABEL_LEN);
  strcpy(Label, ZR_FILE_TYPE);
  
  return (0);

}

/***********************
 * open_and_read_file()
 */

static int open_and_read_file(char *file_path, char *mode)

{

  if ((Zr_file = fopen(file_path, mode)) == NULL) {
    if (!strcmp(mode, "r")) {
      fprintf(stderr, "ERROR - ZR file access\n");
      fprintf(stderr, "Cannot open zr file for reading\n");
      perror(file_path);
      return (-1);
    } else {
      return (open_new_file(file_path));
    }
  }

  /*
   * read in label
   */

  if (fread(Label, R_LABEL_LEN, 1, Zr_file) != 1) {
    if (!strcmp(mode, "r")) {
      fprintf(stderr, "ERROR - ZR file access\n");
      fprintf(stderr, "Cannot read zr file label\n");
      perror(file_path);
      return (-1);
    } else {
      fprintf(stderr, "WARNING - ZR file access\n");
      fprintf(stderr, "Cannot read zr file label\n");
      perror(file_path);
      fprintf(stderr, "Starting new file\n");
      return (open_new_file(file_path));
    }
  }

  /*
   * check label
   */

  if (strcmp(Label, ZR_FILE_TYPE)) {
    if (!strcmp(mode, "r")) {
      fprintf(stderr, "ERROR - ZR file access\n");
      fprintf(stderr, "Incorrect file label in file '%s'\n", file_path);
      fprintf(stderr, "Label should be '%s'\n", ZR_FILE_TYPE);
      fprintf(stderr, "Label found is '%s'\n", Label);
      return (-1);
    } else {
      fprintf(stderr, "WARNING - ZR file access\n");
      fprintf(stderr, "Incorrect file label in file '%s'\n", file_path);
      fprintf(stderr, "Label should be '%s'\n", ZR_FILE_TYPE);
      fprintf(stderr, "Label found is '%s'\n", Label);
      fprintf(stderr, "Starting new file\n");
      return (open_new_file(file_path));
    }
  }

  /*
   * read in header
   */

  if (fread(&Header, sizeof(zr_header_t), 1, Zr_file) != 1) {
    if (!strcmp(mode, "r")) {
      fprintf(stderr, "ERROR - ZR file access\n");
      fprintf(stderr, "Cannot read zr file header\n");
      perror(file_path);
      return (-1);
    } else {
      fprintf(stderr, "WARNING - ZR file access\n");
      fprintf(stderr, "Cannot read zr file header\n");
      perror(file_path);
      fprintf(stderr, "Starting new file\n");
      return (open_new_file(file_path));
    }
  }

  /*
   * decode from BE
   */
  
  BE_to_array_32((ui32 *) &Header, sizeof(zr_header_t));

  /*
   * alloc array for entries
   */

  alloc_entries(Header.n_entries);

  /*
   * read in entries
   */

  if ((int) fread(Entries, sizeof(zr_entry_t),
	    Header.n_entries, Zr_file) != Header.n_entries) {
    if (!strcmp(mode, "r")) {
      fprintf(stderr, "ERROR - ZR file access\n");
      fprintf(stderr, "Cannot read zr file entries\n");
      perror(file_path);
      return (-1);
    } else {
      fprintf(stderr, "WARNING - ZR file access\n");
      fprintf(stderr, "Cannot read zr file entries\n");
      perror(file_path);
      fprintf(stderr, "Starting new file\n");
      return (open_new_file(file_path));
    }
  }

  /*
   * decode from BE
   */
  
  BE_to_array_32((ui32 *) Entries, Header.n_entries * sizeof(zr_header_t));

  return (0);

}

/************************
 * write_and_close_file()
 */

static void write_and_close_file(char *file_path)

{

  si32 n_entries = Header.n_entries;

  /*
   * write label
   */

  fseek(Zr_file, 0L, SEEK_SET);
  
  if (fwrite(Label, R_LABEL_LEN, 1, Zr_file) != 1) {
    fprintf(stderr, "ERROR - ZR file access\n");
    fprintf(stderr, "Cannot write zr file label\n");
    perror(file_path);
    close_file();
    return;
  }

  /*
   * set header to BE format
   */

  BE_from_array_32((ui32 *) &Header, sizeof(zr_header_t));
  
  /*
   * write header
   */

  if (fwrite(&Header, sizeof(zr_header_t), 1, Zr_file) != 1) {
    fprintf(stderr, "ERROR - ZR file access\n");
    fprintf(stderr, "Cannot write zr file header\n");
    perror(file_path);
    close_file();
    return;
  }

  /*
   * Set entries to BE format
   */

  BE_from_array_32((ui32 *) Entries,
		   n_entries * sizeof(zr_entry_t));

  /*
   * write entries
   */

  if ((int) fwrite(Entries, sizeof(zr_entry_t),
		   n_entries, Zr_file) != n_entries) {
    fprintf(stderr, "ERROR - ZR file access\n");
    fprintf(stderr, "Cannot write zr file entries\n");
    perror(file_path);
    close_file();
    return;
  }

  close_file();

}

