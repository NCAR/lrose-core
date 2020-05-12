/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            John H. Merritt
            Space Applications Corporation
            Vienna, Virginia

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
/* 
 * Read SIGMET version 1 and version 2 formatted files.
 *
 * Data is written in little-endian format for version 1 files.
 * This means that on big endian machines, bytes must be swapped.
 * This is auto-detected and all swapping is automatic.
 *
 * Note that this is different in SIGMET version 2 data files.  There
 * the data is written in big-endian format (written w/ an IRIS computer).
 * For that case, the byte swapping logic is reversed.
 *
 * The highest level functions provided is:
 *   nsig_read_sweep  -- Read an entire sweep including all fields.
 *                       Call it until NULL is returned.  That indicates
 *                       end-of-file.
 *----------------------------------------------------------------------
 * 8/13/96
 *
 * John H. Merritt
 * Space Applications Corp.
 * NASA/GSFC Code 910.1 
 *
 * Copyright 1996
 */

#define NSIG_VER2

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "nsig.h"

FILE *uncompress_pipe(FILE *fp);
int big_endian(void);
int little_endian(void);
void swap_4_bytes(void *word);
void swap_2_bytes(void *word);
int rsl_pclose(FILE *fp);

/*********************************************************************
 * Open a file and possibly setup a gunzip pipe                      *
 *********************************************************************/
FILE *nsig2_open(char *file_name)
{
  FILE *fp;
  int save_fd;

   /* Open input file */
  if (file_name == NULL) { /* Use stdin */
    save_fd = dup(0);
    fp = fdopen(save_fd, "r");
  } else if((fp = fopen(file_name,"r")) == NULL) {
    perror(file_name);
    return fp;
  }

  fp = uncompress_pipe(fp); /* Transparently gunzip. */
  return fp;
}

/**********************************************************************
 *   Given an opened file stream read in the headers and fill in      *
 *   the nsig_file data structure.                                    *
 **********************************************************************/
int nsig2_read_record(FILE *fp, char *nsig_rec)
{
   int n;
   int nbytes;
   char *buf;
   /* Input could be a chain of pipes. So read until no more data.
    * For instance, a gzip pipe will write chunks of 4096 bytes, 
    * even when there is really more that we want to read.
    */
   /* Return the number of bytes read. */
   buf = (char *)nsig_rec;

   if (feof(fp)) return -1;
   nbytes = 0;
   while((n = fread(&buf[nbytes], sizeof(char), NSIG_BLOCK-nbytes, fp)) > 0) {
     nbytes += n;
   }
   return nbytes;
}


/*********************************************************
 * Close nsig file                                       *
 *********************************************************/
void nsig2_close(FILE *fp)
   {
     rsl_pclose(fp);
   }

static int do_swap;

int nsig2_endianess(NSIG_Record1 *rec1)
{
  /*
   * If NSIG is version1 and on big-endian, then swap.
   * If NSIG is version2 and on little-endian, then swap.
   */
  /*
  printf("id = %d %d\n", (int)rec1->struct_head.id[0], (int)rec1->struct_head.id[1]);
  */
  if (rec1->struct_head.id[0] == 0) { /* Possible little-endian */
    if (rec1->struct_head.id[1] >= 20)
      /* This is a big-endian file. Typically, version 2. */
      do_swap = little_endian();
    else
      do_swap = big_endian();
  } else if (rec1->struct_head.id[1] == 0) { /* Possible big-endian */
    if (rec1->struct_head.id[0] <= 7)
      /* This is a little-endian file.  Version 1. */
      do_swap = big_endian();
  }
  /*
  printf("DO SWAP = %d\n", do_swap);
  */
  return do_swap;
}

short NSIG2_I2 (twob x)
{/* x is already a pointer. */
  short s;
  memmove(&s, x, sizeof(twob));
  if (do_swap) swap_2_bytes(&s);
  return s;
}

int NSIG2_I4 (fourb x)
{ /* x is already a pointer. */
  int i;
  memmove(&i, x, sizeof(fourb));
  if (do_swap) swap_4_bytes(&i);
  return i;
}


void nsig2_free_ray(NSIG_Ray *r)
{
  if (r == NULL) return;
  free(r->range);
  free(r);
}

void nsig_free_sweep(NSIG_Sweep **s)
{
  int i=0,itype;
  if (s == NULL) return;
  int nparams = s[0]->nparams;
  for (itype=0; itype<nparams; itype++) {
    if (s[itype] == NULL) continue;
    
    if ((int) s[itype]->idh.data_type == NSIG_DTB_EXH)
      free(s[itype]->ray[i]);
    else {
      for (i=0; i<NSIG_I2(s[itype]->idh.num_rays_act); i++)
        nsig_free_ray(s[itype]->ray[i]);
    }
    free(s[itype]->ray);
    free(s[itype]);
  }
  free(s);
}

static int ipos = 0;  /* Current position in the data buffer. */
static NSIG_Data_record data;

int nsig2_read_chunk(FILE *fp, char *chunk)
{
  int i, n;
  int the_code;
  int nwords, end_nwords;

  /* The information saved from call to call is 'data' and represents
   * the data that has been buffered.  When new data is needed
   * it will be read from the file.  'ipos' is global and initially
   * set in 'nsig_read_sweep'.  Assumptions: chunk is big enough.
   *
   * Return number of bytes in 'chunk'  -- this variable is 'i'.
   */

  i = n = 0;
  the_code = 0;

#define Vprint
#undef  Vprint
  while(the_code != 1) {
    if (feof(fp)) return -1;
    if (ipos == sizeof(data)) { /* the_code is in the next chunk */
#ifdef Vprint
      printf("Exceeded block size looking for the_code. Get it from next buffer.\n");
#endif
      n = nsig_read_record(fp, (char *)data);
      if (n <= 0) return n;  /* Problem. */

#ifdef Vprint
      printf("Read %d bytes.\n", n);
      printf("Resetting ipos\n");
#endif
      ipos = sizeof(NSIG_Raw_prod_bhdr);
    }
      
#ifdef Vprint
    printf("ipos = %d -- ", ipos);
#endif
    the_code = NSIG_I2(&data[ipos]);
#ifdef Vprint
    printf("the_code = %d (%d) -- ", the_code, (unsigned short)the_code);
#endif
    ipos += sizeof(twob);
    
    if (the_code < 0) {  /* THIS IS DATA */
      nwords = the_code & 0x7fff;
#ifdef Vprint
      printf("#data words (2-bytes) is %d\n", nwords);
#endif
      if (ipos + sizeof(twob)*nwords > sizeof(data)) {
#ifdef Vprint
        printf("Exceeded block size... transferring and reading new (i=%d).\n", i);
#endif
        /* Need another phyical block. */
            
        /* But, first transfer the remainder to the output chunk. */
        /* And, transfer begining of next block */
        /* Transfer end of current buffer. */
        end_nwords = (NSIG_BLOCK - ipos)/sizeof(twob);
        memmove(&chunk[i], &data[ipos], sizeof(twob)*end_nwords);
        i += end_nwords * sizeof(twob);

        n = nsig_read_record(fp, (char *)data);
        if (n <= 0) return n;  /* Problem. */
        /* New ipos */
        nwords -= end_nwords;
        ipos = sizeof(NSIG_Raw_prod_bhdr);
            
        /* Transfer beginning of new buffer */
        if (i+nwords * sizeof(twob) > NSIG_BLOCK) return -1;
        memmove(&chunk[i], &data[ipos], sizeof(twob) * nwords);
        i += nwords * sizeof(twob);
        ipos += nwords * sizeof(twob);
        
#ifdef Vprint
        printf("Words to transfer (at end of block) is %d\n", end_nwords);
        printf("Transfer %d words from beginning of next buffer.\n", nwords);
        printf("ipos in new buffer is %d\n", ipos);
#endif
      } else { /* Normal situation.  Position to end of data.
                * But, first transfer it to the chunk.
              */
        if (i+nwords * sizeof(twob) > NSIG_BLOCK) return -1;
        memmove(&chunk[i], &data[ipos], sizeof(twob) * nwords);
        i += nwords * sizeof(twob);
        ipos += sizeof(twob) * nwords;
      }
          
    } else if (the_code == 1) {  /* END OF THE RAY. */
#ifdef Vprint
      printf("------------------------------> Reached end of ray.\n");
#endif
      break; /* or continue; */

    } else if (the_code == 0) {  /* UNKNOWN */
      break;
    } else {  /* NUMBER OF ZERO's */
#ifdef Vprint
      printf("#000000000000 to skip is %d (i=%d)\n", the_code, i);
#endif
      if (i+the_code * sizeof(twob) > NSIG_BLOCK) return -1;
      memset(&chunk[i], 0, the_code*sizeof(twob));
      i += the_code * sizeof(twob);
    }
        
    if (ipos >= sizeof(data)) {
#ifdef Vprint
      printf("Exceeded block size ... ipos = %d\n", ipos);
      printf("This should be right at the end of the block.\n");
#endif
      n = nsig_read_record(fp, (char *)data);
      if (n <= 0) return n; /* Problem. */
      ipos = sizeof(NSIG_Raw_prod_bhdr);
    }
    
  } /* End while. */                             
  return i;
}

NSIG_Ext_header_ver0 *nsig2_read_ext_header_ver0(FILE *fp)
{
  NSIG_Data_record chunk;
  int n;
  NSIG_Ext_header_ver0 *xh0;
  xh0 = NULL;
  n = nsig2_read_chunk(fp, (char *)chunk);
  if (n <= 0) return xh0;
#ifdef Vprint
  printf("Ver0 x-header.  %d bytes found.\n", n);
#endif
  n     -= sizeof(NSIG_Ray_header);
  xh0 = (NSIG_Ext_header_ver0 *)calloc(1, n);
  if (xh0) memmove(xh0, &chunk[sizeof(NSIG_Ray_header)], n);
  return xh0;
}


NSIG_Ext_header_ver1 *nsig2_read_ext_header_ver1(FILE *fp)
{
  NSIG_Data_record chunk;
  int n;
  NSIG_Ext_header_ver1 *xh1;
  xh1 = NULL;
  n = nsig2_read_chunk(fp, (char *)chunk);
  if (n <= 0) return xh1;
#ifdef Vprint
  printf("Ver1 x-header.  %d bytes found.\n", n);
#endif
  n     -= sizeof(NSIG_Ray_header);
  xh1 = (NSIG_Ext_header_ver1 *)calloc(1, n);
  if (xh1) memmove(xh1, &chunk[sizeof(NSIG_Ray_header)], n);
  return xh1;
}

NSIG_Ray *nsig2_read_ray(FILE *fp)
{
  int n, nbins;
  NSIG_Ray_header rayh;
  static NSIG_Data_record chunk;
  NSIG_Ray *ray;
  
  n = nsig2_read_chunk(fp, (char *)chunk);
  /* Size of chunk is n */

  if (n == 0) return NULL; /* Silent error. */

  if (n < 0) {
    fprintf(stderr, "nsig_read_ray: chunk return code = %d.\n", n);
    return NULL;
  }

  if (n > NSIG_BLOCK) { /* Whoa! */
    fprintf(stderr, "nsig_read_ray: chunk bigger than buffer. n = %d,\
 maximum block size allowed is %d\n", n, NSIG_BLOCK);
    return NULL;
  }

  ray = (NSIG_Ray *) calloc(1, sizeof(NSIG_Ray));
  memcpy(&ray->h, chunk, sizeof(NSIG_Ray_header));
  n -= sizeof(NSIG_Ray_header);
#ifdef Vprint
  printf("nsig_read_ray: allocating %d bytes for range\n", n);
#endif
  memcpy(&rayh, chunk, sizeof(NSIG_Ray_header));
  nbins = NSIG_I2(rayh.num_bins);
  if (nbins <= 0) return NULL;
#ifdef Vprint
  printf("               rayh.num_bins = %d (nbins %d, n %d)\n", NSIG_I2(rayh.num_bins), nbins, n);
#endif
  ray->range = (unsigned char *)calloc(n, sizeof(unsigned char));
  /* Changed calloc nbins to calloc n for 2-byte data.
  ray->range = (unsigned char *)calloc(nbins, sizeof(unsigned char));
  memmove(ray->range, &chunk[sizeof(NSIG_Ray_header)], nbins);
     Can remove this commented-out code once we know changes work.
  */
  memmove(ray->range, &chunk[sizeof(NSIG_Ray_header)], n);
  
  return ray;
}


NSIG_Sweep **nsig2_read_sweep(FILE *fp, NSIG_Product_file *prod_file)
{
  NSIG_Sweep **s;
  int i, j, n;
  static NSIG_Ingest_data_header **idh = NULL;
  static NSIG_Raw_prod_bhdr *bhdr = NULL;
  NSIG_Ray *nsig_ray;
  int data_mask, iray, nrays[12], max_rays;
  int masks[5];
  int nparams;
  int is_new_ray;
  int idtype[12];
  int is_new_sweep;
  int xh_size;
  NSIG_Ext_header_ver0 *exh0;
  NSIG_Ext_header_ver1 *exh1;

  /*
   * The organization of a RAW PRODUCT FILE:  (page III-38)
   *
   *  Record #1   { <Product_hdr> 0, 0, 0... }
   *  Record #2   { <Ingest_Summary> 0, 0, 0... }
   *  Record #3   { <Raw_Prod_BHdr> <ingest_data_header> Data...} \
   *  Record #4   { <Raw_Prod_BHdr> Data...}                       \
   *      .               .           .                            | Data for
   *      .               .           .                            /  Sweep
   *  Record #N   { <Raw_Prod_BHdr> Data 0...}                    /     #1
   *  Record #N+1 { <Raw_Prod_BHdr> <ingest_data_header> Data...} \
   *  Record #N+2 { <Raw_Prod_BHdr> Data...}                       \
   *      .               .           .                            | Data for
   *      .               .           .                            /  Sweep
   *  Record #M   { <Raw_Prod_BHdr> Data 0...}                    /     #2
   *
   *  What about the order of info in 'Data'?
   *    Data, when it begins a sweep:
   *         a. Raw Product Bhdr
   *         b. Ingest data header for param 1
   *                .
   *                .
   *            Ingest data header for param n+1
   *         c. Ray header
   *         d. Ray data
   *
   *    Ray header and Ray data are encoded with the compression algorithm.
   *    If Ray data spans more than one physical NSIG BLOCK (6144 bytes),
   *    then the 'Data' consists of:
   *         a. Raw Product Bhdr
   *         b. Ray header
   *         c. Ray data
   *
   *    It is just missing all the Ingest data header fields.
   */

#define Vprint
#undef  Vprint
  /* Determine if we need to byte-swap values. */
  (void)nsig_endianess(&prod_file->rec1);
  
  /* Setup the array of ingest data headers [0..nparams-1] */
#ifdef NSIG_VER2
  memmove(&masks[0], prod_file->rec2.task_config.dsp_info.data_mask_cur.mask_word_0,
    sizeof(fourb));
  memmove(&masks[1], &prod_file->rec2.task_config.dsp_info.data_mask_cur.mask_word_1,
    4*sizeof(fourb));
  nparams = 0;
  for (j=0; j < 5; j++) {
    data_mask = masks[j];
    for (i=0; i<32; i++)
      nparams += (data_mask >> i) & 0x1;
  }
#else
  memmove(&data_mask, prod_file->rec2.task_config.dsp_info.data_mask, sizeof(fourb));
  for (nparams=i=0; i<32; i++)
    nparams += (data_mask >> i) & 0x1;
#ifdef Vprint
  printf("data_mask %x\n", data_mask);
#endif
#endif
  /* Number of sweeps */
#ifdef Vprint
  {int nsweeps;
  nsweeps = NSIG_I2(prod_file->rec2.task_config.scan_info.num_swp);
  printf("nsig2.c:::nparams = %d, nsweeps = %d\n", nparams, nsweeps);
  }
#endif


  if (idh == NULL) {
    
    idh = (NSIG_Ingest_data_header **)calloc(nparams, sizeof(NSIG_Ingest_data_header *));
    ipos = 0;
    for (i=0; i<nparams; i++) {
      idh[i] = (NSIG_Ingest_data_header *)&data[sizeof(NSIG_Raw_prod_bhdr) + i*sizeof(NSIG_Ingest_data_header)];
    }
    bhdr = (NSIG_Raw_prod_bhdr *)prod_file->data;
  }

  xh_size = NSIG_I2(prod_file->rec2.ingest_head.size_ext_ray_headers);
#ifdef Vprint
  {int rh_size;
  rh_size = NSIG_I2(prod_file->rec2.ingest_head.size_ray_headers);
  printf("Extended header is %d bytes long.\n", xh_size);
  printf("     Ray header is %d bytes long.\n", rh_size);
  }
#endif
  is_new_ray = 1;
  is_new_sweep = 1;
  max_rays  = NSIG_I2(prod_file->rec2.ingest_head.num_rays);

  /* Ingest initial block for the sweep.  All remaining I/O will
   * be performed in the de-compression loop.
   */
  if (feof(fp)) return NULL;
  n = nsig_read_record(fp, (char *)data);
  if (n <= 0) return NULL;
#ifdef Vprint
  printf("Read %d bytes for data.\n", n);
#endif


  /* This is a NEW sweep. */
  iray = 0;
#ifdef Vprint
  {int isweep;
  isweep = NSIG_I2(idh[0]->sweep_num);
  printf("Number of rays in sweep %d is %d\n", isweep, max_rays);
  }
#endif
  /* Allocate memory for sweep. */
  s = (NSIG_Sweep **) calloc (nparams, sizeof(NSIG_Sweep*));

  /* Now pointers to all possible rays. */
  for (i=0; i<nparams; i++) {
  /* Load sweep headers */
    s[i] = (NSIG_Sweep *) calloc (nparams, sizeof(NSIG_Sweep));
    s[i]->nparams = nparams;
    memmove(&s[i]->bhdr, &bhdr, sizeof(NSIG_Raw_prod_bhdr));
    memmove(&s[i]->idh, idh[i], sizeof(NSIG_Ingest_data_header));
    s[i]->ray = (NSIG_Ray **) calloc (max_rays, sizeof(NSIG_Ray *));
  } 

  /* Process this sweep. Keep track of the end of the ray. */
  ipos = sizeof(NSIG_Raw_prod_bhdr); /* Position in the 'data' array */

  max_rays = 0;
  for (i=0; i<nparams; i++) {
    idtype[i] = NSIG_I2(idh[i]->data_type);
    nrays[i] = (int)NSIG_I2(idh[i]->num_rays_act);
    if (nrays[i] > max_rays) max_rays = nrays[i];
#ifdef Vprint
    printf("New ray: parameter %d has idtype=%d\n", i, idtype[i]);
    printf("Number of expected rays in sweep %d is %d\n", isweep, (int)NSIG_I2(idh[i]->num_rays_exp));
    printf("Number of actual   rays in sweep %d is %d\n", isweep,  (int)NSIG_I2(idh[i]->num_rays_act));
#endif

  }
  if (is_new_sweep) 
    ipos += nparams * sizeof(NSIG_Ingest_data_header);
  
  /* ipos = sizeof(NSIG_Raw_prod_bhdr) + nparams*sizeof(NSIG_Ingest_data_header); */
  /* 'iray' is the true ray index into 's', whereas, 'nsig_iray' is what
   * the NSIG file says it is.  I'll trust 'iray'
   *
   * I have a cursor into the 'data' buffer representing my current
   * position for processing rays.  This cursor will dictate if I read
   * a new NSIG block.  The cursor is call 'ipos'.  It is initialized
   * each time a new ray is encountered.
   */

#ifdef Vprint
  { int ioff, nsig_iray;
  /* Check that all idh pointers 'id' is Ingest data header. */
  ioff = NSIG_I2(bhdr->ray_loc);
  nsig_iray = NSIG_I2(bhdr->ray_num);
  printf("Offset to begining of ray %d is %d, iray=%d\n", nsig_iray, ioff,iray);
  }
#endif
  
  /* DECODE THE DATA HERE */
  /* From III-39 */
  /*
   *       Table 3.5-5
   *   Compression Code Meanings
   *
   * MSB LOW-bits  Meaning
   *  0     0      <unused>
   *  0     1      End of ray.
   *  0     2      <unused>
   *  0   3-32767  3 to 32767 zeros skipped.
   *  1     0      <unused>
   *  1   1-32767  1 to 32767 data words follow.
   */
  
  do {
#ifdef Vprint
    printf("---------------------- New Ray <%d> --------------------\n", iray);
#endif
    if (feof(fp)) { /* Premature eof */
      return NULL; /* This will have to do. */
    }
    /* For all parameters present. */
    is_new_ray = 0;
    for (i=0; i<nparams; i++) {

      /* Keep track of the cursor within the buffer, and, possibly
       * read another buffer when a ray is split across two NSIG blocks
       */
      nsig_ray = NULL;
      if (idtype[i] != 0) { /* Not an extended header. */
        nsig_ray = nsig_read_ray(fp);

      } else { /* Check extended header version. */
        if (xh_size <= 20) {
          exh0 = nsig_read_ext_header_ver0(fp);
          if (exh0) {
            nsig_ray = (NSIG_Ray *)calloc(1, sizeof(NSIG_Ray));
            nsig_ray->range = (unsigned char *)exh0;
          }
        } else {
          exh1 = nsig_read_ext_header_ver1(fp);
          if (exh1) {
            nsig_ray = (NSIG_Ray *)calloc(1, sizeof(NSIG_Ray));
            nsig_ray->range = (unsigned char *)exh1;
          }
        }
      }
      if (nsig_ray) is_new_ray = 1;
      if (iray > nrays[i]) break;
      s[i]->ray[iray] = nsig_ray;

    } /* End for */
    if (is_new_ray) iray++;
    
  } while (iray < max_rays);
#ifdef Vprint
  printf("iray = %d\n", iray);
#endif
  return s;
  
}

/**************************************************
 *  Convert 2 byte binary angle to floating point *
 **************************************************/
float nsig2_from_bang(bang in)
   {
   float result,maxval = 65536.0;
   unsigned short bi_ang;

   memmove(&bi_ang, in, sizeof(bang));
   if (do_swap) swap_2_bytes(&bi_ang);
   result = ((float)(bi_ang)/maxval) * 360.0;
   
   return (result);
   }

/*************************************************
 * convert 4 byte binary angle to floating point *
 *************************************************/
float nsig2_from_fourb_ang(fourb ang)
   {
   double result,maxval;
   unsigned int bi_ang;

   maxval = 4294967296.0;

   memmove(&bi_ang, ang, sizeof(fourb));
   if (do_swap) swap_4_bytes(&bi_ang);
   
   result = ((double)(bi_ang)/maxval) * 360.0;
   
   return ((float)result);
   }
/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996  Paul A. Kucera of Applied Research Corporation,
                        Landover, Maryland, a NASA/GSFC on-site contractor.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
	*/
/*************************************************************/
/*                                                           */
/*    Function: nsig_to_radar.c                              */
/*                                                           */
/*    Paul A. Kucera                                         */
/*    Applied Research Corporation                           */
/*    NASA/GSFC                                              */
/*    TRMM/Code 910.1                                        */
/*                                                           */
/*    Modifications by:                                      */
/*    John H. Merritt                                        */
/*    Space Applications Corporation                         */
/*    NASA/GSFC                                              */
/*    TRMM/Code 910.1                                        */
/*                                                           */
/*    Started: 08 AUG 96                                     */
/*                                                           */
/*    Derived from Paul Kucera's nsig_to_radar.c             */
/*    Copyright 1996                                         */
/*************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

#include"nsig.h"
#include <trmm_rsl/rsl.h>

extern int radar_verbose_flag;
extern int rsl_qfield[]; /* See RSL_select_fields */

   /*  We need this entry for various things esp in Ray_header  */
#define MISSING_HEADER_DATA -9999
   /*  The following is speed of light _in_ _air_ !  */
#define SPEED_OF_LIGHT 299792458
#define MIT_BEAMWIDTH      1.65
#define TOG_BEAMWIDTH      1.65
#define KWA_BEAMWIDTH      1.0
#define DEFAULT_BEAMWIDTH  1.0
#define NSIG_NO_DATA       -1
#define MAX_NSIG_SWEEPS    30
#define MAX_NSIG_RAYS      400
#define NSIG_NO_ECHO       -32.0
#define NSIG_NO_ECHO2     -999.0

static float (*f)(Range x);
static Range (*invf)(float x);

FILE *file;

void  nsig2_get_extended_header_info(NSIG_Sweep **nsig_sweep, int xh_size, int iray,
                                     int nparams,
                                     int *msec, float *azm, float *elev,
                                     float *pitch, float *roll, float *heading,
                                     float *azm_rate, float *elev_rate,
                                     float *pitch_rate, float *roll_rate,
                                     float *heading_rate,
                                     float *lat, float *lon, int *alt, float *rvc,
                                     float *vel_east, float *vel_north, float *vel_up)
{
  static NSIG_Ext_header_ver1 xh;
  int data_type, itype;

  *msec = *azm = *elev = *pitch = *roll = *heading =
	*azm_rate = *elev_rate = *pitch_rate = *roll_rate = *heading_rate =
	*lat = *lon = *alt = *rvc = 0;

  /* Determine where 'itype' for extended header is. */
  for (itype = 0; itype<nparams; itype++) {
	data_type = NSIG_I2(nsig_sweep[itype]->idh.data_type);
    if (data_type == NSIG_DTB_EXH) break;
  }
  /*  printf("...extended header itype=%d, nparams=%d\n", itype, nparams); */
  if (itype == nparams) return;  /* No extended header. */

  /* Version 1. */
  if (nsig_sweep[itype]->ray[iray] == NULL) return;
  if (nsig_sweep[itype]->ray[iray]->range == NULL) return;
  memmove(&xh, nsig_sweep[itype]->ray[iray]->range, sizeof(xh));
  *msec = NSIG_I4(xh.msec);
  /*   printf("...extended header msec= %d\n", *msec); */
  if (xh_size <= 20) /* Stop, only have version 0. */
    return;

  /* Version 1 processing. */
  *azm = nsig_from_bang(xh.azm);
  *elev = nsig_from_bang(xh.elev);
  *pitch = nsig_from_bang(xh.pitch);
  *roll   = nsig_from_bang(xh.roll);
  *heading = nsig_from_bang(xh.heading);
  *azm_rate = nsig_from_bang(xh.azm_rate);
  *elev_rate = nsig_from_bang(xh.elev_rate);
  *pitch_rate = nsig_from_bang(xh.pitch_rate);
  *roll_rate   = nsig_from_bang(xh.roll_rate);
#ifdef NSIG_VER2

#else
  *heading_rate = nsig_from_bang(xh.heading_rate);
#endif
  *lat = nsig_from_fourb_ang(xh.lat);
  *lon = nsig_from_fourb_ang(xh.lon);
  if(*lat > 180.0) *lat -= 360.0;
  if(*lon > 180.0) *lon -= 360.0;
  *alt       = NSIG_I2(xh.alt);
  *rvc       = NSIG_I2(xh.rad_vel_cor)/100.0; /* cm to m */
  *vel_east  = NSIG_I2(xh.vel_e)/100.0; /* cm to m */
  *vel_north = NSIG_I2(xh.vel_n)/100.0; /* cm to m */
  *vel_up    = NSIG_I2(xh.vel_u)/100.0; /* cm to m */
  return;
}

/** Main code **/

Radar * RSL_nsig_to_radar(char *filename)
{
  return RSL_nsig2_to_radar(filename);
}

Radar * RSL_nsig2_to_radar(char *filename)
{
  FILE *fp;
  /* RSL structures */
  Radar                    *radar;
  Ray                      *ray;
  
  int i, j, k, n;
  int year, month, day;
  int hour, minute, sec;
  int numbins, numsweep;
  int num_rays, sea_lvl_hgt;
  int radar_number, num_samples;
  int latd, latm, lats, lond, lonm, lons;
  int data_type;
  int bin_num;
  int sweep_year, sweep_day, sweep_month;
  int sweep_hour, sweep_minute, sweep_second;
  int sweep_sec;
  int z_flag_unc, z_flag_cor, v_flag, w_flag, speckle;
  int ant_scan_mode;
  float second;
  float pw;
  float bin_space;
  float prf, wave, beam_width;
  float vert_half_bw, horz_half_bw;
  float rng_last_bin;
  float rng_first_bin, freq;
  float max_vel, sweep_rate, azim_rate;
  float ray_data;
  float az1, az2;
  double tmp;
  float sqi, log, csr, sig, cal_dbz;
  char radar_type[50], state[2], city[15];
  char site_name[16];
  NSIG_Product_file *prod_file;
  short id;
  int data_mask, nrays;
  int masks[5];
  int nparams, nsweeps;
  NSIG_Sweep **nsig_sweep;
  NSIG_Ray *ray_p;
  int itype, ifield;
  unsigned short nsig_u2byte;   /* New for 2-byte data types, Aug 2009 */
  Sweep *sweep;
  int msec;
  float azm, elev, pitch, roll, heading, azm_rate, elev_rate,
    pitch_rate, roll_rate, heading_rate,
    lat, lon;
  int alt;  /* Altitude */
  float rvc;  /* Radial correction velocity m/s */
  float vel_east, vel_north, vel_up; /* Platform velocity vectors m/sec */
  int xh_size;
  float incr;
  extern int *rsl_qsweep; /* See RSL_read_these_sweeps in volume.c */
  extern int rsl_qsweep_max;
  extern float rsl_kdp_wavelen;

  radar = NULL;
  if (radar_verbose_flag)
    fprintf(stderr, "open file: %s\n", filename);
  
  /** Opening nsig file **/
  if((fp = fopen(filename, "r")) == NULL) return NULL;
  
#ifdef NSIG_VER2
  sprintf(radar_type, "nsig2");
  radar_number = 22;  /** Arbitrary number given to nsig2 data **/
#else
  sprintf(radar_type, "nsig");
  radar_number = 21;  /* What are these suppose to be? */
#endif
  sprintf(state,"NA");
  sprintf(city,"NA");
  
  /* MAINLINE CODE */
  
  prod_file = (NSIG_Product_file *)calloc(1, sizeof(NSIG_Product_file));

  n = nsig_read_record(fp, (char *)&prod_file->rec1);
  nsig_endianess(&prod_file->rec1);
  if (radar_verbose_flag)
    fprintf(stderr, "Read %d bytes for rec1.\n", n);

  id = NSIG_I2(prod_file->rec1.struct_head.id);
  if (radar_verbose_flag)
    fprintf(stderr, "ID = %d\n", (int)id);
  if (id != 7 && id != 27) { /* testing: Use 27 for Version 2 data */
    fprintf(stderr, "File is not a SIGMET version 1 nor version 2 raw product file.\n");
    free(prod_file);
    return NULL;
  }

  n = nsig_read_record(fp, (char *)&prod_file->rec2);
  if (radar_verbose_flag)
    fprintf(stderr, "Read %d bytes for rec2.\n", n);

   /** Test for scan mode -- If scan is a RHI will return NULL  **/
   /** because RSL can't handle RHI's.  In the future, replace  **/
   /** NULL will a routine to convert RHI's to RSL Format       **/
   ant_scan_mode =NSIG_I2(prod_file->rec2.task_config.scan_info.ant_scan_mode);
   if(ant_scan_mode == 2)
      {
      if (radar_verbose_flag)
      fprintf(stderr, "RHI scan detected. Unable to process, returning NULL.\n");
      /*      return NULL; */
      }
  
  /* Count the bits set in 'data_mask' to determine the number
   * of parameters present.
   */
  xh_size = NSIG_I2(prod_file->rec2.ingest_head.size_ext_ray_headers);
  nrays = NSIG_I2(prod_file->rec2.ingest_head.num_rays);
  if (radar_verbose_flag)
    fprintf(stderr, "Expecting %d rays in each sweep.\n", nrays);
#ifdef NSIG_VER2 
  memmove(&masks[0], prod_file->rec2.task_config.dsp_info.data_mask_cur.mask_word_0,
    sizeof(fourb));
  memmove(&masks[1], &prod_file->rec2.task_config.dsp_info.data_mask_cur.mask_word_1,
    4*sizeof(fourb));
  nparams = 0;
  for (j=0; j < 5; j++) {
    data_mask = masks[j];
    for (i=0; i<32; i++)
      nparams += (data_mask >> i) & 0x1;
  }
#else
  memmove(&data_mask, prod_file->rec2.task_config.dsp_info.data_mask, sizeof(fourb));
  for (nparams=i=0; i<32; i++)
    nparams += (data_mask >> i) & 0x1;
#endif

  /* Number of sweeps */
  nsweeps = NSIG_I2(prod_file->rec2.task_config.scan_info.num_swp);
  


   memmove(site_name, prod_file->rec1.prod_end.site_name, sizeof(prod_file->rec1.prod_end.site_name));
   site_name[sizeof(site_name)-1] = '\0';
  if (radar_verbose_flag) {
    fprintf(stderr, "nparams = %d, nsweeps = %d\n", nparams, nsweeps);
    fprintf(stderr, "Site name = <%s>\n", site_name);
  }

    /* nsig_sweep = nsig_read_sweep(fp, prod_file)
     *
     * Use: nsig_sweep[i]->ray[j]->range
     *
     * where 'range' is [0..nbins-1]
     */

    /*
     * All the information you need is in:
     *    prod_file->rec1
     *        .struct_head, .prod_config .prod_end
     *    prod_file->rec2
     *        .struct_head, .ingest_head, .task_config .device_stat,
     *        .dsp1, .dsp2
     *    nsig_sweep[0..nparams-1]  'nparams' is the true number
     *                              of parameters present.  You
     *                              must check the 'id' (or type)
     *                              to determine the field type.
     *                              So far seen, nparams <= 6.
     *    nsig_sweep[i]->bhdr     <NSIG_Raw_prod_bhdr>
     *    nsig_sweep[i]->idh      <NSIG_Ingest_data_header>
     *    nsig_sweep[i]->ray[j]   <NSIG_Ray *>
     *
     * Note:
     *    For extended header access, you'll typically use nsig_sweep[0]
     *    (double check the id) and the ray data allocated (nsig_ray->range)
     *    is a pointer to the extended header, either v0 or v1.
     *    You can typecast the pointer to NSIG_Ext_header_ver0 or
     *    NSIG_Ext_header_ver1, as you like.  To determine which
     *    version of the extended headers you have use:
     *      xh_size <= 20 for version 0, else version 1.
     *    Access:
     *      xh_size = NSIG_I2(prod_file->rec2.ingest_head.size_ext_ray_headers)
     *    
     * Functions:
     *    NSIG_I2(nsig_sweep[i]->idh.num_rays_act);   -- # of rays. (j)
     *    NSIG_I2(nsig_sweep[i]->ray[j]->h.num_bins); -- # of bins in a ray.
     *
     *    NSIG_I2(x), NSIG_I4(x)   - Convert data, x, to floating point.
     *
     *    IMPORTANT NOTE: It must be known whether or not to perform
     *                    byte-swapping.  To determine this, call
     *                    'nsig_endianess'.  It returns 0 for no-swapping
     *                    and 1 for swapping.  Additionally, it transparently
     *                    initializes the nsig library to automatically
     *                    swap when using NSIG_I2 or NSIG_I4.
     *                    The function 'nsig_read_sweep' automatically
     *                    calls 'nsig_endianess', too.
     */

   sea_lvl_hgt = NSIG_I2(prod_file->rec1.prod_end.grnd_sea_ht);

   if (radar_verbose_flag)
     fprintf(stderr, "sea: %d\n", sea_lvl_hgt);
   if (radar_verbose_flag)
     fprintf(stderr, "site_name: %s", site_name);
   
   /** Determine beamwidth from input variables (not saved in nsig file) **/
   if(strncmp(site_name,"mit",3) == 0 || strncmp(site_name,"MIT",3) == 0)
     beam_width = MIT_BEAMWIDTH;
   else if(strncmp(site_name,"tog",3) == 0 || strncmp(site_name,"TOG",3) == 0)
     beam_width = TOG_BEAMWIDTH;
   else if(strncmp(site_name,"kwa",3) == 0 || strncmp(site_name,"KWA",3) == 0)
     beam_width = KWA_BEAMWIDTH;
   else
     beam_width = DEFAULT_BEAMWIDTH;

   if (radar_verbose_flag)
     fprintf(stderr, "beamwidth: %f\n", beam_width);
   
   vert_half_bw = beam_width/2.0;
   horz_half_bw = beam_width/2.0;
   
   /** Reading date and time **/
   month = NSIG_I2(prod_file->rec2.ingest_head.start_time.month);
   year = NSIG_I2(prod_file->rec2.ingest_head.start_time.year);
   day = NSIG_I2(prod_file->rec2.ingest_head.start_time.day);
   sec = NSIG_I4(prod_file->rec2.ingest_head.start_time.sec);

   /* converting seconds since mid to time of day */
   tmp = sec/3600.0;
   hour = (int)tmp;
   tmp = (tmp - hour) * 60.0;
   minute = (int)tmp;
   second = (tmp - minute) * 60.0;

   /** records of the nsig file.                             **/
   num_rays = 0;
   pw = (NSIG_I4(prod_file->rec1.prod_end.pulse_wd))/100.0; /* pulse width */
   prf = NSIG_I4(prod_file->rec1.prod_end.prf);  /* pulse repetition frequency */
   wave = (NSIG_I4(prod_file->rec1.prod_end.wavelen))/100.0; /* wavelength (cm) */
   rsl_kdp_wavelen = wave;  /* EXTERNAL (volume.c) This sets KD_F and KD_INVF
                             * to operate with the proper wavelength.
                             */
   numbins = NSIG_I4(prod_file->rec1.prod_end.num_bin);   /* # bins in ray */
   rng_first_bin = (float)NSIG_I4(prod_file->rec1.prod_end.rng_f_bin)/100.0;
   rng_last_bin = (float)NSIG_I4(prod_file->rec1.prod_end.rng_l_bin)/100.0;
   bin_space = ((rng_last_bin-rng_first_bin)/(numbins-1.0)); /*rng res (m)*/
   
   numsweep = NSIG_I2(prod_file->rec2.task_config.scan_info.num_swp); /* # sweeps in volume */
   num_samples = NSIG_I2(prod_file->rec1.prod_end.num_samp);
   sweep_rate = 3.0; /** Approximate value -- info not stored **/
   azim_rate = sweep_rate*360.0/60.0;
   max_vel = wave*prf/(100.0*4.0);
   /* adjust nyquist for dual prt operations */
   switch(NSIG_I2(prod_file->rec1.prod_end.trg_rate)) {
     case 3: /* dual prt 4:5 */
       max_vel *= 4.0;
       break;
     case 2: /* dual prt 3:4 */
       max_vel *= 3.0;
       break;
     case 1: /* dual prt 2:3 */
       max_vel *= 2.0;
       break;
     default: {}
   }
   freq = (299793000.0/wave)*1.0e-4; /** freq in MHZ **/

   sqi = NSIG_I2(prod_file->rec2.task_config.calib_info.sqi)/256.0;
   log = NSIG_I2(prod_file->rec2.task_config.calib_info.noise)/16.0;
   csr = NSIG_I2(prod_file->rec2.task_config.calib_info.clutr_corr)/(-16.0);
   sig = NSIG_I2(prod_file->rec2.task_config.calib_info.power)/16.0;
   cal_dbz = NSIG_I2(prod_file->rec2.task_config.calib_info.cal_ref)/16.0;
   z_flag_unc = NSIG_I2(prod_file->rec2.task_config.calib_info.z_flag_unc);
   z_flag_cor = NSIG_I2(prod_file->rec2.task_config.calib_info.z_flag_cor);
   v_flag = NSIG_I2(prod_file->rec2.task_config.calib_info.v_flag);
   w_flag = NSIG_I2(prod_file->rec2.task_config.calib_info.w_flag);
   speckle = NSIG_I2(prod_file->rec2.task_config.calib_info.speckle);

   /** Verbose calibration information **/
   if (radar_verbose_flag)
      {
      fprintf(stderr, "LOG = %5.2f\n", log);
      fprintf(stderr, "SQI = %5.2f\n", sqi);
      fprintf(stderr, "CSR = %5.2f\n", csr);
      fprintf(stderr, "SIG = %5.2f\n", sig);
      fprintf(stderr, "Calibration reflectivity: %5.2f dBZ\n", cal_dbz);
      fprintf(stderr, "ZT flags: %d\n", z_flag_unc);  /** can find these **/
      fprintf(stderr, "DZ flags: %d\n", z_flag_cor);  /** defn in the    **/
      fprintf(stderr, "VR flags: %d\n", v_flag);      /** SIGMET Doc     **/
      fprintf(stderr, "SW flags: %d\n", w_flag);
      fprintf(stderr, "Flags: -3856  = SQI thresholding\n");
      fprintf(stderr, "       -21846 = LOG thresholding\n");
      fprintf(stderr, "       -24416 = LOG & SQI thresholding\n");
      fprintf(stderr, "       -24516 = LOG & SQI & SIG thresholding\n");
      fprintf(stderr, "speckle remover: %d\n", speckle);
      }
   
   if (radar_verbose_flag)
     fprintf(stderr, "vel: %f prf: %f\n", max_vel, prf);
   
   /** Extracting Latitude and Longitude from nsig file **/
   lat = nsig_from_fourb_ang(prod_file->rec2.ingest_head.lat_rad);
   lon = nsig_from_fourb_ang(prod_file->rec2.ingest_head.lon_rad);
   if(lat > 180.0) lat -= 360.0;
   if(lon > 180.0) lon -= 360.0;
   if (radar_verbose_flag)
     fprintf(stderr, "nsig_to_radar: lat %f, lon %f\n", lat, lon);
   /** Latitude deg, min, sec **/
   latd = (int)lat;
   tmp = (lat - latd) * 60.0;
   latm = (int)tmp;
   lats = (int)((tmp - latm) * 60.0);
   /** Longitude deg, min, sec **/
   lond = (int)lon;
   tmp = (lon - lond) * 60.0;
   lonm = (int)tmp;
   lons = (int)((tmp - lonm) * 60.0);
   
   /** Allocating memory for radar structure **/
   radar = RSL_new_radar(MAX_RADAR_VOLUMES);
   if (radar == NULL) 
      {
      fprintf(stderr, "nsig_to_radar: radar is NULL\n");
      free(prod_file);
      return NULL;
      }

   /** Filling Radar Header **/
   radar->h.month = month;
   radar->h.day = day;
   radar->h.year = year; /* Year 2000 compliant. */
   radar->h.hour = hour;
   radar->h.minute = minute;
   radar->h.sec = second;
   sprintf(radar->h.radar_type, "%s", radar_type);
   radar->h.number = radar_number;
   memmove(radar->h.name, site_name, sizeof(radar->h.name));
   memmove(radar->h.radar_name, site_name, sizeof(radar->h.radar_name));
   memmove(radar->h.city, city, sizeof(radar->h.city));
   memmove(radar->h.state, state, sizeof(radar->h.state));
   radar->h.latd = latd;
   radar->h.latm = latm;
   radar->h.lats = lats;
   radar->h.lond = lond;
   radar->h.lonm = lonm;
   radar->h.lons = lons;
   radar->h.height = (int)sea_lvl_hgt;
   radar->h.spulse = (int)(pw*1000);
   radar->h.lpulse = (int)(pw*1000);

   if (radar_verbose_flag) {
#ifdef NSIG_VER2
     fprintf(stderr, "\nSIGMET version 2 raw product file.\n");
#else
     fprintf(stderr, "\nSIGMET version 1 raw product file.\n");
#endif
     fprintf(stderr, "Date: %2.2d/%2.2d/%4.4d %2.2d:%2.2d:%f\n",
             radar->h.month, radar->h.day, radar->h.year,
             radar->h.hour, radar->h.minute, radar->h.sec);
     fprintf(stderr, "Name: ");
     for (i=0; i<sizeof(radar->h.name); i++)
       fprintf(stderr, "%c", radar->h.name[i]);
     fprintf(stderr, "\n");
     fprintf(stderr, "Lat/lon (%d %d' %d'', %d %d' %d'')\n",
             radar->h.latd, radar->h.latm, radar->h.lats,
             radar->h.lond, radar->h.lonm, radar->h.lons);
   }

   /** Converting data **/
   if (radar_verbose_flag) fprintf(stderr, "Expecting %d sweeps.\n", numsweep);
   for(i = 0; i < numsweep; i++)
      {
        nsig_sweep = nsig_read_sweep(fp, prod_file);
        if (nsig_sweep == NULL) { /* EOF possibility */
          if (feof(fp)) break;
          else continue;
        }
        if (rsl_qsweep != NULL) {
          if (i > rsl_qsweep_max) break;
          if (rsl_qsweep[i] == 0) continue;
        }
        if (radar_verbose_flag)
          fprintf(stderr, "Read sweep # %d\n", i);
    /* The whole sweep is 'nsig_sweep' ... pretty slick.
         *
         * nsig_sweep[itype]  -- [0..nparams], if non-null.
         */
    for (itype=0; itype<nparams; itype++) {
      if (nsig_sweep[itype] == NULL) continue;
          
      /** Reading date and time **/
      sweep_month = NSIG_I2(nsig_sweep[itype]->idh.time.month);
      sweep_year = NSIG_I2(nsig_sweep[itype]->idh.time.year);
      sweep_day = NSIG_I2(nsig_sweep[itype]->idh.time.day);
      sweep_sec = NSIG_I4(nsig_sweep[itype]->idh.time.sec);
#ifdef NSIG_VER2
      msec      = NSIG_I2(nsig_sweep[itype]->idh.time.msec);
      /*      printf("....... msec == %d\n", msec); */
#endif
      /* converting seconds since mid to time of day */
      tmp = sweep_sec/3600.0;
      sweep_hour = (int)tmp;
      tmp = (tmp - sweep_hour) * 60.0;
      sweep_minute = (int)tmp;
      sweep_second = sweep_sec - (sweep_hour*3600 + sweep_minute*60);

      num_rays = NSIG_I2(nsig_sweep[itype]->idh.num_rays_exp);

      data_type = NSIG_I2(nsig_sweep[itype]->idh.data_type);

      ifield = 0;
      switch (data_type) {
      case NSIG_DTB_EXH: 
          ifield = -1; 
          break;
      case NSIG_DTB_UCR:
      case NSIG_DTB_UCR2:
        ifield = ZT_INDEX;
        f      = ZT_F; 
        invf   = ZT_INVF;
        break;
      case NSIG_DTB_CR:
      case NSIG_DTB_CR2:
        ifield = DZ_INDEX;
        f      = DZ_F; 
        invf   = DZ_INVF;
        break;
      case NSIG_DTB_VEL:
      case NSIG_DTB_VEL2:
        ifield = VR_INDEX;
        f      = VR_F; 
        invf   = VR_INVF;
        break;
      case NSIG_DTB_WID:
      case NSIG_DTB_WID2:
        ifield = SW_INDEX;
        f      = SW_F; 
        invf   = SW_INVF;
        break;
      case NSIG_DTB_ZDR:             
      case NSIG_DTB_ZDR2:
        ifield = DR_INDEX;
        f      = DR_F; 
        invf   = DR_INVF;
        break;
      case NSIG_DTB_KDP:
        ifield = KD_INDEX;
        f      = KD_F; 
        invf   = KD_INVF;
        break;
      case NSIG_DTB_PHIDP:     /* SRB 990127 */
        ifield = PH_INDEX;
        f      = PH_F; 
        invf   = PH_INVF;
        break;
      case NSIG_DTB_RHOHV:     /* SRB 000414 */
        ifield = RH_INDEX;
        f      = RH_F; 
        invf   = RH_INVF;
        break;
      case NSIG_DTB_VELC:
      case NSIG_DTB_VELC2:
        ifield = VC_INDEX;
        f      = VC_F; 
        invf   = VC_INVF;
        break;
      case NSIG_DTB_KDP2:
        ifield = KD_INDEX;
        f      = KD_F; 
        invf   = KD_INVF;
        break;
      case NSIG_DTB_PHIDP2:
        ifield = PH_INDEX;
        f      = PH_F; 
        invf   = PH_INVF;
        break;
      case NSIG_DTB_RHOHV2:
        ifield = RH_INDEX;
        f      = RH_F; 
        invf   = RH_INVF;
        break;
      case NSIG_DTB_SQI:
      case NSIG_DTB_SQI2:
        ifield = SQ_INDEX;
        f      = SQ_F; 
        invf   = SQ_INVF;
        break;
      case NSIG_DTB_HCLASS:
      case NSIG_DTB_HCLASS2:
        ifield = HC_INDEX;
        f      = HC_F; 
        invf   = HC_INVF;
        break;
      default:
        fprintf(stderr,"Unknown field type: %d  Skipping it.\n", data_type);
        continue;
      }

      if (radar_verbose_flag)
        fprintf(stderr, "     nsig_sweep[%d], data_type = %d, rays(expected) = %d, nrays(actual) = %d\n", itype, data_type, num_rays, NSIG_I2(nsig_sweep[itype]->idh.num_rays_act));

      if (data_type != NSIG_DTB_EXH) {
        if (radar->v[ifield] == NULL) {
          if (rsl_qfield[ifield]) {
             radar->v[ifield] = RSL_new_volume(numsweep);
             radar->v[ifield]->h.f = f;
             radar->v[ifield]->h.invf = invf;
           } else {
             /* Skip this field, because, the user does not want it. */
             continue;
           }
        }
         if (radar->v[ifield]->sweep[i] == NULL)
           radar->v[ifield]->sweep[i] = RSL_new_sweep(num_rays);
         } 
      else
      continue;    /* Skip the actual extended header processing.
                    * This is different than getting it, so that
                    * the information is available for the other
                    * fields when filling the RSL ray headers.
                    */

      /** DATA conversion time **/
      sweep = radar->v[ifield]->sweep[i];
      sweep->h.f = f;
      sweep->h.invf = invf;
      sweep->h.sweep_num = i;
      sweep->h.beam_width = beam_width;
      sweep->h.vert_half_bw = vert_half_bw;
      sweep->h.horz_half_bw = horz_half_bw;
      elev = nsig_from_bang(nsig_sweep[itype]->idh.fix_ang);
      sweep->h.elev = elev;
      
      for(j = 0; j < num_rays; j++)
        {
          ray_p = nsig_sweep[itype]->ray[j];
          if (ray_p == NULL) continue;
          bin_num = NSIG_I2(ray_p->h.num_bins);

          /* Load extended header information, if available.
           * We need to pass the entire nsig_sweep and search for
           * the extended header field (it may not be data_type==0).
           */
          nsig2_get_extended_header_info(nsig_sweep, xh_size, j, nparams,
                       &msec, &azm, &elev,
                       &pitch, &roll, &heading,
                       &azm_rate, &elev_rate,
                       &pitch_rate, &roll_rate, &heading_rate,
                       &lat, &lon, &alt, &rvc,
                       &vel_east, &vel_north, &vel_up);
          

          if (radar->v[ifield]->sweep[i]->ray[j] == NULL)
            radar->v[ifield]->sweep[i]->ray[j] = RSL_new_ray(bin_num);
          ray = radar->v[ifield]->sweep[i]->ray[j];
          ray->h.f = f;
          ray->h.invf = invf;
          /** Ray is at nsig_sweep[itype].ray->... **/
          /** Loading nsig data into data structure **/
                  
          ray->h.month  = sweep_month;
          ray->h.day    = sweep_day;
          ray->h.year   = sweep_year; /* Year 2000 compliant. */
          ray->h.hour   = sweep_hour;
          ray->h.minute = sweep_minute;
          if (msec == 0) { /* No extended header */
            ray->h.sec  = NSIG_I2(ray_p->h.sec) + sweep_second;
            elev = sweep->h.elev;
          } else
            ray->h.sec  = sweep_second + msec/1000.0;

          /* add time ... handles end of min,hour,month,year and century. */
          if (ray->h.sec >= 60) /* Should I fix the time no matter what? */
            RSL_fix_time(ray);  /* Repair second overflow. */

          ray->h.ray_num    = j;
          ray->h.elev_num   = i;
          ray->h.range_bin1 = (int)rng_first_bin;
          ray->h.gate_size  = (int)(bin_space+.5); /* Nearest int */
          ray->h.vel_res    = bin_space;
          ray->h.sweep_rate = sweep_rate;
          ray->h.prf        = (int)prf;
            if (prf != 0)
              ray->h.unam_rng = 299793000.0 / (2.0 * prf * 1000.0);  /* km */
            else
              ray->h.unam_rng = 0.0;
            ray->h.fix_angle = (float)sweep->h.elev;
          ray->h.azim_rate  = azim_rate;
          ray->h.pulse_count = (float)num_samples;
          ray->h.pulse_width = pw;
          ray->h.beam_width  = beam_width;
          ray->h.frequency   = freq / 1000.0;  /* GHz */
          ray->h.wavelength  = wave/100.0;     /* meters */
          ray->h.nyq_vel     = max_vel;        /* m/s */
          if (elev == 0.) elev = sweep->h.elev;
          ray->h.elev        = elev;
          /* Compute mean azimuth angle for ray. */
          az1 = nsig_from_bang(ray_p->h.beg_azm);
          az2 = nsig_from_bang(ray_p->h.end_azm);
          /*          printf("az1, %f, az2 %f\n", az1, az2); */
          if(az1 > az2)
            if((az1 - az2) > 180.0) az2 += 360.0;
            else
              ;
          else
            if((az2 - az1) > 180.0) az1 += 360.0;

          az1 = (az1 + az2) / 2.0;
          if (az1 > 360) az1 -= 360;
          ray->h.azimuth     = az1;

          /* From the extended header information, we learn the following. */
          ray->h.pitch        = pitch;
          ray->h.roll         = roll;
          ray->h.heading      = heading;
          ray->h.pitch_rate   = pitch_rate;
          ray->h.roll_rate    = roll_rate;
          ray->h.heading_rate = heading_rate;
          ray->h.lat          = lat;
          ray->h.lon          = lon;
          ray->h.alt          = alt;
          ray->h.rvc          = rvc;
          ray->h.vel_east     = vel_east;
          ray->h.vel_north    = vel_north;
          ray->h.vel_up       = vel_up;

          /*          printf("Processing sweep[%d]->ray[%d]: %d %f %f %f %f %f %f %f %f %d nbins=%d, bin1=%d gate=%d\n",
                 i, j, msec, ray->h.sec, ray->h.azimuth, ray->h.elev, ray->h.pitch, ray->h.roll, ray->h.heading, ray->h.lat, ray->h.lon, ray->h.alt, ray->h.nbins, ray->h.range_bin1, ray->h.gate_size);
                 */
	  /* TODO: ingest data header contains a value for bits-per-bin.
	   * This might be of use to allocate an array for ray->range with
	   * either 1-byte or 2-byte elements.  Then there's no need for
	   * memmove() whenever we need 2 bytes.
	   */

          if (data_type == NSIG_DTB_EXH) continue;
          ray_data = 0;
          for(k = 0; k < bin_num; k++) {
            switch(data_type) {
            case NSIG_DTB_UCR:
            case NSIG_DTB_CR:
              if (ray_p->range[k] == 0) ray_data = NSIG_NO_ECHO;
              else ray_data = (float)((ray_p->range[k]-64.0)/2.0);
              break;
	    /* Simplified the velocity conversion for NSIG_DTB_VEL, using
	     * formula from IRIS Programmer's Manual. BLK, Oct 9 2009.
	     */
            case NSIG_DTB_VEL:
              if (ray_p->range[k] == 0) ray_data = NSIG_NO_ECHO;
              else ray_data = (float)((ray_p->range[k]-128.0)/127.0)*max_vel;
              break;
              
            case NSIG_DTB_WID:
              if (ray_p->range[k] == 0) ray_data = NSIG_NO_ECHO;
              else ray_data =(float)((ray_p->range[k])/256.0)*max_vel;
              break;
              
            case NSIG_DTB_ZDR:
              if (ray_p->range[k] == 0) ray_data = NSIG_NO_ECHO;
              else ray_data = (float)((ray_p->range[k]-128.0)/16.0);
              break;

            case NSIG_DTB_KDP:
		if (ray_p->range[k] == 0 || ray_p->range[k] == 255 ||
		    rsl_kdp_wavelen == 0.0) {
		  ray_data = NSIG_NO_ECHO;
		  break;
		}
		if (ray_p->range[k] < 128)
		  ray_data = (-0.25 *
		    pow((double)600.0,(double)((127-ray_p->range[k])/126.0))) /
		      rsl_kdp_wavelen;
		else if (ray_p->range[k] > 128)
		  ray_data = (0.25 *
		    pow((double)600.0,(double)((ray_p->range[k]-129)/126.0))) /
		      rsl_kdp_wavelen;
		else
		  ray_data = 0.0;
                break;

            case NSIG_DTB_PHIDP:
              if (ray_p->range[k] == 0 || ray_p->range[k] == 255) 
                ray_data = NSIG_NO_ECHO;
	      else
                ray_data = 180.0*((ray_p->range[k]-1.0)/254.0);
              break;

            case NSIG_DTB_RHOHV:
              if (ray_p->range[k] == 0 || ray_p->range[k] == 255) 
                ray_data = NSIG_NO_ECHO;
              else 
                ray_data = sqrt((double)((ray_p->range[k]-1.0)/253.0));
              break;

            case NSIG_DTB_HCLASS:
              if (ray_p->range[k] == 0 || ray_p->range[k] == 255) 
                ray_data = NSIG_NO_ECHO;
	      else
                ray_data = ray_p->range[k];
	      break;

            case NSIG_DTB_SQI:
              if (ray_p->range[k] == 0) ray_data = NSIG_NO_ECHO;
              else ray_data = (float)sqrt((ray_p->range[k]-1.0)/253.0);
              break;

            case NSIG_DTB_VELC:
              if (ray_p->range[k] == 0) ray_data = NSIG_NO_ECHO;
              else {
		incr=75./127.;  /*  (+|- 75m/s) / 254 values */
	        ray_data = (float)(ray_p->range[k]-128)*incr;
	      }
              break;

            case NSIG_DTB_UCR2:
            case NSIG_DTB_CR2:
            case NSIG_DTB_VEL2:
            case NSIG_DTB_VELC2:
            case NSIG_DTB_ZDR2:
            case NSIG_DTB_KDP2:
	      memmove(&nsig_u2byte, &ray_p->range[2*k], 2);
	      nsig_u2byte = NSIG_I2((unsigned char *) &nsig_u2byte);
	      if (nsig_u2byte == 0 || nsig_u2byte == 65535)
	        ray_data = NSIG_NO_ECHO2;
	      else ray_data = (float)(nsig_u2byte-32768)/100.;
	      break;

            case NSIG_DTB_WID2:
	      memmove(&nsig_u2byte, &ray_p->range[2*k], 2);
	      nsig_u2byte = NSIG_I2((unsigned char *) &nsig_u2byte);
	      if (nsig_u2byte == 0 || nsig_u2byte == 65535)
	        ray_data = NSIG_NO_ECHO2;
	      else ray_data = (float)nsig_u2byte/100.;
	      break;

            case NSIG_DTB_PHIDP2:
	      memmove(&nsig_u2byte, &ray_p->range[2*k], 2);
	      nsig_u2byte = NSIG_I2((unsigned char *) &nsig_u2byte);
	      if (nsig_u2byte == 0 || nsig_u2byte == 65535)
	        ray_data = NSIG_NO_ECHO;
	      else
	        ray_data = 360.*(nsig_u2byte-1)/65534.;
	      break;

            case NSIG_DTB_SQI2:
            case NSIG_DTB_RHOHV2:
	      memmove(&nsig_u2byte, &ray_p->range[2*k], 2);
	      nsig_u2byte = NSIG_I2((unsigned char *) &nsig_u2byte);
	      if (nsig_u2byte == 0 || nsig_u2byte == 65535)
	        ray_data = NSIG_NO_ECHO2;
	      else ray_data = (float)(nsig_u2byte-1)/65533.;
              break;

            case NSIG_DTB_HCLASS2:
	      memmove(&nsig_u2byte, &ray_p->range[2*k], 2);
	      nsig_u2byte = NSIG_I2((unsigned char *) &nsig_u2byte);
	      if (nsig_u2byte == 0 || nsig_u2byte == 65535)
	        ray_data = NSIG_NO_ECHO2;
	      else
                ray_data = nsig_u2byte;
            }

            if (ray_data == NSIG_NO_ECHO || ray_data == NSIG_NO_ECHO2)
              ray->range[k] = ray->h.invf(BADVAL);
            else
              ray->range[k] = ray->h.invf(ray_data);

            /*
            if (data_type == NSIG_DTB_KDP)
            printf("v[%d]->sweep[%d]->ray[%d]->range[%d] = %f, %d, %f\n", 
                   ifield, i, j, k, ray->h.f(ray->range[k]), 
                   (int)ray_p->range[k], ray_data);
            */
          }
        }
        }
        nsig_free_sweep(nsig_sweep);
      }

   /* Do not reset radar->h.nvolumes. It is already set properly. */
   if (radar_verbose_flag)
     fprintf(stderr, "Max index of radar->v[0..%d]\n", radar->h.nvolumes);
   

   /** close nsig file **/
   fclose(fp);
   free(prod_file);

   radar = RSL_prune_radar(radar);
   /** return radar pointer **/
   return radar;
}
