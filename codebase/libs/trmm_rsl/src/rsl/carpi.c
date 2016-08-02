/*
    NASA/TRMM, Code 910.1.
    This is the TRMM Office Radar Software Library.
    Copyright (C) 1996, 1997
            Mike Kolander
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
/* Mike Kolander
 * Space Applications Corporation
 * NASA/Goddard 910.1
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <trmm_rsl/rsl.h> 

#define RAD2DEG 57.29578 /* radian to degree conversion */
#define MAXRAYS 512      /* loop safety valve when traversing a sweep */

extern int radar_verbose_flag;


/*************************************************************/
/*                                                           */
/*                     RSL_free_carpi                        */
/*                                                           */
/*************************************************************/
void RSL_free_carpi(Carpi *carpi)
{
	/* Frees memory allocated to a carpi structure, and associated
		 pointer and data arrays.
  */

	if (carpi != NULL)
	{
	  if (carpi->data != NULL)
		{
			if (carpi->data[0] != NULL)
		    free(carpi->data[0]);     /* Free the 2D data array. */
			free(carpi->data);     /* Free the vector of pointers. */
		}
		free(carpi);   /* Free the carpi structure. */
	}
}

/*************************************************************/
/*                                                           */
/*                     RSL_new_carpi                         */
/*                                                           */
/*************************************************************/
Carpi *RSL_new_carpi(int nrows, int ncols)
{
	/* Allocates memory for a carpi structure, and associated pointer
		 and data arrays.
	*/
	Carpi *c;
	Carpi_value *data;
	int row;
	
	/* Allocate a carpi. */
	c = (Carpi *)calloc(1, sizeof(Carpi));
	if (c == NULL) perror("RSL_new_carpi");
	/* Allocate a vector of 'nrows' pointers. */
	c->data = (Carpi_value **)calloc(nrows, sizeof(Carpi_value *));
	if (c->data == NULL) perror("RSL_new_carpi");
	/* Allocate a 2 dimensional array for the actual data values. */
	data = (Carpi_value *)calloc(nrows*ncols, sizeof(Carpi_value));
	if (data == NULL) perror("RSL_new_carpi");
	/* Fill all the 'nrows' pointer slots created above. */
	for (row=0; row<nrows; row++)
		c->data[row] = data + row*ncols;

	return(c);
}

/*************************************************************/
/*                                                           */
/*                     RSL_volume_to_carpi                   */
/*                                                           */
/*************************************************************/
Carpi *RSL_volume_to_carpi(Volume *v, float h, float grnd_r,
						float dx, float dy, int nx, int ny,
						int radar_x, int radar_y, float lat, float lon)
/*
 * Creates a carpi from a volume structure.
 *
 *   +--------------------+   ^
 *   |                    |   |
 *   |                    |   |
 *   |  cell size dx,dy   |   ny
 *   |                    |   |
 *   |                    |   |
 *   +--------------------+   V
 * lat,lon
 *   <-------- nx -------->
 * 
 *  Radar centered at radar_x, radar_y.
 */

{
  Cappi *cappi;
  Carpi *carpi;

  if (v == NULL) return NULL;
  cappi = RSL_cappi_at_h(v, h, grnd_r);
  cappi->lat = lat;
  cappi->lon = lon;
  cappi->interp_method = 0;

  carpi = RSL_cappi_to_carpi(cappi, dx, dy, lat, lon, nx, ny,
														 radar_x, radar_y);
  if (carpi == NULL) return NULL;
  RSL_free_cappi(cappi);
  return carpi;
}

/*************************************************************/
/*                                                           */
/*                      RSL_find_rng_azm                     */
/*                                                           */
/*************************************************************/
void RSL_find_rng_azm(float *r, float *ang, float x, float y)
{
/* Convert Cartesian coords (x,y) to polar (rng,angle); 0<angle<360 */
	*r = sqrt((double)x*x + (double)y*y);

	if (x != 0.0)
	{
		*ang = RAD2DEG*atan((double)y/x);
		if (x > 0.0)
		  *ang = 90.0 - *ang;
		else
		  *ang = 270.0 - *ang;
	}
	else   /* x is 0.0 */
	{
		if (y >= 0.0) *ang = 0.0;
		else *ang = 180.0;
	}
}

/*************************************************************/
/*                                                           */
/*                     RSL_cappi_to_carpi                    */
/*                                                           */
/*************************************************************/
Carpi *RSL_cappi_to_carpi(Cappi *cappi, float dx, float dy, float lat,
						  float lon, int nx, int ny, int radar_x, int radar_y)
   /****** Simple and straightforward algorithm: 
	  Divide each of the nx*ny carpi cells into scx*scy subcells. 
	  Find the data value for each subcell from the cappi rays.
	  Average the subcell data values over a cell to obtain the cell value.
	  Store the cell value into the 2_D carpi array.
	  ********/
{
	Carpi *carpi;
	Ray *first_ray;
	int row, col, j, k, m, n, scx, scy, valid_subcells;
	float x, y, rng, azm, cell, cell_diag, gate_size;
	float carpi_max_rng, radar_max_rng;
	float subcell[3][3];  /* Maximum of 9 subcells per carpi cell*/
   
	if (cappi == NULL) return NULL;
	if (cappi->sweep == NULL) return NULL;
	first_ray = RSL_get_first_ray_of_sweep(cappi->sweep);
	if (first_ray == NULL) return NULL; /* No data. */

	if (radar_verbose_flag) fprintf(stderr,"\nCreating carpi...\n");

	/* Allcate space for a carpi, and fill in its values. */
	carpi = RSL_new_carpi(ny, nx);
	carpi->month = cappi->month;
	carpi->day = cappi->day;
	carpi->year = cappi->year;
	carpi->hour = cappi->hour;
	carpi->minute = cappi->minute;
	carpi->sec = cappi->sec;
	carpi->dx = dx;
	carpi->dy = dy;
	carpi->nx = nx;
	carpi->ny = ny;
	carpi->radar_x = radar_x;
	carpi->radar_y = radar_y;
	carpi->height = cappi->height;
	carpi->lat = lat;
	carpi->lon = lon;
	strncpy(carpi->radar_type, cappi->radar_type, sizeof(cappi->radar_type));
	carpi->field_type    = cappi->field_type;
	carpi->interp_method = cappi->interp_method;
	carpi->f = first_ray->h.f;
	carpi->invf = first_ray->h.invf;

	gate_size = first_ray->h.gate_size / 1000.0;  /* km */
	cell_diag = sqrt(dx*dx + dy*dy);   
	/* How many subcells per carpi cell? The larger the carpi cell relative
		 to gate_size, the more subcells we want. Will have scx*scy subcells
		 per carpi cell. Note below that the max no. of subcells is 9 . */
	if ((dy - gate_size) < 0.0)
	  scy = 1;
	else if ((dy - gate_size) <= 1.0)
	  scy = 2;
	else
	  scy = 3;

	if ((dx - gate_size) < 0.0)
	  scx = 1;
	else if ((dx - gate_size) <= 1.0)
	  scx = 2;
	else
	  scx = 3;
   
	/* Max range of the radar data */
	radar_max_rng = first_ray->h.nbins * gate_size;  /* km */
	/* Max range of desired carpi is max of x and y directions. */
	carpi_max_rng = nx / 2.0 * dx;             /* km */
	if ( (ny / 2.0 * dy) > carpi_max_rng )
	  carpi_max_rng =  ny / 2.0 * dy;
	/* carpi_max_rng is smaller of (radar_max_rng, carpi_max_rng) */
	if (radar_max_rng < carpi_max_rng)
	  carpi_max_rng = radar_max_rng;
	
	if (radar_verbose_flag)
	  fprintf(stderr,"carpi_max_rng:%.1f(km) beam_width:%.1f gate_size:%d(m)\n",
						carpi_max_rng, cappi->sweep->h.beam_width, first_ray->h.gate_size);

	/* For each cell (row,col) comprising the carpi...*/
	for (row=0; row<ny; row++)
	{
		for (col=0; col<nx; col++)
		{
			/* For each subcell (there are scx*scy subcells per cell) 
				 of carpi cell (row,col)...*/
			for (n=0; n<scy; n++)
			{
				/* Convert carpi coords (row,col) to km coords (x,y) relative to
					 radar. */
				y = (row + n/(float)scy - radar_y)*dy;
				for (m=0; m<scx; m++)
				{
					x = (col + m/(float)scx - radar_x)*dx;
					/* Find range and azimuth of subcell (relative to radar) */
					RSL_find_rng_azm(&rng, &azm, x, y);
					/* Check if carpi cell outside of data range by noting 
						 location of lower left corner of cell */
					if (m == 0)
					{
					  if (n == 0)
							if ((rng - cell_diag) > carpi_max_rng)
							{
								/* Totality of carpi cell lies outside data range. */
								cell = (float) BADVAL;
								goto escape;
							}
					}  /* end if (m == 0) */
					/* cell lies in data range. Get data value from cappi. */		  
					subcell[n][m] = RSL_get_value_from_cappi(cappi,rng,azm);
				}  /* end for (m=... */
			} /* end for (n=... */

			/* All subcell values have now been determined. Average them
				 to determine the cell value. */
			valid_subcells = 0;  /* number subcells having valid data values.*/
			cell = 0.0;
			for (j=0; j<scy; j++)
			{
			  for (k=0; k<scx; k++)
				  if ((subcell[j][k] <= BADVAL) && (subcell[j][k] >= NOECHO))
				    continue;
					else  /* valid data value. */
					{
						cell = cell + subcell[j][k];
						valid_subcells++;
					}
			}  /* end for (j=0; j<scy; j++) */
			if (valid_subcells != 0) cell = cell/valid_subcells;
			else cell = (float) BADVAL;

escape:	
			carpi->data[row][col] = (Carpi_value) carpi->invf(cell);
		} /* end for (col=... */
	}  /* end for (row=...  */
   
	return(carpi);
}
