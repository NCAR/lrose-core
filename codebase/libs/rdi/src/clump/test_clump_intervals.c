/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1992 - 2012 */
/* ** University Corporation for Atmospheric Research(UCAR) */
/* ** National Center for Atmospheric Research(NCAR) */
/* ** Research Applications Laboratory(RAL) */
/* ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
/* ** 2012/9/18 21:12:47 */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* test_clump_intervals.c - test the clump intervals code for a 2d data set */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <malloc.h>
#include "clump.h"

char *prog_name;

int num_intervals;
int num_rows;
Interval intervals[1024];
extern int malloc_amount;

main(argc, argv)
     int argc;
     char **argv;
{
  int i;
  int num_clumps;

  if (argc != 1)
    {
      fprintf(stderr, "usage: %s\n", argv[0]);
      exit(0);
    }

  init_intervals();

  printf("1 malloc_amount is %d\n", malloc_amount);
    
  num_clumps = clump_2d(intervals, num_intervals, num_rows);
  printf("num_clumps = %d\n", num_clumps);

  printf("2 malloc_amount is %d\n", malloc_amount);

  /* print out the interval information */
  for (i=0; i<num_intervals; i++)
    printf("interval %d: row %d, begin %d, end %d, ov_north %d, %d, ov_south %d, %d, id %d, index %d\n",
	   i,
	   intervals[i].row,
	   intervals[i].begin,
	   intervals[i].end,
	   intervals[i].overlaps[NORTH_INTERVAL][0],
	   intervals[i].overlaps[NORTH_INTERVAL][1],
	   intervals[i].overlaps[SOUTH_INTERVAL][0],
	   intervals[i].overlaps[SOUTH_INTERVAL][1],
	   intervals[i].id,
	   intervals[i].index);

  /* reset the interval ids */
  for (i=0; i<num_intervals; i++)
    intervals[i].id = 0;

  num_clumps = clump_2d(intervals, num_intervals, num_rows);
  printf("num_clumps = %d\n", num_clumps);

  printf("5 malloc_amount is %d\n", malloc_amount);
  /* print out the interval information */
  for (i=0; i<num_intervals; i++)
    printf("interval %d: row %d, begin %d, end %d, ov_north %d, %d, ov_south %d, %d, id %d\n",
	   i,
	   intervals[i].row,
	   intervals[i].begin,
	   intervals[i].end,
	   intervals[i].overlaps[NORTH_INTERVAL][0],
	   intervals[i].overlaps[NORTH_INTERVAL][1],
	   intervals[i].overlaps[SOUTH_INTERVAL][0],
	   intervals[i].overlaps[SOUTH_INTERVAL][1],
	   intervals[i].id);

  printf("6 malloc_amount is %d\n", malloc_amount);
  return(0);
}

init_intervals()
{
  int a[10];
  int i;
  int j;

  for (i=0; i<10; i++)
    a[i] = 2*i+1;

  for (j=0; j<10; j++)
    for (i=0; i<10; i++)
      {
	intervals[10*j + i].row = j;
	intervals[10*j+i].begin = a[i];
	intervals[10*j+i].end = a[i];
      }

  num_intervals = 100;
  num_rows = 20;
}  

init_intervals6()
{
  int i;
  int j;

  for (i=0, j=0; i<10; i+=2, j++)
    {
      intervals[i].row = j;
      intervals[i].begin = j;
      intervals[i].end = j+1;

      intervals[i+1].row = j;
      intervals[i+1].begin = 11-j-1;
      intervals[i+1].end = 11-j;
    }

  intervals[10].row = 5;
  intervals[10].begin = 5;
  intervals[10].end = 6;


  for (i=11; i<20; i++)
    {
      intervals[i].row = i-6;
      intervals[i].begin = 5;
      intervals[i].end = 5;
    }

  num_intervals = 20;
  num_rows = 20;
}  

init_intervals3()
{
  int i;
  int j;

  for (i=0, j=0; i<10; i+=2, j++)
    {
      intervals[i].row = j;
      intervals[i].begin = j;
      intervals[i].end = j+1;

      intervals[i+1].row = j;
      intervals[i+1].begin = 11-j-1;
      intervals[i+1].end = 11-j;
    }

  intervals[10].row = 5;
  intervals[10].begin = 5;
  intervals[10].end = 6;


  for (i=11; i<20; i++)
    {
      intervals[i].row = i;
      intervals[i].begin = 0;
      intervals[i].end = 0;
    }

  num_intervals = 20;
  num_rows = 20;
}  

init_intervals11()
{
  int i;

  for (i=0; i<5; i++)
    {
      intervals[i].row = i;
      intervals[i].begin = i;
      intervals[i].end = i+1;
    }
  for (i=5; i<10; i++)
    {
      intervals[i].row = i;
      intervals[i].begin = i+2;
      intervals[i].end = i+3;
    }

  num_intervals = 10;
  num_rows = 10;
}  


