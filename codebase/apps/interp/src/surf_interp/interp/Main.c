      
#define SIZE 134
#define SIZE_LESS_TWO 132
#define MAX_STAT 400
#define MAX_PARAM 12

#include <stdio.h>

#include <toolsa/pmu.h> 
#include <toolsa/umisc.h>

#include <math.h>
#include <string.h> /* For strdup */

#include <mdv/mdv_grid.h> /* For Writing, after reading and arithmetic */
#include <mdv/mdv_write.h>
#include <mdv/mdv_utils.h>
#include <mdv/mdv_read.h>
#include <mdv/mdv_handle.h>
#include <mdv/mdv_user.h>


#include "read_sounding_class.h"
#include "pints.h"

typedef struct {
  float lat, lon;
} lat_lon_t;

main()
{

  MDV_handle_t _handle;
  int nx=SIZE, ny=SIZE;

  int nxmm = SIZE_LESS_TWO,nymm = SIZE_LESS_TWO;

  /*
    c  nfieldsanal is the number of fields that will be interpolated.
    */
  int nfieldsanal = 6;

  float pres_li,tli=0.0;
  int nfound,idtm;
  float dzkm,tliK;
  int nfieldscount;
  float bad;

  int max_stat=MAX_STAT;

  long t_end,t_start;
  int analysis_interval_sec=300, ncheckin_sec=10;

  float zint[SIZE*SIZE],ztemp[SIZE*SIZE],
    zu[SIZE*SIZE],zv[SIZE*SIZE];

  float xsta[MAX_STAT],ysta[MAX_STAT];

  float Rscale,Dmax,Dclose,Rfac;

  float datatemps[MAX_STAT];
  float dzr[SIZE*SIZE];
  float dz[MAX_STAT];

  int i;

  float uwind[MAX_STAT],vwind[MAX_STAT],temp[MAX_STAT],
    dewp[MAX_STAT],rlat[MAX_STAT],rlon[MAX_STAT],
    press[MAX_STAT];


  unsigned char outarray[SIZE_LESS_TWO*SIZE_LESS_TWO];
  long boffset,outtemp;

  char input_source[1024],output_dir[1024],sounding_dir[1024];
  char name_paramsv[33],unitsv[33];
  char  datatype[10];
 
  char   proc_name[256];
  char   proc_instance[256];
  char   proc_message[256];
  int      debug;

  float badsf=-999.99;

  float pi  = 3.14159;


  /*
    c.. parameters for pints.
    */       
  int ig = 1;
  int ip = 1;
  float rbints  = 15.0;
  float rdspdx = 16.0;
  float rmx = 1.5*15.0; /* 1.5*rbints */
  int nqd = 3;
  int nlflt = 0;

  /*		  
		  c.. special parameters for pints.
		  */
					   
  float gamma = 1.0;
  float rpints = 0.0;
  float rmxpints = -1.0;
  int ifg   = 0;
  float arcmax = 0.0;
  float rclose = -1.0;

  /*						    
						    c..  define southwest corner (in km) relative to radar.  
						    */
  float xmin = -160.0;
  float ymin = -256.0;
  float xmax,ymax,x0km,y0km;
  float dxkm = 3.0;
  float dykm = 3.0;

  float rlat_radar = 38.9753;
  float rlon_radar = -77.477;

  /*
    c. analysis_interval_sec is the interval in seconds between successive analyses
    c. ncheckin_sec is the intervals between successive check ins with proc_map.
    */
  int it,ncount,ifield;

  int mat_stat=MAX_STAT;

  int error,j;
  /*
    c.. set a default value for the temperature at pres_li
    */
  float tli_default = -20.0;


  /* Stuff to write output file with. */
  lat_lon_t *_locArray, *loc;
  mdv_grid_comps_t comps;
  int ix,iy;
  double yy,xx,lat,lon;
  mdv_grid_t _outGrid;

  char  *name_params[MAX_PARAM];
  char  *units[MAX_PARAM];

  /* The following is not graceful, but it is effective. */

  name_params[0]=strdup("uwind");
  name_params[1]=strdup("vwind");
  name_params[2]=strdup("temp");
  name_params[3]=strdup("dewp");
  name_params[4]=strdup("li");
  name_params[5]=strdup("conv");
  name_params[6]=strdup("blank");
  name_params[7]=strdup("blank");
  name_params[8]=strdup("blank");
  name_params[9]=strdup("blank");
  name_params[10]=strdup("blank");
  name_params[11]=strdup("blank");


  units[0]=strdup("m/s");
  units[1]=strdup("m/s");
  units[2]=strdup("C");
  units[3]=strdup("C");
  units[4]=strdup("C");
  units[5]=strdup("10**-4s-1");
  units[6]=strdup("blank");
  units[7]=strdup("blank");
  units[8]=strdup("blank");
  units[9]=strdup("blank");
  units[10]=strdup("blank");
  units[11]=strdup("blank");

  MDV_init_handle(&_handle);


  /*
    c.. read input file

    READ*,  input_source
    read*,  sounding_dir
    READ*,  proc_name
    read*,  proc_instance
    READ*,  output_dir
    read*,  proc_message
    read*,  pmu_interval
    read*,  debug 
    These are now hard-wired for a testing.

    */


  sprintf(input_source,"63360@couloir");
  sprintf(sounding_dir,"test_sounding");
  sprintf(proc_name,"interp_spdb");
  sprintf(proc_instance,"Operating");
  sprintf(output_dir,"./OutPut");
  sprintf(proc_message,"Running");
  debug=1;

  /*
    c.. register with procmap 
    */

  PMU_auto_init( proc_name, proc_instance, PROCMAP_REGISTER_INTERVAL );

  PMU_auto_register("Initializing");

  bad = badsf;

  xmax = xmin + (nx-1)*dxkm;
  ymax = ymin + (ny-1)*dykm;

  x0km = 0.5*(xmin+xmax);
  y0km = 0.5*(ymin+ymax);

  /*
    2000  continue
    */    

  PMU_auto_register("Reading surface and sounding data");

  pres_li = 500.0;

  read_sounding_class(sounding_dir,pres_li,&tli,tli_default,badsf,debug);


  /*
    call sys_time(t_end)
    Hard-wired for testing.*/ 
  t_end=915578461;

  t_start = t_end-5400;


  get_spdb_metars(input_source,&t_start,&t_end,&max_stat,
		  uwind,vwind,temp,dewp,press,rlat,rlon,&bad,&nfound,
		  &error);


  /*
    c.. check for duplicate station data
    */ 
  for (i=0;i<nfound;i++){ /* do 250 i=1,nfound */
    for (it=i+1;it<nfound;it++){ /*do 260 it=i+1,nfound */

      if ((rlat[i]==rlat[it]) && 
	  (rlon[i]==rlon[it])) rlat[it] = badsf;
          
    }
  }
 
  ncount = 0;
  for (i=0;i<nfound;i++){ /* do 270 i=1,nfound */

    if (rlat[i]!=badsf) {
      rlat[ncount]=rlat[i];
      rlon[ncount]=rlon[i];
      uwind[ncount]=uwind[i];
      vwind[ncount]=vwind[i];
      temp[ncount]=temp[i];
      dewp[ncount]=dewp[i];
      press[ncount]=press[i];
      ncount++;
    }
  }
 
  nfound = ncount;
 
  if (debug) fprintf(stderr,"Error flag : %d\n",error);
  if (debug) fprintf(stderr,"Number of stations reporting : %d\n",nfound);

     

  if (nfound ==0){
    PMU_auto_unregister();
    exit(-1);
  }

  for (i=0;i<nfound;i++){ /* do 1239 i=1,nfound */


    /*
      c  calculate station locations in kilometers relative to radar
      */
    xsta[i] = (rlon[i] - rlon_radar)*111.12*cos(3.14159*rlat_radar/180.0);
    ysta[i] = (rlat[i] - rlat_radar)*111.12;
    /*
      c  now calculate positions in index space.
      */
    xsta[i] = (xsta[i] - xmin)/dxkm;
    ysta[i] = (ysta[i] - ymin)/dykm;

  }

  if (debug) {
    fprintf(stderr,
	    "stat. no \tuwind \tvwind \ttemp \tdewpoint \tpress \tlat \tlon\n"); 

    for (i=0;i<nfound;i++){ /* do 400 i = 1,nfound */
 
      fprintf(stderr,
	      "%d\t %f7.2\t %f7.2\t %f7.2\t %f7.2\t %f7.2\t %f7.2\t %f7.2\n",
	      i+1,uwind[i],vwind[i],temp[i],dewp[i],press[i],
	      rlat[i],rlon[i]);
 
    }

  }

  for (ifield=0;ifield<nfieldsanal;ifield++){ /*do 100 ifield = 1,nfieldsanal */

    if (ifield <5) {

      if (ifield == 0){
	if (debug) fprintf(stderr," interpolating uwind\n");
	for (i=0;i<nfound;i++) /*do 10 i = 1,nfound */
	  datatemps[i] = uwind[i];

      } else if (ifield == 1){
	if (debug) fprintf(stderr," interpolating vwind\n");
	for (i=0;i<nfound;i++) /*do 20 i = 1,nfound */
	  datatemps[i] = vwind[i];

      } else if (ifield == 2) {
	if (debug) fprintf(stderr," interpolating temp\n");
	for (i=0;i<nfound;i++) /* do 30 i = 1,nfound */
	  datatemps[i] = temp[i];

      } else if (ifield == 3){
	if (debug) fprintf(stderr," interpolating dewpoint\n");
	for (i=0;i<nfound;i++) /* do 40 i = 1,nfound */
	  datatemps[i] = dewp[i];

      } else if (ifield == 4){
	if (debug) fprintf(stderr," interpolating lifted index\n");

	/* this seems to be calculated rather than obtained directly. */

	if (tli == badsf)
	  tliK = badsf;
	else
	  tliK = tli + 273.15;
                
	for (i=0;i<nfound;i++)  /* do 50 i = 1,nfound */
	  datatemps[i] = calc_li(press[i],temp[i],dewp[i],tliK,
				 pres_li,bad);
      }

      Rscale=0.0;
      Dmax=0.0;
      Dclose=0.0;
      Rfac=0.0;

      pints(
	    zint,&nx,&ny,xsta,ysta,datatemps,&nfound,                   
	    &rpints,&rmxpints,&gamma,&ip,&ifg,&arcmax,
	    &rclose,&bad,&debug,
	    &Rscale,&Dmax,&Dclose,&Rfac);


      if (ifield == 0) {
	for (i=0;i<nx*ny;i++) zu[i] = zint[i];
      } else if (ifield == 1) { 
	for (i=0;i<nx*ny;i++) zv[i] = zint[i];
      }

    } else {

      if (debug) fprintf(stderr,"Calculating convergence\n");
      /*
	c.. calculate convergence.
	*/
           
      PMU_auto_register("Calculating convergance");

      for (j=1;j<ny-1;j++){ /* do 60 j = 2,ny-1 */
	for (i=1;i<nx-1;i++){ /* do 60 i = 2,nx-1 */
 
	  if ((zu[i-1+nx*j] == bad) ||
	      (zu[i+1+nx*j] == bad) ||
	      (zv[i+nx*(j-1)] ==  bad) ||
	      (zv[i+nx*(j+1)] ==  bad)) {
 
	    zint[i+nx*j] = bad;
 
	  } else {
 
	    zint[i+nx*j]= -((zu[i+1+nx*j]-zu[i-1+nx*j])/(2.0*dxkm*1000.0)
			    + (zv[i+nx*(j+1)]-zv[i+nx*(j-1)])/(2.0*dykm*1000.0))*1.0e4;
 
	  }
 
	}
      }
     

      for (i=1;i<nx-1;i++){ /*do 70 i = 2,nx-1 */
	zint[i] = zint[i+nx];
	zint[i+nx*(ny-1)] = zint[i+nx*(ny-2)];
      }
 
      for (j=0;j<ny;j++){  /* do 80 j = 1,ny */
	zint[j*nx] = zint[1+j*nx];
	zint[nx-1+nx*j] = zint[nx-2+j*nx];
      }

    } 
    

    sprintf(datatype,"%s","mdv1");
    dzkm = 5.0;   
    idtm = 300;


    if (ifield == 0) nfieldscount = 0;

    PMU_auto_register("Writing mdv file");



    memset(&_outGrid, 0, sizeof(mdv_grid_t));

    /* load up grid params */

    _outGrid.nx = nx;
    _outGrid.ny = ny; /* nxmm,nymm */
    _outGrid.nz = 1;
    _outGrid.minx =  xmin;
    _outGrid.miny =  ymin; /* Should these be x0km,y0km ? */
    _outGrid.minz =  0.0;
    _outGrid.dx =  dxkm;
    _outGrid.dy =  dykm;
    _outGrid.dz =  dzkm;

    _outGrid.proj_type = MDV_PROJ_LATLON;
 
    /* alloc location array for (lat, lon) pairs */

    _locArray = 
      (lat_lon_t *) umalloc(_outGrid.nx * _outGrid.ny * sizeof(lat_lon_t));

    /* load up location array - the mdv_grid routines take care of
       the projection geometry */

    MDV_init_proj(&_outGrid, &comps);
  
    loc = _locArray;
    for (iy = 0; iy < _outGrid.ny; iy++) {
      yy = _outGrid.miny + _outGrid.dy * iy;
      for (ix = 0; ix < _outGrid.nx; ix++, loc++) {
	xx = _outGrid.minx + _outGrid.dx * ix;
	MDV_xy2latlon(&comps, xx, yy, &lat, &lon);
	loc->lat = lat;
	loc->lon = lon;
      } 
    } 
  


    /*
      call convert_from_unixtime(iyyd,immd,iddd,ihhd,imnd,issd,t_end)

      sprintf(name_paramsv,"%s",name_params[ifield]);
      sprintf(unitsv,      "%s",units[ifield]);

      call write_mdv(zint,1,nx,ny,dxkm,dykm,dzkm,outarray,
      1       nxmm,nymm,1,nfieldsanal,x0km,y0km,idtm,
      2       datatype,name_paramsv,unitsv,iyyd,immd,iddd,ihhd,
      3       imnd,issd,rlat_radar,rlon_radar,nfieldscount,bad,
      4       output_dir,debug)
      */

    fprintf(stderr,"FIELD %d : I'D WRITE A FILE HERE.\n",ifield);
  }

  MDV_free_handle(&_handle);

  PMU_auto_unregister();

  exit(0);

}




