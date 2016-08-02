// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 

#define false 0
#define true 1

#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <toolsa/umisc.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/pmu.h>

#include "pints.hh"
#include "fort_stuff.hh"
#include "ProjType.hh"
using namespace std;

/*

This routine has been re-coded from Fortran into C++ by Niles Oien.
The re-coding has involved only translating the code, and
I'm sure that a better job could be done by taking advantage
of some features of C that are not in Fortran. For instance, there
is a routine - zero_array - that sets all the values in an array
to 0.0. I'm sure this could be improved on in C - memset could be
used.

This code is in no way object oriented.

Everything is passed into pints by pointer, so that I could
test this routine by calling it from the original Fortran
code and check that the same result was obtained.

Original Fortan comments follow. 
Niles Oien, RAP, NCAR, Jan 1999.

	subroutine pints(z,nx,ny,x,y,d,n,R,Rmax,gamma,np,ifg,
     1  arcmax,Rclose,bad,debug)

C  Note: all units of distance in this program are grid units.  The lower left
C        grid point has a position value of (0,0).
C
C	z(nx,ny)  grid to hold analyzed data : size (nx,ny)

z is now a 1-d array due to C problems with multidimesnional
arrays - Niles.

C       x(n),y(n) x and y locations of the n obs
C       d(n)      corresponding data values of obs
C       R         Normalization factor for weight computation w = exp(-(r/R)**2)
C                   If (R = 0 ) then program will set R = averge ob spacing.
C       Rmax      Maximum distance from grid point to ob in order to use ob.
C                   If (Rmax < 0. ) then use all... If ( Rmax == 0. ) then Rmax
C                   is set to 2 * (whatever R is determined from above)
C       gamma     2nd (and higher) passes convergence factor (0<gamma<=1.) 
C                   R**2 is multiplied by gamma for the later passes 
C                   if ( (*gamma) <=0. then gamma = .3)
C       np        Number of passes of the scheme (recommended 2)
C       ifg       First guess, if (ifg<>0), don't do first guess assume Z already
C                   contains a first guess field.  Note that in this case, R
C                   will be multiplied by gamma for all passes.
C       arcmax    Maximum allowed arc degrees allowed between observations about
C                   before no analysis did to that grid pt. (0 to disable).
C       Rclose    If arcmax criteria fails, but point within Rclose of a station
C                   still do interpolation to that point.
C                   If Rclose=0, set Rclose=R (above) :: if (Rclose<0, ignore )
C       bad       Missing data value, applies to both input and output data. Should
C                   be set to some very unrealistic data value, and never zero. 
C
C   This routine uses a number of internal arrays.  These arrays must be at least
C   as large as the number of observation points.  The parameter MAXSTN sets
C   the size of these arrays.... increase it if needed.  
C
C   Compile: f77 -c -e pints.f
C
C   P.Neilley (NCAR/RAP)  neilley@rap.ucar.edu  1/93
C-----------------------------------------------------------------------------------


New C code commences.

*/


void pints(float *z, 
	    // Single dimensional, 0..(*nx)*(*ny)-1, referenced as z[j*(*nx)+i]
	   int *nx, int *ny,
	   float *x, // 0..n
	   float *y, // 0..n
	   float *d, // 0..n
	   int *n,
	   float *R,
	   float *Rmax,
	   float *gamma,
	   int *np,
	   int *ifg,
	   float *arcmax,
	   float *Rclose,
	   float *bad,
	   bool *debug,
	   float *Rscale, // These last 4 were in a common block.
	   float *Dmax,
	   float *Dclose,
	   float *Rfac,
	   float MinWeight,
	   float MaxInterpDist,
	   ProjType Proj)
{


  int i,j,ipass,ifirst;
  float rgamma;
  float mrj;
  /* Original Fortran.

     parameter (max_stn=400,big=9.9e29)
     real z((*nx),(*ny)),x(n),y(n),d(n),R,(*Rmax),(*gamma),(*arcmax),(*Rclose),(*bad)
     real errs(max_stn),gout(max_stn),angs(max_stn)
     common /barnes_parameters/ Rscale,Dmax,Dclose,Rfac
     logical (*debug)

     */

  const float big=9.9e29;

  float *errs, *angs, *gout;

  // Allocate internal arrays.
  // Niles.

  errs =(float *)umalloc((*n)*sizeof(float));
  angs =(float *)umalloc((*n)*sizeof(float));
  gout =(float *)umalloc((*n)*sizeof(float));


  if ((errs==NULL) || (angs==NULL) || (gout == NULL)){
    fprintf(stderr,"Malloc failed in pints.c\n"); // Unlikely.
    exit(-1);
  }

  // Initialize a few variables


  //  Rfac = alog(0.1)

  *Rfac=log(0.1);

  if ((*debug)) {
    if ((*ifg) == 0)
      fprintf(stderr,"ifg = %d - No first guess.\n",(*ifg));
    else
      fprintf(stderr,"ifg = %d - First guess given.\n",(*ifg));
  }

  *Rscale = (*R)*(*R);
  if ( *R <= 0.0 ) {

    /*-------------- We now use physical distances, not grid distances.
      So don't call the ave_spacing routine.
    *Rscale = ave_spacing(x,y,*n);
    if (*debug) fprintf(stderr,
	    " r calculated from 1.5*average spacing is %f\n",*Rscale);
    *Rscale = (*Rscale) * (*Rscale);
    */

    *Rscale = MaxInterpDist/2.0; // Seems appropriate.

  }

  *Dmax = (*Rmax)*(*Rmax);
  if ( (*Rmax) < 0.0 )
    *Dmax = big;
  else if ( (*Rmax) == 0.0 )
    *Dmax = *Rscale*4.0;


  if ( (*Rclose) > 0.0 )
    *Dclose = (*Rclose)*(*Rclose);
  else if ( (*Rclose) == 0.0 )
    *Dclose = *Rscale;
  else
    *Dclose = big;
     

  // Set up arrays depending upon if first guess supplied

  zero_array(gout,*n);

  if ( (*ifg) != 0 ) {
    ifirst = 2;
    update_gout(x,y,d,*n,*R,(*nx),(*ny),(*bad),gout,Rscale,Dmax,
		Dclose,Rfac,MinWeight,MaxInterpDist,Proj);
  } else {
    zero_array(z,(*nx)*(*ny));
    ifirst = 1;
  }

  rgamma = (*gamma);
  if ( (*gamma) <= 0.0 ) rgamma = 0.3;

  // The analysis loop


  for (ipass = ifirst; ipass <= (*np); ipass++){

    if ( ipass == 1 ){
      copy_array(d,errs,*n);
    } else {
      if ( (*np) == 2 ) *Rscale = *Rscale * rgamma; 
      errors(z,(*nx),(*ny),x,y,d,gout,errs,*n,(*bad));
    }


    for (j=0; j<(*ny); j++) {
      PMU_auto_register("Barnes Interpolation"); 
      mrj=(float) j;
      for (i=0; i<(*nx); i++){

	z[j*(*nx)+i]=barnes(z[j*(*nx)+i],(float)(i),
	mrj,x,y,errs,*n,*R,(*arcmax),
			 (*bad),angs,ipass,(*nx),(*ny),Rscale,
			 Dmax,Dclose,Rfac,MinWeight,
			    MaxInterpDist,Proj );

      }

    }

    if ( ipass != (*np) ) update_gout(x,y,errs,*n,*R,(*nx),(*ny),(*bad),gout,
				   Rscale,Dmax,Dclose,Rfac,
				      MinWeight, MaxInterpDist,Proj);
  }

  //
  // Free stuff.
  //
  ufree(angs); ufree(gout); ufree(errs);

  return;
}

/*
C******************************************************************************************
C Barnes analysis function - computes barnes function at point ((*ri),(*rj)) given set of
C obs at (x,y) with values d.  subject to (*arcmax) coverage constraint.  Parameters
C of analysis function stored in common block.  Z is current data value at the analysis
C point.  If Z==(*bad), no analysis done, otherwise new analysis added to z.

No common blocks now - everything is passed in/out. Niles Oien, Jan 1999

*/
float  barnes(float z,
	      float ri,
	      float rj,
	      float *x,
	      float *y,
	      float *d,
	      int n,
	      float r,
	      float arcmax,
	      float bad,
	      float *angs,
	      int ipass,
	      int nx,
	      int ny,
	      float *Rscale,
	      float *Dmax,
	      float *Dclose,
	      float *Rfac,
	      float MinWeight,
	      float MaxInterpDist,
	      ProjType Proj)
{

           
           bool doarc;
	   float Barnes;   
           int nsta = 0;
           float wsum = 0.0;
           float sum = 0.0;
	   int m; float w;

	   // float dist;
	   // float factor;

	   Barnes = bad; 

	   // factor=1.0;

	   if ( z == bad ) return bad;

           doarc = ((arcmax < 360.0) && (arcmax > 0.0));

	   // do{ // This adaptive loop using factor added by Niles.
	   //   avoids disaster if wsum=0
	   // Commented out as decided to return bad in this instance.

	   for(m=0;m<n;m++){
	     if ((m % 100) == 0) PMU_auto_register("Interpolating...");

	     if ( d[m] != bad ) {

	       // This code was for grid distance and so has been commented out. Niles.
	       //  dist =  (x[m]-ri)*(x[m]-ri) + (y[m]-rj)*(y[m]-rj);
	       //  if ( dist <= *Dmax ) {
	       
	       float InterpDist;
	       if (Proj.flat){

		 float x1,y1,x2,y2;
		 x1=x[m] * Proj.Dx; y1=y[m] * Proj.Dy;
		 x2=ri*Proj.Dx;     y2=rj*Proj.Dy;

		 InterpDist = sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));

	       } else { // x and y are lon and lat
		 double theta,r,plat,plon,qlat,qlon;
		 
		 plon = Proj.Origin_lon + ri * Proj.Dx;
		 plat = Proj.Origin_lat + rj * Proj.Dy;

		 qlon = Proj.Origin_lon + x[m] * Proj.Dx;
		 qlat = Proj.Origin_lat + y[m] * Proj.Dy;

		 PJGLatLon2RTheta(qlat, qlon,
				  plat, plon, &r, &theta);
		 InterpDist = r;

		 
	       }

	       if (InterpDist < MaxInterpDist){
		 
		 //w = (float)exp(*Rfac*dist/(*Rscale*factor));
		 //w = (float)exp(*Rfac*dist/(*Rscale));
		 //
		 // Now use phsical rather than grid distance.
		 //
		 w = (float)exp(*Rfac * InterpDist/(*Rscale));

		 sum =  sum + d[m]*w;
		 wsum = wsum + w;

		 //  if ( dist < *Dclose ) doarc = false;
		 
		 if ( doarc ) angs[nsta] = (float)atan2((double)(x[m]-ri),
							  (double)(y[m]-rj) );
		 nsta++;
	       }

	     }
	     // } Commented out if statement
	     
	   }
	   // factor=2.0*factor;
	   //}while((wsum < 1e-30) && (factor < 100.0));

	   Barnes = bad;

	   //if ( nsta > 0 ){

	   if ((nsta > 0) && (wsum>=MinWeight)) {
	     if (!(doarc)){ 
	       Barnes = sum / wsum + z;
	     } else { 
	       if ( angmax( angs, nsta ) <= arcmax ) {
		 Barnes = sum / wsum + z;
	       }
	     }
	   }

           return Barnes;
}
              
/*
C************************************************************************************
C Computes errors (errs) at points (x,y) between data(d) and analysis (z)
C Use binlinear interpolation of gridded z to random point x,y
C For data points outside of the domain of z, use gout as analyses at those points

           subroutine errors(z,(*nx),(*ny),x,y,d,gout,errs,n,(*bad))
           real z((*nx),(*ny)),x(*),y(*),d(*),errs(*),gout(*)
	   logical out
	   */

void errors(float *z,
	    int nx,
	    int ny,
	    float *x,
	    float *y,
	    float *d,
	    float *gout,
	    float *errs,
	    int n,
	    float bad)
{

  int mx,my;

  float sumrms = 0.0;
  float summean = 0.0;
  int npoints = 0; int i,j;
  float dx,dy,z11,z12,z21,z22;
  float zleft,zright;
  int k;
  float rms;

  mx = nx - 1;
  my = ny - 1;

  for(k=0;k<n;k++){ // Was do k=1,n
    errs[k] = bad;	
    if ( d[k] != bad ) {
      if ( out( x[k], y[k], (float)(mx), (float)(my) ) ) {
	if ( gout[k] != bad ){ 
	  errs[k] = d[k] - gout[k];
	} else {
	  i = (int)rint((double)x[k] + 0.5);
	  j = (int)rint((double)y[k] + 0.5);
	  dx = x[k] - (float)(i-1);
	  dy = y[k] - (float)(j-1);
	  z11 = z[i+j*nx];
	  z12 = z[i+(j+1)*nx];
	  z21 = z[i+1+j*nx];
	  z22 = z[i+1+(j+1)*nx];
	  if ( (z11!=bad) && (z12 != bad) && (z21 !=bad) && (z22 !=bad) ){
	    zleft =  z11 + dy * ( z12 - z11 );
	    zright = z21 + dy * ( z22 - z21 );
	    errs[k] = d[k] - ( zleft + dx*(zright-zleft) );
	    sumrms = sumrms + errs[k]*errs[k];
	    summean = summean + errs[k];
	    npoints++;
	  }
	}
      }
    }
  }
  rms = (float)sqrt((double)(sumrms)/(double)(npoints));
  summean = summean/npoints;
//    fprintf(stderr,"rms difference %f   mean error = %f over %d points\n",
//  	  rms,summean,npoints);

  return;
}

/*
C*************************************************************************************
C This routine computes analyses at data points that lie outside
C the grid the user provides.  This allows these points to be used in the analysis.
C Use regular barnes analysis for these points, but don't do (*arcmax) checking.
*/
void update_gout(float *x,
		 float *y,
		 float *d,
		 int n,
		 float r,
		 int nx,
		 int ny,
		 float bad,
		 float *gout,
		 float *Rscale, float *Dmax, float *Dclose, 
		 float *Rfac, float MinWeight,
		 float MaxInterpDist,
		 ProjType Proj)
{
  int k, mx,my,ipass=0;
  float dummy=0.0;   
  float zero=0.0;
  mx = nx-1;
  my = ny-1;
 


  for (k=0;k<n;k++){ //Was do k=1,n
    if ( out(x[k],y[k],(float)(mx),(float)(my) ) ) 
      gout[k]=barnes(gout[k],x[k],y[k],
	x,y,d,n,r,zero,bad,&dummy,
	ipass,nx,ny,Rscale,Dmax,
		     Dclose,Rfac,MinWeight,
		     MaxInterpDist, Proj);
	}

  return;
}
	
/*
C*************************************************************************************
C Determines the maximum angle between a vector of angles
*/
float angmax(float *x,int n)
{

  int i;
  float angmaxR;

  sort(x,n);
  angmaxR = 0.0;

  for(i=1;i<n;i++){ //Was do i=2,n
    angmaxR = AMAX1(angmaxR,(x[i]-x[i-1]));
  }
  angmaxR = AMAX1(angmaxR,x[0]-x[n-1]+360.0);
  return angmaxR;
}

/*
C****************************************************************************************
C Sorts a float array into an increasing vector
*/

void sort(float *x,int n)
{

int i,j;
float tmp;

  for (i=0;i<n-1;i++){ // Was do i=1,n-1
    for(j=i+1;j<n;j++){ // Was  do j=i+1,n
      if ( x[j] < x[i] ){
		tmp = x[i];
		x[i] = x[j];
		x[j] = tmp;
      }
    }
  }   
  return;
}


/*
C***************************************************************************************
C Copies array a to b
*/
void copy_array(float *a, float *b, int n)
{
  int i;
  for (i=0;i<n;i++) b[i] = a[i];
  return;
}

/*
C*********************************************************************************
C Sets all values of an array to zero
*/

void zero_array(float *z, int n)
{
  int i;

  for(i=0;i<n;i++) z[i] = 0.0;
  return;
}

/*
C**********************************************************************************
C Determines if point (i,j) is within bounds (0.0 -> mx) and (0.0 -> my)
*/

bool out(float ri, float rj, float mx, float my)
{
        return ((ri < 0.0) || (rj < 0.0) || (ri > mx) || (rj > my));
}

/*
C*******************************************************************************
C Computes average spacing, by computing maximum area covered by the stations
C and then dividing that area equally amoung the stations.  This average area
C is assumed a box and thus its dimensions are equal to SQRT(area).  This is
C then set to the average station spacing.

This routine is now never called, since I used
physical rather than grid distances. Niles.

*/

float  ave_spacing(float *x,float *y, int n)
{

  float rxmin,rxmax,rymin,rymax;
  int i;

  rxmin = x[0];
  rxmax = x[0];
  rymin = y[0];
  rymax = y[0];
	
  for (i=1;i<n;i++){
    rxmin = AMIN1(rxmin,x[i]);
    rxmax = AMAX1(rxmax,x[i]);
    rymin = AMIN1(rymin,y[i]);
    rymax = AMAX1(rymax,y[i]);
  }

  return (float) sqrt( (double)(rxmax-rxmin) * (double)(rymax-rymin) / (double)(n));

}







