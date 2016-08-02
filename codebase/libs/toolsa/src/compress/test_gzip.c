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
 * test_gzip.c
 *
 * Tests compression utilities using GZIP compression
 *
 * See gzip_compress.c
 *
 * Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, USA
 *
 * July 1999
 *
 **********************************************************************/

#include <toolsa/umisc.h>
#include <toolsa/compress.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void usage(FILE *out);

int main(int argc, char **argv)

{

  char *infilename;
  char outfilename[MAX_PATH_LEN];
  char gzfilename[MAX_PATH_LEN];
  int compress;
  double compression_percent;
  unsigned int inlen;
  unsigned int outlen, gzlen;
  struct stat filestat;
  void *inbuf, *outbuf;
  FILE *fin, *fout, *gzout;

  if (argc != 2) {
    usage(stderr);
    return (-1);
  }

  infilename = argv[1];

  if (!strncmp(infilename + strlen(infilename) - 5, ".gzip", 5)) {
    fprintf(stderr, "Uncompressing file '%s'\n", infilename);
    STRncopy(outfilename, infilename, strlen(infilename) - 4);
    compress = 0;
  } else {
    fprintf(stderr, "Compressing file '%s'\n", infilename);
    sprintf(outfilename, "%s.gzip", infilename);
    sprintf(gzfilename, "%s.gz", infilename);
    compress = 1;
  }

  /*
   * read file buffer
   */

  if (stat(infilename, &filestat)) {
    fprintf(stderr, "Cannot stat file '%s'\n", infilename);
    perror(infilename);
    return (-1);
  }
  
  inlen = filestat.st_size;
  inbuf = umalloc_min_1(inlen);
  
  if ((fin = fopen(infilename, "r")) == NULL) {
    fprintf(stderr, "Cannot open file '%s' for reading\n", infilename);
    perror(infilename);
    return (-1);
  }
  if (fread(inbuf, 1, inlen, fin) != inlen) {
    fprintf(stderr, "Cannot read file '%s'\n", infilename);
    perror(infilename);
    fclose(fin);
    return (-1);
  }
  fclose(fin);

  /*
   * compress or uncompress buffer
   */

  if (compress) {

    outbuf = gzip_compress(inbuf, inlen, &outlen);

    if (outbuf == NULL) {
      fprintf(stderr, "Compression failed\n");
      return (-1);
    }

    fprintf(stdout, "Compressed %d bytes into %d bytes\n",
	    inlen, outlen);
    
    compression_percent = ((double) (inlen - outlen) / (double) inlen) * 100.0;
    fprintf(stdout, "%.1f percent compression, %.1f percent left\n",
	    compression_percent, 100.0 - compression_percent);

  } else {

    outbuf = ta_decompress(inbuf, &outlen);

    if (outbuf == NULL) {
      fprintf(stderr, "Decompression failed\n");
      return (-1);
    }

    fprintf(stdout, "Uncompressed %d bytes into %d bytes\n",
	    inlen, outlen);

  }

  /*
   * write output files
   */
  
  if ((fout = fopen(outfilename, "w")) == NULL) {
    fprintf(stderr, "Cannot open file '%s' for writing\n", outfilename);
    perror(outfilename);
    ta_compress_free(outbuf);
    return (-1);
  }
  if (fwrite(outbuf, 1, outlen, fout) != outlen) {
    fprintf(stderr, "Cannot write file '%s'\n", outfilename);
    perror(outfilename);
    fclose(fout);
    ta_compress_free(outbuf);
    return (-1);
  }
  fclose(fout);

  if ((gzout = fopen(gzfilename, "w")) == NULL) {
    fprintf(stderr, "Cannot open file '%s' for writing\n", gzfilename);
    perror(gzfilename);
    ta_compress_free(outbuf);
    return (-1);
  }

  gzlen = outlen - sizeof(compress_buf_hdr_t);
  if (fwrite((char *) outbuf + sizeof(compress_buf_hdr_t),
	     1, gzlen, gzout) != gzlen) {
    fprintf(stderr, "Cannot write file '%s'\n", gzfilename);
    perror(gzfilename);
    fclose(gzout);
    ta_compress_free(outbuf);
    return (-1);
  }
  fclose(gzout);

  /*
   * free up
   */
  
  ta_compress_free(outbuf);
  
  return (0);

}

static void usage(FILE *out)

{
  fprintf(out, "Usage: test_gzip filename\n");
  fprintf(out,
	  "Notes:\n"
	  "  If filename does not have .gzip extension, it is \n"
	  "    compressed and stored in file with .gzip extension.\n"
	  "  If filename has .gzip extension, it is uncompressed\n"
	  "    and stored in file without .gzip extension.\n");
}
