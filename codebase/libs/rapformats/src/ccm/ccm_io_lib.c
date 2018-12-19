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
/**********************************************************************
 * CCM_LIB.C: Routines that deal with CCM format files 
 *
 * F. Hage    5/91 - NCAR
 * Keyword: CCM File access, CCM Header access  printing
 */

#define GET_CCM    1

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <toolsa/os_config.h>
#include <dataport/swap.h>
#include <rapformats/ccm_file.h>
#if defined(__linux)
extern void swab(const void *from, void *to, ssize_t n);
#endif

/****************************************************************************
 * GET_CCM_HEADER: Read a ccm file header into the supplied area 
 *    Converts it to Local byte order if necessary
 */

int get_ccm_header(infile,head)
    FILE    *infile;            /* Open stream  */
    struct ccm_header *head;    /* pointer to a header area */
{
    rewind(infile);

    if((fread(head,sizeof(*head),1,infile) != 1)) {
        perror("CCM_IO C library");
        return(-1);
    }

    swap_ccm_header_bytes(head);

    return 0;
}

/*****************************************************************
 * GET_DATA_PLANE:  Gather data plane from the open file.
 *    Returns pointer to array of data or NULL on error.
 *    Don't forget to free array
 */

unsigned short
*get_data_plane(field,plane,c_head,file)
        int field;    /* Data filed of interest */
        int plane;
        struct ccm_header *c_head;
        FILE *file;
{
    unsigned short   *c_data;  /* pointer for data manipulation */
    int    rec_len;
    int bytes_per_plane;
    int bytes_per_field;
    int bytes_read;
    int offset;

    bytes_per_plane = c_head->nx * c_head->ny * sizeof(short);
    bytes_per_field = c_head->nx * c_head->ny * c_head->nz * sizeof(short) + (2 * sizeof(int));
     
    c_data =  (unsigned short *) malloc(bytes_per_plane);  /* Get memory for the plane */
 
    offset = sizeof(*c_head) + (field * bytes_per_field);    /* find offset to beginning of volume*/
    /* Seek to the correct place */
    if((fseek(file,offset,0)) < 0) {
        free(c_data);
        return NULL;
    }
 
    /* Read the record length marker into our local buffer */
    if((bytes_read = fread(((int *)&rec_len),1,sizeof(int),file)) != sizeof(int)) {
            return NULL;
    }

    offset = plane * bytes_per_plane;
    /* Seek to the correct place */
    if((fseek(file,offset,1)) < 0) {
        free(c_data);
        return NULL;
    }
 
    /* Read the data into our local buffer */
    if((bytes_read = fread(((char *)c_data),1,bytes_per_plane,file)) != bytes_per_plane) {
            free(c_data);
            return NULL;
    }
 
    /* Swap bytes to native format if necessary */
    if(rec_len != c_head->nx * c_head->ny * c_head->nz * sizeof(short)) {   /* Swap bytes */
        swab(c_data,c_data,bytes_per_plane);
    }

    return c_data;
}



/******************************************************************************
 * SWAP_CCM_HEADER_BYTES: Swap the bytes if necessary
 *
 */

void
swap_ccm_header_bytes(hd)
    struct ccm_header *hd;
{
    if(hd->pad1 == sizeof(*hd) - (2 * sizeof(int))) return; 
     
    SWAP_array_32((ui32 *) &(hd->pad1),NUM_HEADER_INTS * sizeof(int));    /* Convert the integers */

    SWAP_array_32((ui32 *) &(hd->dx),NUM_HEADER_FLOATS * sizeof(float));    /* Convert the floats */
    /* Char data need not be swapped */
}

/******************************************************************************
 * PRINT_CCM_HEADER: Print out revelant ccm header info to the
 * given open FILE.
 *
 */

void
print_ccm_header(outfile,hd)
    FILE    *outfile;
    struct ccm_header *hd;
{
    int    i;

    char    field_name[32];
    char    field_units[32];

    fprintf(outfile,"\nBegin Time : %d\t%s\n",hd->time_begin,ctime((time_t *) &(hd->time_begin)));
    fprintf(outfile,"\nEnd Time : %d\t%s\n",hd->time_end,ctime((time_t *) &(hd->time_end)));
    fprintf(outfile,"\nTime Centriod : %d \t%s\n",hd->time_cent,ctime((time_t *) &(hd->time_cent)));

    fprintf(outfile,"\nOrigin Lat: %.3f''\n", hd->origin_lat);
    fprintf(outfile,"\nOrigin Lon: %.3f''\n", hd->origin_lon);
    fprintf(outfile,"\nOrigin Altitude: %.3f''\n", hd->origin_alt);
     
    fprintf(outfile,"\nAngle of X axis relative to North : %.3f deg\n", hd->north_angle);
    fprintf(outfile,"\nBad Data Value:%d\n", hd->bad_data_value);
         
    fprintf(outfile,"\nnx: %d\t",(int)hd->nx);
    fprintf(outfile,"min_x: %.3f\t",hd->min_x);
    fprintf(outfile,"max_x: %.3f\t",hd->max_x);
    fprintf(outfile,"dx: %.3f\n",hd->dx);

    fprintf(outfile,"ny: %d\t",(int)hd->ny);
    fprintf(outfile,"min_y: %.3f\t",hd->min_y);
    fprintf(outfile,"max_y: %.3f\t",hd->max_y);
    fprintf(outfile,"dy: %.3f\n",hd->dy);

    fprintf(outfile,"nz: %d\t",(int)hd->nz);
    fprintf(outfile,"min_z: %.3f\t",hd->min_z);
    fprintf(outfile,"max_z: %.3f\t",hd->max_z);
    fprintf(outfile,"dz: %.3f\n",hd->dz);

    fprintf(outfile,"\nnum_fields: %d\n",(int)hd->num_fields);
    fprintf(outfile,"Number    Name                           Units                          Scale factor      Bias\n");
    fprintf(outfile,"-------------------------------------------------------------------------------------------\n");
    for(i=0; i < hd->num_fields && i < NUM_FIELDS; i++) {
        strncpy(field_name,hd->var_name[i],32);
        field_name[31] = '\0';
        strncpy(field_units,hd->var_units[i],32);
        field_units[31] = '\0';
        fprintf(outfile,"%d\t%s\t%s\t",(i+1),field_name,field_units);
        fprintf(outfile,"%f\t%f\n",hd->gsb[i].scale,hd->gsb[i].bias);
    }
}
