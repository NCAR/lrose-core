/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1992 - 2012 */
/* ** University Corporation for Atmospheric Research(UCAR) */
/* ** National Center for Atmospheric Research(NCAR) */
/* ** Research Applications Laboratory(RAL) */
/* ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
/* ** 2012/9/18 21:12:38 */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* print_mallinfo.c - print mallinfo information */

#include <sys/types.h>
#include <malloc.h>


print_mallinfo(int id)
{
  struct mallinfo malli;

  malli = mallinfo();
  
  printf("mallinfo id %d\n", id);
  printf("arena space %d\n", malli.arena);
  printf("number of ordinary blocks %d\n", malli.ordblks);
  printf("number of small blocks %d\n", malli.smblks);
  printf("space in holding block headers %d\n", malli.hblkhd);
  printf("number of holding blocks %d\n", malli.hblks);
  printf("space in small blocks in use %d\n", malli.usmblks);
  printf("space in free small blocks %d\n", malli.fsmblks);
  printf("space in ordinary blocks in use %d\n", malli.uordblks);
  printf("space in free ordinary blocks %d\n", malli.fordblks);
  printf("space penalty if keep option is used %d\n", malli.keepcost);
  
}


