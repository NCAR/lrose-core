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
/* Mike Kolander
 * Space Applications Corporation
 * NASA/Goddard 910.1
 */
#include <stdlib.h>
#include <string.h>
#include <trmm_rsl/rsl.h>

extern int radar_verbose_flag;

/*************************************************************/
/*                                                           */
/*                     RSL_free_slice                        */
/*                                                           */
/*************************************************************/
void RSL_free_slice(Slice *slice)
{
	/* Frees memory allocated to a slice structure, and associated
		 pointer and data arrays.
  */

	if (slice != NULL)
	{
	  if (slice->data != NULL)
		{
			if (slice->data[0] != NULL)
		    free(slice->data[0]);     /* Free the 2D data array. */
			free(slice->data);     /* Free the vector of pointers. */
		}
		free(slice);   /* Free the slice structure. */
	}
}

/*************************************************************/
/*                                                           */
/*                     RSL_new_slice                         */
/*                                                           */
/*************************************************************/
Slice *RSL_new_slice(int nrows, int ncols)
{
	/* Allocates memory for a slice structure, and associated pointer
		 and data arrays.
	*/
	Slice *s;
	Slice_value *data;
	int row;
	
	/* Allocate a slice. */
	s = (Slice *)calloc(1, sizeof(Slice));
	if (s == NULL) perror("RSL_new_slice");
	/* Allocate a vector of 'nrows' pointers. */
	s->data = (Slice_value **)calloc(nrows, sizeof(Slice_value *));
	if (s->data == NULL) perror("RSL_new_slice");
	/* Allocate a 2 dimensional array for the actual data values. */
	data = (Slice_value *)calloc(nrows*ncols, sizeof(Slice_value));
	if (data == NULL) perror("RSL_new_slice");
	/* Fill all the 'nrows' pointer slots created above. */
	for (row=0; row<nrows; row++)
		s->data[row] = data + row*ncols;

	return(s);
}

/*************************************************************/
/*                                                           */
/*                      RSL_new_cube                         */
/*                                                           */
/*************************************************************/
Cube *RSL_new_cube(int ncarpi)
{
	/* Allocate memory for a cube structure, and an associated array
		 of carpi pointers.
  */
  Cube *cube; 

	cube = (Cube *) calloc(1, sizeof(Cube));
	if (cube == NULL) return(NULL);
  cube->carpi = (Carpi **) calloc(ncarpi, sizeof(Carpi *));
  if (cube->carpi == NULL) return(NULL);
	return(cube);
}

/*************************************************************/
/*                                                           */
/*                     RSL_free_cube                         */
/*                                                           */
/*************************************************************/
void RSL_free_cube(Cube *cube)
{
	/* Frees memory allocated to a cube structure and associated
	   carpi structures.
  */
	int j;
	
	if (cube != NULL)
	{
		if (cube->carpi != NULL)
		{
	    for (j=0; j<cube->nz; j++)
	      if (cube->carpi[j] != NULL)
	        RSL_free_carpi(cube->carpi[j]);
			free(cube->carpi);
		}
		free(cube);
	}
}

/*************************************************************/
/*                                                           */
/*                     RSL_volume_to_cube                    */
/*                                                           */
/*************************************************************/
Cube *RSL_volume_to_cube(Volume *v, float dx, float dy, float dz,
					  int nx, int ny, int nz, float grnd_r,
					  int radar_x, int radar_y, int radar_z)
/* radar_z = 0 is the only thing that makes sense. Why pass it? */
{
  float lat=0;
  float lon=0;
  int i;
  Cube *cube;
  
  if (v == NULL) return NULL;
  /* check validity of radar site coordinates in cube. */
  if (radar_z != 0) return NULL;
  if ((radar_x < 0) || (radar_x > nx)) return NULL;
  if ((radar_y < 0) || (radar_y > ny)) return NULL;
  
  cube = (Cube *)RSL_new_cube(nz);
  if (cube == NULL) return NULL;

  cube->nx = nx;
  cube->ny = ny;
  cube->nz = nz;
  cube->dx = dx;
  cube->dy = dy;
  cube->dz = dz;
  if (v->h.type_str != (char *) NULL)
	cube->data_type = (char *)strdup(v->h.type_str);
  cube->lat = lat;
  cube->lon = lon;
  
  /* Create nz carpis */
  for (i=0; i<nz; i++)
	  cube->carpi[i] = (Carpi *)RSL_volume_to_carpi(v, (i+1)*dz, grnd_r,
																									dx, dy, nx, ny,
																									radar_x, radar_y,
																									lat, lon);
  return cube;
}

/*************************************************************/
/*                                                           */
/*                  RSL_get_slice_from_cube                  */
/*                                                           */
/*************************************************************/
Slice *RSL_get_slice_from_cube(Cube *cube, int x, int y, int z)
/*
   Check validity of parameters x,y,z , which define the plane of the
   required slice. Two of the three parameters must equal -1 and the
   third must be nonnegative; eg, the vertical plane y=100 is 
   specified by the parameters x=-1, y=100, z=-1

   Assumes valid ranges for x, y, z are:
      0 <= x <= nx-1 ,    0 <= y <= ny-1 ,    1 <= z <= nz
   The range of z starts at 1 , since a cappi (or carpi) at 
   height z=0 makes no sense.
*/
{
	int i, j;
	Slice *slice;

	if (cube == NULL) return(NULL);

	/* Slice defined by the plane y = const */
	if ((x == -1) && (z == -1) && (y > -1) && (y < cube->ny))
	{
		slice = (Slice *) RSL_new_slice(cube->nz, cube->nx);
		slice->data_type = (char *)strdup(cube->data_type);
	  slice->dx = cube->dx;
	  slice->dy = cube->dz;
	  slice->nx = cube->nx;
	  slice->ny = cube->nz;
		slice->f = cube->carpi[0]->f;
		slice->invf = cube->carpi[0]->invf;
		  /* Retrieve the required data values from the cube and place into
			 the slice structure. */
	  for (j=0; j<cube->nz; j++)
		  for (i=0; i<cube->nx; i++)
			  slice->data[j][i] = (Slice_value) cube->carpi[j]->data[y][i];
	}


	/* Slice defined by the plane x = const */
	else if ((y == -1) && (z == -1) && (x > -1) && (x < cube->nx))
	{
		slice = (Slice *) RSL_new_slice(cube->nz, cube->ny);
		slice->data_type = (char *)strdup(cube->data_type);
	  slice->dx = cube->dy;
	  slice->dy = cube->dz;
	  slice->nx = cube->ny;
	  slice->ny = cube->nz;
		slice->f = cube->carpi[0]->f;
		slice->invf = cube->carpi[0]->invf;
	  /* Retrieve the required data values from the cube and place into
			 the slice structure. */
	  for (j=0; j<cube->nz; j++)
			for (i=0; i<cube->ny; i++)
			  slice->data[j][i] = (Slice_value) cube->carpi[j]->data[i][x];
	}


	/* Want slice defined by the plane z = const ; ie, a carpi */
	else if ((x == -1) && (y == -1) && (z > 0) && (z <= cube->nz))
	{
		slice = (Slice *) RSL_new_slice(cube->ny, cube->nx);
		slice->data_type = (char *)strdup(cube->data_type);
	  slice->dx = cube->dx;
	  slice->dy = cube->dy;
	  slice->nx = cube->nx;
	  slice->ny = cube->ny;
		slice->f = cube->carpi[z-1]->f;
		slice->invf = cube->carpi[z-1]->invf;
	  /* Just copy carpi data values into slice structure. */
		for (j=0; j<cube->ny; j++)
		  for (i=0; i<cube->nx; i++)
			  slice->data[j][i] = (Slice_value) cube->carpi[z-1]->data[j][i];
	}


	else  /* Invalid parameters. */
	{
	  if (radar_verbose_flag)
		{
			fprintf(stderr,"\nRSL_get_slice_from_cube(): passed invalid parameters\n");
			fprintf(stderr,"nx:%d ny:%d nz:%d x:%d y:%d z:%d\n",cube->nx,
							cube->ny,cube->nz,x,y,z);
		}
	  return(NULL);
	}
	
	return(slice);
}
