

#define bool int 

/*

This routine has been re-coded from Fortran.
Original Fortan comments follow. 
Niles Oien, RAP, NCAR, Jan 1999.

	subroutine pints(z,nx,ny,x,y,d,n,R,Rmax,gamma,np,ifg,
     1  arcmax,Rclose,bad,debug)

C  Note: all units of distance in this program are grid units.  The lower left
C        grid point has a position value of (0,0).
C
C	z(nx,ny)  grid to hold analyzed data : size (nx,ny)
C       x(n),y(n) x and y locations of the n obs
C       d(n)      corresponding data values of obs
C       R         Normalization factor for weight computation w = exp(-(r/R)**2)
C                   If (R = 0 ) then program will set R = averge ob spacing.
C       Rmax      Maximum distance from grid point to ob in order to use ob.
C                   If (Rmax < 0. ) then use all... If ( Rmax == 0. ) then Rmax
C                   is set to 2 * (whatever R is determined from above)
C       gamma     2nd (and higher) passes convergence factor (0<gamma<=1.) 
C                   R**2 is multiplied by gamma for the later passes 
C                   if ( gamma <=0. then gamma = .3)
C       np        Number of passes of the scheme (recommended 2)
C       ifg       First guess, if ifg<>0, don't do first guess assume Z already
C                   contains a first guess field.  Note that in this case, R
C                   will be multiplied by gamma for all passes.
C       arcmax    Maximum allowed arc (degrees) allowed between observations about
C                   before no analysis did to that grid pt. (0 to disable).
C       Rclose    If arcmax criteria fails, but point within Rclose of a station
C                   still do interpolation to that point.
C                   If (Rclose=0, set Rclose=R (above) :: if (Rclose<0, ignore )
C       bad       Missing data value, applies to both input and output data. Should
C                   be set to some very unrealistic data value, and never zero. 
C
C   This routine uses a number of internal arrays.  These arrays must be at least
C   as large as the number of observation points.  The parameter MAXSTN sets
C   the size of these arrarys.... increase it if needed.  
C
C   Compile: f77 -c -e pints.f
C
C   P.Neilley (NCAR/RAP)  neilley@rap.ucar.edu  1/93
C-----------------------------------------------------------------------------------


New C code commences.

*/


void pints_(float *z, 
	    // Single dimensional, 0..nx*ny-1, referenced as z[j*nx+i]
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
	   float *Rfac);


/*
C******************************************************************************************
C Barnes analysis function - computes barnes function at point (ri,rj) given set of
C obs at (x,y) with values d.  subject to arcmax coverage constraint.  Parameters
C of analysis function stored in common block.  Z is current data value at the analysis
C point.  If Z==bad, no analysis done, otherwise new analysis added to z.
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
	      float *Rfac);


              
/*
C************************************************************************************
C Computes errors (errs) at points (x,y) between data(d) and analysis (z)
C Use binlinear interpolation of gridded z to random point x,y
C For data points outside of the domain of z, use gout as analyses at those points

           subroutine errors(z,nx,ny,x,y,d,gout,errs,n,bad)
           real z(nx,ny),x(*),y(*),d(*),errs(*),gout(*)
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
	    float bad);


/*
C*************************************************************************************
C This routine computes analyses at data points that lie outside
C the grid the user provides.  This allows these points to be used in the analysis.
C Use regular barnes analysis for these points, but don't do arcmax checking.
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
		 float *Rscale, float *Dmax, float *Dclose, float *Rfac);

	
/*
C*************************************************************************************
C Determines the maximum angle between a vector of angles
*/
float angmax(float *x,int n);

/*
C****************************************************************************************
C Sorts a float array into an increasing vector
*/
void sort(float *x,int n);

/*
C***************************************************************************************
C Copies array a to b
*/
void copy_array(float *a, float *b, int n);

/*
C*********************************************************************************
C Sets all values of an array to zero
*/
void zero_array(float *z, int n);

/*
C**********************************************************************************
C Determines if point (i,j) is within bounds (0.0 -> mx) and (0.0 -> my)
*/
bool out(float ri, float rj, float mx, float my);

/*
C*******************************************************************************
C Computes average spacing, by computing maximum area covered by the stations
C and then dividing that area equally amoung the stations.  This average area
C is assumed a box and thus its dimensions are equal to SQRT(area).  This is
C then set to the average station spacing
*/

float  ave_spacing(float *x,float *y, int n);




