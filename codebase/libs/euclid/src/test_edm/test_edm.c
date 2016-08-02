/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1992 - 2010 */
/* ** University Corporation for Atmospheric Research(UCAR) */
/* ** National Center for Atmospheric Research(NCAR) */
/* ** Research Applications Laboratory(RAL) */
/* ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
/* ** 2010/10/7 23:12:39 */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <euclid/clump.h>

#define NX 40
#define NY 40

static void print_array(char *label,
			unsigned char *array)

{
  
  int i, ix, iy;
  
  fprintf(stderr, "***** %s *****\n", label);
  i = 0;
  for (iy = 0; iy < NY; iy++) {
    for (ix = 0; ix < NX; ix++) {
      fprintf(stdout, "%3d ", array[i++]);
    } /* ix */
    fprintf(stdout, "\n");
  } /* iy */

}
    
int main (int argc, char **argv)

{

  unsigned char in_array[NX*NY];
  unsigned char edm_array[NX*NY];
  unsigned char inv_edm_array[NX*NY];
  char *input_file_path;
  int i;
  int npoints = NX * NY;
  int inval;
  FILE *infile;
  
  if (argc != 2) {
    fprintf(stderr, "Usage: %s input_input_file_path\n", argv[0]);
    return (-1);
  }

  input_file_path = argv[1];

  if ((infile = fopen(input_file_path, "r")) == NULL) {
    fprintf(stderr, "ERROR - %s\n", argv[0]);
    perror(input_file_path);
    return (-1);
  }
  
  for (i = 0; i < npoints; i++) {
    fscanf(infile, "%d", &inval);
    in_array[i] = inval;
  } /* i */

  fclose (infile);

  print_array("Input", in_array);
    
  EG_simple_edm_2d(in_array, edm_array, NX, NY);

  print_array("Edm", edm_array);

  EG_inverse_edm_2d(in_array, inv_edm_array, NX, NY);

  print_array("Edm", inv_edm_array);

  return (0);

}

