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
/******************************************************************
    File: gint_user.c
    Routines for the users of gint-save_volume

*****************************************************************/

#define GINT_USER

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>

#include <toolsa/os_config.h>
#include <rapformats/gint_user.h>
/******************************************************************
 * GINT_FREE_HEADER: Free up any allocated memory in the Gint
 *  Tvolume_header
 */
int GINT_free_header(Tvolume_header *hd)
{
    if(hd->vh) {
	free(hd->vh);
	hd->vh = NULL;
    } 

    if(hd->fi) {
	free(hd->fi);
	hd->fi = NULL;
    }

    if(hd->ai) {
	free(hd->ai);
	hd->ai = NULL;
    }

    if(hd->li) {
	free(hd->li);
	hd->li = NULL;
    }

    return 0;
}

/******************************************************************
 * GET_HEADER: Gets the file's header information. This version
 *   Allocates space for header data, if pointers are set to null.
 *   If pointers are non-null - They are first freed, then reallocated
 *   Users should call GINT_free_header to deallocate space.
 */

int GINT_get_header(Tvolume_header *hd,FILE *file)
{

if(hd->vh == NULL) {
    if((hd->vh = (Volume_header *) calloc(1,sizeof(Volume_header))) == NULL ) {
        printf("Cannot allocate memory - -gint_user.c.\n");
        return(1);
    }
}

/* read v header */
if(fseek(file,0,SEEK_SET)<0) {
    printf("Error fseek (0) -gint_user.c.\n");
    return (1);
}
if(fread(hd->vh,sizeof(Volume_header),1,file)!= 1) {
    printf("Failed in reading the header (0) - gint_user.c.\n");
    return (1);
}

#ifdef DBG
/* check data id */
if(hd->vh->data_id[0]==' ' || hd->vh->data_id[0]=='\0'){
    printf("Data_id error in header - gint_user.c ID = %s\n", hd->vh->data_id);
    return (1);
}
#endif

/* ALLOCATE SPACE FOR FIELD INFO and READ */
if(hd->fi !=NULL ) free(hd->fi);
if((hd->fi=(Field_infor *)calloc(hd->vh->n_fields,sizeof(Field_infor))) == NULL){
    printf("Failed in allocating space for header - gint_user.c.\n");
    return (1);
}
if(fread(hd->fi,sizeof(Field_infor),hd->vh->n_fields,file)!=hd->vh->n_fields) {
    printf("Failed in reading remaining hd - gint_user.c.\n");
    return (1);
}


/* ALLOCATE SPACE FOR ALTITUDE INFO and READ */
if(hd->ai !=NULL ) free(hd->ai);
if((hd->ai=(Altitude_infor *)calloc(hd->vh->nz,sizeof(Altitude_infor))) == NULL){
    printf("Failed in allocating space for header - gint_user.c.\n");
    return (1);
}
if(fread(hd->ai,sizeof(Altitude_infor),hd->vh->nz,file)!=hd->vh->nz) {
    printf("Failed in reading remaining hd - gint_user.c.\n");
    return (1);
}


/* ALLOCATE SPACE FOR DATA PLANE FILE LOCATION INFO and READ */
if(hd->li !=NULL ) free(hd->li);
if((hd->li=(Location_infor *)calloc(hd->vh->n_fields*hd->vh->nz,sizeof(Location_infor))) == NULL){
    printf("Failed in allocating space for header - gint_user.c.\n");
    return (1);
}
if(fread(hd->li,sizeof(Location_infor),hd->vh->n_fields*hd->vh->nz,file)!=hd->vh->n_fields*hd->vh->nz) {
    printf("Failed in reading remaining hd - gint_user.c.\n");
    return (1);
}


if(hd->vh->encode==0){ /* we add default loc and size */
    int c_size,i,cnt,t,sz;
    c_size=sizeof(Volume_header)+hd->vh->n_fields*sizeof(Field_infor)+
    hd->vh->nz*sizeof(Altitude_infor)+hd->vh->nz*hd->vh->n_fields*sizeof(Location_infor);
    t=hd->vh->nz*hd->vh->n_fields;
    cnt=c_size;
    sz=hd->vh->nx*hd->vh->ny;
    for(i=0;i<t;i++){
    hd->li[i].off=cnt;
    hd->li[i].len=sz;
    cnt += sz;
    }
}

return(0);
}

/********************************************************************
 * GET_PLANE : get a cappi plane. Return a pointor to the data of
 * decoded cappi, or NULL in case of failure 
 */

unsigned char *GINT_get_plane(int field, int cappi_ind, FILE *file, Tvolume_header *hd)
{
    unsigned char *buf,*ipt;
    int offset,len,ind,nxy;

    /* check */
    if(field<0 || field>=hd->vh->n_fields ||
           cappi_ind<0 || cappi_ind>=hd->vh->nz || file == NULL) return (NULL);

    /* allocate the space */
    nxy = hd->vh->nx * hd->vh->ny;
    if((buf=(unsigned char *)calloc(1,nxy))==NULL){
        printf("Failed in allocating cappi space.\n");
        return (NULL);
    }


    offset=hd->li[field+hd->vh->n_fields*cappi_ind].off;
    len=hd->li[field+hd->vh->n_fields*cappi_ind].len;

    /* read a cappi */
    if(fseek(file,offset,SEEK_SET)<0){
        printf("Failed in lseek (%d) - gint_user.c.\n",offset);
        return (NULL);
    }
    offset=hd->vh->nx*hd->vh->ny-len;
    ipt=buf+offset;
    if(fread(ipt,len,1,file)!=1){
	printf("Failed in reading  field: %d  cappi: %d, len: %d.\n",field, cappi_ind,len);
        return (NULL);
    }

    /* decode */
    switch(hd->vh->encode) {
        case NOT_ENCODED:
        break;

      case RL7_ENCODED: {
        ind = GINT_run_length_decode(len,ipt,buf);
        if (ind > nxy) {
          free(buf);
          return NULL;
        }
        break;
      }

      case RL8_ENCODED: {
        ind = GINT_run_length_decode_byte(len,ipt,buf);
        if (ind > nxy) {
          free(buf);
          return NULL;
        }
        break;
      }
    } 
    return (buf);
}

/*******************************************************************
 * GINT_RUN_LENGTH_ENCODE_BYTE:
 * run length encode of 8 bit string.
 * 255 is used for control code and all 255 data are changed
 * to be 254
 *
 *    Calling routine must allocate enough space for encoded
 *    data.
 *
 *     Returns the length of the encoded data
 */

int GINT_run_length_encode_byte(int len, unsigned char *stri,unsigned char *stro)
{
unsigned char *ipt,*opt,*ept,tmp,*tpt;
int i,nrun;

ipt=stri;
opt=stro;
ept=ipt+len;

while(ipt<ept){
    tmp=*ipt;

    /* find number of runs */
    tpt=ipt+1;
    while(tpt<ept && *tpt==tmp) tpt++;
    nrun=tpt-ipt;

    if(nrun==1){
    if(tmp==255) *opt++=254;
    else *opt++=tmp;
    ipt++;
    }
    else if(nrun<4){
    for(i=0;i<nrun;i++){
        if(*ipt==255) *opt++=254;
        else *opt++=*ipt;
        ipt++;
    }
    }
    else{
    if(nrun>254) nrun=254;
    *opt++=255;
    *opt++=nrun;
    if(tmp==255) tmp=254;
    *opt++=tmp;
    ipt+=nrun;
    }
}

return ((int)(opt-stro));
}
    
/*******************************************************************
 * GINT_RUN_LENGTH_DECODE_BYTE:
 * run length decode of 8 bit string. 
 *
 * The Calling routine must allocate enough space for the decoded data
 * Returns the length of the decoded data array
 */

int GINT_run_length_decode_byte( int len, unsigned char *stri, unsigned char *stro)
{
unsigned char *ipt,*opt,*ept,tmp;
int i,nrun;

ipt=stri;
opt=stro;
ept=ipt+len;

while(ipt<ept){

    if(*ipt!=255){
    *opt++=*ipt++;
    continue;
    }
    ipt++;
    nrun=*ipt++;
    tmp=*ipt++;
    for(i=0;i<nrun;i++) *opt++=tmp;
}

return ((int)(opt-stro));
}
        
/*******************************************************************
 * RUN_LENGTH_ENCODE:
 * Run length encode of str of less then 8 bits
 */

int GINT_run_length_encode(int len, unsigned char *stri, unsigned char *stro,unsigned char mask)
{
int cnt,i,ind,max;
unsigned char test,tmp;

    max=127;
    mask=mask&0xfe;
    cnt=0;
    ind=1;
    test=stri[0]&mask;
    stro[0]=test;
    for(i=1;i<len;i++){
        tmp=stri[i]&mask;
        if(test==tmp && cnt<max){
            cnt++;
            continue;
        }
        if(cnt>0) stro[ind++]=(cnt<<1)+1; 
        stro[ind++]=tmp;
        test=tmp;
        cnt=0;
    }
    if(cnt>0) stro[ind++]=(cnt<<1)+1;

    return (ind);
}

/*******************************************************************
 * GINT_RUN_LENGTH_DECODE: run length decode of str of less then 8 bits 
 * 
 * Returns the length of the decoded array
 */

int GINT_run_length_decode(int len, unsigned char *stri,unsigned char *stro)
{
    int cnt,i,ind,j;
    unsigned char test = 0;

    ind=0;
    for(i=0;i<len;i++){
        if((stri[i]&1)==0){
            stro[ind++]=stri[i];
            test=stri[i];
        }
        else{
            cnt=(stri[i]>>1);
            for(j=0;j<cnt;j++) stro[ind++]=test;
        }
    }

    return (ind);
}

/******************************************************************
 * GINT_PUT_HEADER: writes the image header
 */

int GINT_put_header(Tvolume_header *hd, FILE *file)
{
    static Volume_header *vhd;
    int size;
    
    /* write volume header */
    if(fseek(file,0,SEEK_SET)<0) {
        printf("Error fseek (0) -gint_user.c.\n");
        return (1);
    }
    vhd=hd->vh;
    if(fwrite(vhd,sizeof(Volume_header),1,file)!=1){
        printf("Failed in writing the header (0) - gint_user.c.\n");
        return (1);
    }

    size=vhd->n_fields*sizeof(Field_infor);
    if(fwrite(hd->fi,size,1,file) != 1 ) {
        printf("Failed in writing field info header \n");
        return (1);
    }
    size = vhd->nz*sizeof(Altitude_infor);
    if(fwrite(hd->ai,size,1,file) != 1 ) {
        printf("Failed in writing alt info header \n");
        return (1);
    }
    size = vhd->n_fields*vhd->nz*sizeof(Location_infor);
    if(fwrite(hd->li,size,1,file) != 1 ) {
        printf("Failed in writing location info header \n");
        return (1);
    }

    return(0);
}

/********************************************************************
 * GINT_PUT_PLANE : write a plane of data. Return a non-zero flag in case
 * of failure: Assumes everyting in header is set correctly
 */

int GINT_put_plane( unsigned char *buf, int field, int cappi_ind, FILE *file, Tvolume_header *hd)
{
        unsigned char *ipt;
        int offset,len;

        /* check */
        if(field<0 || field>=hd->vh->n_fields || cappi_ind<0 || cappi_ind>=hd->vh->nz) return (1);

	/* calc position of data in file */
        offset=hd->li[field+hd->vh->n_fields*cappi_ind].off;
        len=hd->li[field+hd->vh->n_fields*cappi_ind].len;

        /* write a cappi */
        if(fseek(file,offset,SEEK_SET)<0){
            printf("Failed in fseek (%d) - gint_user.c.\n",offset);
            return (-1);
        }
        ipt=buf;
        if(fwrite(ipt,len,1,file)!=1){
                printf("Failed in writing the data\n");
                return (-2);
        }

        return (0);
}

/**************************************************************
 * GINT_PRINT_HEADER: Print out GINT header in a nice way
 * Rachel Ames 1/95
 * F. Hage 2/95
 */

void GINT_print_header(Tvolume_header *hd, FILE *outfile)
{
int ifield,iz;
char field_name[16];
char unit_name[16];

   fprintf(outfile,"\nPrint_Header");
   fprintf(outfile,"\n------------\n");
   fprintf(outfile,"\nName of volume %s",hd->vh->file_name); 
   fprintf(outfile,"\nProjection Type (CART=-1, LATLON=0) %ld",hd->vh->proj_type); 
   fprintf(outfile,"\nNumber dimensions: nx = %ld, ny = %ld, nz = %ld",hd->vh->nx,hd->vh->ny,hd->vh->nz);
   fprintf(outfile,"\nGrid Spacing: (dx,dy)  %ld meters", hd->vh->xss);
   fprintf(outfile,"\nData Encoding: %d", hd->vh->encode);
   fprintf(outfile,"\nNumber of fields = %ld",hd->vh->n_fields);
   fprintf(outfile,"\nLast time of previous volume: %s",ctime(&hd->vh->l_time));
   fprintf(outfile,"Last time of current  volume: %s",ctime(&hd->vh->time));
   fprintf(outfile,"Origin of the grid (%ld m, %ld m) ",hd->vh->xstt,hd->vh->ystt);
   fprintf(outfile,"\nLongitude and Latitude of radar (%f, %f) ",
                     hd->vh->origin_lon/1000000.,hd->vh->origin_lat/1000000.);
    
   fprintf(outfile,"\n    Field      units     scale        offset    bad data value");
   for (ifield = 0; ifield < hd->vh->n_fields; ifield ++) {
      bzero(field_name,16); bzero(unit_name,16);
      strncpy(field_name,hd->fi[ifield].field_names,8);
      strncpy(unit_name,hd->fi[ifield].unit_names,8);
      fprintf(outfile,"\n%8s   %8s      %5.3f       %5.3f     %xx",
            field_name,unit_name,hd->fi[ifield].scale/65536.,hd->fi[ifield].offset/65536.,
            (int)hd->fi->bad_data_value);
   }

   fprintf(outfile,"\n\nAltitude Information:");
   fprintf(outfile,"\nLevel #\t  Height (M)");
   for (iz = 0; iz < hd->vh->nz; iz ++) {
      fprintf(outfile,"\n%4d  \t %8ld  ",iz,hd->ai[iz].z);
   }

   fprintf(outfile,"\n\nPlane Location Information:");
   fprintf(outfile,"\nPlane #\t  len \t  Offset (M)");
   for (iz = 0; iz < hd->vh->nz * hd->vh->n_fields; iz ++) {
      fprintf(outfile,"\n%4d  \t %8ld  \t %8ld ",iz,hd->li[iz].len,hd->li[iz].off);
   }
   fprintf(outfile,"\n");
}
