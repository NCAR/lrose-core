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
FILE *nsig_open(char *file_name)
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
int nsig_read_record(FILE *fp, char *nsig_rec)
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
void nsig_close(FILE *fp)
   {
     rsl_pclose(fp);
   }

static int do_swap;

int nsig_endianess(NSIG_Record1 *rec1)
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

short NSIG_I2 (twob x)
{/* x is already a pointer. */
  short s;
  memmove(&s, x, sizeof(twob));
  if (do_swap) swap_2_bytes(&s);
  return s;
}

int NSIG_I4 (fourb x)
{ /* x is already a pointer. */
  int i;
  memmove(&i, x, sizeof(fourb));
  if (do_swap) swap_4_bytes(&i);
  return i;
}


void nsig_free_ray(NSIG_Ray *r)
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

int nsig_read_chunk(FILE *fp, char *chunk)
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

NSIG_Ext_header_ver0 *nsig_read_ext_header_ver0(FILE *fp)
{
  NSIG_Data_record chunk;
  int n;
  NSIG_Ext_header_ver0 *xh0;
  xh0 = NULL;
  n = nsig_read_chunk(fp, (char *)chunk);
  if (n <= 0) return xh0;
#ifdef Vprint
  printf("Ver0 x-header.  %d bytes found.\n", n);
#endif
  n     -= sizeof(NSIG_Ray_header);
  xh0 = (NSIG_Ext_header_ver0 *)calloc(1, n);
  if (xh0) memmove(xh0, &chunk[sizeof(NSIG_Ray_header)], n);
  return xh0;
}


NSIG_Ext_header_ver1 *nsig_read_ext_header_ver1(FILE *fp)
{
  NSIG_Data_record chunk;
  int n;
  NSIG_Ext_header_ver1 *xh1;
  xh1 = NULL;
  n = nsig_read_chunk(fp, (char *)chunk);
  if (n <= 0) return xh1;
#ifdef Vprint
  printf("Ver1 x-header.  %d bytes found.\n", n);
#endif
  n     -= sizeof(NSIG_Ray_header);
  xh1 = (NSIG_Ext_header_ver1 *)calloc(1, n);
  if (xh1) memmove(xh1, &chunk[sizeof(NSIG_Ray_header)], n);
  return xh1;
}

NSIG_Ray *nsig_read_ray(FILE *fp)
{
  int n, nbins;
  NSIG_Ray_header rayh;
  static NSIG_Data_record chunk;
  NSIG_Ray *ray;
  
  n = nsig_read_chunk(fp, (char *)chunk);
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


NSIG_Sweep **nsig_read_sweep(FILE *fp, NSIG_Product_file *prod_file)
{
  NSIG_Sweep **s;
  int i, n;
  static NSIG_Ingest_data_header **idh = NULL;
  static NSIG_Raw_prod_bhdr *bhdr = NULL;
  NSIG_Ray *nsig_ray;
  int data_mask, iray, nrays[12], max_rays;
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
float nsig_from_bang(bang in)
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
float nsig_from_fourb_ang(fourb ang)
   {
   double result,maxval;
   unsigned int bi_ang;

   maxval = 4294967296.0;

   memmove(&bi_ang, ang, sizeof(fourb));
   if (do_swap) swap_4_bytes(&bi_ang);
   
   result = ((double)(bi_ang)/maxval) * 360.0;
   
   return ((float)result);
   }
