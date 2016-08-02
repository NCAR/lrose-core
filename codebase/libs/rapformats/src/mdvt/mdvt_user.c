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
 *  MDVT_USER.C  Subroutines useful for accessing MDVT data.
 *
 *  F. Hage.  Dec 1993. RAP
 *
 */


#define MDVT_USER_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>

#include <toolsa/os_config.h>
#include <toolsa/utim.h>
#include <rapformats/mdvt_user.h>

static int name_compare(void const *v1, void const *v2);

#define HUGE_POS 1.0E32
#define HUGE_NEG -1.0E32
/******************************************************************************
 * MDVT_LOAD_DATA_HEADER: Load mdv data file header into given area
 */


int MDVT_load_data_header( FILE *infile, master_header_t  *m_head)
{

    fseek(infile,0,0);
    if((fread(m_head,sizeof(*m_head),1,infile)) != 1) {
        return MDVT_FAILURE;
    }
    return MDVT_SUCCESS;
}

/******************************************************************************
 * MDVT_LOAD_FIELD_HEADER: Load mdv data field header into the given structure 
 *    into its data record;
 */


int MDVT_load_field_header( FILE *infile, field_header_t  *f_head, int  field_num)
{
    long file_location;

    file_location = sizeof(master_header_t) + (field_num * sizeof(field_header_t));

    fseek(infile,file_location,0);
    if((fread(f_head,sizeof(*f_head),1,infile)) != 1) {
        return MDVT_FAILURE;
    }
    return MDVT_SUCCESS;
}

/******************************************************************************
 * MDVT_GET_PLANE: Allocate space for a data plane and read the desired plane
 *     into the buffer from the Open file. Caller is responsible for freeing
 *     up the buffer when done.
 */


float * MDVT_get_plane( FILE *infile, field_header_t  *f_head, int   plane_num)
{
    long file_location;
    long num_points;
    float *buf;
    
    num_points = f_head->nx * f_head->ny;
    file_location = f_head->file_index + (plane_num * num_points);
    
    if(fseek(infile,file_location,0) < 0) return (float *) NULL;

    if((buf =(float *) calloc(num_points,sizeof(float))) ==NULL) return (float *) NULL;

    if((fread(buf,sizeof(float),num_points,infile)) != num_points) {
        free(buf);
        return (float *) NULL;
    }

    return buf;
}

/******************************************************************************
 * MDVT_GET_VOLUME: Allocate space for a data volume and read the desired plane
 *     into the buffer from the Open file. Caller is responsible for freeing
 *     up the buffer when done.
 */


float * MDVT_get_volume( FILE *infile, field_header_t  *f_head)
{
    long file_location;
    long num_points;
    float *buf;
    
    num_points = f_head->nx * f_head->ny * f_head->nz;
    file_location = f_head->file_index;
    
    if(fseek(infile,file_location,0) < 0) return (float *) NULL;

    if((buf = (float *) calloc(num_points,sizeof(float))) ==NULL) return (float *) NULL;

    if((fread(buf,sizeof(float),num_points,infile)) != num_points) {
        free(buf);
        return (float *) NULL;
    }

    return buf;
}

/******************************************************************************
 * MDVT_GET_CHAR_PLANE: Allocate space for a data volume and read the plane
 *     into the buffer from the Open file. Data array is then dynamically scaled
 *     to fit in to a unsigned char array. The unsigned char array is returned
 *     along with the scale, bias, bad_data and missing data values,
 *     used to restore the original values. Caller is responsible for freeing
 *     up the returned buffer when done.
 */


unsigned char * MDVT_get_char_plane( FILE *infile,
                field_header_t  *f_head,
		int    plane_num,
                double *scale, double *bias,
                unsigned char *bad_data, unsigned char *missing_data)
{
    long i;
    long file_location;
    long num_points;
    double min_val,max_val;
    double data_range;
    float *buf,*ptr;
    unsigned char  *c_buf,*c_ptr;
    
    num_points = f_head->nx * f_head->ny;
    file_location = f_head->file_index + (plane_num * num_points);
    
    if(fseek(infile,file_location,0) < 0) return (unsigned char *) NULL;

    if((buf = (float *) calloc(num_points,sizeof(float))) ==NULL) return (unsigned char *) NULL;

    if((fread(buf,sizeof(float),num_points,infile)) != num_points) {
        free(buf);
        return (unsigned char *) NULL;
    }

    /* Calculate extent of data range */
    ptr = buf;
    min_val = HUGE_POS;
    max_val = HUGE_NEG;
    for(i=0; i < num_points; i++) {
            if(*ptr != f_head->bad_data_value && *ptr != f_head->missing_data_value) {
            if(*ptr > max_val) max_val = *ptr;
            if(*ptr < min_val) min_val = *ptr;
        }
        ptr++;
    }

    data_range = max_val - min_val + 1.0;
    *scale = data_range / 253.0;
    *bias = min_val - *scale; /* Data is additionally offset by 1 scaled unit */
    *bad_data =  0;
    *missing_data = 255;

    if((c_buf = (unsigned char *) calloc(num_points,sizeof(unsigned char))) ==NULL) {
        free(buf);
        return (unsigned char *) NULL;
    }

    /* Now loop through array and scale values */
    ptr = buf;
    c_ptr = c_buf;
    for(i=0; i < num_points; i++) {
        if(*ptr == f_head->bad_data_value) {
            *c_ptr = 0;
        } else {
            if(*ptr == f_head->missing_data_value) {
                *c_ptr = 255;
            } else {
		/* Scale value to nearest int. 1.0 Added to avoid  using 0 as a value */
                *c_ptr = (((double) *ptr - min_val) / *scale) + 1.5;
	    }
        }
	ptr++;
	c_ptr++;
    }

    free(buf);    /* Free up floating point array */

    return c_buf;
}

/******************************************************************************
 * MDVT_GET_CHAR_VOLUME: Allocate space for a data volume and read the volume
 *     into the buffer from the Open file. Data array is then dynamically scaled
 *     to fit in to a unsigned char array. The unsigned char array is returned
 *     along with the scale, bias, bad_data and missing data values,
 *     used to restore the original values. Caller is responsible for freeing
 *     up the returned buffer when done.
 */


unsigned char * MDVT_get_char_volume( FILE *infile,
                field_header_t  *f_head,
                double *scale, double *bias,
                unsigned char *bad_data, unsigned char *missing_data)
{
    long i;
    long file_location;
    long num_points;
    double min_val,max_val;
    double data_range;
    float *buf,*ptr;
    unsigned char  *c_buf,*c_ptr;
    
    num_points = f_head->nx * f_head->ny * f_head->nz;
    file_location = f_head->file_index;
    
    if(fseek(infile,file_location,0) < 0) return (unsigned char *) NULL;

    if((buf = (float *) calloc(num_points,sizeof(float))) ==NULL) return (unsigned char *) NULL;

    if((fread(buf,sizeof(float),num_points,infile)) != num_points) {
        free(buf);
        return (unsigned char *) NULL;
    }

    /* Calculate extent of data range */
    ptr = buf;
    min_val = HUGE_POS;
    max_val = HUGE_NEG;
    for(i=0; i < num_points; i++) {
        if(*ptr != f_head->bad_data_value && *ptr != f_head->missing_data_value) {
            if(*ptr > max_val) max_val = *ptr;
            if(*ptr < min_val) min_val = *ptr;
        }
        ptr++;
    }

    data_range = max_val - min_val + 1.0;
    *scale = data_range / 253.0;
    *bias = min_val - *scale; /* Data is additionally offset by 1 scaled unit */
    *bad_data =  0;
    *missing_data = 255;

    if((c_buf = (unsigned char *) calloc(num_points,sizeof(unsigned char))) ==NULL) {
        free(buf);
        return (unsigned char *) NULL;
    }

    /* Now loop through array and scale values */
    ptr = buf;
    c_ptr = c_buf;
    for(i=0; i < num_points; i++) {
        if(*ptr == f_head->bad_data_value) {
            *c_ptr = 0;
        } else {
            if(*ptr == f_head->missing_data_value) {
                *c_ptr = 255;
            } else {
		/* Scale value to nearest int. 1.0 Added to avoid  using 0 as a value */
                *c_ptr = (((double) *ptr - min_val) / *scale) + 1.5;
	    }
        }
	ptr++;
	c_ptr++;
    }

    free(buf);    /* Free up floating point array */

    return c_buf;
}

/***************************************************************************
 * FIND_DATA_SETS: Find appropriatly named data sets and
 * Place them in order
 *
 */

int
MDVT_find_data_sets( char    **top_dir, int     num_dirs, char    *name_list[],
    char    *match, /* the file name must contain this string to be valid */
    int name_len,   /* minimum length of the file names wanted in bytes */
    int min,        /* minimum integer the files will atoi() to */
    int max_dirs,   /* max number of dirs to find */
    int list_size)  /* Size of the name list buffers */
{
    int     i,j;
    int     d_count;
    int     data_set_count;
    char    buf[1024];
    char     *name[4096];
    DIR *dirp;
    struct dirent *dent;

    data_set_count = 0;

    for(j=0; j < num_dirs; j++) {
        d_count = 0;
        if((dirp = opendir(top_dir[j])) == NULL) continue;
        /* Loop thru directory looking for the directory names */

        for(dent = readdir(dirp); dent != NULL; dent = readdir(dirp)) {
            if((strlen(dent->d_name) >= name_len) && (strstr(dent->d_name,match) != NULL) && (atoi(dent->d_name) >= min)) {
                name[d_count] = calloc(list_size,1);
                strncpy(name[d_count] , dent->d_name,list_size);
                d_count++;
            }
        }

        closedir(dirp);

        if(d_count == 0) continue;

        /* sort the array into ascending order */
        qsort(name,d_count,sizeof( char *),name_compare);

        if(d_count + data_set_count > max_dirs) d_count = max_dirs- data_set_count;    /* limit the number of found files */

        for(i=0; i < d_count; i++) {
	    sprintf(buf,"%s/%s",top_dir[j],name[i]);
	    strncpy(name_list[data_set_count++],buf,list_size);
            free(name[i]);
	}
    }

    return data_set_count;
}
 
/***************************************************************************
 * FIND_FORECAST_FILES: Find appropriatly named data files and
 * Place them in order
 */

int
MDVT_find_forecast_files( char    *dir, char    *name_list[],
    char    *match, /* the file name must contain this string to be valid */
    int name_len,   /* minimum length of the file names wanted in bytes */
    int min,        /* minimum integer the files will atoi() to */
    int max_dirs,   /* max number of files to find */
    int list_size)  /* Size of the name list buffers */
{
    int     i,d_count;
    char     *name[4096];
    char     path[1024];
    time_t   now;
    DIR     *dirp;
    struct  dirent *dent;
    struct stat sbuf;

    if((dirp = opendir(dir)) == NULL) return 0;
    d_count = 0;

    now = time(0);

    /* Loop thru directory looking for the directory names */

    for(dent = readdir(dirp); dent != NULL; dent = readdir(dirp)) {
        if((strlen(dent->d_name) >= name_len) && (strstr(dent->d_name,match) != NULL) && (atoi(dent->d_name) >= min)) {
	    sprintf(path,"%s/%s",dir,dent->d_name);
	    if((stat(path,&sbuf)) < 0) continue;
	    if(now > sbuf.st_mtime +2 ) { /* Only accept files that haven't been modified for at least two seconds */
                name[d_count] = calloc(list_size,1);
                strncpy(name[d_count] , dent->d_name,list_size);
                d_count++;
	    }
        }
    }

    closedir(dirp);

    if(d_count == 0) return 0;;

    /* sort the array into ascending order */
    qsort(name,d_count,sizeof( char *),name_compare);

    if(d_count > max_dirs) d_count = max_dirs;    /* limit the number of found files */

    for(i=0; i < d_count; i++) {
        strncpy(name_list[i],name[i],list_size);
        free(name[i]);
    }

    return d_count;
}

/*************************************************************************
 * MDVT_NAME_TO_UTIME: Given run_dir_name + data_file_name, Return unix 
 * time stamp for that file. If f_name == NULL return the data set
 * generation time. MDVT file name conventions are:
 *  YYYYMMDD_HHMMSS.ext/HHHMMSS.ext
 *    ^ dir_name ^     / ^ file_name ^
 */

long MDVT_name_to_utime(char *dir_name,char *f_name)
{
    int day,hour,minute,second;
    int delta_value; 	/* Holds info about the time diff between gen time and forecast time */

    char  *ptr;	
    char  day_string[32];
    char time_string[32];
    UTIMstruct ut;

    day = 0;
    ptr = strstr(dir_name,"199");
    if(ptr == NULL) return -1;
     
    strncpy(day_string,ptr,8);
    day_string[8] = 0;
    day = atoi(day_string);

    ut.year = day / 10000;
    day -= ut.year * 10000;
     
    ut.month = day  / 100;
    day -= ut.month * 100;
     
    ut.day = day;

    ptr += 9;  /* HHMMSS starts 9 chars into name */
    strncpy(time_string,ptr,8);
    hour = atoi(time_string);

    ut.hour = hour / 10000;
    hour -= ut.hour * 10000;
     
    ut.min = hour /100;
    hour -= ut.min * 100;

    ut.sec = hour;  

    UTIMdate_to_unix(&ut);
    UTIMunix_to_date(ut.unix_time,&ut);

    /* If no forecast name is specified, return the data set generation time */
    if(f_name == NULL) return ut.unix_time;
     
    delta_value = atoi(f_name);
    hour = delta_value / 10000;
    delta_value -= hour * 10000;

    minute = delta_value / 100;
    delta_value -= minute * 100;

    second = delta_value;

    delta_value = (3600 * hour) + (60 * minute) + second;

    return (ut.unix_time + delta_value);
}

/*************************************************************************
 * MDVT_OPEN_DATA_TIME: This routine finds the appropriate file to open based on
 *  a data time. This routine assumes that the latest data set must contain
 *  appropriate. Thus a search is made for the most recent data set, then
 *  the forcast time is added to the generation time to get the forcast file name
 */

FILE *MDVT_open_data_time(long dtime,int num_data_dirs,char **data_dir,char *data_set_suffix,char *file_suffix)
{
    int    i;
    int    hours,mins,secs;
    int    num_dir_entries;
    long   data_time;
    long   delta;

    char    file_name[256];

    static char    *dir[256];
    static int     data_list_len = 0;

    /* Allocate static memory areas */
    if(data_list_len == 0) {
        data_list_len = 256;

        /* Allocate space for sorted name lists */
        for(i=0; i < 256; i++) {
           dir[i] = calloc(256,1);
        }
    }

    num_dir_entries = MDVT_find_data_sets(data_dir,num_data_dirs,dir,data_set_suffix,16,19900000,256,256);
    if(num_dir_entries <=0) return (FILE *) NULL;

    /* Get the latest data set generation time */
    data_time = MDVT_name_to_utime(dir[num_dir_entries -1],NULL);

    /* Compute the delta values based on the difference */
    delta = dtime - data_time;
    hours = delta / 3600;
    delta -= hours * 3600;
    mins =  delta / 60;
    delta -= mins * 60;
    secs = delta;

    sprintf(file_name,"%s/%02d%02d%02d.%s",dir[num_dir_entries -1],hours,mins,secs,file_suffix);
    return fopen(file_name,"rw");
}
 
/*****************************************************************************
 * NAME_COMPARE: function for qsorting
 */

static int name_compare(void const *v1, void const *v2)
{
    char **s1 = (char **) v1;
    char **s2 = (char **) v2;
    return strcmp(*s1, *s2);
}

