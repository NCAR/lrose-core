
#include <stdio.h>
#include <string.h>

/* Re-coded from Fortran, Niles Oien Jan 1999.

There is definitely room for improvement in the way this is done - all
that has been perpetrated here is a translation from Fortran. Many of the
file handling routines could be done by C routines rather
than calling 'system'. I am not sure that this code will be retained,
however, which is why I am not upgrading it now.

      subroutine read_sounding_class(sounding_dir,pres_li,tli,
     1                               tli_default,badsf,debug)

      dimension press(1000),temp(1000)
      character fname1*64,fname2*128,dummyc*72
      character*128 comm
      character*64 sounding_dir
      logical debug


c.. first find latest sounding
*/

void read_sounding_class(char *sounding_dir,
			 float pres_li, float *tli,
			 float tli_default, float badsf,
			 int debug);
