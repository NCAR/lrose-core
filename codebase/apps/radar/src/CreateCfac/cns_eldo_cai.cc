//****
//******************************************************************
//**** THIS PROGRAM CALCULATES NAVIGATIONAL ERRORS:
//****
//****    D_TILT_aft, D_TILT_fore, D_ROLL, D_PITCH, D_HEADING
//****    RANGE_DELAY_aft, RANGE_DELAY_fore, D_Xwe, D_Ysn, D_Z
//****    D_VH_acft
//****
//**** FROM COMPARISONS BETWEEN:
//****
//****    - RADAR-DERIVED SURFACE AND DIGITAL TERRAIN MAP
//****     (or CONSTANT GROUND LEVEL);
//****    - DOPPLER VELOCITY AT SURFACE LEVEL AND ZERO;
//****    - DOPPLER  VELOCITY AT LOW-ELEVATION CLOSE TO THE AIRCRAFT
//****      AND THE PROJECTION OF THE FLIGHT-LEVEL (IN SITU) WIND;
//****
//**** DIFFERENT OPTIONS ARE AVAILABLE (see DATA_cns FILE)
//****
//****
//**** THIS PROGRAM ALSO PRODUCES A "RADAR-DERIVED" SURFACE MAP
//**** (FILE "SURF_EL_*" HAS THE SAME STRUCTURE AS FILE "SURF_DTM_*")
//****
//****
//******************************************************************
//**** Author: Frank ROUX (LA, UPS-CNRS, Toulouse), March 2000  ****
//******************************************************************
//     Modified by Huaqing Cai at NCAR, April, 2010
//    Converted to C/C++ by Brenda Javornik, NCAR, EOL, August 2023
//******************************************************************

// TODO:  name the new app: CreateCfac in the app/radar dir;
//        follow after radarcal for a template.


//      program cns_eldo
void main(int argc, char *argv[]) {

#define nvar 12
#define nxysurfmax 200

//     include '/home/users/rouf/SOURCES/ELDO/mes_parametres'
// CAI-START: Inlcude the parameter file mes_parametres directly below
//      instead of using the inlcude function above
//******************************************************************
//**** FICHIER 'mes_parametres'
//******************************************************************
//

//      parameter (MAXRAD=2,MAXFREQ=5,MAXPARM=10,MAXPORT=2000)
//      parameter (MAXFREQRAD=MAXFREQ*MAXPARM)
//      parameter (MAXPARAD=MAXRAD*MAXPARM)
//      parameter (MAXPORAD=MAXRAD*MAXPORT)
//      parameter (MAXPARIS=256)

#define MAXRAD 2
#define MAXFREQ 5
#define MAXPARM 10
#define MAXPORT 2000
#define MAXFREQRAD MAXFREQ*MAXPARM
#define MAXPARAD MAXRAD*MAXPARM
#define MAXPORAD MAXRAD*MAXPORT
#define MAXPARIS 256      

// Variable for reading text files

int ntimes;    // Number of times rays were collected --- Number of rays
int nranges;   // Number of range gates per time  --- Number of gates for each ray
int nsweep;    // Sweep number in each netcdf file; Used to identify different sweep

int*4 counter;  // Ray number


int start_year,start_mon,start_day;
int start_hour,start_min,start_sec;


//  Scaler variable for coccrection factors

float azimuth_correction;
float elevation_correction;
float range_correction;
float longitude_correction;
float latitude_correction;
float pressure_altitude_correction;
float radar_altitude_correction;
float ew_gound_speed_correction;
float ns_ground_speed_correction;
float vertical_velocity_correction;
float heading_correction;
float roll_correction;
float pitch_correction;
float drift_correction;
float rotation_correction;
float tilt_correction;

// Variable for cfac files
float tilt_corr_aft;
float tilt_corr_fore;
float rot_angle_corr_aft;
float rot_angle_corr_fore;
float pitch_corr_cfac;
float drift_corr_cfac;
float range_delay_corr_aft;
float range_delay_corr_fore;
float pressure_alt_corr;
float ew_gndspd_corr;

// Scaler variable for each ray

int sweep_number;

float*8  time;
float azimuth;
float elevation;
float*8 latitude;
float*8 longitude;
float*8 altitude;
float altitude_agl;
float heading;
float roll;
float pitch;
float drift;
float rotation;
float tilt;
float ew_velocity;
float ns_velocity;
float vertical_velocity;
float ew_wind;
float ns_wind;
float vertical_wind;

// One dimensional array of DBZ, VR, SW, NCP, etc

float range(MAXPORT);
float ZE(MAXPORT),NCP(MAXPORT),VR(MAXPORT),SW(MAXPORT);
float VS(MAXPORT),VL(MAXPORT),VG(MAXPORT),VU(MAXPORT);

// Variables for input file list
        CHARACTER(len=80) infilename;
int  nfile,ifile,lastfile     // total number of netcdf text file & file number
int iopen;

c Variables declarations previous in Franks' common block, which has been deleted

c From COMMON /CSPD_OU_CELV/
      integer*4 nb_portes;
      real d_porte(MAXPORAD);

c From COMMON /CFAC/
c
      real corr_azest(MAXRAD),corr_elhor(MAXRAD),corr_dist(MAXRAD);
     &    ,corr_lon(MAXRAD),corr_lat(MAXRAD);
     &    ,corr_p_alt(MAXRAD),corr_r_alt(MAXRAD);
     &    ,corr_vwe_av(MAXRAD),corr_vsn_av(MAXRAD);
     &    ,corr_vnz_av(MAXRAD);
     &    ,corr_cap(MAXRAD),corr_roul(MAXRAD);
     &    ,corr_tang(MAXRAD),corr_derv(MAXRAD);
     &    ,corr_rota(MAXRAD),corr_incl(MAXRAD);

c From COMMON /RYIB/
c
      integer*2 ih_rdl,im_rdl,is_rdl,ims_rdl;
      integer*4 num_swp,j_julien,etat_rdl,no_rdl;
      real azest_rdl,elhor_rdl,puiscre_em,vit_bal_rdl;

c From COMMON /ASIB/ ************************************************
c
      real*8  lon_av,lat_av,p_alt_av,r_alt_av;
      real    vwe_av,vsn_av,vnz_av;
     &    ,cap_av,roul_av,tang_av,derv_av;
     &    ,rota_rdl,incl_rdl;
     &    ,vent_we,vent_sn,vent_nz;
     &    ,chg_cap,chg_tang;

c CAI-STOP
c
      real*4  dgate_corr(MAXPORT),dgate_true(MAXPORT);
     &       ,vdop_corr(MAXPORT);
     &       ,xms(9),xml(9);
     &       ,rota_start(2),rota_end(2),xp(2);
     &       ,ssc(2),scc(2);
     &       ,sxa(2),sya(2),sza(2);
     &       ,sacfthspd(2),stime(2);
     &       ,xp_acft(2),su_acft(2),sv_acft(2),sw_acft(2);
     &       ,su_wind(2),sv_wind(2),sw_wind(2),xp_wind(2);
     &       ,stilt(2),stilt2(2),xsweeps(2);
     &       ,rota_prev(2);
     &       ,swdzsurf_sweep(2),dzsurfsweep_mean(2);
     &       ,dzsurfsweep_rms(2);
     &       ,swvsurf_sweep(2),vsurfsweep_mean(2),vsurfsweep_rms(2);
     &       ,swinsitu_sweep(2),dvinsitusweep_mean(2);
     &       ,dvinsitusweep_rms(2);
     &       ,var(nvar),xmat(nvar,nvar),vect(nvar);
     &       ,xinv(nvar,nvar),res(nvar);
     &       ,vect_dzsurf(nvar),xmat_dzsurf(nvar,nvar);
     &       ,vect_vsurf(nvar),xmat_vsurf(nvar,nvar);
     &       ,vect_dvinsitu(nvar),xmat_dvinsitu(nvar,nvar);
     &       ,alt_dtm(nxysurfmax,nxysurfmax);
     &       ,swdzsurf_wri(nxysurfmax,nxysurfmax);
     &       ,sw_or_altsurf_wri(nxysurfmax,nxysurfmax);
     &       ,zs_rot(2,500),zs_el(2,500),zs_az(2,500);
     &       ,zs_dsurf(2,500),zs_dhor(2,500);
     &       ,zs_zsurf(2,500);
     &       ,zs_hsurf(2,500);
     &       ,vs_dhor(2,500),vs_vdopsurf(2,500);
     &       ,vi_dhor(2,500),vi_vdop(2,500),vi_vinsitu(2,500);
     &       ,rms_var_zsurf(nvar),rms_var_vsurf(nvar);
     &       ,rms_var_vinsitu(nvar);
     &       ,corr_var(nvar,nvar);
     &       ,s_vpv(2,2),sv_vpv(2,2),svv_vpv(2,2);
     &       ,x_vpv(2,2),xv_vpv(2,2),xvv_vpv(2,2);
c
      integer*2 iyymmdd(3),ig_dismiss(15);
c
      integer*4 nb_ray(2),nb_sweep(2);
     &         ,n_dzsurf(2),n_vsurf(2),n_dvinsitu(2);
     &         ,nsurf_wri(2);
     &         ,ndismiss_vhacft(2),ndismiss_vdopcorr(2);
     &         ,ndismiss_vdopsurf(2);
     &         ,swp(2),swp_prev(2);
     &         ,ndop_ok(2),nref_ok(2);
     &         ,istart_sweep(2);
     &         ,itab(nxysurfmax),ihms_dtm(6),ialtsurf_wri(nxysurfmax);
c
      character path_abs*18,directory*60,dir_read*60;
     &         ,fich_sis*30;
     &         ,dtm_file*50,fich_cornav*30;
     &         ,wrisurfile*50;
     &         ,yymmdd_dtm*12,suff_dtm*20;
     &         ,yymmdd*12,c_hms_min*7,c_hms_max*7;
c CAI START   command line arguments
     &         ,argu*30;
c CAI STOP

c
c     include '/home/users/rouf/SOURCES/ELDO/mes_commons'
c
c CAI-START: This is common block for transfering data from tape to this
c      program, and it will be replaced by read in a text file
c      so that this common block is no longer used
c     include '/home/caihq/navigation/roux_nav/SOURCES-ELDO/mes_commons'
c CAI-STOP
      common/cosinang/crr,srr,cti,sti
     &               ,chdg,shdg,cdri,sdri,cpit,spit
     &               ,caze,saze,celh,selh
c
c CAI-START
c////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c//////// THES VALUES ARE CORRECT FOR IPP1/IPP2=4/5 ONLY ////////
c     data xms/0.,-10.,+20.,+10.,0.,-10.,-20.,+10.,0./
c     data xml/-8.,-16.,+16.,+8.,0.,-8.,-16.,+16.,+8./
c////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c CAI-STOP: xml and xms are not used anymore for ELDORA
c
//******************************************************************
//**** CONSTANT PARAMETRES
//******************************************************************
c
      path_abs='/d1/navigation/roux_nav/';
      pipi=6.38319;
      deg_lon0=111.32;
      deg_lat=111.13;
      conv=3.14159/180.;
      rayter=6370.;
      xncp_min=0.25;
      sw_max=5.;
      vdop_max=200.;
      selh_surf=-0.15;
      zacftmin_surf=1.5;
      igstart_surf=5;
      refsurf_min0=20.;
      gradrefsurf_min0=50.;
      dhsurf_max=999.;
      vdopsurf_max=999.;
      dmax_insitu=5.;
      xpmin_contray=3.;
      dvdop_max=10.;
      dvdopinsitu_max=999.;
      selhinsitu_max=0.1;
      ssurfins_min=1.;
c
//******************************************************************
//**** READ THE INPUT PARAMETERS ON FILE "DATA_cns"
//******************************************************************
c
c CAI-START ---- read in command line arguments
       CALL GETARG(1, argu)

c     open(99,file='DATA_cns_Cai',status='unknown')

      open(99,file=argu,status='unknown')
c CAI STOP

      print *,' '
      read(99,*)directory
      ndir=1;
      do while(directory(ndir:ndir).ne.' ')
         ndir=ndir+1;
      enddo
      ndir=ndir-1;
      print *,' DIRECTORY FOR THE OUTPUT FILES :',directory(1:ndir)
c
c CAI-START: No tape driver, change to number of text files
      print *,' '
      read(99,*)dir_read
      ndirr=1
      do while(dir_read(ndirr:ndirr).ne.' ')
         ndirr=ndirr+1
      enddo
      ndirr=ndirr-1
      print *,' DIRECTORY FOR READ FILES :',dir_read(1:ndirr)

      print *,' '
      read(99,*)nfile
      print *,' Total Number of Sweep Files :',nfile

c CAI-STOP

      read(99,*)iyymmdd
      write(yymmdd,"(i12)")10000*iyymmdd(3)+100*iyymmdd(2)+iyymmdd(1)
      print *,' YYYYMMDD :',yymmdd
c
      read(99,*)ihms_min,ihms_max
      print *,' HHMMSS (min,max) :',ihms_min,ihms_max
      ih_min=ihms_min/10000;
      im_min=(ihms_min-10000*ih_min)/100;
      is_min=ihms_min-10000*ih_min-100*im_min;
      ih_max=ihms_max/10000;
      im_max=(ihms_max-10000*ih_max)/100;
      is_max=ihms_max-10000*ih_max-100*im_max;
c
      read(99,*)orig_lat,orig_lon
      print *,' ORIGIN_LATITUDE,_LONGITUDE :',orig_lat,orig_lon
c
      print *,' '
      read(99,*)ig_dismiss
      print *,' 15 GATES TO DISMISS (0 if not) :',ig_dismiss
c
      print *,' '
      read(99,*)dmin,dmax0
      print *,' DMIN,DMAX FROM RADAR [km]:',dmin,dmax0
c
      print *,' '
      read(99,*)ipr_alt
      print *,' ALTITUDE (1:pressure,2:radar) :',ipr_alt
c
      print *,' '
      read(99,*)ref_min0,ref_max
      print *,' REF_min(at 10km),REF_max [dBZ]:',ref_min0,ref_max
c
      print *,' '
      read(99,*)ichoice_vdop
      print *,' WHICH VDOP (1:VR,2:VG,3:VU) :'
     &       ,ichoice_vdop
      if(ichoice_vdop == 1.or.ichoice_vdop == 2)then
        ictrl_contray=0
        print *,' -> WILL NOT CONTROL CONTINUITY ALONG THE RAY ////////'
      else
        ictrl_contray=0
      endif
c
      print *,' '
      print *,' CORRECTION OF NAVIGATIONAL ERRORS'
      print *,' '
      print *,' FIELDS TAKEN INTO ACCOUNT:'
      kdzsurf=1
      kvsurf=1
      kdvinsitu=1
      read(99,*)
      read(99,*)rw_dzsurf,rw_vsurf,rw_dvinsitu
      print *,'   -> REL.WGHT_dZsurf,Vsurf,dVinsitu (1/0) :'
     &       ,rw_dzsurf,rw_vsurf,rw_dvinsitu
c
      print *,' '
      print *,' CORRECTIONS TO CALCULATE:'
      read(99,*)
      read(99,*)idtiltaft,idtiltfore
      print *,'   -> D_TILT_AFT,D_TILT_FORE (1/0) :'
     &       ,idtiltaft,idtiltfore
      read(99,*)idrotaaft,idrotafore
      print *,'   -> D_ROTA_AFT,D_ROTA_FORE (1/0) :'
     &       ,idrotaaft,idrotafore
      read(99,*)idpitch,idhdg
      print *,'   -> D_PITCH,D_HEADING (1/0) :'
     &       ,idpitch,idhdg
      read(99,*)irdaft,irdfore
      print *,'   -> RANGE_DELAY_AFT,RANGE_DELAY_FORE (1/0) :'
     &       ,irdaft,irdfore
      read(99,*)idxwe,idysn,idzacft
      print *,'   -> D_XWE,D_YSN,D_ZACFT (1/0) :'
     &       ,idxwe,idysn,idzacft
      read(99,*)idvh
      print *,'   -> D_VHACFT (1/0) :',idvh
c
      print *,' '
      read(99,*)
      read(99,*)isim
      print *,' SIMULATION AVEC +dXXX_GUESS INITIAUX (1/0) :',isim
c
      print *,' '
      read(99,*)
      read(99,*)dtiltaft_guess,dtiltfore_guess
      print *,' D_TILT_AFT,D_TILT_FORE (deg) guess :'
     &       ,dtiltaft_guess,dtiltfore_guess
      read(99,*)drotaaft_guess,drotafore_guess
      print *,' D_ROTA_AFT,D_ROTA_FORE (deg) guess :'
     &       ,drotaaft_guess,drotafore_guess
      read(99,*)dpitch_guess,dhdg_guess
      print *,' D_PITCH,D_HEADING (deg) guess :'
     &       ,droll_guess,dpitch_guess,dhdg_guess
      read(99,*)rdaft_guess,rdfore_guess
      print *,' RANGE_DELAY_AFT,RANGE_DELAY_FORE (km) guess :'
     &       ,rdaft_guess,rdfore_guess
      read(99,*)dxwe_guess,dysn_guess,dzacft_guess
      print *,' D_XWE,D_YSN,D_ZACFT (km) guess :'
     &       ,dxwe_guess,dysn_guess,dzacft_guess
      read(99,*)dvh_guess
      print *,' D_VHACFT (m/s) guess :',dvh_guess
      read(99,*)
c
      print *,'  '
      read(99,*)idtmfile,dtm_file,zsurf_cst
      ndtmfile=0
      if(idtmfile.ne.0)then
        do while(dtm_file(ndtmfile+1:ndtmfile+1).ne.' ')
           ndtmfile=ndtmfile+1
        enddo
      endif
      if(idtmfile == 0)print *,' NO "SURF_DTM_*" FILE WILL BE READ '
     &                         ,'-> ZSURF_CST (km) =',zsurf_cst
      if(idtmfile == 1)print *,' WILL READ "SURF_DTM_*" FILE :'
     &                         ,dtm_file(1:ndtmfile)
c
      read(99,*)iwrisurfile,wrisurfile
      nsf=0;
      if(iwrisurfile == 1)then
        do while(wrisurfile(nsf+1:nsf+1).ne.' ')
           nsf=nsf+1;
        enddo
        print *,' WILL WRITE "SURF_EL_*" FILE : '
     &         ,wrisurfile(1:nsf)
        read(99,*)xywidth_wrisurf,hxy_wrisurf
        xmin_wrisurf=-xywidth_wrisurf/2.;
        xmax_wrisurf=+xywidth_wrisurf/2.;
        ymin_wrisurf=-xywidth_wrisurf/2.;
        ymax_wrisurf=+xywidth_wrisurf/2.;
        print *,' -> Xmin,max_wrisurf:',xmin_wrisurf,xmax_wrisurf
        print *,'    Ymin,max_wrisurf:',ymin_wrisurf,ymax_wrisurf
        print *,'    Hx,y_wrisurf:',hxy_wrisurf
        nx_wrisurf=((xmax_wrisurf-xmin_wrisurf)/hxy_wrisurf+1.);
        ny_wrisurf=((ymax_wrisurf-ymin_wrisurf)/hxy_wrisurf+1.);
        print *,'    Nx,Ny_wrisurf:',nx_wrisurf,ny_wrisurf
        if(nx_wrisurf > nxysurfmax.or.ny_wrisurf > nxysurfmax)then
	  print *,' //////// Nx,Ny_wrisurf :',nx_wrisurf,ny_wrisurf
     &           ,' > NxySURFmax ////////'
	  print *,' //////// MODIFY l.30 AND RECOMPILE THE PROGRAM ////////'
          go to 3
        endif
c
//**** OPEN "SURF_EL_*" FILE #30 FOR WRITING (if IWRISURFILE=1)
c
        print *,' OPEN "SURF_EL_*" FILE #30 FOR WRITING :'
     &   ,directory(1:ndir)//'/'//wrisurfile(1:nsf)
        open(30
     &   ,file=directory(1:ndir)//'/'//wrisurfile(1:nsf)
     &   ,form='formatted',status='unknown')
        iolat_wrisurf=(1000.*orig_lat);
        iolon_wrisurf=(1000.*orig_lon);
        ixmin_wrisurf=(1000.*xmin_wrisurf);
        iymin_wrisurf=(1000.*ymin_wrisurf);
        ihxy_wrisurf=(1000.*hxy_wrisurf);
        write(30,111)yymmdd,'ELDO'
     &               ,iolat_wrisurf,iolon_wrisurf
     &               ,0,0,0,0,0
     &               ,ih_min,im_min,is_min
     &               ,ih_max,im_max,is_max
     &               ,ixmin_wrisurf,iymin_wrisurf,0
     &               ,nx_wrisurf,ny_wrisurf,1
     &               ,ihxy_wrisurf,ihxy_wrisurf,0
c
      else
        print *,' NO "SURF_EL_*" FILE WILL BE WRITTEN'
      endif
c
      close(99)
      if(no_lect > 900)go to 3
c
//******************************************************************
//**** GENERATE THE SURFACE ARRAYS
//******************************************************************
c
      print *,' '
      print *,' GENERATE THE SURFACE ARRAYS'
      print *,' '
c
      if(idtmfile == 1)then
c
c------------------------------------------------------------------
c---- FROM THE INPUT "SURF_DTM_*" FILE
c------------------------------------------------------------------
c
       print *,' IFIDTM=1 -> READ THE "SURF_DTM_*" FILE #20 :'
     &         ,directory(1:ndir)//'/'//dtm_file(1:ndtmfile)
       open(20
     &     ,file=directory(1:ndir)//'/'//dtm_file(1:ndtmfile)
     &     ,form='formatted',status='unknown')
        read(20,111)yymmdd_dtm,suff_dtm
     &              ,iolat_dtm,iolon_dtm
     &              ,iha_dtm,ima_dtm,isa_dtm
     &              ,iua_dtm,iva_dtm
     &              ,ihms_dtm,ixmin_dtm,iymin_dtm,nul1
     &              ,nx_dtm,ny_dtm,nul2,ihx_dtm,ihy_dtm,nul3
  111   format(a12,a4,22i7)

c CAI START
        write(*,*)'nx_dtm,ny_dtm,ixmin_dtm',nx_dtm,ny_dtm,ixmin_dtm
c CAI STOP
	olat_dtm=float(iolat_dtm)/1000.;
	olon_dtm=float(iolon_dtm)/1000.;
	xlatm_surf=(olat_dtm+orig_lat)/2.;
	deg_lon=deg_lon0*cos(conv*xlatm_surf);
	dx_dtm=(olon_dtm-orig_lon)*deg_lon
	dy_dtm=(olat_dtm-orig_lat)*deg_lat;
	xmin_dtm=float(ixmin_dtm)/1000.+dx_dtm;
	ymin_dtm=float(iymin_dtm)/1000.+dy_dtm;
	hx_dtm=float(ihx_dtm)/1000.;
	hy_dtm=float(ihy_dtm)/1000.;
	xmax_dtm=xmin_dtm+float(nx_dtm-1)*hx_dtm;
	ymax_dtm=ymin_dtm+float(ny_dtm-1)*hy_dtm;
	print *,'    X_DTM_min,max :',xmin_dtm,xmax_dtm
	print *,'    Y_DTM_min,max :',ymin_dtm,ymax_dtm
	print *,'    Hx,y_DTM :',hx_dtm,hy_dtm
	print *,'    Nx,y_DTM:',nx_dtm,ny_dtm
        saltdtm=0.;
        altdtm_mean=0.;
	altdtm_min=+999.;
	altdtm_max=-999.;
        do jdtm=1,ny_dtm
	   read(20,333)(itab(idtm),idtm=1,nx_dtm)
  333      format(500i6)
	   do idtm=1,nx_dtm
	      if(itab(idtm) > -1000)then
		h_dtm=float(itab(idtm))/1000.;
		alt_dtm(idtm,jdtm)=h_dtm;
	        saltdtm=saltdtm+1.;
	        altdtm_mean=altdtm_mean+h_dtm;
	        altdtm_min=amin1(altdtm_min,h_dtm);
	        altdtm_max=amax1(altdtm_max,h_dtm);
	      else
		alt_dtm(idtm,jdtm)=-999.;
	      endif
	   enddo
	enddo
	altdtm_mean=altdtm_mean/amax1(1.,saltdtm);
        close(20)
c
      elseif(idtmfile == 0)then
c
c------------------------------------------------------------------
c---- FROM ZSURF_CST (read in DATA_cns)
c------------------------------------------------------------------
c
        print *,' IFIDTM=0 -> ALT_SURF(i,j)=cst'
     &         ,' (',zsurf_cst,' )'
        xmin_dtm=xmin_wrisurf;
        ymin_dtm=ymin_wrisurf;
        hx_dtm=hxy_wrisurf;
        hy_dtm=hxy_wrisurf;
        nx_dtm=nx_wrisurf;
        ny_dtm=ny_wrisurf;
	xmax_dtm=xmin_dtm+float(nx_dtm-1)*hx_dtm;
	ymax_dtm=ymin_dtm+float(ny_dtm-1)*hy_dtm;
        do jdtm=1,ny_dtm
	   do idtm=1,nx_dtm
	      alt_dtm(idtm,jdtm)=zsurf_cst;
	   enddo
	enddo
        altdtm_mean=zsurf_cst;
	altdtm_min=zsurf_cst;
	altdtm_max=zsurf_cst;
c
      endif
      print *,'     -> NPTS:',int(saltdtm)
     &       ,' ALTSURF_mean,min,max:',altdtm_mean
     &       ,altdtm_min,altdtm_max
      zsurfrad_min=altdtm_min-1.;
      zsurfrad_max=altdtm_max+1.;
      print *,' '
      print *,' -> AUTHORIZED ZSURF_RAD_min,max :'
     &       ,zsurfrad_min,zsurfrad_max
c
//******************************************************************
//**** MIN AND MAX TIMES
//******************************************************************
c
      tmin=3.6*float(ih_min)+0.06*float(im_min)
     &     +0.001*float(is_min);
      tmax=3.6*float(ih_max)+0.06*float(im_max)
     &     +0.001*float(is_max);
      write(c_hms_min,"(i7)")1000000+ihms_min;
      write(c_hms_max,"(i7)")1000000+ihms_max;
      write(fich_cornav,"('CORNAV_E_',a6,'_',a6)")
     &     c_hms_min(2:7),c_hms_max(2:7);
      write(fich_sis,"('SIS_E_',a6,'_',a6)")
     &     c_hms_min(2:7),c_hms_max(2:7)
c
//******************************************************************
//**** OPEN THE OUPUT "CORNAV_EL_*" FILE #10
//******************************************************************
c
      print *,' '
      print *,' OPEN "CORNAV_EL_*" FILE #10 :'
     &         ,directory(1:ndir)//'/'//fich_cornav
      open(10,file=directory(1:ndir)//'/'//fich_cornav
     &       ,form='formatted',status='unknown')
      write(10,"(' YYYYMMDD : ',a12)")yymmdd
      write(10,"(' HHMMSS_min HHMMSS_max : ',a6,3x,a6,/)")
     &     c_hms_min(2:7),c_hms_max(2:7)
      write(10,"( ' FIELDS TAKEN INTO ACCOUNT',/
     &           ,'  -> REL.WGHT_dZsurf,Vsurf,dVinsitu : ',3f6.3,/)")
     &     rw_dzsurf,rw_vsurf,rw_dvinsitu
      write(10,"( ' VARIABLES TAKEN INTO ACCOUNT',/
     &           ,'  -> D_TILT_AFT,D_TILT_FORE (1/0) : ',2i2,/
     &           ,'  -> D_ROTA_AFT,D_ROTA_FORE (1/0) : ',2i2,/
     &           ,'  -> D_PITCH,D_HEADING (1/0) : ',2i2,/
     &           ,'  -> RANGE_DELAY_AFT,RANGE_DELAY_FORE (1/0) : '
     &           ,2i2,/
     &           ,'  -> D_XWE,D_YSN,D_ZACFT (1/0) : ',3i2,/
     &           ,'  -> D_VHACFT (1/0) : ',i2)")
     &     idtiltaft,idtiltfore
     &    ,idrotaaft,idrotafore
     &    ,idpitch,idhdg
     &    ,irdaft,irdfore
     &    ,idxwe,idysn,idzacft
     &    ,idvh
      if(idtmfile == 1)then
        write(10,"(' READS THE SURF_DTM_* FILE :',a50)")
     &       directory(1:ndir)//'/'//dtm_file(1:ndtmfile)
      else
        write(10,"( ' NO SURF_DTM_* FILE TO READ '
     &             ,'-> ALT_SURF(x,y)=CST (',f6.3,')')")
     &       zsurf_cst
      endif
      if(iwrisurfile == 1)then
        write(10,"(' WRITES THE SURF_EL_* FILE :',a50,//)")
     &       directory(1:ndir)//'/'//wrisurfile(1:nsf)
      else
        write(10,"(' NO SURF_EL_* FILE TO WRITE ',//)")
      endif
c
//******************************************************************
//**** OPEN THE OUTPUT "SIS_EL_*" FILE #50
//******************************************************************
c
      print *,' '
      print *,' OPEN "SIS_EL_*" FILE #50 :'
     &       ,directory(1:ndir)//'/'//fich_sis
      open(50,file=directory(1:ndir)//'/'//fich_sis
     &       ,form='unformatted',status='unknown')
      write(50)iyymmdd,orig_lon,orig_lat,ihms_min,ihms_max
c
//******************************************************************
//**** INITIALIZATIONS
//******************************************************************
c
      time_prev=-999;
      ihms_prev=-999;
      do iradar=1,2;
         istart_sweep(iradar)=0;
         rota_prev(iradar)=-999.;
         nb_ray(iradar)=0;
         stilt(iradar)=0.;
         stilt2(iradar)=0.;
         rota_start(iradar)=-999.;
         rota_end(iradar)=-999.;
         sxa(iradar)=0.;
         sya(iradar)=0.;
         sza(iradar)=0.;
         sacfthspd(iradar)=0.;
         stime(iradar)=0.;
         ssc(iradar)=0.;
         scc(iradar)=0.;
         xp_acft(iradar)=0.;
         su_acft(iradar)=0.;
         sv_acft(iradar)=0.;
         sw_acft(iradar)=0.;
         xp_wind(iradar)=0.;
         su_wind(iradar)=0.;
         sv_wind(iradar)=0.;
         sw_wind(iradar)=0.;
         xsweeps(iradar)=0.;
         n_dvinsitu(iradar)=0;
         n_dzsurf(iradar)=0;
         n_vsurf(iradar)=0;
         ndismiss_vhacft(iradar)=0;
         ndismiss_vdopcorr(iradar)=0;
         ndismiss_vdopsurf(iradar)=0;
         swdzsurf_sweep(iradar)=0.;
         dzsurfsweep_mean(iradar)=0.;
         dzsurfsweep_rms(iradar)=0.;
         swvsurf_sweep(iradar)=0.;
         vsurfsweep_mean(iradar)=0.;
         vsurfsweep_rms(iradar)=0.;
         swinsitu_sweep(iradar)=0.;
         dvinsitusweep_mean(iradar)=0.;
         dvinsitusweep_rms(iradar)=0.;
         nsurf_wri(iradar)=0;
         nb_sweep(iradar)=0;
	 do jgd=1,2
	    s_vpv(iradar,jgd)=0.;
	    sv_vpv(iradar,jgd)=0.;
	    svv_vpv(iradar,jgd)=0.;
	    x_vpv(iradar,jgd)=0.;
	    xv_vpv(iradar,jgd)=0.;
	    xvv_vpv(iradar,jgd)=0.;
	 enddo
      enddo
      ssurfins=0.	// Olivier (r�el)
      swdzsurf_tot=0.;
      swdzmsurf_tot=0.;
      swdz2surf_tot=0.;
      swadzsurf_tot=0.;
      do i=1,nvar
        rms_var_zsurf(i)=0.;
      enddo
      swvsurf_tot=0.;
      swvmsurf_tot=0.;
      swv2surf_tot=0.;
      swavsurf_tot=0.;
      do i=1,nvar
        rms_var_vsurf(i)=0.;
      enddo
      swdvinsitu_tot=0.;
      swdvminsitu_tot=0.;
      swdv2insitu_tot=0.;
      swadvinsitu_tot=0.;
      do i=1,nvar
        rms_var_vinsitu(i)=0.;
      enddo
      do i=1,nvar
        do j=1,nvar
           corr_var(i,j)=0.;
        enddo
      enddo
      do i=1,nvar
	 var(i)=0.;
	 do j=1,nvar
	    xmat_dzsurf(i,j)=0.;
	    xmat_vsurf(i,j)=0.;
	    xmat_dvinsitu(i,j)=0.;
	    xmat(i,j)=0.;
	 enddo
	 vect_dzsurf(i)=0.;
	 vect_vsurf(i)=0.;
	 vect_dvinsitu(i)=0.;
	 vect(i)=0.;
	 res(i)=0.;
      enddo
      do i_wrisurf=1,nxysurfmax
 	 do j_wrisurf=1,nxysurfmax
	    swdzsurf_wri(i_wrisurf,j_wrisurf)=0.;
	    sw_or_altsurf_wri(i_wrisurf,j_wrisurf)=0.;
	 enddo
      enddo
      do ig=1,MAXPORT
	 vdop_corr(ig)=-999.;
      enddo
      do i=1,2
         do n=1,500
            zs_rot(i,n)=0.;
            zs_el(i,n)=0.;
            zs_az(i,n)=0.;
            zs_dsurf(i,n)=0.;
            zs_dhor(i,n)=0.;
            zs_zsurf(i,n)=0.;
            zs_hsurf(i,n)=0.;
            vs_dhor(i,n)=0.;
            vs_vdopsurf(i,n)=0.;
            vi_dhor(i,n)=0.;
            vi_vdop(i,n)=0.;
            vi_vinsitu(i,n)=0.;
         enddo
      enddo


	nb1=0;
	nb2=0;
	nb3=0;
	nb4=0;
	nb5=0;
	nb6=0;
	nb7=0;
	nb8=0;
	nsup=0;
	nbtotals=0	// Olivier
	nbon=0		// Olivier
	nmauvais=0	// Olivier
c
      if(no_lect == 999)go to 3
c
//******************************************************************
//*** READ THE ELDORA DATA FROM TEXT FILES CREATED BY ANOTHER PROGRAM
//******************************************************************
c
      print *,' '
      print *,'**************************************'
      print *,' READ THE ELDORA DATA FROM TEXT FILES'
      print *,'**************************************'
      print *,' '
c 1   call lit_eldo_2()
c CAI-START
//******************************************************************
c    The subroutine lit_eldo_2() is replaced by the following lines
c    of code, which read in a number of text files ray by ray
//******************************************************************
      iopen = 0;
      ifile = 1;
      lastfile = 0;
  1   if(iopen  ==  0) then

        write(infilename,'(i10)') ifile
        infilename = dir_read(1:ndirr) // '/'
     &  // TRIM(ADJUSTL(infilename)) // '.txt'
        open(55,file=infilename, status='old')
        iopen = 1
      endif
      read(55,101,END=5)counter
     &  ,nsweep,NTIMES,NRANGES
     &  ,start_year,start_mon,start_day
     &  ,start_hour,start_min,start_sec,time
     &  ,azimuth,elevation,latitude,longitude,altitude
     &  ,altitude_agl,heading,roll,pitch,drift
     &  ,rotation,tilt,ew_velocity
     &  ,ns_velocity,vertical_velocity,ew_wind,ns_wind
     &  ,vertical_wind,azimuth_correction,elevation_correction
     &  ,range_correction,longitude_correction,latitude_correction
     &  ,pressure_altitude_correction,radar_altitude_correction
     &  ,ew_gound_speed_correction,ns_ground_speed_correction
     &  ,vertical_velocity_correction,heading_correction
     &  ,roll_correction,pitch_correction,drift_correction
     &  ,rotation_correction,tilt_correction

      read(55,102,END=5)counter,(range(J),J=1,nranges)
      read(55,102,END=5)counter,(ZE(J),J=1,nranges)
      read(55,102,END=5)counter,(NCP(J),J=1,nranges)
      read(55,102,END=5)counter,(VR(J),J=1,nranges)
      read(55,102,END=5)counter,(SW(J),J=1,nranges)
 101  format(I10,2x,50x,3I10,I5,5I3,d20.8,2f10.4,3d20.8,29f10.4)
 102  format(I10,2000f10.4)
c after successful read
        goto 6
c When end of file reached

  5     iopen = 0
        close(55)
        if(ifile  ==  nfile) then
           lastfile=1
           goto 7
        else
           ifile = ifile +1
           goto 1
        endif

  6     continue

c ************ Get the ray time *************
        ih_rdl1 = start_hour;
        im_rdl1 = start_min;
        is_rdl1 = start_sec;
        ims_rdl1 = (time-INT(time))*1000;

c add to the start seconds by time, which is the elpased time after start time
        is_rdl1 = is_rdl1+INT(time);
c adjusting hh,mm,ss for passing 60 mark, assign to Frank's ray time variables
        ims_rdl = ims_rdl1;
        is_rdl = MOD(is_rdl1,60);
        im_rdl1 = im_rdl1+is_rdl1/60;
        im_rdl = MOD(im_rdl1, 60);
        ih_rdl1 = ih_rdl1+im_rdl1/60;
        ih_rdl = MOD(ih_rdl1, 60);

c Assign the aircraft position/angles to Frank's variables
        azest_rdl = azimuth;
        elhor_rdl = elevation;
        lat_av = latitude;
        lon_av = longitude;
        p_alt_av = altitude;
        r_alt_av = altitude_agl;
        cap_av = heading;
        roul_av = roll;
        tang_av = pitch;
        derv_av = drift;
        rota_rdl = rotation;
        incl_rdl = tilt;
        vwe_av = ew_velocity;
        vsn_av = ns_velocity;
        vnz_av = vertical_velocity;
        vent_we = ew_wind;
        vent_sn = ns_wind;
        vent_nz = vertical_wind;

c Assign  the total number of gates and range of each gates,
c  The aft/fore radar are different
        nb_portes = nranges
        if (tilt  <  0) then //AFT,iradar_ray=1,iaftfore= -1
           do ig = 1, nranges
              d_porte(ig) = range(ig)
           enddo
        elseif(tilt  >  0) then //FORE,iradar_ray=2,iaftfore= +1
           do ig = 1, nranges
              d_porte(MAXPORT+ig) = range(ig)  // This change fixed icorrupted infilename
           enddo
        endif
c Assign the swp number read from text file to num_swp
       num_swp = nsweep

c Assign the correction factors to Frank's variable
c NOTE: Here the correction factors are arrays with two elements
c This is different from any other variables

        if (tilt  <  0) then   // AFT, iradar_ray=1,iaftfore= -1
           corr_azest(1) = azimuth_correction;
           corr_elhor(1) = elevation_correction;
           corr_dist(1) = range_correction;
           corr_lon(1) = longitude_correction;
           corr_lat(1) = latitude_correction;
           corr_p_alt(1) = pressure_altitude_correction;
           corr_r_alt(1) = radar_altitude_correction;
           corr_vwe_av(1) = ew_gound_speed_correction;
           corr_vsn_av(1) = ns_ground_speed_correction;
           corr_vnz_av(1) = vertical_velocity_correction;
           corr_cap(1) = heading_correction;
           corr_roul(1) = roll_correction;
           corr_tang(1) = pitch_correction;
           corr_derv(1) = drift_correction;
           corr_rota(1) = rotation_correction;
           corr_incl(1) = tilt_correction;
        elseif(tilt  >  0) then   // FORE, iradar_ray=2,iaftfore= +1
           corr_azest(2) = azimuth_correction;
           corr_elhor(2) = elevation_correction;
           corr_dist(2) = range_correction;
           corr_lon(2) = longitude_correction;
           corr_lat(2) = latitude_correction;
           corr_p_alt(2) = pressure_altitude_correction;
           corr_r_alt(2) = radar_altitude_correction;
           corr_vwe_av(2) = ew_gound_speed_correction;
           corr_vsn_av(2) = ns_ground_speed_correction;
           corr_vnz_av(2) = vertical_velocity_correction;
           corr_cap(2) = heading_correction;
           corr_roul(2) = roll_correction;
           corr_tang(2) = pitch_correction;
           corr_derv(2) = drift_correction;
           corr_rota(2) = rotation_correction;
           corr_incl(2) = tilt_correction;
        endif
c


c TEST reading of text files
c       print*,'File:',infilename,' Ray:', counter
c    &      ,' HHMMSS:',ih_rdl,im_rdl,is_rdl,' EL:',elhor_rdl
c TEST-END

//******************************************************************
//**** CONTROL FOR THE END OF THE READING ALL TEXT FILES
//******************************************************************
c
  7   iend=0
      if(ifile == nfile  &&  lastfile  == 1)then
          iend=2
          print *,' '
          print *,'**** END OF READING ALL TEXT FILES ****'
      endif

c CAI-STOP


//******************************************************************
//**** CONTROL OF CURRENT TIME
//******************************************************************
c
      ih_ray=ih_rdl;
      im_ray=im_rdl;
      is_ray=is_rdl;
      ims_ray=ims_rdl;
      ihhmmss=10000*ih_ray+100*im_ray+is_ray;
      if(ihhmmss.le.0)go to 1
c
      time_ks=3.6*float(ih_ray)+0.06*float(im_ray)
     &        +0.001*float(is_ray)+1.e-6*float(ims_ray)
      if(    time_ks-time_prev < -80.
     &   .or.time_ks-tmin < -80.)then
        time_ks=time_ks+86.4
	ihhmmss=ihhmmss+240000
      endif
      time_prev=time_ks
      if(time_ks < tmin)then
        if(ihhmmss/10 > ihms_prev)then
	  print *,' HHMMSS:',ihhmmss,' < HHMMSS_min:',ihms_min
	  ihms_prev=ihhmmss/10
        endif
c CAI-START
        if(iend .ne. 2) go to 1   // only when end of text file not reached
c CAI-STOP
      endif
      if(time_ks > tmax)then
	iend=2
	print *,' '
	print *,' HHMMSSms:',100*ihhmmss+ims_rdl
     &         ,' > HHMMSSms_max:',100*ihms_max
      endif
      if(iend == 2)go to 2
c
//******************************************************************
//**** CONTROL OF LAT, LON, P_ALT AND R_ALT
//******************************************************************
c
      if(    abs(lat_av) < 0.001
     &   .or.abs(lon_av) < 0.001
     &   .or.(     abs(p_alt_av) < 0.001
     &         && abs(r_alt_av) < 0.001))go to 1

c	print *,'P_ALT_AV= ',p_alt_av
c
//******************************************************************
//**** RADAR IDENTIFICATION THROUGH TILT_RAY (=INCL_RDL)
//****  -> AFT : IRADAR_RAY=1, IAFTFORE=-1
//****  -> FORE : IRADAR_RAY=2, IAFTFORE=+1
//**********************F********************************************
c
      tilt_ray=incl_rdl;
      if(abs(tilt_ray) < 15.)then
        go to 1
      elseif(abs(tilt_ray) < 30.)then
        if(tilt_ray < -15.)then
          iradar_ray=1;
          iaftfore=-1;
          swp(iradar_ray)=num_swp;
        endif
        if(tilt_ray > +15.)then
          iradar_ray=2;
          iaftfore=+1;
          swp(iradar_ray)=num_swp;
        endif
      else
	go to 1
      endif
c////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      print *,' /////////////////////////////////////////////////'
c////////      print *,' '
c////////      print *,' /////////////////////////////////////////////////'
c////////      print *,'HHMMSSms:',1000*ihhmmss+ims_rdl
c////////     &       ,'   -  ROTA,INCL_rdl:',rota_rdl,incl_rdl
c////////      print *,'NUM_SWP,RAY :',num_swp,no_rdl
c////////      print *,'AZE,ELH_rdl:',azest_rdl,elhor_rdl
c////////      print *,'LON,LAT,ALT_av:',lon_av,lat_av,p_alt_av
c////////     &       ,' CAP_av:',cap_av
c////////      print *,'VENT_we,sn,nz:',vent_we,vent_sn,vent_nz
c////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** NYQUIST VELOCITY
//******************************************************************
c CAI-START----- Oliver's modification is for P3, for ELDORA, we don't
c                need these NYQUIST velocity stuff////////
c CAI-STOP
c
c     vnyq=vit_nonamb(iradar_ray)	// Mod Oliv
c////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c//////// THIS VALUE IS CORRECT FOR IPP1/IPP2=4/5 ONLY ////////
c      vnyq_el=vnyq/20.
c      vnyq_s=5.*vnyq_el	//Olivier
c      vnyq_l=4.*vnyq_el	//Olivier
c////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** CONTROL FOR AN END OF SWEEP
//******************************************************************
c
      if(nb_ray(iradar_ray) == 1)then
        tandrot=0.;
      else
        tandrot=tan(conv*(rota_rdl-rota_prev(iradar_ray)));
      endif

      if(     nb_ray(iradar_ray) > 1
     &    && (    (swp(iradar_ray).ne.swp_prev(iradar_ray))
     &         .or.(abs(tandrot) > 0.2)       ) )
     &  iend=1
c////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      if(iend.ge.1)then
c////////        print *,' '
c////////        print *,'NB_RAY(iradar_ray)=',nb_ray(iradar_ray)
c////////	print *,'SWP(iradar_ray),SWP_PREV(iradar_ray)='
c////////     &         ,swp(iradar_ray),swp_prev(iradar_ray)
c////////        print *,'ROTA_RDL,PREV=',rota_rdl,rota_prev(iradar_ray)
c////////     &         ,' -> TANDROT=',tandrot
c////////        print *,'=>IEND=',iend,' NUM_SWP=',swp(iradar_ray)
c////////      endif
c////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//****    END OF A SWEEP ( IEND = 1 )
//**** or END OF THE TAPE or END OF CONSIDERED PERIOD ( IEND = 2 )
//******************************************************************
c
  2   continue
      if(iend.ge.1)then
	rota_end(iradar_ray)=rota_prev(iradar_ray)
c
//******************************************************************
//**** MEAN VALUES FOR THE PAST SWEEP
//******************************************************************
c
        if(nb_ray(iradar_ray) > 1)then
          xp(iradar_ray)=float(nb_ray(iradar_ray))
c
          tilt_mean=stilt(iradar_ray)/xp(iradar_ray)
	  tilt_rms=sqrt(amax1(0.,(xp(iradar_ray)*stilt2(iradar_ray)
     &                 -stilt(iradar_ray)*stilt(iradar_ray))
     &                  /(xp(iradar_ray)*(xp(iradar_ray)-1))))
c
          nb_sweep(iradar_ray)=nb_sweep(iradar_ray)+1
          xacft_mean=sxa(iradar_ray)/xp(iradar_ray)
          yacft_mean=sya(iradar_ray)/xp(iradar_ray)
          zacft_mean=sza(iradar_ray)/xp(iradar_ray)
  	  acfthspd_mean=sacfthspd(iradar_ray)/xp(iradar_ray)
          time_ks_mean=stime(iradar_ray)/xp(iradar_ray)
	  ihmean=time_ks_mean/3.6
	  immean=(time_ks_mean-3.6*float(ihmean))/0.06
	  ismean=(time_ks_mean-3.6*float(ihmean)
     &	         -0.06*float(immean))/0.001
	  ihms=10000*ihmean+100*immean+ismean
          hdg_mean=atan2( ssc(iradar_ray)/xp(iradar_ray)
     &                   ,scc(iradar_ray)/xp(iradar_ray))/conv
	  uacft_mean=su_acft(iradar_ray)/xp_acft(iradar_ray)
	  vacft_mean=sv_acft(iradar_ray)/xp_acft(iradar_ray)
          wacft_mean=sw_acft(iradar_ray)/xp_acft(iradar_ray)
	  uwind_mean=su_wind(iradar_ray)/xp_wind(iradar_ray)
	  vwind_mean=sv_wind(iradar_ray)/xp_wind(iradar_ray)
          wwind_mean=sw_wind(iradar_ray)/xp_wind(iradar_ray)
c
//******************************************************************
//**** CONTROL PRINTS FOR THE PAST SWEEP
//******************************************************************
c
 	  print *,' '
	  print *,' '
	  print *,' *******************************************'
 	  print *,' **** CONTROL PRINTS FOR THE PAST SWEEP ****'
	  print *,' *******************************************'
 	  print *,' '
	  print *,' HHMMSS :',ihms
          print *,' RADAR(aft=1,fore=2) :',iradar_ray
          print *,' SWEEP(aft=-1,fore=+1) :',iaftfore
     &           ,' NO_SWEEP(this program) :',nb_sweep(iradar_ray)
     &           ,'      [ on tape :',swp(iradar_ray),' ]'
          print *,' X_we/OLON,Y_sn/OLAT,Z_acft :'
     &           ,xacft_mean,yacft_mean,zacft_mean
          print *,' HEADING :',hdg_mean
	  print *,' U,V,W_acft :',uacft_mean,vacft_mean,wacft_mean
	  print *,' U,V,W_insitu :',uwind_mean,vwind_mean,wwind_mean
          print *,' -> NB_RAYS_THIS_SWEEP :',nb_ray(iradar_ray)
          print *,' -> TILT_mean,rms :',tilt_mean,tilt_rms
	  print *,' -> ROTA_start,end :',rota_start(iradar_ray)
     &     	                        ,rota_end(iradar_ray)
          print *,' '
          print *,' -> NREF_OK:',nref_ok(iradar_ray)
     &           ,'    NDOP_OK:',ndop_ok(iradar_ray)
          print *,' '
c
	  if(kdzsurf == 1)then
	    if(swdzsurf_sweep(iradar_ray) > 0.)then
	      bias_dzsurf=dzsurfsweep_mean(iradar_ray)
     &                    /swdzsurf_sweep(iradar_ray)
	      stdv_dzsurf=sqrt(  swdzsurf_sweep(iradar_ray)
     &                          *dzsurfsweep_rms(iradar_ray)
     &                         - dzsurfsweep_mean(iradar_ray)
     &                          *dzsurfsweep_mean(iradar_ray))
     &                    /swdzsurf_sweep(iradar_ray)
	      print *,' -> dZHSURF_npts,swghts,mean,stdv :'
     &               ,n_dzsurf(iradar_ray),swdzsurf_sweep(iradar_ray)
     &               ,bias_dzsurf,stdv_dzsurf
              if(iwrisurfile == 1)
     &          print *,'     [ NPTS_SURF FOR SURF_EL_*:'
     &                 ,nsurf_wri(iradar_ray),' ]'
            else
	      print *,' //////// NPTS_dZHSURF :',n_dzsurf(iradar_ray),' ////////'
            endif
          endif
c
	  if(kvsurf == 1)then
	    if(swvsurf_sweep(iradar_ray) > 0.)then
	      bias_vsurf=vsurfsweep_mean(iradar_ray)
     &                    /swvsurf_sweep(iradar_ray)
	      stdv_vsurf=sqrt(  swvsurf_sweep(iradar_ray)
     &                          *vsurfsweep_rms(iradar_ray)
     &                         - vsurfsweep_mean(iradar_ray)
     &                          *vsurfsweep_mean(iradar_ray))
     &                    /swvsurf_sweep(iradar_ray)
	      print *,' -> VSURF_npts,swghts,mean,stdv :'
     &               ,n_vsurf(iradar_ray),swvsurf_sweep(iradar_ray)
     &               ,bias_vsurf,stdv_vsurf
            else
	      print *,' //////// NPTS_VSURF :',n_vsurf(iradar_ray),' ////////'
	      print *,' //////// Ndismissed_VACFT,VDOPCORR,VDOPSURF:'
     &               ,ndismiss_vhacft(iradar_ray)
     &               ,ndismiss_vdopcorr(iradar_ray)
     &               ,ndismiss_vdopsurf(iradar_ray),' ////////'
            endif
          endif
c
	  if(kdvinsitu == 1)then
	    if(swinsitu_sweep(iradar_ray) > 0.)then
	      bias_dvinsitu=dvinsitusweep_mean(iradar_ray)
     &                      /swinsitu_sweep(iradar_ray)
	      stdv_dvinsitu=sqrt(  swinsitu_sweep(iradar_ray)
     &                            *dvinsitusweep_rms(iradar_ray)
     &                           - dvinsitusweep_mean(iradar_ray)
     &                            *dvinsitusweep_mean(iradar_ray))
     &                    /swinsitu_sweep(iradar_ray)
	      print *,' -> dVINSITU_npts,swghts,mean,stdv :'
     &               ,n_dvinsitu(iradar_ray),swinsitu_sweep(iradar_ray)
     &               ,bias_dvinsitu,stdv_dvinsitu
              print *,'     -> LEFT_swghts,mean,stdv:'
     &               ,s_vpv(iradar_ray,1)
     &               ,sv_vpv(iradar_ray,1)
     &                /amax1(0.001,s_vpv(iradar_ray,1))
     &               ,sqrt( s_vpv(iradar_ray,1)*svv_vpv(iradar_ray,1)
     &                     -sv_vpv(iradar_ray,1)*sv_vpv(iradar_ray,1))
     &                /amax1(0.001,s_vpv(iradar_ray,1))
              print *,'     -> RIGHT_swghts,mean,stdv:'
     &               ,s_vpv(iradar_ray,2)
     &               ,sv_vpv(iradar_ray,2)
     &                /amax1(0.001,s_vpv(iradar_ray,2))
     &               ,sqrt( s_vpv(iradar_ray,2)*svv_vpv(iradar_ray,2)
     &                     -sv_vpv(iradar_ray,2)*sv_vpv(iradar_ray,2))
     &                /amax1(0.001,s_vpv(iradar_ray,2))
            else
	      print *,' //////// NPTS_VINSITU :'
     &               ,n_dvinsitu(iradar_ray),' ////////'
            endif
          endif
          print *,' '
	  print *,' *******************************************'
 	  print *,' '
 	  print *,' '
c
//******************************************************************
//**** WRITE THE RESULTS FOR THE PAST SWEEP
//**** ON THE "SIS_EL_*" FILE #50
//******************************************************************
c
//**** SWEEP HEADER
c
          write(50)iaftfore,nb_sweep(iradar_ray)
     &             ,xacft_mean,yacft_mean,zacft_mean
     &             ,time_ks_mean,hdg_mean
     &             ,u_mean,v_mean,w_mean
c
//******************************************************************
//**** SWEEP DATA: DZ_surf
//******************************************************************
c
          print *,' '
          if(kdzsurf == 1 && n_dzsurf(iradar_ray) > 0)then
            write(50)n_dzsurf(iradar_ray)
            write(50)( zs_rot(iradar_ray,n)
     &                ,zs_el(iradar_ray,n),zs_az(iradar_ray,n)
     &                ,zs_dsurf(iradar_ray,n),zs_dhor(iradar_ray,n)
     &                ,zs_zsurf(iradar_ray,n),zs_hsurf(iradar_ray,n)
     &                ,n=1,n_dzsurf(iradar_ray))
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      print *,' '
c////////      print *,' SIS_* -> NPTS_Zsurf:',n_dzsurf(iradar_ray)
c////////      print *,' [ ROT - DH - Z_surf - H_surf ]'
c////////      do n=1,n_dzsurf(iradar_ray)
c////////         print *,zs_rot(iradar_ray,n),zs_dhor(iradar_ray,n)
c////////     &          ,zs_zsurf(iradar_ray,n),zs_hsurf(iradar_ray,n)
c////////      enddo
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          else
            write(50)0
          endif
c
//******************************************************************
//**** SWEEP DATA: VDOP_surf
//******************************************************************
c
          if(kvsurf == 1 && n_vsurf(iradar_ray) > 0)then
            write(50)n_vsurf(iradar_ray)
            write(50)(vs_dhor(iradar_ray,n),vs_vdopsurf(iradar_ray,n)
     &                ,n=1,n_vsurf(iradar_ray))
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      print *,' '
c////////      print *,' SIS_* -> NPTS_VDOP_surf:',n_vsurf(iradar_ray)
c////////      print *,' [ DH - VDOP_surf ]'
c////////      do n=1,n_vsurf(iradar_ray)
c////////         print *,vs_dhor(iradar_ray,n),vs_vdopsurf(iradar_ray,n)
c////////      enddo
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          else
            write(50)0
          endif
c
//******************************************************************
//**** SWEEP DATA: DVDOP_insitu
//******************************************************************
c
          if(kdvinsitu == 1 && n_dvinsitu(iradar_ray) > 0)then
            write(50)n_dvinsitu(iradar_ray)
            write(50)( vi_dhor(iradar_ray,n)
     &                ,vi_vdop(iradar_ray,n),vi_vinsitu(iradar_ray,n)
     &                ,n=1,n_dvinsitu(iradar_ray))
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      print *,' '
c////////      print *,' NPTS_Vinsitu:',n_dvinsitu(iradar_ray)
c////////      print *,' DH - Vdop - Vinsitu'
c////////      do n=1,n_dvinsitu(iradar_ray)
c////////         print *,vi_dhor(iradar_ray,n)
c////////     &          ,vi_vdop(iradar_ray,n),vi_vinsitu(iradar_ray,n)
c////////      enddo
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          else
            write(50)0
          endif
c
        endif    ////  of  //// if(nb_ray(iradar_ray) > 1)then  ////
c
//******************************************************************
//**** END OF THE TAPE or END OF CONSIDERED PERIOD ( IEND = 2 )
//******************************************************************
c
        if(iend == 2)then
	  print *,' '
	  print *,' ****************************************************'
	  print *,'   HHMMSS :',ih_ray,im_ray,is_ray
     &           ,'   -> END OF CONSIDERED PERIOD'
	  print *,'   NB_SWEEPS_READ FOR AFT AND FORE RADARS :'
     &           ,nb_sweep
	  print *,' ****************************************************'
	  print *,' '
	  print *,' '
          write(10,"(' NB_SWEEPS FOR THE AFT AND FORE RADARS: '
     &               ,2i5,/)")
     &              ,int(xsweeps(1)),int(xsweeps(2))
c
//******************************************************************
//****  SUM OF INDIVIDUAL WEIGHTS, MEAN, RMS VALUES OF
//****    - DIFFERENCE OF SURFACE ALTITUDE (RADAR-DTM);
//****    - DOPPLER VELOCITY OF THE GROUND CLUTTER;
//****    - DIFFERENCE OF DOPPLER VELOCITY (RADAR-FLIGHT_LEVEL);
//******************************************************************
c
	  print *,' '
	  print *,' **********************************************'
	  print *,' ************ MEAN AND RMS ERRORS *************'
	  print *,' **********************************************'
	  print *,' '
c
	  if(kdzsurf == 1)then
	    bias_dzsurf=swdzmsurf_tot/amax1(1.,swdzsurf_tot)
	    stdv_dzsurf=sqrt( swdzsurf_tot*swdz2surf_tot
     &                       -swdzmsurf_tot*swdzmsurf_tot)
     &                  /amax1(1.,swdzsurf_tot)
            print *,' '
            print *,' dZ_surf (km) sum_wghts,mean,stdv :'
     &             ,swdzsurf_tot,bias_dzsurf,stdv_dzsurf
            write(10,"(//,' dZ_surf (km) sum_wghts,mean,stdv :'
     &                 ,f10.1,2f8.3,/)")
     &            swdzsurf_tot,bias_dzsurf,stdv_dzsurf
          endif
c
	  if(kvsurf == 1)then
	    bias_vsurf=swvmsurf_tot/amax1(1.,swvsurf_tot)
	    stdv_vsurf=sqrt( swvsurf_tot*swv2surf_tot
     &                      -swvmsurf_tot*swvmsurf_tot)
     &                 /amax1(1.,swvsurf_tot)
            print *,' '
            print *,' VDOP_surf (m/s) sum_wghts,mean,stdv :'
     &             ,swvsurf_tot,bias_vsurf,stdv_vsurf
            write(10,"(' VDOP_surf (m/s) sum_wghts,mean,stdv :'
     &                 ,f10.1,2f8.3,/)")
     &            swvsurf_tot,bias_vsurf,stdv_vsurf
          endif
c
	  if(kdvinsitu == 1)then
	    bias_dvinsitu=swdvminsitu_tot/amax1(1.,swdvinsitu_tot)
	    stdv_dvinsitu=sqrt( swdvinsitu_tot*swdv2insitu_tot
     &                         -swdvminsitu_tot*swdvminsitu_tot)
     &                    /amax1(1.,swdvinsitu_tot)
            print *,' '
            print *,' dVDOP_insitu (m/s) sum_wghts,mean,stdv :'
     &             ,swdvinsitu_tot,bias_dvinsitu,stdv_dvinsitu
            write(10,"(' dVDOP_insitu (m/s) sum_wghts,mean,stdv :'
     &                 ,f10.1,2f8.3)")
     &            swdvinsitu_tot,bias_dvinsitu,stdv_dvinsitu
	    do iradar=1,2
	       print *,'   IRADAR (AR=1,AV=2) :',iradar
               bias_dvinsitu_ir_g
     &          =xv_vpv(iradar,1)/amax1(1.,x_vpv(iradar,1))
               stdv_dvinsitu_ir_g
     &          =sqrt( x_vpv(iradar,1)*xvv_vpv(iradar,1)
     &                -xv_vpv(iradar,1)*xv_vpv(iradar,1))
     &           /amax1(1.,x_vpv(iradar,1))
               print *,'     -> VDOP-PROJWIND_LEFT_npts,mean,stdv:'
     &                ,x_vpv(iradar,1)
     &                ,bias_dvinsitu_ir_g,stdv_dvinsitu_ir_g
               write(10,"('   IRADAR (AR=1,AV=2) :',i1,/
     &                    ,'    -> VDOP-PROJWIND_LEFT_npts,mean,stdv:'
     &                    ,f10.1,2f8.3)")
     &               iradar,x_vpv(iradar,1)
     &              ,bias_dvinsitu_ir_g,stdv_dvinsitu_ir_g
               bias_dvinsitu_ir_d
     &          =xv_vpv(iradar,2)/amax1(1.,x_vpv(iradar,2))
               stdv_dvinsitu_ir_d
     &          =sqrt( x_vpv(iradar,2)*xvv_vpv(iradar,2)
     &                -xv_vpv(iradar,2)*xv_vpv(iradar,2))
     &           /amax1(1.,x_vpv(iradar,2))
               print *,'     -> VDOP-PROJWIND_RIGHT_npts,mean,stdv:'
     &                ,x_vpv(iradar,2)
     &               ,bias_dvinsitu_ir_d,stdv_dvinsitu_ir_d
               write(10,"('    -> VDOP-PROJWIND_RIGHT_npts,mean,stdv:'
     &                    ,f10.1,2f8.3)")
     &               x_vpv(iradar,2)
     &              ,bias_dvinsitu_ir_d,stdv_dvinsitu_ir_d
	    enddo
	    print *,' '
            write(10,"(//)")
          endif
c
	  print *,' '
	  print *,' **********************************************'
	  print *,' '
	  print *,' '
	  print *,' '
c
//******************************************************************
//****  (IF SUM_WGHTS_surf+insitu > SUM_WGHTS_min)
//****   -> NAVIGATIONAL ERROS CAN BE CALCULATED
//******************************************************************
c
	  if(ssurfins > ssurfins_min)then
c
//******************************************************************
//**** RMS VALUES OF THE NORMALIZED VARIABLES
//******************************************************************
c
	    print *,' '
	    print *,' **********************************************'
	    print *,' *** RMS VALUES OF THE NORMALIZED VARIABLES ***'
	    print *,' **********************************************'
	    print *,' '
c
            if(swdzsurf_tot > 1.)then
              print *,' DZ_surf -> sWGHTs:',swdzsurf_tot
              print *,'          rms_VAR(dTaft,dTfore):'
     &               ,(sqrt(rms_var_zsurf(i)/swadzsurf_tot),i=1,2)
              print *,'          -------(dRaft,dRfore):'
     &               ,(sqrt(rms_var_zsurf(i)/swadzsurf_tot),i=3,4)
              print *,'          -------(dPitch,dHdg):'
     &               ,(sqrt(rms_var_zsurf(i)/swadzsurf_tot),i=5,6)
              print *,'          -------(RDaft,RDfore):'
     &               ,(sqrt(rms_var_zsurf(i)/swadzsurf_tot),i=7,8)
              print *,'          -------(dXwe,dYsn,dZacft):'
     &               ,(sqrt(rms_var_zsurf(i)/swadzsurf_tot),i=9,11)
              print *,'          -------(dVHacft):'
     &               ,sqrt(rms_var_zsurf(12)/swadzsurf_tot)
            else
              print *,' //////// DZ_surf -> sWGHTs:',swdzsurf_tot,' ////////'
            endif
c
            if(swvsurf_tot > 1.)then
              print *,' VDOP_surf -> sWGHTs:',swvsurf_tot
              print *,'          rms_VAR(dTaft,dTfore):'
     &               ,(sqrt(rms_var_vsurf(i)/swavsurf_tot),i=1,2)
              print *,'          -------(dRaft,dRfore):'
     &               ,(sqrt(rms_var_vsurf(i)/swavsurf_tot),i=3,4)
              print *,'          -------(dPitch,dHdg):'
     &               ,(sqrt(rms_var_vsurf(i)/swavsurf_tot),i=5,6)
              print *,'          -------(RDaft,RDfore):'
     &               ,(sqrt(rms_var_vsurf(i)/swavsurf_tot),i=7,8)
              print *,'          -------(dXwe,dYsn,dZacft):'
     &               ,(sqrt(rms_var_vsurf(i)/swavsurf_tot),i=9,11)
              print *,'          -------(dVHacft):'
     &               ,sqrt(rms_var_vsurf(12)/swavsurf_tot)
            else
              print *,' //////// VDOP_surf -> sWGHTs:',swvsurf_tot,' ////////'
            endif
c
            if(swdvinsitu_tot > 1.)then
              print *,' DVDOP_insitu -> sWGHTs:',swdvinsitu_tot
              print *,'          rms_VAR(dTaft,dTfore):'
     &               ,(sqrt(rms_var_vinsitu(i)/swadvinsitu_tot),i=1,2)
              print *,'          -------(dRaft,dRfore):'
     &               ,(sqrt(rms_var_vinsitu(i)/swadvinsitu_tot),i=3,4)
              print *,'          -------(dPitch,dHdg):'
     &               ,(sqrt(rms_var_vinsitu(i)/swadvinsitu_tot),i=5,6)
              print *,'          -------(RDaft,RDfore):'
     &               ,(sqrt(rms_var_vinsitu(i)/swadvinsitu_tot),i=7,8)
              print *,'          -------(dXwe,dYsn,dZacft):'
     &               ,(sqrt(rms_var_vinsitu(i)/swadvinsitu_tot),i=9,11)
              print *,'          -------(dVHacft):'
     &               ,sqrt(rms_var_vinsitu(12)/swadvinsitu_tot)
            else
              print *,' //////// DVDOP_insitu -> sWGHTs:'
     &               ,swdvinsitu_tot,' ////////'
            endif
c
//******************************************************************
//**** NORMALIZED CORRELATION MATRIX BETWEEN THE NVAR VARIABLES
//******************************************************************
c
            print *,' '
            sp_zsvszi=swdzsurf_tot+swvsurf_tot+swdvinsitu_tot
            if(sp_zsvszi > 1.)then
	    print *,' **********************************************'
	    print *,' ******* NORMALIZED CORRELATION MATRIX ********'
	    print *,' *******            (*1000)            ********'
	    print *,' *******   BETWEEN THE NVAR VARIABLES  ********'
	    print *,' **********************************************'
	    print *,' '
              print *,'        dTa-dTf-dRa-dRf-dP-dH-RDa-RDf'
     &               ,'-dX-dY-dZ-dV '
              do i=1,nvar
c
                 if(i == 1)then
                   print *,' dTa  - '
     &                    ,(int(1000.*corr_var(i,j)
     &                          /amax1( 0.01
     &                                 ,sqrt( corr_var(i,i)
     &                                       *corr_var(j,j))))
     &                      ,j=1,nvar)
c
                 elseif(i == 2)then
                   print *,' dTf  - '
     &                    ,(int(1000.*corr_var(i,j)
     &                          /amax1( 0.01
     &                                 ,sqrt( corr_var(i,i)
     &                                       *corr_var(j,j))))
     &                      ,j=1,nvar)
c
                 elseif(i == 3)then
                   print *,' dRa  - '
     &                    ,(int(1000.*corr_var(i,j)
     &                          /amax1( 0.01
     &                                 ,sqrt( corr_var(i,i)
     &                                       *corr_var(j,j))))
     &                      ,j=1,nvar)
c
                 elseif(i == 4)then
                   print *,' dRf  - '
     &                    ,(int(1000.*corr_var(i,j)
     &                          /amax1( 0.01
     &                                 ,sqrt( corr_var(i,i)
     &                                       *corr_var(j,j))))
     &                      ,j=1,nvar)
c
                 elseif(i == 5)then
                   print *,' dP   - '
     &                    ,(int(1000.*corr_var(i,j)
     &                          /amax1( 0.01
     &                                 ,sqrt( corr_var(i,i)
     &                                       *corr_var(j,j))))
     &                      ,j=1,nvar)
c
                 elseif(i == 6)then
                   print *,' dH   - '
     &                    ,(int(1000.*corr_var(i,j)
     &                          /amax1( 0.01
     &                                 ,sqrt( corr_var(i,i)
     &                                       *corr_var(j,j))))
     &                      ,j=1,nvar)
c
                 elseif(i == 7)then
                   print *,' RDa - '
     &                    ,(int(1000.*corr_var(i,j)
     &                          /amax1( 0.01
     &                                 ,sqrt( corr_var(i,i)
     &                                       *corr_var(j,j))))
     &                      ,j=1,nvar)
c
                 elseif(i == 8)then
                   print *,' RDf - '
     &                    ,(int(1000.*corr_var(i,j)
     &                          /amax1( 0.01
     &                                 ,sqrt( corr_var(i,i)
     &                                       *corr_var(j,j))))
     &                      ,j=1,nvar)
c
                 elseif(i == 9)then
                   print *,' dX   - '
     &                    ,(int(1000.*corr_var(i,j)
     &                          /amax1( 0.01
     &                                 ,sqrt( corr_var(i,i)
     &                                       *corr_var(j,j))))
     &                      ,j=1,nvar)
c
                 elseif(i == 10)then
                   print *,' dY   - '
     &                    ,(int(1000.*corr_var(i,j)
     &                          /amax1( 0.01
     &                                 ,sqrt( corr_var(i,i)
     &                                       *corr_var(j,j))))
     &                      ,j=1,nvar)
                 elseif(i == 11)then
                   print *,' dZ   - '
     &                    ,(int(1000.*corr_var(i,j)
     &                          /amax1( 0.01
     &                                 ,sqrt( corr_var(i,i)
     &                                       *corr_var(j,j))))
     &                      ,j=1,nvar)
c
                 elseif(i == 12)then
                   print *,' dV   - '
     &                    ,(int(1000.*corr_var(i,j)
     &                          /amax1( 0.01
     &                                 ,sqrt( corr_var(i,i)
     &                                       *corr_var(j,j))))
     &                      ,j=1,nvar)
c
                 endif
              enddo
            else
              print *,' //////// sw_Zsurf+Vsurf+Vinsitu:',sp_zsvsvi,' ////////'
            endif
            print *,' '
c
//******************************************************************
//**** NORMALIZATION OF THE MATRICES AND VECTORS
//**** FOR DZ_surf, VDOP_surf et DVDOP_insitu
//**** BY THE SUM OF THE POSITIVE VALUES OF THE OBSERVED ERRORS
//**** THEN BUILD A UNIQUE MATRICE AND VECTOR BY SUMMING
//******************************************************************
c
            print *,' '
            print *,' NORMALIZATION OF THE MATRICES AND VECTORS'
            print *,' SumPosVal_DZsurf,VDOPsurf,DVDOPinsitu:'
     &             ,swadzsurf_tot,swavsurf_tot,swadvinsitu_tot
     &
            itest_xmat=0
            do i=1,nvar
               vect(i)=0.
               if(kdzsurf == 1 && swadzsurf_tot > 0.)
     &           vect(i)=vect(i)+rw_dzsurf*vect_dzsurf(i)
     &                           /swadzsurf_tot
               if(kvsurf == 1 && swavsurf_tot > 0.)
     &           vect(i)=vect(i)+rw_vsurf*vect_vsurf(i)
     &                           /swavsurf_tot
               if(kdvinsitu == 1 && swadvinsitu_tot > 0.)
     &           vect(i)=vect(i)+rw_dvinsitu*vect_dvinsitu(i)
     &                           /swadvinsitu_tot
               do j=1,nvar
                  xmat(i,j)=0.
                  if(kdzsurf == 1 && swadzsurf_tot > 0.)
     &              xmat(i,j)=xmat(i,j)+rw_dzsurf*xmat_dzsurf(i,j)
     &                                  /swadzsurf_tot
                  if(kvsurf == 1 && swavsurf_tot > 0.)
     &              xmat(i,j)=xmat(i,j)+rw_vsurf*xmat_vsurf(i,j)
     &                                  /swavsurf_tot
                  if(kdvinsitu == 1 && swadvinsitu_tot > 0.)
     &              xmat(i,j)=xmat(i,j)+rw_dvinsitu*xmat_dvinsitu(i,j)
     &                                  /swadvinsitu_tot
                  if(abs(xmat(i,j)) > 0.)itest_xmat=1
               enddo
c
//******************************************************************
//**** CHECK THAT NO ELEMENT OF THE MATRIX' MAIN DIAGONAL IS NULL
//******************************************************************
c
               if(abs(xmat(i,i)).le.0.)then
                 do j=1,nvar
                    xmat(i,j)=0.;
                    xmat(j,i)=0.;
                 enddo
                 xmat(i,i)=1.;
                 vect(i)=0.;
               endif
c
            enddo
            print *,' '
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      print *,' '
c////////      print *,' NORMALIZED MATRIX AND VECTOR'
c////////      do i=1,nvar
c////////         print *,' i:',i,' XMAT(i,1->nvar)',(xmat(i,j),j=1,nvar)
c////////     &          ,' VECT(i):',vect(i)
c////////      enddo
c////////      print *,' '
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** INVERSION OF THE MATRIX
//**** CALCULATION OF THE RESULTING VECTOR
//******************************************************************
c
            if(itest_xmat == 1)then
              call resoud(xmat,xinv,vect,res,nvar)
            else
              print *,' //////// XMAT=0 ////////'
            endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      print *,' '
c////////      print *,' RESULTING VECTOR'
c////////      do i=1,nvar
c////////         print *,' RES(',i,'):',res(i)
c////////      enddo
c////////      print *,' '
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** ASSIGNEMENT OF THE RESULTS
//******************************************************************
c

c CAI START -- add the function to write out cfac files in SOLO format
            print *,' '
            print *,' '
            print *,' '
            print *,' /////////////////////////////////////////////'
            print *,'     CORRECTIONS FOR NAVIGATIONAL ERRORS'
            print *,' //////////// (add these values)  ////////////'
            print *,' /////////////////////////////////////////////'
            print *,' '
            print *,' '
c
            write(10
     &            ,"(//
     &               ,' /////////////////////////////////////////////'
     &             ,/,'    CORRECTIONS FOR NAVIGATIONAL ERRORS'
     &             ,/,' //////////// (add these values)  ////////////'
     &             ,/,' /////////////////////////////////////////////'
     &               ,//)")
c
            if(idtiltaft == 1)then
              dtiltaft_res=res(1)
              print *,' D_TILT_aft (deg) guess,residual,total : '
     &               ,dtiltaft_guess,dtiltaft_res
     &               ,dtiltaft_guess+dtiltaft_res
              write(10,"(' D_TILT_aft (deg) guess,residual,total : '
     &                   ,3f7.3,/)")
     &             dtiltaft_guess,dtiltaft_res
     &            ,dtiltaft_guess+dtiltaft_res
c CAI
              tilt_corr_aft = dtiltaft_guess+dtiltaft_res;
            else
              dtiltaft_res=0.;
              tilt_corr_aft = 0.0;
            endif
c
            if(idtiltfore == 1)then
              dtiltfore_res=res(2)
              print *,' D_TILT_fore (deg) guess,residual,total : '
     &               ,dtiltfore_guess,dtiltfore_res
     &               ,dtiltfore_guess+dtiltfore_res
              write(10,"(' D_TILT_fore  (deg) guess,residual,total : '
     &                   ,3f7.3,/)")
     &             dtiltfore_guess,dtiltfore_res
     &            ,dtiltfore_guess+dtiltfore_res
c CAI
              tilt_corr_fore = dtiltfore_guess+dtiltfore_res;
            else
              dtiltfore_res=0.;
              tilt_corr_fore = 0.0;
            endif
c
            if(idrotaaft == 1)then
              drotaaft_res=res(3);
              print *,' D_ROTA_aft (deg) guess,residual,total : '
     &               ,drotaaft_guess,drotaaft_res
     &               ,drotaaft_guess+drotaaft_res
              write(10,"(' D_dROTA_aft (deg) guess,residual,total : '
     &                   ,3f7.3,/)")
     &             drotaaft_guess,drotaaft_res
     &            ,drotaaft_guess+drotaaft_res
c CAI
              rot_angle_corr_aft = drotaaft_guess+drotaaft_res;
            else
              drotaaft_res=0.;
              rot_angle_corr_aft = 0.0;
            endif
c
            if(idrotafore == 1)then
              drotafore_res=res(4);
              print *,' D_ROTA_fore (deg) guess,residual,total : '
     &               ,drotafore_guess,drotafore_res
     &               ,drotafore_guess+drotafore_res
              write(10,"(' D_dROTA_fore (deg) guess,residual,total : '
     &                   ,3f7.3,/)")
     &             drotafore_guess,drotafore_res
     &            ,drotafore_guess+drotafore_res
c CAI
              rot_angle_corr_fore = drotafore_guess+drotafore_res;
            else
              drotafore_res=0.;
              rot_angle_corr_fore = 0.0;
            endif
c
            if(idpitch == 1)then
              dpitch_res=res(5);
              print *,' D_PITCH (deg) guess,residual,total : '
     &               ,dpitch_guess,dpitch_res,dpitch_guess+dpitch_res
              write(10,"(' D_PITCH (deg) guess,residual,total : '
     &                   ,3f7.3,/)")
     &             dpitch_guess,dpitch_res,dpitch_guess+dpitch_res
c CAI
              pitch_corr_cfac = dpitch_guess+dpitch_res;
            else
              dpitch_res=0.;
              pitch_corr_cfac = 0.0;
            endif
c
            if(idhdg == 1)then
              dhdg_res=res(6);
              print *,' D_HEADING (deg) guess,residual,total : '
     &               ,dhdg_guess,dhdg_res,dhdg_guess+dhdg_res
              write(10,"(' D_HEADING (deg) guess,residual,total : '
     &                   ,3f7.3,/)")
     &             dhdg_guess,dhdg_res,dhdg_guess+dhdg_res
c CAI
              drift_corr_cfac = dhdg_guess+dhdg_res
            else
              dhdg_res=0.;
              drift_corr_cfac = 0.0;
            endif
c
            if(irdaft == 1)then
              rdaft_res=100.*res(7);
              print *,' RANGE_DELAY_AFT (m) guess,residual,total : '
     &               ,1000.*rdaft_guess,rdaft_res
     &               ,1000.*rdaft_guess+rdaft_res
              write(10,"(' RANGE_DELAY_AFT (m) guess,residual,total : '
     &                   ,3f6.0,/)")
     &             1000.*rdaft_guess,rdaft_res
     &            ,1000.*rdaft_guess+rdaft_res
c CAI
              range_delay_corr_aft = 1000.*rdaft_guess+rdaft_res;
            else
              rdaft_res=0.;
              range_delay_corr_aft = 0.0;
            endif
c
            if(irdfore == 1)then
              rdfore_res=100.*res(8);
              print *,' RANGE_DELAY_FORE (m) guess,residual,total : '
     &               ,1000.*rdfore_guess,rdfore_res
     &               ,1000.*rdfore_guess+rdfore_res
              write(10,"(' RANGE_DELAY_FORE (m) guess,residual,total : '
     &                   ,3f6.0,/)")
     &             1000.*rdfore_guess,rdfore_res
     &            ,1000.*rdfore_guess+rdfore_res
c CAI
              range_delay_corr_fore = 1000.*rdfore_guess+rdfore_res;
            else
              rdfore_res=0.;
              range_delay_corr_fore = 0.0;
            endif
c
            if(idxwe == 1)then
              dxwe_res=100.*res(9);
              print *,' D_XWE (m) guess,residual,total : '
     &               ,1000.*dxwe_guess,dxwe_res
     &               ,1000.*dxwe_guess+dxwe_res
              write(10,"(' D_XWE (m) guess,residual,total : '
     &                   ,3f6.0,/)")
     &             1000.*dxwe_guess,dxwe_res
     &            ,1000.*dxwe_guess+dxwe_res
            else
              dxwe_res=0.;
            endif
c
            if(idysn == 1)then
              dysn_res=100.*res(10);
              print *,' D_YSN (m) guess,residual,total : '
     &               ,1000.*dysn_guess,dysn_res
     &               ,1000.*dysn_guess+dysn_res
              write(10,"(' D_YSN (m) guess,residual,total : '
     &                   ,3f6.0,/)")
     &             1000.*dysn_guess,dysn_res
     &            ,1000.*dysn_guess+dysn_res
            else
              dxwe_res=0.;
            endif
c
            if(idzacft == 1)then
              dzacft_res=100.*res(11);
              print *,' D_ZACFT (m) guess,residual,total : '
     &               ,1000.*dzacft_guess,dzacft_res
     &               ,1000.*dzacft_guess+dzacft_res
              write(10,"(' D_ZACFT (m) guess,residual,total : '
     &                   ,3f6.0,/)")
     &             1000.*dzacft_guess,dzacft_res
     &            ,1000.*dzacft_guess+dzacft_res
c CAI
              pressure_alt_corr = 1000.*dzacft_guess+dzacft_res
            else
              dzacft_res=0.;
              pressure_alt_corr = 0.0;
            endif
c
            if(idvh == 1)then
              dvh_res=res(12);
              print *,' D_VHACFT (m/s) guess,residual,total : '
     &               ,dvh_guess,dvh_res,dvh_guess+dvh_res
              write(10,"(' D_VHACFT (m/s) guess,residual,total : '
     &                   ,3f6.2,/)")
     &             dvh_guess,dvh_res,dvh_guess+dvh_res
c CAI
              ew_gndspd_corr = dvh_guess+dvh_res
            else
              dvh_res=0.;
              ew_gndspd_corr = 0.0;
            endif
c
            print *,' '
	    print *,' '
	    print *,' '
	    print *,' '
	    print *,' //////////////////////////////////////////////////'
	    print *,' //////////////////////////////////////////////////'
	    print *,' //////////////////////////////////////////////////'
c
            write(10,"(///
     &                 ,' /////////////////////////////////////////////'
     &                )")
c
	  else    ////  of  ////  if(ssurfins > ssurfins_min)then  ////
c
            write(10,"(/////
     &                ,' /////////////////////////////////////////////'
     &              ,/,'    NO CORRECTIONS FOR NAVIGATIONAL ERRORS'
     &              ,/,' //////////// (not enough points) ////////////'
     &              ,/,' /////////////////////////////////////////////'
     &              ,///)")
c
	    print *,' '
	    print *,' '
	    print *,' '
	    print *,' /////////////////////////////////////////////'
	    print *,'    NO CORRECTIONS FOR NAVIGATIONAL ERRORS'
	    print *,' //////////// (not enough points) ////////////'
	    print *,' /////////////////////////////////////////////'
	    print *,' '
c
	  endif    ////  of  ////  if(ssurfins > ssurfins_min)then  ////
          print *,' '
          print *,' END OF "CORNAV_EL_*" FILE #10 :'
     &           ,directory(1:ndir)//'/'//fich_cornav
	  close(10)
c
c CAI
//******************************************************************
c             Write the cfac files using SOLO format
//******************************************************************

c Write the aft cafc file

         open(11,file=directory(1:ndir)//'/'//'cfac.aft'
     &       ,form='formatted',status='unknown')

              write(11,"('azimuth_corr           ='
     &                   ,f8.3)")0.0

              write(11,"('elevation_corr         ='
     &                   ,f8.3)")0.0

              write(11,"('range_delay_corr       ='
     &                   ,f8.3)")range_delay_corr_aft

              write(11,"('longitude_corr         ='
     &                   ,f8.3)")0.0
              write(11,"('latitude_corr          ='
     &                   ,f8.3)")0.0
              write(11,"('pressure_alt_corr      ='
     &                   ,f8.3)")pressure_alt_corr
              write(11,"('radar_alt_corr         ='
     &                   ,f8.3)")0.0
              write(11,"('ew_gndspd_corr         ='
     &                   ,f8.3)")ew_gndspd_corr

              write(11,"('ns_gndspd_corr         ='
     &                   ,f8.3)")0.0
              write(11,"('vert_vel_corr          ='
     &                   ,f8.3)")0.0
              write(11,"('heading_corr           ='
     &                   ,f8.3)")0.0
              write(11,"('roll_corr              ='
     &                   ,f8.3)")0.0
              write(11,"('pitch_corr             ='
     &                   ,f8.3)")pitch_corr_cfac
              write(11,"('drift_corr             ='
     &                   ,f8.3)")drift_corr_cfac
              write(11,"('rot_angle_corr         ='
     &                   ,f8.3)")rot_angle_corr_aft
              write(11,"('tilt_corr              ='
     &                   ,f8.3)")tilt_corr_aft

              close(11)

c Write the fore cafc file

          open(12,file=directory(1:ndir)//'/'//'cfac.fore'
     &       ,form='formatted',status='unknown')

              write(12,"('azimuth_corr           ='
     &                   ,f8.3)")0.0

              write(12,"('elevation_corr         ='
     &                   ,f8.3)")0.0

              write(12,"('range_delay_corr       ='
     &                   ,f8.3)")range_delay_corr_fore

              write(12,"('longitude_corr         ='
     &                   ,f8.3)")0.0
              write(12,"('latitude_corr          ='
     &                   ,f8.3)")0.0
              write(12,"('pressure_alt_corr      ='
     &                   ,f8.3)")pressure_alt_corr
              write(12,"('radar_alt_corr         ='
     &                   ,f8.3)")0.0
              write(12,"('ew_gndspd_corr         ='
     &                   ,f8.3)")ew_gndspd_corr

              write(12,"('ns_gndspd_corr         ='
     &                   ,f8.3)")0.0
              write(12,"('vert_vel_corr          ='
     &                   ,f8.3)")0.0
              write(12,"('heading_corr           ='
     &                   ,f8.3)")0.0
              write(12,"('roll_corr              ='
     &                   ,f8.3)")0.0
              write(12,"('pitch_corr             ='
     &                   ,f8.3)")pitch_corr_cfac
              write(12,"('drift_corr             ='
     &                   ,f8.3)")drift_corr_cfac
              write(12,"('rot_angle_corr         ='
     &                   ,f8.3)")rot_angle_corr_fore
              write(12,"('tilt_corr              ='
     &                   ,f8.3)")tilt_corr_fore

              close(12)


c CAI ******  End of writing the cfac files  ******************



//******************************************************************
//**** WRITES THE "SURF_EL*" FILE #30 (if IWRISURFILE=1)
//******************************************************************
c
          if(iwrisurfile == 1)then
            print *,' '
  	    print *,' WRITES THE "SURF_EL_*" FILE #30 :'
     &        ,directory(1:ndir)//'/'//wrisurfile(1:nsf)
            print *,' INTERPOLATION OF THE RADAR-DERIVED SURFACE MAP'
	    call inter(swdzsurf_wri,sw_or_altsurf_wri
     &                 ,nx_wrisurf,ny_wrisurf,nxysurfmax)
	    nwrisurf_ok=0
	    do j_wrisurf=1,ny_wrisurf
	       do i_wrisurf=1,nx_wrisurf
	  	  if(abs(sw_or_altsurf_wri(i_wrisurf,j_wrisurf))
     &                < 10.)then
		    ialtsurf_wri(i_wrisurf)
     &               =(1000.*sw_or_altsurf_wri(i_wrisurf,j_wrisurf))
		    nwrisurf_ok=nwrisurf_ok+1
		  else
	 	    ialtsurf_wri(i_wrisurf)=-9999
		  endif
	       enddo
	       write(30,222)(ialtsurf_wri(i_wrisurf)
     &                       ,i_wrisurf=1,nx_wrisurf)
 222           format(500i6)
	    enddo
	    print *,' -> NPTS WRITTEN ON THE "SURF_EL_*" FILE #30'
     &             ,nwrisurf_ok
	    close(30)
	  endif
c
//******************************************************************
//**** END OF "SIS_EL_*" FILE #50
//******************************************************************
c
          print *,' '
          print *,' END OF "SIS_EL_*" FILE #50 :'
     &          ,directory(1:ndir)//'/'//fich_sis
          write(50)999,999
     &              ,-999.,-999.,-999.,-999.
     &              ,-999.,-999.,-999.
     &              ,-999.,-999.,-999.
          write(50)-999
          write(50)-999
          write(50)-999
          close(50)
c
//******************************************************************
//**** END OF PROGRAMM
//******************************************************************
c
          print *,' '
          print *,' **************************'
          print *,' **** END OF PROGRAMM  ****'
          print *,' **************************'

	  print *,'N1,N2,N3,N4,N5,N6,N7,N8= ',nb1,nb2,nb3,nb4,nb5,nb6
     &                                       ,nb7,nb8
	  print *,'NSUP=', nsup
	  print *,'NTOTAL_OK= ',nbtotals
	  print *,'NBON, NMAUVAIS= ',nbon,nmauvais
c
          go to 3
c
        endif    ////  of  //// if(iend == 2)then  ////
c
//******************************************************************
//**** INITIALIZATIONS AT THE BEGINNING OF A SWEEP (if IEND=1)
//******************************************************************
c
        istart_sweep(iradar_ray)=0;
        xsweeps(iradar_ray)=xsweeps(iradar_ray)+1.;
        nb_ray(iradar_ray)=0;
        stilt(iradar_ray)=0.;
	stilt2(iradar_ray)=0.;
	rota_prev(iradar_ray)=-999.;
	rota_start(iradar_ray)=-999.;
	rota_end(iradar_ray)=-999.;
	sxa(iradar_ray)=0.;
	sya(iradar_ray)=0.;
	sza(iradar_ray)=0.;
	sacfthspd(iradar_ray)=0.;
	stime(iradar_ray)=0.;
	ssc(iradar_ray)=0.;
	scc(iradar_ray)=0.;
	xp_acft(iradar_ray)=0.;
	su_acft(iradar_ray)=0.;
	sv_acft(iradar_ray)=0.;
	sw_acft(iradar_ray)=0.;
	xp_wind(iradar_ray)=0.;
	su_wind(iradar_ray)=0.;
	sv_wind(iradar_ray)=0.;
	sw_wind(iradar_ray)=0.;
        n_dvinsitu(iradar_ray)=0;
        n_dzsurf(iradar_ray)=0;
        n_vsurf(iradar_ray)=0;
        ndismiss_vhacft(iradar_ray)=0;
        ndismiss_vdopcorr(iradar_ray)=0;
        ndismiss_vdopsurf(iradar_ray)=0;
c
        do n=1,500
           zs_rot(iradar_ray,n)=0.;
           zs_el(iradar_ray,n)=0.;
           zs_az(iradar_ray,n)=0.;
           zs_dsurf(iradar_ray,n)=0.;
           zs_dhor(iradar_ray,n)=0.;
           zs_zsurf(iradar_ray,n)=0.;
           zs_hsurf(iradar_ray,n)=0.;
           vs_dhor(iradar_ray,n)=0.;
           vs_vdopsurf(iradar_ray,n)=0.;
           vi_dhor(iradar_ray,n)=0.;
           vi_vdop(iradar_ray,n)=0.;
           vi_vinsitu(iradar_ray,n)=0.;
        enddo
c
        swdzsurf_sweep(iradar_ray)=0.;
        dzsurfsweep_mean(iradar_ray)=0.;
        dzsurfsweep_rms(iradar_ray)=0.;
        swvsurf_sweep(iradar_ray)=0.;
        vsurfsweep_mean(iradar_ray)=0.;
        vsurfsweep_rms(iradar_ray)=0.;
        nsurf_wri(iradar_ray)=0;
        swinsitu_sweep(iradar_ray)=0.;
        dvinsitusweep_mean(iradar_ray)=0.;
        dvinsitusweep_rms(iradar_ray)=0.;
	do jgd=1,2
	   s_vpv(iradar_ray,jgd)=0.;
	   sv_vpv(iradar_ray,jgd)=0.;
	   svv_vpv(iradar_ray,jgd)=0.;
	enddo
c
      endif  ////  of  //// if(iend.ge.1)then
c
//************************************************************************
//**** NEW RAY
//************************************************************************
c
      nb_ray(iradar_ray)=nb_ray(iradar_ray)+1;
c
//******************************************************************
//**** FRENCH->ENGLISH TRANSLATIONS
//**** CONSTANT CORRECTIONS READ ON THE TAPE
//******************************************************************
c
      azeast_ray=azest_rdl+corr_azest(iradar_ray)	// Mod Oliv
      elhor_ray=elhor_rdl+corr_elhor(iradar_ray)	//
      xlon_acft=lon_av+corr_lon(iradar_ray)		//
      xlat_acft=lat_av+corr_lat(iradar_ray)		//
      p_alt_acft=p_alt_av+corr_p_alt(iradar_ray)	//
      r_alt_acft=r_alt_av+corr_r_alt(iradar_ray)	//
      roll_acft=roul_av+corr_roul(iradar_ray)		//
      pitch_acft=tang_av+corr_tang(iradar_ray)		//
      hdg_acft=cap_av+corr_cap(iradar_ray)		//
      drift_acft=derv_av+corr_derv(iradar_ray)		//
      rota_ray=rota_rdl+corr_rota(iradar_ray)		//
      tilt_ray=incl_rdl+corr_incl(iradar_ray)		//
      wind_we=vent_we;
      wind_sn=vent_sn;
      wind_nz=vent_nz;
      acftspd_we=vwe_av;
      acftspd_sn=vsn_av;
      acftspd_nz=vnz_av;
c	print *,'IRADAR_RAY= ',iradar_ray
c	print *,'AZ,EL,Xlon,Xlat,Palt,Roll,Pitch,Hdg,Drift,Rota
c     &         ,Tilt= ',corr_azest(iradar_ray), corr_elhor(iradar_ray)
c     &         , corr_lon(iradar_ray),corr_lat(iradar_ray)
c     &         ,corr_p_alt(iradar_ray),corr_roul(iradar_ray)
c     &         ,corr_tang(iradar_ray),corr_cap(iradar_ray)
c     &         ,corr_derv(iradar_ray),corr_rota(iradar_ray)
c     &         , corr_incl(iradar_ray)
c
//******************************************************************
//**** EARTH-RELATIVE ANGLES AND
//**** PARAMETERS FOR THE ANALYSIS
//******************************************************************
c
      if(iaftfore == -1)then
        dtilt_guess=dtiltaft_guess;
        drota_guess=drotaaft_guess;
      else
        dtilt_guess=dtiltfore_guess;
        drota_guess=drotafore_guess;
      endif
c------------------------------------------------------------------
c---- ( IF ISIM=1 ) -> SIMULATED TRUE NAVIGATION WITHOUT dXXX_GUESS
c------------------------------------------------------------------
      if(isim == 1)then
c
        call azel( rota_ray+roll_acft
     &            ,tilt_ray
     &            ,hdg_acft,drift_acft
     &            ,pitch_acft
     &            ,azeast_ray_true,elhor_ray_true
     &            ,cxa_true,cya_true,cza_true
     &            ,cwe_true,csn_true,cnz_true)
        caze_true=caze;
	saze_true=saze;
	celh_true=celh;
	selh_true=selh;

c
        dcwe_dt_true=+crr*sti*spit*shdg-srr*sti*chdg+cti*cpit*shdg;
        dcwe_dr_true=+srr*cti*spit*shdg+crr*cti*chdg;
        dcwe_dp_true=-crr*cti*cpit*shdg-sti*spit*shdg;
        dcwe_dh_true=-crr*cti*spit*chdg-srr*cti*shdg+sti*cpit*chdg;
c
        dcsn_dt_true=+crr*sti*spit*chdg+srr*sti*shdg+cti*cpit*chdg;
        dcsn_dr_true=+srr*cti*spit*chdg-crr*cti*shdg;
        dcsn_dp_true=-crr*cti*cpit*chdg-sti*spit*chdg;
        dcsn_dh_true=+crr*cti*spit*shdg-srr*cti*chdg-sti*cpit*shdg;
c
        dcnz_dt_true=-crr*sti*cpit+cti*spit;
        dcnz_dr_true=-srr*cti*cpit;
        dcnz_dp_true=-crr*cti*spit+sti*cpit;
        dcnz_dh_true=0.;
c
        duacft_dv_true=+shdg*cdri+chdg*sdri;
        dvacft_dv_true=+chdg*cdri-shdg*sdri;
c
      endif
c------------------------------------------------------------------
      call azel( rota_ray+drota_guess+roll_acft
     &          ,tilt_ray+dtilt_guess
     &          ,hdg_acft+dhdg_guess,drift_acft
     &          ,pitch_acft+dpitch_guess
     &          ,azeast_ray,elhor_ray
     &          ,cxa,cya,cza,cwe,csn,cnz)
      if(sin(conv*(rota_ray+drota_guess+roll_acft)) < 0.)then
        side=-1.;
        ilr=1;
      else
        side=+1.;
        ilr=2;
      endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      if(      iradar_ray == 1
c////////     &     && nb_ray(iradar_ray)
c////////     &          == 10*(nb_ray(iradar_ray)/10) )then
c////////        print *,' '
c////////        print *,'NO_RAY:',nb_ray(iradar_ray)
c////////     &         ,' ROTA_RAY:',rota_ray+roll_acft
c////////     &         ,' EL_RAY:',elhor_ray
c////////      endif
c////////        print *,'IRADAR_RAY:',iradar_ray
c////////     &         ,' NO_RAY:',nb_ray(iradar_ray)
c////////      endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
      dcwe_dt=+crr*sti*spit*shdg-srr*sti*chdg+cti*cpit*shdg;
      dcwe_dr=+srr*cti*spit*shdg+crr*cti*chdg;
      dcwe_dp=-crr*cti*cpit*shdg-sti*spit*shdg;
      dcwe_dh=-crr*cti*spit*chdg-srr*cti*shdg+sti*cpit*chdg;
c
      dcsn_dt=+crr*sti*spit*chdg+srr*sti*shdg+cti*cpit*chdg;
      dcsn_dr=+srr*cti*spit*chdg-crr*cti*shdg;
      dcsn_dp=-crr*cti*cpit*chdg-sti*spit*chdg;
      dcsn_dh=+crr*cti*spit*shdg-srr*cti*chdg-sti*cpit*shdg;
c
      dcnz_dt=-crr*sti*cpit+cti*spit;
      dcnz_dr=-srr*cti*cpit;
      dcnz_dp=-crr*cti*spit+sti*cpit;
      dcnz_dh=0.;
c
      duacft_dv=+shdg*cdri+chdg*sdri;
      dvacft_dv=+chdg*cdri-shdg*sdri;
c
//******************************************************************
//**** DISTANCE OF THE RANGE GATES
//******************************************************************
c
      if(iaftfore == -1)then
        d_dgate_guess=rdaft_guess;
      else
        d_dgate_guess=rdfore_guess;
      endif
      ngates=nb_portes;
c------------------------------------------------------------------
c---- ( IF ISIM=1 ) -> SIMULATED TRUE RANGE GATES WITHOUT dXXX_GUESS
c------------------------------------------------------------------
      if(isim == 1)then
        do ig=1,ngates;
c CAI-START
c          dgate_true(ig)=d_porte(iradar*MAXPORT+ig)
c    &                        +corr_dist(iradar+1);
c It seems that the above code if wrong, since iradar has not been assigned values yet,
c therefore, following are the new code:
           dgate_true(ig)=d_porte((iradar_ray-1)*MAXPORT+ig)
     &                        +corr_dist(iradar_ray);
        enddo
      endif
c------------------------------------------------------------------
      do ig=1,ngates
         dgate_corr(ig)=d_porte((iradar_ray-1)*MAXPORT+ig)   // Mod Oliv
     &                  +corr_dist(iradar_ray)+d_dgate_guess;
      enddo
      ddg=dgate_corr(2)-dgate_corr(1);
c
//******************************************************************
//**** AIRCRAFT POSITION, (PRESSURE OR RADAR) ALTITUDE AND HEADING
//******************************************************************
c
      ylat=(xlat_acft+orig_lat)/2.;
      deg_lon=deg_lon0*cos(conv*ylat);
c------------------------------------------------------------------
c---- ( IF ISIM=1 ) -> SIMULATED TRUE AIRCRAFT POSITION WITHOUT dXXX_GUESS
c------------------------------------------------------------------
      if(isim == 1)then
        x_acft_true=(xlon_acft-orig_lon)*deg_lon;
        y_acft_true=(xlat_acft-orig_lat)*deg_lat;
        if(ipr_alt == 1)then
	  z_acft_true=p_alt_acft;
        else
	  z_acft_true=r_alt_acft;
        endif
      endif
c------------------------------------------------------------------
      x_acft=(xlon_acft-orig_lon)*deg_lon+dxwe_guess;
      y_acft=(xlat_acft-orig_lat)*deg_lat+dysn_guess;
      if(ipr_alt == 1)then
	z_acft=p_alt_acft+dzacft_guess;
      else
	z_acft=r_alt_acft+dzacft_guess;
      endif

c	print *,'Z_ACFT,P_ALT,D_GUESS= ',z_acft,p_alt_acft,dzacft_guess
c
//******************************************************************
//**** ADD TO THE MEAN PARAMETERS FOR THE CURRENT SWEEP
//******************************************************************
c
      stilt(iradar_ray)=stilt(iradar_ray)+tilt_ray;
      stilt2(iradar_ray)=stilt2(iradar_ray)+tilt_ray*tilt_ray;
      if(nb_ray(iradar_ray) == 1)rota_start(iradar_ray)=rota_ray;
      sxa(iradar_ray)=sxa(iradar_ray)+x_acft;;
      sya(iradar_ray)=sya(iradar_ray)+y_acft;
      sza(iradar_ray)=sza(iradar_ray)+z_acft;
      stime(iradar_ray)=stime(iradar_ray)+time_ks;
      ssc(iradar_ray)=ssc(iradar_ray)+shdg;
      scc(iradar_ray)=scc(iradar_ray)+chdg;
      dmax=amin1(dmax0,dgate_corr(ngates));
c
//******************************************************************
//**** AIRCRAFT SPEED
//******************************************************************
c
      if(    (abs(acftspd_we) < 10. && abs(acftspd_sn) < 10.)
     &   .or.(abs(acftspd_we) > 200..or.abs(acftspd_sn) > 200.) )then
        print *,' //////// NO_RAY:',nb_ray
     &         ,' -> U,V,W_acft:',acftspd_we,acftspd_sn,acftspd_nz
     &         ,' ////////'
        go to 1
      endif
c----------------------------------------------------------------------
c---- ( IF ISIM=1 ) -> SIMULATED TRUE AIRCRAFT SPEED WITHOUT dXXX_GUESS
c----------------------------------------------------------------------
      if(isim == 1)then
        acftspd_we_true=acftspd_we;
        acftspd_sn_true=acftspd_sn;
      endif
c----------------------------------------------------------------------
      acftspd_we=acftspd_we+duacft_dv*dvh_guess;
      acftspd_sn=acftspd_sn+dvacft_dv*dvh_guess;
      acftspd_hor=sqrt(acftspd_we*acftspd_we+acftspd_sn*acftspd_sn);
      sacfthspd(iradar_ray)=sacfthspd(iradar_ray)+acftspd_hor;
      xp_acft(iradar_ray)=xp_acft(iradar_ray)+1.;
      su_acft(iradar_ray)=su_acft(iradar_ray)+acftspd_we;
      sv_acft(iradar_ray)=sv_acft(iradar_ray)+acftspd_sn;
      sw_acft(iradar_ray)=sw_acft(iradar_ray)+acftspd_nz;
      proj_acftspd=acftspd_we*cwe+acftspd_sn*csn+acftspd_nz*cnz;
c
//******************************************************************
//**** FLIGHT-LEVEL WIND
//******************************************************************
c
c----------------------------------------------------------------------
c---- ( IF ISIM=1 ) -> SIMULATED TRUE AIRCRAFT SPEED WITHOUT dXXX_GUESS
c----------------------------------------------------------------------
      if(isim == 1)then
        proj_wind=wind_we*cwe_true+wind_sn*csn_true+wind_nz*cnz_true
        wa_we_true=wind_we-acftspd_we_true
        wa_sn_true=wind_sn-acftspd_sn_true
      endif
c----------------------------------------------------------------------
      if(    (abs(wind_we).le.0. && abs(wind_sn).le.0.)
     &   .or.(abs(wind_we) > 100..or.abs(wind_sn) > 100.))then
        print *,' //////// NO_RAY:',nb_ray(iradar_ray),' -> Uwe,Vsn_wind:'
     &         ,wind_we,wind_sn,' ////////'
        go to 1
      endif
      if(abs(wind_nz).le.0..or.abs(wind_nz) > 50.)wind_nz=0.
      xp_wind(iradar_ray)=xp_wind(iradar_ray)+1.;
      su_wind(iradar_ray)=su_wind(iradar_ray)+wind_we;
      sv_wind(iradar_ray)=sv_wind(iradar_ray)+wind_sn;
      sw_wind(iradar_ray)=sw_wind(iradar_ray)+wind_nz;
      proj_wind=wind_we*cwe+wind_sn*csn+wind_nz*cnz;
      wa_we=wind_we-acftspd_we;
      wa_sn=wind_sn-acftspd_sn;
      wa_nz=wind_nz-acftspd_nz;
c
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c//////// ELIMINATION OF GATES CONTAMINATED WITH GROUND-CLUTTER
c//////// ONLY FOR TOGA-COARE DATA ////////
c////////  -> aft_SWEEP : dROTA=+6 deg
c////////  -> fore_SWEEP : dROTA=+3 deg)
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////
c////////     if(tilt_ray < -15.)then
c////////       rota_sidelobe=rota_ray+roll_acft+6.
c////////     elseif(tilt_ray > 15.)then
c////////       rota_sidelobe=rota_ray+roll_acft+3.
c////////     endif
c////////      if(a_don.le.1993 && cos(conv*rota_sidelobe) < 0.)then
c////////       dmax_sidelobe=(z_min-z_acft)/cos(conv*rota_sidelobe)
c////////       dmax=amin1(dmax0,dmax_sidelobe)
c////////     else
c////////       dmax=dmax0
c////////     endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** DISMISS THE SPECIFIED RANGE GATES
//******************************************************************
c
      do iig=1,15
         if(ig_dismiss(iig) > 0)then
           ig=ig_dismiss(iig);
           ze(ig)=-999.;
           vr(ig)=-999.;
	   vs(ig)=-999.	//Olivier
	   vl(ig)=-999.	//Olivier
           vg(ig)=-999.;
           vu(ig)=-999.;
         endif
      enddo
c
//******************************************************************
//**** RANGE GATES FOR COMPARISONS WITH FLIGHT-LEVEL (IN SITU) DATA
//******************************************************************
c
      ngates_insitu_max=-999;
      if(abs(selh) < selhinsitu_max)then
        ig=1;
        do while (     ig < MAXPORT
     &             && dgate_corr(ig) < dmax_insitu)
           ngates_insitu_max=ig;
           ig=ig+1;
        enddo
      endif
c
//******************************************************************
//**** CHECK THE NCP, SW AND REFLECTIVITY VALUES
//******************************************************************
c
      ngates_max=1;
      do ig=1,ngates
         ref_min=ref_min0+20.*(alog10(dgate_corr(ig))-1.)
         if(    dgate_corr(ig) < dmin
     &      .or.dgate_corr(ig) > dmax
     &      .or.ncp(ig) < xncp_min
     &      .or.sw(ig) > sw_max
     &      .or.ze(ig) < ref_min
     &      .or.ze(ig) > ref_max)then
           ze(ig)=-999.;
           vr(ig)=-999.;
	   vs(ig)=-999.	//Olivier
	   vl(ig)=-999.	//Olivier
           vg(ig)=-999.;
           vu(ig)=-999.;
         else
	   ngates_max=ig;
           nref_ok(iradar_ray)=nref_ok(iradar_ray)+1;
         endif
      enddo
  10  continue
c
//******************************************************************
//**** CHOOSE WHICH DOPPLER VELOCITY WILL BE USED (FOLLOWING ICHOICE_VDOP)
//****   -> 1:RAW(VR), 2:CORRECTED FOR VACFT(VG), 3:UNFOLDED(VU)
//******************************************************************

      do ig=1,ngates_max

	 vdop_read=-999.
	 vdop_corr(ig)=-999.
	 if(ze(ig) > -900.)then
c CAI-START: get rid of all Oliver's modification, since it is for P3
c          if(ichoice_vdop == 1)then		// Olivier
c            if(     abs(vr(ig)) > 0.
c    &           && abs(vr(ig)) < vdop_max
c    &           && abs(vs(ig)) > 0.
c    &           && abs(vs(ig)) < vdop_max
c    &           && abs(vl(ig)) > 0.
c    &           && abs(vl(ig)) < vdop_max
c    &           && proj_acftspd > -900.   )then
c
c               d_vs_vl=vs(ig)-vl(ig)		// Olivier
c               kvsl=ifix((d_vs_vl/vnyq_el)*0.5)+5	// Olivier

c               if(kvsl.ge.1 && kvsl.le.9)then		// Olivier
c                 vs_depl=vs(ig)+xms(kvsl)*vnyq_el;
c                 vl_depl=vl(ig)+xml(kvsl)*vnyq_el;
c                 vsl_depl=(vs_depl+vl_depl)/2.		// Olivier

c               if(    abs(vs_depl-vl_depl) < vnyq_el/2.
c    &		     && abs(vr(ig)-vsl_depl) < vnyq_el/2.     )then
c          	    vdop_read=vr(ig)+proj_acftspd;
c                   nbon=nbon+1;
c               else
c                   nmauvais=nmauvais+1;
c               endif

c                  nbtotals=nbtotals+1;

c              endif

c             endif					// Olivier
c            endif					// Olivier

c	      if(     abs(vr(ig)) > 0.
c    &		  && abs(vr(ig)) < vdop_max
c    &	          && proj_acftspd > -900.     )then	// Olivier
c		 vdop_read=vr(ig)+proj_acftspd		// Olivier
c	      endif					// Olivier
c	    endif					// Olivier

            if(     ichoice_vdop == 1
     &         && abs(vr(ig)) > 0. && abs(vr(ig)) < vdop_max
     &         && proj_acftspd > -900.)
     &         vdop_read=vr(ig)+proj_acftspd;
c CAI-STOP


           if(     ichoice_vdop == 2
     &         && abs(vr(ig)) > 0. && abs(vr(ig)) < vdop_max)
     &       vdop_read=vr(ig);

           if(     ichoice_vdop == 3
     &         && abs(vu(ig)) > 0. && abs(vu(ig)) < vdop_max)
     &       vdop_read=vu(ig);

           if(vdop_read > -900.)then
             ndop_ok(iradar_ray)=ndop_ok(iradar_ray)+1;
             vdop_corr(ig)=vdop_read;
           endif

         endif
      enddo

//******************************************************************
//**** ( if     ( KZsurf=1  or  KVsurf=1 )
//****      and  Z_ACFT > Z_ACFTmin
//****      and SIN(ELEV_HOR) < SELH_SURF
//****      and  VFF_AV>0 )
//****  -> DETERMINE ALTITUDE (THEN DOPPLER VELOCITY)
//****     OF THE SURFACE FOR THIS RAY
//******************************************************************
c
      if(     (kdzsurf+kvsurf).ge.1
     &    && selh < selh_surf
     &    && z_acft > zacftmin_surf)then
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      if( nb_ray(iradar_ray) == 10*(nb_ray(iradar_ray)/10) )then
        print *,' '
        print *,' ',1000*ihhmmss+ims_ray
     &         ,' IRADAR:',iradar_ray
     &         ,' NO_RAY:',nb_ray(iradar_ray)
        print *,'    ROTA,TILT_RAY:',rota_ray,tilt_ray
        print *,'    ROLL,PITCH,HDG,DRIFT_ACFT:',roll_acft
     &         ,pitch_acft,hdg_acft,drift_acft
        print *,'    AZ_EAST:',azeast_ray,' EL_HOR:',elhor_ray
        print *,'    CWE,CSN,CNZ:',cwe,csn,cnz
        print *,'    U,V,W_ACFT:',acftspd_we,acftspd_sn,acftspd_nz
     &         ,' PROJ_VACFT:',proj_acftspd
c////////        print *,'    U,V,W_WIND:',wind_we,wind_sn,wind_nz
c////////     &         ,'    PROJ_WIND:',proj_wind
      endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
        dsurf_ray=-999.;
        dhorsurf_ray=-999.;
        hsurf_ray=-999.;
        refsurf_ray=0.;
        refsurf_min=refsurf_min0*((abs(selh))**0.7);
        gradrefsurf_ray=0.;
        gradrefsurf_min=gradrefsurf_min0*((abs(selh))**0.7);
        wghtsurf_ray=0.;
c
        refmax_ray=-999.;
        ig_refmax=999		// Olivier (real->entier)
        d_refmax=-999.;
        h_refmax=-999.;
        z_refmax=-999.;
        gradrefmax_ray=-999.;
        ig_gradrefmax=999	// Olivier (real->entier)
        d_gradrefmax=-999.;
        h_gradrefmax=-999.;
        z_gradrefmax=-999.;
c
//******************************************************************
//**** DETERMINE REFmax AND dREF/dD|max
//******************************************************************
c
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      if( nb_ray(iradar_ray) == 10*(nb_ray(iradar_ray)/10) )then
c////////        print *,'    -> REFSURF,GRADREFSURF_min:'
c////////     &         ,refsurf_min,gradrefsurf_min
c////////      endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        distmax=(z_acft-altdtm_min+1.)/abs(selh);

	dhor_prevgate=0.	// Mod Oliv
	z_prevgate=0.		// Mod Oliv

	do ig=igstart_surf,ngates_max
	   if(dgate_corr(ig).le.distmax)then
	     d_ig=dgate_corr(ig);
	     dver_ig=d_ig*selh;
	     dhor_ig=d_ig*celh;
	     frac1=2.*(z_acft+dver_ig)/rayter;
	     frac2=(z_acft*z_acft+d_ig*d_ig
     &              +2.*z_acft*dver_ig)/(rayter*rayter)
             z_ig=rayter*(sqrt(1.+frac1+frac2)-1.);
             theta=atan(dhor_ig/(rayter+z_acft+dver_ig));
             dhor_ig=rayter*theta;
	     if(ze(ig) > -900.)then
               if(ze(ig) > refmax_ray)then
                 refmax_ray=ze(ig);
                 ig_refmax=ig;
                 d_refmax=d_ig;
                 dhor_refmax=dhor_ig;
                 z_refmax=z_ig;
               endif
               if(ig > 1 && ze(ig-1) > -900.)then
                 gradref=(ze(ig)-ze(ig-1))
     &                   /(d_ig-dgate_corr(ig-1));
                 if(gradref > gradrefmax_ray)then
                   gradrefmax_ray=gradref;
                   ig_gradrefmax=ig;
                   d_gradrefmax=(d_ig+dgate_corr(ig-1))/2.;
                   dhor_gradrefmax=(dhor_prevgate+dhor_ig)/2.;
                   z_gradrefmax=(z_prevgate+z_ig)/2.;
                 endif
               endif
c
             endif
             z_prevgate=z_ig;
             dhor_prevgate=dhor_ig;
           endif
        enddo
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      if( nb_ray(iradar_ray) == 10*(nb_ray(iradar_ray)/10) )then
c////////        print *,'     REF(1->IGmax+5) :'
c////////     &         ,(int(ze(ig)),ig=1,ig_refmax+5)
c////////         print *,'    -> IG_RefMAX:',ig_refmax,' -> REF: max,d,z:'
c////////     &         ,refmax_ray,d_refmax,z_refmax
c////////         print *,'    -> IG_GradMAX:',ig_gradrefmax,' -> GRAD: max,d,z:'
c////////     &          ,gradrefmax_ray,d_gradrefmax,z_gradrefmax
c////////      endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** WEIGHT ASSOCIATED WITH THE OBTAINED SURFACE POINT
//******************************************************************
c
        if(     refmax_ray > refsurf_min
     &      && gradrefmax_ray > gradrefsurf_min)then
          if(     (d_refmax > d_gradrefmax)
     &        && abs(z_refmax-z_gradrefmax) < 1.)then
            wght_ref=1.+(refmax_ray-refsurf_min)/refsurf_min;
            wght_grad=1.+(gradrefmax_ray-gradrefsurf_min)
     &                  /(gradrefsurf_min);
            wghtsurf_ray=sqrt(wght_ref*wght_grad);
            dsurf_ray=d_refmax;
            hsurf_ray=z_refmax;
            dhorsurf_ray=dhor_refmax;
            xsurf_ray=x_acft+dhorsurf_ray*caze;
            ysurf_ray=y_acft+dhorsurf_ray*saze;
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      if( nb_ray(iradar_ray) == 10*(nb_ray(iradar_ray)/10) )then
c////////        print *,'     -> SURF: d,dhor,z:'
c////////     &         ,dsurf_ray,dhorsurf_ray,hsurf_ray
c////////        print *,'     -> X,Y,H_SURF:',xsurf_ray,ysurf_ray,hsurf_ray
c////////        print *,'        WGHTSURF_ray :',wghtsurf_ray
c////////      endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////          else
c////////      if( nb_ray(iradar_ray) == 10*(nb_ray(iradar_ray)/10) )
c////////     &  print *,'    //////// VALUES OK, BUT PB ON d_REF AND/OR d_GRAD //////'
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////        else
c////////      if( nb_ray(iradar_ray) == 10*(nb_ray(iradar_ray)/10) )
c////////     &  print *,'    //////// PB ON REF AND/OR GRAD VALUES //////'
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        endif
c
c--------------------------------------------------------------
c---- ( IF ISIM=1 ) -> (XYH)SURF_RAY FROM dXXX_GUESS & DTM
c--------------------------------------------------------------
c
        if(isim == 1)then
c
c---- *_app -> with dXXX_GUESS ("apparent" navigation)
c---- *_true -> without ("true" navigation)
c
	  hsurf_ray=-999.;
          wghtsurf_ray=0.;
	  igsurf_ray=-999;
	  do ig=igstart_surf,ngates_max
	     if(dgate_corr(ig).le.distmax)then
c
	       d_app=dgate_corr(ig);
  	       dver_app=d_app*selh;
	       dhor_app=d_app*celh;
	       frac1=2.*(z_acft+dver_app)/rayter;
	       frac2=(z_acft*z_acft+d_app*d_app
     &                +2.*z_acft*dver_app)/(rayter*rayter);
               z_app=rayter*(sqrt(1.+frac1+frac2)-1.);
               theta=atan(dhor_app/(rayter+z_acft+dver_app));
               dhor_app=rayter*theta;
               x_app=x_acft+dhor_app*caze;
               y_app=y_acft+dhor_app*saze;
c
	       d_true=dgate_true(ig);
  	       dver_true=d_true*selh_true;
	       dhor_true=d_true*celh_true;
	       frac1=2.*(z_acft_true+dver_true)/rayter;
	       frac2=(z_acft_true*z_acft_true+d_true*d_true
     &                +2.*z_acft_true*dver_true)/(rayter*rayter)
               z_true=rayter*(sqrt(1.+frac1+frac2)-1.);
               theta=atan(dhor_true/(rayter+z_acft_true+dver_true));
               dhor_true=rayter*theta;
               x_true=x_acft_true+dhor_true*caze_true;
               y_true=y_acft_true+dhor_true*saze_true;
c
	       if(     igsurf_ray == -999
     &             && x_true > xmin_dtm
     &             && x_true < xmax_dtm
     &             && y_true > ymin_dtm
     &             && y_true < ymax_dtm)then
                 isurf_true=(x_true-xmin_dtm)/hx_dtm+1
	         jsurf_true=(y_true-ymin_dtm)/hy_dtm+1
	         aa=alt_dtm(isurf_true,jsurf_true)
	         bb=(-alt_dtm(isurf_true,jsurf_true)
     &               +alt_dtm(isurf_true+1,jsurf_true))/hx_dtm
	         cc=(-alt_dtm(isurf_true,jsurf_true)
     &               +alt_dtm(isurf_true,jsurf_true+1))/hy_dtm
                 dd=(+alt_dtm(isurf_true,jsurf_true)
     &               -alt_dtm(isurf_true+1,jsurf_true)
     &               -alt_dtm(isurf_true,jsurf_true+1)
     &               +alt_dtm(isurf_true+1,jsurf_true+1))
     &               /(hx_dtm*hy_dtm);
	         x_dtm=xmin_dtm+float(isurf_true-1)*hx_dtm;
	         dx=x_true-x_dtm;
	         y_dtm=ymin_dtm+float(jsurf_true-1)*hy_dtm;
	         dy=y_true-y_dtm;
                 hsurf_dtm=aa+bb*dx+cc*dy+dd*dx*dy;
c
		 if(hsurf_dtm.ge.z_true)then
c
		   xsurf_true=x_true;
		   ysurf_true=y_true;
		   hsurf_true=z_true;
	           dxh_dtm=bb+dd*dy;
	           dyh_dtm=cc+dd*dx;
c
		   igsurf_ray=ig;
		   dsurf_ray=d_app;
		   xsurf_ray=x_app;
		   ysurf_ray=y_app;
		   hsurf_ray=z_app;
		   wghtsurf_ray=1.;
c
		 endif
c
	       endif
	     endif
	  enddo
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      if( nb_ray(iradar_ray) == 10*(nb_ray(iradar_ray)/10) )then
        print *,'     -> X,Y,H_SURF-TRUE:'
     &         ,xsurf_true,ysurf_true,hsurf_true
        print *,'        I,J_SURF_true :',isurf_true,jsurf_true
     &         ,' dxH,dyH :',dxh_dtm,dyh_dtm
        print *,'     -> X,Y,H_SURF-RAY:'
     &         ,xsurf_ray,ysurf_ray,hsurf_ray
      endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
	endif
c--------------------------------------------------------------
c
//******************************************************************
//**** IF THIS SURFACE POINT IS CORRECT (if WGHTSURF_ray > 0)
//**** THEN COMPARE WITH THE SURFACE POINT DERIVED FROM THE DTM
//******************************************************************
c
        if(hsurf_ray > -900. && wghtsurf_ray > 0.)then
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      if( nb_ray(iradar_ray) == 10*(nb_ray(iradar_ray)/10) )then
c////////      print *,' '
c////////      print *,' ',1000*ihhmmss+ims_ray
c////////     &       ,' NO_RADAR:',iradar_ray,' FORE/AFT:',iaftfore
c////////     &       ,' NO_RAY:',nb_ray(iradar_ray)
c////////      print *,'    ROTA_RAY:',rota_ray+roll_acft
c////////     &       ,' EL_HOR:',elhor_ray,' AZ_EAST:',azeast_ray
c////////       print *,'    DISTMAX:',distmax
c////////       print *,'    IG_RefMAX:',ig_refmax,' -> REF: max,d,z:'
c////////     &       ,refmax_ray,d_refmax,z_refmax
c////////       print *,'    IG_GradMAX:',ig_gradrefmax,' -> GRAD: max,d,z:'
c////////     &        ,gradrefmax_ray,d_gradrefmax,z_gradrefmax
        print *,'    -> X,Y,H_SURF-RAY:',xsurf_ray,ysurf_ray,hsurf_ray
      endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** INTERPOLATION OF ALT_DTM(x,y) [READ ON SURF_DTM_* OR CONSTANT]
//******************************************************************
c
	  if(     xsurf_ray > xmin_dtm
     &        && xsurf_ray < xmax_dtm
     &        && ysurf_ray > ymin_dtm
     &        && ysurf_ray < ymax_dtm)then
            isurf_ray=(xsurf_ray-xmin_dtm)/hx_dtm+1;
	    jsurf_ray=(ysurf_ray-ymin_dtm)/hy_dtm+1;
	    aa=alt_dtm(isurf_ray,jsurf_ray);
	    bb=(-alt_dtm(isurf_ray,jsurf_ray);
     &          +alt_dtm(isurf_ray+1,jsurf_ray))/hx_dtm
	    cc=(-alt_dtm(isurf_ray,jsurf_ray)
     &          +alt_dtm(isurf_ray,jsurf_ray+1))/hy_dtm
            dd=(+alt_dtm(isurf_ray,jsurf_ray)
     &          -alt_dtm(isurf_ray+1,jsurf_ray)
     &          -alt_dtm(isurf_ray,jsurf_ray+1)
     &          +alt_dtm(isurf_ray+1,jsurf_ray+1))
     &          /(hx_dtm*hy_dtm)
	    x_dtm=xmin_dtm+float(isurf_ray-1)*hx_dtm;
	    dx=xsurf_ray-x_dtm;
	    y_dtm=ymin_dtm+float(jsurf_ray-1)*hy_dtm;
	    dy=ysurf_ray-y_dtm;
            hsurf_dtm=aa+bb*dx+cc*dy+dd*dx*dy;
     	    d_hsurf=hsurf_ray-hsurf_dtm;
	    dxh_dtm=bb+dd*dy;
	    dyh_dtm=cc+dd*dx;
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      if( nb_ray(iradar_ray) == 10*(nb_ray(iradar_ray)/10) )then
        print *,'        I,J_SURF_ray :',isurf_ray,jsurf_ray
     &         ,' dxH,dyH :',dxh_dtm,dyh_dtm
        print *,'     -> H_SURF-DTM:',hsurf_dtm
     &         ,'  =>> D_HSURF :',d_hsurf
      endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** IF ( ABS(HSURF_RADAR-HSURF_DTM) < DHSURF_MAX ) THEN
//**** //////// DHSURF_MAX=999. //////// -> NOT IN USE ////////
//******************************************************************
c
            if(abs(d_hsurf) < dhsurf_max)then
              ssurfins=ssurfins+wghtsurf_ray

c
//******************************************************************
//**** CASE "DZ_surf"
//******************************************************************
c
	      if(kdzsurf == 1)then
c
c----------------------------------------------------------------------
c---- ( IF ISIM=1 ) -> SIMULATED DZ_surf FROM dXXX_GUESS
c----------------------------------------------------------------------
          if(isim == 1)then
            d_hsurf_dxxx=-dsurf_ray
     &                    *(-dcnz_dt+dxh_dtm*dcwe_dt+dyh_dtm*dcsn_dt)
     &                     *dtilt_guess*conv
     &                   -dsurf_ray
     &                    *(-dcnz_dr+dxh_dtm*dcwe_dr+dyh_dtm*dcsn_dr)
     &                     *drota_guess*conv
     &                   -dsurf_ray
     &                    *(-dcnz_dp+dxh_dtm*dcwe_dp+dyh_dtm*dcsn_dp)
     &                     *dpitch_guess*conv
     &                   -dsurf_ray
     &                    *(-dcnz_dh+dxh_dtm*dcwe_dh+dyh_dtm*dcsn_dh)
     &                     *dhdg_guess*conv
     &                   -(-cnz+dxh_dtm*cwe+dyh_dtm*csn)
     &                    *d_dgate_guess
     &                   -dxh_dtm*dxwe_guess
     &                   -dyh_dtm*dysn_guess
     &                   +dzacft_guess
c////////            d_hsurf=d_hsurf_dxxx
	  endif
c----------------------------------------------------------------------
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      if( nb_ray(iradar_ray) == 10*(nb_ray(iradar_ray)/10) )then
        print *,'     -> D_HSURF_dXXX :',d_hsurf_dxxx
      endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** ADD WEIGHTS AND DZ_surf
//******************************************************************
c
                n_dzsurf(iradar_ray)=n_dzsurf(iradar_ray)+1
                swdzsurf_sweep(iradar_ray)
     &           =swdzsurf_sweep(iradar_ray)+wghtsurf_ray
c
	        dzsurfsweep_mean(iradar_ray)
     &           =dzsurfsweep_mean(iradar_ray)
     &            +wghtsurf_ray*d_hsurf
	        dzsurfsweep_rms(iradar_ray)
     &           =dzsurfsweep_rms(iradar_ray)
     &            +wghtsurf_ray*d_hsurf*d_hsurf
c
                swdzsurf_tot=swdzsurf_tot+wghtsurf_ray
 	        swdzmsurf_tot=swdzmsurf_tot
     &                       +wghtsurf_ray*d_hsurf
	        swdz2surf_tot=swdz2surf_tot
     &                       +wghtsurf_ray*d_hsurf*d_hsurf
                swadzsurf_tot=swadzsurf_tot
     &                        +wghtsurf_ray*abs(d_hsurf)
c////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      print *,'    -> WGHTSURF_RAY:',wghtsurf_ray,' DZSURF:',d_hsurf
c////////      print *,'       N_DZSURF:',n_dzsurf(iradar_ray)
c////////     &       ,' SWDZ,SDZ,SDZ2:',swdzsurf_sweep(iradar_ray)
c////////     &       ,dzsurfsweep_mean(iradar_ray),dzsurfsweep_rms(iradar_ray)
c////////      print *,'       //////// VR,PROJ_ACFTSPD,VCORR_SURF:'
c////////     &       ,vr(ig_refmax),proj_acftspd,vdop_corr(ig_refmax),' ////////'
c////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** VALUES OF VAR(1->NVAR) FOR FIELD "DZ_surf"
//****  - VAR(1->6) -> [dT_aft,dT_fore,dR_aft,dR_fore,dP,dH] in DEGREES
//****  - VAR(7->11) -> [RD_aft,RD_fore,dXwe,dYsn,dZ] in HECTOMETERS
//****  - VAR(12) -> [dVH] in METER/SECOND
//******************************************************************
c
                if(iaftfore == -1)then
                  if(idtiltaft == 1)then
                    var(1)=dsurf_ray
     &                     *(-dcnz_dt+dxh_dtm*dcwe_dt+dyh_dtm*dcsn_dt)
     &                     *conv
                  else
                    var(1)=0.
                    xmat_dzsurf(1,1)=xmat_dzsurf(1,1)+wghtsurf_ray
                  endif
                  var(2)=0.
                else
                  var(1)=0.;
                  if(idtiltfore == 1)then
                     var(2)=dsurf_ray
     &                      *(-dcnz_dt+dxh_dtm*dcwe_dt+dyh_dtm*dcsn_dt)
     &                      *conv;
                  else
                    var(2)=0.;
                    xmat_dzsurf(2,2)=xmat_dzsurf(2,2)+wghtsurf_ray;
                  endif
                endif
c
                if(iaftfore == -1)then
                  if(idrotaaft == 1)then
                    var(3)=dsurf_ray
     &                     *(-dcnz_dr+dxh_dtm*dcwe_dr+dyh_dtm*dcsn_dr)
     &                     *conv;
                  else
                    var(3)=0.;
                    xmat_dzsurf(3,3)=xmat_dzsurf(3,3)+wghtsurf_ray;
                  endif
                  var(4)=0.
                else
                  var(3)=0.
                  if(idrotafore == 1)then
                    var(4)=dsurf_ray
     &                     *(-dcnz_dr+dxh_dtm*dcwe_dr+dyh_dtm*dcsn_dr)
     &                     *conv;
                  else
                    var(4)=0.;
                    xmat_dzsurf(4,4)=xmat_dzsurf(2,2)+wghtsurf_ray;
                  endif
                endif
c
                if(idpitch == 1)then
                  var(5)=dsurf_ray
     &                   *(-dcnz_dp+dxh_dtm*dcwe_dp+dyh_dtm*dcsn_dp)
     &                   *conv;
                else
                  var(5)=0.;
                  xmat_dzsurf(5,5)=xmat_dzsurf(4,4)+wghtsurf_ray;
                endif
c
                if(idhdg == 1)then
                  var(6)=dsurf_ray
     &                   *(+dxh_dtm*dcwe_dh+dyh_dtm*dcsn_dh)
     &                   *conv;
                else
                  var(6)=0.;
                  xmat_dzsurf(6,6)=xmat_dzsurf(5,5)+wghtsurf_ray;
                endif
c
                if(iaftfore == -1)then
                   if(irdaft == 1)then
                     var(7)=(-cnz+dxh_dtm*cwe+dyh_dtm*csn)
     &                      *0.1;
                   else
                     var(7)=0.;
                     xmat_dzsurf(7,7)=xmat_dzsurf(6,6)+wghtsurf_ray;
                   endif
                   var(8)=0.;
                else
                   var(7)=0.;
                   if(irdfore == 1)then
                     var(8)=(-cnz+dxh_dtm*cwe+dyh_dtm*csn)
     &                      *0.1;
                   else
                     var(8)=0.;
                     xmat_dzsurf(8,8)=xmat_dzsurf(8,8)+wghtsurf_ray;
                   endif
                endif
c
                if(idxwe == 1)then
                  var(9)=dxh_dtm*0.1;
                else
                  var(9)=0.;
                  xmat_dzsurf(9,9)=xmat_dzsurf(9,9)+wghtsurf_ray;
                endif
c
                if(idysn == 1)then
                  var(10)=dyh_dtm*0.1;
                else
                  var(10)=0.;
                  xmat_dzsurf(10,10)=xmat_dzsurf(10,10)+wghtsurf_ray;
                endif
c
                if(idzacft == 1)then
                  var(11)=-0.1;
                else
                  var(11)=0.;
                  xmat_dzsurf(11,11)=xmat_dzsurf(11,11)+wghtsurf_ray;
                endif
c
                var(12)=0.;
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      print *,'    VAR_DZSURF(1->12):',(var(i),i=1,12)
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** ADD TO XMAT_dzsurf(1->NVAR,1->NVAR) AND VECT_dzsurf(1->NVAR)
//******************************************************************
c
                do i=1,nvar
                   do j=1,nvar
 	            xmat_dzsurf(i,j)=xmat_dzsurf(i,j)
     &          	            +wghtsurf_ray*var(i)*var(j);
                   enddo
                   vect_dzsurf(i)=vect_dzsurf(i)
     &                            +wghtsurf_ray*var(i)*d_hsurf;
                enddo
c
//******************************************************************
//**** ADD TO COVARIANCE MATRIX FOR FIELD "DZ_surf"
//******************************************************************
c
                do i=1,nvar
                   rms_var_zsurf(i)=rms_var_zsurf(i)
     &                              +wghtsurf_ray*var(i)*var(i);
                   do j=1,nvar
                      corr_var(i,j)=corr_var(i,j)
     &                              +wghtsurf_ray*var(i)*var(j);
                   enddo
                enddo
c
//******************************************************************
//**** CASE "DZ_surf" ONLY -> D_VH CANNOT BE CALCULATED
//******************************************************************
c
                if(rw_vsurf+rw_dvinsitu.le.0.)then
                  xmat_vsurf(12,12)=xmat_vsurf(12,12)+wghtsurf_ray;
c
//******************************************************************
//**** CASE "FLAT SURFACE" -> D_HEADING,D_XWE,D_YSN CANNOT BE OBTAINED
//******************************************************************
c
                  if(altdtm_min.ge.altdtm_max)then
                    xmat_vsurf(6,6)=xmat_vsurf(6,6)+wghtsurf_ray;
                    xmat_vsurf(9,9)=xmat_vsurf(9,9)+wghtsurf_ray;
                    xmat_vsurf(10,10)=xmat_vsurf(10,10)+wghtsurf_ray;
                  endif
c
                endif
c
//******************************************************************
//**** ARRAYS FOR "SIS_EL_*" FILE #50
//******************************************************************
c
                zs_rot(iradar_ray,n_dzsurf(iradar_ray))=rota_ray;
                zs_el(iradar_ray,n_dzsurf(iradar_ray))=elhor_ray;
                zs_az(iradar_ray,n_dzsurf(iradar_ray))=azeast_ray;
                zs_dsurf(iradar_ray,n_dzsurf(iradar_ray))=dsurf_ray;
                zs_dhor(iradar_ray,n_dzsurf(iradar_ray))
     &                 =side*dsurf_ray*celh;
                zs_zsurf(iradar_ray,n_dzsurf(iradar_ray))=hsurf_ray;
                zs_hsurf(iradar_ray,n_dzsurf(iradar_ray))=hsurf_dtm;
c
	      endif     ////  of //// if(kdzsurf == 1)then
c
//******************************************************************
//**** (if IWRISURFILE=1)
//**** WEIGHTED SUM FOR ALT_SURF(x,y)
//**** TO BE WRITTEN ON "SURF_EL_*" FILE #30
//******************************************************************
c
              if(iwrisurfile == 1)then
c
	        if(     xsurf_ray > xmin_wrisurf-hxy_wrisurf
     &              && xsurf_ray < xmax_wrisurf+hxy_wrisurf
     &              && ysurf_ray > ymin_wrisurf-hxy_wrisurf
     &              && ysurf_ray < ymax_wrisurf+hxy_wrisurf
     &              && hsurf_ray > zsurfrad_min
     &              && hsurf_ray < zsurfrad_max)then
c
 	          nsurf_wri(iradar_ray)=nsurf_wri(iradar_ray)+1;
	          i_wrisurf=(xsurf_ray-xmin_wrisurf)/hxy_wrisurf+1;
	          if(xsurf_ray < xmin_wrisurf)i_wrisurf=i_wrisurf-1;
	          j_wrisurf=(ysurf_ray-ymin_wrisurf)/hxy_wrisurf+1;
	          if(ysurf_ray < ymin_wrisurf)j_wrisurf=j_wrisurf-1;
c
	          do ii=max0(i_wrisurf,1)
     &                  ,min0(i_wrisurf+1,nx_wrisurf)
	             xi=xmin_wrisurf+float(ii-1)*hxy_wrisurf;
	             dx=(xsurf_ray-xi)/hxy_wrisurf;
	             do jj=max0(j_wrisurf,1)
     &                     ,min0(j_wrisurf+1,ny_wrisurf)
		        yj=ymin_wrisurf+float(jj-1)*hxy_wrisurf;
		        dy=(ysurf_ray-yj)/hxy_wrisurf;
		        d2=dx*dx+dy*dy;
		        wghtsurf_wri=wghtsurf_ray*((4.-d2)/(4.+d2));
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      print *,'      II,JJ:',ii,jj,' WGTHSURF_ray,wri:'
c////////     &       ,wghtsurf_ray,wghtsurf_wri
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		        swdzsurf_wri(ii,jj)=swdzsurf_wri(ii,jj)
     &                                     +wghtsurf_wri
	                sw_or_altsurf_wri(ii,jj)
     &                                    =sw_or_altsurf_wri(ii,jj)
     &                                     +wghtsurf_wri*hsurf_ray
 	             enddo
	          enddo
c
	        endif
c
	      endif
c
//******************************************************************
//**** CASE "VDOP_surf"
//******************************************************************
c
  	      if(kvsurf == 1 && acftspd_hor > 0.)then
c
                if(vdop_corr(ig_refmax) > -900..or.isim == 1)then
                  vdopsurf_ray=vdop_corr(ig_refmax)
	if(abs(vdopsurf_ray).le.1.)nb1=nb1+1
	if(abs(vdopsurf_ray).le.2. && abs(vdopsurf_ray) > 1.)nb2=nb2+1
	if(abs(vdopsurf_ray).le.3. && abs(vdopsurf_ray) > 2.)nb3=nb3+1
	if(abs(vdopsurf_ray).le.4. && abs(vdopsurf_ray) > 3.)nb4=nb4+1
	if(abs(vdopsurf_ray).le.5. && abs(vdopsurf_ray) > 4.)nb5=nb5+1
	if(abs(vdopsurf_ray).le.6. && abs(vdopsurf_ray) > 5.)nb6=nb6+1
	if(abs(vdopsurf_ray).le.7. && abs(vdopsurf_ray) > 6.)nb7=nb7+1
	if(abs(vdopsurf_ray).le.8. && abs(vdopsurf_ray) > 7.)nb8=nb8+1
	if(abs(vdopsurf_ray) > 8.)nsup=nsup+1
c
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c      if( nb_ray(iradar_ray) == 10*(nb_ray(iradar_ray)/10) )then
c	if(ig_refmax.ne.-999)then	// Olivier
c
c	v_ctrl=-999.				// Olivier
c
c	d_vs_vl=vs(ig_refmax)-vl(ig_refmax)	// Olivier
c	kvsl=ifix((d_vs_vl/vnyq_el)*0.5)+5	// Olivier
c        print *,'    d_VS_VL :',d_vs_vl,' -> KVSL :',kvsl
c	if(kvsl.ge.1 && kvsl.le.9)then		// Olivier
c	  vs_depl=vs(ig_refmax)+xms(kvsl)*vnyq_el	// Olivier
c	  vl_depl=vl(ig_refmax)+xml(kvsl)*vnyq_el	// Olivier
cc	  vsl_depl=(vs_depl+vl_depl)/2.		// Olivier
c
c	  if(    abs(vs_depl-vl_depl) < vnyq_el/2.
c     &	     && abs(vr(ig_refmax)-vsl_depl) < vnyq_el/2.)then // Oliv
c		print *,'IG_REFMAX= ',ig_refmax
c		print *,'VR= ',vr(ig_refmax)
c		print *,'VS, VL= ', vs(ig_refmax),vl(ig_refmax)
c		print *,'VS_depl,VL_depl= ',vs_depl,vl_depl
c		print *,'VSL_depl= ',vsl_depl
c	          v_ctrl=vr(ig_refmax)			// Olivier
c		print *,'VDOP_CTRL= ',v_ctrl
c
c	      if(proj_acftspd > -900.)then
c		v_corr=v_ctrl+proj_acftspd
c		print *,' VDOP_CORR= ',v_corr
c	      endif
c
c	  endif
c
c	endif
c
c	endif
c      endif

c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
c--------------------------------------------------------------
c---- ( IF ISIM=1 ) -> SIMULATED VDOPSURF_RAY FROM dXXX_GUESS
c--------------------------------------------------------------
c
                  if(isim == 1)then
                    vdopsurf_ray=-(-acftspd_we_true*dcwe_dt_true
     &                             -acftspd_sn_true*dcsn_dt_true
     &                             -acftspd_nz*dcnz_dt_true)
     &                            *dtilt_guess*conv
     &                           -(-acftspd_we_true*dcwe_dr_true
     &                             -acftspd_sn_true*dcsn_dr_true
     &                             -acftspd_nz*dcnz_dr_true)
     &                            *drota_guess*conv
     &                           -(-acftspd_we_true*dcwe_dp_true
     &                             -acftspd_sn_true*dcsn_dp_true
     &                             -acftspd_nz*dcnz_dp_true)
     &                            *dpitch_guess*conv
     &                           -(-acftspd_we_true*dcwe_dh_true
     &                             -acftspd_sn_true*dcsn_dh_true
     &                             -acftspd_nz*dcnz_dh_true)
     &                            *dhdg_guess*conv
     &                           -(-cwe_true*duacft_dv_true
     &                             -csn_true*dvacft_dv_true)*dvh_guess
	          endif
c--------------------------------------------------------------
c
c                  if(abs(vdopsurf_ray) < vdopsurf_max)then

cTEST CAI
                   print *,'vdopsurf_ray =',vdopsurf_ray
cTEST END

		   if(abs(vdopsurf_ray) < 6.)then
c
//******************************************************************
//**** ADD WEIGHTS AND VDOP_surf
//******************************************************************
c
                    n_vsurf(iradar_ray)=n_vsurf(iradar_ray)+1
                    swvsurf_sweep(iradar_ray)
     &               =swvsurf_sweep(iradar_ray)+wghtsurf_ray
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      if( nb_ray(iradar_ray) == 10*(nb_ray(iradar_ray)/10) )then
        print *,'     -> VDOPSURF_RAY :',vdopsurf_ray
c////////        print *,'        SWVSURF_SWEEP(',iradar_ray,') :'
c////////     &         ,swvsurf_sweep(iradar_ray)
      endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
 	            vsurfsweep_mean(iradar_ray)
     &               =vsurfsweep_mean(iradar_ray)
     &                +wghtsurf_ray*vdopsurf_ray
	            vsurfsweep_rms(iradar_ray)
     &               =vsurfsweep_rms(iradar_ray)
     &                +wghtsurf_ray*vdopsurf_ray*vdopsurf_ray;
c
                    swvsurf_tot=swvsurf_tot+wghtsurf_ray
  	            swvmsurf_tot=swvmsurf_tot
     &                           +wghtsurf_ray*vdopsurf_ray
	            swv2surf_tot=swv2surf_tot
     &                           +wghtsurf_ray*vdopsurf_ray*vdopsurf_ray;
                    swavsurf_tot=swavsurf_tot
     &                           +wghtsurf_ray*abs(vdopsurf_ray);
c////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      print *,'    -> WGHTSURF_RAY:',wghtsurf_ray,' VSURF:',vdopsurf_ray
c////////      print *,'        N_VSURF:',n_vsurf(iradar_ray)
c////////     &       ,' SWV,SV,SV2:',swvsurf_sweep(iradar_ray)
c////////     &       ,vsurfsweep_mean(iradar_ray),vsurfsweep_rms(iradar_ray)
c////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** VALUES OF VAR(1->NVAR) FOR FIELD "V_surf"
//****  - VAR(1->6) -> [dT_aft,dT_fore,dR_aft,dR_fore,dP,dH] in DEGREES
//****  - VAR(7->11) -> [RD_aft,RD_fore,dXwe,dYsn,dZ] in HECTOMETERS
//****  - VAR(12) -> [dVH] in METER/SECOND
//******************************************************************
c
                    if(iaftfore == -1)then
                      if(idtiltaft == 1)then
                        var(1)=(-acftspd_we*dcwe_dt-acftspd_sn*dcsn_dt
     &                          -acftspd_nz*dcnz_dt)
     &                         *conv;
                      else
                        var(1)=0.;
                        xmat_vsurf(1,1)=xmat_vsurf(1,1)+wghtsurf_ray;
                      endif
                      var(2)=0.;
                    else
                      var(1)=0.;
                      if(idtiltfore == 1)then
                        var(2)=(-acftspd_we*dcwe_dt-acftspd_sn*dcsn_dt
     &                          -acftspd_nz*dcnz_dt)
     &                         *conv;
                      else
                        var(2)=0.;
                        xmat_vsurf(2,2)=xmat_vsurf(2,2)+wghtsurf_ray;
                      endif
                    endif
c
                    if(iaftfore == -1)then
                      if(idrotaaft == 1)then
                        var(3)=(-acftspd_we*dcwe_dr-acftspd_sn*dcsn_dr
     &                          -acftspd_nz*dcnz_dr)
     &                         *conv;
                      else
                        var(3)=0.;
                        xmat_vsurf(3,3)=xmat_vsurf(3,3)+wghtsurf_ray;
                      endif
                      var(4)=0.;
                    else
                      var(3)=0.;
                      if(idrotafore == 1)then
                        var(4)=(-acftspd_we*dcwe_dr-acftspd_sn*dcsn_dr
     &                          -acftspd_nz*dcnz_dr)
     &                         *conv;
                      else
                        var(4)=0.;
                        xmat_vsurf(4,4)=xmat_vsurf(4,4)+wghtsurf_ray;
                      endif
                    endif
c
                    if(idpitch == 1)then
                      var(5)=(-acftspd_we*dcwe_dp-acftspd_sn*dcsn_dp
     &                        -acftspd_nz*dcnz_dp)
     &                       *conv;
                    else
                      var(5)=0.;
                      xmat_vsurf(5,5)=xmat_vsurf(5,5)+wghtsurf_ray;
                    endif
c
                    if(idhdg == 1)then
                      var(6)=(-acftspd_we*dcwe_dh
     &                        -acftspd_sn*dcsn_dh)
     &                       *conv;
                    else
                      var(6)=0.;
                      xmat_vsurf(6,6)=xmat_vsurf(6,6)+wghtsurf_ray;
                    endif
c
                    var(7)=0.;
                    var(8)=0.;
                    var(9)=0.;
                    var(10)=0.;
                    var(11)=0.;
c
                    if(idvh == 1)then
                      var(12)=-duacft_dv*cwe-dvacft_dv*csn;
                    else
                      var(12)=0.;
                      xmat_vsurf(12,12)=xmat_vsurf(12,12)+wghtsurf_ray;
                    endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      print *,'    VAR_VSURF(1->12):',(var(i),i=1,12)
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** ADD TO XMAT_vsurf(1->NVAR,1->NVAR) AND VECT_vsurf(1->NVAR)
//******************************************************************
c
                    do i=1,nvar
                       do j=1,nvar
    	                  xmat_vsurf(i,j)=xmat_vsurf(i,j)
     &                	                  +wghtsurf_ray*var(i)*var(j)
                       enddo
                       vect_vsurf(i)=vect_vsurf(i)
     &                               +wghtsurf_ray*var(i)*vdopsurf_ray
                    enddo
c
//******************************************************************
//**** ADD TO COVARIANCE MATRIX FOR FIELD "VDOP_surf"
//******************************************************************
c
                    do i=1,nvar
                       rms_var_vsurf(i)=rms_var_vsurf(i)
     &                                  +wghtsurf_ray*var(i)*var(i)
                       do j=1,nvar
                          corr_var(i,j)=corr_var(i,j)
     &                                  +wghtsurf_ray*var(i)*var(j)
                       enddo
                    enddo
c
//******************************************************************
//**** CASE "VDOP_surf" and/or "DVDOP_insitu" ONLY :
//**** -> RGE-DLY_aft,RGE-DLY_aft,D_XWE,D_YSN,D_ZACFT CANNOT BE CALCULATED
//******************************************************************
c
                    if(rw_dzsurf.le.0.)then
                      do ij=7,11
                         xmat_vsurf(ij,ij)=xmat_vsurf(ij,ij)
     &                                     +wghtsurf_ray
                      enddo
                    endif
c
//******************************************************************
//**** ARRAYS FOR "SIS_EL_*" FILE #50
//******************************************************************
c
                    vs_dhor(iradar_ray,n_vsurf(iradar_ray))
     &                     =side*dsurf_ray*celh
                    vs_vdopsurf(iradar_ray,n_vsurf(iradar_ray))
     &                     =vdopsurf_ray
c
	          else  ////  of  //// if(abs(vdopsurf_ray) < vdopsurf_max) ////
	            ndismiss_vdopsurf(iradar_ray)
     &               =ndismiss_vdopsurf(iradar_ray)+1
	          endif  //// of //// if(abs(vdopsurf_ray) < vdopsurf_max) ////
c
	        else  ////  of  ////  if(vdop_corr(ig_refmax) > -900.) ////
	          ndismiss_vdopcorr(iradar_ray)
     &             =ndismiss_vdopcorr(iradar_ray)+1
	        endif  ////  of  ////  if(vdop_corr(ig_refmax) > -900.) ////
c
	      else  ////  of  //// if(kvsurf == 1 && acftspd_hor > 0.)  ////
	        if(acftspd_hor.le.0.)ndismiss_vhacft(iradar_ray)
     &                           =ndismiss_vhacft(iradar_ray)+1
	      endif  ////  of  //// if(kvsurf == 1 && acftspd_hor > 0.)  ////
c
            endif  ////  of  //// if(abs(d_hsurf) < dhsurf_max)  ////
c
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////          else ////  of  ////  if(xsurf_ray > xmin_dtm ... )  ////
c////////            if( nb_ray(iradar_ray) == 10*(nb_ray(iradar_ray)/10) )then
c////////              print *,'     //////// OUT OF DTM LIMITS ////////'
c////////              print *,'     //////// X_ray :',xsurf_ray
c////////     &               ,' XDTM_min,max :',xmin_dtm,xmax_dtm,' ////////'
c////////              print *,'     //////// Y_ray :',ysurf_ray
c////////     &               ,' YDTM_min,max :',ymin_dtm,ymax_dtm,' ////////'
c////////            endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          endif  ////  of  ////  if(xsurf_ray > xmin_dtm ... )  ////
        endif  ////  of  ////  if(hsurf_ray > -999. && wghtsurf_ray > 0.)  ////
      endif  ////  of  ////  if(kdzsurf+kvsurf.ge.1 ... )  ////
c
//******************************************************************
//**** CASE "DVDOP_insitu"
//**** (if D<DMAX_insitu and ||sin(ELEV_HOR)||<0.1)
//******************************************************************
c
      if(kdvinsitu == 1 && ngates_insitu_max > 1)then
c
//******************************************************************
//**** CONTROL CONTINUITY ALONG THE RAY ( if ICTRL_CONTRAY=1 )
//**** DISMISS VDOP IF |VDOP-VDOP_PREV|>dVDOP_MAX AFTER UNFOLDING
//******************************************************************
c
        if(ictrl_contray == 1)then
c
          init=0
          do ig=1,ngates_insitu_max
	     d_ig=dgate_corr(ig)
	     if(     ze(ig) > -900.
     &           && vdop_corr(ig) > -900.)then
c
	       xis=0.;
	       svis=0.;
	       xrad=0.;
	       svrad=0.;
               if(init == 0)then
                 init=1;
                 xis=xpmin_contray+1.;
	         svis=xis*proj_wind;
	       else
	         init=2;
                 if(d_ig < dmax_insitu)then
                   xis=(dmax_insitu-d_ig)/ddg;
                   svis=xis*proj_wind;
                   igmin=1;
                 else
                   igmin=((d_ig-dmax_insitu)/ddg);
                 endif
                 do jg=igmin,max0(1,ig-1)
                    if(abs(vdop_corr(jg)) < vdop_max)then
	              xrad=xrad+1.;
	              svrad=svrad+vdop_corr(jg);
                    endif
	         enddo
               endif
               xctrl=xis+xrad;
	       if(xctrl.ge.xpmin_contray)then
                 vctrl=(svis+svrad)/xctrl;
                 dv=vdop_corr(ig)-vctrl;
	         idepl=0;
	         if(ichoice_vdop == 1.or.ichoice_vdop == 2)then
                   if(abs(dv) > vnyq)then
		     idepl=1;
		     do while (dv > +vnyq)
  		        vdop_corr(ig)=vdop_corr(ig)-2.*vnyq;
                        dv=vdop_corr(ig)-vctrl;
                     enddo
		     do while (dv < -vnyq)
		        vdop_corr(ig)=vdop_corr(ig)+2.*vnyq;
                        dv=vdop_corr(ig)-vctrl;
                     enddo
                   endif
                 endif
                 if(abs(dv) > dvdop_max)then
                   vdop_corr(ig)=-999.;
	           if(init1)init=0;
                 endif
	       endif
c
	     endif
          enddo
c
        endif    ////////  OF if(ictrl_contray == 1)
c
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      itest=1
      do ig=1,ngates_insitu_max
         if(     ze(ig) > -900.
     &         && vdop_corr(ig) > -900.)itest=1
      enddo
      if(     itest == 1
     &    && nb_ray(iradar_ray) == 5*(nb_ray(iradar_ray)/5))then
        print *,' '
        print *,' ',1000*ihhmmss+ims_ray
     &         ,' IRADAR:',iradar_ray
     &         ,' NO_RAY:',nb_ray(iradar_ray)
        print *,'    ROTA,TILT_RAY:',rota_ray,tilt_ray
        print *,'    ROLL,PITCH,HDG,DRIFT_ACFT:',roll_acft
     &         ,pitch_acft,hdg_acft,drift_acft
        print *,'    AZ_EAST:',azeast_ray,' EL_HOR:',elhor_ray
      endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        do ig=1,ngates_insitu_max

c	if(     ze(ig) > 10.
c     &      && abs(vr(ig)) > 0.
c     &      && abs(vr(ig)) < vdop_max
c     &      && abs(vs(ig)) > 0.
c     &      && abs(vs(ig)) < vdop_max
c     &      && abs(vl(ig)) > 0.
c     &      && abs(vl(ig)) < vdop_max
c     &      && proj_acftspd > -900.
c     &      && dgate_corr(ig) < 10.
c     &      && elhor_ray < 5.
c     &      && elhor_ray > -5.     )then	// 111111
c
c       d_vs_vl=vs(ig)-vl(ig)     // Olivier
c       kvsl=ifix((d_vs_vl/vnyq_el)*0.5)+5      // Olivier
c       if(kvsl.ge.1 && kvsl.le.9)then         // Olivier
c         vs_depl=vs(ig)+xms(kvsl)*vnyq_el       // Olivier
c         vl_depl=vl(ig)+xml(kvsl)*vnyq_el       // Olivier
c         vsl_depl=(vs_depl+vl_depl)/2.         // Olivier
c
c         if(     abs(vs_depl-vl_depl) < vnyq_el/2.
c c    &       && abs(vr(ig)-vsl_depl) < vnyq_el/2.     )then // 112 Oliv
c		print *,'IG= ',ig
c		print *,'VR= ',vr(ig)
c		print *,'VS,VL= ',vs(ig),vl(ig)
c		print *,'VS_depl,VL_depl= ',vs_depl,vl_depl
c		print *,'VSL_depl= ',vsl_depl
c
c	    if(proj_acftspd > -900.)then
c	      v_corr=vr(ig)+proj_acftspd                // Olivier
c	      vdop_corr(ig)=v_corr
c              print *,'    -> VDOP_CORR :',v_corr       // Olivier
c	    endif
c
c         endif
c
c       endif
c	endif                               // Olivier

c------------------------------------------------------------------
c---- ( IF ISIM=1 ) -> SIMULATED dV_dopinsitu WITH dXXX_GUESS
c------------------------------------------------------------------
      if(ig == 1 && isim == 1) {
        ze(1)=999.;
        dv_dopinsitu=-( wa_we_true*dcwe_dt_true
                      +wa_sn_true*dcsn_dt_true
                      +wa_nz*dcnz_dt_true)
                     *dtilt_guess*conv
                    -( wa_we_true*dcwe_dr_true
                      +wa_sn_true*dcsn_dr_true
                      +wa_nz*dcnz_dr_true)
                     *drota_guess*conv
                    -( wa_we_true*dcwe_dp_true
                      +wa_sn_true*dcsn_dp_true
                      +wa_nz*dcnz_dp_true)
                     *dpitch_guess*conv
                    -( wa_we_true*dcwe_dh_true
                      +wa_sn_true*dcsn_dh_true
                      +wa_nz*dcnz_dh_true)
                     *dhdg_guess*conv
                    -(-cwe_true*duacft_dv_true
                      -csn_true*dvacft_dv_true)*dvh_guess;
	vdop_corr(1)=dv_dopinsitu+proj_wind_true;
	do iig=2,ngates_insitu_max
	   ze(iig)=-999.;
	   vdop_corr(iig)=-999.;
	enddo
      }
c------------------------------------------------------------------
           if(     ze(ig) > -900.
              && vdop_corr(ig) > -900.)then
c
             wghtinsitu_ig=1.-0.5*dgate_corr(ig)/dmax_insitu;
c
             dv_dopinsitu=vdop_corr(ig)-proj_wind
c
             if(abs(dv_dopinsitu) < dvdopinsitu_max)then
c
//******************************************************************
//**** ADD WEIGHTS AND DVDOP_insitu
//******************************************************************
c
               n_dvinsitu(iradar_ray)=n_dvinsitu(iradar_ray)+1;
               ssurfins=ssurfins+wghtinsitu_ig;
	       swinsitu_sweep(iradar_ray)
     &          =swinsitu_sweep(iradar_ray)+wghtinsitu_ig;
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      if( nb_ray(iradar_ray) == 5*(nb_ray(iradar_ray)/5) )then
c////////        print *,'    IG=',ig,' -> DVDOPINSITU_RAY :',dv_dopinsitu
c////////        print *,'       SWVSURF_SWEEP(',iradar_ray,') :'
c////////     &         ,swvsurf_sweep(iradar_ray)
c////////      endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
	       dvinsitusweep_mean(iradar_ray)
     &          =dvinsitusweep_mean(iradar_ray)
     &           +wghtinsitu_ig*dv_dopinsitu;
	       dvinsitusweep_rms(iradar_ray)
     &          =dvinsitusweep_rms(iradar_ray)
     &           +wghtinsitu_ig*dv_dopinsitu*dv_dopinsitu
c
               swdvinsitu_tot=swdvinsitu_tot+wghtinsitu_ig;
	       swdvminsitu_tot=swdvminsitu_tot
     &                         +wghtinsitu_ig*dv_dopinsitu;
	       swdv2insitu_tot=swdv2insitu_tot
     &                         +wghtinsitu_ig
     &                          *dv_dopinsitu*dv_dopinsitu;
               swadvinsitu_tot=swadvinsitu_tot
     &                         +wghtinsitu_ig*abs(dv_dopinsitu);
c
               s_vpv(iradar_ray,ilr)=s_vpv(iradar_ray,ilr)
     &                               +wghtinsitu_ig;
               sv_vpv(iradar_ray,ilr)=sv_vpv(iradar_ray,ilr)
     &                                +wghtinsitu_ig*dv_dopinsitu;
               svv_vpv(iradar_ray,ilr)=svv_vpv(iradar_ray,ilr)
     &                                 +wghtinsitu_ig
     &                                  *dv_dopinsitu*dv_dopinsitu;
               x_vpv(iradar_ray,ilr)=x_vpv(iradar_ray,ilr)
     &                               +wghtinsitu_ig;
               xv_vpv(iradar_ray,ilr)=xv_vpv(iradar_ray,ilr)
     &                                +wghtinsitu_ig*dv_dopinsitu;
               xvv_vpv(iradar_ray,ilr)=xvv_vpv(iradar_ray,ilr)
     &                                 +wghtinsitu_ig
     &                                  *dv_dopinsitu*dv_dopinsitu;
c
//******************************************************************
//**** VALUES OF VAR(1->NVAR) FOR FIELD "DV_insitu"
//****  - VAR(1->6) -> [dT_aft,dT_fore,dR_aft,dR_fore,dP,dH] in DEGREES
//****  - VAR(7->11) -> [RD_aft,RD_fore,dXwe,dYsn,dZ] in HECTOMETERS
//****  - VAR(12) -> [dVH] in METER/SECOND
//******************************************************************
c
               if(iaftfore == -1) {
                 if(idtiltaft == 1) {
                   var(1)=( wa_we*dcwe_dt+wa_sn*dcsn_dt
                          +wa_nz*dcnz_dt)*conv;
                 } else {
                   var(1)=0.;
                   xmat_dvinsitu(1,1)=xmat_dvinsitu(1,1)
                                     +wghtinsitu_ig;
                 }
                 var(2)=0.;
               } else {
                 var(1)=0.;
                 if(idtiltfore == 1) {
                   var(2)=( wa_we*dcwe_dt+wa_sn*dcsn_dt
                          +wa_nz*dcnz_dt)*conv;
                 } else {
                   var(2)=0.;
                   xmat_dvinsitu(2,2)=xmat_dvinsitu(2,2)
                                     +wghtinsitu_ig;
                 }
               }
//
               if(iaftfore == -1) {
                 if(idrotaaft == 1) {
                   var(3)=( wa_we*dcwe_dr+wa_sn*dcsn_dr
                          +wa_nz*dcnz_dr)*conv;
                 } else {
                   var(3)=0.;
                   xmat_dvinsitu(3,3)=xmat_dvinsitu(3,3)
                                     +wghtinsitu_ig;
                 }
                 var(4)=0.;
               } else {
                 var(3)=0.;
                 if(idrotafore == 1) {
                   var(4)=( wa_we*dcwe_dr+wa_sn*dcsn_dr
                          +wa_nz*dcnz_dr)*conv;
                 } else {
                   var(4)=0.;
                   xmat_dvinsitu(4,4)=xmat_dvinsitu(4,4)
                                     +wghtinsitu_ig;
                 }
               }
c
               if(idpitch == 1)then
                 var(5)=( wa_we*dcwe_dp+wa_sn*dcsn_dp
     &                   +wa_nz*dcnz_dp)*conv;
               else
                 var(5)=0.;
                 xmat_dvinsitu(5,5)=xmat_dvinsitu(5,5)
     &                              +wghtinsitu_ig;
               endif
c
               if(idhdg == 1)then
                 var(6)=(wa_we*dcwe_dh+wa_sn*dcsn_dh)*conv;
               else
                 var(6)=0.;
                 xmat_dvinsitu(6,6)=xmat_dvinsitu(6,6)
     &                              +wghtinsitu_ig;
               endif
c
               var(7)=0.;
               var(8)=0.;
               var(9)=0.;
               var(10)=0.;
               var(11)=0.;
c
               if(idvh == 1)then
                 var(12)=-duacft_dv*cwe-dvacft_dv*csn;
               else
                 var(12)=0.;
                 xmat_dvinsitu(12,12)=xmat_dvinsitu(12,12)
     &                                +wghtinsitu_ig;
               endif
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c////////      print *,'    VAR_DVINSITU(1->12):',(var(i),i=1,12)
c//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c
//******************************************************************
//**** ADD TO XMAT_dvinsitu(1->NVAR,1->NVAR) AND VECT_dvinsitu(1->NVAR)
//******************************************************************
c
               do i=1,nvar
   	          do j=1,nvar
 	             xmat_dvinsitu(i,j)=xmat_dvinsitu(i,j)
     &                                  +wghtinsitu_ig*var(i)*var(j);
 	          enddo
 	          vect_dvinsitu(i)=vect_dvinsitu(i)
     &                             +wghtinsitu_ig*var(i)*dv_dopinsitu;
               enddo
c
//******************************************************************
//**** ADD TO COVARIANCE MATRIX FOR FIELD "DVDOP_insitu"
//******************************************************************
c
               do i=1,nvar
                  rms_var_vinsitu(i)=rms_var_vinsitu(i)
     &                               +wghtinsitu_ig*var(i)*var(i);
                  do j=1,nvar
                     corr_var(i,j)=corr_var(i,j)
     &                             +wghtinsitu_ig*var(i)*var(j);
                  enddo
               enddo
c
//******************************************************************
//**** CASE "VDOP_surf" and/or "DVDOP_insitu" ONLY :
//**** -> RGE-DLY_aft,RGE-DLY_aft,D_XWE,D_YSN,D_ZACFT CANNOT BE CALCULATED
//******************************************************************
c
               if(rw_dzsurf.le.0.) {
                 do ij=7,11
                    xmat_vsurf(ij,ij)=xmat_vsurf(ij,ij)
     &                                +wghtinsitu_ig;
                 enddo
               }
c
//******************************************************************
//**** ARRAYS FOR "SIS_EL_*" FILE
//******************************************************************
c
               vi_dhor(iradar_ray,n_dvinsitu(iradar_ray))
     &                 =side*dgate_corr(ig)*celh;
               vi_vdop(iradar_ray,n_dvinsitu(iradar_ray))=vdop;
               vi_vinsitu(iradar_ray,n_dvinsitu(iradar_ray))=proj_wind;
c
             endif  ////  of  ////  if(abs(dv_dopinsitu) < dvdopinsitu_max)  ////
c
           endif  ////  of  ////  if(ze(ig) > -900. ... )  ////
        enddo ////  of  ////  do ig=1,ngates_insitu_max  ////
c
      endif  ////  of  ////  if(kdvinsitu == 1 && ngates_insitu_max > 1)  ////
c
//******************************************************************
//**** STORE FOR NEXT RAY
//******************************************************************

      istart_sweep(iradar_ray)=1;
      swp_prev(iradar_ray)=swp(iradar_ray);
      vnyq_prev=vnyq;
      rota_prev(iradar_ray)=rota_ray;
      tilt_prev=tilt_ray;

      go to 1

//  3   stop
//      end
}

//
//******************************************************************
//
//**** CALCULATE DIRECTOR COSINES, AZIM_EST ET DE ELEV_HOR
//**** FROM LEE ET AL. (JTech, 1994, 11, 572-578)
//
//******************************************************************
//
/*
      subroutine azel(rotaroll,tilt_ray
     &                ,hdg_acft,drift_acft,pitch_acft
     &                ,azeast_ray,elhor_ray
     &                ,cxa,cya,cza,cwe,csn,cnz)
      common/cosinang/crr,srr,cti,sti
     &               ,chdg,shdg,cdri,sdri,cpit,spit
     &               ,caze,saze,celh,selh
*/
void azel(rotaroll,tilt_ray
     &                ,hdg_acft,drift_acft,pitch_acft
     &                ,azeast_ray,elhor_ray
     &                ,cxa,cya,cza,cwe,csn,cnz) {


      common/cosinang/crr,srr,cti,sti
     &               ,chdg,shdg,cdri,sdri,cpit,spit
     &               ,caze,saze,celh,selh

      conv=3.14159/180.

      crr=cos(conv*rotaroll);
      srr=sin(conv*rotaroll);
      cti=cos(conv*tilt_ray);
      sti=sin(conv*tilt_ray);
      if(srr > 0.)then
        side=+1.;
      else
        side=-1.;
      endif
c
      chdg=cos(conv*hdg_acft);
      shdg=sin(conv*hdg_acft);
      cdri=cos(conv*drift_acft);
      sdri=sin(conv*drift_acft);
      cpit=cos(conv*pitch_acft);
      spit=sin(conv*pitch_acft);
c
      cxa=+srr*cti;
      cya=-cti*crr*spit+sti*cpit;
      cza=+cti*crr*cpit+sti*spit;
c
      cwe=+chdg*cxa+shdg*cya;
      csn=-shdg*cxa+chdg*cya;
      cnz=+cza;
      azeast_ray=atan2(csn,cwe)/conv;
      caze=cos(conv*azeast_ray);
      saze=sin(conv*azeast_ray);
      do while (azeast_ray.le.0.)
         azeast_ray=azeast_ray+360.;
      enddo
      chor=amax1(0.1,sqrt(cwe*cwe+csn*csn));
      elhor_ray=atan(cnz/chor)/conv;
      celh=cos(conv*elhor_ray);
      selh=sin(conv*elhor_ray);
//
      return
      end
//
//****************************************************************************
//
//**** INVERSION OF MATRIX (NVAR,NVAR)
//
//****************************************************************************
//
//      subroutine resoud(xmat,xinv,vect,res,nvar)
void resoud(xmat,xinv,vect,res,nvar) {
//
      dimension xmat(nvar,nvar),vect(nvar),res(nvar)
//
      chol_inv(xmat,res,vect,nvar,ierr);
//
}

//
//****************************************************************************
//
//**** INTERPOLATION TO FILL THE HOLES IN THE RADAR-DERIVED SURFACE MAP
//
//****************************************************************************
//
      subroutine inter(sp,sz,nx,ny,nxysurfmax)
      dimension sp(nxysurfmax,nxysurfmax),sz(nxysurfmax,nxysurfmax)
     &         ,x(1000),y(1000),s(1000),d(1000)
      nsaut=5;
      nmin=5;
      spmin=1.;
      nin=0;
      nintx=0;
      ninty=0;
      nout=0;
c
      do j=1,ny
         do i=1,nx
            if(sp(i,j) > spmin)then
              sz(i,j)=sz(i,j)/sp(i,j);
              nin=nin+1;
	    else
	      sz(i,j)=-999.;
            endif
         enddo
      enddo
c
      print *,'     -> ALONG X'
      do j=1,ny
         imax=1;
  1      imin=imax;
	 iant=0;
	 n=0;
	 do i=imin,nx
	    imax=i;
	    if(sz(i,j) > -900.)then
	      if(iant.ne.0 && (i-iant) > nsaut+1)go to 2
	      iant=i;
	      n=n+1;
	      x(n)=float(i);
	      y(n)=sz(i,j);
            endif
         enddo
  2      if(n.ge.nmin)then
	   q1=(y(2)-y(1))/(x(2)-x(1));
	   qn=(y(n)-y(n-1))/(x(n)-x(n-1));
	   call spline(x,y,s,d,q1,qn,n)
	   do i=imin,imax
              if(sz(i,j) < -900.)then
	        xi=float(i);
	        val=splin(xi,x,y,s,d,q1,qn,n);
                if(val > 0.)then
                  sz(i,j)=val;
                  nintx=nintx+1;
                endif
              endif
           enddo
         endif
	 if(imax.le.(nx-nsaut+1))go to 1
      enddo
c
      print *,'     -> ALONG Y'
      do i=1,nx
         jmax=1;
  3      jmin=jmax;
         jant=0;
 	 n=0;
	 do j=jmin,ny
	    jmax=j;
	    if(sz(i,j) > -900.)then
	      if(jant.ne.0 && (j-jant) > nsaut+1)go to 4
	      jant=j;
	      n=n+1;
	      x(n)=float(j);
	      y(n)=sz(i,j);
            endif
         enddo
  4      if(n.ge.nmin)then
	   q1=(y(2)-y(1))/(x(2)-x(1));
	   qn=(y(n)-y(n-1))/(x(n)-x(n-1));
	   call spline(x,y,s,d,q1,qn,n);
	   do j=jmin,jmax
              if(sz(i,j) < -900.)then
	        yj=float(j);
	        val=splin(yj,x,y,s,d,q1,qn,n);
                if(val > 0.)then
                  sz(i,j)=val;
                  ninty=ninty+1;
                  nput=nout+1;
                endif
              else
                nout=nout+1;
              endif
           enddo
         endif
	 if(jmax.le.(ny-nmin+1))go to 3
      enddo
c
      print *,'     -> N_in,int_X,int_Y,out :',nin,nintx,ninty,nout
c
      return
      end
c
//****************************************************************************
c
//**** SUBROUTINE SPLINE
c
//****************************************************************************
c
      subroutine spline(x,u,s,del,q1,qn,n)
      dimension x(1000),u(1000),s(1000),del(1000)
      dimension a(1000),v(1000)
c
      del(2)=x(2)-x(1);
      v(1)=6.*(((u(2)-u(1))/del(2))-q1);
      n1=n-1;
      do i=2,n1
         del(i+1)=x(i+1)-x(i);
         v(i)=((u(i-1)/del(i))-u(i)*((1./del(i))+(1./del(i+1)))
     &         +(u(i+1)/del(i+1)))*6.;
      enddo
      v(n)=(qn+(u(n1)-u(n))/del(n))*6.;
c
      a(1)=2.*del(2);
      a(2)=1.5*del(2)+2.*del(3);
      v(2)=v(2)-.5*v(1);
      do i=3,n1
         c=del(i)/a(i-1);
         a(i)=2.*(del(i)+del(i+1))-c*del(i);
         v(i)=v(i)-c*v(i-1);
      enddo
      c=del(n)/a(n1);
      a(n)=2.*del(n)-c*del(n);
      v(n)=v(n)-c*v(n1);
c
      s(n)=v(n)/a(n);
      do j=1,n1
         i=n-j;
         s(i)=(v(i)-del(i+1)*s(i+1))/a(i);
      enddo
c
       return
      end
c
//****************************************************************************
c
//**** FUNCTION SPLIN
c
//****************************************************************************
c
      function splin(v,x,u,s,del,q1,qn,n)
      dimension x(1000),u(1000),s(1000),del(1000)
c
      if(v-x(1))50,10,20
  10  splin=u(1);
      return
  20  do k=2,n
         if(v-x(k))30,30,40;
  30     k1=k-1;
         ff1=s(k1)*(x(k)-v)**3.;
         ff2=s(k)*(v-x(k1))**3.;
         ff3=1./(6.*del(k));
         f1=(ff1+ff2)*ff3;
         f2=(v-x(k1))*(u(k)/del(k)-s(k)*del(k)/6.);
         f3=(x(k)-v)*(u(k1)/del(k)-s(k1)*del(k)/6.);
         splin=f1+f2+f3;
         return
  40     continue
      enddo
  50  splin=0.;
c
      return
      end
