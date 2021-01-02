/* r.connectivity

   This program reads a rast map and finds all connected areas with
   a raster value greater than a specific threshold.  It then calculates
   the connectivity function and writes it an output text file.


   Usage: r.catchment dem=raster map
                      input=raster map
                      connect=text output file with connectivity function
                      output=raster file of connected regions
                      threshold=threshold above which connectivity is considered
                      -p treat threshold as a percentile
                      -z treat zero's as real data

   Copyright (C) Andrew Western, August, 1999
*/

#include <stdio.h>
#include <time.h>
/* #include "gis.h" */
#include <math.h>
#include "nrutil.h"

#define MAX_BINS 200
#define INIT_CALL -2 /* must be negative */
#define UP 1.0
#define DOWN -1.0
#define NO_DATA 0
#define LOW 2
#define HIGH 3
#define HIT_EDGE 4

/* structures */
struct bin_struct {
  double min_range, max_range, sum_distance, sum_directional_distance;
  int num_pairs, num_connected,num_directional, num_dir_connected; 
};

/* external variables */
double x_res, y_res, diag_res;
long rows, cols;
struct Flag *directional_flag, *percentile_flag, *flat_flag, 
  *two_directions_flag,*up_down_flag;
int use_mask=0;

/* functions */
int find_connected(int **, int **, long, long, int);
void calc_thresh (int **data, int **mask, int *threshold);
void sort (unsigned long, int *);
double slope(long r1,long r2,long c1,long c2, int **elevation);

int search_up (long cell_row, long cell_col, long calling_r, long calling_c, 
  int **elevation, int **up_down_slope,int **connected,int region, 
  int descriptor);

int search_down (long cell_row, long cell_col, long calling_r, long calling_c, 
  int **elevation, int **up_down_slope,int **connected,int region, 
  int descriptor);

int find_steepest(double *slope_dat, int **elevation, double *steepest, 
  double *second_steepest, long r, long c);

int main(int argc, char **argv)
{
  struct Option *in_opt, *dem_opt, *connect_opt, *out_cell_opt, *threshold_opt;
  struct Option *bin_opt;
  extern struct Flag *percentile_flag, *directional_flag, *flat_flag;
  extern struct Flag *two_directions_flag,*up_down_flag;
  struct Flag *zero_flag;
  struct Cell_head inhead;
  double north,south,east,west;
  long r,c,rr,cc;
  long min_row, max_row, min_col, max_col;
  long max_row_dif, max_col_dif;
  char *dem_name;
  char *dem_mapset;
  char *in_name;
  char *in_mapset;
  char *out_name;
  char *out_mapset;
  char *mask_name;
  char *mask_mapset;
  int dem_file;
  int in_file;
  int out_file;
  int mask_file;
  extern int use_mask;
  FILE *connect_file;
  FILE *bin_file;
  CELL *in_buf;
  CELL *dem_buf;
  CELL *out_buf;
  CELL *mask_buf;
  int **data;
  int **elevation;
  int **connected;
  int **mask;
  int **up_down_slope;
  char msg[200];
  char in_str[1000];
  int threshold;
  int i;
  int region=1;

  int j,k;

  extern double x_res, y_res, diag_res;
  extern long rows, cols;

  double x1, x2, y1, y2, distance;
  
  struct bin_struct bin[MAX_BINS];
  int num_bins;

  clock();

  G_gisinit (argv[0]);

  dem_opt = G_define_option();
  dem_opt->key = "dem";
  dem_opt->answer=NULL;
  dem_opt->type = TYPE_STRING;
  dem_opt->description = "input dem";
  dem_opt->required = NO;
 
  in_opt = G_define_option();
  in_opt->key = "input";
  in_opt->answer=NULL;
  in_opt->type = TYPE_STRING;
  in_opt->description = "input raster";
  in_opt->required = YES;
 
  bin_opt = G_define_option();
  bin_opt->key = "bin";
  bin_opt->answer=NULL;
  bin_opt->type = TYPE_STRING;
  bin_opt->description = "range bins for calculating connectivity";
  bin_opt->required = YES;
 
  out_cell_opt = G_define_option();
  out_cell_opt->key = "output";
  out_cell_opt->answer=NULL;
  out_cell_opt->type = TYPE_STRING;
  out_cell_opt->description = "output map of connected regions";
  out_cell_opt->required = NO;
 
  connect_opt = G_define_option();
  connect_opt->key = "connect";
  connect_opt->answer=NULL;
  connect_opt->type = TYPE_STRING;
  connect_opt->description = "file of connectivity function";
  connect_opt->required = YES;
 
  threshold_opt = G_define_option();
  threshold_opt->key = "threshold";
  threshold_opt->answer=NULL;
  threshold_opt->type = TYPE_STRING;
  threshold_opt->description = "value  for thresholding data at";
  threshold_opt->required = YES;

  directional_flag = G_define_flag();
  directional_flag->key = 'd';
  directional_flag->description = "consider directional connectivity";

  zero_flag = G_define_flag();
  zero_flag->key = 'z';
  zero_flag->description = "consider zero data within the masked area";

  percentile_flag = G_define_flag();
  percentile_flag->key = 'p';
  percentile_flag->description = "use a percentile to determine threshold";
 
  flat_flag = G_define_flag();
  flat_flag->key = 'f';
  flat_flag->description = "include flats when doing directional connectivity";
 
  two_directions_flag = G_define_flag();
  two_directions_flag->key = '2';
  two_directions_flag->description = "include two steepest directions when \
	doing directional connectivity";
 
  up_down_flag = G_define_flag();
  up_down_flag->key = 'u';
  up_down_flag->description = "allow directional search to zig-zag up and down";
 
  if (G_parser(argc,argv))
    exit(1);
  
  in_name=in_opt->answer;
  dem_name=dem_opt->answer; 
  out_name=out_cell_opt->answer; 
  if (directional_flag->answer && !dem_opt->answer) 
    G_fatal_error("must supply dem to calculate gradient for \
	directional connectivity");

  if(sscanf(threshold_opt->answer,"%d",&threshold)<1)
    G_fatal_error("r.connectivity: Threshold must be an interger value");

  G_get_set_window(&inhead);

  north=inhead.north;
  south=inhead.south;
  east=inhead.east;
  west=inhead.west;
  cols=inhead.cols;
  rows=inhead.rows;
  x_res=(inhead.east-inhead.west)/(float)cols;
  y_res=(inhead.north-inhead.south)/(float)rows;
  diag_res=pow(x_res*x_res+y_res*y_res,0.5);

  /* now read the bin information - do this early cos its quick and 
     we need to check its right */

  if((bin_file=fopen(bin_opt->answer,"r"))==NULL)
    G_fatal_error("r.connectivity: Couldn't open range bin file");

  i=0;
  while(fgets(in_str,1000,bin_file)!=NULL)
  {
    if(sscanf(in_str,"%lf%lf",&bin[i].min_range,&bin[i].max_range)<2)
      G_fatal_error("r.connectivity: Problem reading range file");
    if(bin[i].min_range>bin[i].max_range)
      G_fatal_error("r.connectivity: range minimum greater than \
	range maximum");
    i++;
  }
  fclose (bin_file);
  num_bins=i;

  for(i=1; i<num_bins;i++) if(bin[i].min_range<bin[i-1].max_range)
    G_fatal_error("ranges overlap or are not monotonically increasing\n");

  for(i=0;i<num_bins;i++)
  {
    bin[i].sum_distance=0.;
    bin[i].sum_directional_distance=0.;
    bin[i].num_pairs=0;
    bin[i].num_connected=0;
    bin[i].num_directional=0;
    bin[i].num_dir_connected=0;
  }

  /* these are used to limit the search distance for matched pairs so that
     execution times are improved for big arrays */
  max_row_dif=(int)(bin[num_bins-1].max_range/y_res)+1;
  max_col_dif=(int)(bin[num_bins-1].max_range/x_res)+1;

  /* allocate and initialise memory for the connected regions */

  if((connected=(int**)malloc(rows*sizeof(int*)))==NULL)
    G_fatal_error("r.connectivity:  Can't allocate memory");
  for(r=0;r<rows;r++)
  {
    if((connected[r]=(int*)malloc(cols*sizeof(int)))==NULL)
      G_fatal_error("r.connectivity:  Can't allocate memory");
  }

  for (r=0;r<rows;r++)
  for (c=0;c<cols;c++)
  {
    connected[r][c]=0;
  }

  /* now allocate memory and read in the input data */

  if((data=(int**)malloc(rows*sizeof(int*)))==NULL)
    G_fatal_error("r.connectivity:  Can't allocate memory");
  for(r=0;r<rows;r++)
  {
    if((data[r]=(int*)malloc(cols*sizeof(int)))==NULL)
      G_fatal_error("r.connectivity:  Can't allocate memory");
  }
  
  in_mapset=G_find_cell(in_name,"");
  if(in_mapset == NULL) 
  {
    sprintf(msg,"Cannot find %s", in_name);
    G_fatal_error(msg);
  }

  in_file=-1;
  in_file=G_open_cell_old(in_name,in_mapset);
  if(in_file<=0)
  {
    sprintf(msg,"Cannot open %s", in_name);
    G_fatal_error(msg);
  }

  /* check if there is a mask and read that as well if the include zero
     data flag has been set - this is to allow inclusion of zero data*/

  mask_mapset=G_find_cell("MASK","");
  
  if(mask_mapset && zero_flag->answer)
  {
    use_mask=1;

    /* allocate memory for the mask */
    if((mask=(int**)malloc(rows*sizeof(int*)))==NULL)
      G_fatal_error("r.connectivity:  Can't allocate memory");
    for(r=0;r<rows;r++)
    {
      if((mask[r]=(int*)malloc(cols*sizeof(int)))==NULL)
        G_fatal_error("r.connectivity:  Can't allocate memory");
    }
    mask_file=0;
    if(mask_mapset) mask_file=G_open_cell_old("MASK",mask_mapset);
  }
 
  /* read input into memory */

  in_buf=G_allocate_cell_buf();
  if(use_mask) mask_buf=G_allocate_cell_buf();

  for (r=0;r<rows;r++)
  {
    G_get_map_row(in_file,in_buf,r); 
    if(use_mask)G_get_map_row(mask_file,mask_buf,r);
    for (c=0;c<cols;c++)
    {
      data[r][c]=(int)in_buf[c];
      if(use_mask) mask[r][c]=(int)mask_buf[c];
    }
  }

  G_close_cell(in_file);
  if(use_mask) G_close_cell(mask_file);

  /* if we are doing directional connectivity allocate memory and read data
     for this */

  if(directional_flag->answer) 
  {
    if((elevation=(int**)malloc(rows*sizeof(int*)))==NULL)
      G_fatal_error("r.connectivity:  Can't allocate memory");
    if((up_down_slope=(int**)malloc(rows*sizeof(int*)))==NULL)
      G_fatal_error("r.connectivity:  Can't allocate memory");
    for(r=0;r<rows;r++)
    {
      if((elevation[r]=(int*)malloc(cols*sizeof(int)))==NULL)
        G_fatal_error("r.connectivity:  Can't allocate memory");
      if((up_down_slope[r]=(int*)malloc(cols*sizeof(int)))==NULL)
        G_fatal_error("r.connectivity:  Can't allocate memory");
      for(c=0;c<cols;c++) up_down_slope[r][c]=0;
    }
 
    dem_mapset=G_find_cell(dem_name,"");
    if(dem_mapset == NULL)
    {
      sprintf(msg,"Cannot find %s", dem_name);
      G_fatal_error(msg);
    }

    dem_file=-1;
    dem_file=G_open_cell_old(dem_name,dem_mapset);
    if(dem_file<=0)
    {
      sprintf(msg,"Cannot open %s", dem_name);
      G_fatal_error(msg);
    }
  

    for (r=0;r<rows;r++)
    {
      G_get_map_row(dem_file,in_buf,r);
      for (c=0;c<cols;c++) elevation[r][c]=(int)in_buf[c];
    }

  }
  
    
  /* if the -p flag is set we need to make a copy of the data, sort it and 
     calculate the threshold */

  if(percentile_flag->answer) 
  {
    if(threshold<0.||threshold>100.)
      G_fatal_error("r.connectivity: percentile threshold must be between \
	0 and 100");
    calc_thresh(data,mask,&threshold);
    fprintf(stderr,"Threshold is %1d\n",threshold);
  }

  /* now threshold the data - this will result in the array data[rows][cols]
     being set to :
     NO_DATA (0)  -  ignore pixel
     LOW     (1)  -  pixel <= threshold
     HIGH    (2)  -  pixel >  threshold
  */

  for(r=0;r<rows;r++)
  for(c=0;c<cols;c++)
  {
    if(use_mask)
    {
      if(mask[r][c])
      {
        if(data[r][c]>threshold) data[r][c]=HIGH;
        else data[r][c]=LOW;
      }
      else data[r][c]=NO_DATA;
    }
    else
    {
      if(data[r][c]!=0)
      {
        if(data[r][c]>threshold) data[r][c]=HIGH;
        else data[r][c]=LOW;
      }
      else data[r][c]=NO_DATA;
    }
  }

  /* now find the connected regions */

  region=1;
  for(r=0;r<rows;r++)
  for(c=0;c<cols;c++)
  {
    if (data[r][c]==HIGH && connected[r][c]==0)
    {
      find_connected (data,connected,r,c,region);
      region++;
    }
  }

  /* now calculate the connectivity function */

  for(r=0;r<rows;r++)
  {
  G_percent(r,rows,5);
    for(c=0;c<cols;c++)
    {
      if(connected[r][c])
      {
        x1=west+((double)c+0.5)*x_res;
        y1=north-((double)r+0.5)*y_res;
        min_row=LMAX(0,r-max_row_dif);
        max_row=LMIN(rows-1,r+max_row_dif);
        min_col=LMAX(0,c-max_col_dif);
        max_col=LMIN(cols-1,c+max_col_dif);
  
        /* if we are interested in directional stuff we need to find the up and 
           downslope connected areas now */
        if(directional_flag->answer)
        {
          up_down_slope[r][c]=1;
          if(search_up(r, c, INIT_CALL, INIT_CALL, elevation, up_down_slope, 
            connected, connected[r][c],1)==HIT_EDGE)
            fprintf(stderr,"\ar.catchments: Warning - dem edge encountered\n");
          if(search_down(r, c, INIT_CALL, INIT_CALL, elevation, up_down_slope, 
            connected, connected[r][c],1)==HIT_EDGE)
            fprintf(stderr,"\ar.catchments: Warning - dem edge encountered\n");
        }
  
  
        for(rr=min_row;rr<=max_row;rr++)
        for(cc=min_col;cc<=max_col;cc++)
        {
          if(data[rr][cc]!=NO_DATA)
          {
            x2=west+((double)cc+0.5)*x_res;
            y2=north-((double)rr+0.5)*y_res;
            distance=pow((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2),0.5);
            i=0;
            while (bin[i].max_range<distance&&i<num_bins-1) i++;
            if(distance>=bin[i].min_range)
            {
              bin[i].sum_distance+=distance;
              bin[i].num_pairs++;
              if(connected[r][c]==connected[rr][cc]) bin[i].num_connected++;
              if(directional_flag->answer && 
                up_down_slope[r][c]==up_down_slope[rr][cc]) 
              {
                bin[i].sum_directional_distance+=distance;
                bin[i].num_directional++;
                if(connected[r][c]==connected[rr][cc]) bin[i].num_dir_connected++;
              }
            }
          }
        }
  
        /* now reset the up and downslope connected regions */
        if(directional_flag->answer)
        {
          up_down_slope[r][c]=0;
          search_up(r, c, INIT_CALL, INIT_CALL, elevation, up_down_slope, 
            connected, connected[r][c],0);
          search_down(r, c, INIT_CALL, INIT_CALL, elevation, up_down_slope, 
            connected, connected[r][c],0);
        }
      }
    }
  }


  /* now write the result file */

  if((connect_file=fopen(connect_opt->answer,"w"))==NULL)
    G_fatal_error("r.connectivity: Couldn't open output file");
  
  for(i=0;i<num_bins;i++) 
  {
    if(bin[i].num_pairs>0) 
    {
      fprintf(connect_file,"%6.3f  ",
        bin[i].sum_distance/(double)bin[i].num_pairs);
      fprintf(connect_file,"%8.6f  ",
        (double)bin[i].num_connected/(double)bin[i].num_pairs);
      if(directional_flag->answer) 
      {
        if(bin[i].num_directional>0)
        {
          fprintf(connect_file,"%6.3f  ",
            bin[i].sum_directional_distance/(double)bin[i].num_directional);
          fprintf(connect_file,"%8.6f  ",
            (double)bin[i].num_dir_connected/(double)bin[i].num_directional);
        }
        else fprintf(connect_file," 0.0    0.0       ");
      }
      fprintf(connect_file,"%6d", bin[i].num_pairs);
      if(directional_flag->answer) 
        fprintf(connect_file,"%6d", bin[i].num_directional);
      fprintf(connect_file,"\n");
    }
  }

  fclose(connect_file);

  /* now we've just got to write out the cell map if its name was supplied */

  if(out_cell_opt->answer)
  {
    if((out_file=G_open_cell_new(out_name))<=0)
        G_fatal_error("r.connectivity: couldn't open output file");
   
    out_buf=G_allocate_cell_buf();
  
    for (r=0;r<rows;r++)
    {
      for (c=0;c<cols;c++) out_buf[c]=(CELL)connected[r][c];
      G_put_map_row(out_file,out_buf);
    }
    G_close_cell(out_file);
  }

  fprintf(stderr,"Execution time %-1.0f seconds\n",clock()/CLOCKS_PER_SEC);

  return;
}

/*---------------------------------------------------------------------------*/

int find_connected (int **data, int **connected, long r, long c, int region)

{
  extern long rows, cols;
  long min_row, max_row, min_col, max_col;

  connected[r][c]=region;

  min_row=LMAX(0,r-1);
  max_row=LMIN(rows-1,r+1);
  min_col=LMAX(0,c-1);
  max_col=LMIN(cols-1,c+1);

  for(r=min_row;r<=max_row;r++)
  for(c=min_col;c<=max_col;c++)
  if (data[r][c]==HIGH && connected[r][c]==0)
    find_connected (data,connected,r,c,region);
}

/*--------------------------------------------------------------*/

double slope(long r1,long r2,long c1,long c2, int **elevation)

{
  double slope;
  long index, neighbour;
  extern double diag_res, x_res, y_res;
  extern long rows, cols;

  if(r2!=r1 && c2!=c1)
  {
    slope=((double)(elevation[r2][c2]-elevation[r1][c1]))/diag_res;
  }
  else if (c2==c1)
  {
    slope=((double)(elevation[r2][c2]-elevation[r1][c1]))/y_res;
  }
  else
  {
    slope=((double)(elevation[r2][c2]-elevation[r1][c1]))/x_res;
  }
  return slope;
}

/*---------------------------------------------------------------------*/

void calc_thresh (int **data, int **mask, int *threshold)

{
  extern long rows, cols;
  extern int use_mask;
  unsigned long i, num_pts;
  long r,c;
  int percentile;
  int *copy;
  if((copy=(int*)malloc((rows*cols+1)*sizeof(int)))==NULL)
    G_fatal_error("Couldn't allocate memory");
  
  i=1;
  for(r=0;r<rows;r++)
  for(c=0;c<cols;c++)
  {
    if(use_mask && mask[r][c])
    {
      copy[i]=data[r][c];
      i++;
    }
    else if (data[r][c]!=0)
    {
      copy[i]=data[r][c];
      i++;
    }
  }

  num_pts=i-1;

  sort(num_pts,copy);

  i=(unsigned long)((double)num_pts*(double)*threshold/100.+0.5);

  *threshold=copy[i];
}


/*---------------------------------------------------------------------*/

int search_down (long cell_row, long cell_col, long calling_r, long calling_c, 
  int **elevation, int **up_down_slope,int **connected,int region, 
  int descriptor)

/* cell_row and cell_column refer to the cell we are at */

{
  int hit_edge=0;
  long r, c;
  double steepest, second_steepest;
  double slope_dat[8];
  int i;

  extern long rows, cols;
  extern struct Flag *flat_flag, *two_directions_flag,*up_down_flag;

  up_down_slope[cell_row][cell_col]=descriptor;

  hit_edge=find_steepest(slope_dat, elevation, &steepest, &second_steepest,
    cell_row,cell_col);

  if(cell_row==0||cell_col==0||cell_row==rows-1||cell_col==cols-1) hit_edge=1;

  i=0;
  for (r=LMAX(0,cell_row-1);r<=LMIN(rows-1,cell_row+1);r++)
  for (c=LMAX(0,cell_col-1);c<=LMIN(cols-1,cell_col+1);c++)
  {
    if(r==cell_row && c==cell_col) continue;

    if(slope_dat[i]>0.0 || (slope_dat[i]>=0.0&&flat_flag->answer))
    {
      if(slope_dat[i]>=steepest ||
        (slope_dat[i]>=second_steepest&&two_directions_flag->answer))
      {
        if(/*connected[r][c]==region &&*/ up_down_slope[r][c]!=descriptor) 
        {
          hit_edge=search_down(r, c, cell_row, cell_col, elevation, 
            up_down_slope, connected,region,descriptor);
        }
      }
    }
    i++;
  }

  /* this does the up and down zig-zags */
  if(up_down_flag->answer && up_down_slope[cell_row][cell_col]==descriptor)
  {
    for (r=LMAX(0,cell_row-1);r<=LMIN(rows-1,cell_row+1);r++)
    for (c=LMAX(0,cell_col-1);c<=LMIN(cols-1,cell_col+1);c++)
    {
      if((elevation[r][c]>elevation[cell_row][cell_col]) ||
        (elevation[r][c]>=elevation[cell_row][cell_col] && flat_flag->answer))
      {
        if(/*connected[r][c]==region &&*/ up_down_slope[r][c]!=descriptor) 
        {
          hit_edge=search_up (r,c,cell_row,cell_col,elevation,
            up_down_slope, connected,region,descriptor);
        }
      }
    }
  }
  /* end up and down zig zag */

  return hit_edge;
}


/*---------------------------------------------------------------------*/

int search_up (long cell_row, long cell_col, long calling_r, long calling_c, 
  int **elevation, int **up_down_slope,int **connected,int region, 
  int descriptor)

/* cell_row and cell_column refer to the cell we are at */

{
  int hit_edge=0;
  long r, c;
  double steepest, second_steepest, slope_to_calling_cell;
  double slope_dat[8];
  int i;

  extern long rows, cols;
  extern struct Flag *flat_flag, *two_directions_flag,*up_down_flag;

  hit_edge=find_steepest(slope_dat,elevation,&steepest,&second_steepest,
      cell_row,cell_col);

  if(cell_row==0||cell_col==0||cell_row==rows-1||cell_col==cols-1) hit_edge=1;

  if(calling_r!=INIT_CALL)  slope_to_calling_cell=
    DOWN*slope(cell_row,calling_r,cell_col,calling_c, elevation);
  else slope_to_calling_cell=1.;

  if(calling_r==INIT_CALL || (slope_to_calling_cell>=steepest) || 
    (slope_to_calling_cell>=second_steepest && two_directions_flag))
  {
    /* its upslope */
    up_down_slope[cell_row][cell_col]=descriptor;
    for (r=LMAX(0,cell_row-1);r<=LMIN(rows-1,cell_row+1);r++)
    for (c=LMAX(0,cell_col-1);c<=LMIN(cols-1,cell_col+1);c++)
    {
      if((elevation[r][c]>elevation[cell_row][cell_col]) ||
        (elevation[r][c]>=elevation[cell_row][cell_col] && flat_flag->answer))
      {
        if(/*connected[r][c]==region && */up_down_slope[r][c]!=descriptor) 
        {
          hit_edge=search_up (r,c,cell_row,cell_col,elevation,
            up_down_slope, connected,region,descriptor);
        }
      }
    }
  }

  /* the next bit does the up and downzig-zag bit */
  i=0;
  if(up_down_flag->answer && up_down_slope[cell_row][cell_col]==descriptor)
  {
    i=0;
    for (r=LMAX(0,cell_row-1);r<=LMIN(rows-1,cell_row+1);r++)
    for (c=LMAX(0,cell_col-1);c<=LMIN(cols-1,cell_col+1);c++)
    {
      if(r==cell_row && c==cell_col) continue;
  
      if(slope_dat[i]>0.0 || (slope_dat[i]>=0.0&&flat_flag->answer))
      {
        if(slope_dat[i]>=steepest ||
          (slope_dat[i]>=second_steepest&&two_directions_flag->answer))
        {
          if(/*connected[r][c]==region && */up_down_slope[r][c]!=descriptor) 
          {
            hit_edge=search_down(r, c, cell_row, cell_col, elevation, 
              up_down_slope, connected,region,descriptor);
          }
        }
      }
      i++;
    }
  }
  /* the end of the zig-zag bit */

  return hit_edge;
}


/*----------------------------------------------------------------*/


int find_steepest(double *slope_dat, int **elevation, double *steepest, 
  double *second_steepest, long r, long c)

{
  extern long rows, cols;
  extern double diag_res, x_res, y_res;
  long rr, cc;
  int hit_edge=0;
  int i;

  *steepest=-1.;
  *second_steepest=-1.;
  i=0;
  for (rr=LMAX(0,r-1);rr<=LMIN(rows-1,r+1);rr++)
  for (cc=LMAX(0,c-1);cc<=LMIN(cols-1,c+1);cc++)
  {
    if(rr==r && cc==c) continue;

    if(elevation[rr][cc]<=0)
    {
      hit_edge=1;
      slope_dat[i]=-1.;
      i++;
      continue;
    }

    slope_dat[i]=DOWN*slope(r,rr,c,cc,elevation);

    if(slope_dat[i]>*second_steepest)
    {
      if(slope_dat[i]>*steepest)
      {
        *second_steepest=*steepest;
        *steepest=slope_dat[i];
      }
      else 
      {
        *second_steepest=slope_dat[i];
      }
    }

    i++;

  }
  for(i=0;i<8;i++) slope_dat[i]+=0.01/diag_res;
  return hit_edge;
}
