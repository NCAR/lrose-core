C  *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
C  ** Copyright UCAR (c) 1990 - 2016                                         
C  ** University Corporation for Atmospheric Research (UCAR)                 
C  ** National Center for Atmospheric Research (NCAR)                        
C  ** Boulder, Colorado, USA                                                 
C  ** BSD licence applies - redistribution and use in source and binary      
C  ** forms, with or without modification, are permitted provided that       
C  ** the following conditions are met:                                      
C  ** 1) If the software is modified to produce derivative works,            
C  ** such modified software should be clearly marked, so as not             
C  ** to confuse it with the version available from UCAR.                    
C  ** 2) Redistributions of source code must retain the above copyright      
C  ** notice, this list of conditions and the following disclaimer.          
C  ** 3) Redistributions in binary form must reproduce the above copyright   
C  ** notice, this list of conditions and the following disclaimer in the    
C  ** documentation and/or other materials provided with the distribution.   
C  ** 4) Neither the name of UCAR nor the names of its contributors,         
C  ** if any, may be used to endorse or promote products derived from        
C  ** this software without specific prior written permission.               
C  ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
C  ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
C  ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
C  *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C  spolTS.f
C 
C  John Hubbert, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
C 
C  Dec 2021
C 
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C 
C  Perform regression clutter filtering
C 
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

C BELOW IS DOCUMENTATION FOR THE NCAR?CISL FFTPACK. IT CAN DO ARBITRARY LENGTHS FFTs.
C  THE PACKAGE USES 3 CALLS or SUBROUTINE
c     1 CFFT1I(N, WSAVE, LENSAV, IER)  INITIALIZES THE NEEDED PARAMETERS AND ARRAYS for FFT "PACK"
c     2 CFFT1F(N, INC, C, LENC, WSAVE, LENSAV, WORK, LENWRK, IER)  does the FFT
c     3 CFFT1B(N, INC, C, LENC, WSAVE, LENSAV, WORK, LENWRK, IER)  does the Inver. FFT
c     When a new sequence length is selected, the initialization sub CFFT1I has to be called
c     Subsequent calls to CFFT1F and CFFT1B do not require CFFT1I
C     FOR BLOCK PROCESSING,  THE FFTPACK WILL NEED TO BE REINITILIZED FOR THE REGRESSION FILTER.
C     AFTER  BLOCK PROCESSING, THE FFT PACK WILL NEED TO BE REINITIALIZED FOR SPECTRA PLOTING  
c___________________________________________________________________
cSUBROUTINE CFFT1F (N, INC, C, LENC, WSAVE, LENSAV, WORK, LENWRK, IER)
c INTEGER    N, INC, LENC, LENSAV, LENWRK, IER
c COMPLEX    C(LENC)
c REAL       WSAVE(LENSAV), WORK(LENWRK)
c
c FFTPACK 5.1 routine CFFT1F computes the one-dimensional Fourier
c transform of a single periodic sequence within a complex array.
c This transform is referred to as the forward transform or Fourier
c analysis, transforming the sequence from physical to spectral
c space.
c
c This transform is normalized since a call to CFFT1F followed
c by a call to CFFT1B (or vice-versa) reproduces the original
c array subject to algorithm constraints, roundoff error, etc.
c
c Input Arguments
c
c N       Integer length of the sequence to be transformed.  The
c         transform is most efficient when N is a product of
c         small primes.
c
c INC     Integer increment between the locations, in array C, of two
c         consecutive elements within the sequence to be transformed.
c
c C       Complex array of length LENC containing the sequence to be
c         transformed.
c
c LENC    Integer dimension of C array.  LENC must be at least
c         INC*(N-1) + 1.
c WSAVE   Real work array with dimension LENSAV.  WSAVE's contents
c         must be initialized with a call to subroutine CFFT1I before
c         the first call to routine CFFT1F or CFFT1B for a given
c         transform length N.  WSAVE's contents may be re-used for
c         subsequent calls to CFFT1F and CFFT1B with the same N.
c LENSAV  Integer dimension of WSAVE array.  LENSAV must be at least
c         2*N + INT(LOG(REAL(N))/LOG(2.)) + 4.
c
c
c WORK    Real work array of dimension LENWRK.
c
c LENWRK  Integer dimension of WORK array.  LENWRK must be at
c         least 2*N.
c____________________________________________________________________

C)     This subroutine loads in the next ray of time series data
C      THIS SUBROUTINE IS CALLED FROM SUBROUTINE GETTIME WHERE
C      THE RAYCOUNT IS INCREMENTED. THE SUBROUTINE initialize_timeseries
C      GETS THE FIRST RAY OF  TIME SERIES AS WELL AS ELEVATION, AZIMUTH, 
C      AND TIME ARRAYS. THIS SUBROUTINE THEN RETRIEVES SUBSEQUENT RAYS OF TS. 
C
       subroutine loadnetcdf
C
       implicit none
      include 'netcdf.inc'
      integer npulses,ngates
      real time_inc
      parameter (npulses=30000,ngates = 1000)
c      the input time series arrays are dynamically allocated, in terms
c      of "gates" dimension, in te initialize subroutine below. The
c    timeseries arrays here are  dimentioned by "gates" which is
c    determined  
      REAL, DIMENSION(:,:), ALLOCATABLE :: IHc_A,QHc_A,
     > IVc_A,QVc_A,IVx_A,QVx_A,IHx_A,QHx_A
       REAL, DIMENSION(:), ALLOCATABLE :: azimuth_in, elevation_in
       INTEGER,DIMENSION(:), ALLOCATABLE ::  range_in  
        real elevation(npulses), azimuth(npulses),range_ary(ngates)
        double precision rvp8time(npulses)
       integer  status, stat,indexangle
c      REAL, DIMENSION(:,:), ALLOCATABLE :: IHc_in,QHc_in,
c    > IVc_in,QVc_in,IVx_in,QVx_in,IHx_in,QHx_in
c
c     DOUBLE PRECISION, ALLOCATABLE, DIMENSION(:) :: rvp8time_in
      common /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
       real lambda,Ts,range,azim,elev,range_inc
       real vi(1025),vq(1025),ui(1025),uq(1025),vui(1025),vuq(1025),
     >  uvi(1025),uvq(1025),extraI,extraQ
       integer year,month,day,gates,hits,hour,min,sec
     &     ,rng_index,raycount,nkeep,iwdw,nn,jj
       common /raydat/range_inc,range,azim,elev,hour,min,time_inc,
     >  sec,year,month,day,hits,gates,rng_index,raycount,maxrayno
     >  ,range_ary
c      common/netcdf/Elevation,Azimuth,rvp8time,pulses,I_in,Q_in
       common/netcdf/elevation,azimuth,rvp8time,pulses,IHc_in,QHc_in,
     > IVc_in,QVc_in,IVx_in,QVx_in,IHx_in,QHx_in
      common /infile1/filename
C     This is the name of the data file we will read 
      character filename*80
C     We are reading 2D data, a npulse by ngate grid. 
      integer frtime, NDIMS,maxrayno
      parameter (NDIMS=2)
C     The start and count arrays will tell the netCDF library where to
C     read our data.
      integer start(NDIMS), count(NDIMS),retval,x,y
c     parameter (frtime = 20246, gates = 591)
c     dimension the I and Q  to accomodate all gates
c     and up to 512 samples . Dimension elevation and Azmuth etc to
c     accomodate the entire length (30,000?) 
c IHc  Htx copolar
C IVx  Htx crosspolar
c IVc  Vtx copolar
c IHx  Vtx crosspolar
      real IHc_in(ngates,1024), QHc_in(ngates,1024)
      real IVx_in(ngates,1024), QVx_in(ngates,1024)
      real IVc_in(ngates,1024), QVc_in(ngates,1024)
      real IHx_in(ngates,1024), QHx_in(ngates,1024)
      real days
      integer Prt(npulses)
c
C     This will be the netCDF ID for the file and data variable.
      integer ncid, IHc_varid, QHc_varid,IVx_varid, QVx_varid,
     > IVc_varid, QVc_varid,IHx_varid, QHx_varid, pulses
C     Open the file. NF_NOWRITE tells netCDF we want read-only access to
C     the file.
      retval = nf_open(FILENAME, NF_NOWRITE, ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)
c
C     GET THE VARID OF THE DATA VARIABLE  BASED ON ITS NAME.
c
c   HH
      retval = nf_inq_varid(ncid, 'IHc', IHc_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_varid(ncid, 'QHc', QHc_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
c   VH
      retval = nf_inq_varid(ncid, 'IVx', IVx_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_varid(ncid, 'QVx', QVx_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
c   VV
      retval = nf_inq_varid(ncid, 'IVc', IVc_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_varid(ncid, 'QVc', QVc_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
c   HV
      retval = nf_inq_varid(ncid, 'IHx', IHx_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_varid(ncid, 'QHx', QHx_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
c
c      DYNAMIC ALLOCATION
c
      allocate (IHc_A(gates,hits+1), QHc_A(gates,hits+1))
      allocate (IVx_A(gates,hits+1), QVx_A(gates,hits+1))
      allocate (IVc_A(gates,hits+1), QVc_A(gates,hits+1))
      allocate (IHx_A(gates,hits+1), QHx_A(gates,hits+1))
      allocate (range_in(gates), stat=STATUS)
      allocate (azimuth_in(gates), stat=STATUS)
      allocate (elevation_in(gates), stat=STATUS)

c     Set up ray reads of I&Q values
      count(1) = gates     ! get all gates ie, read in one ray of time series data
c     count(2) = 512      ! Get this length of time series
      count(2) = hits+1      ! Get this length of time series
       start(1)= 1        ! start at gate one
C
c      NOTE: that the retrieved time series length is 512 (arbitrary) but the starting
c       pulse of the next ray retrieval is incremented by hits.
C
       start(2)= (raycount-1)*hits +1   !Start with first pulse in the current ray.
c      print *,"count, start", count(1),count(2),start(1),start(2)
c
C     READ THE DATA.
C
c  HH
      retval = nf_get_vara_real(ncid, IHc_varid, start, count, IHc_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_get_vara_real(ncid, QHc_varid, start, count, QHc_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
c  VH
      retval = nf_get_vara_real(ncid, IVx_varid, start, count, IVx_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_get_vara_real(ncid, QVx_varid, start, count, QVx_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
c  VV
      retval = nf_get_vara_real(ncid, IVc_varid, start, count, IVc_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_get_vara_real(ncid, QVc_varid, start, count, QVc_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
c  HV
      retval = nf_get_vara_real(ncid, IHx_varid, start, count, IHx_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_get_vara_real(ncid, QHx_varid, start, count, QHx_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
C
C Azimuth, Elevation and Rvp8time are read in in subroutine
C initialize_timeseres
CJHc ELEVATION
CJH      retval = nf_inq_varid(ncid, 'elevation_hc', elev_varid)
CJH      if (retval .ne. nf_noerr) call handle_err(retval)
CJHc AZIMUTH
CJH      retval = nf_inq_varid(ncid, 'azimuth_hc', az_varid)
CJH      if (retval .ne. nf_noerr) call handle_err(retval)
CJHC READ AZ, EL
CJH       retval = nf_get_var_real(ncid, elev_varid, elevation_in)
CJH      if (retval .ne. nf_noerr) call handle_err(retval)
CJH      retval = nf_get_var_real(ncid, az_varid, azimuth_in)
CJH      if (retval .ne. nf_noerr) call handle_err(retval)
c
C   TRANSFER DATA TO PASSABLE ARRAYS
c
       do x = 1, gates
         do y=1, hits+1
             IHc_in(x,y) = IHc_A(x,y)
             QHc_in(x,y) = QHc_A(x,y)
             IHx_in(x,y) = IHx_A(x,y)
             QHx_in(x,y) = QHx_A(x,y)
             IVc_in(x,y) = IVc_A(x,y)
             QVc_in(x,y) = QVc_A(x,y)
             IVx_in(x,y) = IVx_A(x,y)
             QVx_in(x,y) = QVx_A(x,y)
          end do
       end do
c********************
c  DO CMD ON THE RAY OF RAW DATA
       call CMD(hits,gates,maxrayno)
c*******************
       indexangle = (raycount-1)*hits + hits/2 
       azim= azimuth(indexangle)
       elev=elevation(indexangle)
       time_inc = rvp8time((raycount-1)*hits +1)
c      azim = azimuth_in(start(2))
c      elev = elevation_in(start(2))
c       range= range_ary(rng_index)
c
C     write the data.
c      do x = 1, gates      !gates
c         do y = 1,hits   !frtime 
c          print *,'Time, Az, Ele ',Time(start(2)+y),azimuth(start(2)+y),
c    $                                  elevation(start(2)+y)
c           print *, 'I_in(' ,x,  y, ') = ', I_in(x, y),Q_in(x,y)
c         write(3,*) x,y, I_in(x, y),Q_in(x,y)            
c         write(6,*) x,y, I_in(x, y),Q_in(x,y)            
c         end do
c       print *,"NEXT GATE?"
c       read(*,*)
c      end do
c
C     Close the file, freeing all resources.
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      return
      end
C-------------------------------------------------------
C      This Subroutine gets the time, number of gates and
c      the number of transmit pulses, and first ray of I&Q data
c
       subroutine initialize_timeseries
c
       implicit none
       include 'netcdf.inc'
       integer npulses,ngates,i
      parameter (npulses=30000,ngates = 1000)
       integer  status, stat
        real time_inc
       real elevation(npulses), azimuth(npulses),range_ary(ngates)
       real basedbzh,basedbzv,noiseh,noisev
        double precision rvp8time(npulses)
       REAL, DIMENSION(:,:), ALLOCATABLE :: IHc_A,QHc_A,
     > IVc_A,QVc_A,IVx_A,QVx_A,IHx_A,QHx_A
       REAL, DIMENSION(:), ALLOCATABLE :: azimuth_in, elevation_in,
     > prt_in 
       INTEGER, DIMENSION(:), ALLOCATABLE :: range_in  
      double precision, allocatable, dimension(:) :: rvp8time_in
      common /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
       real vi(1025),vq(1025),ui(1025),uq(1025),vui(1025),vuq(1025),
     >  uvi(1025) ,uvq(1025),extraI,extraQ
       integer year,month,day,gates,hits,hour,min,sec
     &         ,rng_index,raycount
       common /raydat/range_inc,range,azim,elev,hour,min,time_inc,
     >  sec,year,month,day,hits,gates,rng_index,raycount,maxrayno
     >  ,range_ary
c      common/netcdf/elevation,azimuth,rvp8time,pulses,I_in,Q_in
       common/netcdf/elevation,azimuth,rvp8time,pulses,IHc_in,QHc_in,
     > IVc_in,QVc_in,IVx_in,QVx_in,IHx_in,QHx_in
       integer iwdw,jj,nn,pulses,nkeep
       real lambda,Ts, range,range_inc,elev,azim,range_start
      common /infile1/filename
       character*80 infile
      common /radarconstants/basedbzh,basedbzv,noiseh,noisev
C     This is the name of the data file we will read 
      character FILENAME*80
c      parameter (FILE_NAME='kftg_ts.nc')
C     The start and count arrays will tell the netCDF library where to
C     read our data.
      integer frtime, NDIMS, maxrayno
      parameter (NDIMS=2)
      integer start(NDIMS), count(NDIMS)
      
c     parameter (frtime = 20246, gates = 591)
c     dimension the I and Q  to accomodate all gates
c     and up to 512 samples . Dimension elevation and Azmuth etc to
c     accomodate the entire length (30,000?) 
c
C IMPORTANT NOTE: the array dimentions ngates needs to dimentioned to the ngates
C dimension of the netcdf file!Also note that the dimentions of the 2-D
C fortran data array is switched as compared to the netcdf dimentioned
C arrays!! i.e., in the netcdf file: (time, gates) in fortran
C (gates,time)
c
C      THESE ARE PASSABLE ARRAYS
      real IHc_in(ngates,1024), QHc_in(ngates,1024)
      real IVx_in(ngates,1024), QVx_in(ngates,1024)
      real IVc_in(ngates,1024), QVc_in(ngates,1024)
      real IHx_in(ngates,1024), QHx_in(ngates,1024)
      real days
c
      integer Prt(npulses)
c     The next line is for converting linux time to month/day/year/hr/mim/sec
      integer utime, idate(6)
      integer total_secs, hr, minute,second
      double precision tot_secs,frac_secs

C     This will be the netCDF ID for the file and data variable.

      integer  IHc_varid, QHc_varid,IVx_varid, QVx_varid,
     > IVc_varid, QVc_varid,IHx_varid, QHx_varid
      integer ncid, Time_varid, elev_varid, az_varid,
     $       Prt_varid, gates_id, frtime_id, range_varid
      integer base_year_varid,base_month_varid,base_day_varid,
     $ base_hour_varid, base_min_varid,base_second_varid,
     $ radar_conh_varid
c
C     Loop indexes, and error handling.
      integer x, y, z, retval
      open(1,file="out.d")
      open(321,file="TS1")
c    DETERMINE THE NUMBER OF HITS TO BE USED. This value should not
C     change during the file access process. The number of samples can
c     be changed via "nn" but the program will continue to read rays
c     with the number of specified "hits". The file can be rewound and
c     that point the number of hits can be changed.
C
C   SET THE LENGTH OF THE TIME SERIES HERE, called hits
C
100   Write(*,*) "ENTER THE NUMBER OF 'HITS' TO BE USED."
      write(*,*) "This is a constant but can be changed by rewinding"
      write(*,*) "the data file."
      read(*,*)   hits
      if(hits.gt.7.and.hits.lt.1025) then
      else
         write(*,*) "HITS NEEDS To BE >=8 and  <=1024"
         goto 100
      endif  
      nn=hits
c
C     OPEN THE FILE. NF_NOWRITE tells netCDF we want read-only access to
C     the file.
c
      print *, filename
      retval = nf_open(FILENAME, NF_NOWRITE, ncid)
      Print *,"back from get  file id"
      if (retval .ne. nf_noerr) call handle_err(retval)
      print *,"opened file",filename
c
C     GET THE IDs OF THE DIMENSIONS GATES AND FRTIME (frtime is time in netcdf file)
c
      retval = NF_INQ_DIMID(NCID, 'time', frtime_id)
      IF (retval .NE. NF_NOERR) CALL HANDLE_ERR(retval)
      retval = NF_INQ_DIMID(NCID, 'gates', gates_id)
      IF (retval .NE. NF_NOERR) CALL HANDLE_ERR(retval)
      print *,'dimension ids= ', frtime_id, gates_id
c
c
c     GET THE DIMENTION LENGHTS
c
      retval = NF_INQ_DIMLEN(NCID, frtime_id, frtime)
      IF (retval .NE. NF_NOERR) CALL HANDLE_ERR(retval)
      ! get unlimited dimension name and current length
      retval = NF_INQ_DIMLEN(NCID,gates_id, gates)
      IF (retval .NE. NF_NOERR) CALL HANDLE_ERR(retval)
c
c      DYNAMIC ALLOCATION
      allocate (IHc_A(gates,hits+1), QHc_A(gates,hits+1))
      allocate (IVx_A(gates,hits+1), QVx_A(gates,hits+1))
      allocate (IVc_A(gates,hits+1), QVc_A(gates,hits+1))
      allocate (IHx_A(gates,hits+1), QHx_A(gates,hits+1))
c     allocate (IHc_A(gates, 512), QHc_A(gates, 512))
c     allocate (IVx_A(gates, 512), QVx_A(gates, 512))
c     allocate (IVc_A(gates, 512), QVc_A(gates, 512))
c     allocate (IHx_A(gates, 512), QHx_A(gates, 512))
      allocate (range_in(gates), stat=STATUS)
      allocate (azimuth_in(frtime), stat=STATUS)
      allocate (elevation_in(frtime), stat=STATUS)
      allocate (rvp8time_in(frtime), stat=STATUS)
      allocate (prt_in(frtime), stat=STATUS)
c the number range is gates
c the number of pulses/samples is frtime
c hits is the user defined time series length
      print *,'frtime, gates length =', frtime, gates
      pulses = frtime
      maxrayno= frtime/hits
      jj=gates
c
C     Get the varid of the data variable, based on its name.
c

c   HH
      retval = nf_inq_varid(ncid, 'IHc', IHc_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_varid(ncid, 'QHc', QHc_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
c   VH
      retval = nf_inq_varid(ncid, 'IVx', IVx_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_varid(ncid, 'QVx', QVx_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
c   VV
      retval = nf_inq_varid(ncid, 'IVc', IVc_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_varid(ncid, 'QVc', QVc_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
c   HV
      retval = nf_inq_varid(ncid, 'IHx', IHx_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_inq_varid(ncid, 'QHx', QHx_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
c TIME
      retval = nf_inq_varid(ncid, 'time_offset_hc', Time_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)

        retval = nf_inq_varid(ncid, 'base_year', base_year_varid)
        if (retval .ne. nf_noerr) call handle_err(retval)
        retval = nf_inq_varid(ncid, 'base_month', base_month_varid)
        if (retval .ne. nf_noerr) call handle_err(retval)
        retval = nf_inq_varid(ncid, 'base_day', base_day_varid)
        if (retval .ne. nf_noerr) call handle_err(retval)
        retval = nf_inq_varid(ncid, 'base_hour', base_hour_varid)
        if (retval .ne. nf_noerr) call handle_err(retval)
        retval = nf_inq_varid(ncid, 'base_sec', base_second_varid)
        if (retval .ne. nf_noerr) call handle_err(retval)
        retval = nf_inq_varid(ncid, 'base_min', base_min_varid)
        if (retval .ne. nf_noerr) call handle_err(retval)
c
C   GET RADAR CALIB CONSTANTS FOR Hc AND Vc CHANNELS
c
c2345678901234567890123456789012345678901234567890123456789012345678901
c        1         2         3         4         5         6         7
c      retval = nf_get_att_real(ncid,NF_GLOBAL,
c    $  'cal_radar_constant_h', rconh)
c      if (retval .ne. nf_noerr) call handle_err(retval)
c
       retval = nf_get_att_real(ncid,NF_GLOBAL,
     $  'cal_noise_dbm_vc', noisev)
       if (retval .ne. nf_noerr) call handle_err(retval)
c
       retval = nf_get_att_real(ncid,NF_GLOBAL,
     $  'cal_noise_dbm_hc', noiseh)
       if (retval .ne. nf_noerr) call handle_err(retval)
c
       retval = nf_get_att_real(ncid,NF_GLOBAL,
     $  'cal_base_dbz_1km_hc', basedbzh)
       if (retval .ne. nf_noerr) call handle_err(retval)
c
       retval = nf_get_att_real(ncid,NF_GLOBAL,
     $  'cal_base_dbz_1km_vc', basedbzv)
       if (retval .ne. nf_noerr) call handle_err(retval)

       write (6,*)"global values", basedbzv,basedbzh,noisev,noiseh
c      read(5,*)
c
c  GET MONTH, DAY, YEAR, HOUR, MIN, SECOND
c
      retval = nf_get_var_int(ncid, base_year_varid, year)
      if (retval .ne. nf_noerr) call handle_err(retval)
c
      retval = nf_get_var_int(ncid, base_month_varid, month)
      if (retval .ne. nf_noerr) call handle_err(retval)
c
      retval = nf_get_var_int(ncid, base_day_varid, day)
      if (retval .ne. nf_noerr) call handle_err(retval)
c
      retval = nf_get_var_int(ncid, base_hour_varid, hour)
      if (retval .ne. nf_noerr) call handle_err(retval)
c
      retval = nf_get_var_int(ncid, base_min_varid, min)
      if (retval .ne. nf_noerr) call handle_err(retval)
c
      retval = nf_get_var_int(ncid, base_second_varid, sec)
      if (retval .ne. nf_noerr) call handle_err(retval)
c
      print*,"M/D/Y",month,day,year
      print*,"H/M/S", hour, min, sec
C
c      rconh = 69.1658
c      rconv = 69.8758 
c      print *,'RADAR CONSTANT H=', rconh
c      read(5,*)
       
c ELEVATION
      retval = nf_inq_varid(ncid, 'elevation_hc', elev_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
c AZIMUTH
      retval = nf_inq_varid(ncid, 'azimuth_hc', az_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
c RANGE
      retval = nf_inq_varid(ncid, 'range', range_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
c PRT
      retval = nf_inq_varid(ncid, 'prt_hc', Prt_varid)
      if (retval .ne. nf_noerr) call handle_err(retval)
c
c     Read one dimentional data
c
      retval = nf_get_var_double(ncid, Time_varid, rvp8time_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_get_var_real(ncid, elev_varid, elevation_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_get_var_real(ncid, az_varid, azimuth_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_get_var_real(ncid, Prt_varid, prt_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
c RANGE
      retval = nf_get_var_int(ncid, range_varid, range_in)
      if (retval .ne. nf_noerr) call handle_err(retval)
c
c TRANSFER TO ARRAYS THAT CAN BE PASSED TO MAIN AND SUBROUTINES
c
      do i=1,frtime
          elevation(i)=elevation_in(i)
          azimuth(i)  =azimuth_in(i)
          rvp8time(i) =rvp8time_in(i)
c     print *,"elevation, Azimuth ",i,elevation(i),azimuth(i),prt_in(i)
      enddo
      Ts = prt_in(1)
c      print *,"in initialize Ts=",prt_in(1)
      do i=1,gates
          range_ary(i)=real(range_in(i))/1000
c         write(6,*) "RANGE ",i,range_ary(i)
      enddo
c     read(5,*)
      
c
c     CONVERT UNIX TIME TO YEAR/MONTH/DAY/HOUR/MINUTE/SEC      
c
      goto 1000
      days=  float(int(rvp8time(1)))  !get whole number of days
      print *,'days',days
      utime = int(rvp8time(1))*24*60*60  !convert to seconds
      call unix2c(utime, idate)
      year = idate(1)
      month = idate(2)
      day = idate(3)
      print *, rvp8time(1),'Date =', idate
C     now get hours, minutes and seconds
      utime= int((rvp8time(1) - days)*24*60*60)
      call unix2c(utime, idate)
      print *, rvp8time(1),'Date =', idate
      hour = idate(4)
      min = idate(5)
      sec = idate(6)
c     GET FRACTIONAL SECONDS
c     get the total number of even seconds in double precision days
c     and subtract from the unix days to get fractional seconds
      total_secs= int(days)*24*60*60 + hour*60*60 + min*60 +sec
      tot_secs = dble(total_secs)
c     convert to days
      frac_secs = rvp8time(1)- (tot_secs/(24*60*60)) !in days
      frac_secs = 24*60*60*frac_secs  ! convert to secondso
      print *,'fractional seconds=', frac_secs
1000   continue
c     Set up the first ray read of I&Q values
c     count(1) = gates     ! get all gates ie, read in one ray of time series data
c     count(2) = 512      ! Get this length of time series
      count(1) = gates   ! get all gates ie, read in one ray of time series data
      count(2) = hits+1     ! Get this length of time series
c     count(2) = 512      ! Get this length of time series
       start(1)= 1        ! start at gate one
       start(2)= 1   !Start with first pulse. This will change with ray number
c                     in gettime
C     READ THE DATA.
c

c  HH
      retval = nf_get_vara_real(ncid, IHc_varid, start, count, IHc_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_get_vara_real(ncid, QHc_varid, start, count, QHc_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
c  VH
      retval = nf_get_vara_real(ncid, IVx_varid, start, count, IVx_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_get_vara_real(ncid, QVx_varid, start, count, QVx_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
c  VV
      retval = nf_get_vara_real(ncid, IVc_varid, start, count, IVc_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_get_vara_real(ncid, QVc_varid, start, count, QVc_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
c  HV
      retval = nf_get_vara_real(ncid, IHx_varid, start, count, IHx_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
      retval = nf_get_vara_real(ncid, QHx_varid, start, count, QHx_A)
      if (retval .ne. nf_noerr) call handle_err(retval)
c
C   TRANSFER DATA TO PASSABLE ARRAYS
c
       do x = 1, gates
         do y=1, hits+1
             IHc_in(x,y) = IHc_A(x,y)
             QHc_in(x,y) = QHc_A(x,y)
             IHx_in(x,y) = IHx_A(x,y)
             QHx_in(x,y) = QHx_A(x,y)
             IVc_in(x,y) = IVc_A(x,y)
             QVc_in(x,y) = QVc_A(x,y)
             IVx_in(x,y) = IVx_A(x,y)
             QVx_in(x,y) = QVx_A(x,y)
          end do
       end do
c********************
c  DO CMD ON THE RAY OF RAW DATA
       call CMD(hits,gates,maxrayno)
c*******************
c  PRINT DATA FOR INSPECTION
c
       do x = 1, gates      !gates
          do y = 1,hits   !frtime 
cc          print *,'Time, Az, Ele ',Time(start(2)+y),azimuth_in(start(2)+y),
cc    $                                  elevation_in(start(2)+y)
c           print *, 'TSHc_in(' ,x,  y, ') = ', IHc_in(x, y),QHc_in(x,y)
c           write(1,*) x,y, IHc_in(x,y),QHc_in(x,y)
          end do
c       print *,"NEXT GATE?"
c       read(*,*)
       end do
c
C     SET UP NEEDED PARAMETERS
c     range= range_in(1)/1000 !range_in is in meters
      time_inc = rvp8time((raycount-1)*hits +1)
      range= range_in(1) !range_in is in km
      range_start=1.   ! this is what the old chill timeseries used
      range_inc= 0.15
       raycount = 1
C
C     USE THE CENTER OF THE RAY DATA FOR AZIMUTH ANGLE
      azim=azimuth_in((raycount-1)*hits + hits/2)
C
      elev=elevation_in((raycount-1)*hits + hits/2)
       rng_index = 0
C     Close the file, freeing all resources.
      retval = nf_close(ncid)
      if (retval .ne. nf_noerr) call handle_err(retval)

      return
      end
c-----------------------------------------------------------
      subroutine handle_err(errcode)
      implicit none
      include 'netcdf.inc'
      integer errcode

      print *, 'Error: ', nf_strerror(errcode)
      stop 2
      end
c-----------------------------------------------------------
c     unix2c Converts Unix system time to date/time integer array.
      subroutine unix2c(utime, idate)
      implicit none
      integer utime, idate(6)
c     utime  input  Unix system time, seconds since 1970.0
c     idate  output Array: 1=year, 2=month, 3=date, 4=hour, 5=minute, 6=secs
c     -Author  Clive Page, Leicester University, UK.   1995-MAY-2
      integer mjday, nsecs
      real day
c     Note the MJD algorithm only works from years 1901 to 2099.
      mjday    = int(utime/86400 + 40587)
      idate(1) = 1858 + int( (mjday + 321.51) / 365.25)
      day      = aint( mod(mjday + 262.25, 365.25) ) + 0.5
      idate(2) = 1 + int(mod(day / 30.6 + 2.0, 12.0) )
      idate(3) = 1 + int(mod(day,30.6))
      nsecs    = mod(utime, 86400)
      idate(6) = mod(nsecs, 60)
      nsecs    = nsecs / 60
      idate(5) = mod(nsecs, 60)
      idate(4) = nsecs / 60
      end

c--------------------------------------------------------
       program chill_spectra
      character*15 ptype(10),time*12
      parameter (npulses=30000,ngates = 1000)
      common/CMDFLG/CMDflag,CMDF_RHV
       integer CMDflag(1024),CMDF_RHV(1024)
       common /nointerpvars/zdnoint,pdpnoint,rhvnoint
       real zdnoint(1024),pdpnoint(1024),rhvnoint(1024)
	common 
     +        /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
     +         /specfilt/sigmaf,xorder,nor,onoff,dc,polyopt
        common/polyfitparms/xarray,polycoef,polyorder,iauto
      real xi(1025),xq(1025),yi(1025),yq(1025),xyi(1025),xyq(1025)
     +,yxi(1025),yxq(1025)
c2345678901234567890123456789012345678901234567890123456789012345678901
c        1         2         3         4         5         6         7
       integer polyopt,polyorder
        real ui(1025),uq(1025),vi(1025),vq(1025),uvi(1025),uvq(1025),
     >  vui(1025),vuq(1025),nor(1025),extraI,extraQ
       real plfile(-5:3000,0:54),gwnd(4,4)
        real lambda,ldrh,ldrx,ldrx1,ldrx2,ldrv
	real  timsr(1:1025,0:8) 
        real xarray(1025),gain(20)
        real*8 polycoef(0:39)
         complex tmpc,tmpc1,j
        integer rtype(4),rng_index,raycount,eof
	character*6 scale(2),onoff*3,dc*3, hardxgks*70,label(0:54)*12
	character*14 wdwtype(6),singletype(4),filename*80,infile*80
        character xlab*10,ylab*10 
        integer nwidth,notchopt,lenblock,regwdth(26)
       common /rhotest/irhohv
       common /interppoly/interp,lenpinterp,regwdth,icmd,gain
        common /specnotchparms/nwidth,notchopt,lenblock,wdwtype,
     >  interp2
        common /tssaved/xi,xq,yi,yq,xyi,xyq,yxi,yxq
        common /ray/plfile,rtype
        common /labels/label
        common /infile1/filename
        common /onoff/polyon,notchon
        real range_ary(ngates)
        character*3 polyon,notchon
       real    cit(500),cib(500),tlut(50),boundary(500),
     >         var(500),avg(500)
       common /statpts/var,cit,cib,boundary,avg,maxbin,wdthsc
        common /radardata/rng,zh, zdr,ldrh,ldrv, ldrx, zv, dop, pvvhh,
     & r0hhvv, r0hhvh, r0vvhv, r2hhhh, r2vvvv, r1hvvh, r1vhhv, r0hvvh,
     &    r3hhvv, r3vvhh, phhvh, pvvhv, pdp3, phvvh, pvhhv,pdp_2lg
     &    ,wdth_hh,wdth_vh,pdpx,r1hhvv,ro11, rx_r1hv,phvvh_dp,phvvh_vel
     &    ,phidp,phhvv,cpa,cpaV,cpaUV,cpaVU,cpaUF
        common /specplot/timsr,magorph,logorlin
        common /hardcopy/hardxgks,scale
       integer year,month,day,gates,hits,hour,min,sec,maxrayno
       integer kskip,rays2skip
       common /raydat/range_inc,range,azim,elev,hour,min,time_inc,
     >  sec,year,month,day,hits,gates,rng_index,raycount,maxrayno
     >  ,range_ary
      integer trparms,tlparms,blparms,brparms,reset,scatprm,scatype(2)
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
       common /scatparms/scatarry,scatprm,scatype,npts,tlut
       common /offsets/vvhh_offset,vvhv_offset,hhvh_offset,zdroffset
       real scatarry(50000,2)
       character*80 dbname,pltdir
       common/dashed/dshv
c---
       common /FFTPACK/wsave,work,lenwrk,lensav,lenc,inc,ier
       real wsave(3000), work(3000)
      complex cts(1025) ! data
c---
      character*16 dshv(4)
      data (gwnd(i,1), i=1,4) /0.00,0.95, 0.50 ,0.99/
      data (gwnd(i,2), i=1,4) /0.00,0.95, 0.04 ,0.50/
      data dshv /'$$$$$$$$$$$$$$$$',
     +           '$$$$$$$''$$$$$$$''',
     +           '$$$$''$''''$$$$''$''''',
     +           '$''$''$''$''$''$''$''$'''/

c       data infile/"/usr/home/hubbert/DATA/"/
        data j/(0.,1.)/
c       data hardxgks
c    >/'ctrans -d ps.color -f font13 gmeta | lpr -Pps_radar'/
      data hardxgks
     >/'ctrans -d ps.color -f font12 gmeta|lp -dcolor_duplex'/
c
c------------FFTPACK parameters
       lensav = 3000
       lenwrk= 3000
       inc=1
       ier=0
       lenc=1025
c----------
        write(6,*)"ENTER RADAR FILE NAME"
c       read(5,*) filename 
        read(5,*) infile 
c       filename=infile(24: )
        filename=infile
        write(6,*) filename,infile
c       write(infile(23:),*)filename
        open(3,file="timeseries.d")
c       open(1,file=infile) !NETCDF READER OPENS THIS FILE
c      open(1,file='/usr/home/hubbert/DATA/CHL20000222_224723.asc')
c      open(1,file='/usr/home/hubbert/DATA/CHL20000307_225556.asc')
c
c open files in plt program directory
c get value of environment variable MPLT_HOME which should be set to
c point to directory where filter files etc are held
c pltdir may be different depending on what machine is in use
        print *,"before getenv"
        call getenv('MPLT_HOME',pltdir)
        print *,"after getenv"
        print *,'MPLT_HOME is <',pltdir,'>'
        print *,"after getenv"
        if(pltdir(1:3).eq.'   ') pltdir='./'
        print *,' MPLT home directory is ',pltdir
c               terminate home directory with /
        do i=1,80
           if(pltdir(i:i).eq.' ') then
                if(pltdir(i-1:i-1).ne.'/') pltdir(i:i)='/'
                goto 18
           endif
        enddo
18      continue
        print *,'appeneded:',pltdir

        open(4,file='stats.dat')
c find table in plt program directory
        call dbcat (dbname,pltdir,'t_table')
        open(8,file=dbname,status='OLD')

        do 1111 ii = 1, 45
c          read(8,*) nn, tlut(nn)
           read(8,*) n, tlut(n)
1111    continue
        close(8)
c SET UP X  array for polyfit
         do 2525 i=1,512
            xarray(i)= float(i)
2525     continue

cccc  SET UP UNIT NUMBERS FOR PLOT PARAMETERS 

        reset  = 50
        trparms= 51
        tlparms= 52
        blparms= 53
        brparms= 54
        scatprm= 59

c
cccc   SAVE THE DEFAULT VALUES FOR NCAR_GRAPHICS
c

        call agsetf('FRAME.',2.)
        call agsetf ('WINDOW.',1.)
        call agsetf('X/NICE.',0.)
        call agsetf('Y/NICE.',0.)
        xlab=""
        ylab=""
        call anotat(xlab,ylab,0,0,0,0)

c
        call agsave(reset)
        rewind(reset)
        call agsave(scatprm)
        rewind(scatprm)

        call toplprm
        call toprprm
        call botlprm
        call botrprm


cccc     FOR DISPLAYING ON xgks, Activate X-Window Workstation
      Call GOPKS(6,0)
      Call GOPWK(2,0,8)
      Call GACWK(2)

c***************
c     INITIALIZE
        iauto = 2   !auto regression, 2=no
        interp=2  !poly interp flag, 2=no
        polyorder = 3
        polyopt=2   ! NO REGRESSION FILTER
        icmd=0       !no CMD directed filtering
        irhohv=0     !no no RHOHV TEST
        wdth2=.032
        wdthsc=.024
	pi=3.1415927
        rad2deg = 180/pi
        vvhh_offset=120.0/(2*rad2deg)
        vvhv_offset=0.0
        hhvh_offset=0.0
	zdroffset=0.0
	nptype=1
	ptype(1)='SPECTRA POW'
	ptype(2)='SPECTRA phase'
	ptype(3)='TIME SERIES re'
	ptype(4)='TIME SERIES im'
	ptype(5)='TIME SERIES mag'
	ptype(6)='TIME SERIES pha'
	ptype(7)='CROSS SPECTRA m'
	ptype(8)='CROSS SPECTRA p'
	ptype(9)='1 lag SPEC pow'
	ptype(10)='1 lag SPEC phase'
	scale(1)='LINEAR'
	scale(2)='LOG'
	magorph=0
	logorlin=2
	iskip=0
	ihard=0
	itype=1
	iwdw=1
	onoff='off'
	dc='no'
        polyon='no'
        notchon='no'
        sigmaf=6.  !m/s
       xorder=3.
        notchopt=2 !DO NOT DO NOTCH
        nwidth = 5 !SPECTRUM NOTCH WIDTH. DC and 2 on each side
	print *,onoff 
      wdwtype(1)='RECTANGULAR'
      wdwtype(2)='HAMMING'
      wdwtype(3)='HANNING'
      wdwtype(4)='BARTLETT'
      wdwtype(5)='BLACKMAN'
      wdwtype(6)='BLACKMAN-NUT'
      SINGLETYPE(1)='XX'
      SINGLETYPE(2)='YY'
      SINGLETYPE(3)='XY'
      SINGLETYPE(4)='YX'
c
c*********************************
c  These interpolation widths are for a length 64 sequence
        regwdth(1)= 1     ! 1  there are interpolation width for 16 point filters (Greg)
        regwdth(2)= 3     ! 3
        regwdth(3)= 3     !3
        regwdth(4)= 5!5      !5
        regwdth(5)= 5!5     !5
        regwdth(6)= 7  !5     !5 For 3.1ms,16pt,CNR=60dB,SNR=10, 7 worked better
        regwdth(7)= 7     !7
        regwdth(8)= 7     !7
        regwdth(9)= 9 !7 9 is better for GAUS FIT    !9
        regwdth(10)= 9
        regwdth(11)= 9
        regwdth(12)= 11
        regwdth(13)= 11
        regwdth(14)= 11
        regwdth(15)= 13
        regwdth(16)= 13
        regwdth(17)= 15
        regwdth(18)= 15
        regwdth(19)= 15
        regwdth(20)= 17
        regwdth(21)= 19
        regwdth(22)= 19
        regwdth(23)= 19
        regwdth(24)= 21
        regwdth(25)= 21
        regwdth(26)= 21
        gain(1)  = 1.
        gain(2)  = 1.
        gain(3)  = 1.003
        gain(4)  = 1.003
        gain(5)  = 1.005
        gain(6)  = 1.005
        gain(7)  = 1.0057
        gain(8)  = 1.0057
        gain(9)  = 1.006
        gain(10)  =1.01
        gain(11)  =1.01
        gain(12)  =1.015
        gain(13)  =1.02
        gain(14)  = 1.02
        gain(15)  = 1.02
        gain(16)  = 1.02
        gain(17)  = 1.025
        gain(18)  = 1.03
        gain(19)  = 1.03
        gain(20)  = 1.04   !did not check this value
c
cccc  SET UP LABEL ARRAY
        label(0)='Range'
        label(1)='Zh '
        label(2)='Zdr'
        label(3)='LDRh '
        label(4)='LDRv'
        label(5)='LDR_X'
        label(6)='Zv'
        label(7)='Veloc.'
        label(8)='Phidp'
        label(9)='r0hhvv'
        label(10)='r0hhvh'
        label(11)='r0vvhv'
        label(12)='r2hhhh'
        label(13)='r2vvvv'
        label(14)='r1hvvh'
        label(15)='r1vhhv'
        label(16)='r0hvvh'
        label(17)='r3hhvv'
        label(18)='r3vvhh'
        label(19)='Range'
        label(20)='Phhvh'
        label(21)='Pvvhv'
        label(22)='Phidp_3'
        label(23)='Phvvh'
        label(24)='Pvhhv'
        label(25)='PDP_2lags'
        label(26)='Wdth_hh'
        label(27)='Wdth_vh'
        label(28)='r0hhvvB'
        label(29)='PDP_x'
        label(30)='r1hhvv'
        label(31)='RX/RC'
        label(32)='PDP_300'
        label(33)='PDP_300ma'
        label(34)='RHV_300'
        label(35)='RHV_300ma'
        label(36)='PDP_300ma2'
        label(37)='PDP_300ma3'
        label(38)='Phvvh_dp'
        label(39)='phvvh_vel'
        label(40)='phhvv'
        label(41)='pvvhh'
        label(42)='co_dop_deg'
        label(43)='x_dop_deg'
        label(44)='LDR_tr1'
        label(45)='LDR_tr2'
        label(46)='tilt'
        label(47)='ellip'
        label(48)='VH/HV_dB'
        label(49)='RAW_POW'
        label(50)='POW_HH'
        label(51)='CPA_UF'
        label(52)='POW_H3V'
        label(53)='A_Order'
        label(54)='CMD_F'
c
c
c Initial ray plot types
c
        rtype(1)=1
        rtype(2)=2
        rtype(3)=3
        rtype(4)=9
c
c  GET TIME SERIES
c
       print *,"CALLING initialize_timeseries"
c
33330  call initialize_timeseries ! ONLY CALLED AT BEGINNING AND AT REWIND
c
       lenblock=nn  !THIS IS THE LENGTH FOR POLYFIT. CAN BE CHANGED BELOW
C
C  INITIALIZE FFTPACK FOR NEW SEQUENCE LENGTH
C
c      CALL INITILIZE FFTPACK. Arbitrary lengths
       call  CFFT1I(nn, WSAVE, LENSAV, IER)
c
33333  print *,"GETTING DATA, time series length=",nn
C      THIS IS WHERE ALL GATES ARE RETRIEVED AFTER THE FIRST GATE
C      WHICH IS RETRIEVE IN SUBROUTINE initialize_timeseries
C
       call gettime(eof)  
C
c*******************************
       do 99 i=1,nn
        write(6,*) vi(i),vq(i)!,uvi(i),uvq(i),ui(i),uq(i),vui(i),vuq(i)
99      continue
C THIS IS NOT NEDDED SINCE FFTPACK HANDLES ARBITRARY LENGTHS
c*******calculate log2(n)
c       m=0
c        n=nn
c       do 3 i=1,30
c           m=m+1
c           nn=nn/2
c           if(nn.eq.1) mm=m
c3      continue
c        n=2**mm
c        nn=n
c        m=mm
c        print *,"N,M ",n,mm

c***************
c.. SAVE A COPY OF THE TIMSERIES
33334   continue
        do 8 i=1,nn
          xi(i)=ui(i)
          xq(i)=uq(i)
          yi(i)=vi(i)
          yq(i)=vq(i)
	  xyi(i)=uvi(i)
	  xyq(i)=uvq(i)
	  yxi(i)=vui(i)
	  yxq(i)=vuq(i)
8        continue
9        continue
c    
       call process
c
	lambda=.11   ! meters
c      Ts=.001 ! THE PRT IS READ FROM NETCDF FILE INTO Ts
       call calculate
ccc
c     LOAD THE TIME SERIES FILES
c
	call loadtseries(nptype,ptype,np)
c
10000   continue 
        print *, "Ts, maxrayno=",Ts,maxrayno
        print *, "Range index",rng_index," Ray count",raycount
        print *, "RANGE", range,"km"
        print *, "Zh, Zv,PH ", zh,zv,plfile(rng_index,49)
        print *, "CPA = ", cpa, 'PRT= ',Ts
        print *,"RTYPE=",rtype(1),rtype(2),rtype(3),rtype(4)
        print *,"jj=",jj,"rng_index=",rng_index
        print *,"R1hhvv= ",r1hhvv
        print *,"r2hhhh= ",r2hhhh," r2vvvv= ",r2vvvv
        print *,"r1hvvh= ", r1hvvh," r1hhhh= ",ro11**.25
        print *,"r1vhhv= ",r1vhhv, " r1hhhh= ",ro11**.25
        print *, " r0hvvh = ", r0hvvh
        print *,"r3hhvv, r3vvhh= ",r3hhvv,r3vvhh
        print *,"PDP_3= ",pdp3," Phidp= ",pvvhh
        write(6,311) year,month,day,hour,min,sec
311     format("DATE ",i4,1x,i2,1x,i2," TIME ",i2,":",i2,":",i2)
        write(6,312)range,range_inc,gates,hits
312     format("RNG ",f9.4,2x,"BIN LGTH ",f6.4,2x,"GATES ",i4,2x,
     >  "HITS ", i4)
         Write(6,313) azim,elev
313     format("AZIMUTH ",f7.3,2x,"ELEV. ",f7.3)
           
	write(6,315) r0vvhv,r0hhvh,pvvhv,phhvh
315     format("CROSS PRAMETERS r0vvhv ",f6.3,1x,"rhhvh",f6.3,1x,
     >        "pvvhv",f8.3,1x,"phhhv",f8.3)
        write(6,308) dop,pvvhh,r0hhvv,wdth_hh,wdth_vh
308     format('VEL.',f6.1,' D-PHASE',f7.1,' ROhv ',
     >          f7.4,' WDTHhh ',f5.2,' WDTH_vh ',f5.2)
        write(6,309) r0hvvh, ldrh, ldrx, rx_r1hv
309     format("R0hvvh ",f7.4,1x," LDRh ",f8.3,1x,"LDRx ",f8.3,
     >  " LDR_rx_r1hv ",f8.3)
        write(6,*)"Pxh+Pxv=pdp ",pdpx
        print *, '----- '
        print *,'<<<<<<<<< SPECTRA MENU >>>>>>>>>'
       write(6,12) wdwtype(iwdw),singletype(itype),ptype(nptype),
     +	onoff,sigmaf,xorder,dc,nn,polyorder,nwidth,icmd
12    format(/'SELECT AD OPTIONS:'/
     x	     3X, ' 0. (RETURN) NEXT BIN.'/
     x       3X, ' 1. REWIND.'/
     x       3X, ' 2. SKIP BINS.'/
     x       3x, ' 3. WINDOW TYPE IS >>>',a14/
     x	     3X, ' 4. SINGLE SPECTRA TYPE IS >>>',a14//
     x	     3X, ' 5. PLOT.'/
     x	     3X, ' 6.    CURRENTLY PLOTTING >>>> ',a14 //
c    x	     3X, ' 6. MAG. or PHASE.'/
     x	     3X, ' 7. PLOT SINGLE SPECTRUM.'/
     x	     3X, ' 8. LOG or LINEAR.'/
     x	     3X, ' 9. HARD COPY.'/
     x	     3X, '10. FILTERING IS ',a3,'.'/
     x	     3X, '11. WIDTH FOR FILTER.',f5.1,'m/s'/
     x	     3X, '12. ORDER FOR EXP. FILTER.',f4.1/
     x       3x, '13. REMOVE DC?  <<',a3,'>>'/
     x	     3X, '14. RAY PLOT MENU'/
     x	     3X, '15. CHANGE SEQUENCE LENGTH, now= ',i4/
     x	     3X, '16. PHASE OFFSETS'/
     x       3x, '17. POLY Removal. Polyorder= ',i3/
     x       3x, '18. SPECTRUM NOTCH= ',i3/
     x       3x, '19. CMD ON/OFF= ',i3/
     x       3x, '20. HARD COPY'/
     x       3x, '21. OUTPUT VOLUME'/
     x       3x, '22. QUIT.'/
     x         "$OPTION?    ")
       iopt = ttinp(0)
       if(iopt.gt.22.or.iopt.lt.0) goto 10000
      if (iopt .eq. 0) goto 33333
      goto (100,200,300,400,500,600,700,800,900,1000,1100,
     +1200,1300,1400,1500,1600,1700,1800,1900,2000,2100,2200),iopt  
c...........................................
100   continue
      rewind(1)
        eof=0
        raycount=0
        jj=0
      goto 33330
c............................................
200   continue
       write(6,*)'ENTER:   0=RETURN'
       write(6,*)'ENTER:   1= Skip bins',rng_index
       write(6,*)'         2= Skip to a ray',raycount
       iopt = ttinp(0)
       if (iopt.eq.0) goto 10000
       if(iopt.lt.1.or.iopt.gt.2) goto 200
       igo=iopt
       if (igo.eq.1) then
          write(6,*)'ENTER GATES TO SKIP'
          read(5,*,err=200) iskip
          kskip=rng_index + iskip
            If(iskip.eq. 0) goto 10000
            if(kskip.gt.gates) goto 200
            if (kskip.gt. 0) then
               rng_index = kskip-1
               jj=gates-rng_index
               goto 33333
            else
              Print*, 'NEGATIVE GATE REQUESTED'
              PRINT*, 'USE RAY COUNT TO GO BACKWARDS'
             goto 200
            endif
       else 
C
C  RAY SKIP SECTION
c      if(iopt.eq.2) then
          write(6,*)'ENTER THE RAY NUMBER'
           iopt = ttinp(0)
           if(iopt.eq.raycount) goto 10000
           if(iopt.lt.1.or.iopt.gt.maxrayno) then
               print*,'EXCEEDED MAX RAY COUNT'
               goto 200
           endif
           raycount=iopt-1  ! RAY COUNT INCREMENTED IN GETTIME
           rng_index=0
           jj=0    ! this tells gettime to get next ray of data
           goto 33333
      endif
c     return
c............................................
300    continue
	write(6,11)
11    format(/'SELECT WINDOW TYPE:'/
     x       3X, '1. RECTANGULAR.'/
     x       3x, '2. HAMMING.'/
     x	     3X, '3. HANNING.'/
     x	     3X, '4. BARTLETT.'/
     x	     3X, '5. BLACKMAN.'/
     x	     3X, '6. BLACKMAN NUT.'/
     x           '$OPTION?    ')
       iopt = ttinp(0)
       if(iopt.lt.1.or.iopt.gt.6) goto 300
       if(iopt.eq.iwdw) then
           goto 10000
       else
          print *,"BEFORE WINDOW SELECT.nn=",nn
          do 350 i=1,nn
c            print *,"save", xi(i),xq(i)
             ui(i)=xi(i)
             uq(i)=xq(i)
             vi(i)=yi(i)
             vq(i)=yq(i)
             uvi(i)=xyi(i)
             uvq(i)=xyq(i)
             vui(i)=yxi(i)
             vuq(i)=yxq(i)
350        continue
       endif

      iwdw=iopt
      goto 9
c............................................
400    continue
	write(6,401)
401    format(/'SELECT TIME SERIES:'/
     x       3X, '1. xx.'/
     x       3x, '2. yy.'/
     x	     3X, '3. xy.'/
     x	     3X, '4. yx.'/
     x	     '$OPTION?    ')
       iopt = ttinp(0)
       if(iopt.lt.1.or.iopt.gt.4) goto 400
      itype=iopt
      goto 10000
c..............................................
500    continue
      ihard=0
      print *,"BEFORE PLOT TS. length=",np
      call plotspec(ptype,nptype,ihard,np)
      open(18,file='realparts.txt')
      open(19,file='imagparts.txt')
      write(19,*) nn,range_ary(rng_index),azim,elev
      write(18,*)nn,range_ary(rng_index),azim,elev
      do 505, i=1,nn
          write(18,*)i*Ts*1000,ui(i),vi(i)
          write(19,*)i*Ts*1000,uq(i),vq(i)
505   continue
      close(18)
      close(19)
      goto 10000
c..............................................
600    continue
	print *,'SELECT PLOT TYPE'
	print *, '0. (RETURN) NO CHANGE'
	print *, '1. SPECTRA mag'
	print *, '2. SPECTRA phase'
	print *, '3. TIME SERIES real'
	print *, '4. TIME SERIES imag'
	print *, '5. TIME SERIES mag'
	print *, '6. TIME SERIES pha'
	print *, '7. CROSS SPECTRA mag'
	print *, '8. CROSS SPECTRA pha'
        print *, '9. FIRST LAG SPECTRUM MAGNITUDE '
        print *, '10. FIRST LAG SPECTRUM PHASE'
c	print *, '8. ShhSvv* both'
       iopt = ttinp(0)
       if(iopt.lt.0.or.iopt.gt.10) goto 600
       if(iopt.eq.0)  goto 10000
       nptype=iopt
c      goto (10,20,30,40,50),iopt
c0        ptype(
c      write(6,*)'ENTER:   0=MAGNITUDE'
c      write(6,*)'         1=PHASE'
c      iopt = ttinp(0)
c      if(iopt.lt.0.or.iopt.gt.1) goto 600
c      magorph=iopt
	call loadtseries(nptype,ptype,np)
       
       goto 10000
c..............................................
700    continue
c     call singlespec
      goto 10000
c..............................................
800    continue
       write(6,*)'ENTER:   1=LINEAR'
       write(6,*)'         2=LOG'
       iopt = ttinp(0)
       if(iopt.lt.1.or.iopt.gt.2) goto 800
       if(logorlin.eq.iopt) goto 10000
       logorlin=iopt
       Print *,"at loglin opt. np=",np
       call loglin(np)
       goto 10000
c............................................
900    continue
       ihard=1
      call plotspec(ptype,nptype,ihard,np)
      ihard=0
      goto 10000
c............................................
1000   continue
       if(onoff.eq.'on') then
	   onoff='off'
       else
	   onoff='on'
       endif
        do 1001 i=1,nn
	  ui(i)=xi(i)
	  uq(i)=xq(i)
	  vi(i)=yi(i)
	  vq(i)=yq(i)
	  uvi(i)=xyi(i)
	  uvq(i)=xyq(i)
	  vui(i)=yxi(i)
	  vuq(i)=yxq(i)
1001    continue
      goto 9
c............................................
1100   continue
       write(6,*)'ENTER WIDTH'
       read(5,*,err=1100) sigmaf
        do 1101 i=1,nn
          ui(i)=xi(i)
          uq(i)=xq(i)
          vi(i)=yi(i)
          vq(i)=yq(i)
          uvi(i)=xyi(i)
          uvq(i)=xyq(i)
          vui(i)=yxi(i)
          vuq(i)=yxq(i)
1101    continue

       goto 9
c............................................
1200    continue
       write(6,*)'ENTER ORDER FOR EXPONETIAL'
       read(5,*,err=1200) xorder
        do 1201 i=1,nn
          ui(i)=xi(i)
          uq(i)=xq(i)
          vi(i)=yi(i)
          vq(i)=yq(i)
          uvi(i)=xyi(i)
          uvq(i)=xyq(i)
          vui(i)=yxi(i)
          vuq(i)=yxq(i)
1201    continue

      goto 9
c..............................................
1300   continue
       write(6,*)'ENTER:   1=No Filter'
       write(6,*)'         2=Remove DC'
       write(6,*)'         3=Remove Poly Fit'
       write(6,*)'         4=Turn off Poly Fit'
       iopt = ttinp(0)
       if(iopt.lt.1.or.iopt.gt.4) goto 1300
       polyopt=iopt

       if(iopt.eq.2) then
           dc='yes'
       endif
       if(iopt.eq.1) then
          dc='no'
       endif
        do 1301 i=1,nn
          ui(i)=xi(i)
          uq(i)=xq(i)
          vi(i)=yi(i)
          vq(i)=yq(i)
          uvi(i)=xyi(i)
          uvq(i)=xyq(i)
          vui(i)=yxi(i)
          vuq(i)=yxq(i)
1301    continue

       goto 9
c..............................................
1400   continue
c
c GO TO RAY PLOT MENU     
C GO THROUGH THE ENTIRE CURRENT RAY OF DATA TO CALCULATE RADAR VARIABLES
c
1401   continue
c
C     SET JJ and  RNG_INDEX TO BEGINNING OF RAY
      jj=gates
      rng_index = 0
      do 666 k=1, gates
        call gettime(eof)
        call process
        call calculate
c     print *,'IN RAY LOOP. rng_ind,range' rng_index,
c    > range_ary(rng_index)
666   continue
        print *,"RTYPE",rtype(1),rtype(2),rtype(3),rtype(4)
        print *,"nn=",nn         ! TIME SERIES LENGTH
c       if(rtype(1).ne.1) read(*,*)
c       goto 1401
c     print *,"BEFORE SHORT PULSE CALL"
c     call shortpulse(1) 
c     call shortpulse(2) 
c     print *,"BEFORE RAYMENU  CALL"
      call raymenu(gates)
c
c     RETURN FROM RAY PLOT. GET GATE 1 TIME SERIES
c
      jj=gates
      rng_index = 0
      goto 33333
c.........................................
1500  continue
      write(6,*) "ENTER NEW SEQUENCE LENGTH"
      write(6,*)"length= ", nn
      read(5,*,err=1500) nn
      goto 9
1600  continue
        write(6,1601) vvhh_offset*rad2deg,vvhv_offset*rad2deg,
     >   hhvh_offset*rad2deg, zdroffset
1601    format(/'SELECT AN OPTION:'/
     x       3X, ' 0. (RETURN) NEXT BIN.'/
     x       3X, ' 1. P_VVHH OFFSET.',f8.2/
     x       3X, ' 2. P_VVHV OFFSET.',f8.2/
     x       3X, ' 3. P_HHVH OFFSET.',f8.2/
     x       3X, ' 4. ZDR    OFFSET',f8.2/
     x       '$OPTION?    ')
       iopt = ttinp(0)
       if(iopt.gt.4) goto 10000
      if (iopt .eq. 0) goto 9
      goto (1610,1620,1630,1640),iopt
1610  write(6,*) 'ENTER P_VVHH offset' 
      read(5,*,err=1610) vvhh_offset
      vvhh_offset=vvhh_offset/(2*rad2deg)
      goto 1600
1620  write(6,*) 'ENTER P_VVHV offset' 
      read(5,*,err=1620) vvhv_offset
      vvhv_offset=vvhv_offset/rad2deg
      goto 1600
1630  write(6,*) 'ENTER P_HHVH offset' 
      read(5,*,err=1630) hhvh_offset
      hhvh_offset=hhvh_offset/rad2deg
      goto 1600
1640  write(6,*) 'ENTER ZDR offset' 
      read(5,*,err=1640) zdroffset
      goto 1600
c-----------------------------------
1700   continue
       write(6,*)'ENTER:   0=RETURN'
       write(6,*)'         1=Poly Fit. Option: on=1, off=2 ',polyopt
       write(6,*)'         2=Change Poly order',polyorder
       write(6,*)'         3=Change Poly block length',lenblock
       write(6,*)'         4=Interpolate? 1=Y, 2=N',interp
       write(6,*)'         5=Auto Order Select? 1=Y, 2=N',iauto
       iopt = ttinp(0)
       if(iopt.eq.0) goto 10000
       goto(1701,1702,1703,1704,1705) iopt
1701   continue
       write(6,*) "ENTER OPTION. on=1,off=2"
       read(5,*,err=1701)ip
       if(ip.lt.1.or.ip.gt.2) goto 1701
       if (polyopt.eq.ip) goto 10000
       polyopt=ip
          do 1750 i=1,nn
          ui(i)=xi(i)
          uq(i)=xq(i)
          vi(i)=yi(i)
          vq(i)=yq(i)
          uvi(i)=xyi(i)
          uvq(i)=xyq(i)
          vui(i)=yxi(i)
          vuq(i)=yxq(i)
1750    continue
         if(polyopt.eq.1)  polyon='yes'
         if(polyopt.eq.2)  polyon='no'
        goto 9
1702    continue
        write(6,*) "ENTER POLYNOMIAL ORDER"
        read(5,*,err=1702)ip
        if(ip.lt.1.or.ip.gt.39) goto 1702
        if(ip.eq.polyorder) goto 10000
        polyorder=ip
        do 1751 i=1,nn
          ui(i)=xi(i)
          uq(i)=xq(i)
          vi(i)=yi(i)
          vq(i)=yq(i)
          uvi(i)=xyi(i)
          uvq(i)=xyq(i)
          vui(i)=yxi(i)
          vuq(i)=yxq(i)
1751    continue
        goto 9
1703    continue
        write(6,*) "ENTER BLOCK LENGTH",lenblock
           write(6,*) "The sequence length must be divisible"
           write(6,*) "by  the block length"
           read(5,*,err=1703) ip
           if(ip.eq.lenblock) goto 10000
           If(mod(nn,lenblock).ne.0) goto 1703
         lenblock = ip
        do 1752 i=1,nn
          ui(i)=xi(i)
          uq(i)=xq(i)
          vi(i)=yi(i)
          vq(i)=yq(i)
          uvi(i)=xyi(i)
          uvq(i)=xyq(i)
          vui(i)=yxi(i)
          vuq(i)=yxq(i)
1752    continue
        goto 9
1704    continue
        write(6,*) "ENTER INTERP OPTION:
     > Linear=1 NO=2, GAUSS=3  ",interp
        read(5,*,err=1704) ip
        if(ip.lt.1.or.ip.gt.3) goto 1704
        if(ip.eq.interp) goto 10000
        interp=ip
        do 1753 i=1,nn
          ui(i)=xi(i)
          uq(i)=xq(i)
          vi(i)=yi(i)
          vq(i)=yq(i)
          uvi(i)=xyi(i)
          uvq(i)=xyq(i)
          vui(i)=yxi(i)
          vuq(i)=yxq(i)
1753    continue
        goto 9
1705    continue
        write(6,*) "TURN ON/OFF AUTO POLY ORDER?" 
        write(6,*) "SELECT: 1=YES, 2=No", iauto
          read(5,*,err=1705) ip 
          if(ip.lt.1.or.ip.gt.2) goto 1705
          if(iauto.eq.ip) goto 10000
          iauto=ip
        do 1755 i=1,nn
          ui(i)=xi(i)
          uq(i)=xq(i)
          vi(i)=yi(i)
          vq(i)=yq(i)
          uvi(i)=xyi(i)
          uvq(i)=xyq(i)
          vui(i)=yxi(i)
          vuq(i)=yxq(i)
1755    continue
        goto 9
c*****
c---------------------------
1800    continue
       write(6,*)'ENTER:   0=RETURN'
       write(6,*)'         1=Do SPECTRUM NOTCH'
       write(6,*)'         2=Turn off NOTCH'
       write(6,*)'         3=Change NOTCH WIDTH',nwidth
       write(6,*)'         4=Interpolate? 1=Y, 2=N',interp2
       iopt = ttinp(0)
       if(iopt.eq.0) goto 10000
       if(iopt.lt.0.or.iopt.gt.4) goto 1800
c      if(notchopt.eq.iopt) goto 10000
       if(iopt.eq.3) then
1802       write(6,*) "Enter NOTCH WIDTH (odd). Now = ", nwidth
           iorder= ttinp(0)
           if(iorder.lt.1.or.iorder.gt.19) then
              write(6,*) "width must be 1 to 19 (odd)"
              goto 1802
           endif
           nwidth=iorder
       endif
       if (iopt.eq.1 .or. iopt.eq.2) then
         notchopt=iopt
         if(iopt.eq.1) then
            notchon='yes'
         else
            notchon='no'
         endif
       endif
       if(iopt.eq.4) then
1805     write(6,*) 'INTERPOLATE 0 Vel NOTCH? ENTER: 1=yes, 2=no'
         read(5,*,err=1805)  interp2
         if(interp2.ne.1.and.interp2.ne.2) goto 1805
       endif
c
        do 1801 i=1,nn
          ui(i)=xi(i)
          uq(i)=xq(i)
          vi(i)=yi(i)
          vq(i)=yq(i)
          uvi(i)=xyi(i)
          uvq(i)=xyq(i)
          vui(i)=yxi(i)
          vuq(i)=yxq(i)
1801    continue
       goto 9
cc1800    continue
cc     goto (1610,1620,1630,1640),iopt  write(6,*)'ENTER:   0=RETURN'
cc       write(6,*)'         1=Do SPECTRUM NOTCH'
cc       write(6,*)'         2=Turn off NOTCH'
cc       write(6,*)'         3=Change NOTCH WIDTH',nwidth
cc       iopt = ttinp(0)
cc       if(iopt.eq.0) goto 10000
cc       if(iopt.lt.0.or.iopt.gt.3) goto 1800
cc       if(notchopt.eq.iopt) goto 10000
cc       if(iopt.eq.3) then
cc1802       write(6,*) "Enter NOTCH WIDTH (odd). Now = ", nwidth
cc           iorder= ttinp(0)
cc           if(iorder.lt.1.or.iorder.gt.19) then
cc              write(6,*) "oder must be 1 to 19 (odd)"
cc              goto 1802
cc           endif
cc           nwidth=iorder
cc       else
cc         notchopt=iopt
cc         if(iopt.eq.1) then
cc            notchon='yes'
cc         else
cc            notchon='no'
cc         endif
cc       endif
cc        do 1801 i=1,nn
cc          ui(i)=xi(i)
cc          uq(i)=xq(i)
cc          vi(i)=yi(i)
cc          vq(i)=yq(i)
cc          uvi(i)=xyi(i)
cc          uvq(i)=xyq(i)
cc          vui(i)=yxi(i)
cc          vuq(i)=yxq(i)
cc1801    continue
cc       goto 9
1900   continue !CMD ON/OFF
        write(6,1901) icmd,irhohv
1901    format(/'SELECT AN OPTION:'/
     x       3X, ' 0. (RETURN) '/
     x       3X, ' 1. CMD on =1 ', i3/
     x       3X, ' 2. RHOhv TEST on. ',i3/
     x       '$OPTION?    ')
       iopt = ttinp(0)
       if(iopt.eq.0) goto 10000
       if(iopt.lt.0.or.iopt.gt.2) goto 1900
       goto (1910,1920),iopt       
1910   write(6,*) "ENTER CMD FLAG (on=1, off=0)"
       read(5,*,err=1900) iop
       if(iop.lt.0.or. iop.gt.1) then
          write(6,*)"BAD NUMBER"
          goto 1900
       else
          icmd=iop
          goto  9
       endif
1920   write(6,*) "ENTER RHOHV TEST FLAG, (on=1, off=0)"
       read(5,*,err=1900) iop
       if(iop.lt.0.or. iop.gt.1) then
          write(6,*)"BAD NUMBER"
          goto 1900
       else
          irhohv =iop
          if(irhohv.eq.1) then
             CMDflag=max(CMDflag,CMDF_RHV)
          else
             do 1930 i=1,gates
              if(CMDF_RHV(i).eq.1) CMDflag(i)=0.0
1930         continue
          endif
          goto  9
       endif
2000   continue
       ihard=1
       call plotspec(ptype,nptype,ihard,np)
       goto 10000
2100   continue
       open(90,file='outvolume.txt')
       write(90,*) filename ! this is the filename
       write(90,*) "No. OF RAYS = ",maxrayno,
     > "No. OF GATES = ", gates
       write(90,*)" Rng    Zh    Zdr    Vel    Phidp      Rohv    Width
     >    Order    CMD_Flg"
C     SET JJ and  RNG_INDEX TO BEGINNING OF RAY
      jj=gates
      rng_index = 0
c
c RAY LOOP
c
      do 2140 irayno=1,maxrayno
c
c  GATE LOOP
c
      do 2150 k=1, gates
        call gettime(eof)
        call process
        call calculate
2150  continue
c
c  FULL RAY OF DATA HAS BEEN FILLED.
C  NOW OUTPUT DATA
c
C  FIRST WRITE RAY HEADER
c
       write(90,*) "az= ",azim, " elev= ",elev
c  WRITE RAY DATA
       do  2160 i=1,gates
        write(90,2170) plfile(i,19),plfile(i,1),plfile(i,2),plfile(i,7),
     >  plfile(i,8),plfile(i,9),plfile(i,26),plfile(i,53),
     >  plfile(i,54) 
2160   continue
2140   continue
       close(90)
       goto 10000
2170  format(f7.3,1x,f7.2,1x,f7.3,1x,f6.2,1x,f8.2,1x,f6.3,1x,f7.2,1x,
     > f5.1,1x,f5.1) 
2200   continue
	end
c-------------------------
C CMD FLAG SUBROUTINES
cc----------------------------------------------------------------
      subroutine CMD(hits,gates,maxrayno)
      integer hits,gates,maxrayno,CMDflag(1024),CMDF_RHV(1024)
      real rhvNF(1024),rhvFLT(1024),wght(6)
      data wght/1.,.5,.3333333,.25,.2,.16666666/
      common /RHVtest/rhvNF,rhvFLT
       common /rhotest/irhohv
      real combint(1024),impfac
      common/CMDFLG/CMDflag,CMDF_RHV
      kerZ=9
      kerSPIN=11
      kerZDR=7
      call CMDvars(hits,gates,maxrayno)
      call getTDBZ(gates,kerZ) !kernal is 9
      call getSPIN(gates,kerSPIN)
      call TZDRTPHI(gates,kerZDR)
      call getinterests(gates,kerZ,kerSPIN,kerZDR)   !membership functions applied here
C     get max of SPIN and TDBZ
      call combinterest(gates,kerZDR,combint)   
C   apply thresshold here and create CMD flag array.
      thres=.499
      do 10 i=1 +kerZDR/2, gates-kerZDR/2 
        if(combint(i).gt.thres) then
           CMDflag(i)=1
        else
           CMDflag(i)=0
        endif
c       write(6,*) "CMD_conbint ",combint(i)
10     continue
C THE FOLLOWING IS BASED ON kerZDR=7
      CMDflag(1)=1  !gates close to the radar always should be filtered
      CMDflag(2)=1
      CMDflag(3)=1
      CMDflag(gates)=CMDflag(gates-3)
      CMDflag(gates-1)=CMDflag(gates-3)
      CMDflag(gates-2)=CMDflag(gates-3)
C     CMDflag is set for all gates
c
c  DO MIKE'S INFILL ALGORITHM
c
c      kinfil=6
c      xnorm= 1. + .5 + 1./3. + .25 + .2 + 1./6.
c      do 500 k= 1+kinfil, gates-kinfil
c         if(CMDflag(k).eq.0) then
c         sumb=0.0
c         suma=0.0
c         do 510 ik=k-kinfil,k-1
c           suma=suma+wght(ik)*cmdFLAG(ik)
c510   continue
c         do 511 ik=k+1,k+kinfil
c           sumb=sumb+wght(ik)*cmdFLAG(ik)
c511   continue
c      sumb=sumb/xnorm
c      suma=suma/xnorm
c      if((suma.ge.0.35).and.(sumb.ge.0.35)) CMDflag(k)=1
c      endif       
c500   continue
c
c   END INFILL
c
c DO RHOHV TEST
c
      CMDF_RHV=0.0
      do 100 i=1,gates
         if((rhvNF(i).lt.0.98).AND.(rhvFLT(i).gt.0.5)
     >     .AND.(CMDflag(i).eq.0)) then !CMDflag=0 means only test gates not filtered
         xnum=max(1.-rhvNF(i), 0.)
         xden=max(1.-rhvFLT(i),.005)
         impfac=xnum/xden  
            if(impfac.ge.3.) then
c              CMDflag(i) = 1
               CMDF_RHV(i) =1
               if(irhohv.eq.1) CMDflag(i)=1
            endif
c      write(6,*)"NUM ",xnum,1.-rhvNF(i)
c      write(6,*)"DEN ",xden,1.-rhvFLT(i)
c      write(6,*)"impfac = ",impfac
       endif
100    continue
c
c  DO MIKE'S INFILL ALGORITHM
c
      kinfil=6
      xnorm= 1. + .5 + 1./3. + .25 + .2 + 1./6.
c
      do 600 irepeat=1,2
      do 500 k= 1+kinfil, gates-kinfil
c
         if(CMDflag(k).eq.0) then
         sumb=0.0
         suma=0.0
         do 510 ik=1,kinfil
           suma=suma+wght(ik)*cmdFLAG(k+ik)
510   continue
         do 511 ik=1,kinfil
           sumb=sumb+wght(ik)*cmdFLAG(k-ik)
511   continue
      sumb=sumb/xnorm
      suma=suma/xnorm
      write(6,*)"INFILL ", k,sumb,suma,xnorm,CMDflag(k)
      if((suma.ge.0.3).and.(sumb.ge.0.3)) then 
         CMDflag(k)=1
         write(6,*) "CMDflag=",CMDflag(k)
      endif
      endif
500   continue
600   continue
c
c   END INFILL

      return
      end
c-------------------------------
c   THIS SUBROUTINE GETS dBZ, ZDR, PHIDP and CPA for 
c      CALCULATING CMD interest fields
c
       subroutine CMDvars(hits,gates,maxrayno)
       parameter (ngates=1000, npulses = 30000)
       integer hits, gates, maxrayno,pulses
       real elevation(npulses), azimuth(npulses)
       double precision rvp8time(npulses)
       complex tmpc, tmpc1, tmpc2,j
       data pi/3.1415926/
       common /rhotest/irhohv
       common/netcdf/elevation,azimuth,rvp8time,pulses,IHc_in,QHc_in,
     > IVc_in,QVc_in,IVx_in,QVx_in,IHx_in,QHx_in
       common/CMDvar/ ZCMD,zdrCMD,pdpCMD,cpaCMD
       common /offsets/vvhh_offset,vvhv_offset,hhvh_offset,zdroffset
       common /features/TDBZ,TZDR,TPHI,spinCMD
       real TDBZ(1024),TZDR(1024),TPHI(1024),spinCMD(1024)
      real IHc_in(ngates,1024), QHc_in(ngates,1024)
      real IVx_in(ngates,1024), QVx_in(ngates,1024)
      real IVc_in(ngates,1024), QVc_in(ngates,1024)
      real IHx_in(ngates,1024), QHx_in(ngates,1024)
      real ui(1025),uq(1025),vi(1025),vq(1025),uvi(1025),uvq(1025),
     >  vui(1025),vuq(1025),nor(1025)
      real ZCMD(1024),zdrCMD(1024),pdpCMD(1024),cpaCMD(1024)
      real rhvNF(1024),rhvFLT(1024)
      common /RHVtest/rhvNF,rhvFLT
c--------------------------------
c DOUBLE PRESCISION FOR POLY FIT SUBROUTINE
c
       real*8 Hci(1025),Hcq(1025),Vci(1025),Vcq(1025)
      real*8 ax(1025),y(1025),polyI(1025),polyR(1025),v1(1025),v2(1025)
     >    ,dd,e1,polycoef(0:39)
       integer polyorder
c-----------------------------------
c     let  polyorder=5 (see below)  for RHOHV test. Only used where CMDflag=0
      e1=dble(0.0)
      nn=hits
c
C  SET GENERIC X ARRAY TO PASS TO POLYFIT ROUTINE
        do 1 i=1,nn
         ax(i) = dble(1.*i)
1       continue
c
       j= cmplx(0.,1.)
      rad2deg=180./pi
c
C  CALCULATE THE NEEDED RADAR VARIABLES
C
       do 10 i=1,gates
         do 20 k=1,hits
           ui(k) = IHc_in(i,k)
           uq(k) = QHc_in(i,k)
           vq(k) = QVc_in(i,k)
           vi(k) = IVc_in(i,k)
c          write(6,*) "TS ",i,k,IHc_in(i,k), QHc_in(i,k)
20     continue
c      read(5,*)
c
C GET Z AND ZDR
          call xcor0(ui,uq,ui,uq,nn,g,h)
          call xcor0(vi,vq,vi,vq,nn,e,f)
         ZCMD(i)= 10*log10(g)
         zdrCMD(i)= 10*log10(g/e)
         varu=g
         varv=e
c GET PHIDP
        call xcor0(vi,vq,ui,uq,nn,a,b) !SPOL  -Vel + PHIDP
        call xcor1(ui,uq,vi,vq,nn,c,d) !SPOL  -Vel - PHIDP
c
        phhvv= atan2(b,a)*rad2deg   ! -Vel + PHIDP
        pvvhh= atan2(d,c)*rad2deg   ! -VEL + -PHIDP
C
C       PUT IN PHASE OFFSET ON THE TWO VVHH LAG1 COVARIANCES SO THAT
C       PHIDP STARTS AT ABOUT -70
C
C       THE TWO LAG 1 VVHH ARE:  (add offsets to put phidp at about -70deg.)
C
        tmpc  = cmplx(a,b)*cexp(-j*vvhh_offset)  !velocity +phidp
        tmpc1 = cmplx(c,d)*cexp(j*vvhh_offset)   ! velocity - phidp
c
        tmpc2= tmpc*conjg(tmpc1)     !2PHIDP  Velocity should be eliminated
        pdp = atan2(aimag(tmpc2),real(tmpc2))/2
        pdpCMD(i)=pdp*180./pi
c
c CALCULATE  UNFILTERED RHOHV for RHOHV TEST
c
c
        call xcor1(ui,uq,ui,uq,nn,e,f)
        call xcor1(vi,vq,vi,vq,nn,g,h)
        r2hhhh=(e**2+f**2)**.5
        r2hhhh=r2hhhh/varu
        r2vvvv=(g**2+h**2)**.5
        r2vvvv=r2vvvv/varv

        rouv=((a**2+b**2)**.5+(c**2+d**2)**.5)/2
        rouv=rouv/((varv*varu)**.5)
        r1hhvv=rouv
        ro11=(r2hhhh+ r2vvvv)/2
        rouv=rouv/(ro11**.25)
        r0hhvv=rouv
        rhvNF(i)=r0hhvv ! RHOHV not filtered for RHOHV test
c  NOW GET CPA
c
C   HH (u) channel
c   Get CPA
        cpa_i=0.0
        cpa_q=0.0
        den=0.0
        do 150 k=1,nn/2
           cpa_i=cpa_i + ui(k)
           cpa_q=cpa_q + uq(k)
           den= den+ sqrt(ui(k)**2 + uq(k)**2)
150     continue
        cpa= sqrt(cpa_i**2 + cpa_q**2)
        cpaHH=cpa/den
c
        cpa_i=0.0
        cpa_q=0.0
        den=0.0
        do 151 k=nn/2+1,nn
           cpa_i=cpa_i + ui(k)
           cpa_q=cpa_q + uq(k)
           den= den+ sqrt(ui(k)**2 + uq(k)**2)
151     continue
        cpa= sqrt(cpa_i**2 + cpa_q**2)
        cpaHH2=cpa/den
        cpaHH=(cpaHH+cpaHH2)/2
c
c   Get CPAV
c
        cpaV_i=0.0
        cpaV_q=0.0
        denV=0.0
        do 200 k=1,nn/2
           cpaV_i=cpaV_i + vi(k)
           cpaV_q=cpaV_q + vq(k)
           denV= denV+ sqrt(vi(k)**2 + vq(k)**2)
200     continue
        cpaV= sqrt(cpaV_i**2 + cpaV_q**2)
        cpaV=cpaV/denV
c
        cpaV_i=0.0
        cpaV_q=0.0
        denV=0.0
        do 201 k=nn/2+1,nn
           cpaV_i=cpaV_i + vi(k)
           cpaV_q=cpaV_q + vq(k)
           denV= denV+ sqrt(vi(k)**2 + vq(k)**2)
201     continue
        cpaV2= sqrt(cpaV_i**2 + cpaV_q**2)
        cpaV2=cpaV2/denV
        cpaV=(cpaV+cpaV2)/2
c
        cpaCMD(i) = max(cpaV,cpaHH)
c 
c PUT TIME SERIES INTO DOUBLE PRECISION ARRAYS
c
        do 100 k=1,nn
          Hci(k)=dble(ui(k))
          Hcq(k)=dble(uq(k))
          Vci(k)=dble(vi(k))
          Vcq(k)=dble(vq(k))
100     continue
c
c  GET RHOHV for FILTER DATA FOR RHOHV TEST
c
        iordr=5
        n=nn
        call LS_POLY(iordr,e1,n,l,ax,Hci,polycoef,dd,polyR)
        call subpoly(Hci,polyR,n)
c****
         call LS_POLY(iordr,e1,n,l,ax,Hcq,polycoef,dd,polyI)
         call subpoly(Hcq,polyI,n)
c******
         call LS_POLY(iordr,e1,n,l,ax,Vci,polycoef,dd,v1)
         call subpoly(Vci,v1,n)
c******
         call LS_POLY(iordr,e1,n,l,ax,Vcq,polycoef,dd,v2)
         call subpoly(Vcq,v2,n)
c
c   PUT TS into single precision arrays
        do 110 k=1,nn
           ui(k) = Hci(k)
           uq(k) = Hcq(k)
           vi(k) = Vci(k)
           vq(k) = Vcq(k)
110     continue
c
c  NOW CALCULATE RHOHV FILTERED
c
       call xcor0(ui,uq,ui,uq,nn,g,h)
       call xcor0(vi,vq,vi,vq,nn,e,f)
       varu=g
       varv=e
        call xcor0(vi,vq,ui,uq,nn,a,b) !SPOL  -Vel + PHIDP
        call xcor1(ui,uq,vi,vq,nn,c,d) !SPOL  -Vel - PHIDP
        call xcor1(ui,uq,ui,uq,nn,e,f)
        call xcor1(vi,vq,vi,vq,nn,g,h)
        r2hhhh=(e**2+f**2)**.5
        r2hhhh=r2hhhh/varu
        r2vvvv=(g**2+h**2)**.5
        r2vvvv=r2vvvv/varv

        rouv=((a**2+b**2)**.5+(c**2+d**2)**.5)/2
        rouv=rouv/((varv*varu)**.5)
        r1hhvv=rouv
        ro11=(r2hhhh+ r2vvvv)/2
        rouv=rouv/(ro11**.25)
        r0hhvv=rouv
        rhvFLT(i)=r0hhvv ! RHOHV filtered for RHOHV test
c       write(6,*) "RHOHV= ",i,rhvNF(i),rhvFLT(i)
c
10     continue   !GATES LOOP
       do 5, i=1,gates
c      write(6,*)i,hits,ZCMD(i),zdrCMD(i),pdpCMD(i),cpaCMD(i)
5      continue
       return
       end
c-----------------------
       subroutine getTDBZ(gates,ker)
      real ZCMD(1024),zdrCMD(1024),pdpCMD(1024),cpaCMD(1024)
       real TDBZ(1024),TZDR(1024),TPHI(1024),spinCMD(1024)
       integer gates
       common/CMDvar/ ZCMD,zdrCMD,pdpCMD,cpaCMD
       common /features/TDBZ,TZDR,TPHI,spinCMD
       do 10 i=1+ker/2,gates- ker/2
       sum=0.0
         do 20 k=i-ker/2,i+ker/2-1       ! just ker/2-1 difference for the kernal
           sum = sum + (ZCMD(k)-ZCMD(k+1))**2
20     continue
           TDBZ(i) = sum/(ker -1) 
10     continue
       return
       end
c-----------------------
       subroutine getSPIN(gates,ker)
      real ZCMD(1024),zdrCMD(1024),pdpCMD(1024),cpaCMD(1024)
       real TDBZ(1024),TZDR(1024),TPHI(1024),spinCMD(1024)
       integer gates
       common/CMDvar/ ZCMD,zdrCMD,pdpCMD,cpaCMD
       common /features/TDBZ,TZDR,TPHI,spinCMD
       do 10 i=1+ker/2,gates-ker/2
       spin =0.0
         do 20 k=i-ker/2 +1,i+ker/2 -1  !if ker =11, then only 9 spin inflection points
           a=ZCMD(k)-ZCMD(k-1)
           b=ZCMD(k+1)-ZCMD(k)
           if(a*b.lt.0.0) then    ! ie, k is inflection point
            avg= (abs(a) + abs(b))/2
c           write(6,*) "INFLECTION POINT!!!!!!!!!!!!!"
            if(avg.gt.6.5) then
c         write(6,*)"**************** got >6.5"
              spin = spin +1.
            endif
           endif
20       continue
         spinCMD(i)=100.*spin/(ker-2)
c        write(6,*) "SPIN", spinCMD(i)
10     continue
       return
       end
c-----------------------
       subroutine TZDRTPHI(gates,ker)
      real ZCMD(1024),zdrCMD(1024),pdpCMD(1024),cpaCMD(1024)
       real TDBZ(1024),TZDR(1024),TPHI(1024),spinCMD(1024)
       integer gates
       common/CMDvar/ ZCMD,zdrCMD,pdpCMD,cpaCMD
       common /features/TDBZ,TZDR,TPHI,spinCMD
       do 10 i=1+ker/2,gates -ker/2
          sumzd=0.0
          sumph=0.0
          zdstd=0.0
          phstd=0.0
c GET MEAN
          do 20  k=i-ker/2, i+ker/2
            sumzd=sumzd + zdrCMD(k)
            sumph=sumph + pdpCMD(k)
20        continue
          zdavg=sumzd/ker
          phavg=sumph/ker
c  GET STD
          do 30 k=i-ker/2, i+ker/2
           zdstd= zdstd +(zdrCMD(k) -zdavg)**2
           phstd= phstd +(pdpCMD(k) -phavg)**2
30        continue
       TZDR(i)=sqrt(zdstd/ker)
       TPHI(i)=sqrt(phstd/ker)
10     continue
       return
       end
c-----------------------
       subroutine getinterests(gates,k1,k2,k3)
       common /features/TDBZ,TZDR,TPHI,spinCMD
      real ZCMD(1024),zdrCMD(1024),pdpCMD(1024),cpaCMD(1024)
       real TDBZ(1024),TZDR(1024),TPHI(1024),spinCMD(1024)
       integer gates
       common/CMDvar/ ZCMD,zdrCMD,pdpCMD,cpaCMD
c CALCULATE INTERESTS FROM THE FEATURE FIELDS
c
c   get TDBZ interest
c
       do 10 i=1+k1/2, gates-k1/2       
         if(TDBZ(i).lt.20.) then
            TDBZ(i) = 0.
        else if(TDBZ(i).gt.40.) then
            TDBZ(i)=1.
        else
         TDBZ(i) = 0.05*TDBZ(i) -1.
        endif
10     continue
       TDBZ(k1/2)=TDBZ(k1/2 +1)  ! FILL IN 2 POINTS
       TDBZ(gates-k1/2 +1)=TDBZ(gates-k1/2)
c
c  get SPIN interest
c
       do 20 i=1+k2/2, gates-k2/2
          if(spinCMD(i).lt.15.) then
             spinCMD(i) = 0.0
          else if(spinCMD(i).gt. 30.) then
             spinCMD(i) = 1.
          else
            spinCMD(i)=0.0666667*spinCMD(i) -1.
          endif
20     continue
       spinCMD(k2/2) = spinCMD(k/2+1)
       spinCMD(k2/2-1) = spinCMD(k/2+1)
       spinCMD(gates -k2/2+1) = spinCMD(k/2+1)
       spinCMD(gates -k2/2+2) = spinCMD(k/2+1)
c
c  get ZDR interest
       do 30 i=1+k3/2, gates-k3/2
         if(TZDR(i).lt.1.2) then
            TZDR(i) = 0.0
         else if(TZDR(i).gt.2.4) then
            TZDR(i) = 1.0
         else
            TZDR(i) = 0.833333*TZDR(i) -1.
         endif
30     continue
c  get PHI interest
       do 40 i=1+k3/2, gates-k3/2
         if(TPHI(i).lt.10.) then
            TPHI(i) = 0.0
         else if(TPHI(i).gt.15.) then
            TPHI(i) = 1.0
         else
            TPHI(i) = 0.2*TPHI(i) -2.
         endif
40     continue
c
c  get CPA interest
c
       do 50 i=1, gates
         if(cpaCMD(i).lt.0.6) then
            cpaCMD(i) = 0.0
         else if(cpaCMD(i).gt.0.9) then
            cpaCMD(i) = 1.0
         else
            cpaCMD(i) = 3.333333*cpaCMD(i) -2.
         endif
50     continue
       return
       end
c-----------------------
       subroutine combinterest(gates,ker,combint)
       common /features/TDBZ,TZDR,TPHI,spinCMD
      real ZCMD(1024),zdrCMD(1024),pdpCMD(1024),cpaCMD(1024)
       real TDBZ(1024),TZDR(1024),TPHI(1024),spinCMD(1024)
       integer gates
       real maxint,combint(1024)
       common/CMDvar/ ZCMD,zdrCMD,pdpCMD,cpaCMD
c WE USE THE Zdr KERNAL SINCE SPIN AND TDBZ WERE EXTENDED
       do 10, i=1+ker/2, gates-ker/2
          maxint=max(TDBZ(i),spinCMD(i))
          combint(i)=(maxint + TZDR(i) + TPHI(i) + cpaCMD(i))/4
        write(6,*) "combint",combint(i)
       write(6,*)"INTS",i*.15,TDBZ(i),spinCMD(i),maxint,TZDR(i),TPHI(i),
     >            cpaCMD(i)
10     continue
       return
       end
c-----------------------
cc----------------------------------------------------------------
       subroutine process
      parameter (npulses=30000,ngates = 1000)
      character*15 ptype(10),time*12
        common /CMDFLG/CMDflag
        integer CMDflag(1024)
        common
     +        /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
     +         /specfilt/sigmaf,xorder,nor,onoff,dc,polyopt
        common/polyfitparms/xarray,polycoef,polyorder,iauto
        common /interppoly/interp,lenpinterp,regwdth,icmd,gain
      real xi(1025),xq(1025),yi(1025),yq(1025),xyi(1025),xyq(1025)
     +,yxi(1025),yxq(1025)
c2345678901234567890123456789012345678901234567890123456789012345678901
c        1         2         3         4         5         6         7
        integer nwidth,notchopt,lenblock,regwdth(26)
       common /specnotchparms/nwidth,notchopt,lenblock,wdwtype,interp2
       integer polyopt,polyorder,i,m
        real ui(1025),uq(1025),vi(1025),vq(1025),uvi(1025),uvq(1025),
     >  vui(1025),vuq(1025),nor(1025),extraI,extraQ
        real tmpui(1025), tmpuq(1025),gain(20)
c
c DOUBLE PRESCISION FOR POLY FIT SUBROUTINE
c
       real*8 Hci(1025),Hcq(1025),Vci(1025),Vcq(1025),HVi(1025),
     >   HVq(1025), VHi(1025),VHq(1025)
       real*8 ax(1025),y(1025),polyI(1025),polyR(1025),v(1025),
     >    dd,e1
       real polycurvI(1025),polycurvR(1025)
       real savui(1025), savuq(1025)
c   DP PRECISON END
c
       real plfile(-5:3000,0:54),gwnd(4,4)
        real lambda,ldrh,ldrx,ldrx1,ldrx2,ldrv
        real  timsr(1:1025,0:8)
        real xarray(1025)
        real*8 polycoef(0:39)
         complex tmpc,tmpc1,j
        integer rtype(4),rng_index,raycount,eof
        character*6 scale(2),onoff*3,dc*3, hardxgks*70,label(0:54)*12
        character*14 wdwtype(6),singletype(4),filename*80,infile*80
        character xlab*10,ylab*10
        common /ray/plfile,rtype
        common /labels/label
        common /infile1/filename
       real    cit(500),cib(500),tlut(50),boundary(500),
     >         var(500),avg(500)
       common /statpts/var,cit,cib,boundary,avg,maxbin,wdthsc
        common /radardata/rng,zh, zdr,ldrh,ldrv, ldrx, zv, dop, pvvhh,
     & r0hhvv, r0hhvh, r0vvhv, r2hhhh, r2vvvv, r1hvvh, r1vhhv, r0hvvh,
     &    r3hhvv, r3vvhh, phhvh, pvvhv, pdp3, phvvh, pvhhv,pdp_2lg
     &    ,wdth_hh,wdth_vh,pdpx,r1hhvv,ro11, rx_r1hv,phvvh_dp,phvvh_vel
     &    ,phidp,phhvv,cpa,cpaV,cpaUV,cpaVU,cpaUF
        common /specplot/timsr,magorph,logorlin
        common /hardcopy/hardxgks,scale
       integer year,month,day,gates,hits,hour,min,sec,maxrayno
       common /raydat/range_inc,range,azim,elev,hour,min,time_inc,
     >  sec,year,month,day,hits,gates,rng_index,raycount,maxrayno
     >  ,range_ary
      real range_ary(ngates)
      integer trparms,tlparms,blparms,brparms,reset,scatprm,scatype(2)
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
       common /scatparms/scatarry,scatprm,scatype,npts,tlut
       common /offsets/vvhh_offset,vvhv_offset,hhvh_offset,zdroffset
       real scatarry(50000,2)
c---
       common /FFTPACK/wsave,work,lenwrk,lensav,lenc,inc,ier
       real wsave(3000), work(3000)
      complex cts(1025),cts1(1025) ! data
c---

C
C  THESE FILES ARE FOR THE THE U (HH) POLYFITTED CURVES
       open(31,file="TSRealPoly.dat")
       open(32,file="TSImagPoly.dat")
c      print *,"in process, zdroff=",zdroffset
c
c   CALCULATE THE UNFILTERED POWER AND PUT IN PLFILE
c       (this is now done after windowing)
c        call xcor0(ui,uq,ui,uq,nn,g,h)
c      print *,'rng_index,g =', rng_index,g
c       plfile(rng_index,49)=10*log10(g)
C  SET GENERIC X ARRAY TO PASS TO POLYFIT ROUTINE
        do 1 i=1,nn
         ax(i) = dble(1.*i)
1       continue
C    GET UNFILTERED, UNWINDOWED CPA (used as a threshold in scatter plots)
c
C   HH (u) channel
c   Get CPA
        cpa_i=0.0
        cpa_q=0.0
        den=0.0
        do 100 i=1,nn
           cpa_i=cpa_i + ui(i)
           cpa_q=cpa_q + uq(i)
           den= den+ sqrt(ui(i)**2 + uq(i)**2)
100     continue
        cpaUF= sqrt(cpa_i**2 + cpa_q**2)
        cpaUF=cpaUF/den
        print *,'RAW********CPAUF = ', cpaUF
        plfile(rng_index,51) = cpaUF
c
C GET AVERAGE POWER OF 0 Vel PLUS NEXT TWO VEL BINS
c
c       a=nn
        n=nn
c       mmm=nint(log10(a)/log10(2.))
c---------------------------------------------
C GET POWER VIA NEW FFT ROUTINES
C TRANFER REAL AND IMAGINERY PARTS IN COMPLEX VARIABLE CTS
       cts1 = cmplx(ui,uq)
c DO FFTS
       call  CFFT1F(N,INC,CTS1,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
C PUT BACK INTO REAL ARRAYS, now in frequency domain
        tmpui= real(cts1)
        tmpuq= aimag(cts1)
c       call fft2(ui,uq,nn,mmm)
c THE MIDDLE 3 VELOCITIES ARE: i=1 (0V), i=2 and i=N
 
         p1= tmpui(1)**2 + tmpuq(1)**2
         p2= tmpui(2)**2 + tmpuq(2)**2
         p3= tmpui(nn)**2 + tmpuq(nn)**2
         pavg= 10*log10((p1+p2+p3))
c
c DO FOR V CHANNEL
C
C TRANFER REAL AND IMAGINERY PARTS IN COMPLEX VARIABLE CTS
       cts1 = cmplx(vi,vq)
c DO FFTS
       call  CFFT1F(N,INC,CTS1,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
C PUT BACK INTO REAL ARRAYS, now in frequency domain
        tmpui= real(cts1)
        tmpuq= aimag(cts1)
c       call fft2(ui,uq,nn,mmm)
c THE MIDDLE 3 VELOCITIES ARE: i=1 (0V), i=2 and i=N

         p1= tmpui(1)**2 + tmpuq(1)**2
         p2= tmpui(2)**2 + tmpuq(2)**2
         p3= tmpui(nn)**2 + tmpuq(nn)**2
         pavgv= 10*log10((p1+p2+p3))
         if(pavgv.gt.pavg) then
            pavgmax= pavgv
         else
            pavgmax= pavg
         endif
        pavg=pavgmax
         plfile(rng_index,52) = pavg
C      DONE CALCULATE POWER FROM 3 VELOCITY BINS
c       call ifft2(ui,uq,nn,mmm)
c-----------------------------------------------
       If(iauto.eq.1) then   !Auto regression order select is on
c
c  SET REGRESSION ORDER BASED ON CLUTTER POWER IN plfile(rng_index,52) above
c
       if (pavg.lt.-65) then
          polyorder = 2
          else
       if (pavg.lt.-50) then
          polyorder = 5
         else 
          if (pavg.lt. -30.) then
              polyorder = 6
          else
             if (pavg.lt. -20.) then
              polyorder = 8
            else
               if (pavg.lt. 0.) then
               polyorder = 9
               else
                 polyorder = 11
               end if
            end if
          end if
        end if
       endif
        lenpinterp = regwdth(polyorder)  ! was polyorder +2
       endif
        plfile(rng_index,53) = polyorder
       write(6,*)"************* order =", polyorder,pavg
c
c   CALCULATE THE RAW UNWINDOWED, UNFILTERED POWER
C    SO THAT THE WINDOWED TS POW CAN BE COPENSATED BELOW
c
        call xcor0(ui,uq,ui,uq,nn,g,h)
        powraw=g
c
c To compare power removed to power before clutter filter, it is better to 
c to calculate power from windowed data rather than add 4.19 dB to compesate 
c for the hanning window (or an other window)
c
        call windowdata
c
c    NOW CALCULATE THE UNFILTERED POWER, but windowed, AND PUT IN PLFILE
c
        call xcor0(ui,uq,ui,uq,nn,g,h)
        plfile(rng_index,49)=10*log10(g)
c************************
c  29 JULY 2020  Mike and Greg simply compensate by adding mean power loss dur to the window
c  instead of calculating the actual [power removed and then compensating as done below
c  so comment this out for standard operations.  
c***********************
cjh        if(iwdw.ne.1) then  ! 1 means rectangular window and no compensation needed
cjh        compensate=sqrt(powraw/g)  !sqrt is for compensating TS not power
c       compensate= 1.97 !10^(5.69/10) Blackman Nuttall Window average
c
c   NOW COMPENSATE TIME SERIES FOR WINDOW FUNCTION LOSS
c
cjh        do 249 i=1,nn
cjh          ui(i)=compensate*ui(i)    
cjh          uq(i)=compensate*uq(i)    
cjh          vi(i)=compensate*vi(i)    
cjh          vq(i)=compensate*vq(i)    
cjh          uvi(i)=compensate*uvi(i)    
cjh          uvq(i)=compensate*uvq(i)    
cjh          vui(i)=compensate*vui(i)    
cjh          vuq(i)=compensate*vuq(i)    
cjh 249     continue
c      print *,'rng_index,g =', rng_index,g
cjh        plfile(rng_index,49)=10*log10(compensate**2*g)
cjh        endif


      if (dc.eq.'yes') then
         call removedc(ui,uq,nn)
         call removedc(vi,vq,nn)
         call removedc(uvi,uvq,nn)
         call removedc(vui,vuq,nn)
      endif
c*************
      if(((polyopt.eq.1).AND.(icmd.eq.0)).OR.
     > ((polyopt.eq.1).AND.(icmd.eq.1.).AND.
     >   (CMDflag(rng_index).eq.1))) then  !polyopt=1 means do clutter filter
c*******************
C FIRST SAVE A COPY OF THE Ui and Uq time series
c
        do 300 i=1,nn
          savui(i) = ui(i)
          savuq(i) = uq(i)
300     continue
C  BREAK 64 point  time series into two-32 point blocks
C  BREAK 64 point  time series into four-32 point blocks
c  AND COPY TIME SERIES INTO DOUBLE PRECISION ARRAYS
c
         len=nn/lenblock  !len is the number of blocks
         n=lenblock  !just for poly call below
c        print *,'PROCESS,nn,n,lenblock,len ',nn,n,lenblock,len   
         do 10 i=1,len
           do 20 m=1,lenblock   !Copy time series into arrays
c BREAK INTO POWER OF 2 LENGTH SUBSEQUENCES as REQUEXTED by lenblock
               Hci(m)= dble(ui(m+(i-1)*lenblock))
               Hcq(m)= dble(uq(m+(i-1)*lenblock))
               Vci(m)= dble(vi(m+(i-1)*lenblock))
               Vcq(m)= dble(vq(m+(i-1)*lenblock))
              VHi(m)= dble(vui(m+(i-1)*lenblock))
              VHq(m)= dble(vuq(m+(i-1)*lenblock))
              HVi(m)= dble(uvi(m+(i-1)*lenblock))
              HVq(m)= dble(uvq(m+(i-1)*lenblock))
20         continue
c        print *,"BEFORE POLY CALL, n=", n
c
c        Force array length to  BLOCK length
c  NEW DOUBLE PRECISION POLY FIT ROUTINE
c      n          data length
c      polycoef(0:39)  poly coefficients
c      v(size)    fitted poly data output
c      m          order
c      dd         STD of fit
c      ax,Hcq(etc.)        input arrays
c      e1         e1 = 0 => fit is to order m
c      l          fir order if e1 Not equal 1
         e1=dble(0.0)
c2345678901234567890123456789012345678901234567890123456789012345678901
c        1         2         3         4         5         6         7
        call LS_POLY(polyorder,e1,n,l,ax,Hci,polycoef,dd,polyR)
c       print*,"1", "POLYORDER",polyorder
         call subpoly(Hci,polyR,n)
c****
c        print*,"2", "POLYORDER",polyorder
         call LS_POLY(polyorder,e1,n,l,ax,Hcq,polycoef,dd,polyI)
c        print*, "3POLYORDER",polyorder
         call subpoly(Hcq,polyI,n)
c******
c        print*, "4POLYORDER",polyorder
         call LS_POLY(polyorder,e1,n,l,ax,Vci,polycoef,dd,v)
c        print*, "5POLYORDER",polyorder
         call subpoly(Vci,v,n)
c        print*, "6POLYORDER",polyorder
         call LS_POLY(polyorder,e1,n,l,ax,Vcq,polycoef,dd,v)
c        print*, "7POLYORDER",polyorder
         call subpoly(Vcq,v,n)
c        print*, "8POLYORDER",polyorder
         call LS_POLY(polyorder,e1,n,l,ax,HVi,polycoef,dd,v)
c        print*, "9POLYORDER",polyorder
         call subpoly(HVi,v,n)
c        print*, "10POLYORDER",polyorder
         call LS_POLY(polyorder,e1,n,l,ax,HVq,polycoef,dd,v)
c        print*, "11POLYORDER",polyorder
         call subpoly(HVq,v,n)
c        print*, "12POLYORDER",polyorder
         call LS_POLY(polyorder,e1,n,l,ax,VHi,polycoef,dd,v)
c        print*, "13POLYORDER",polyorder
         call subpoly(VHi,v,n)
c        print*, "14POLYORDER",polyorder
         call LS_POLY(polyorder,e1,n,l,ax,VHq,polycoef,dd,v)
c        print*, "15POLYORDER",polyorder
         call subpoly(VHq,v,n)
c        print*, "16POLYORDER",polyorder
C
C NOW COPY REGRESSION FILTERED DATA BACK INTO ORIGINAL ARRAYS
C
           do 21 m=1,lenblock   !Copy regression filtered time series into arrays
               ui(m+(i-1)*lenblock)= real(Hci(m))*gain(polyorder)
               uq(m+(i-1)*lenblock)= real(Hcq(m))*gain(polyorder)
               vi(m+(i-1)*lenblock)= real(Vci(m))*gain(polyorder)
               vq(m+(i-1)*lenblock)=real( Vcq(m))*gain(polyorder)
               vui(m+(i-1)*lenblock)= real(VHi(m))*gain(polyorder)
               vuq(m+(i-1)*lenblock)= real(VHq(m))*gain(polyorder)
               uvi(m+(i-1)*lenblock)= real(HVi(m))*gain(polyorder)
               uvq(m+(i-1)*lenblock)= real(HVq(m))*gain(polyorder)
c THESE ARAYS CONTAIN THE POLYFIT CURVE FOR Ui, Uq
               polycurvR(m+(i-1)*lenblock)= real(polyR(m))
               polycurvI(m+(i-1)*lenblock)= real(polyI(m))
21         continue
10    continue    !FINISHED BLOCK regression filter blocks
C
C DO INTERPOLATION IF REQUESTED
c
c       minterp = nint(log10(real(n))/log10(2.)) !for FFT call below
      if(interp.ne.2) then
         call zd_pdp_rho_filtered(rng_index)
         call interpolate(ui,uq,vi,vq,nn,1)
c        call interpolate(vi,vq,n,minterp)
         call interpolate(vui,vuq,uvi,uvq,nn,1)
c        call interpolate(uvi,uvq,n,minterp)
      endif
c
C  WRITE THE REAL AND IMAGINARY PARTS OF THE POLYNIMIAL FIT CURVE TO Ui, Uq
      write(31,*) nn
      write(32,*) nn
      do 1000 i=1, nn
        write(31,*) i*Ts*1000,savui(i),polycurvR(i)
        write(32,*) i*Ts*1000,savuq(i),polycurvI(i)
c       write(31,*) i,savui(i),polycurvR(i)
c       write(32,*) i,savuq(i),polycurvI(i)
1000  continue
      endif   ! end poly filter section
c
C     DO SPECTRUM NOTCH FILTER
C
       if(notchopt.eq.1) then
       call specnotch
       endif
c----
      if(onoff.eq.'on') then
         print *,'filter is called'
         call filt(vi,vq,ui,uq,uvi,uvq,vui,vuq,nn,rng)
      endif
c
      close(31)
      close(32)
      return
      end
cc----------------------------------------------------------------
c  THIS SUBROUTINE CALCULATES ZDR, PHDP and RHOHV FOR FILTERED 
C       AND NOT INTERPOLATED DATA FOR A SINGLE GATE
c
       subroutine zd_pdp_rho_filtered(rng_index)
      common /radarconstants/basedbzh,basedbzv,noiseh,noisev
        common
     +        /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
        real ui(1025),uq(1025),vi(1025),vq(1025),uvi(1025),uvq(1025),
     >  vui(1025),vuq(1025)
       common /nointerpvars/zdnoint,pdpnoint,rhvnoint
       real zdnoint(1024),pdpnoint(1024),rhvnoint(1024)
       real lambda
       common /offsets/vvhh_offset,vvhv_offset,hhvh_offset,zdroffset
       integer rng_index
        real noiseh,noisev
       complex tmpc,tmpc1,tmpc2,j
        rad2deg=180./3.1415926
        j=cmplx(0.,1.)
        data pi/3.1415927/
        write(6,*) "MADE IT TO ZDRPDPRHV",rng_index
C GET  ZDR
          call xcor0(ui,uq,ui,uq,nn,g,h)
          call xcor0(vi,vq,vi,vq,nn,e,f)
c        ZCMD(i)= 10*log10(g)
         zh= 10*log10(g) -noiseh + basedbzh 
         zv= 10*log10(e) -noisev + basedbzv 
         zdnoint(rng_index)= zh - zv - zdroffset
         varu=g
         varv=e
c GET PHIDP
        call xcor0(vi,vq,ui,uq,nn,a,b) !SPOL  -Vel + PHIDP
        call xcor1(ui,uq,vi,vq,nn,c,d) !SPOL  -Vel - PHIDP
c
        phhvv= atan2(b,a)*rad2deg   ! -Vel + PHIDP
        pvvhh= atan2(d,c)*rad2deg   ! -VEL + -PHIDP
C
C       PUT IN PHASE OFFSET ON THE TWO VVHH LAG1 COVARIANCES SO THAT
C       PHIDP STARTS AT ABOUT -70
C
C       THE TWO LAG 1 VVHH ARE:  (add offsets to put phidp at about -70deg.)
C
        tmpc  = cmplx(a,b)*cexp(-j*vvhh_offset)  !velocity +phidp
        tmpc1 = cmplx(c,d)*cexp(j*vvhh_offset)   ! velocity - phidp
c
        tmpc2= tmpc*conjg(tmpc1)     !2PHIDP  Velocity should be eliminated
        pdp = atan2(aimag(tmpc2),real(tmpc2))/2
        pdpnoint(rng_index)=pdp*180./pi
c
c CALCULATE  UNFILTERED RHOHV for RHOHV TEST
c
c
        call xcor1(ui,uq,ui,uq,nn,e,f)
        call xcor1(vi,vq,vi,vq,nn,g,h)
        r2hhhh=(e**2+f**2)**.5
        r2hhhh=r2hhhh/varu
        r2vvvv=(g**2+h**2)**.5
        r2vvvv=r2vvvv/varv

        rouv=((a**2+b**2)**.5+(c**2+d**2)**.5)/2
        rouv=rouv/((varv*varu)**.5)
        r1hhvv=rouv
        ro11=(r2hhhh+ r2vvvv)/2
        rouv=rouv/(ro11**.25)
        r0hhvv=rouv
        rhvnoint(rng_index)=r0hhvv 
        write(6,*) "AT END OF ZDRPDPRHHV",rng_index
        return
        end
c-------------------------------------------------
c
       subroutine specnotch
        common
     +        /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
       real ui(1025),uq(1025),vi(1025),vq(1025),uvi(1025),uvq(1025),
     >    vui(1025),vuq(1025),lambda,extraI,extraQ
        integer nwidth,notchopt
       common /specnotchparms/nwidth,notchopt,lenblock,wdwtype,interp2
       character*14 wdwtype(6)
c---
       common /FFTPACK/wsave,work,lenwrk,lensav,lenc,inc,ier
       real wsave(3000), work(3000)
      complex cts1(1025),cts2(1025),cts3(1025),cts4(1025) ! data
c---

c      DO THE DFTs
        a=nn
        n=nn
        index=nwidth/2
        m=nint(log10(a)/log10(2.))
        print *,'SPEC NOTCH BEFORE FFT CALL n,m=',n,m
c
c   THE FOLLOWING WAS TO TEST THE EFFECT OF FFT ON POWER
c   AFETR THE FFT2 CALL, THE power (xcor0 call) should be
c   multiplied by nn, the sequence length. However, if using
c   the IFFT after FFT, the power is restored.
c-----------------
c      call xcor0(ui,uq,ui,uq,nn,a,b)
c      call fft2(ui,uq,nn,m)
c      call xcor0(ui,uq,ui,uq,nn,c,d)
c      c=c/64.
c      call ifft2(ui,uq,nn,m)
c      call xcor0(ui,uq,ui,uq,nn,e,f)
c      print *,c,c*64.
c      print *,'FFT POW ',a,c,a/c
c      print *,'FFT POW ',a,e,a/e
c      read(5,*)
c----------------
C TRANFER REAL AND IMAGINERY PARTS IN COMPLEX VARIABLE CTS
       cts1 = cmplx(ui,uq) 
       cts2 = cmplx(vi,vq) 
       cts3 = cmplx(uvi,uvq) 
       cts4 = cmplx(vui,vuq) 
c DO FFTS
       call  CFFT1F(N,INC,CTS1,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1F(N,INC,CTS2,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1F(N,INC,CTS3,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1F(N,INC,CTS4,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
C PUT BACK INTO REAL ARRAYS
        ui= real(cts1)
        uq= aimag(cts1)
        vi= real(cts2)
        vq= aimag(cts2)
        uvi=real(cts3)
        uvq=aimag(cts3)
        vui=real(cts4)
        vuq=aimag(cts4)
c       call fft2(ui,uq,n,m)
c       call fft2(vi,vq,n,m)
c       call fft2(uvi,uvq,n,m)
c       call fft2(vui,vuq,n,m)
C      DO THE NOTCH
c      FIRST TAKE OUT DC
c 00001 is used instead of zero for plotting purposes
       ui(1)=0.0000001
       uq(1)=0.0000001
       vi(1)=0.0000001
       vq(1)=0.0000001
       uvi(1)=0.0000001
       uvq(1)=0.0000001
       vui(1)=0.0000001
       vuq(1)=0.0000001
c-------
       do 10, i=1,index
       ui(1+i)=0.0000001
       uq(1+i)=0.0000001
       vi(1+i)=0.0000001
       vq(1+i)=0.0000001
       uvi(1+i)=0.0000001
       uvq(1+i)=0.0000001
       vui(1+i)=0.0000001
       vuq(1+i)=0.0000001
c-------
       ui(1+n-i)= 0.0000001
       uq(1+n-i)= 0.0000001
       vi(1+n-i)= 0.0000001
       vq(1+n-i)= 0.0000001
       uvi(1+n-i)=0.0000001
       uvq(1+n-i)=0.0000001
       vui(1+n-i)=0.0000001
       vuq(1+n-i)=0.0000001
10     continue
C      DONE NOTCH. DO IFFT
       cts1 = cmplx(ui,uq) 
       cts2 = cmplx(vi,vq) 
       cts3 = cmplx(uvi,uvq) 
       cts4 = cmplx(vui,vuq) 
       call  CFFT1B(N,INC,CTS1,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1B(N,INC,CTS2,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1B(N,INC,CTS3,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1B(N,INC,CTS4,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
C PUT BACK INTO REAL ARRAYS
        ui= real(cts1)
        uq= aimag(cts1)
        vi= real(cts2)
        vq= aimag(cts2)
        uvi=real(cts3)
        uvq=aimag(cts3)
        vui=real(cts4)
        vuq=aimag(cts4)
      if(interp2.eq.1) then
         call interpolate(ui,uq,vi,vq,n,2)
         call interpolate(vui,vuq,uvi,uvq,n,2)
      endif
c       call ifft2(ui,uq,n,m)
c       call ifft2(vi,vq,n,m)
c       call ifft2(uvi,uvq,n,m)
c       call ifft2(vui,vuq,n,m)       
       return
       end
c------------------------------------------------------------------
       subroutine subpoly(y,v,nn)
       real*8 v(nn),y(nn) 
       do 1000 i=1,nn
          y(i) = y(i)-v(i) 
1000   continue
       return
       end
c-------------------------------------------------------
c      subroutine subpoly(xarray,y,nn,coef,od)
c      integer od
c      real xarray(nn)
c      real  coef(4), y(nn),pfit
c SUBTRACT POLYFIT FROM DATA
c      xinc= 1./(xarray(nn)-xarray(1))
c      do 1000 i=1,nn
c         x=xinc*(i-1)
c         pfit =0.0
c        do 1001 m=1,od+1
c          pfit =pfit + coef(m)*x**(m-1)
c1001     continue
c         y(i) = y(i) - pfit
c1000   continue
c      return
c      end
c
c-------------------------------------------------------
c-------------------------------------------------------
c   THIS SUBROUTINE INTERPOLATES ACCROSS THE ZERO VELOCITY GAP
c    FOR BOTH WN AND REG> FILTERS
C    GAUSSIAN OPTION
c    interp 1,2,3 : linear, no interp, GAUSSIAN
c
      subroutine interpolate(x,y,x1,y1,n,iii)
       real x(n),y(n),magnitude(n),ph(n),xinc
       real x1(n), y1(n),ph1(n),pdif(n),gain(20)
       real xsave(n),ysave(n),x1save(n),y1save(n)
       complex sumph,tmpc
        real xarray(1025)
       integer end1, end2, regwdth(26),order,GAUSS,polyorder
       common /interppoly/interp,lenpinterp,regwdth,icmd,gain
        common/polyfitparms/xarray,polycoef,polyorder,iauto
       common /specnotchparms/nwidth,notchopt,lenblock,wdwtype,interp2
       character wdwtype(6)*14
c---
c        common /FFTPACK/wsave,lenwrk,lensav,ier
       common /FFTPACK/wsave,work,lenwrk,lensav,lenc,inc,ier
       real wsave(3000), work(3000)
       complex cts1(n),cts2(n) ! data
        real*8 polycoef(0:39)
c---
c------------FFTPACK parameters !make these parameters?
c       lensav = 3000
c    lenwrk= 3000
c      inc=1
c      ier=0
c      lenc=n
c----------
c  SAVE INPUT TSERIES
c----------------
       write(6,*) "IN INTERPOLATE. order=", polyorder,iii
       order=polyorder
       if(iii.eq.2) then   !Window + notch filter
            len=nwidth     !+2
         else if (iii.eq.1) then  ! Regression filter
            len= regwdth(order)
       endif
c      write(6,*) 'IN INTERPOLATE, n,m, len =', n,m,len
C PUt REAL AND IMAGINARY SEUQENCES INTO COMPLEX SEQUENCE FOR PASSING TO FFTPACK
        cts1= cmplx(x,y)
        cts2= cmplx(x1,y1)
       call CFFT1F(N,INC,CTS1,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call CFFT1F(N,INC,CTS2,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       write(6,*) "AFTER FIRST FFT CALL"
C PUT BACK INTO REAL AND IMAGINARY
        x = real(cts1)
        y = aimag(cts1)
        x1 = real(cts2)
        y1 = aimag(cts2)
c 
C SAVE ORIGINAL SPECTRA
       xsave=x
       ysave=y
       x1save=x1
       y1save=y1
c---
c      call fft2(x,y,n,m) !old fft routine
c      call fft2(x1,y1,n,m)
c
c CALCULATE PHASE DIFFERENCEa INFREQUNCY DOMAIN
c
       sumph = cmplx(0.,0.)
       sumph = cmplx(0.,0.)
       do 1 i=1,n
         sumph= sumph + cmplx(x(i),-y(i))*cmplx(x1(i),y1(i))
         tmpc =         cmplx(x(i),-y(i))*cmplx(x1(i),y1(i))
         pdif(i) = atan2(aimag(tmpc), real(tmpc))
1      continue
       phvh= atan2(aimag(sumph),real(sumph))
c      zero velocity is at n = 1
c
c FIRST DO H CHANNEL X,y
c
c GET MAG and PHASE
       do 10 i=1,n
         magnitude(i)= log10(x(i)**2 + y(i)**2)  !**0.5  INTERPOLATE log10 POWER
c        magnitude(i)= (x(i)**2 + y(i)**2)  !**0.5  INTERPOLATE linear  POWER
         ph(i)= atan2(y(i),x(i))
c        write(6,*) "MAG, PHA ",i,magnitude(i),ph(i)
10     continue
c NOW INTERPOLATE MAGNITUDE as requested by length len
c FIND END POINTS and CALCULATE INCREMENT
c AND CALCULATE REAL AND IMAGINARY PARTS, x, y
       end1= (len+1)/2 +1
       end2= n-(len-1)/2
       base2=magnitude(end2)
c*****TRY AVERAGING FIRST 2 POINTS AND LAST 2 POINTS
c THIS MADE A SMALL IMPROVEMENT MEAN POW BUT STD POW INCREASED A BIT TOO
c  FOR VELOCITY SMALL IMPR. IN MEAN STD STAYED THE SAME. OTHER AVERAGES COULD BE TRIED
c      magnitude(end1) = (magnitude(end1) + magnitude(end1+1))/2
c      magnitude(end2) = (magnitude(end2) + magnitude(end2-1))/2
c*******
       xinc = (magnitude(end1)-magnitude(end2))/(len+1)
c      write(6,*) "end1, end2,base2,xinc", end1,end2,base2,xinc
c      read(5,*)
       do 20 i=1, (len-1)/2
          magnitude(end2+i)=  (10**(base2+xinc*i))**.5  !BACK TO MAGNITUDE LINEAR
c         magnitude(end2+i)=  ((base2+xinc*i))**.5  !GET MAGNITUDE LINEAR
          if(iii.eq. 2) then
          x(end2+i)= magnitude(end2+i)*cos(-phvh/2)
          y(end2+i)= magnitude(end2+i)*sin(-phvh/2)
          else
c         x(end2+i)= (magnitude(end2+i))*cos(-pdif(end2+i)/2)
c         y(end2+i)= (magnitude(end2+i))*sin(-pdif(end2+i)/2)
          x(end2+i)= magnitude(end2+i)*cos(-phvh/2)
          y(end2+i)= magnitude(end2+i)*sin(-phvh/2)
c1        x(end2+i)= (magnitude(end2+i))*cos(ph(end2+i))
c1        y(end2+i)= (magnitude(end2+i))*sin(ph(end2+i))
c         x(end2+i)= magnitude(end2+i)*cos(3.14159/4)
c         y(end2+i)= magnitude(end2+i)*sin(3.14159/4)
          endif
20     continue
       do 30 i=1, (len +1)/2
         magnitude(i)=(10**(base2+ xinc*((len-1)/2 +i)))**.5  !FOR LOG POW INTERP
c        magnitude(i)=((base2+ xinc*((len-1)/2 +i)))**.5  !FOR LIN POW INTERP
         if(iii.eq. 2) then
         x(i) = magnitude(i)*cos(-phvh/2)
         y(i) = magnitude(i)*sin(-phvh/2)
         else
c        x(i) = (magnitude(i))*cos(-pdif(i)/2)
c        y(i) = (magnitude(i))*sin(-pdif(i)/2)
c1       x(i) = (magnitude(i))*cos(ph(i))
c1       y(i) = (magnitude(i))*sin(ph(i))
         x(i) = magnitude(i)*cos(-phvh/2)
         y(i) = magnitude(i)*sin(-phvh/2)
c        x(i) = magnitude(i)*cos(3.14159/4)
c        y(i) = magnitude(i)*sin(3.14159/4)
         endif
30     continue
c----------------------
c
c NOW DO V CHANNEL x1,y1
c
c GET MAG and PHASE
       do 40 i=1,n
         magnitude(i)= log10(x1(i)**2 + y1(i)**2) !**0.5 LOG POWER
c        magnitude(i)= (x1(i)**2 + y1(i)**2) !**0.5 LINEAR POWER
         ph1(i)= atan2(y1(i),x1(i))
c        write(6,*) "MAG, PHA ",i,magnitude(i),ph1(i)
40     continue
c NOW INTERPOLATE MAGNITUDE as requested by length len
c FIND END POINTS and CALCULATE INCREMENT
c AND CALCULATE REAL AND IMAGINARY PARTS, x, y
       end1= (len+1)/2 +1
       end2= n-(len-1)/2
       base2=magnitude(end2)
       xinc = (magnitude(end1)-magnitude(end2))/(len+1)
c      write(6,*) "end1, end2,base2,xinc", end1,end2,base2,xinc
c      read(5,*)
       do 50 i=1, (len-1)/2
          magnitude(end2+i)=  (10**(base2+xinc*i))**.5 ! FOR LOG POWER INTERP
c         magnitude(end2+i)=  ((base2+xinc*i))**.5 ! FOR LINEAR POW INTERP
          if(iii.eq. 2) then
          x1(end2+i)= magnitude(end2+i)*cos(phvh/2)
          y1(end2+i)= magnitude(end2+i)*sin(phvh/2)
          else
c         x1(end2+i)= (magnitude(end2+i))*cos(pdif(end2+i)/2)
c         y1(end2+i)= (magnitude(end2+i))*sin(pdif(end2+i)/2)
c1        x1(end2+i)= (magnitude(end2+i))*cos(ph1(end2+i))
c1        y1(end2+i)= (magnitude(end2+i))*sin(ph1(end2+i))
          x1(end2+i)= magnitude(end2+i)*cos(phvh/2)
          y1(end2+i)= magnitude(end2+i)*sin(phvh/2)
c         write(6,*) "INTERP PHASE rad = ", ph1(end2+i)
c         x1(end2+i)= magnitude(end2+i)*cos(ph(end2+i)+phvh)
c         y1(end2+i)= magnitude(end2+i)*sin(ph(end2+i)+phvh)
c         x1(end2+i)= magnitude(end2+i)*cos(3.14159/4)
c         y1(end2+i)= magnitude(end2+i)*sin(3.14159/4)
          endif
50     continue
       do 60 i=1, (len +1)/2
         magnitude(i)=(10**(base2+ xinc*((len-1)/2 +i)))**.5 !FOR LOG POW INTERP
c        magnitude(i)=((base2+ xinc*((len-1)/2 +i)))**.5  !FOR LINEAR POW INTERP
         if(iii.eq. 2) then
         x1(i) = magnitude(i)*cos(phvh/2)
         y1(i) = magnitude(i)*sin(phvh/2)
         else
c        x1(i) = (magnitude(i))*cos(pdif(i)/2)
c        y1(i) = (magnitude(i))*sin(pdif(i)/2)
c1        x1(i) = (magnitude(i))*cos(ph1(i))
c1       y1(i) = (magnitude(i))*sin(ph1(i))
         x1(i) = magnitude(i)*cos(phvh/2)
         y1(i) = magnitude(i)*sin(phvh/2)
c        x1(i) = magnitude(i)*cos(ph(i)+phvh)
cc       y1(i) = magnitude(i)*sin(ph(i)+phvh)
c        x1(i) = magnitude(i)*cos(3.14159/4)
c        y1(i) = magnitude(i)*sin(3.14159/4)
         endif
60     continue
c
c      DO GAUSSIAN FIT IF REQUESTED. Linear interpolation above is then initial guess
c      interp=3 means do GAUSSIAN
       regw=regwdth(order)   !the width of regression filter
c
        call xcor1(x,y,x,y,n,a,b)
        angle=atan2(b,a)
        ratio= abs(angle/3.1416)
c!!!!!!!!!!!!!!! ADJUSTABLE PARAMETER thres
        thres = .25    !arrived at by trial and error
        if(interp.eq.3.and.ratio.lt.thres) then
c        print *,"DO GAUSS. ratio = ",ratio
         call fitgauss(x,y,x1,y1,n,nwidth,regw,iii) !iii=2 means WN filter
c       else
c         x=xsave  !FOR TESTING NO INTERPLOLATION FOR LARGE VELCOCITIES
c         y=ysave
c         x1=x1save
c         y1=y1save
        endif
C!!!!!!!!!!!!!!!!!!
c
c DO IFFT, back to time domain
C PUT REAL AND IMAGINARY SEUQENCEWS INTO COMPLEX SEQUENCE FOR PASSING TO FFTPACK
        cts1= cmplx(x,y)
        cts2= cmplx(x1,y1)
       call CFFT1B(N,INC,CTS1,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call CFFT1B(N,INC,CTS2,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
C PUT BACK INTO REAL AND IMAGINARY
        x =  real(cts1)
        y = aimag(cts1)
        x1 = real(cts2)
        y1 =aimag(cts2)
c      call ifft2(x,y,n,m)
c      call ifft2(x1,y1,n,m)
       return
       end
c-----------------------------------------------------
C FIT A GAUSSIAN CURVE TO DATA AND ITERATE 10 TIMES
c DATA IS IN FREQUENCY DOMAIN BUT REARRANGE TO PUT
c
      subroutine fitgauss(x,y,x1,y1,n,nwidth,regw,iii)
c
      real x(n),y(n),x1(n),y1(n),f(n),f1(n),ph(n),ph1(n)
c     select data for interpolation. choose "iext" points on each side of 0-vel gap
c
c    len is the gap width, an odd number
      if(iii.eq.2) then
        len=nwidth
      else
        len=regw
      endif
c!!!!!!!!!!!!
      iext=3 ! The number of points on each side of the notch to use for interpolation
      iteration =5!10  !number of gaussian fir iterations
c!!!!!!!!!!!!
      lentot=len+2*iext    !TOTAL INTERPOLATION LENGTH. Gap plus extrapolation points
      l=lentot/2   !  l is oneside interpolation length .
C   ZERO VELOCITY IS AT INDEX 1
c
c  PREPARE DATA. GET POWER AND SAVE PHASE
c   ZERO VELOCITY IS AT INDEX 1 for X,y,x1,y1
c
      do 1 i=1,l
c HH
       f(i)= x(n-l+i)**2 +y(n-l+i)**2
       f(l+i+1) = x(i+1)**2 + y(i+1)**2
       ph(i) = atan2(y(n-l+i),x(n-l+i))
       ph(l+i+1) = atan2(y(i+1),x(i+1))
c VV
       f1(i)= x1(n-l+i)**2 +y1(n-l+i)**2
       f1(l+i+1) = x1(i+1)**2 + y1(i+1)**2
       ph1(i) = atan2(y1(n-l+i),x1(n-l+i))
       ph1(l+i+1) = atan2(y1(i+1),x1(i+1))
1     continue
c   PUT IN CENTER POINT
      f(l+1)=  x(1)**2 + y(1)**2    ! zero velocity point
      f1(l+1)=x1(1)**2 + y1(1)**2    ! zero velocity point
      ph(l+1) = atan2(y(1),x(1))
      ph1(l+1) = atan2(y1(1),x1(1))
c
C  NOW DO GAUSSIAN FIT
c
c****************
c     DO H FIT FIRST
c********************
c
       do 100 j=1,iteration   !GAUSS fit iterate
       sumh =0.0
       sumh1 = 0.0
       sumh2=0.0
c
c   GET MEAN
c
       do 5 i=1,lentot
         sumh =sumh +i*f(i)  !weighted
         sumh1=sumh1+f(i)    !scale
5      continue
        avg =sumh/sumh1
c
C GET STANDARD DEVIATION
c
       do 10 i=1,lentot
         sumh2=sumh2+f(i)*(float(i)-avg)**2
10     continue
       dev=sqrt(sumh2/sumh1)
c
c   CALCULATE GAUSSIAN FIT for CENTER len POINTS (0-vel gap points)
C    AND REPLACE CENTER len POINTs BY GAUSSIAN
       do 20 i = iext+1,iext+len
         e=-(float(i)-avg)**2/(2*dev**2)
         f(i)=exp(e)*sumh1/((2.*3.14159)**.5*dev)
20     continue
100   continue
C
C   PUT INTERPOLATED VALUES BACK into POWER SPECTRUM AND
C   GET REAL AND IMAGINARY PARTS.
C   JUST DO OVER GAP WIDTH
      lw=len/2   ! Half gap width. len should always be odd
       ! l is lentot/2 , half of interpolation length
      do 200 i=1,lw
       x(n-lw+i)= sqrt(f(l-lw+i))*cos(ph(l-lw+i)) ! rreal part
       y(n-lw+i)= sqrt(f(l-lw+i))*sin(ph(l-lw+i))
c
       x(i+1)=sqrt(f(l+i+1))*cos(ph(l+i+1))  !imaginary part
       y(i+1)=sqrt(f(l+i+1))*sin(ph(l+i+1))
200   continue
      x(1)=sqrt(f(l+1))*cos(ph(l+1)) ! center point (0 vel)
      y(1)=sqrt(f(l+1))*sin(ph(l+1))
c
c****************
c     NOW DO V FIT
c********************
c
       do 101 j=1,iteration  !iterate GAUSS FIT
       sumh =0.0
       sumh1 = 0.0
       sumh2=0.0
c
c  CALCULATE MEAN
c
       do 6 i=1,lentot
         sumh =sumh +i*f1(i)  !weighted
         sumh1=sumh1+f1(i)    !scale
6      continue
        avg =sumh/sumh1
c
C GET STANDARD DEVIATION
c
       do 11 i=1,lentot
         sumh2=sumh2+f1(i)*(float(i)-avg)**2
11     continue
       dev=sqrt(sumh2/sumh1)
c   CALCULATE GAUSSIAN FIT for CENTER len POINTS (0-vel gap points)
C    AND REPLACE CENTER len POINTs BY GAUSSIAN
       do 21 i=iext+1,iext+len
         e=-(float(i)-avg)**2/(2*dev**2)
         f1(i)=exp(e)*sumh1/((2.*3.14159)**.5*dev)
21     continue
101   continue
C
C   PUT INTERPOLATED VALUES BACK into POWER SPECTRUM AND
C   GET REAL AND IMAGINARY PARTS.
C   JUST DO OVER GAP WIDTH
      lw=len/2   ! Half gap width. len should always be odd
       ! l is  Half of interpolation length
      do 201 i=1,lw
       x1(n-lw+i)= sqrt(f1(l-lw+i))*cos(ph1(l-lw+i)) ! rreal part
       y1(n-lw+i)= sqrt(f1(l-lw+i))*sin(ph1(l-lw+i)) !imaginary part
c
       x1(i+1)=sqrt(f1(l+i+1))*cos(ph1(l+i+1))  !real part
       y1(i+1)=sqrt(f1(l+i+1))*sin(ph1(l+i+1))
201   continue
      x1(1)=sqrt(f1(l+1))*cos(ph1(l+1)) ! center point (0 vel)
      y1(1)=sqrt(f1(l+1))*sin(ph1(l+1))
c
      return
      end
c-------------------------------------------------------
c----------------------------------------------------------------
       subroutine calculate
       common  /specfilt/sigmaf,xorder,nor,onoff,dc,polyopt
        integer polyopt
       common /interppoly/interp,lenpinterp,regwdth,icmd,gain
       real gain(20), regwdth(26)
       common /nointerpvars/zdnoint,pdpnoint,rhvnoint
       real zdnoint(1024),pdpnoint(1024),rhvnoint(1024)
       common/CMDFLG/CMDflag
       character*3 onoff,dc
       integer CMDflag(1024)
       real nor(1025)
       common /offsets/vvhh_offset,vvhv_offset,hhvh_offset,zdroffset
       common /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
c2345678901234567890123456789012345678901234567890123456789012345678901
c        1         2         3         4         5         6         7
        common /radarconstants/basedbzh,basedbzv,noiseh,noisev
        common /ray/plfile,rtype
        common /labels/label
        common /radardata/rng,zh, zdr,ldrh,ldrv, ldrx, zv, dop, pvvhh,
     & r0hhvv , r0hhvh, r0vvhv, r2hhhh, r2vvvv, r1hvvh, r1vhhv, r0hvvh,
     &    r3hhvv, r3vvhh, phhvh, pvvhv, pdp3, phvvh, pvhhv,pdp_2lg,
     &    wdth_hh,wdth_vh,pdpx,r1hhvv,ro11, rx_r1hv,phvvh_dp,phvvh_vel
     &    ,phidp,phhvv,cpa,cpaV,cpaUV,cpaVU,cpaUF
       integer year,month,day,gates,hits,hour,min,sec,maxrayno
       common /raydat/range_inc,range,azim,elev,hour,min,time_inc,
     >   sec,year,month,day,hits,gates,rng_index,raycount,maxrayno
     >  ,range_ary
       integer rng_index,raycount
       complex cm(3,3)
         character label(0:54)*12
       real lin_noiseh, lin_noisev,snrh,snrv
       common/netcdf/Elevation,Azimuth,rvp8time,pulses,IHc_in,QHc_in,
     > IVc_in,QVc_in,IVx_in,QVx_in,IHx_in,QHx_in
        integer eof,npulses,ngates,jj,ii,i
      parameter (npulses=30000,ngates = 1000)
      real IHc_in(ngates,1024), QHc_in(ngates,1024)
      real IVx_in(ngates,1024), QVx_in(ngates,1024)
      real IVc_in(ngates,1024), QVc_in(ngates,1024)
      real IHx_in(ngates,1024), QHx_in(ngates,1024)
       real Ts
       real elevation(npulses), azimuth(npulses),range_ary(ngates)

        double precision rvp8time(npulses)
c2345678901234567890123456789012345678901234567890123456789012345678901
        real ui(1025),uq(1025),vi(1025),vq(1025),uvi(1025),uvq(1025),
     >    vui(1025),vuq(1025),extraI,extraQ
        real ldr,ldrh,ldrv,ldrx,ldrx1,ldrx2, plfile(-5:3000,0:54)
        real ldr1,ldr2,lambda,basedbzh,basedbzv,noiseh,noisev

        integer rtype(4)
c
        complex tmpc,tmpc1,tmpc2,j,tmpc3,tmpc4,tmpc7,tmpc8
        data pi/3.1415927/
        data rad2deg/57.29578/
        data j/(0.,1.)/

c      print *,"IN CLACULATE. rng_index=",rng_index
c      Print *,"jj,rtype",jj,rtype(1),rtype(2),rtype(3),rtype(4)
         call xcor0(vi,vq,vi,vq,nn,e,f)
         call xcor0(ui,uq,ui,uq,nn,g,h)
         varv=e
         varu=g
c        print *,"zdroff IN CAL",zdroffset
c        read(5,*)  
c
c      USING SNR MADE ZDR VERY NOISY IN LOW SNR REGIONS
c
c        lin_noiseh= 10**(noiseh/10)
c        lin_noisev= 10**(noisev/10)
c        snrh= (g-lin_noiseh)/lin_noiseh
c        if(snrh.le.0.0) then 
c           snrh=-10.
c        else
c           snrh= 10*log10(snrh)
c        endif
c        snrv= (e-lin_noisev)/lin_noisev
c        if(snrv.le.0.0) then 
c           snrv=-10.
c        else
c           snrv= 10*log10(snrv)
c        endif
c        zh= snrh + basedbzh + 20*log10(range)
c        zv= snrv + basedbzv + 20*log10(range)
         POW_HH= 10*log10(g)
         if(iwdw.eq.3) POW_HH=POW_HH+4.19 !Hanning
         if(iwdw.eq.5) POW_HH=POW_HH+5.23 !Blackman
         if(iwdw.eq.6) POW_HH=POW_HH+5.89 !Blackman-Nutall
         zh= 10*log10(g) -noiseh + basedbzh + 20*log10(range)
         zv= 10*log10(e) -noisev + basedbzv + 20*log10(range)
         if(iwdw.eq.3) zh=zh+4.19
         if(iwdw.eq.3) zv=zv+4.19
         if(iwdw.eq.5) zh=zh+5.23
         if(iwdw.eq.5) zv=zv+5.23
         if(iwdw.eq.6) zh=zh+5.89
         if(iwdw.eq.6) zv=zv+5.89
c        zh= 10*log10(g) + 20*log10(range)
c        zv= 10*log10(e)+ 20*log10(range)
c        zh= 10*log10(g)
         zdr=zh-zv-zdroffset
c
cc   Get CPA (OLD)  !CPA FOR JUST THE HH CHANNEL. EXPED TO ALL 4 CHANNELS
c        cpa_i=0.0
c        cpa_q=0.0
c        den=0.0
c        do 100 i=1,nn
c           cpa_i=cpa_i + ui(i)
c           cpa_q=cpa_q + uq(i)
c           den= den+ sqrt(ui(i)**2 + uq(i)**2)
c100     continue
c        cpa= sqrt(cpa_i**2 + cpa_q**2)
c        cpa=cpa/den
c
C   HH (u) channel
c   Get CPA 
        cpa_i=0.0
        cpa_q=0.0
        den=0.0
        do 100 i=1,nn
           cpa_i=cpa_i + ui(i)
           cpa_q=cpa_q + uq(i)
           den= den+ sqrt(ui(i)**2 + uq(i)**2)
100     continue
        cpa= sqrt(cpa_i**2 + cpa_q**2)
        cpa=cpa/den

        print *,'FILTERED********cpa= ', cpa
c
C   VV (v) channel
c
c   Get CPAV
c
        cpaV_i=0.0
        cpaV_q=0.0
        denV=0.0
        do 200 i=1,nn
           cpaV_i=cpaV_i + vi(i)
           cpaV_q=cpaV_q + vq(i)
           denV= denV+ sqrt(vi(i)**2 + vq(i)**2)
200     continue
        cpaV= sqrt(cpaV_i**2 + cpaV_q**2)
        cpaV=cpaV/denV
c
C   VH (vu) channel
c
c   Get CPAVU
c
        cpaVU_i=0.0
        cpaVU_q=0.0
        denVU=0.0
        do 300 i=1,nn
           cpaVU_i=cpaVU_i + vui(i)
           cpaVU_q=cpaVU_q + vuq(i)
           denVU= denVU+ sqrt(vui(i)**2 + vuq(i)**2)
300     continue
        cpaVU= sqrt(cpaVU_i**2 + cpaVU_q**2)
        cpaVU=cpaVU/denVU
c
C   HV (uv) channel
c
c   Get CPAUV
c
        cpaUV_i=0.0
        cpaUV_q=0.0
        denUV=0.0
        do 400 i=1,nn
           cpaUV_i=cpaUV_i + uvi(i)
           cpaUV_q=cpaUV_q + uvq(i)
           denUV= denUV+ sqrt(uvi(i)**2 + uvq(i)**2)
400     continue
        cpaUV= sqrt(cpaUV_i**2 + cpaUV_q**2)
        cpaUV=cpaUV/denUV
c
c  THE ORIGINAL CHILL DATA HAD THE V TS AS THE LEADING SAMPLE
C  AND THE ORIGINAL CALCULATION IS BASED ON THAT. SPOL DATA HAS THE 
C  H LEADING THE V AND THUS THIS NEEDS TO BE ACCOUNTED FOR BELOW
c--------
c       call xcor0(ui,uq,vi,vq,nn,a,b) !CHILL
        call xcor0(vi,vq,ui,uq,nn,a,b) !SPOL  -Vel + PHIDP
c       call xcor1(vi,vq,ui,uq,nn,c,d) !CHILL
        call xcor1(ui,uq,vi,vq,nn,c,d) !SPOL  -Vel - PHIDP
c-------- these are auto correlations lag1 so no change needed for SPOL
        call xcor1(ui,uq,ui,uq,nn,e,f) 
        call xcor1(vi,vq,vi,vq,nn,g,h)
        r2hhhh=(e**2+f**2)**.5
        r2hhhh=r2hhhh/varu
        r2vvvv=(g**2+h**2)**.5
        r2vvvv=r2vvvv/varv

        rouv=((a**2+b**2)**.5+(c**2+d**2)**.5)/2
        rouv=rouv/((varv*varu)**.5)
        r1hhvv=rouv
        ro11=(r2hhhh+ r2vvvv)/2
        rouv=rouv/(ro11**.25)
        r0hhvv=rouv
c
        phhvv= atan2(b,a)*rad2deg   ! -Vel + PHIDP
        pvvhh= atan2(d,c)*rad2deg   ! -VEL + -PHIDP
c       tmpc=cmplx(c,d)*cmplx(a,-b) !CHILL
C
C       PUT IN PHASE OFFSET ON THE TWO VVHH LAG1 COVARIANCES SO THAT
C       PHIDP STARTS AT ABOUT -70
C
C       THE TWO LAG 1 VVHH ARE:  (add offsets to put phidp at about -70deg.)
C
        tmpc = cmplx(a,b)*cexp(-j*vvhh_offset)  !velocity +phidp
        tmpc1 = cmplx(c,d)*cexp(j*vvhh_offset)   ! velocity - phidp
c
        phhvv= atan2(aimag(tmpc),real(tmpc))*rad2deg   ! -Vel + PHIDP
        pvvhh= atan2(aimag(tmpc1),real(tmpc1))*rad2deg
c
c        tmpc=cmplx(c,-d)*cmplx(a,b) !SPOL 
        tmpc2= tmpc*conjg(tmpc1)     !2PHIDP  Velocity should be eliminated
c        tmpc=tmpc*cexp(j*vvhh_offset)
c        pdp=atan2(aimag(tmpc),real(tmpc))/2
        pdp = atan2(aimag(tmpc2),real(tmpc2))/2
c       pdp=pdp*cexp(j*vvhh_offset) ! pdp is a real phase while multiplier is complex.BAD?
c       ph1=atan2(b,a)
c       ph2=atan2(d,c)
c
c       SUBTRACT PHIDP FROM THE TWO LAG1S
c
        tmpc=tmpc*conjg(csqrt(tmpc2)) !-phidp
c       tmpc1=tmpc1*(-csqrt(tmpc2))   ! NEGATIVE SIGN APPEARS INCORRECT
        tmpc1=tmpc1*(csqrt(tmpc2))    ! +phidp
c  AT THIS POINT tmpc and tmpc1 both have only the velocity and it should be negative velocity 12/5/2018
c
c      NOW GET DOPPLER VELOCITY
C
        
C        tmpc=cmplx(a,b)*cexp(j*pdp)
C        tmpc1=cmplx(c,d)*cexp(-j*pdp)
        dop=atan2(aimag(tmpc),real(tmpc))
        dop1=atan2(aimag(tmpc1),real(tmpc1))
CTEMP!!!!
c
c       phhvv= dop  ! VEL1  
c       pvvhh= dop1  ! VEL2 
C TEMP!!!!
C    Negative sign on velocity removed 15 July 2020. Velocity did not agree with Mike.
         dop=-(dop+dop1)/2
c        dop=(dop+dop1)/2   ! Negative removed 
c       dop=(ph1+ph2)/2
c       pdp=(ph2-ph1)/2
        pdp=pdp*180/pi
        phidp = pdp



        lambda=.1067 ! meters
c       Ts=.001  !Ts READ FROM NETCDF FILE
c       Ts=.001
        const=lambda/(4.*pi*Ts)
        co_dop_dg=dop*rad2deg
        dop=const*dop

c        CO-TO-CROSS PARAMETERS
c
         call xcor0(uvi,uvq,uvi,uvq,nn,a,b)
         call xcor0(vui,vuq,vui,vuq,nn,c,d)
         varuv=a
         varvu=c
         ldrh=10*log10(varvu/varu)
         ldrv=10*log10(varuv/varv)

        call xcor0(vi,vq,uvi,uvq,nn,a,b)
        xmagv= sqrt(a**2+b**2)
        xmagv=xmagv/(varuv*varv)**.5
        r0vvhv=xmagv
        tmpc=cmplx(a,b)*cexp(j*vvhv_offset)
        pvvhv= atan2(aimag(tmpc),real(tmpc))*rad2deg 
        call xcor0(ui,uq,vui,vuq,nn,a,b)
        xmagu= sqrt(a**2+b**2)
        xmagu=xmagu/(varvu*varu)**.5
        r0hhvh=xmagu
        tmpc1=cmplx(a,b)*cexp(j*hhvh_offset)
        phhvh=atan2(aimag(tmpc1),real(tmpc1))*rad2deg
        tmpc=tmpc*conjg(tmpc1)
        pdpx=atan2(aimag(tmpc),real(tmpc))
        pdpx=pdpx*rad2deg 
c
c   DO WIDTHS
c

c        r2hhhh=r2hhhh*64/63
c       temp=abs(log(r2hhhh))
        temp=abs(log((1/r2hhhh)))
        temp=lambda*sqrt(temp)
        wdth_hh=temp/(2.*1.4142*pi*Ts*2.)
    
        call xcor1(vui,vuq,vui,vuq,nn,a,b)
        r2vuvu=(a**2+b**2)**.5
        r2vuvu=r2vuvu/varvu
        temp=abs(log(r2vuvu))
        temp=lambda*sqrt(temp)
        wdth_vh=temp/(2.*1.4142*pi*Ts*2.)
c
C TEMP CALCULATION OF SPECTRUM WIDTH OF VV CHANNEL
C


         r2hhhh=r2hhhh*64/63
        temp=abs(log((1/r2hhhh)))
c       temp=log(abs(1/((r2hhhh)*(63/64))))
        temp=lambda*sqrt(temp)
        wdth_vh=temp/(2.*1.4142*pi*Ts*2.)
        print *,"WIDTHS ",raycount,rng_index,
     >  wdth_hh,wdth_vh
c
c
c   DO X TO X correlation and LDR
c
        call xcor0(vui,vuq,uvi,uvq,nn,a,b)
        call xcor1(uvi,uvq,vui,vuq,nn,c,d)
c       call xcor0(uvi,uvq,vui,vuq,nn,a,b)
c       call xcor1(vui,vuq,uvi,uvq,nn,c,d)
        tmpc=cmplx(a,b)
        tmpc1=cmplx(c,d)
        tmpc=tmpc*conjg(tmpc1)
        phvvh_dp=(atan2(aimag(tmpc),real(tmpc)))/2
        phvvh_dp= phvvh_dp*rad2deg
c       if (phvvh_dp.lt. -60) phvvh_dp=phvvh_dp+180.
c
        tmpc=cexp(j*pi/2)*cmplx(a,b)
        tmpc1=cexp(-j*pi/2)*cmplx(c,d)
        tmpc=tmpc1*conjg(tmpc)
        phvvh_cor=(atan2(aimag(tmpc),real(tmpc)))/2
c
        tmpc= cmplx(a,b)*cexp(j*phvvh_cor)*cexp(j*pi/2)
        tmpc1=cmplx(c,d)*cexp(-j*phvvh_cor)*cexp(-j*pi/2)
        dopx=atan2(aimag(tmpc),real(tmpc))
        dopx1=atan2(aimag(tmpc1),real(tmpc1))
        dopx=(dopx+dopx1)/2
        x_dop_dg=dopx*rad2deg
c
c       tmpc= cmplx(c,d)*cmplx(a,b)
c       dopx=atan2(aimag(tmpc),real(tmpc))/2
        phvvh_vel=const*dopx
        phvvh=atan2(b,a)*rad2deg
        pvhhv=atan2(d,c)*rad2deg
c
c        call xcor0(ui,uq,vi,vq,nn,a,b)
c       call xcor1(vi,vq,ui,uq,nn,c,d)
c       tmpc=cmplx(c,d)*cmplx(a,-b)
c       pdp=atan2(aimag(tmpc),real(tmpc))/2
c       tmpc=cmplx(a,b)*cexp(j*pdp)
c       tmpc1=cmplx(c,d)*cexp(-j*pdp)
c       dop=atan2(aimag(tmpc),real(tmpc))
c       dop1=atan2(aimag(tmpc1),real(tmpc1))
c        dop=(dop+dop1)/2

        r1hvvh=(a**2+b**2)**.5
        ldrx1=r1hvvh/varu
        ldrx1=ldrx1/(ro11**.25)
c
        r1hvvh=r1hvvh/(varuv*varvu)**.5
        r0hvvh=r1hvvh/(ro11**.25)
c
        r1vhhv=(c**2+d**2)**.5
        ldrx2= r1vhhv/varu
        ldrx2=ldrx2/(ro11**.25)
        r1vhhv=r1vhhv/(varuv*varvu)**.5
        r0vhhv= r1vhhv/(ro11**.25)
        r0hhvvB= r1hhvv/((r1hvvh + r1vhhv)/2)
c
        r0X=( r0hvvh+ r0vhhv)/2
        ldrx=10*log10((ldrx1+ldrx2)/2)
        ldrx1=10*log10(ldrx1)
        ldrx2=10*log10(ldrx2)
c
        call xcor1(ui,uq,vi,vq,nn,a,b)
        r3uva=(a**2+b**2)**.5/(varu*varv)**.5
        r3hhvv=r3uva
        call xcor2(vi,vq,ui,uq,nn,c,d)
        r3uvb=(c**2+d**2)**.5/(varu*varv)**.5
        r3vvhh=r3uvb
        tmpc=cmplx(c,d)*cmplx(a,-b)
        pdp3=atan2(aimag(tmpc),real(tmpc))/2
        pdp3=rad2deg*pdp3
c
        call xcor1(uvi,uvq,uvi,uvq,nn,a,b)
        r2hvvh= (a**2+b**2)**.5/(varuv)
c       write(6,*) "r2hvvh, r1hvvh", r2hvvh
        r3=(r3uva+r3uvb)/2
        den=r3+r1hhvv
        pdp_2lg=(pdp3*r3 + pdp*r1hhvv)/den

        sumc=0.
        sumx=0.
          do 30 i =1,nn
           tmpc3=cmplx(0.0,0.0)
           tmpc4=cmplx(0.0,0.0)
           tmpc7=cmplx(0.0,0.0)
           tmpc8=cmplx(0.0,0.0)


          do 31 l=1,nn-i+1
               if(i.eq.1) goto 32 ! do zerolag only

c
                tmpc3=tmpc3 +cmplx(uvi(l),uvq(l))*
     >               conjg(cmplx(vui(l+i-1),vuq(l+i-1)))
c
                tmpc4=tmpc4 + cmplx(ui(l),uq(l))*
     >               conjg(cmplx(vi(l+i-1),vq(l+i-1)))
32     continue

                tmpc7 =tmpc7 + conjg(cmplx(vui(l),vuq(l)))*
     >               (cmplx(uvi(l+i-1),uvq(l+i-1)))
c
                tmpc8 =tmpc8 + conjg(cmplx(vi(l),vq(l)))*
     >               (cmplx(ui(l+i-1),uq(l+i-1)))
                sumx= sumx + cabs(tmpc3) + cabs(tmpc7)
                sumc= sumc + cabs(tmpc4) + cabs(tmpc8)
31         continue
30     continue
        sumx=10*log10(sumx/sumc)
        rx_r1hv=sumx
c
c CONSTRUCT THE COVARIANCE MATRIX
c
          zdroff=10**(.1*zdroffset)
         cm(1,1)=varu
         cm(3,3)=varv*zdroff
         cm(2,2)= (varuv + varvu)
         cm(1,3)= r0hhvv*cexp(-j*phidp/rad2deg)*(varu*varv)**.5
         cm(3,1)=conjg(cm(1,3))
        cm(1,2)= r0hhvh*cexp(j*phhvh)*(varvu*varu)**.5*sqrt(2.)
         cm(2,1)=conjg(cm(1,2))
         cm (2,3)= r0vvhv*cexp(-j*pvvhv)*(varuv*varv)**.5*sqrt(2.)
         cm(2,3)=cm(2,3)*zdroff
          cm(3,2)=conjg(cm(2,3))
          xnorm=cabs(cm(1,1))
          cm(1,1)=cm(1,1)/xnorm
          cm(1,2)=cm(1,2)/xnorm
          cm(1,3)=cm(1,3)/xnorm
          cm(2,1)=cm(2,1)/xnorm
          cm(2,2)=cm(2,2)/xnorm
          cm(2,3)=cm(2,3)/xnorm
          cm(3,1)=cm(3,1)/xnorm
          cm(3,2)=cm(3,2)/xnorm
          cm(3,3)=cm(3,3)/xnorm
      call  eigen(cm,ldr1,ldr2,tilt,elip)
         if (ldr2.lt.ldr1) then
             ldr=ldr2
            else
             ldr=ldr1
         endif
c        if(ldr.lt.0.0.and. ldr. gt.-50.) then
c           plfile(i,61)=ldr2
c        else
c           plfile(i,61)=1e36
c        endif


c
c   LOAD THE RAY PLOT FILE
c
        print*,"in calculate. rng_index, range =",
     >           rng_index,range_ary(rng_index)
        plfile(rng_index,0)=range_ary(rng_index)
        plfile(rng_index,1)=zh 
        if((interp.ne.2.and.polyopt.eq.1.and.icmd.eq.1
     >     .and.CMDflag(rng_index).eq.1).or.(polyopt.eq.1.and.
     >      interp.ne.2))  then
           plfile(rng_index,2)=zdnoint(rng_index)
        else
           plfile(rng_index,2)=zdr
        endif
        plfile(rng_index,3)=ldrh
        plfile(rng_index,4)=ldrv
        plfile(rng_index,5)=ldrx
        plfile(rng_index,6)=zv 
        plfile(rng_index,7)=dop
        if((interp.ne.2.and.polyopt.eq.1.and.icmd.eq.1  
     >     .and.CMDflag(rng_index).eq.1).or.(polyopt.eq.1.and.
     >      interp.ne.2))  then
           plfile(rng_index,8)=pdpnoint(rng_index)
           plfile(rng_index,9)=rhvnoint(rng_index)
        else
           plfile(rng_index,8)=phidp
           plfile(rng_index,9)=r0hhvv
        endif
        plfile(rng_index,10)=r0hhvh
        plfile(rng_index,11)=r0vvhv
        plfile(rng_index,12)=r2hhhh
        plfile(rng_index,13)=r2vvvv
        plfile(rng_index,14)=r1hvvh
        plfile(rng_index,15)=r1vhhv
        plfile(rng_index,16)=r0hvvh
        plfile(rng_index,17)=r3uva 
        plfile(rng_index,18)=r3uvb
        plfile(rng_index,19)=range_ary(rng_index)
        plfile(rng_index,20)=phhvh
        plfile(rng_index,21)=pvvhv
        plfile(rng_index,22)=pdp3
        plfile(rng_index,23)=phvvh
        plfile(rng_index,24)=pvhhv
        plfile(rng_index,25)=pdp_2lg
        plfile(rng_index,26)=wdth_hh
        plfile(rng_index,27)=wdth_vh
        plfile(rng_index,28)=r0hhvvB
        plfile(rng_index,29)=pdpx
        plfile(rng_index,30)=r1hhvv
        plfile(rng_index,31)=sumx
        plfile(rng_index,38)=phvvh_dp
        plfile(rng_index,39)=phvvh_vel
        plfile(rng_index,40)=phhvv
        plfile(rng_index,41)=pvvhh
        plfile(rng_index,42)=co_dop_dg
        plfile(rng_index,43)=x_dop_dg
        plfile(rng_index,44)=ldr1
        plfile(rng_index,45)=ldr2
        plfile(rng_index,46)= tilt*rad2deg
        plfile(rng_index,47)= elip*rad2deg
        plfile(rng_index,48)=10*log10(varuv/varvu)
        plfile(rng_index,50)= POW_HH 
        plfile(rng_index,54)=real(CMDflag(rng_index))
cTEMPORAY FOR REGRESSION CLUTTER REMOVED
c       xtemp= 10**(plfile(rng_index,49)/10)
c    >         -10**(POW_HH/10)
c       plfile(rng_index,50)= 10*log10(xtemp)
c
        return
        end
c___________________________________________________________________
       subroutine loglin(np)
	real  timsr(1:1025,0:8)
       common /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
c2345678901234567890123456789012345678901234567890123456789012345678901
c        1         2         3         4         5         6         7
       real ui(1025),uq(1025),vi(1025),vq(1025),uvi(1025),uvq(1025),
     >  vui(1025),vuq(1025),lambda,extraI,extraQ
	character*6 scale(2)
        common /specplot/timsr,magorph,logorlin
	if(logorlin.eq.1) then
	   do 30 j=1,7,2
	      do 40 i=1,np+1
		 timsr(i,j)=10.**(timsr(i,j)/20)
40            continue
30         continue
        else
	   do 10 j=1,7,2
	      do 20 i=1,np+1
		 if(timsr(i,j).le..0)  timsr(i,j)=1.e-6
		 timsr(i,j)=20*log10(timsr(i,j))   !NOTE 20LOG10
20            continue
10         continue
	 endif
         return
	 end
c-------------------------------------------------------
      subroutine gettime(eof) 
c
c  THIS SUBROUTINE READS IN THE REQUESTED TIME SERIES FOR A GATE. IF AT
C THE END OF THE RAY, THE NEXT RAY IS RETRIEVED. 
        implicit none
       common /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
       real vi(1025),vq(1025),ui(1025),uq(1025),vui(1025),vuq(1025),
     >   uvi(1025),uvq(1025),extraI,extraQ
c       common/netcdf/elevation,azimuth,rvp8time,pulses,I_in,Q_in
       common/netcdf/Elevation,Azimuth,rvp8time,pulses,IHc_in,QHc_in,
     > IVc_in,QVc_in,IVx_in,QVx_in,IHx_in,QHx_in
        integer eof,gates,npulses,ngates,month,sec,jj,ii,i
      parameter (npulses=30000,ngates = 1000)
      real IHc_in(ngates,1024), QHc_in(ngates,1024)
      real IVx_in(ngates,1024), QVx_in(ngates,1024)
      real IVc_in(ngates,1024), QVc_in(ngates,1024)
      real IHx_in(ngates,1024), QHx_in(ngates,1024)
       real Ts
       real elevation(npulses), azimuth(npulses),range_ary(ngates)
       integer pulses,maxrayno,hits,day,hour,min,nkeep,iwdw
       integer year,nn
c
        double precision rvp8time(npulses)
        character*400 line
        character*4 mode
        integer continuous,raycount,rng_index
        real range_start,range_inc
        real Hpow,Vpow,lambda
        real azim,elev,range
        real time_inc
c
       common /raydat/range_inc,range,azim,elev,hour,min,time_inc,
     >  sec,year,month,day,hits,gates,rng_index,raycount,maxrayno
     >  ,range_ary
       character time*12, dummy*6,mod*10
C      First time through rayno=1, and jj=gates
       print *,"IN gettime, jj,raycount,maxray= ",jj,
     >   raycount,maxrayno
C     jj is count down index to end of ray
       if(jj.eq.0)  then ! at end of ray, then get next ray
           print *,"GETTING NEXT RAY"
           if(raycount+1.gt.maxrayno) then
             print *,"END OF FILE"
             eof=1
             return
           endif
C        GET NEXT RAY of TIME SERIES       
         raycount=raycount+1
         Print *,"in gettime BEFORE LOAD TS CALL"
         call loadnetcdf
c        elev=Elevation(raycount*hits+1) ! NOW DONE IN loadnetcdf
c        azim  = Azimuth(raycount*hits +1) ! NOW DONE IN loadnetcdf
         jj=gates      !reset gate counter
         rng_index=0
      endif
c
c     LOAD IN TIME SERIES
c
      rng_index=rng_index + 1
       range=range_ary(rng_index)
      do 100 i=1,nn
         ii= i
         vi(i) = IVc_in(rng_index,i)
         vq(i) = QVc_in(rng_index,i)
         ui(i) = IHc_in(rng_index,i)
         uq(i) = QHc_in(rng_index,i)
         vui(i)= IVx_in(rng_index,i)
         vuq(i)= QVx_in(rng_index,i)
         uvi(i)= IHx_in(rng_index,i)
         uvq(i)= QHx_in(rng_index,i)
c        print *,"I&Q",rng_index,i,
c    > IHc_in(rng_index,i),QHc_in(rng_index,i),"NKEEP =",nkeep
100   continue
c     These next two lines will present a problem if the TS length is 512
      extraI = IHc_in(rng_index,nn+i)
      extraQ = QHc_in(rng_index,nn+i)
      jj=jj-1     !counts down to end of ray
      return
      end
c---------------------------------------------------------------
c___________________________________________________________________
       subroutine windowdata
	real  timsr(1:1025,0:8),w(1025)
	character*2 scale(2)
       common /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
      real ui(1025),uq(1025),vi(1025),vq(1025),uvi(1025),uvq(1025),
     >  vui(1025),vuq(1025),lambda,extraI,extraQ
        common /specplot/timsr,magorph,logorlin
	pi=3.1415927
c       print *,'IN WINDOW NP= ',nn,"iwdw=",iwdw
	do 100 i=1,nn
	   goto (1,2,3,4,5,6),iwdw
c..................................
c          RECTANGULAR
1          return
c....................................
c          HAMMING
2          w(i)=.54-.46*cos(2*pi*(i-1)/(nn-1))
           print *,"wi",w(i)
           goto 100
c...................................
c          HANNING
3	   w(i)=.5*(1.-cos(2*pi*(i-1)/(nn-1)))
           goto 100
c...................................
c          BARTLETT
4          if(i.le.nn/2) then
	   w(i)=2.*(i-1)/(nn-1)
             else
	   w(i)=2.-(2.*(i-1)/(nn-1))
           endif
           goto 100
c....................................
c          BLACKMAN
5      w(i)=.42-.5*cos(2*pi*(i-1)/(nn-1))+.08*cos(4*pi*(i-1)/(nn-1))
           goto 100
c.....................................
C          BLACKMAN-NUTALL
6      w(i)=0.3635819 -0.4891775*cos(2*pi*(i-1)/(nn-0))+
     >                0.1365995*cos(4*pi*(i-1)/(nn-0))-
     >                0.0106411*cos(6*pi*(i-1)/(nn-0))
c............................................
100       continue
	      do 20 i=1,nn
                 ui(i)=w(i)*ui(i)
                 uq(i)=w(i)*uq(i)
                 vi(i)=w(i)*vi(i)
                 vq(i)=w(i)*vq(i)
                 uvi(i)=w(i)*uvi(i)
                 uvq(i)=w(i)*uvq(i)
                 vui(i)=w(i)*vui(i)
                 vuq(i)=w(i)*vuq(i)
20            continue
	   return
	   end
c________________________________________________________________----
c      THIS SUBROUTINE CALCULATES THE DFT's OF THE TIME SERIES, PLACES
c       THIS DATA INTO AN ARRAY 'timsr',(in mag. and phase form), AND
c       THEN REARRANGES THE DATA TO GO FROM THE NEGATIVE FOLD VELOCITY
c       TO POSITIVE FOLD VELOCITY. timsr(i,0) CONTAINES THE VELOCITY
C       VALUES FOR PLOTTIN
c
        subroutine loadtseries(nptype,ptype,np)
        real lambda
	character*6 scale(2),onoff*3,ptype(10)*15,tmp*11,dc*3
        integer polyopt
	real  timsr(1:1025,0:8),x(1025),nor(1025)
        complex uc,uc1,vc,uvc,uvc1,vuc,tmpc
        complex tmpc1,tmpc2,tmpc3,tmpc4,tmpc5,tmpc6,tmpc7,tmpc8
       common /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
       common /tssaved/xi,xq,yi,yq,xyi,xyq,yxi,yxq
c2345678901234567890123456789012345678901234567890123456789012345678901
c        1         2         3         4         5         6         7
      real xi(1025),xq(1025),yi(1025),yq(1025),xyi(1025),xyq(1025),
     +     yxi(1025),yxq(1025)
      real ai(1025),aq(1025),bi(1025),bq(1025),abi(1025),abq(1025),
     >  bai(1025),baq(1025) 
      real ui(1025),uq(1025),vi(1025),vq(1025),uvi(1025),uvq(1025),
     >   vui(1025),vuq(1025),extraI,extraQ
        common /specplot/timsr,magorph,logorlin
     +         /specfilt/sigmaf,xorder,nor,onoff,dc,polyopt
        data rad2deg/57.29578/
c---
       common /FFTPACK/wsave,work,lenwrk,lensav,lenc,inc,ier
       real wsave(3000), work(3000)
c---
      complex cts1(1025), cts2(1025),cts3(1025),cts4(1025)

c       if(dc.eq.'yes') then
c    call removedc(ui,uq,nn)
c    call removedc(vi,vq,nn)
c    call removedc(uvi,uvq,nn)
c    call removedc(vui,vuq,nn)
c        endif
c
        pi=3.1415927

10     continue
c
c       foldvel=2*lambda/(4*Ts*2)
        foldvel=lambda/(4*Ts*2) ! HH and VV TS have half Vfold for Nyquist
c       IF CROSSPSECTRA REQUESTED, double the length
        if (nptype .eq.8 .or. nptype .eq.7) then
          nn=2*nn
        vspacing=foldvel/(nn/2)
	l=0
        do 20 i=-nn/2,nn/2
	   l=l+1
           timsr(l,0)=i*vspacing
c          write(6,*) timsr(l,0)
20      continue
        nn=nn/2  !  nn retains original sequence length
        else
c       foldvel=lambda/(4*Ts*2)
        foldvel=lambda/(2*(4*Ts))
                vspacing=foldvel/(nn/2)
        l=0
        do 21 i=-nn/2,nn/2
           l=l+1
           timsr(l,0)=i*vspacing
c          write(6,*) timsr(l,0)
21      continue
        endif

c       BRANCH ACCORDING TO PLOT TYPE REQUESTED
c
c       print *, '0. (RETURN) NO CHANGE'
c       print *, '1. SPECTRA mag'
c       print *, '2. SPECTRA phase'
c       print *, '3. TIME SERIES real'
c       print *, '4. TIME SERIES imag'
c       print *, '5. TIME SERIES mag'
c       print *, '6. TIME SERIES pha'
c       print *, '7. CROSS SPECTRA mag'
c       print *, '8. CROSS SPECTRA pha'
c       print *, '8. ShhSvv* both'
c       print *, '9. FIRST LAG Spectrum Mag.'
c       print *, '10. FIRST LAG Spectrum Phase '

        if (nptype.eq.8 .or. nptype .eq.7) then
          np=2*nn
c         mp=m+1
c        get cross covariances time series
          do 30 i =1,nn
           tmpc1=cmplx(0.0,0.0)
           tmpc2=cmplx(0.0,0.0)
           tmpc3=cmplx(0.0,0.0)
           tmpc4=cmplx(0.0,0.0)
           tmpc5=cmplx(0.0,0.0)
           tmpc6=cmplx(0.0,0.0)
           tmpc7=cmplx(0.0,0.0)
           tmpc8=cmplx(0.0,0.0)
          
           do 31 j=1,nn-i+1
               if(i.eq.1) goto 32 ! do zerolag only 

                tmpc1 = tmpc1+  cmplx(ui(j),uq(j))*
     >               conjg(cmplx(vui(j+i-1),vuq(j+i-1)))
c
                tmpc2=tmpc2+cmplx(vi(j),vq(j))*
     >               conjg(cmplx(uvi(j+i-1),uvq(j+i-1)))
c
                tmpc3=tmpc3 +cmplx(uvi(j),uvq(j))*
     >               conjg(cmplx(vui(j+i-1),vuq(j+i-1)))
c
                tmpc4=tmpc4 + cmplx(ui(j),uq(j))*
     >               conjg(cmplx(vi(j+i-1),vq(j+i-1)))
32     continue        

                tmpc5 =tmpc5 + conjg(cmplx(vui(j),vuq(j)))*
     >               (cmplx(ui(j+i-1),uq(j+i-1)))
c
                tmpc6 =tmpc6 + conjg(cmplx(uvi(j),uvq(j)))*
     >               (cmplx(vi(j+i-1),vq(j+i-1)))
c
                tmpc7 =tmpc7 + conjg(cmplx(vui(j),vuq(j)))*
     >               (cmplx(uvi(j+i-1),uvq(j+i-1)))
c
                tmpc8 =tmpc8 + conjg(cmplx(vi(j),vq(j)))*
     >               (cmplx(ui(j+i-1),uq(j+i-1)))
31         continue
                timsr(nn-1+i,1)=  real(tmpc1)/nn
                timsr(nn-1+i,2)= aimag(tmpc1)/nn
                timsr(nn-1+i,3)=  real(tmpc2)/nn
                timsr(nn-1+i,4)= aimag(tmpc2)/nn
                timsr(nn-1+i,5)=  real(tmpc3)/nn
                timsr(nn-1+i,6)= aimag(tmpc3)/nn
                timsr(nn-1+i,7)=  real(tmpc4)/nn
                timsr(nn-1+i,8)= -aimag(tmpc4)/nn
                timsr(nn+1-i,1)=  real(tmpc5)/nn
                timsr(nn+1-i,2)= aimag(tmpc5)/nn
                timsr(nn+1-i,3)=  real(tmpc6)/nn
                timsr(nn+1-i,4)= aimag(tmpc6)/nn
                timsr(nn+1-i,5)=  real(tmpc7)/nn
                timsr(nn+1-i,6)= aimag(tmpc7)/nn
                timsr(nn+1-i,7)=  real(tmpc8)/nn
                timsr(nn+1-i,8)= -aimag(tmpc8)/nn

30     continue
              do 33 i=1, 8
                timsr(np,i)=cmplx(0.0,0.)
33            continue
c          do 41 i=1,np
cc         write(6,*) timsr(i,1),timsr(i,2),timsr(i,3),timsr(i,4),
cc    >  timsr(i,5),timsr(i,6), timsr(i,7),timsr(i,8)
41      continue
          else
          np=nn
           Print *,"BEFORE LOAD REAL TS. length=",nn
          do 40 i =1,nn
	        timsr(i,1)=ui(i)
                timsr(i,2)=uq(i)
      	        timsr(i,3)=vi(i)
                timsr(i,4)=vq(i)
                timsr(i,5)=uvi(i)
                timsr(i,6)=uvq(i)
                timsr(i,7)=vui(i)
                timsr(i,8)=vuq(i)
c       GET RAW TIMESERIES AND LAG IT 1
                ui(i) =xi(i+1)
                uq(i) =xq(i+1)
                vi(i) =yi(i+1)
                vq(i) =yq(i+1)
                uvi(i)=xyi(i+1)
                uvq(i)=xyq(i+1)
                vui(i)=yxi(i+1)
                vuq(i)=yxq(i+1)
40       continue
c LOAD IN LAST MEMBERS OF a,b TIME SERIES
                ui(nn)=extraI*0.0
c               ui(nn)= xi(1)
                uq(nn)=extraQ*0.0
c               uq(nn)=xq(1)
                vi(nn)=extraI
c               vi(nn)=yi(1)
                vq(nn)=extraQ
c               vq(nn)= yq(1)
                uvi(nn)=extraI*0.0
                uvi(nn)= xyi(1)
                uvq(nn)=extraQ*0.0
                uvq(nn)= xyq(1)
                vui(nn)=extraI
                vui(nn)=yxi(1)
                vuq(nn)=extraQ
                vuq(nn)= yxq(1)
C  WINDOW THE LAGGED TIME SERIES
                call windowdata
c   SAVE 1 LAG, WINDOWED TIME SERIES
               do 45 i=1,nn
                ai(i)= ui(i)
                aq(i)= uq(i)
                bi(i)= vi(i)
                bq(i)= vq(i)
                abi(i)=uvi(i)
                abq(i)=uvq(i)
                bai(i)=vui(i)
                baq(i)=vuq(i)
c   RESTORE u,v TIME SERIES
                ui(i)=timsr(i,1)
                uq(i)=timsr(i,2)
                vi(i)=timsr(i,3)
                vq(i)=timsr(i,4)
                uvi(i)=timsr(i,5)
                uvq(i)=timsr(i,6)
                vui(i)=timsr(i,7)
                vuq(i)=timsr(i,8)
45             continue
             endif
          write(6,*) "IN LOADTS BRANCH nptype=  ", nptype
         goto (100,200,300,400,500,600,700,800,900,1000), nptype
c      
c.............................
c       RAW TIMESERIES (ie, real & imag. parts)
300     continue
400     continue
        np=nn
	return
c............................
500     continue  ! mag & phase of times series requested.
600     continue
           Print *,"BEFORE LOARD MAG TS. length=",nn
        np=nn
        do 55 i=1,nn
           tmp1=timsr(i,1)
           tmp2=timsr(i,2)
           timsr(i,1)=(tmp1**2+tmp2**2)**.5
           timsr(i,2)=atan2(tmp2,tmp1)*rad2deg
c 
           tmp1=timsr(i,3)
           tmp2=timsr(i,4)
           timsr(i,3)=(tmp1**2+tmp2**2)**.5
           timsr(i,4)=atan2(tmp2,tmp1)*rad2deg
c 
           tmp1=timsr(i,5)
           tmp2=timsr(i,6)
           timsr(i,5)=(tmp1**2+tmp2**2)**.5
           timsr(i,6)=atan2(tmp2,tmp1)*rad2deg
c 
           tmp1=timsr(i,7)
           tmp2=timsr(i,8)
           timsr(i,7)=(tmp1**2+tmp2**2)**.5
           timsr(i,8)=atan2(tmp2,tmp1)*rad2deg
55     continue
        return

c SPECTRA REQUESTED
c
100     continue
200     continue
700     continue
800     continue
c       DO DFT's AND REARRANGE SPECTRA TO ACCOMADATE NEGATIVE VELOCITIES
c*******calculate log2(nn)
       if (nptype .eq. 7 .or. nptype .eq. 8) then
          n= np
         print *,'CROSS SPECTRA CALC NP=',np
       else
         n=nn
       endif
c       m=0
c       do 60 i=1,20
c           m=m+1
c           n=n/2
c           if(n.eq.1) mm=m 
c60      continue
c        n=2**mm
c        m=mm
        print *,'BEFORE FFT CALL n=',n
C TRANFER REAL AND IMAGINERY PARTS IN COMPLEX VARIABLE CTS
       do 1500 i=1,n
       cts1(i) = cmplx(timsr(i,7),timsr(i,8))
       cts2(i) = cmplx(timsr(i,1),timsr(i,2))
       cts3(i) = cmplx(timsr(i,3),timsr(i,4))
       cts4(i) = cmplx(timsr(i,5),timsr(i,6))
1500   continue
c DO FFTS
C  INITIALIZE FOR DOUBLE LENGTH CROSS COVARIANCE SEQUENCE
       call  CFFT1I(N, WSAVE, LENSAV, IER)
c
       call  CFFT1F(N,INC,CTS1,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1F(N,INC,CTS2,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1F(N,INC,CTS3,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1F(N,INC,CTS4,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
C PUT BACK INTO REAL ARRAYS
        do 1501 i=1,n     
        timsr(i,7)= real(cts1(i))
        timsr(i,8)= aimag(cts1(i))
        timsr(i,1)= real(cts2(i))
        timsr(i,2)= aimag(cts2(i))
        timsr(i,3)= real(cts3(i))
        timsr(i,4)= aimag(cts3(i))
        timsr(i,5)= real(cts4(i))
        timsr(i,6)= aimag(cts4(i))
1501    continue
c
c       call fft2(timsr(1,7),timsr(1,8),n,m)
c       call fft2(timsr(1,1),timsr(1,2),n,m)
c       call fft2(timsr(1,3),timsr(1,4),n,m)
c       call fft2(timsr(1,5),timsr(1,6),n,m)
cc       get magnitude and phases of spectra
        do 51 i=1,n
           tmp1=timsr(i,1)
           tmp2=timsr(i,2)
           timsr(i,1)=n*(tmp1**2+tmp2**2)**.5   !FACTOR OF N ADDED BECAUSE OF FFTPACK
           if(timsr(i,1).lt. 0.0001) timsr(i,1)=.0001
           timsr(i,2)=atan2(tmp2,tmp1)*rad2deg
c 
           tmp1=timsr(i,3)
           tmp2=timsr(i,4)
           timsr(i,3)=n*(tmp1**2+tmp2**2)**.5
           if(timsr(i,3).lt. 0.0001) timsr(i,3)=.0001
           timsr(i,4)=atan2(tmp2,tmp1)*rad2deg
c 
           tmp1=timsr(i,5)
           tmp2=timsr(i,6)
           timsr(i,5)=n*(tmp1**2+tmp2**2)**.5
           if(timsr(i,5).lt. 0.0001) timsr(i,5)=.0001
           timsr(i,6)=atan2(tmp2,tmp1)*rad2deg
c 
           tmp1=timsr(i,7)
           tmp2=timsr(i,8)
           timsr(i,7)=n*(tmp1**2+tmp2**2)**.5
           if(timsr(i,7).lt. 0.0001) timsr(i,7)=.0001
           timsr(i,8)=atan2(tmp2,tmp1)*rad2deg
c          write(6,*) timsr(i,1),timsr(i,2),timsr(i,3),timsr(i,4)
c    >               ,timsr(i,5),timsr(i,6),timsr(i,7),timsr(i,8)
51     continue
       goto 12345
c--------------------
900    continue  ! first lag auto cor. product of spectra
1000   continue
        n=nn
c       m=0
c       do 960 i=1,20
c           m=m+1
c           n=n/2
c           if(n.eq.1) mm=m
960      continue
c       n=2**mm
c       m=mm
        write(6,*) "GOT TO 1-LAG SECTION"
            write(6,*) timsr(1,1),timsr(1,2)
            write(6,*) timsr(1,3),timsr(1,4)
            write(6,*) timsr(1,5),timsr(1,6)
            write(6,*) timsr(1,7),timsr(1,8)
        do 967 i=2,n
        write(6,*) i, n
            write(6,*) timsr(i,1),timsr(i,2)
            write(6,*) timsr(i,3),timsr(i,4)
            write(6,*) timsr(i,5),timsr(i,6)
            write(6,*) timsr(i,7),timsr(i,8)
            write(6,*) ai(i-1),aq(i-1)
            write(6,*) bi(i-1),bq(i-1)
            write(6,*) abi(i-1),abq(i-1)
            write(6,*) bai(i-1),baq(i-1)
            
967     continue
        do 969 i=1,n
        write(6,*)"i= ", i
        write(6,*) ai(i),aq(i)
        write(6,*) bi(i),bq(i)
        write(6,*) abi(i),abq(i)
        write(6,*) bai(i),baq(i)
969     continue
        print *,'BEFORE FFT CALL n=',n
C TRANFER REAL AND IMAGINERY PARTS IN COMPLEX VARIABLE CTS
       do 1520 i=1,n
       cts1(i) = cmplx(timsr(i,7),timsr(i,8))
       cts2(i) = cmplx(timsr(i,1),timsr(i,2))
       cts3(i) = cmplx(timsr(i,3),timsr(i,4))
       cts4(i) = cmplx(timsr(i,5),timsr(i,6))
1520   continue
c DO FFTS
C  INITIALIZE IF DOUBLE LENGTH CROSS COVARIANCE SEQUENCE
       call  CFFT1I(N, WSAVE, LENSAV, IER)
c
       call  CFFT1F(N,INC,CTS1,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1F(N,INC,CTS2,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1F(N,INC,CTS3,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1F(N,INC,CTS4,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
C PUT BACK INTO REAL ARRAYS
        do 1521 i=1,n
        timsr(i,7)= real(cts1(i))
        timsr(i,8)= aimag(cts1(i))
        timsr(i,1)= real(cts2(i))
        timsr(i,2)= aimag(cts2(i))
        timsr(i,3)= real(cts3(i))
        timsr(i,4)= aimag(cts3(i))
        timsr(i,5)= real(cts4(i))
        timsr(i,6)= aimag(cts4(i))
1521    continue
c
c       call fft2(timsr(1,1),timsr(1,2),n,m)
c       call fft2(timsr(1,3),timsr(1,4),n,m)
c       call fft2(timsr(1,5),timsr(1,6),n,m)
c       call fft2(timsr(1,7),timsr(1,8),n,m)
c
C TRANFER REAL AND IMAGINERY PARTS IN COMPLEX VARIABLE CTS
       cts1 = cmplx(ai,aq)
       cts2 = cmplx(bi,bq)
       cts3 = cmplx(abi,abq)
       cts4 = cmplx(bai,baq)
c DO FFTS
       call  CFFT1F(N,INC,CTS1,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1F(N,INC,CTS2,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1F(N,INC,CTS3,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
       call  CFFT1F(N,INC,CTS4,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
C PUT BACK INTO REAL ARRAYS
        ai= real(cts1)
        aq= aimag(cts1)
        bi= real(cts2)
        bq= aimag(cts2)
        abi=real(cts3)
        abq=aimag(cts3)
        bai=real(cts4)
        baq=aimag(cts4)
c
c       call fft2(ai,aq,n,m)
c       call fft2(bi,bq,n,m)
c       call fft2(abi,abq,n,m)
c       call fft2(bai,baq,n,m)

c   SAVE THE WRAPPED PRODUCT OF LAST POINT TO FIRST POINT
c         tmpc1= cmplx(timsr(1,1),timsr(1,2))*
c     >          conjg(cmplx(timsr(n,1),timsr(n,2)))
c         tmpc2= cmplx(timsr(1,3),timsr(1,4))*
c     >          conjg(cmplx(timsr(n,3),timsr(n,4)))
c         tmpc3= cmplx(timsr(1,5),timsr(1,6))*
c     >          conjg(cmplx(timsr(n,5),timsr(n,6)))
c         tmpc4= cmplx(timsr(1,7),timsr(1,8))*
c     >          conjg(cmplx(timsr(n,7),timsr(n,8)))
cc       get magnitude and phases of first lag product of spectra
        do 951 i=1,n
         tmpc5= cmplx(timsr(i,1),timsr(i,2))
         tmpc6= cmplx(ai(i),aq(i))
         tmpc5= tmpc6*conjg(tmpc5)

           timsr(i,1)=cabs(tmpc5)
           timsr(i,2)=atan2(aimag(tmpc5),real(tmpc5))
c-------
         tmpc5= cmplx(timsr(i,3),timsr(i,4))
         tmpc6= cmplx(bi(i),bq(i))
         tmpc5= tmpc6*conjg(tmpc5)

           timsr(i,3)=cabs(tmpc5)
           timsr(i,4)=atan2(aimag(tmpc5),real(tmpc5))
c-------
         tmpc5= cmplx(timsr(i,5),timsr(i,6))
         tmpc6= cmplx(abi(i),abq(i))
         tmpc5= tmpc6*conjg(tmpc5)

           timsr(i,5)=cabs(tmpc5)
           timsr(i,6)=atan2(aimag(tmpc5),real(tmpc5))
c-------
         tmpc5= cmplx(timsr(i,7),timsr(i,8))
         tmpc6= cmplx(bai(i),baq(i))
         tmpc5= tmpc6*conjg(tmpc5)

           timsr(i,7)=cabs(tmpc5)
           timsr(i,8)=atan2(aimag(tmpc5),real(tmpc5))
c-------
C END spetrum first lag
951     continue
        do 971 i=2,n
        write(6,*) i, n, "1-lag spec"
            write(6,*) timsr(i,1),timsr(i,2)
            write(6,*) timsr(i,3),timsr(i,4)
            write(6,*) timsr(i,5),timsr(i,6)
            write(6,*) timsr(i,7),timsr(i,8)
971     continue
C   DO LAST POINT FROM SAVED PRODUCT
c       timsr(n,1)=cabs(tmpc1)
c       timsr(n,2)=atan2(aimag(tmpc1),real(tmpc1))
c       timsr(n,3)=cabs(tmpc2)
c       timsr(n,4)=atan2(aimag(tmpc2),real(tmpc2))
c       timsr(n,5)=cabs(tmpc3)
c       timsr(n,6)=atan2(aimag(tmpc3),real(tmpc3))
c       timsr(n,7)=cabs(tmpc4)
c       timsr(n,8)=atan2(aimag(tmpc4),real(tmpc4))
c
12345   continue ! common manipulation

c      REARRANGE
       do 65 j=1,8
          do 70 i=1,n
             x(i)=timsr(i,j)
70        continue
          do 80 i=1,n/2
             timsr(i,j)=x(n/2+i) 
             timsr(n/2+i,j)=x(i)
80        continue
          timsr(n+1,j)=x(n/2+1)
65     continue
c      MUST ALSO REORDER THE FILTER CURVE nor 
       if(onoff.eq.'on') then
       do 85 i=1,n
	  x(i)=nor(i)
85     continue
       do 90 i=1,n/2
	  nor(i)=x(n/2+i)
	  nor(n/2+i)=x(i)
90     continue
       nor(n+1)=x(n/2+1)
       endif
c  IF LOG PLOTS ARE SPECIFIED (logorlin=2) THEN GO TAKE LOG.
       if(logorlin.eq.2) then
	  call loglin(np)   !WHY NP HERE?
       endif
C   IF FFTPACK IS INITIALIZED TO DOUBL:E LENGTH, INTITALIZE TO SINGLE LENGTH 
       call  CFFT1I(NN, WSAVE, LENSAV, IER)
       return
c............................................
c
c      COVARIANCE TYPE REQUESTED
c700    continue
c600    continue
c       do 801 i=1,n
c	  uc=  cmplx(ui(i),uq(i))
c	  uc1= cmplx(ui(i+1),uq(i+1))
c	  vc=  cmplx(vi(i),vq(i))
c	  uvc= cmplx(uvi(i),uvq(i))
c	  uvc1=cmplx(uvi(i+1),uvq(i+1))
c	  vuc= cmplx(vui(i),vuq(i))
c	  timsr(i,1)=real(vc*conjg(uc))
c	  timsr(i,2)=aimag(vc*conjg(uc))
c	  timsr(n+i,1)=real(uc1*conjg(vc))
c	  timsr(n+i,2)=aimag(uc1*conjg(vc))
c
cc         next 2 lines are co-to-cross 0-lag
c	  timsr(i,3)=real(vuc*conjg(vc))
c	  timsr(i,4)=aimag(vuc*conjg(vc))
c
c	  timsr(i,5)=real(vuc*conjg(uvc))
c	  timsr(i,6)=aimag(vuc*conjg(uvc))
c	  timsr(n+i,5)=real(uvc1*conjg(vuc))
c	  timsr(n+i,6)=aimag(uvc1*conjg(vuc))
c
c	  timsr(i,7)=real(vuc*conjg(uc))
c	  timsr(i,8)=aimag(vuc*conjg(uc))
c	  timsr(n+i,7)=real(uc1*conjg(vuc))
c	  timsr(n+i,8)=aimag(uc1*conjg(vuc))
801    continue
c.............................................
       
       return
       end
c_________________________________________________________________
c
      subroutine plotspec(ptype,nptype,ihard,np)
        parameter (npulses=30000,ngates = 1000)
      CHARACTER*16 date,time*20,bottomlb*8,ptype(10)*15,tmp*6
       CHARACTER*18 stormlb*14,rang*19,zhzv*23
      CHARACTER*25 scale(2)*6
	character onoff*3,dc*3
       character  azth*26,rcount*30,ldrs*85
	real wndow(4,4),lambda,ldrh,ldrv, ldrx
	real  timsr(1:1025,0:8)
       integer polyopt
        integer raycount,rng_index
        character hardxgks*70,xlab*10,ylab*10,corrs*78,slgth*14
        common /hardcopy/hardxgks,scale
       common  /specfilt/sigmaf,xorder,nor,onoff,dc,polyopt
       common /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
      real ui(1025),uq(1025),vi(1025),vq(1025),uvi(1025),uvq(1025),
     >   vui(1025),vuq(1025),nor(1025),extraI,extraQ
        common /specplot/timsr,magorph,logorlin
	 common /specparms/spec4,spec1
        common /radardata/rng,zh, zdr,ldrh,ldrv, ldrx, zv, dop, pvvhh,
     & r0hhvv, r0hhvh, r0vvhv, r2hhhh, r2vvvv, r1hvvh, r1vhhv, r0hvvh,
     &    r3hhvv, r3vvhh, phhvh, pvvhv, pdp3, phvvh, pvhhv,pdp_2lg
     &    ,wdth_hh,wdth_vh,pdpx,r1hhvv,ro11, rx_r1hv,phvvh_dp,phvvh_vel
     &    ,phidp,phhvv,cpa,cpaV,cpaUV,cpaVU,cpaUF

       integer year,month,day,gates,hits,hour,min,sec,maxrayno
       common /raydat/range_inc,range,azim,elev,hour,min,time_inc,
     >  sec,year,month,day,hits,gates,rng_index,raycount,maxrayno
     >  ,range_ary
      real range_ary(ngates)
      integer trparms,tlparms,blparms,brparms,reset,scatprm
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
      real gwnd(4,4)

c
        real spec4(350),spec1(350)
      data (wndow(i,1), i=1,4) /0.02, .48, 0.52 ,0.98/
      data (wndow(i,2), i=1,4) /0.52, .98, 0.52 ,.98/
      data (wndow(i,3), i=1,4) /0.02, 0.48, 0.02 ,0.48/
      data (wndow(i,4), i=1,4) /0.52, .98, 0.02 ,0.48/
c      reset  the plot parameters
      call agrstr(reset)
      rewind(reset)

       call chkst1(ihard)
       print *,"1"
c
c       print *, '0. (RETURN) NO CHANGE'
c       print *, '1. SPECTRA mag'
c       print *, '2. SPECTRA phase'
c       print *, '3. TIME SERIES real'
c       print *, '4. TIME SERIES imag'
c       print *, '5. TIME SERIES mag'
c       print *, '6. TIME SERIES pha'
c       print *, '7. CROSS SPECTRA mag'
c       print *, '8. CROSS SPECTRA pha'

c      SET FLAG FOR MAG OR PHASE
     
       magorph=0
       md=mod(nptype,2)
       if(md.eq.0) magorph=1
       

cc     SET CORRECT LENGTH FOR COVARIANCE PLOTTING

c      if(ptype(nptype).eq.'ShhSvv* both') then
c         lgth=2*nn-1
c      else
	  lgth=np
          length=np
       print *,"in plotspec LGTH",lgth
c      endif
c       goto (1(100,200,300,400,500,600,700,800), nptype
C
c  FIRST WINDOW
c
c	POSITION THE GRAPH WINDOW
C
        tmp=ptype(nptype)
	call agsetp ('GRAPH WINDOW.',wndow(1,1),4)
       print *,"2"
c   
      call agstup(timsr(1,0) ,1,0,lgth,1,timsr(1,1+magorph),
     +            1,0,lgth,1)
       print *,"3"
      call agback
      call agcurv(timsr(1,0),1,timsr(1,1+magorph),1,length,1)
       print *,"4"

c       if(tmp.ne.'ShhSvv') then
c          call ezxy (timsr(1,0),timsr(1,1+magorph),nn+1,0)
c       else
c...     aginit initializes plot parms. It calls agstup.
cc         call aginit(2,timsr(1,1),timsr(1,2),
cc     +                   lgth,1,lgth,0,iivx,iiex,iivy,iiey)
ccc...    agback plot the background
cc         call agseti ('LABEL/CONTROL.',0)
cc         call agback
ccC...   POINTS PUTS PLOTTING FILE INTO BUFFER.
cc        call points(timsr(1,1),timsr(1,2),lgth,'+',' $')
cc        endif
c      ..............................
c
c*****   set up 2nd graph
c 
	call agsetp ('GRAPH WINDOW.',wndow(1,2),4)
      call agstup(timsr(1,0) ,1,0,lgth,1,timsr(1,3+magorph),
     +            1,0,lgth,1)
      call agback
      call agcurv(timsr(1,0),1,timsr(1,3+magorph),1,length,1)


c       if(tmp.ne.'ShhSvv') then
c	   call ezxy (timsr(1,0),timsr(1,3+magorph),nn+1,0)
c        else
c...     aginit initializes plot parms. It calls agstup.
c         call aginit(2,timsr(1,3),timsr(1,4),
c     +                   nn,1,nn,0,iivx,iiex,iivy,iiey)
cc...    agback plot the background
c         call agseti ('LABEL/CONTROL.',0)
c         call agback
cC...   POINTS PUTS PLOTTING FILE INTO BUFFER.
c        call points(timsr(1,3),timsr(1,4),nn,'+',' $')
c        endif
c        print *,'n=',nn
c      ..............................
c
c******* set  window on bottom left of the page *******************
C
	call agsetp ('GRAPH WINDOW.',wndow(1,3),4)
      call agstup(timsr(1,0) ,1,0,lgth,1,timsr(1,5+magorph),
     +            1,0,lgth,1)
      call agback
      call agcurv(timsr(1,0),1,timsr(1,5+magorph),1,length,1)

c	if(tmp.ne.'ShhSvv') then
c	   call ezxy (timsr(1,0),timsr(1,5+magorph),nn+1,0)
c        else
cc...     aginit initializes plot parms. It calls agstup.
c         call aginit(2,timsr(1,5),timsr(1,6),
c     +                   lgth,1,lgth,0,iivx,iiex,iivy,iiey)
cc...    agback plot the background
c         call agseti ('LABEL/CONTROL.',0)
c         call agback
cC...   POINTS PUTS PLOTTING FILE INTO BUFFER.
c        call points(timsr(1,5),timsr(1,6),lgth,'+',' $')
c        endif
c      ..............................
c
c*****   set up 4th graph s
	call agsetp ('GRAPH WINDOW.',wndow(1,4),4)
      call agstup(timsr(1,0) ,1,0,lgth,1,timsr(1,7+magorph),
     +            1,0,lgth,1)
      call agback
      call agcurv(timsr(1,0),1,timsr(1,7+magorph),1,length,1)

c	if(tmp.ne.'ShhSvv') then
c            call ezxy (timsr(1,0),timsr(1,7+magorph),nn+1,0)
c        else
cc...     aginit initializes plot parms. It calls agstup.
c         call aginit(2,timsr(1,7),timsr(1,8),
c     +                   lgth,1,lgth,0,iivx,iiex,iivy,iiey)
cc...    agback plot the background
c         call agseti ('LABEL/CONTROL.',0)
c         call agback
cC...   POINTS PUTS PLOTTING FILE INTO BUFFER.
c        call points(timsr(1,7),timsr(1,8),lgth,'+',' $')
c        endif
c      ..............................
c..IF DATA HAS BEEN FILTERED, PLOT THE FILTER CURVE ON ONE PLOT
       if(onoff.eq.'on' .and. ntype .ne.7 .and.
     >       nptype .ne. 8) then
	   print *,'in if before norm plot'
c          TURN OFF BACKGROUND

	   call agseti('BACKGROUND TYPE.',4)
	   call agsetp ('GRAPH WINDOW.',wndow(1,1),4)
           call ezxy (timsr(1,0),nor,nn+1,0)
       endif
       write(azth,417)azim,elev
417    format('AZ:',f8.3,' ELEV: ',f8.3)
c      write(rang,421)range
       write(rang,421) range_ary(rng_index)
421    format('Range (km) ', f7.3)
       write(date,423) year,month,day
423    format("DATE ",i4,1x,i2,1x,i2)
       write(time,424)hour,min,sec,time_inc
424    format(i2,":",i2,":",i2," inc",f7.3)
c       write(time,424)hour,min,sec
c424    format(i2,":",i2,":",i2)
       write(rcount,425) raycount,rng_index
425    format("RAY NUMBER ",i4," Rng Index",i4)
       write(corrs,426) r0hhvv,r2hhhh,r0hhvh,r0vvhv,r1hvvh
426    format("r0hhvv ",f5.3,"   r2hhhh ",f5.3,"   r0hhvh ",f5.3, 
     >        "   r0vvhv ", f5.3,"   r1hvvh ",f5.3)
       write(ldrs,427)ldrh,ldrx,rx_r1hv,wdth_hh,wdth_vh,cpa
427    format("LDRh ",f6.2,"  LDRx ",f6.2,"  LDR_R1 ",f6.2,
     >"  WDTH_hh  ",f5.2,"  WDTH_hv ",f5.2,"  CPA ",f6.3)
       write(slgth,428) nn
428    format("Seq. Len. ",i4)
       write(zhzv,429) zh,zv
429    format("Ph ",f7.2," Pv ",f7.2)
c2345678901234567890123456789012345678901234567890123456789012345678901
       call set(0.,1.,0.,1.,0.,1.,0.,1.,1)
        if (nptype .eq. 7 .or. nptype .eq. 8) then
            call plchmq(.25,.97,'HHVH',.011,0.,-1.)
            call plchmq(.75,.97,'VVHV',.011,0.,-1.)
            call plchmq(.25,.47,'HVVH',.011,0.,-1.)
            call plchmq(.75,.47,'HHVV',.011,0.,-1.)

            else
            call plchmq(.25,.97,'HH',.011,0.,-1.)
            call plchmq(.75,.97,'VV',.011,0.,-1.)
            call plchmq(.25,.47,'HV',.011,0.,-1.)
            call plchmq(.75,.47,'VH',.011,0.,-1.)
         endif

       call plchmq(.01,.99,date,.011,0.,-1.)
       call plchmq(.20,.99,time,.011,0.,-1.)
       call plchmq(.51,.99,azth,.011,0.,-1.)
       call plchmq(.65,.52,rang,.010,0.,-1.)
       call plchmq(.3,.52,rcount,.011,0.,-1.)
       call plchmq(.03,.52,ptype(nptype),.012,0.,-1.)
       call plchmq(.03,.01,corrs,.011,0.,-1.)
       call plchmq(.03,.04,ldrs,.011,0.,-1.)
       call plchmq(.73,.50,slgth,.011,0.,-1.)
       call plchmq(.45,.49,zhzv,.011,0.,-1.)

       call chkst2(ihard)
       call sflush
       return
       end
c-------------------------------------------------------------------

        subroutine chkst1(ihard)

cccc  CHECK THE BEGINING STATES OF ihard

         call GCLRWK(2,0)
         if(ihard.eq.1) then
cccc        OPEN Metafile Workstation
c           Call GOPWK(1,2,1)
            Call GOPWK(1,2,20)  !for ps hard copy gmeta.ps
            Call GACWK(1)

         print *,'---->  BEGINING of HARDCOPY with XGKS'
         endif

      return
      end

c-----------------------------------------
        subroutine chkst2(ihard)

cccc  CHECK THE END STATES OF ihard

        character hardxgks*70
        CHARACTER scale(2)*6
        common /hardcopy/hardxgks,scale
c
        if (ihard.eq.1) then
cccc        CLOSE Metafile Workstation
            Call GDAWK(1)
            Call GCLWK(1)
            call system(hardxgks)
            print *,hardxgks
         endif
      print *,' '

      return
      end
c______________________________________________________
      FUNCTION TTINP(N)
C
C        PERMITS THE INPUT OF NUMERICAL VALUES WITH ERROR RECOVERY
C
      DATA ITT/5/
    1 CONTINUE
      READ (ITT,101,ERR=90) VAL
  101 FORMAT(F10.0)
      TTINP=VAL
      RETURN
C
C       ERROR DETECTED ON INPUT
C
   90 CONTINUE
      print 200
  200 FORMAT(' +++  INPUT CONVERSION ERROR  +++')
      print 201
  201 FORMAT('$RE-TYPE THE NUMBER: ')
      GO TO 1
      END
c_______________________________________________________________
	subroutine xcor0(x,y,x1,y1,n,a,b)
	real x(1025),y(1025),x1(1025),y1(1025)
        sumr=0.0
	sumi=0.0
c
c*****  (x + jy)*conjg(x1 + jy1)
c
	do 16 i=1,n
	   sumr=sumr+x(i)*x1(i)+y(i)*y1(i)
	   sumi=sumi+y(i)*x1(i)-x(i)*y1(i)
16      continue
        a=sumr/n
        b=sumi/n
	return
	end
c--------------------------------------------------
	subroutine xcor1(x,y,x1,y1,n,a,b)
	real x(n),y(n),x1(n),y1(n)
c****** get variances ************
        sumr=0.0
	sumi=0.0
c*****   {x(i+1)+jy(i+1)}*{x1(i)-jy1(i)}
	do 16 i=1,n-1
	   sumr=sumr+x(i+1)*x1(i)+y(i+1)*y1(i)
	   sumi=sumi+y(i+1)*x1(i)-x(i+1)*y1(i)
16      continue
	a=sumr/(n)
	b=sumi/(n)
	return
	end
c------------------------------------------------------------------
       subroutine filt(xi,xq,yi,yq,xyi,xyq,yxi,yxq,n,rng)
        real xi(1025),xq(1025),yi(1025),yq(1025),xyi(1025),xyq(1025),
     > yxi(1025),yxq(1025)
        real nor(1025),mean,mag 
	character*3 onoff,dc
       integer polyopt
	 common  /specfilt/sigma,xorder,nor,onoff,dc,polyopt
c---
       common /FFTPACK/wsave,work,lenwrk,lensav,lenc,inc,ier
       real wsave(3000), work(3000)
      complex cts(1025) ! data
c---

        pi=3.1415927
c******** GET DOPPLER ANGLE ****************
c..If rmdop2 was called first, doppler angle shouldn't be calculatd
c... by pulse pair. One correlation gives phi-dp the other gives
c.. velocity plus a phi-dp. Could work. Should use simple first lag
c.. of autocorrelation.
c       call removedc(xi,xq,n)
c       call removedc(yi,yq,n)
        call xcor0(yi,yq,xi,xq,n,a,b)
        call xcor1(xi,xq,yi,yq,n,c,d)
c       call xcor1(xi,xq,xi,xq,n,c,d)
c       call xcor1(yi,yq,yi,yq,n,c,d)
        ph1=atan2(b,a)
        ph2=atan2(d,c)
        dop=(ph1+ph2)
        pdp=(ph1-ph2)/2
c
c*******calculate log2(n)
        m=0
	nn=n
        do 3 i=1,30
            m=m+1
            nn=nn/2
            if(nn.eq.1) mm=m
3      continue
        n=2**mm
        m=mm
c
c      sigma=float(n/6)
       if(dop.gt.2*pi) dop=dop-2*pi
       if(dop.lt.0.) dop=dop+2*pi
       mean=(dop*(n+1))/(2*pi)
       n1=int(mean)
       n2=int(mean+1.)
       if(mean.eq.float(int(mean))) mean=mean+.1
c      c=1/(((2*pi)**.5)*sigma)
       c=1.0
       do 10 i=1,n/2
          z=float(n1-i+1)
	  if(n1-i+1.lt.1) then
	     l=n1-i+1+n
          else
	     l=n1-i+1
          endif
          nor(l)=c*exp(-.5*abs((((z-mean)/sigma)**xorder)))
	  z1=float(n2+i-1)
	     if(n2+i-1.gt.n) then
		l1=n2+i-1-n 
             else
		l1=n2+i-1
             endif
          nor(l1)=c*exp(-.5*abs((((z1-mean)/sigma)**xorder)))
10     continue
c..FILTER THE MAGNITUDES
c----------------------------
C PUT REAL AND IMAGINARY SEUQENCES INTO COMPLEX SEQUENCE FOR PASSING TO FFTPACK
        cts= cmplx(xi,xq)
       call CFFT1F(N,INC,CTS,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
        xi =  real(cts)
        xq = aimag(cts)
c      call fft2(xi,xq,n,m)
       do 20 i=1,n
         mag=(xi(i)**2+xq(i)**2)**.5
         mag=mag*nor(i)
         ph=atan2(xq(i),xi(i))
         xi(i)=mag*cos(ph)
         xq(i)=mag*sin(ph)
20     continue
c..INVERSE
        cts= cmplx(xi,xq)
       call CFFT1B(N,INC,CTS,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
        xi =  real(cts)
        xq = aimag(cts)
c      call ifft2(xi,xq,n,m)
c
c----------------------------
c..DO  yi yq
        cts= cmplx(yi,yq)
       call CFFT1F(N,INC,CTS,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
        yi =  real(cts)
        yq = aimag(cts)
c      call fft2(yi,yq,n,m)
c..FILTER THE MAGNITUDE
       do 30 i=1,n
         mag=(yi(i)**2+yq(i)**2)**.5
         mag=mag*nor(i)
         ph=atan2(yq(i),yi(i))
         yi(i)=mag*cos(ph)
         yq(i)=mag*sin(ph)
30     continue
c..INVERSE
        cts= cmplx(yi,yq)
       call CFFT1B(N,INC,CTS,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
        yi =  real(cts)
        yq = aimag(cts)
c      call ifft2(yi,yq,n,m)
c
c----------------------------
c..DO xyi xyq
        cts= cmplx(xyi,xyq)
       call CFFT1F(N,INC,CTS,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
        xyi =  real(cts)
        xyq = aimag(cts)
c      call fft2(xyi,xyq,n,m)
c..FILTER THE MAGNITUDE
       do 40 i=1,n
         mag=(xyi(i)**2+xyq(i)**2)**.5
         mag=mag*nor(i)
         ph=atan2(xyq(i),xyi(i))
         xyi(i)=mag*cos(ph)
         xyq(i)=mag*sin(ph)
40     continue
c..INVERSE
        cts= cmplx(xyi,xyq)
       call CFFT1B(N,INC,CTS,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
        xyi =  real(cts)
        xyq = aimag(cts)
c      call ifft2(xyi,xyq,n,m)
c----------------------------
c..DO  yxi yxq
        cts= cmplx(yxi,yxq)
       call CFFT1F(N,INC,CTS,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
        yxi =  real(cts)
        yxq = aimag(cts)
c      call fft2(yxi,yxq,n,m)
c..FILTER THE MAGNITUDE
       do 50 i=1,n
         mag=(yxi(i)**2+yxq(i)**2)**.5
         mag=mag*nor(i)
         ph=atan2(yxq(i),yxi(i))
         yxi(i)=mag*cos(ph)
         yxq(i)=mag*sin(ph)
50     continue
c..INVERSE
        cts= cmplx(yxi,yxq)
       call CFFT1B(N,INC,CTS,LENC,WSAVE,LENSAV,WORK,LENWRK,IER)
        yxi =  real(cts)
        yxq = aimag(cts)
c       call ifft2(yxi,yxq,n,m)
c      print *,"AT END OF FILT", n,m
       return
       end
c------------------------------------------------------
        subroutine fft2(ar,ai,n,m)
        real ar(1025),ai(1025)
	l=2**m
	if(l.ne.n) then
	    write(6,*) '2log(length) not equal m'
	    stop
        endif
        nv2=n/2
        nm1=n-1
        j=1
        do 30 i=1,nm1
            if(i .ge. j)goto 10
                tr=ar(j)
                ti=ai(j)
                ar(j)=ar(i)
                ai(j)=ai(i)
                ar(i)=tr
                ai(i)=ti
10          k=nv2
20          if (k .ge. j)goto 30
                j=j-k
                k=k/2
            goto 20
30      j=j+k
        pi=3.14159265359793
        do 50 l=1,m
            le=2**l
            le1=le/2
100         format('le=',i6,'i=',i4)
            ur=1.0
            ui=0.0
            wr= cos(pi/le1)
            wi=-sin(pi/le1)
            do 50 j=1,le1
                do 40 i=j,n,le
                    ip=i+le1
                    tr=ar(ip)*ur-ai(ip)*ui
                    ti=ai(ip)*ur+ar(ip)*ui
                    ar(ip)=ar(i)-tr
                    ai(ip)=ai(i)-ti
                    ar(i)=ar(i)+tr
                    ai(i)=ai(i)+ti
40              continue
                tr=ur*wr-ui*wi
                ti=ui*wr+ur*wi
                ur=tr
                ui=ti
50          continue
            return
            end
c----------------------------------------------------------------
        subroutine ifft2(ar,ai,n,m)
        real ar(1025),ai(1025)
        nv2=n/2
        nm1=n-1
        j=1
        do 30 i=1,nm1
            if(i .ge. j)goto 10
                tr=ar(j)
                ti=ai(j)
                ar(j)=ar(i)
                ai(j)=ai(i)
                ar(i)=tr
                ai(i)=ti
10          k=nv2
20          if (k .ge. j)goto 30
                j=j-k
                k=k/2
            goto 20
30      j=j+k
        pi=3.14159265359793
        do 50 l=1,m
            le=2**l
            le1=le/2
100         format('le=',i6,'i=',i4)
            ur=1.0
            ui=0.0
            wr= cos(pi/le1)
            wi= sin(pi/le1)
            do 50 j=1,le1
                do 40 i=j,n,le
                    ip=i+le1
                    tr=ar(ip)*ur-ai(ip)*ui
                    ti=ai(ip)*ur+ar(ip)*ui
                    ar(ip)=ar(i)-tr
                    ai(ip)=ai(i)-ti
                    ar(i)=ar(i)+tr
                    ai(i)=ai(i)+ti
40              continue
                tr=ur*wr-ui*wi
                ti=ui*wr+ur*wi
                ur=tr
                ui=ti
50          continue
            do 60 i=1,n
		ar(i) = ar(i)/n
		ai(i) = ai(i)/n
60	    continue
	    return
            end
c----------------------------------------------------------
c 	THIS SUBROUTINE REMOVES THE DC COMPONET OF THE PASSED SEQUENCE.
c
	subroutine removedc(x,y,n)
	real x(1025),y(1025)
	sum1=0.0
	sum2=0.0
	do 10 i=1,n
	   sum1=sum1+x(i)
	   sum2=sum2+y(i)
10	continue
	sum1=sum1/n
	sum2=sum2/n
	do 20 i=1,n
	   x(i)=x(i)-sum1
	   y(i)=y(i)-sum2
20	continue
	return
	end
c------------------------------------------------
	subroutine xcor2(x,y,x1,y1,n,a,b)
	real x(n),y(n),x1(n),y1(n)
c****** get variances ************
        sumr=0.0
	sumi=0.0
c*****   {x(i+1)+jy(i+1)}*{x1(i)-jy1(i)}
	do 16 i=1,n-2
	   sumr=sumr+x(i+2)*x1(i)+y(i+2)*y1(i)
	   sumi=sumi+y(i+2)*x1(i)-x(i+2)*y1(i)
16      continue
	a=sumr/(n)
	b=sumi/(n)
	return
	end
c---------------------------------------------------------------------
      subroutine resetparms


      integer trparms,tlparms,blparms,brparms,reset,scatprm
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
      real gwnd(4,4)

        call agrstr(reset)
        rewind(reset)

        call agsetf('FRAME.',2.)
        call toplprm
        call toprprm
        call botlprm
        call botrprm

        return
        end

c___________________________________________________________________

      subroutine toplprm


      integer trparms,tlparms,blparms,brparms,reset,scatprm
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
      real gwnd(4,4)

       common/dashed/dshv
      character*16 dshv(4),xlab*19,ylab*10
        call agrstr(reset)
        rewind(reset)

        call agsetf('TOP/WIDTH.',wdth2)
        call agsetf ('WINDOW',1.)
        call agsetp ('GRAPH.',gwnd(1,1),4)
        call agsetf('BOTTOM/WIDTH.',wdth2)
        call agsetf('AXIS/LEFT/NUMERIC/WIDTH.',wdth2)
        call agseti('LABEL/CONTROL.',0)
        call agsetf ('LEFT/CONTROL.',1.)
        call agsetf ('BOTTOM/CONTROL.',1.)
        call agseti('DASH/SELECTOR.',4)
        call agseti('DASH/LENGTH.',16)
        call agsetc('DASH/PATTERN/1.',dshv(1))
        call agsetc('DASH/PATTERN/2.',dshv(2))
        call agsetc('DASH/PATTERN/3.',dshv(3))
        call agsetc('DASH/PATTERN/4.',dshv(4))
cccc    TURN ON BACKGROUND

        call agseti('BACKGROUND TYPE.',1)
cccc    SUPPRESS THE RIGHT AXIS SO THAT TWO PLOTS CAN USE SAME ABSCISSA.

        call agseti('RIGHT/CONTROL.',0)

cccc    STORE THE PLOT PARAMETERS
        xlab=""
        ylab=""
        call anotat(xlab,ylab,0,0,0,0)

        call agsave(tlparms)
        rewind(tlparms)

        return
        end
c___________________________________________________________________
cccc    PLOT USES RIGHT VERTICAL AS ORDINATE AXIS

        subroutine toprprm


       common/dashed/dshv
      character*16 dshv(4),xlab*19,ylab*10
      integer trparms,tlparms,blparms,brparms,reset,scatprm
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
      real gwnd(4,4)

        call agrstr(reset)
        rewind(reset)

cccc    SET UP 2nd GRAPH ON SAME AXIS
        call agseti('BACKGROUND TYPE.',4)
        call agsetf ('RIGHT/CONTROL.',1.)
        call agsetf ('BOTTOM/CONTROL.',1.)
        call agseti ('LEFT/CONTROL.',0)
        call agseti ('TOP/CONTROL.',0)


        call agsetf ('WINDOW',1.)
        call agsetp ('GRAPH.',gwnd(1,1),4)

c       call agsetf('AXIS/BOTTOM/NUMERIC/WIDTH.',wdth2)
c       call agsetf('AXIS/TOP/NUMERIC/WIDTH.',wdth2)
        call agseti ('LABEL/CONTROL.',0)
        call agseti ('DASH/SELECTOR.',4)
        call agseti ('DASH/LENGTH.',16)
        call agsetc ('DASH/PATTERN/1.',dshv(1))
        call agsetc ('DASH/PATTERN/2.',dshv(2))
        call agsetc ('DASH/PATTERN/3.',dshv(3))
        call agsetc ('DASH/PATTERN/4.',dshv(4))
c       call agsetf('TOP/MAJOR/COUNT.',4.)
c       call agsetf('BOTTOM/MAJOR/COUNT.',4.)

cccc    TURN ON NUMERIC LABELS

        call agsetf ('BOTTOM/NUMERIC/TYPE.',0.) !Turn off bottom labels
        call agsetf ('RIGHT/NUMERIC/TYPE.',1.E36) !Scientific notation
        call agsetf('AXIS/RIGHT/NUMERIC/WIDTH.',wdth2)
        xlab=""
        ylab=""
        call anotat(xlab,ylab,0,0,0,0)

        call agsave(trparms)
        rewind(trparms)

        return
        end
c___________________________________________________________________
        subroutine botlprm

       common/dashed/dshv
      character*16 dshv(4),xlab*19,ylab*10

      integer trparms,tlparms,blparms,brparms,reset,scatprm
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
      real gwnd(4,4)

cccc    SET 2nd WINDOW ON BOTTOM OF THE PAGE

        call agrstr(reset)
        rewind(reset)

cccc    TURN ON BACKGROUND
        call agseti('BACKGROUND TYPE.',1)

cccc    SUPPRESS THE RIGHT AXIS SO THAT TWO PLOTS CAN USE SAME ABSCISSA.
        call agsetf ('WINDOW',1.)
        call agsetp ('GRAPH.',gwnd(1,2),4)
        call agseti ('LABEL/CONTROL.',0)
        call agsetf ('LEFT/CONTROL.',1.)
        call agsetf ('BOTTOM/CONTROL.',1.)
        call agseti ('DASH/SELECTOR.',4)
        call agseti ('DASH/LENGTH.',16)
        call agsetc ('DASH/PATTERN/1.',dshv(1))
        call agsetc ('DASH/PATTERN/2.',dshv(2))
        call agsetf('AXIS/BOTTOM/NUMERIC/WIDTH.',wdth2)
        call agsetf('AXIS/TOP/NUMERIC/WIDTH.',wdth2)
        call agsetf('AXIS/LEFT/NUMERIC/WIDTH.',wdth2)
cccc    SUPPRESS THE RIGHT AXIS SO THAT TWO PLOTS CAN USE SAME ABSCISSA.
        call agseti('RIGHT/CONTROL.',0)
c
        xlab=""
        ylab=""
        call anotat(xlab,ylab,0,0,0,0)

        call agsave(blparms)
        rewind(blparms)

        return
        end
c___________________________________________________________________

        subroutine botrprm

       common/dashed/dshv
      character*16 dshv(4),xlab*19,ylab*10

      integer trparms,tlparms,blparms,brparms,reset,scatprm
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
      real gwnd(4,4)

cccc    SET UP 2nd GRAPH ON SAME AXIS
        call agrstr(reset)
        rewind(reset)

        call agseti('BACKGROUND TYPE.',4)

cccc    TURN ON RIGHT PRIMARY
        call agsetf ('RIGHT/CONTROL.',1.)
        call agsetf ('BOTTOM/CONTROL.',1.)
        call agseti ('LEFT/CONTROL.',0)
        call agseti ('TOP/CONTROL.',0)

        call agsetf ('WINDOW',1.)
        call agsetp ('GRAPH.',gwnd(1,2),4)

cccc    TURN ON NUMERIC LABELS

        call agseti ('LABEL/CONTROL.',0)
        call agseti ('DASH/SELECTOR.',4)
        call agseti ('DASH/LENGTH.',16)
        call agsetc ('DASH/PATTERN/1.',dshv(1))
        call agsetc ('DASH/PATTERN/2.',dshv(2))

        call agsetf ('RIGHT/NUMERIC/TYPE.',1.E36)
        call agsetf ('BOTTOM/NUMERIC/TYPE.',0.)
        call agsetf('AXIS/BOTTOM/NUMERIC/WIDTH.',wdth2)
        call agsetf('AXIS/RIGHT/NUMERIC/WIDTH.',wdth2)
        xlab=""
        ylab=""
        call anotat(xlab,ylab,0,0,0,0)



        call agsave(brparms)
        rewind(brparms)

        return
        end
c___________________________________________________________________
      subroutine plotray(ihard)
       integer year,month,day,gates,hits,hour,min,sec
     &         ,rng_index,raycount,maxrayno
       common /raydat/range_inc,range,azim,elev,hour,min,time_inc,
     > sec,year,month,day,hits,gates,rng_index,raycount,maxrayno
      character date*18,azth*27,time*14,rayno*17
      integer trparms,tlparms,blparms,brparms,reset,scatprm
       integer polyopt
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
       common  /specfilt/sigmaf,xorder,nor,onoff,dc,polyopt
        common /infile1/filename
       common /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
       real vi(1025),vq(1025),ui(1025),uq(1025),vui(1025),vuq(1025),
     >     uvi(1025),uvq(1025),nor(1025),extraI,extraQ

       character*3 onoff,dc,dcstat*15,filtstat*16,filename*80

      real gwnd(4,4)
      real plfile(-5:3000,0:54)
        integer rtype(4)
        common/labels/label
        character label(0:54)*12
        common /ray/plfile,rtype
c
      parameter (npulses=30000,ngates = 1000)
        character polyon*3,notchon*3
        common /onoff/polyon,notchon
        integer nwidth,notchopt
        common /specnotchparms/nwidth,notchopt,lenblock,wdwtype
        common/polyfitparms/xarray,polycoef,polyorder,iauto
        integer polyorder
        real xarray(1025)
        real*8 polycoef(0:39)
       character polyonoff*43,wdwtype(6)*14,notchonoff*43


      call chkst1(ihard)
      length=gates

c     print *,'length=',length,plfile(length,0)

cccc  SET 1st GRAPH ON TOP WINDOW OF THE PAGE
      call agrstr(tlparms)
      rewind(tlparms)
      call agstup(plfile(1,0),1,0,length,1,plfile(1,rtype(1)),
     +            1,0,length,1)
      call agback
      call agcurv(plfile(1,0),1,plfile(1,rtype(1)),1,length,1)

cccc  SET 2nd GRAPH ON SAME AXIS OF THE TOP WINDOW

      call agrstr(trparms)
      rewind(trparms)
      call agstup(plfile(1,0),1,0,length,1,plfile(1,rtype(2)),
     +            1,0,length,1)
      call agback
      call agcurv(plfile(1,0),1,plfile(1,rtype(2)),1,length,2)

cccc  SET 1st GRAPH ON BOTTOM WINDOW OF THE PAGE
      call agrstr(blparms)
      rewind(blparms)
      call agstup(plfile(1,0),1,0,length,1,plfile(1,rtype(3)),
     +            1,0,length,1)
      call agback
      call agcurv(plfile(1,0),1,plfile(1,rtype(3)),1,length,1)

cccc  SET 2nd GRAPH ON SAME AXIS OF THE BOTTOM WINDOW
      call agrstr(brparms)
      rewind(brparms)
      call agstup(plfile(1,0),1,0,length,1,plfile(1,rtype(4)),
     +            1,0,length,1)
      call agback
      call agcurv(plfile(1,0),1,plfile(1,rtype(4)),1,length,2)
     

       write(azth,417)azim,elev
417    format('AZ:',f8.2,' ELEV: ',f8.2)
c       write(el,418) stelev,elevation
c418    format('EL:',f8.2,' to ',f8.2)
c      write(bottomlb,421)radar
421    format(a9,8x,'Range (km)')
       write(rayno,422) raycount
422    format('RAY NO.=',i8)
       write(time,423)hour,min,sec
423    format("TIME ",i2,':',i2,':',i2)
       write(date,424)year,month,day
424    format(i4,1x,'Mo ',i2,1x,'Day ',i2)
       write(dcstat,431) dc
431    format('DC removed? ',a3)
       write(filtstat,433) onoff
433    format('Filtering is ',a3)
        write(polyonoff,434) polyon,polyorder,nn,lenblock
434     format('POLY Filt ',a3,' ORDER ',i1,' TS len',i4,' BLOCK',i4)
        write(notchonoff,435) notchon,nwidth,wdwtype(iwdw)
435     format('NOTCH ',a3,' WIDTH ',i2,' WDW TYPE ',a14)

       call set(0.,1.,0.,1.,0.,1.,0.,1.,1)

        call plchmq(.04,.06,dcstat   ,.011,.0,-1.)
        call plchmq(.04,.03,filtstat ,.011,0.,-1.)
        call plchmq(.60,.05,filename ,.011,0.,-1.)
       call plchmq(.01,.99,date,.011,0.,-1.)
       call plchmq(.25,.99,time,.011,0.,-1.)
       call plchmq(.41,.99,azth,.011,0.,-1.)
       call plchmq(.99,.85,label(rtype(2)),.013,90.,0.)
       call plchmq(.03,.85,label(rtype(1)),.013,90.,0.)
       call plchmq(.01,.52,rayno,.011,0.,-1.)
       call plchmq(.21,.50,polyonoff,.011,0.,-1.)
       call plchmq(.21,.54,notchonoff,.011,0.,-1.)
c      call plchmq(.70,.52,bottomlb,.013,0.,0.)
       call plchmq(.99,.35,label(rtype(4)),.013,90.,0.)
       call plchmq(.03,.35,label(rtype(3)),.013,90.,0.)

       call chkst2(ihard)
       call sflush
       return
       end
c______________________________________________________________________
        subroutine raymenu(length)
      parameter (npulses=30000,ngates = 1000)
       common /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
        common/polyfitparms/xarray,polycoef,polyorder,iauto
        real xarray(1025)
        real*8 polycoef(0:39)
        integer polyorder 
       real vi(1025),vq(1025),ui(1025),uq(1025),vui(1025),vuq(1025),
     >    uvi(1025),uvq(1025),nor(1025),extraI,extraQ
      integer eof
      integer trparms,tlparms,blparms,brparms,reset,scatprm
       integer year,month,day,gates,hits,hour,min,sec,maxrayno
       common /raydat/range_inc,range,azim,elev,hour,min,time_inc,
     >  sec,year,month,day,hits,gates,rng_index,raycount,maxrayno
     >  ,range_ary
       real  range_ary(ngates)
       integer rng_index,raycount
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >  ,wdth2
      real gwnd(4,4)
       integer polyopt
       common /labels/label
       real plfile(-5:3000,0:54),lambda
        integer rtype(4),pbegin,pend
        common /ray/plfile,rtype
        common  /specfilt/sigmaf,xorder,nor,onoff,dc,polyopt

        character*20 rayfile,label(0:54)*12,time*4,onoff*3,dc*3
c
       do 1  i=1,100
          write(6,*) plfile(i,0), plfile(i,1)
1      continue
c

c10000    call displhead
        write(6,*)"RTPYE",rtype(1),rtype(2),rtype(3),rtype(4)
10000    continue
        write(6,*) label(rtype(1)),label(rtype(2)),label(rtype(3)),
     +              label(rtype(4))

        print *,'       <<<<<<<<< RAY PLOT MENU >>>>>>>>>'
        write(6,10) label(rtype(1)),label(rtype(2)),label(rtype(3)),
     +              label(rtype(4))
10      format(/'SELECT AN OPTION :'/
     x       3x, '0. (RETURN) EXIT TO MAIN MENU.'//
     x       3x, '1. PICK VARIABLES FOR PLOTTING.'/
     x       3x, '2. SCALE X-AXIS.'/
     x       3x, '3. SCALE ',a12/
     x       3x, '4. SCALE ',a12/
     x       3x, '5. SCALE ',a12/
     x       3x, '6. SCALE ',a12/
     x       3x, '7. RESET PLOT PARAMETERS.'//
     x       3x, '8. PLOT a RAY.'/
     x       3x, '9. HARD COPY a RAY.'/
     x       3x, '10.SAVE the PLOT in rayfile.'/
     x       3x, '11.NEXT RAY.'/
     x       3x, '12.SKIP RAYS.'/
     x       3x, '13.REWIND FILE.'/
     x       3x, '14.SCATTER PLOT MENU.'/
     x       '$OPTION? ')
      iopt = ttinp(0)
      if (iopt.eq.0) then
        return
      end if
      if (iopt.gt.14) goto 1000
      goto (100,200,300,400,500,600,700,800,900,1000,1100,
     >      1200,1300,1400),iopt

100   call pickplot(rtype)
      goto 10000

200   call scalxaxis
      goto 10000

300   call scaltopl
      goto 10000

400   call scaltopr
      goto 10000

500   call scalbotl
      goto 10000

600   call scalbotr
      go to 10000

700   call resetparms
      goto 10000

800   call plotray(0)
      go to 10000

900   call plotray(1)
      goto 10000

1000  print*,'enter the NAME of rayfile'
      read(5,*,err=1000) rayfile
      open(15,file=rayfile)
      print *,'enter RANGE ( start, end ) for DATA output '
      read(5,*,err=1000) pbegin,pend
c       
          write(15,1011)label(0),label(rtype(1)),
     +    label(rtype(2)),label(rtype(3)),label(rtype(4))
      do 1010 i = 1, length
        if (plfile(i,0).ge.pbegin.and.plfile(i,0).le.pend) then
c      
          write(15,1012)plfile(i,0),plfile(i,rtype(1)),
     +    plfile(i,rtype(2)),plfile(i,rtype(3)),plfile(i,rtype(4))
        endif
1010  continue
1011  format(5(2x,a10))
1012  format(5(2x,f10.4))
      close(15)
      goto 10000
c---------------
1100  continue
c     read(1,'(a4)',err=2000,end=1120) time
c       if(time.ne.'TIME') then
c1110       print *," BAD DATA FILE FORMAT"
c          print *,'TIME= ',time
c          stop
c       else
c          backspace(1)
        jj=0  !forces next ray read in gettime
1115       call gettime(eof)
c          call process(xarray,polycoef,polyorder)
           call process
           call calculate
           if (jj.ne.0) goto 1115
c       endif
c      call shortpulse(1)
c      call shortpulse(2)

         goto 10000
    
1120  print *,"******END OF FILE REACHED********"
      print *,'***********************************'
      print *,"REWIND TO BEGINNING"
       rewind(1)
       eof=0
       raycount=0
      return
c----------------------
1200  continue !RAY SKIP
      write(6,*)"ENTER NUMBER OF RAYS TO SKIP"
        read(5,*) nrays
        if(nrays.eq.0) goto 10000
        if(raycount+nrays.lt.1) goto 10000
c
c  GOTO SPECIFIED RAY
c
        raycount=raycount+nrays-1
        jj=0  !forces next ray read in gettime
1215       call gettime(eof)
           call process
           call calculate
           if (jj.ne.0) goto 1215
       goto 10000
1300   continue
        rewind(1)
        eof=0
        raycount=0
        return
1400   continue
       call scattermenu(length,eof)
       goto 10000
      end
c___________________________________________________________________

        subroutine pickplot(typ)

        integer typ(4)
       integer trparms,tlparms,blparms,brparms,reset,scatprm
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
      real gwnd(4,4)


        print *,'   <<< SELECT FOUR VARIABLES TO PLOT BY NUMBER >>>'
        call varmenu

         do 100 i=1,4
13          ntyp=ttinp(0)
            if (ntyp.eq.0) then
               return
            end if
20          if (ntyp.gt.54.or.ntyp.lt.0.or.ntyp.eq.19) then
               print *,'<<<<<  BAD NUMBER  >>>>>'
               goto 13
            end if
            typ(i)=ntyp
100      continue

         return
         end
c___________________________________________________________________

        subroutine scalxaxis


c2345678901234567890123456789012345678901234567890123456789012345678901
c        1         2         3         4         5         6         7

       integer trparms,tlparms,blparms,brparms,reset,scatprm
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
      real gwnd(4,4)

        print *,'<<< ENTER MIN AND MAX VALUES; REAL!!!!!>>>'
        read(5,*) xxmin,xxmax

        call agrstr(tlparms)
        rewind(tlparms)
        call agsetf ('X/MINIMUM.', xxmin)
        call agsetf ('X/MAXIMUM.', xxmax)
        call agseti('WINDOW.', 1)
        call agsave(tlparms)
        rewind(tlparms)

        call agrstr(trparms)
        rewind(trparms)
        call agsetf ('X/MINIMUM.', xxmin)
        call agsetf ('X/MAXIMUM.', xxmax)
        call agseti('WINDOW.', 1)
        call agsave(trparms)
        rewind(trparms)

        call agrstr(blparms)
        rewind(blparms)
        call agsetf ('X/MINIMUM.', xxmin)
        call agsetf ('X/MAXIMUM.', xxmax)
        call agseti('WINDOW.', 1)
        call agsave(blparms)
        rewind(blparms)

        call agrstr(brparms)
        rewind(brparms)
        call agsetf ('X/MINIMUM.', xxmin)
        call agsetf ('X/MAXIMUM.', xxmax)
        call agseti('WINDOW.', 1)
        call agsave(brparms)
        rewind(brparms)

        return
        end
c___________________________________________________________________

        subroutine scalbotr


       integer trparms,tlparms,blparms,brparms,reset,scatprm
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
      real gwnd(4,4)

        print *,'<<< ENTER MIN AND MAX VALUES; REAL!!!!!>>>'
        read(5,*) xxmin,xxmax
        call agrstr(brparms)
        rewind(brparms)
        call agsetf ('Y/MINIMUM.', xxmin)
        call agsetf ('Y/MAXIMUM.', xxmax)
        call agseti('WINDOW.', 1)
        call agsave(brparms)
        rewind(brparms)

        return
        end
c___________________________________________________________________

        subroutine scalbotl


       integer trparms,tlparms,blparms,brparms,reset,scatprm

      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
      real gwnd(4,4)
        print *,'<<< ENTER MIN AND MAX VALUES; REAL!!!!!>>>'
        read(5,*) xxmin,xxmax
        call agrstr(blparms)
        rewind(blparms)
        call agsetf ('Y/MINIMUM.', xxmin)
        call agsetf ('Y/MAXIMUM.', xxmax)
        call agseti('WINDOW.', 1)
        call agsave(blparms)
        rewind(blparms)

        return
        end
c___________________________________________________________________

        subroutine scaltopr


       integer trparms,tlparms,blparms,brparms,reset,scatprm
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
      real gwnd(4,4)

        print *,'<<< ENTER MIN AND MAX VALUES; REAL!!!!!>>>'
        read(5,*) xxmin,xxmax

        call agrstr(trparms)
        rewind(trparms)
        call agsetf ('Y/MINIMUM.', xxmin)
        call agsetf ('Y/MAXIMUM.', xxmax)
        call agseti('WINDOW.', 1)
        call agsave(trparms)
        rewind(trparms)

        return

        end
c---------------------------------------------------------------------

        subroutine scaltopl


       integer trparms,tlparms,blparms,brparms,reset,scatprm
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
      real gwnd(4,4)

        print *,'<<< ENTER MIN AND MAX VALUES; REAL!!!!!>>>'
        read(5,*) xxmin,xxmax

        call agrstr(tlparms)
        rewind(tlparms)
        call agsetf ('Y/MINIMUM.', xxmin)
        call agsetf ('Y/MAXIMUM.', xxmax)
        call agseti('WINDOW.', 1)
        call agsave(tlparms)
        rewind(tlparms)

        return
        end
c___________________________________________________________________
c______________________________________________________________________

        subroutine varmenu
       common /labels/label
       character  label(0:54)*12
c2345678901234567890123456789012345678901234567890123456789012345678901234567890
c --0--     --1--     --2--     --3--     --4--     --5--     --6--     --7--
       write(6,10) (label(i),label(i+27), i=1,27)
c      write(6,10) label(1),label(2),label(3),label(4),label(5),label(6),
c    >   label(7),label(8),label(9),label(10),label(11),label(12),
c    &             label(13),label(14),label(15),label(16),label(17),label(18),
c    &             label(19),label(20),label(21),label(22),label(23),label(24),
c    &             label(25),label(26),label(27),label(28),label(29),label(30),
c    &             label(31),label(32),label(33),label(34),label(35),label(36),
c    &             label(37),label(38),label(39),label(40),label(41),label(42),
c    &             label(43),label(44),label(45),label(46),label(47),label(49),
c    &             label(49),label(50)
10     format(3x, ' 0.  EXIT TO PREV MENU.'/
     +        3x, ' 1. ',a12,'      28.',a12,/
     +        3x, ' 2. ',a12,'      29.', a12,/
     +        3x, ' 3. ',a12,'      30.' ,a12,/
     +        3x, ' 4. ',a12,'      31.',a12,/
     +        3x, ' 5. ',a12,'      32.',a12,/
     +        3x, ' 6. ',a12,'      33.',a12,/
     +        3x, ' 7. ',a12,'      34.',a12,/
     +        3x, ' 8. ',a12,'      35.',a12,/
     +        3x, ' 9. ',a12,'      36.',a12,/
     +        3x, ' 10. ',a12,'     37.',a12,/
     +        3x, ' 11. ',a12,'     38.',a12,/
     +        3x, ' 12. ',a12,'     39.',a12,/
     +        3x, ' 13. ',a12,'     40.',a12,/ 
     +        3x, ' 14. ',a12,'     41.',a12,/
     +        3x, ' 15. ',a12,'     42.',a12,/
     +        3x, ' 16. ',a12,'     43.',a12,/
     +        3x, ' 17. ',a12,'     44.',a12,/
     +        3x, ' 18. ',a12,'     45.',a12,/
     +        3x, ' 19. ',a12,'     46.',a12,/
     +        3x, ' 20. ',a12,'     47.',a12,/
     +        3x, ' 21. ',a12,'     48.',a12,/
     +        3x, ' 22. ',a12,'     49.',a12,/
     +        3x, ' 23. ',a12,'     50.',a12,/
     +        3x, ' 24. ',a12,'     51.',a12,/
     +        3x, ' 25. ',a12,'     52.',a12,/ 
     +        3x, ' 26. ',a12,'     53.',a12,/ 
     +        3x, ' 27. ',a12,'     54.',a12,/)
              return
              end

c__________________________________________________________________________


        subroutine scattermenu(length,eof)
      parameter (npulses=30000,ngates = 1000)
       common /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
        real ui(1025),uq(1025),vi(1025),vq(1025),uvi(1025),uvq(1025),
     >   vui(1025),vuq(1025),extraI,extraQ
      common  /specfilt/sigmaf,xorder,nor,onoff,dc,polyopt
c2345678901234567890123456789012345678901234567890123456789012345678901
c        1         2         3         4         5         6         7
       integer polyopt
       real plfile(-5:3000,0:54)
       real    cit(500),cib(500),boundary(500),
     >         var(500),avg(500)
       common /statpts/var,cit,cib,boundary,avg,maxbin,wdthsc

       integer rtype(4),rng_index,raycount,firsttime,scatype(2),fstray
        character*6 scale(2), onoff*3,dc*3, hardxgks*70,label(0:54)*12
        common /ray/plfile,rtype
        common /labels/label
        common /cpathres/cpat
       common /scatparms/scatarry,scatprm,scatype,npts,tlut
       real scatarry(50000,2),lambda,tlut(50)
      integer trparms,tlparms,blparms,brparms,reset,scatprm
      common /pltparms/tlparms,trparms,blparms,brparms,reset,gwnd
     >                 ,wdth2
      real gwnd(4,4),nor(1025)
       data xmin,xmax,ymin,ymax,deltax/0.,0.,0.,0.,0./
       character time*14
        common /hardcopy/hardxgks,scale
       integer year,month,day,gates,hits,hour,min,sec,eof,maxrayno
       common /raydat/range_inc,range,azim,elev,hour,min,time_inc,
     >  sec,year,month,day,hits,gates,rng_index,raycount,maxrayno
     >  ,range_ary
        real range_ary(ngates)
       integer scattype(2)
        save rnginitial,rngfinal,numrays
1001    continue
c1001	call displhead

c23456789012345678901234567890123456789012345678901234567890123456789012
c        1         2         3         4         5         6         7

20      print *,'    <<<<<<<<< SCATTER/STATISTICS PLOT MENU >>>>>>>>>'
	write(6,10) label(scatype(1)),label(scatype(2)),rnginitial,
     +              rngfinal,numrays
10      format(/'SELECT AN OPTION:'/
     x	3x, ' 0. (RETURN) EXIT TO MAIN MENU.'/
     x  3x, ' 1. VARIABLES SELECT.'/
     x  3x, ' 2. RANGE SELECT.'/
     x  3x, ' 3. SELECT NUMBER of RAYS.'/
     x	3x, ' 4. CREATE SCATTER PLOT with :'/
     x  3x, '                   VARIABLE_1  = ',a12/
     x  3x, '                   VARIABLE_2  = ',a12/
     x  3x, '                   RANGE       = ',f7.3,' to ',f7.3/
     x  3x, '                   No. of RAYS = ',i3//
     x  3x, ' 5. SKIPRAY.'/
     x	3x, ' 7. SCALE VERTICAL AXIS.'/
     x  3x, ' 8. SCALE HORIZONTAL AXIS.'/
     x	3x, ' 9. RESET SCATTER PLOT PARAMETERS.'//
     x  3x, '10. REPLOT ( after 4 ).'/
     x  3x, '11. HARD COPY SCATTER PLOT (after 4 or 10 ).'/)
        write(6,11)cpat 
11      format(3x, '***** STATISTIC SECTION *****'/
     x  3x, '12. CALCULATE & PLOT STATISTICS (after 4 or 10).'/
     x  3x, '13. RE-PLOT STATISTICS ( after 12 ).'/
     x  3x, '14. HARD COPY STATISTICS ( after 12 or 13 ).'/
     x  3x, '15. ENTER CPA THRESHOLD ',f5.3/
     x  '$OPTION?    ')
12     iopt = ttinp(0)
       if (iopt.gt.15.or.iopt.lt.0) then
           print *,'<<<<<  BAD NUMBER, TRY AGAIN  >>>>>'
           goto 12  
       end if
       if (iopt.eq.0) then      
	 return
       end if
       goto (100,200,300,400,500,600,700,800,900,1000,1100,
     +       1200,1300,1400,1500),iopt  

100   call varselect
      goto 1001

200   write(6,*) "ENTER MIN & MAX RANGE VALUES.<<REAL!!!!>>>"
      read(5,*,err=200) rnginitial,rngfinal
      goto 1001

300   write(6,*) "ENTER THE NUMBER OF RAYS TO BE PROCESSED"
      read (5,*,err=300) numrays
      go to 1001

400   fstray = raycount
      call getdata(numrays,rnginitial,rngfinal,time,eof)
        if(eof.eq.1) then
            print *,"END OF FILE"
            print *,"RAY NO.= ",raycount
        endif
      print *,'Before scatplot, rnginitial,rngfinal,time,fstray,
     >   rslope,rint, numrays', rnginitial,rngfinal,time,fstray,
     >   rslope,rint, numrays
      call scatplot(rnginitial,rngfinal,time,fstray,rslope,rint,
     >                numrays,0)

      goto 1001

500   continue      
1203      call gettime(eof)
      call windowdata
      if (dc.eq.'yes') then
         call removedc(ui,uq,nn)
         call removedc(vi,vq,nn)
         call removedc(uvi,uvq,nn)
         call removedc(vui,vuq,nn)
      endif

      if(onoff.eq.'on') then
         print *,'filter is called'
         call filt(vi,vq,ui,uq,uvi,uvq,vui,vuq,nn,rng)
      endif
        call calculate
        if (jj.ne.0) goto 1203

      goto 1001

600   goto 1001

700   call scalvertical(ymin,ymax)
      goto 1001

800   call scalhorizontal(xmin,xmax)
      goto 1001

900   call agrstr(reset)
      rewind(reset)
      call agsave(scatprm)
      rewind(scatprm)
      goto 1001

1000  call scatplot(rnginitial,rngfinal,time,fstray,rslope,rint,
     >                numrays,0)
      goto 1001

1100  call scatplot(rnginitial,rngfinal,time,fstray,rslope,rint,
     >                   numrays,1)
      goto 1001

1200  print *,'<<<<< ENTER BIN LENGTH >>>>>'
      read (5,*) deltax
      if(deltax.le.0.) then
	print *,'********************************'
	print *,'*    BIN LENGTH must be > 0    *'
        print *,'********************************'
	goto 1200
      end if
      call statistic(xmin,xmax,ymin,ymax,deltax,rslope,rint,time)
cccc  Li Liu added here.                             Oct. 8, 1991
      if (maxbin.eq.0.or.maxbin.ge.501) then
	  print*,'<<<<<                         >>>>>'
	  print*,'<<<<< BIN LENGTH is too small >>>>>'
	  print*,'<<<<<                         >>>>>'
	  goto 1200
      else
          call statplot(xmin,xmax,ymin,ymax,rnginitial,rngfinal,
     >         time,fstray,rslope,rint,numrays,0)
      end if
      goto 1001

1300  call statplot(xmin,xmax,ymin,ymax,rnginitial,rngfinal,
     >         time,fstray,rslope,rint,numrays,0)
      goto 1001

1400  call statplot(xmin,xmax,ymin,ymax,rnginitial,rngfinal,
     >       time,fstray,rslope,rint,numrays,1)
      goto 1001
1500  write(6,*) "ENTER CPA threhold"
      read(5,*,err=1500) cpat
      goto 1001
      end
c__________________________________________________________

      subroutine statplot(xmin,xmax,ymin,ymax,rnginitial,
     >           rngfinal,time,fstray,rslope,rint,numrays,ihard)
       integer fstray
      real x1(2),y1(2),xwin(4)
       real    cit(500),cib(500),boundary(500),
     >         var(500),avg(500)
       common /statpts/var,cit,cib,boundary,avg,maxbin,wdthsc
       common /scatparms/scatarry,scatprm,scatype,npts,tlut
       real scatarry(50000,2),tlut(50)
       integer scatprm,scatype(2)
       character time*14
        data xwin/.05,.95,.05,.95/
       call chkst1(ihard)
      call agrstr(scatprm)
      rewind(scatprm)

      call pltsct(rnginitial,rngfinal,time,fstray,rslope,
     >             rint,numrays)
      call pltbtm
         call agsetp ('GRAPH.',xwin,4)
      if (xmin.le.xmax) then
	    call agsetf ('X/MINIMUM.', xmin)
	    call agsetf ('X/MAXIMUM.', xmax)
      end if
      if (ymin.le.ymax) then
	    call agsetf ('Y/MINIMUM.', ymin)
	    call agsetf ('Y/MAXIMUM.', ymax)
      end if

      call agsetf('AXIS/LEFT/NUMERIC/WIDTH.',wdthsc)
      call agsetf('AXIS/BOTTOM/NUMERIC/WIDTH.',wdthsc)
      call agstup(boundary(1),1,0,maxbin,1,avg(1),1,0,maxbin,1)

      call agseti ('LABEL/CONTROL.',0)
      call agback

cccc  POINTS PUTS PLOTTING FILE INTO BUFFER.
c     call points(boundary,avg,maxbin,ichar('+'),0)
      call points(boundary,cit,maxbin,ichar('+'),0)
      call points(boundary,cib,maxbin,ichar('+'),0)

cccc  DRAW BAR LINES for STDV on MEAN CURVE
      call agcurv(boundary(1),1,avg(1),1,maxbin,1)
      mm=2
c     print *,'maxbin=',maxbin
      do 500 i=1,maxbin
	    if(avg(i).ne.1.e36) then
               x1(1)=boundary(i)
	       x1(2)=x1(1)
cccc  Changed by Li Liu on Sep. 21, 92.
c	       y1(1)=avg(i)+var(i)/2
c	       y1(2)=avg(i)-var(i)/2
 	       y1(1)=avg(i)+var(i)
 	       y1(2)=avg(i)-var(i)
 	       call agcurv(x1,1,y1,1,mm,1)
            end if
500   continue

      call chkst2(ihard)
      call sflush
       return
       end
c____________________________________________________________________
      subroutine findminmax(a,npts,xmin,xmax)

cccc  IF MIN & MAX ABSISSA VALUES ARE NOT SPECIFIED (FOR SCATTER-STATISTIC PLOT)
cccc  THEN THE MIN & MAX WILL BE FOUND HERE.

      real a(5000,2),xmin,xmax

      xmin=a(1,1)
      xmax=xmin
      do 10 i=2,npts
	 if(a(i,1).gt.xmax) xmax=a(i,1)
	 if(a(i,1).lt.xmin) xmin=a(i,1)
10    continue

      return
      end	  
c___________________________________________________________________

      subroutine statistic(xmin,xmax,ymin,ymax,deltax,rslope,rint,time)

cccc  THIS SUBROUTINE AVERAGES THE ORDINATE VALUES IN THE SPECIFIED BINS.
cccc  " statzdp " AVERAGES ONLY Zdp IN A LINEAR FASION.
       parameter (npulses=30000,ngates = 1000)
      integer counter(550)

      common /infile1/filename
       character*80 filename
       common /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
        real ui(1025),uq(1025),vi(1025),vq(1025),uvi(1025),uvq(1025),
     >  vui(1025),vuq(1025),extraI,extraQ
      common  /specfilt/sigmaf,xorder,nor,onoff,dc,polyopt
c2345678901234567890123456789012345678901234567890123456789012345678901
c        1         2         3         4         5         6         7
       integer polyopt
       real plfile(-5:3000,0:54),avg(500),avgx(500),boundary(500)
        integer rtype(4),rng_index,raycount,firsttime,scatype(2)
        character*6 scale(2), onoff*3,dc*3, hardxgks*70,label(0:54)*12
        character date*49,time*14
        common /ray/plfile,rtype
        common /labels/label
       common /scatparms/scatarry,scatprm,scatype,npts,tlut
       real scatarry(50000,2),lambda,var(500)
       real    cit(500),cib(500),tlut(50),nor(1025)
       common /statpts/var,cit,cib,boundary,avg,maxbin,wdthsc
        common /hardcopy/hardxgks,scale
       integer year,month,day,gates,hits,hour,min,sec,maxrayno
       common /raydat/range_inc,range,azim,elev,hour,min,time_inc,
     > sec,year,month,day,hits,gates,rng_index,raycount,maxrayno
     >  ,range_ary
      real range_ary(ngates)
c     save xmin,xax,ymin,ymax,deltax
      print *,"IN STAT x,y min,max", xmin,xmax ,ymin,ymax

cccc  FIRST DEFINE BINS
c     print *,'x: min,max,delta',xmin,xmax,deltax
      if(xmin.eq.0.0.and.xmax.eq.0.0) then
	    call findminmax(scatarry,npts,xxmin,xxmax)
c           print *,'x: min,max,delta',xxmin,xxmax,deltax
      else
	    xxmin=xmin
	    xxmax=xmax
      end if

      if(ymax.le.ymin) then
c        write(6,1) ymin,ymax
c        format('ymin=',f6.2,'is lt. or eq ymax=',f7.2)
         print *, '!!!!!! YOU MUST ENTER YMIN YMAX !!!!!'
	 call scalvertical(ymin,ymax)
      end if

      l=1
      start=xxmin
      if(start+deltax.ge.xxmax) then
	   maxbin = 0
	   return
      end if
10    if(start.lt.xxmax) then
cccc  Li Liu added here.                     Oct. 8, 1991
	   if (l.gt.500) then
	      maxbin = 500
	      go to 9999
 	   else
	      boundary(l)=start+deltax
	      l=l+1
	      start=start+deltax
	      go to 10
           end if
      end if
1000  maxbin=l-1
c     print *,'MAXBIN=',maxbin
	 
cccc  ZERO THE ARRAYS
      do 12 i=1,500 
	   avg(i)=0.0
	   avgx(i)=0.0
	   var(i)=0.0
	   counter(i)=0
12    continue

cccc  COMPUTE THE MEAN OF EACH BIN
       do 20 i=1,npts
	   l=1
cccc  FIRST CHECK IF POINT IS OUT OF BOUNDS
	   if(scatarry(i,1).lt.xxmin.or.
     +        scatarry(i,1).gt.xxmax) goto 20
cccc  CHECK IF ORDINATE VALUE IS OUT OF BOUNDS
 	   if(scatarry(i,2).lt.ymin.or.
     +        scatarry(i,2).gt.ymax) goto 20
cccc  FIND WHICH BIN POINT BELONGS TO
           if(scatarry(i,2).eq.1.e36) goto 20
15         if(scatarry(i,1).le.boundary(l)) then
 	       avg(l)=avg(l)+scatarry(i,2)
	       avgx(l)=avgx(l)+scatarry(i,1)
	       counter(l)=counter(l)+1
           else
               l=l+1
	       goto 15
           end if
20     continue

cccc  DIVIDE TO GET MEAN
       do 30 i=1,maxbin
	   if(counter(i).eq.0) then
               avg(i)=1.e36
	       avgx(i)=1.e36
           else
	       avg(i)=avg(i)/counter(i)
	       avgx(i)=avgx(i)/counter(i)
	   end if 
30     continue

cccc Fit a linear line to the mean values.  10/3/94, Li Liu
       print*,'Linear Fitting'
       call LSE(rslope,rint,avgx,avg,maxbin)
       print*,'Slope,Intercept:',rslope,rint

cccc  GET VARIANCES
       do 40 i=1,npts
	   l=1
cccc  CHECK IF POINT IS OUT OF BOUNDS
	   if(scatarry(i,1).lt.xxmin.or.
     +        scatarry(i,1).gt.xxmax) goto 40
cccc  CHECK IF ORDINATE VALUE IS OUT OF BOUNDS
 	   if(scatarry(i,2).lt.ymin.or.
     +        scatarry(i,2).gt.ymax) goto 40
cccc  FIND WHICH BIN POINT BELONGS TO
35         if(scatarry(i,1).le.boundary(l)) then
 	       var(l)=var(l)+(scatarry(i,2)-avg(l))**2
           else
	       l=l+1
	       goto 35
           end if
40     continue

cccc  DIVIDE BY COUNTER AND TAKE SQUARE ROOT
cccc  changed 'if(counter(i).eq.0)' to 'if(counter(i).le.10.)' 
cccc                                                   Sep. 18, 92, Li Liu
       do 50 i=1,maxbin
	   if(counter(i).le.10.) then
               var(i) = 0.
               cit(i) = 0.
               cib(i) = 0.
           else
cccc  Calculate the CONFIDENCE INTERVAL by using T_tabel. Sep. 21, 92.  Li Liu
	       nc    = counter(i)
 	       var(i)= sqrt(var(i)/(nc-1.))
c	       print *,'in counter:',i,'  nc = ',nc
	       if (nc.ge.46) then 
		  mc = 46
	       else
		  mc = nc
	       endif
c	       print *,'                 nc = ',nc
	       ci    = var(i)/sqrt(nc*1.)*tlut(mc-1)
	       cit(i) = avg(i)+ci
	       cib(i) = avg(i)-ci
           end if
50     continue

cccc   PUT MIDPOINTS INTO BOUNDARY
cccc   CHANGED OCT 31 1989. USE AVERAGE OF X VALUES.
       do 60 i=1,maxbin
c	   boundary(i)=boundary(i)-deltax/2
 	   boundary(i)=avgx(i)
60     continue

999    write(6,*)'<< STATISTICS RESULTS are in "stats.dat" >>'
       write(4,*) filename
       write(4,901) label(scatype(1)),label(scatype(2))
901    format('X_variable:',a15,2x,'Y_variable:',a15)
       write(date,903)year,month,day,time,hour,min,sec
903    format('Date ',i4,"/",i2,"/",i2,' Time ',a14,' To ',
     >         i2,":",i2,":",i2)
       write(4,*) date
       write(4,905) 'X_axis','MEAN','STDV','CIb','CIt'
       write(4,905) '------','----','----','---','---'
905    format(5(1x,a12))
       do 909 i=1,maxbin
	  write(4,906)avgx(i),avg(i),var(i),cib(i),cit(i)
909    continue 
906    format(5(1x,f12.5),a30)
       rewind(4)

9999   return
       end
c___________________________________________________________________________
c_____________________________________________________________

        subroutine LSE(a,b,x,y,n)

cccc    This is a Linear Least Square Estimate subroutine to fit a linear
cccc    equation for (xi,yi) (i=1,...,n), so that
cccc                            yi = a * xi + b
cccc    INPUTs: x(i), y(i), n, (i=1,...,n ).
cccc    OUTPUTs: a ( slope ), b ( intercept ).
cccc                                                Li Liu   Sep. 23, 92

        real x(500),y(500),a,b
        real xsum,ysum,xxsum,xysum,det

         xsum = 0.
         ysum = 0.
        xxsum = 0.
        xysum = 0.
        total = float(n)
        do 10 i = 1,n
          if (x(i).gt.1.e35.or.y(i).gt.1.e35) then
            total = total-1.
          else
            xsum =  xsum + x(i)
            ysum =  ysum + y(i)
           xxsum = xxsum + x(i)*x(i)
           xysum = xysum + x(i)*y(i)
          endif
10      continue
        det = total * xxsum - xsum**2
          a = ( total*xysum - xsum*ysum ) / det
          b = ( ysum*xxsum - xsum*xysum ) / det

        return
        end

c___________________________________________________________________


        subroutine scalhorizontal(xmin,xmax)

       common /scatparms/scatarry,scatprm,scatype,npts,tlut
       real scatarry(50000,2),lambda,tlut(50)
       integer scatprm,scatype(2)


10	print *,'<<< ENTER MIN AND MAX HORIZONTAL VALUES >>>'
 	read(5,*,err=10) xmin,xmax
	call agrstr(scatprm)
	rewind(scatprm)
 	call agsetf ('X/MINIMUM.', xmin)
 	call agsetf ('X/MAXIMUM.', xmax)
	call agsetf('WINDOW.', 1.)
	call agsave(scatprm)
	rewind(scatprm)

        return
	end
c____________________________________________________________________

        subroutine scalvertical(ymin,ymax)
       common /scatparms/scatarry,scatprm,scatype,npts,tlut
       real scatarry(50000,2),lambda,tlut(50)
       integer scatprm,scatype(2)

10	print *,'<<< ENTER MIN AND MAX VERTICAL VALUES >>>'
 	read(5,*,err=10) ymin,ymax
	call agrstr(scatprm)
	rewind(scatprm)
 	call agsetf ('Y/MINIMUM.', ymin)
 	call agsetf ('Y/MAXIMUM.', ymax)
	call agsetf('WINDOW.', 1.)
	call agsave(scatprm)
	rewind(scatprm)

        return
	end
c_____________________________________________________________________

        subroutine varselect
       common /scatparms/scatarry,scatprm,scatype,npts,tlut
       real scatarry(50000,2),tlut(50)

        integer scatype(2),scatprm

20      print *,'<<< SELECT TWO VARIABLES FOR SCAT. PLOT BY NUMBER >>>'
	call varmenu

13	 do 100 i=1,2
            ntyp=ttinp(0)
            if (ntyp.eq.0) then
	       return
            end if
   	    if (ntyp.gt.53.or.ntyp.lt.0) then
	       print *,'<<<<<  BAD NUMBER  >>>>>>'
	       goto 13  
            end if
	    scatype(i)=ntyp
	    if(ntyp.eq.19) scatype(i)=0
100      continue

         return
	 end
c_______________________________________________________________________

        subroutine getdata(numrays,rnginitial,rngfinal,time,eof)

cccc    GETS DATA FOR SCATTER PLOT
      parameter (npulses=30000,ngates = 1000)
       common/tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
c2345678901234567890123456789012345678901234567890123456789012345678901
c        1         2         3         4         5         6         7
        real ui(1025),uq(1025),vi(1025),vq(1025),uvi(1025),uvq(1025),
     >    vui(1025) ,vuq(1025),extraI,extraQ
        common /radardata/rng,zh, zdr,ldrh,ldrv, ldrx, zv, dop, pvvhh,
     & r0hhvv , r0hhvh, r0vvhv, r2hhhh, r2vvvv, r1hvvh, r1vhhv, r0hvvh,
     &    r3hhvv, r3vvhh, phhvh, pvvhv, pdp3, phvvh, pvhhv,pdp_2lg,
     &    wdth_hh,wdth_vh,pdpx,r1hhvv,ro11, rx_r1hv,phvvh_dp,phvvh_vel
     &    ,phidp,phhvv,cpa,cpaV,cpaUV,cpaVU,cpaUF
        real ldrv,ldrx
       common  /specfilt/sigmaf,xorder,nor,onoff,dc,polyopt
       integer polyopt
       integer eof
       real plfile(-5:3000,0:54),scatarry(50000,2),lambda,tlut(50)
        real nor(1025)
        integer rtype(4),rng_index,raycount,firsttime,scatype(2)
        character*6 scale(2), onoff*3,dc*3, hardxgks*70,label(0:54)*12
        common /ray/plfile,rtype
        common /labels/label
        common /cpathres/cpat
        common /scatparms/scatarry,scatprm,scatype,npts,tlut
        common /hardcopy/hardxgks,scale
       integer year,month,day,gates,hits,hour,min,sec,maxrayno
       common /raydat/range_inc,range,azim,elev,hour,min,time_inc,
     >   sec,year,month,day,hits,gates,rng_index,raycount,maxrayno
     >  ,range_ary
       real range_ary(ngates)
        character time*14
        open(50,file='ClutterBins.txt')

c       print *,'*********IN GETDATA CPAUF = ',cpaUF
c       read(5,*)
        length=gates
         ieof=0
	 npts=0
c	 print *,'length=',length,'numrays',numrays
c	 print *,(scatype(i), i=1,2)
cccc     FIRST GET INITIAL TIME
	 write(time,1) hour,min,sec
1        format('Time: ',i2,':',i2,':',i2)
	 do 10 j=1,numrays
          print *,"getting ray ",j 
c	 print *,'numrays',numrays,' j=',j,' idata=',idata
          if(j.gt.1) then  !present plfile for begin ray is full so just transfer data
c             GET EACH GATE OF DATA AND PROCESS        
1203          call gettime(eof)
              call process
              call calculate
              if (jj.ne.0) goto 1203
c            call shortpulse(1)
c            call shortpulse(2)

          endif
            do 30 m=1,length
               if(plfile(m,0).ge.rnginitial.and.plfile(m,0).le.
     >            rngfinal.and.plfile(m,51).gt.cpat) then
                  print *,'cpaUF= ',plfile(m,51)
	          npts=npts+1
	          scatarry(npts,1)=plfile(m,scatype(1))
	          scatarry(npts,2)=plfile(m,scatype(2))
                  if(plfile(m,49).gt.5..and.(plfile(m,49)-plfile(m,50))
     >               .lt.48.) then
                     write(50,*) 
     >               raycount,plfile(m,0),plfile(m,49),plfile(m,50)
                  endif
c     print *,'j,m',j,m,'sa',scatarry(npts,1),scatarry(npts,2),
c    + 'npts',npts,rnginitial,rngfinal
	        end if
30          continue
c        read(5,*)
10      continue
        close(50)

	return
	end
c_______________________________________________________________

      subroutine scatplot(rnginitial,rngfinal,time,fstray,
     >       rslope,rint,numrays,ihard)
       real scatarry(50000,2),lambda,tlut(50)
       real    cit(500),cib(500),boundary(500),
     >         var(500),avg(500),xwin(4)
       common /statpts/var,cit,cib,boundary,avg,maxbin,wdthsc

        integer rtype(4),rng_index,raycount,scatype(2),fstray
        character*6 label(0:54)*12,time*14
        common /labels/label
        common /scatparms/scatarry,scatprm,scatype,npts,tlut
        integer scatprm

        data xwin/.05,.95,.05,.95/

        call chkst1(ihard)
        do 10 i=1,npts
           print *,'scatarray1,2',i,scatarry(i,1),scatarry(i,2)
10      continue
        call pltsct(rnginitial,rngfinal,time,fstray,rslope,rint,
     >              numrays)

        call pltbtm

cccc	SUPPRESS THE FRAME ADVANCE

	call agrstr(scatprm)
	rewind(scatprm)
         call agsetp ('GRAPH.',xwin,4)

	call agsetf('AXIS/LEFT/NUMERIC/WIDTH.',wdthsc)
	call agsetf('AXIS/BOTTOM/NUMERIC/WIDTH.',wdthsc)
	call agstup(scatarry(1,1),1,0,npts,1,scatarry(1,2),1,0,
     +               npts,1)
	
	call agseti ('LABEL/CONTROL.',0)
	call agback

	call points(scatarry(1,1),scatarry(1,2),npts,-2,0)

	call chkst2(ihard)
        call sflush
       return
       end
c____________________________________________________________________
c************************************************************************
c concatinate strings b and c into a, stopping at first blank
c string a must be dimensioned character*80             Dave Brunkow
        subroutine dbcat(a,b,c)
        implicit none
        character*(*) a,b,c
        integer i,j,k
        integer lena,lenb,lenc

c       print *,'dbcat: a,b,c=',a,'>,<',b,'>,<',c,'>'
c       print *,'dbcat: len(a,b,c)=',len(a),',',len(b),',',len(c)

        lena= len(a)
        lenb= len(b)
        lenc= len(c)

        do i=1,lenb
                if(b(i:i).eq.' ') then
                    k=i
                    do j=1,lenc
                        if(c(j:j).eq.' ') goto 1
                        if(k.gt.lena) goto 2
                        a(k:k)= c(j:j)
                        k=k+1
                    end do
                    goto 1
                endif
                if(i.gt.lena) goto 2
                a(i:i)= b(i:i)
        end do
c               pad out with blanks
1       do i=k,lena
                a(i:i)=' '
        end do
        return

2       print *,'dbcat: length of  first arg needs to be increased'
        call exit(1)
        end

c********************************************************
c____________________________________________________________________________

        subroutine pltsct(rnginitial,rngfinal,time,fstray,rslope,rint,
     >              numrays)
      parameter (npulses=30000,ngates = 1000)
        character*6 label(0:54)*12
        common /labels/label
       common /scatparms/scatarry,scatprm,scatype,npts,tlut
       real scatarry(50000,2),tlut(50)
       integer scatype(2),scatprm
       integer year,month,day,gates,hits,hour,min,sec,maxrayno
       common /raydat/range_inc,range,azim,elev,hour,min,time_inc,
     >  sec,year,month,day,hits,gates,rng_index,raycount,maxrayno
     >  ,range_ary
       real range_ary(ngates)
       common /tseries/ui,uq,vi,vq,uvi,uvq,vui,vuq,nn,iwdw,jj,lambda,Ts
     >        ,nkeep,extraI,extraQ
       real vi(1025),vq(1025),ui(1025),uq(1025),vui(1025),vuq(1025),
     >   uvi(1025),uvq(1025),extraI,extraQ
       real lambda

        integer rng_index,raycount,fstray
        character*30 lsefit,time*14,date*15,rng*24,rayn*36,slgth*14

       call set(0.,1.,0.,1.,0.,1.,0.,1.,1)

       write(lsefit,421) rslope,rint
421    format('Slope:',f8.3,', Int:',f8.3)
       write(rayn,423) fstray,numrays
423    format('Start RAY#:',i8,', total:',i4,' rays')
       write(rng,424) rnginitial,rngfinal
424    format('from ',f6.3,' to ',f6.3,' km')
       write(date,425) year,month,day
425    format('Date ',i4,1x,i2,1x,i2)

        call plchmq(.15,.99,date ,.011,.0,-1.)
        call plchmq(.45,.99,time ,.011,0.,-1.)
        call plchmq(.70,.99,lsefit,.011,0.,-1.)
        call plchmq(.15,.97,rayn ,.011,0.,-1.)
        call plchmq(.70,.97,rng,.011,0.,-1.)
        call plchmq(.04,.50,label(scatype(2)),.013,90.,0.)
        call plchmq(.50,.10,label(scatype(1)),.013,0.,0.)
       write(slgth,428) nn
428    format("Seq. Len. ",i4)
       call plchmq(.73,.08,slgth,.011,0.,-1.)


        return
        end
c____________________________________________________________________________
       subroutine pltbtm
       common  /specfilt/sigmaf,xorder,nor,onoff,dc,polyopt
        common /infile1/filename
       integer polyopt
       character*3 onoff,dc,dcstat*15,filtstat*16,filename*80
       real nor(1025)
       write(dcstat,421) dc
421    format('DC removed? ',a3)
       write(filtstat,423) onoff 
423    format('Filtering is ',a3)
        call plchmq(.04,.06,dcstat   ,.011,.0,-1.)
        call plchmq(.04,.03,filtstat ,.011,0.,-1.)
        call plchmq(.60,.05,filename ,.011,0.,-1.)
        return
        end

c-------------------------------------------------------------
c   THIS PROGRAM FINDS THE EIGENVALUES OF A 3X3 SYMMETRIC MATRIX.
c   AN EIGENVECTOR IS ALSO CALCULATED BY THE SLATEC LIBRARY ROUTINE
C   " SSIEV ". THE VECTOR IS NORMALIZED TO A MAGNITUDE OF ONE AND
C   THE REAL AND IMAGINARY PARTS OF THE POLARIZATION RATIO ARE
C   CACULATED VIA: V = [2RE{CHI}  2IM{CHI}  1-CHI*CONJG(CHI)]/(1+CHI*CONJG(CHI))
c   i.e. V*V=1 WHERE CHI = POLARIZATIO RATIO.
c
c               by john hubbert, november 1993
       subroutine eigen(x,ldr1,ldr2,tiltsav,elipsav)
c
c x --  complex 3X3 array contain covariance matrix
c
c ldr -- the ldr of the rotated polarization basis
c
       complex x(3,3)
       integer flag
       real ldr1,ldr2
c      open (1,file='emat', status='unknown')
c      open (2,file='evals',status='unknown')
       raddeg=180/3.1415926
       n=3
       lda=3
       ldevec=3

c
c    DO XPOL OPTIMUN POLARIZATIONS 
c
      flag=1
          call optxpol(x,flag,ldr1,ldr2,tiltsav,elipsav)  !radar xpol
c 

      return           
      end             
c-----------------------------------------------------------
      subroutine optxpol(x,flag,ldr1,ldr2,tiltsav,elipsav)

       real eval(3),yy(3,3),work(6)
       integer flag
       complex x(3,3)
       real norm(3),im,ldr1,ldr2
       raddeg=180/3.1415926
       n=3
       lda=3
       ldevec=3

c
c     CONVERT THE MEMBERS OF THE COVARIANCE MATRIX TO THE PROPER
c     FORM
c
         call convert(x,yy)
c
c  print the A0 matrix
c
c      write(6,*)'THE A0 MATRIX IS:'
c     do 1 i=1,3
c      write(6,2) yy(i,1),yy(i,2),yy(i,3)
1     continue
2     format(3f9.4) 

c
c     GET EIGENVALUES, EIGENVECTORS
c
       call ssiev(yy,lda,n,eval,work,1,info)
c  yy contain the eigenvectors upn return
c      call evcsf(n,yy,lda,eval,evec,ldevec)
c
      norm(1)= (yy(1,1)**2 + yy(2,1)**2 + yy(3,1)**2)**.5
      norm(2)= (yy(1,2)**2 + yy(2,2)**2 + yy(3,2)**2)**.5
      norm(3)= (yy(1,3)**2 + yy(2,3)**2 + yy(3,3)**2)**.5
      do 10 i=1,3
        do 11 j=1,3
         yy(j,i)=yy(j,i)/norm(i)
11    continue
10    continue
c
c
c OUTPUT
c
c      write(2,*)'EIGENVALUES'
c      write(2,*) (eval(i), i=1,3)
c      write(2,*) ' '
c      write(2,*)'vector1     vector2      vector3 (columns)'
c     do 3 i=1,3
c      write(2,*) (yy(i,j), j=1,3)
3     continue
c
c  FIND SMALLEST EIGENVALUE
c
      if(eval(1).lt.eval(2)) then
          value=eval(1)
      else
          value=eval(2)
      endif
      if(value.gt.eval(3)) value=eval(3)
c
c find the eigen polarizations
c  SOLVE FOR REAL AND IMAG. PARTS
c
c      write(2,*)'X-POL roots'
c  vector 1

      im=yy(3,1)/(1.+yy(1,1))
      re=yy(2,1)/(1.+yy(1,1))
c     a= 1. + yy(3,1)**2/yy(2,1)**2
c     b= 2*yy(1,1)/yy(2,1)
c     c= -1.
c     rt1= (-b + sqrt(b**2-4*a*c))/(2*a)
c     rt2= (-b - sqrt(b**2-4*a*c))/(2*a)
c     re=rt1
c the sign on the solution (either rt1 or rt2) must match the sign
c on the yy(2,1) which is the real part of the vector.
c     y=re*yy(2,1)
c     if(y.lt.0.) re=rt2
c     write(2,*)'   rt1       rt2        selected rt'
c     print *,rt1,rt2,re
c     if(rt1*rt2.gt.0.) then
c       print *,'both roots have same sign'
c     endif
c     im=re*yy(3,1)/yy(2,1)
      tilt=.5*atan2(2*re,1. - re**2-im**2)
      elip=.5*asin(2*im/(1. + re**2 + im**2))
c      write(2,*) 'real,imag ',re,im
c      write(2,*) 'tilt= ',tilt*raddeg,' elip= ',elip*raddeg
c
c     save tilt and ellip. angles for transformation 
c
      if(value.eq.eval(1)) then
        tiltsav=tilt
        elipsav=elip
      endif

c
c  vector 2
c

      im=yy(3,2)/(1.+yy(1,2))
      re=yy(2,2)/(1.+yy(1,2))
c
c     a= 1. + yy(3,2)**2/yy(2,2)**2
c     b= 2*yy(1,2)/yy(2,2)
c     c= -1.
c     rt1= (-b + sqrt(b**2-4*a*c))/(2*a)
c     rt2= (-b - sqrt(b**2-4*a*c))/(2*a)
c     re=rt1
c     y=re*yy(2,2)
c     if(y.lt.0.) re=rt2
c     write(2,*)'   rt1       rt2        selected rt'
c     print *,rt1,rt2,re
c     if(rt1*rt2.gt.0.) then
c       print *,'both roots have same sign'
c     endif
c     im=re*yy(3,2)/yy(2,2)
      tilt=.5*atan2(2*re, 1.-re**2-im**2)
      elip=.5*asin(2*im/(1. + re**2 + im**2))
c      write(2,*) 'real,imag ',re,im
c      write(2,*) 'tilt= ',tilt*raddeg,' elip= ',elip*raddeg
      if(value.eq.eval(2)) then
        tiltsav=tilt
        elipsav=elip
      endif


c  vector 3
c

      im=yy(3,3)/(1.+yy(1,3))
      re=yy(2,3)/(1.+yy(1,3))
c
c     a= 1. + yy(3,3)**2/yy(2,3)**2
c     b= 2*yy(1,3)/yy(2,3)
c     c= -1.
c     rt1= (-b + sqrt(b**2-4*a*c))/(2*a)
c     rt2= (-b - sqrt(b**2-4*a*c))/(2*a)
c     re=rt1
c     y=re*yy(2,3)
c     if(y.lt.0.) re=rt2
c     write(2,*)'   rt1       rt2        selected rt'
c     print *,rt1,rt2,re
c     if(rt1*rt2.gt.0.) then
c       print *,'both roots have same sign'
c     endif
c     im=re*yy(3,3)/yy(2,3)
      tilt=.5*atan2(2*re, 1.-re**2-im**2)
      elip=.5*asin(2*im/(1. + re**2 + im**2))
c      write(2,*) 'real,imag ',re,im
c      write(2,*) 'tilt= ',tilt*raddeg,' elip= ',elip*raddeg
c      write(2,*) 'end X-POL roots'
c      write(2,*)
      if(value.eq.eval(3)) then
        tiltsav=tilt
        elipsav=elip
      endif

c
c  TRANSFORM TO CHARACTERISTIC BASIS
c
      call transformation(x,tiltsav,elipsav,ldr1)
      call transformation(x,tiltsav,-elipsav,ldr2)
      return
      end
c--------------------------------------------------------------------

      subroutine convert(x,y) ! get real matrix "A" for radar xpol case
      complex x(3,3)
      complex shh2,shv2,svv2,shhshv,shhsvv,shvsvv
      real y(3,3)
      shh2=x(1,1)
      shv2=x(2,2)/2
      svv2=x(3,3)
c     2nd term conjugated
      
      shhshv=x(1,2)/sqrt(2.)
      shhsvv=x(1,3)
      shvsvv=x(2,3)/sqrt(2.)

      y(1,1)=2*real(shv2)
      y(1,2)=real(shvsvv - shhshv)
      y(1,3)=aimag(shvsvv - shhshv)
      y(2,2)=.5*(real(shh2+svv2 - 2*shhsvv))
      y(2,3)=-aimag(shhsvv)
      y(3,3)=.5*(real(shh2+svv2 + 2*shhsvv))
c    
c     make matrix symmetric
c
      y(2,1)=y(1,2)
      y(3,1)=y(1,3)
      y(3,2)=y(2,3)
      return
      end
c--------------------------------------------------------------
      subroutine transformation(x,tiltsav,elipsav,ldr)

c   This program reads in a covariance matrix and will then transform
c   it to any other polarization basis.
      complex x(3,3),j,cz,z,ph,cph
c     complex sx,sy,sxsy,sxy,sxsxy,sxysx,sxysy,sysxy,sysx
      complex sh,shv,sv,shshv,shsv,shvsv,shvsh,svshv,svsh
      real norm,ldr
      complex s11,s12,s13,s21,s22,s23,s31,s32,s33

       raddeg=180/3.1415926
       degrad=3.1415926/180
c

      j=cmplx(0.,1.)

      sh=x(1,1)
      shv=x(2,2)/2
      sv= x(3,3)

c     2nd term conjugated

      shshv=(x(1,2)/sqrt(2.))
      shsv=(x(1,3))
      shvsv=(x(2,3)/sqrt(2.))

      shvsh=conjg(shshv)
      svsh= conjg(shsv)
      svshv=conjg(shvsv)

c    Calculate deg. pol

      dpol= ((real(sh) -real(shv))**2 + 4*cabs(shshv)**2)**.5
      dpol= dpol/(real(sh) +real(shv))
c      write(2,*) 'UNrotated deg. pol :', dpol
c      write(2,*)'LDR=',10*log10(cabs(shv/sh))



c
c      write(2,*)'tilt=',tiltsav*raddeg
c      write(2,*)'elip=',elipsav*raddeg
      phi=tiltsav
      elp=elipsav

c z is the polarization ratio, cz is the conjugate

       z= (tan(phi) + j*tan(elp))/(1.-j*tan(phi)*tan(elp))
       cz=conjg(z)
       ph= cexp(-j*atan(tan(phi)*tan(elp)))
       cph=conjg(ph)

       norm= (1. + cabs(z)**2)**2
c      write(2,*)'pol. ratio=',cabs(z),'e**j',
c     >      atan2(aimag(z),real(z))*raddeg
c      write(2,*) 'VECTOR PHASE CONSTANT=', cabs(ph),'e**j',
c     >          atan2(aimag(ph),real(ph))*raddeg
c      write(2,*)'(1. + cabs(z)**2)**2=', norm
c
c co-powers
c 
         s11= ((sh+4*shv*cabs(z)**2 + cabs(z)**4*sv) +
     >        2*real( 2*svshv*z**2*cz + z**2*svsh
     >       + 2*z*shvsh))/norm

         s33= ((sv+ 4*shv*cabs(z)**2 + cabs(z)**4*sh)+
     >       2*real( -2*shvsh*z**2*cz + z**2*svsh
     >       - 2*z*svshv))/norm

c cross power

c
         s22= (( (1.0-cabs(z)**2)**2 *shv
     >              + cabs(z)**2*(sv+sh))
     >      +2*real(cz*(1.-cabs(z)**2)*shvsv
     >      -       z*(1.-cabs(z)**2)*shvsh
     >      -       z**2*svsh))/norm

c
c  copol covariance
c
         s31 = cph**4*(cz**4*shsv + svsh
c        s31 = (cz**4*shsv + svsh
     >           + cz**2*(sh+sv-4*shv)
     >        + 2*cz*(-cz**2*shvsv - shvsh +
     >          cz**2*shshv + svshv))/norm

c
c the cross-covariances
c

       s21 =cph**2*(2*conjg(z)*(1.-cabs(z)**2)*shv -
c      s21 =                (2*conjg(z)*(1.-cabs(z)**2)*shv -
     >       cz*sh + cz**2*z*sv +
     >       z*svsh - cz**3*shsv
     >       +(1.-cabs(z)**2)*(cz**2*shvsv + shvsh)
     >       +2*cz*(z*svshv - cz*shshv))/norm


       s32 =cph**2*(-2*conjg(z)*(1.-cabs(z)**2)*shv +
c      s32 =                (-2*conjg(z)*(1.-cabs(z)**2)*shv +
     >       cz*sv - cz**2*z*sh -
     >       z*svsh + cz**3*shsv
     >       +(1.-cabs(z)**2)*(cz**2*shshv + svshv)
     >       -2*cz*(-z*shvsh + cz*shvsv))/norm

        s32=s32*sqrt(2.)/s11
        s21=s21*sqrt(2.)/s11
c************************************************************
c
c LARRY: only need s22 and s11 to calculate LDR
        s22=2*s22/s11
c*************************************************************
        s31=s31/s11
        s33=s33/s11
        s11=cmplx(1.)

        s23=conjg(s32)
        s13=conjg(s31)
        s12=conjg(s21)

c  print the radar cov. matrix without phase correction term,
c  first real and imag. parts then mag. and phase.
c
c      write(2,*) 'CHARACTERISTIC BASIS C-MATRIX'
c      write(2,*)
c      write(2,*) 'REAL AND IMAG. OF CM'
c      write(2,*) 
c      write(2,100) s11,s12,s13
c      write(2,100) s21,s22,s23
c      write(2,100) s31,s32,s33
100    format(3(1x,2(f11.7)))
c
c      write(2,*) "mag. and phase"
c
c23456789012345678901234567890123456789012345678901234567890123456789012
c        1         2         3         4         5         6         7


c      write(2,101) cabs(s11),atan2(aimag(s11),real(s11))*raddeg,
c    >              cabs(s12),atan2(aimag(s12),real(s12))*raddeg,
c    >              cabs(s13),atan2(aimag(s13),real(s13))*raddeg
c      write(2,101) cabs(s21),atan2(aimag(s21),real(s21))*raddeg,
c    >              cabs(s22),atan2(aimag(s22),real(s22))*raddeg,
c    >              cabs(s23),atan2(aimag(s23),real(s23))*raddeg
c      write(2,101) cabs(s31),atan2(aimag(s31),real(s31))*raddeg,
c    >              cabs(s32),atan2(aimag(s32),real(s32))*raddeg,
c    >              cabs(s33),atan2(aimag(s33),real(s33))*raddeg
101     format(3(1x,f11.7,1x,f9.4))

c    Calculate deg. pol

      s12=s12/sqrt(2.)
      s22=s22/2
      dpol= ((real(s11) -real(s22))**2 + 4*cabs(s12)**2)**.5
      dpol= dpol/(real(s11) +real(s22))
c      write(2,*) 'rotated deg. pol :', dpol
c      write(2,*)'s11/s22=',10*log10(cabs(s11/s22))
      cox= cabs(s12)/(cabs(s11*s22))**.5
c********************************************************************
c 
c LARRY: This next line of code is the LDR that we want, ie, the ldr of the 
c        rotated polarization basis.
c
c******************************************************************

c      write(2,*)"rot. LDR= ", 10*log10(cabs(s22))
      ldr=10*log10(cabs(s22))
c      write(2,*)'cros cor=',cox

      return
       end
c**********************************************************
c the rest of this code is canned subroutines from the SLATEC library
c******************************************************************

c     SUBROUTINE SSIEV (A, LDA, N, E, WORK, JOB, INFO)
C***BEGIN PROLOGUE  SSIEV
C***PURPOSE  Compute the eigenvalues and, optionally, the eigenvectors
C            of a real symmetric matrix.
C***LIBRARY   SLATEC
C***CATEGORY  D4A1
C***TYPE      SINGLE PRECISION (SSIEV-S, CHIEV-C)
C***KEYWORDS  COMPLEX HERMITIAN, EIGENVALUES, EIGENVECTORS, MATRIX,
C             SYMMETRIC
C***AUTHOR  Kahaner, D. K., (NBS)
C           Moler, C. B., (U. of New Mexico)
C           Stewart, G. W., (U. of Maryland)
C***DESCRIPTION
C
C     Abstract
C      SSIEV computes the eigenvalues and, optionally, the eigenvectors
C      of a real symmetric matrix.
C
C     Call Sequence Parameters-
C       (The values of parameters marked with * (star) will be  changed
C         by SSIEV.)
C
C       A*      REAL (LDA,N)
C               real symmetric input matrix.
C               Only the diagonal and upper triangle of A must be input,
C               as SSIEV copies the upper triangle to the lower.
C               That is, the user must define A(I,J), I=1,..N, and J=I,.
C               ..,N.
C               On return from SSIEV, if the user has set JOB
C               = 0        the lower triangle of A has been altered.
C               = nonzero  the N eigenvectors of A are stored in its
C               first N columns.  See also INFO below.
C
C       LDA     INTEGER
C               set by the user to
C               the leading dimension of the array A.
C
C       N       INTEGER
C               set by the user to
C               the order of the matrix A and
C               the number of elements in E.
C
C       E*      REAL (N)
C               on return from SSIEV, E contains the N
C               eigenvalues of A.  See also INFO below.
C
C       WORK*   REAL (2*N)
C               temporary storage vector.  Contents changed by SSIEV.
C
C       JOB     INTEGER
C               set by user on input
C               = 0         only calculate eigenvalues of A.
C               = nonzero   calculate eigenvalues and eigenvectors of A.
C
C       INFO*   INTEGER
C               on return from SSIEV, the value of INFO is
C               = 0 for normal return.
C               = K if the eigenvalue iteration fails to converge.
C                   eigenvalues and vectors 1 through K-1 are correct.
C
C
C     Error Messages-
C          No. 1   recoverable  N is greater than LDA
C          No. 2   recoverable  N is less than one
C
C***REFERENCES  (NONE)
C***ROUTINES CALLED  IMTQL2, TQLRAT, TRED1, TRED2, XERMSG
C***REVISION HISTORY  (YYMMDD)
C   800808  DATE WRITTEN
C   890831  Modified array declarations.  (WRB)
C   890831  REVISION DATE from Version 3.2
C   891214  Prologue converted to Version 4.0 format.  (BAB)
C   900315  CALLs to XERROR changed to CALLs to XERMSG.  (THJ)
C   900326  Removed duplicate information from DESCRIPTION section.
C           (WRB)
C***END PROLOGUE  SSIEV
c****************************************************************************
c*****************************************************************
      SUBROUTINE SSIEV (A, LDA, N, E, WORK, JOB, INFO)
      INTEGER INFO,JOB,LDA,N
      REAL A(LDA,*),E(*),WORK(*)
C***FIRST EXECUTABLE STATEMENT  SSIEV
       IF (N .GT. LDA) CALL XERMSG ('SLATEC', 'SSIEV', 'N .GT. LDA.',
     +       1, 1)
       IF(N .GT. LDA) RETURN
       IF (N .LT. 1) CALL XERMSG ('SLATEC', 'SSIEV', 'N .LT. 1', 2, 1)
       IF(N .LT. 1) RETURN
C
C       CHECK N=1 CASE
C
      E(1) = A(1,1)
      INFO = 0
      IF(N .EQ. 1) RETURN
C
C     COPY UPPER TRIANGLE TO LOWER
C
      DO 10 J=1,N
      DO 10 I=1,J
         A(J,I)=A(I,J)
   10 CONTINUE
C
      IF(JOB.NE.0) GO TO 20
C
C     EIGENVALUES ONLY
C
      CALL TRED1(LDA,N,A,E,WORK(1),WORK(N+1))
      CALL TQLRAT(N,E,WORK(N+1),INFO)
      RETURN
C
C     EIGENVALUES AND EIGENVECTORS
C
   20 CALL TRED2(LDA,N,A,E,WORK,A)
      CALL IMTQL2(LDA,N,E,WORK,A,INFO)
      RETURN
      END
c****************************************************************************
      SUBROUTINE IMTQL2 (NM, N, D, E, Z, IERR)
C
      INTEGER I,J,K,L,M,N,II,NM,MML,IERR
      REAL D(*),E(*),Z(NM,*)
      REAL B,C,F,G,P,R,S,S1,S2
      REAL PYTHAG
C
C***FIRST EXECUTABLE STATEMENT  IMTQL2
      IERR = 0
      IF (N .EQ. 1) GO TO 1001
C
      DO 100 I = 2, N
  100 E(I-1) = E(I)
C
      E(N) = 0.0E0
C
      DO 240 L = 1, N
         J = 0
C     .......... LOOK FOR SMALL SUB-DIAGONAL ELEMENT ..........
  105    DO 110 M = L, N
            IF (M .EQ. N) GO TO 120
            S1 = ABS(D(M)) + ABS(D(M+1))
            S2 = S1 + ABS(E(M))
            IF (S2 .EQ. S1) GO TO 120
  110    CONTINUE
C
  120    P = D(L)
         IF (M .EQ. L) GO TO 240
         IF (J .EQ. 30) GO TO 1000
         J = J + 1
C     .......... FORM SHIFT ..........
         G = (D(L+1) - P) / (2.0E0 * E(L))
         R = PYTHAG(G,1.0E0)
         G = D(M) - P + E(L) / (G + SIGN(R,G))
         S = 1.0E0
         C = 1.0E0
         P = 0.0E0
         MML = M - L
C     .......... FOR I=M-1 STEP -1 UNTIL L DO -- ..........
         DO 200 II = 1, MML
            I = M - II
            F = S * E(I)
            B = C * E(I)
            IF (ABS(F) .LT. ABS(G)) GO TO 150
            C = G / F
            R = SQRT(C*C+1.0E0)
            E(I+1) = F * R
            S = 1.0E0 / R
            C = C * S
            GO TO 160
  150       S = F / G
            R = SQRT(S*S+1.0E0)
            E(I+1) = G * R
            C = 1.0E0 / R
            S = S * C
  160       G = D(I+1) - P
            R = (D(I) - G) * S + 2.0E0 * C * B
            P = S * R
            D(I+1) = G + P
            G = C * R - B
C     .......... FORM VECTOR ..........
            DO 180 K = 1, N
               F = Z(K,I+1)
               Z(K,I+1) = S * Z(K,I) + C * F
               Z(K,I) = C * Z(K,I) - S * F
  180       CONTINUE
C
  200    CONTINUE
C
         D(L) = D(L) - P
         E(L) = G
         E(M) = 0.0E0
         GO TO 105
  240 CONTINUE
C     .......... ORDER EIGENVALUES AND EIGENVECTORS ..........
      DO 300 II = 2, N
         I = II - 1
         K = I
         P = D(I)
C
         DO 260 J = II, N
            IF (D(J) .GE. P) GO TO 260
            K = J
            P = D(J)
  260    CONTINUE
C
         IF (K .EQ. I) GO TO 300
         D(K) = D(I)
         D(I) = P
C
         DO 280 J = 1, N
            P = Z(J,I)
            Z(J,I) = Z(J,K)
            Z(J,K) = P
  280    CONTINUE
C
  300 CONTINUE
C
      GO TO 1001
C     .......... SET ERROR -- NO CONVERGENCE TO AN
C                EIGENVALUE AFTER 30 ITERATIONS ..........
 1000 IERR = L
 1001 RETURN
      END
c***********************************************************************
      SUBROUTINE TQLRAT (N, D, E2, IERR)
C
      INTEGER I,J,L,M,N,II,L1,MML,IERR
      REAL D(*),E2(*)
      REAL B,C,F,G,H,P,R,S,MACHEP
      REAL PYTHAG
      LOGICAL FIRST
C
      SAVE FIRST, MACHEP
      DATA FIRST /.TRUE./
C***FIRST EXECUTABLE STATEMENT  TQLRAT
      IF (FIRST) THEN
         MACHEP = R1MACH(4)
      ENDIF
      FIRST = .FALSE.
C
      IERR = 0
      IF (N .EQ. 1) GO TO 1001
C
      DO 100 I = 2, N
  100 E2(I-1) = E2(I)
C
      F = 0.0E0
      B = 0.0E0
      E2(N) = 0.0E0
C
      DO 290 L = 1, N
         J = 0
         H = MACHEP * (ABS(D(L)) + SQRT(E2(L)))
         IF (B .GT. H) GO TO 105
         B = H
         C = B * B
C     .......... LOOK FOR SMALL SQUARED SUB-DIAGONAL ELEMENT ..........
  105    DO 110 M = L, N
            IF (E2(M) .LE. C) GO TO 120
C     .......... E2(N) IS ALWAYS ZERO, SO THERE IS NO EXIT
C                THROUGH THE BOTTOM OF THE LOOP ..........
  110    CONTINUE
C
  120    IF (M .EQ. L) GO TO 210
  130    IF (J .EQ. 30) GO TO 1000
         J = J + 1
C     .......... FORM SHIFT ..........
         L1 = L + 1
         S = SQRT(E2(L))
         G = D(L)
         P = (D(L1) - G) / (2.0E0 * S)
         R = PYTHAG(P,1.0E0)
         D(L) = S / (P + SIGN(R,P))
         H = G - D(L)
C
         DO 140 I = L1, N
  140    D(I) = D(I) - H
C
         F = F + H
C     .......... RATIONAL QL TRANSFORMATION ..........
         G = D(M)
         IF (G .EQ. 0.0E0) G = B
         H = G
         S = 0.0E0
         MML = M - L
C     .......... FOR I=M-1 STEP -1 UNTIL L DO -- ..........
         DO 200 II = 1, MML
            I = M - II
            P = G * H
            R = P + E2(I)
            E2(I+1) = S * R
            S = E2(I) / R
            D(I+1) = H + S * (H + D(I))
            G = D(I) - E2(I) / G
            IF (G .EQ. 0.0E0) G = B
            H = G * P / R
  200    CONTINUE
C
         E2(L) = S * G
         D(L) = H
C     .......... GUARD AGAINST UNDERFLOW IN CONVERGENCE TEST ..........
         IF (H .EQ. 0.0E0) GO TO 210
         IF (ABS(E2(L)) .LE. ABS(C/H)) GO TO 210
         E2(L) = H * E2(L)
         IF (E2(L) .NE. 0.0E0) GO TO 130
  210    P = D(L) + F
C     .......... ORDER EIGENVALUES ..........
         IF (L .EQ. 1) GO TO 250
C     .......... FOR I=L STEP -1 UNTIL 2 DO -- ..........
         DO 230 II = 2, L
            I = L + 2 - II
            IF (P .GE. D(I-1)) GO TO 270
            D(I) = D(I-1)
  230    CONTINUE
C
  250    I = 1
  270    D(I) = P
  290 CONTINUE
C
      GO TO 1001
C     .......... SET ERROR -- NO CONVERGENCE TO AN
C                EIGENVALUE AFTER 30 ITERATIONS ..........
 1000 IERR = L
 1001 RETURN
      end
c***********************************************************************
      SUBROUTINE TRED1 (NM, N, A, D, E, E2)
C
      INTEGER I,J,K,L,N,II,NM,JP1
      REAL A(NM,*),D(*),E(*),E2(*)
      REAL F,G,H,SCALE
C
C***FIRST EXECUTABLE STATEMENT  TRED1
      DO 100 I = 1, N
  100 D(I) = A(I,I)
C     .......... FOR I=N STEP -1 UNTIL 1 DO -- ..........
      DO 300 II = 1, N
         I = N + 1 - II
         L = I - 1
         H = 0.0E0
         SCALE = 0.0E0
         IF (L .LT. 1) GO TO 130
C     .......... SCALE ROW (ALGOL TOL THEN NOT NEEDED) ..........
         DO 120 K = 1, L
  120    SCALE = SCALE + ABS(A(I,K))
C
         IF (SCALE .NE. 0.0E0) GO TO 140
  130    E(I) = 0.0E0
         E2(I) = 0.0E0
         GO TO 290
C
  140    DO 150 K = 1, L
            A(I,K) = A(I,K) / SCALE
            H = H + A(I,K) * A(I,K)
  150    CONTINUE
C
         E2(I) = SCALE * SCALE * H
         F = A(I,L)
         G = -SIGN(SQRT(H),F)
         E(I) = SCALE * G
         H = H - F * G
         A(I,L) = F - G
         IF (L .EQ. 1) GO TO 270
         F = 0.0E0
C
         DO 240 J = 1, L
            G = 0.0E0
C     .......... FORM ELEMENT OF A*U ..........
            DO 180 K = 1, J
  180       G = G + A(J,K) * A(I,K)
C
            JP1 = J + 1
            IF (L .LT. JP1) GO TO 220
C
            DO 200 K = JP1, L
  200       G = G + A(K,J) * A(I,K)
C     .......... FORM ELEMENT OF P ..........
  220       E(J) = G / H
            F = F + E(J) * A(I,J)
  240    CONTINUE
C
         H = F / (H + H)
C     .......... FORM REDUCED A ..........
         DO 260 J = 1, L
            F = A(I,J)
            G = E(J) - H * F
            E(J) = G
C
            DO 260 K = 1, J
               A(J,K) = A(J,K) - F * E(K) - G * A(I,K)
  260    CONTINUE
C
  270    DO 280 K = 1, L
  280    A(I,K) = SCALE * A(I,K)
C
  290    H = D(I)
         D(I) = A(I,I)
         A(I,I) = H
  300 CONTINUE
C
      RETURN
      END
c*******************************************************************
      SUBROUTINE TRED2 (NM, N, A, D, E, Z)
C
      INTEGER I,J,K,L,N,II,NM,JP1
      REAL A(NM,*),D(*),E(*),Z(NM,*)
      REAL F,G,H,HH,SCALE
C
C***FIRST EXECUTABLE STATEMENT  TRED2
      DO 100 I = 1, N
C
         DO 100 J = 1, I
            Z(I,J) = A(I,J)
  100 CONTINUE
C
      IF (N .EQ. 1) GO TO 320
C     .......... FOR I=N STEP -1 UNTIL 2 DO -- ..........
      DO 300 II = 2, N
         I = N + 2 - II
         L = I - 1
         H = 0.0E0
         SCALE = 0.0E0
         IF (L .LT. 2) GO TO 130
C     .......... SCALE ROW (ALGOL TOL THEN NOT NEEDED) ..........
         DO 120 K = 1, L
  120    SCALE = SCALE + ABS(Z(I,K))
C
         IF (SCALE .NE. 0.0E0) GO TO 140
  130    E(I) = Z(I,L)
         GO TO 290
C
  140    DO 150 K = 1, L
            Z(I,K) = Z(I,K) / SCALE
            H = H + Z(I,K) * Z(I,K)
  150    CONTINUE
C
         F = Z(I,L)
         G = -SIGN(SQRT(H),F)
         E(I) = SCALE * G
         H = H - F * G
         Z(I,L) = F - G
         F = 0.0E0
C
         DO 240 J = 1, L
            Z(J,I) = Z(I,J) / H
            G = 0.0E0
C     .......... FORM ELEMENT OF A*U ..........
            DO 180 K = 1, J
  180       G = G + Z(J,K) * Z(I,K)
C
            JP1 = J + 1
            IF (L .LT. JP1) GO TO 220
C
            DO 200 K = JP1, L
  200       G = G + Z(K,J) * Z(I,K)
C     .......... FORM ELEMENT OF P ..........
  220       E(J) = G / H
            F = F + E(J) * Z(I,J)
  240    CONTINUE
C
         HH = F / (H + H)
C     .......... FORM REDUCED A ..........
         DO 260 J = 1, L
            F = Z(I,J)
            G = E(J) - HH * F
            E(J) = G
C
            DO 260 K = 1, J
               Z(J,K) = Z(J,K) - F * E(K) - G * Z(I,K)
  260    CONTINUE
C
  290    D(I) = H
  300 CONTINUE
C
  320 D(1) = 0.0E0
      E(1) = 0.0E0
C     .......... ACCUMULATION OF TRANSFORMATION MATRICES ..........
      DO 500 I = 1, N
         L = I - 1
         IF (D(I) .EQ. 0.0E0) GO TO 380
C
         DO 360 J = 1, L
            G = 0.0E0
C
            DO 340 K = 1, L
  340       G = G + Z(I,K) * Z(K,J)
C
            DO 360 K = 1, L
               Z(K,J) = Z(K,J) - G * Z(K,I)
  360    CONTINUE
C
  380    D(I) = Z(I,I)
         Z(I,I) = 1.0E0
         IF (L .LT. 1) GO TO 500
C
         DO 400 J = 1, L
            Z(I,J) = 0.0E0
            Z(J,I) = 0.0E0
  400    CONTINUE
C
  500 CONTINUE
C
      RETURN
      END

C *** NOTE *** This file contains the source code of the 37 most
C frequently cross-referenced subroutines in the SLATEC library.  We
C recommend that you compile it as a unit and link it with any other
C SLATEC routines that you use -- chances are they need at least several
C of these routines in any case.
C
C *** FURTHER NOTE*** You must edit the first five routines (R1MACH,
C D1MACH, I1MACH, FDUMP, and XERHLT) to make them correspond to your
C machine type.  For further information on these routines, consult
C their header files in the alphabetical header-file directory.
C
      REAL FUNCTION R1MACH (I)
C
      INTEGER SMALL(2)
      INTEGER LARGE(2)
      INTEGER RIGHT(2)
      INTEGER DIVER(2)
      INTEGER LOG10(2)
C
      REAL RMACH(5)
      SAVE RMACH
C
      EQUIVALENCE (RMACH(1),SMALL(1))
      EQUIVALENCE (RMACH(2),LARGE(1))
      EQUIVALENCE (RMACH(3),RIGHT(1))
      EQUIVALENCE (RMACH(4),DIVER(1))
      EQUIVALENCE (RMACH(5),LOG10(1))
C
C     MACHINE CONSTANTS FOR THE AMIGA
C     ABSOFT FORTRAN COMPILER USING THE 68020/68881 COMPILER OPTION
C
C     DATA SMALL(1) / Z'00800000' /
C     DATA LARGE(1) / Z'7F7FFFFF' /
C     DATA RIGHT(1) / Z'33800000' /
C     DATA DIVER(1) / Z'34000000' /
C     DATA LOG10(1) / Z'3E9A209B' /
C
C     MACHINE CONSTANTS FOR THE AMIGA
C     ABSOFT FORTRAN COMPILER USING SOFTWARE FLOATING POINT
C
C     DATA SMALL(1) / Z'00800000' /
C     DATA LARGE(1) / Z'7EFFFFFF' /
C     DATA RIGHT(1) / Z'33800000' /
C     DATA DIVER(1) / Z'34000000' /
C     DATA LOG10(1) / Z'3E9A209B' /
C
C     MACHINE CONSTANTS FOR THE APOLLO
C
C     DATA SMALL(1) / 16#00800000 /
C     DATA LARGE(1) / 16#7FFFFFFF /
C     DATA RIGHT(1) / 16#33800000 /
C     DATA DIVER(1) / 16#34000000 /
C     DATA LOG10(1) / 16#3E9A209B /
C
C     MACHINE CONSTANTS FOR THE BURROUGHS 1700 SYSTEM
C
C     DATA RMACH(1) / Z400800000 /
C     DATA RMACH(2) / Z5FFFFFFFF /
C     DATA RMACH(3) / Z4E9800000 /
C     DATA RMACH(4) / Z4EA800000 /
C     DATA RMACH(5) / Z500E730E8 /
C
C     MACHINE CONSTANTS FOR THE BURROUGHS 5700/6700/7700 SYSTEMS
C
C     DATA RMACH(1) / O1771000000000000 /
C     DATA RMACH(2) / O0777777777777777 /
C     DATA RMACH(3) / O1311000000000000 /
C     DATA RMACH(4) / O1301000000000000 /
C     DATA RMACH(5) / O1157163034761675 /
C
C     MACHINE CONSTANTS FOR THE CDC 170/180 SERIES USING NOS/VE
C
C     DATA RMACH(1) / Z"3001800000000000" /
C     DATA RMACH(2) / Z"4FFEFFFFFFFFFFFE" /
C     DATA RMACH(3) / Z"3FD2800000000000" /
C     DATA RMACH(4) / Z"3FD3800000000000" /
C     DATA RMACH(5) / Z"3FFF9A209A84FBCF" /
C
C     MACHINE CONSTANTS FOR THE CDC 6000/7000 SERIES
C
C     DATA RMACH(1) / 00564000000000000000B /
C     DATA RMACH(2) / 37767777777777777776B /
C     DATA RMACH(3) / 16414000000000000000B /
C     DATA RMACH(4) / 16424000000000000000B /
C     DATA RMACH(5) / 17164642023241175720B /
C
C     MACHINE CONSTANTS FOR THE CELERITY C1260
C
C     DATA SMALL(1) / Z'00800000' /
C     DATA LARGE(1) / Z'7F7FFFFF' /
C     DATA RIGHT(1) / Z'33800000' /
C     DATA DIVER(1) / Z'34000000' /
C     DATA LOG10(1) / Z'3E9A209B' /
C
C     MACHINE CONSTANTS FOR THE CONVEX
C     USING THE -fn COMPILER OPTION
C
C     DATA RMACH(1) / Z'00800000' /
C     DATA RMACH(2) / Z'7FFFFFFF' /
C     DATA RMACH(3) / Z'34800000' /
C     DATA RMACH(4) / Z'35000000' /
C     DATA RMACH(5) / Z'3F9A209B' /
C
C     MACHINE CONSTANTS FOR THE CONVEX
C     USING THE -fi COMPILER OPTION
C
C     DATA RMACH(1) / Z'00800000' /
C     DATA RMACH(2) / Z'7F7FFFFF' /
C     DATA RMACH(3) / Z'33800000' /
C     DATA RMACH(4) / Z'34000000' /
C     DATA RMACH(5) / Z'3E9A209B' /
C
C     MACHINE CONSTANTS FOR THE CONVEX
C     USING THE -p8 OR -pd8 COMPILER OPTION
C
C     DATA RMACH(1) / Z'0010000000000000' /
C     DATA RMACH(2) / Z'7FFFFFFFFFFFFFFF' /
C     DATA RMACH(3) / Z'3CC0000000000000' /
C     DATA RMACH(4) / Z'3CD0000000000000' /
C     DATA RMACH(5) / Z'3FF34413509F79FF' /
C
C     MACHINE CONSTANTS FOR THE CRAY
C
C     DATA RMACH(1) / 200034000000000000000B /
C     DATA RMACH(2) / 577767777777777777776B /
C     DATA RMACH(3) / 377224000000000000000B /
C     DATA RMACH(4) / 377234000000000000000B /
C     DATA RMACH(5) / 377774642023241175720B /
C
C     MACHINE CONSTANTS FOR THE DATA GENERAL ECLIPSE S/200
C
C     NOTE - IT MAY BE APPROPRIATE TO INCLUDE THE FOLLOWING CARD -
C     STATIC RMACH(5)
C
C     DATA SMALL /    20K,       0 /
C     DATA LARGE / 77777K, 177777K /
C     DATA RIGHT / 35420K,       0 /
C     DATA DIVER / 36020K,       0 /
C     DATA LOG10 / 40423K,  42023K /
C
C     MACHINE CONSTANTS FOR THE DEC RISC
C
c     DATA RMACH(1) / Z'00800000' /
c     DATA RMACH(2) / Z'7F7FFFFF' /
c     DATA RMACH(3) / Z'33800000' /
c     DATA RMACH(4) / Z'34000000' /
c     DATA RMACH(5) / Z'3E9A209B' /
C
C     MACHINE CONSTANTS FOR THE ELXSI 6400
C     (ASSUMING REAL*4 IS THE DEFAULT REAL)
C
C     DATA SMALL(1) / '00800000'X /
C     DATA LARGE(1) / '7F7FFFFF'X /
C     DATA RIGHT(1) / '33800000'X /
C     DATA DIVER(1) / '34000000'X /
C     DATA LOG10(1) / '3E9A209B'X /
C
C     MACHINE CONSTANTS FOR THE HARRIS 220
C
C     DATA SMALL(1), SMALL(2) / '20000000, '00000201 /
C     DATA LARGE(1), LARGE(2) / '37777777, '00000177 /
C     DATA RIGHT(1), RIGHT(2) / '20000000, '00000352 /
C     DATA DIVER(1), DIVER(2) / '20000000, '00000353 /
C     DATA LOG10(1), LOG10(2) / '23210115, '00000377 /
C
C     MACHINE CONSTANTS FOR THE HONEYWELL 600/6000 SERIES
C
C     DATA RMACH(1) / O402400000000 /
C     DATA RMACH(2) / O376777777777 /
C     DATA RMACH(3) / O714400000000 /
C     DATA RMACH(4) / O716400000000 /
C     DATA RMACH(5) / O776464202324 /
C
C     MACHINE CONSTANTS FOR THE HP 730
C
C     DATA RMACH(1) / Z'00800000' /
C     DATA RMACH(2) / Z'7F7FFFFF' /
C     DATA RMACH(3) / Z'33800000' /
C     DATA RMACH(4) / Z'34000000' /
C     DATA RMACH(5) / Z'3E9A209B' /
C
C     MACHINE CONSTANTS FOR THE HP 2100
C     3 WORD DOUBLE PRECISION WITH FTN4
C
C     DATA SMALL(1), SMALL(2) / 40000B,       1 /
C     DATA LARGE(1), LARGE(2) / 77777B, 177776B /
C     DATA RIGHT(1), RIGHT(2) / 40000B,    325B /
C     DATA DIVER(1), DIVER(2) / 40000B,    327B /
C     DATA LOG10(1), LOG10(2) / 46420B,  46777B /
C
C     MACHINE CONSTANTS FOR THE HP 2100
C     4 WORD DOUBLE PRECISION WITH FTN4
C
C     DATA SMALL(1), SMALL(2) / 40000B,       1 /
C     DATA LARGE(1), LARGE(2) / 77777B, 177776B /
C     DATA RIGHT(1), RIGHT(2) / 40000B,    325B /
C     DATA DIVER(1), DIVER(2) / 40000B,    327B /
C     DATA LOG10(1), LOG10(2) / 46420B,  46777B /
C
C     MACHINE CONSTANTS FOR THE HP 9000
C
C     DATA SMALL(1) / 00004000000B /
C     DATA LARGE(1) / 17677777777B /
C     DATA RIGHT(1) / 06340000000B /
C     DATA DIVER(1) / 06400000000B /
C     DATA LOG10(1) / 07646420233B /
C
C     MACHINE CONSTANTS FOR THE IBM 360/370 SERIES,
C     THE XEROX SIGMA 5/7/9, THE SEL SYSTEMS 85/86  AND
C     THE PERKIN ELMER (INTERDATA) 7/32.
C
C     DATA RMACH(1) / Z00100000 /
C     DATA RMACH(2) / Z7FFFFFFF /
C     DATA RMACH(3) / Z3B100000 /
C     DATA RMACH(4) / Z3C100000 /
C     DATA RMACH(5) / Z41134413 /
C
C     MACHINE CONSTANTS FOR THE IBM PC
C
      DATA SMALL(1) / 1.18E-38      /
      DATA LARGE(1) / 3.40E+38      /
      DATA RIGHT(1) / 0.595E-07     /
      DATA DIVER(1) / 1.19E-07      /
      DATA LOG10(1) / 0.30102999566 /
C
C     MACHINE CONSTANTS FOR THE IBM RS 6000
C
C     DATA RMACH(1) / Z'00800000' /
C     DATA RMACH(2) / Z'7F7FFFFF' /
C     DATA RMACH(3) / Z'33800000' /
C     DATA RMACH(4) / Z'34000000' /
C     DATA RMACH(5) / Z'3E9A209B' /
C
C     MACHINE CONSTANTS FOR THE INTEL i860
C
C     DATA RMACH(1) / Z'00800000' /
C     DATA RMACH(2) / Z'7F7FFFFF' /
C     DATA RMACH(3) / Z'33800000' /
C     DATA RMACH(4) / Z'34000000' /
C     DATA RMACH(5) / Z'3E9A209B' /
C
C     MACHINE CONSTANTS FOR THE PDP-10 (KA OR KI PROCESSOR)
C
C     DATA RMACH(1) / "000400000000 /
C     DATA RMACH(2) / "377777777777 /
C     DATA RMACH(3) / "146400000000 /
C     DATA RMACH(4) / "147400000000 /
C     DATA RMACH(5) / "177464202324 /
C
C     MACHINE CONSTANTS FOR PDP-11 FORTRAN SUPPORTING
C     32-BIT INTEGERS (EXPRESSED IN INTEGER AND OCTAL).
C
C     DATA SMALL(1) /    8388608 /
C     DATA LARGE(1) / 2147483647 /
C     DATA RIGHT(1) /  880803840 /
C     DATA DIVER(1) /  889192448 /
C     DATA LOG10(1) / 1067065499 /
C
C     DATA RMACH(1) / O00040000000 /
C     DATA RMACH(2) / O17777777777 /
C     DATA RMACH(3) / O06440000000 /
C     DATA RMACH(4) / O06500000000 /
C     DATA RMACH(5) / O07746420233 /
C
C     MACHINE CONSTANTS FOR PDP-11 FORTRAN SUPPORTING
C     16-BIT INTEGERS  (EXPRESSED IN INTEGER AND OCTAL).
C
C     DATA SMALL(1), SMALL(2) /   128,     0 /
C     DATA LARGE(1), LARGE(2) / 32767,    -1 /
C     DATA RIGHT(1), RIGHT(2) / 13440,     0 /
C     DATA DIVER(1), DIVER(2) / 13568,     0 /
C     DATA LOG10(1), LOG10(2) / 16282,  8347 /
C
C     DATA SMALL(1), SMALL(2) / O000200, O000000 /
C     DATA LARGE(1), LARGE(2) / O077777, O177777 /
C     DATA RIGHT(1), RIGHT(2) / O032200, O000000 /
C     DATA DIVER(1), DIVER(2) / O032400, O000000 /
C     DATA LOG10(1), LOG10(2) / O037632, O020233 /
C
C     MACHINE CONSTANTS FOR THE SUN
C
c     DATA RMACH(1) / Z'00800000' /
c     DATA RMACH(2) / Z'7F7FFFFF' /
c     DATA RMACH(3) / Z'33800000' /
c     DATA RMACH(4) / Z'34000000' /
c     DATA RMACH(5) / Z'3E9A209B' /
C
C     MACHINE CONSTANTS FOR THE SUN
C     USING THE -r8 COMPILER OPTION
C
C     DATA RMACH(1) / Z'0010000000000000' /
C     DATA RMACH(2) / Z'7FEFFFFFFFFFFFFF' /
C     DATA RMACH(3) / Z'3CA0000000000000' /
C     DATA RMACH(4) / Z'3CB0000000000000' /
C     DATA RMACH(5) / Z'3FD34413509F79FF' /
C
C     MACHINE CONSTANTS FOR THE UNIVAC 1100 SERIES
C
C     DATA RMACH(1) / O000400000000 /
C     DATA RMACH(2) / O377777777777 /
C     DATA RMACH(3) / O146400000000 /
C     DATA RMACH(4) / O147400000000 /
C     DATA RMACH(5) / O177464202324 /
C
C     MACHINE CONSTANTS FOR THE VAX
C     (EXPRESSED IN INTEGER AND HEXADECIMAL)
C     THE HEX FORMAT BELOW MAY NOT BE SUITABLE FOR UNIX SYSTEMS
C     THE INTEGER FORMAT SHOULD BE OK FOR UNIX SYSTEMS
C
C     DATA SMALL(1) /       128 /
C     DATA LARGE(1) /    -32769 /
C     DATA RIGHT(1) /     13440 /
C     DATA DIVER(1) /     13568 /
C     DATA LOG10(1) / 547045274 /
C
C     DATA SMALL(1) / Z00000080 /
C     DATA LARGE(1) / ZFFFF7FFF /
C     DATA RIGHT(1) / Z00003480 /
C     DATA DIVER(1) / Z00003500 /
C     DATA LOG10(1) / Z209B3F9A /
C
C     MACHINE CONSTANTS FOR THE Z80 MICROPROCESSOR
C
C     DATA SMALL(1), SMALL(2) /     0,    256/
C     DATA LARGE(1), LARGE(2) /    -1,   -129/
C     DATA RIGHT(1), RIGHT(2) /     0,  26880/
C     DATA DIVER(1), DIVER(2) /     0,  27136/
C     DATA LOG10(1), LOG10(2) /  8347,  32538/
C
C
C***FIRST EXECUTABLE STATEMENT  R1MACH
      IF (I .LT. 1 .OR. I .GT. 5) CALL XERMSG ('SLATEC', 'R1MACH',
     +   'I OUT OF BOUNDS', 1, 2)
C
      R1MACH = RMACH(I)
      RETURN
C
      END
      DOUBLE PRECISION FUNCTION D1MACH (I)
C
      INTEGER SMALL(4)
      INTEGER LARGE(4)
      INTEGER RIGHT(4)
      INTEGER DIVER(4)
      INTEGER LOG10(4)
C
      DOUBLE PRECISION DMACH(5)
      SAVE DMACH
C
      EQUIVALENCE (DMACH(1),SMALL(1))
      EQUIVALENCE (DMACH(2),LARGE(1))
      EQUIVALENCE (DMACH(3),RIGHT(1))
      EQUIVALENCE (DMACH(4),DIVER(1))
      EQUIVALENCE (DMACH(5),LOG10(1))
C
C     MACHINE CONSTANTS FOR THE AMIGA
C     ABSOFT FORTRAN COMPILER USING THE 68020/68881 COMPILER OPTION
C
C     DATA SMALL(1), SMALL(2) / Z'00100000', Z'00000000' /
C     DATA LARGE(1), LARGE(2) / Z'7FEFFFFF', Z'FFFFFFFF' /
C     DATA RIGHT(1), RIGHT(2) / Z'3CA00000', Z'00000000' /
C     DATA DIVER(1), DIVER(2) / Z'3CB00000', Z'00000000' /
C     DATA LOG10(1), LOG10(2) / Z'3FD34413', Z'509F79FF' /
C
C     MACHINE CONSTANTS FOR THE AMIGA
C     ABSOFT FORTRAN COMPILER USING SOFTWARE FLOATING POINT
C
C     DATA SMALL(1), SMALL(2) / Z'00100000', Z'00000000' /
C     DATA LARGE(1), LARGE(2) / Z'7FDFFFFF', Z'FFFFFFFF' /
C     DATA RIGHT(1), RIGHT(2) / Z'3CA00000', Z'00000000' /
C     DATA DIVER(1), DIVER(2) / Z'3CB00000', Z'00000000' /
C     DATA LOG10(1), LOG10(2) / Z'3FD34413', Z'509F79FF' /
C
C     MACHINE CONSTANTS FOR THE APOLLO
C
C     DATA SMALL(1), SMALL(2) / 16#00100000, 16#00000000 /
C     DATA LARGE(1), LARGE(2) / 16#7FFFFFFF, 16#FFFFFFFF /
C     DATA RIGHT(1), RIGHT(2) / 16#3CA00000, 16#00000000 /
C     DATA DIVER(1), DIVER(2) / 16#3CB00000, 16#00000000 /
C     DATA LOG10(1), LOG10(2) / 16#3FD34413, 16#509F79FF /
C
C     MACHINE CONSTANTS FOR THE BURROUGHS 1700 SYSTEM
C
C     DATA SMALL(1) / ZC00800000 /
C     DATA SMALL(2) / Z000000000 /
C     DATA LARGE(1) / ZDFFFFFFFF /
C     DATA LARGE(2) / ZFFFFFFFFF /
C     DATA RIGHT(1) / ZCC5800000 /
C     DATA RIGHT(2) / Z000000000 /
C     DATA DIVER(1) / ZCC6800000 /
C     DATA DIVER(2) / Z000000000 /
C     DATA LOG10(1) / ZD00E730E7 /
c     DATA LOG10(2) / ZC77800DC0 /
C
C     MACHINE CONSTANTS FOR THE BURROUGHS 5700 SYSTEM
C
C     DATA SMALL(1) / O1771000000000000 /
C     DATA SMALL(2) / O0000000000000000 /
C     DATA LARGE(1) / O0777777777777777 /
C     DATA LARGE(2) / O0007777777777777 /
C     DATA RIGHT(1) / O1461000000000000 /
C     DATA RIGHT(2) / O0000000000000000 /
C     DATA DIVER(1) / O1451000000000000 /
C     DATA DIVER(2) / O0000000000000000 /
C     DATA LOG10(1) / O1157163034761674 /
C     DATA LOG10(2) / O0006677466732724 /
C
C     MACHINE CONSTANTS FOR THE BURROUGHS 6700/7700 SYSTEMS
C
C     DATA SMALL(1) / O1771000000000000 /
C     DATA SMALL(2) / O7770000000000000 /
C     DATA LARGE(1) / O0777777777777777 /
C     DATA LARGE(2) / O7777777777777777 /
C     DATA RIGHT(1) / O1461000000000000 /
C     DATA RIGHT(2) / O0000000000000000 /
C     DATA DIVER(1) / O1451000000000000 /
C     DATA DIVER(2) / O0000000000000000 /
C     DATA LOG10(1) / O1157163034761674 /
C     DATA LOG10(2) / O0006677466732724 /
C
C     MACHINE CONSTANTS FOR THE CDC 170/180 SERIES USING NOS/VE
C
C     DATA SMALL(1) / Z"3001800000000000" /
C     DATA SMALL(2) / Z"3001000000000000" /
C     DATA LARGE(1) / Z"4FFEFFFFFFFFFFFE" /
C     DATA LARGE(2) / Z"4FFE000000000000" /
C     DATA RIGHT(1) / Z"3FD2800000000000" /
C     DATA RIGHT(2) / Z"3FD2000000000000" /
C     DATA DIVER(1) / Z"3FD3800000000000" /
C     DATA DIVER(2) / Z"3FD3000000000000" /
C     DATA LOG10(1) / Z"3FFF9A209A84FBCF" /
C     DATA LOG10(2) / Z"3FFFF7988F8959AC" /
C
C     MACHINE CONSTANTS FOR THE CDC 6000/7000 SERIES
C
C     DATA SMALL(1) / 00564000000000000000B /
C     DATA SMALL(2) / 00000000000000000000B /
C     DATA LARGE(1) / 37757777777777777777B /
C     DATA LARGE(2) / 37157777777777777777B /
C     DATA RIGHT(1) / 15624000000000000000B /
C     DATA RIGHT(2) / 00000000000000000000B /
C     DATA DIVER(1) / 15634000000000000000B /
C     DATA DIVER(2) / 00000000000000000000B /
C     DATA LOG10(1) / 17164642023241175717B /
C     DATA LOG10(2) / 16367571421742254654B /
C
C     MACHINE CONSTANTS FOR THE CELERITY C1260
C
C     DATA SMALL(1), SMALL(2) / Z'00100000', Z'00000000' /
C     DATA LARGE(1), LARGE(2) / Z'7FEFFFFF', Z'FFFFFFFF' /
C     DATA RIGHT(1), RIGHT(2) / Z'3CA00000', Z'00000000' /
C     DATA DIVER(1), DIVER(2) / Z'3CB00000', Z'00000000' /
C     DATA LOG10(1), LOG10(2) / Z'3FD34413', Z'509F79FF' /
C
C     MACHINE CONSTANTS FOR THE CONVEX
C     USING THE -fn OR -pd8 COMPILER OPTION
C
C     DATA DMACH(1) / Z'0010000000000000' /
C     DATA DMACH(2) / Z'7FFFFFFFFFFFFFFF' /
C     DATA DMACH(3) / Z'3CC0000000000000' /
C     DATA DMACH(4) / Z'3CD0000000000000' /
C     DATA DMACH(5) / Z'3FF34413509F79FF' /
C
C     MACHINE CONSTANTS FOR THE CONVEX
C     USING THE -fi COMPILER OPTION
C
C     DATA DMACH(1) / Z'0010000000000000' /
C     DATA DMACH(2) / Z'7FEFFFFFFFFFFFFF' /
C     DATA DMACH(3) / Z'3CA0000000000000' /
C     DATA DMACH(4) / Z'3CB0000000000000' /
C     DATA DMACH(5) / Z'3FD34413509F79FF' /
C
C     MACHINE CONSTANTS FOR THE CONVEX
C     USING THE -p8 COMPILER OPTION
C
C     DATA DMACH(1) / Z'00010000000000000000000000000000' /
C     DATA DMACH(2) / Z'7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF' /
C     DATA DMACH(3) / Z'3F900000000000000000000000000000' /
C     DATA DMACH(4) / Z'3F910000000000000000000000000000' /
C     DATA DMACH(5) / Z'3FFF34413509F79FEF311F12B35816F9' /
C
C     MACHINE CONSTANTS FOR THE CRAY
C
C     DATA SMALL(1) / 201354000000000000000B /
C     DATA SMALL(2) / 000000000000000000000B /
C     DATA LARGE(1) / 577767777777777777777B /
C     DATA LARGE(2) / 000007777777777777774B /
C     DATA RIGHT(1) / 376434000000000000000B /
C     DATA RIGHT(2) / 000000000000000000000B /
C     DATA DIVER(1) / 376444000000000000000B /
C     DATA DIVER(2) / 000000000000000000000B /
C     DATA LOG10(1) / 377774642023241175717B /
C     DATA LOG10(2) / 000007571421742254654B /
C
C     MACHINE CONSTANTS FOR THE DATA GENERAL ECLIPSE S/200
C
C     NOTE - IT MAY BE APPROPRIATE TO INCLUDE THE FOLLOWING CARD -
C     STATIC DMACH(5)
C
C     DATA SMALL /    20K, 3*0 /
C     DATA LARGE / 77777K, 3*177777K /
C     DATA RIGHT / 31420K, 3*0 /
C     DATA DIVER / 32020K, 3*0 /
C     DATA LOG10 / 40423K, 42023K, 50237K, 74776K /
C
C     MACHINE CONSTANTS FOR THE DEC RISC
C
c     DATA SMALL(1), SMALL(2) / Z'00000000', Z'00100000'/
c     DATA LARGE(1), LARGE(2) / Z'FFFFFFFF', Z'7FEFFFFF'/
c     DATA RIGHT(1), RIGHT(2) / Z'00000000', Z'3CA00000'/
c     DATA DIVER(1), DIVER(2) / Z'00000000', Z'3CB00000'/
c     DATA LOG10(1), LOG10(2) / Z'509F79FF', Z'3FD34413'/
C
C     MACHINE CONSTANTS FOR THE ELXSI 6400
C     (ASSUMING REAL*8 IS THE DEFAULT DOUBLE PRECISION)
C
C     DATA SMALL(1), SMALL(2) / '00100000'X,'00000000'X /
C     DATA LARGE(1), LARGE(2) / '7FEFFFFF'X,'FFFFFFFF'X /
C     DATA RIGHT(1), RIGHT(2) / '3CB00000'X,'00000000'X /
C     DATA DIVER(1), DIVER(2) / '3CC00000'X,'00000000'X /
C     DATA LOG10(1), LOG10(2) / '3FD34413'X,'509F79FF'X /
C
C     MACHINE CONSTANTS FOR THE HARRIS 220
C
C     DATA SMALL(1), SMALL(2) / '20000000, '00000201 /
C     DATA LARGE(1), LARGE(2) / '37777777, '37777577 /
C     DATA RIGHT(1), RIGHT(2) / '20000000, '00000333 /
C     DATA DIVER(1), DIVER(2) / '20000000, '00000334 /
C     DATA LOG10(1), LOG10(2) / '23210115, '10237777 /
C
C     MACHINE CONSTANTS FOR THE HONEYWELL 600/6000 SERIES
C
C     DATA SMALL(1), SMALL(2) / O402400000000, O000000000000 /
C     DATA LARGE(1), LARGE(2) / O376777777777, O777777777777 /
C     DATA RIGHT(1), RIGHT(2) / O604400000000, O000000000000 /
C     DATA DIVER(1), DIVER(2) / O606400000000, O000000000000 /
C     DATA LOG10(1), LOG10(2) / O776464202324, O117571775714 /
C
C     MACHINE CONSTANTS FOR THE HP 730
C
C     DATA DMACH(1) / Z'0010000000000000' /
C     DATA DMACH(2) / Z'7FEFFFFFFFFFFFFF' /
C     DATA DMACH(3) / Z'3CA0000000000000' /
C     DATA DMACH(4) / Z'3CB0000000000000' /
C     DATA DMACH(5) / Z'3FD34413509F79FF' /
C
C     MACHINE CONSTANTS FOR THE HP 2100
C     THREE WORD DOUBLE PRECISION OPTION WITH FTN4
C
C     DATA SMALL(1), SMALL(2), SMALL(3) / 40000B,       0,       1 /
C     DATA LARGE(1), LARGE(2), LARGE(3) / 77777B, 177777B, 177776B /
C     DATA RIGHT(1), RIGHT(2), RIGHT(3) / 40000B,       0,    265B /
C     DATA DIVER(1), DIVER(2), DIVER(3) / 40000B,       0,    276B /
C     DATA LOG10(1), LOG10(2), LOG10(3) / 46420B,  46502B,  77777B /
C
C     MACHINE CONSTANTS FOR THE HP 2100
C     FOUR WORD DOUBLE PRECISION OPTION WITH FTN4
C
C     DATA SMALL(1), SMALL(2) /  40000B,       0 /
C     DATA SMALL(3), SMALL(4) /       0,       1 /
C     DATA LARGE(1), LARGE(2) /  77777B, 177777B /
C     DATA LARGE(3), LARGE(4) / 177777B, 177776B /
C     DATA RIGHT(1), RIGHT(2) /  40000B,       0 /
C     DATA RIGHT(3), RIGHT(4) /       0,    225B /
C     DATA DIVER(1), DIVER(2) /  40000B,       0 /
C     DATA DIVER(3), DIVER(4) /       0,    227B /
C     DATA LOG10(1), LOG10(2) /  46420B,  46502B /
C     DATA LOG10(3), LOG10(4) /  76747B, 176377B /
C
C     MACHINE CONSTANTS FOR THE HP 9000
C
C     DATA SMALL(1), SMALL(2) / 00040000000B, 00000000000B /
C     DATA LARGE(1), LARGE(2) / 17737777777B, 37777777777B /
C     DATA RIGHT(1), RIGHT(2) / 07454000000B, 00000000000B /
C     DATA DIVER(1), DIVER(2) / 07460000000B, 00000000000B /
C     DATA LOG10(1), LOG10(2) / 07764642023B, 12047674777B /
C
C     MACHINE CONSTANTS FOR THE IBM 360/370 SERIES,
C     THE XEROX SIGMA 5/7/9, THE SEL SYSTEMS 85/86, AND
C     THE PERKIN ELMER (INTERDATA) 7/32.
C
C     DATA SMALL(1), SMALL(2) / Z00100000, Z00000000 /
C     DATA LARGE(1), LARGE(2) / Z7FFFFFFF, ZFFFFFFFF /
C     DATA RIGHT(1), RIGHT(2) / Z33100000, Z00000000 /
C     DATA DIVER(1), DIVER(2) / Z34100000, Z00000000 /
C     DATA LOG10(1), LOG10(2) / Z41134413, Z509F79FF /
C
C     MACHINE CONSTANTS FOR THE IBM PC
C     ASSUMES THAT ALL ARITHMETIC IS DONE IN DOUBLE PRECISION
C     ON 8088, I.E., NOT IN 80 BIT FORM FOR THE 8087.
C
      DATA SMALL(1) / 2.23D-308  /
      DATA LARGE(1) / 1.79D+308  /
      DATA RIGHT(1) / 1.11D-16   /
      DATA DIVER(1) / 2.22D-16   /
      DATA LOG10(1) / 0.301029995663981195D0 /
C
C     MACHINE CONSTANTS FOR THE IBM RS 6000
C
C     DATA DMACH(1) / Z'0010000000000000' /
C     DATA DMACH(2) / Z'7FEFFFFFFFFFFFFF' /
C     DATA DMACH(3) / Z'3CA0000000000000' /
C     DATA DMACH(4) / Z'3CB0000000000000' /
C     DATA DMACH(5) / Z'3FD34413509F79FF' /
C
C     MACHINE CONSTANTS FOR THE INTEL i860
C
C     DATA DMACH(1) / Z'0010000000000000' /
C     DATA DMACH(2) / Z'7FEFFFFFFFFFFFFF' /
C     DATA DMACH(3) / Z'3CA0000000000000' /
C     DATA DMACH(4) / Z'3CB0000000000000' /
C     DATA DMACH(5) / Z'3FD34413509F79FF' /
C
C     MACHINE CONSTANTS FOR THE PDP-10 (KA PROCESSOR)
C
C     DATA SMALL(1), SMALL(2) / "033400000000, "000000000000 /
C     DATA LARGE(1), LARGE(2) / "377777777777, "344777777777 /
C     DATA RIGHT(1), RIGHT(2) / "113400000000, "000000000000 /
C     DATA DIVER(1), DIVER(2) / "114400000000, "000000000000 /
C     DATA LOG10(1), LOG10(2) / "177464202324, "144117571776 /
C
C     MACHINE CONSTANTS FOR THE PDP-10 (KI PROCESSOR)
C
C     DATA SMALL(1), SMALL(2) / "000400000000, "000000000000 /
C     DATA LARGE(1), LARGE(2) / "377777777777, "377777777777 /
C     DATA RIGHT(1), RIGHT(2) / "103400000000, "000000000000 /
C     DATA DIVER(1), DIVER(2) / "104400000000, "000000000000 /
C     DATA LOG10(1), LOG10(2) / "177464202324, "476747767461 /
C
C     MACHINE CONSTANTS FOR PDP-11 FORTRAN SUPPORTING
C     32-BIT INTEGERS (EXPRESSED IN INTEGER AND OCTAL).
C
C     DATA SMALL(1), SMALL(2) /    8388608,           0 /
C     DATA LARGE(1), LARGE(2) / 2147483647,          -1 /
C     DATA RIGHT(1), RIGHT(2) /  612368384,           0 /
C     DATA DIVER(1), DIVER(2) /  620756992,           0 /
C     DATA LOG10(1), LOG10(2) / 1067065498, -2063872008 /
C
C     DATA SMALL(1), SMALL(2) / O00040000000, O00000000000 /
C     DATA LARGE(1), LARGE(2) / O17777777777, O37777777777 /
C     DATA RIGHT(1), RIGHT(2) / O04440000000, O00000000000 /
C     DATA DIVER(1), DIVER(2) / O04500000000, O00000000000 /
C     DATA LOG10(1), LOG10(2) / O07746420232, O20476747770 /
C
C     MACHINE CONSTANTS FOR PDP-11 FORTRAN SUPPORTING
C     16-BIT INTEGERS (EXPRESSED IN INTEGER AND OCTAL).
C
C     DATA SMALL(1), SMALL(2) /    128,      0 /
C     DATA SMALL(3), SMALL(4) /      0,      0 /
C     DATA LARGE(1), LARGE(2) /  32767,     -1 /
C     DATA LARGE(3), LARGE(4) /     -1,     -1 /
C     DATA RIGHT(1), RIGHT(2) /   9344,      0 /
C     DATA RIGHT(3), RIGHT(4) /      0,      0 /
C     DATA DIVER(1), DIVER(2) /   9472,      0 /
C     DATA DIVER(3), DIVER(4) /      0,      0 /
C     DATA LOG10(1), LOG10(2) /  16282,   8346 /
C     DATA LOG10(3), LOG10(4) / -31493, -12296 /
C
C     DATA SMALL(1), SMALL(2) / O000200, O000000 /
C     DATA SMALL(3), SMALL(4) / O000000, O000000 /
C     DATA LARGE(1), LARGE(2) / O077777, O177777 /
C     DATA LARGE(3), LARGE(4) / O177777, O177777 /
C     DATA RIGHT(1), RIGHT(2) / O022200, O000000 /
C     DATA RIGHT(3), RIGHT(4) / O000000, O000000 /
C     DATA DIVER(1), DIVER(2) / O022400, O000000 /
C     DATA DIVER(3), DIVER(4) / O000000, O000000 /
C     DATA LOG10(1), LOG10(2) / O037632, O020232 /
C     DATA LOG10(3), LOG10(4) / O102373, O147770 /
C
C     MACHINE CONSTANTS FOR THE SUN
C
c     DATA DMACH(1) / Z'0010000000000000' /
c     DATA DMACH(2) / Z'7FEFFFFFFFFFFFFF' /
c     DATA DMACH(3) / Z'3CA0000000000000' /
c     DATA DMACH(4) / Z'3CB0000000000000' /
c     DATA DMACH(5) / Z'3FD34413509F79FF' /
C
C     MACHINE CONSTANTS FOR THE SUN
C     USING THE -r8 COMPILER OPTION
C
C     DATA DMACH(1) / Z'00010000000000000000000000000000' /
C     DATA DMACH(2) / Z'7FFEFFFFFFFFFFFFFFFFFFFFFFFFFFFF' /
C     DATA DMACH(3) / Z'3F8E0000000000000000000000000000' /
C     DATA DMACH(4) / Z'3F8F0000000000000000000000000000' /
C     DATA DMACH(5) / Z'3FFD34413509F79FEF311F12B35816F9' /
C
C     MACHINE CONSTANTS FOR THE SUN 386i
C
C     DATA SMALL(1), SMALL(2) / Z'FFFFFFFD', Z'000FFFFF' /
C     DATA LARGE(1), LARGE(2) / Z'FFFFFFB0', Z'7FEFFFFF' /
C     DATA RIGHT(1), RIGHT(2) / Z'000000B0', Z'3CA00000' /
C     DATA DIVER(1), DIVER(2) / Z'FFFFFFCB', Z'3CAFFFFF'
C     DATA LOG10(1), LOG10(2) / Z'509F79E9', Z'3FD34413' /
C
C     MACHINE CONSTANTS FOR THE UNIVAC 1100 SERIES FTN COMPILER
C
C     DATA SMALL(1), SMALL(2) / O000040000000, O000000000000 /
C     DATA LARGE(1), LARGE(2) / O377777777777, O777777777777 /
C     DATA RIGHT(1), RIGHT(2) / O170540000000, O000000000000 /
C     DATA DIVER(1), DIVER(2) / O170640000000, O000000000000 /
C     DATA LOG10(1), LOG10(2) / O177746420232, O411757177572 /
C
C     MACHINE CONSTANTS FOR THE VAX (D-FLOATING)
C     (EXPRESSED IN INTEGER AND HEXADECIMAL)
C     THE HEX FORMAT BELOW MAY NOT BE SUITABLE FOR UNIX SYSYEMS
C     THE INTEGER FORMAT SHOULD BE OK FOR UNIX SYSTEMS
C
C     DATA SMALL(1), SMALL(2) /        128,           0 /
C     DATA LARGE(1), LARGE(2) /     -32769,          -1 /
C     DATA RIGHT(1), RIGHT(2) /       9344,           0 /
C     DATA DIVER(1), DIVER(2) /       9472,           0 /
C     DATA LOG10(1), LOG10(2) /  546979738,  -805796613 /
C
C     DATA SMALL(1), SMALL(2) / Z00000080, Z00000000 /
C     DATA LARGE(1), LARGE(2) / ZFFFF7FFF, ZFFFFFFFF /
C     DATA RIGHT(1), RIGHT(2) / Z00002480, Z00000000 /
C     DATA DIVER(1), DIVER(2) / Z00002500, Z00000000 /
C     DATA LOG10(1), LOG10(2) / Z209A3F9A, ZCFF884FB /
C
C     MACHINE CONSTANTS FOR THE VAX (G-FLOATING)
C     (EXPRESSED IN INTEGER AND HEXADECIMAL)
C     THE HEX FORMAT BELOW MAY NOT BE SUITABLE FOR UNIX SYSYEMS
C     THE INTEGER FORMAT SHOULD BE OK FOR UNIX SYSTEMS
C
C     DATA SMALL(1), SMALL(2) /         16,           0 /
C     DATA LARGE(1), LARGE(2) /     -32769,          -1 /
C     DATA RIGHT(1), RIGHT(2) /      15552,           0 /
C     DATA DIVER(1), DIVER(2) /      15568,           0 /
C     DATA LOG10(1), LOG10(2) /  1142112243, 2046775455 /
C
C     DATA SMALL(1), SMALL(2) / Z00000010, Z00000000 /
C     DATA LARGE(1), LARGE(2) / ZFFFF7FFF, ZFFFFFFFF /
C     DATA RIGHT(1), RIGHT(2) / Z00003CC0, Z00000000 /
C     DATA DIVER(1), DIVER(2) / Z00003CD0, Z00000000 /
C     DATA LOG10(1), LOG10(2) / Z44133FF3, Z79FF509F /
C
C***FIRST EXECUTABLE STATEMENT  D1MACH
      IF (I .LT. 1 .OR. I .GT. 5) CALL XERMSG ('SLATEC', 'D1MACH',
     +   'I OUT OF BOUNDS', 1, 2)
C
      D1MACH = DMACH(I)
      RETURN
C
      END
      INTEGER FUNCTION I1MACH (I)
C
      INTEGER IMACH(16),OUTPUT
      SAVE IMACH
      EQUIVALENCE (IMACH(4),OUTPUT)
C
C     MACHINE CONSTANTS FOR THE AMIGA
C     ABSOFT COMPILER
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          5 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         31 /
C     DATA IMACH( 9) / 2147483647 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -126 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         53 /
C     DATA IMACH(15) /      -1022 /
C     DATA IMACH(16) /       1023 /
C
C     MACHINE CONSTANTS FOR THE APOLLO
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          6 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         31 /
C     DATA IMACH( 9) / 2147483647 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -125 /
C     DATA IMACH(13) /        129 /
C     DATA IMACH(14) /         53 /
C     DATA IMACH(15) /      -1021 /
C     DATA IMACH(16) /       1025 /
C
C     MACHINE CONSTANTS FOR THE BURROUGHS 1700 SYSTEM
C
C     DATA IMACH( 1) /          7 /
C     DATA IMACH( 2) /          2 /
C     DATA IMACH( 3) /          2 /
C     DATA IMACH( 4) /          2 /
C     DATA IMACH( 5) /         36 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         33 /
C     DATA IMACH( 9) / Z1FFFFFFFF /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -256 /
C     DATA IMACH(13) /        255 /
C     DATA IMACH(14) /         60 /
C     DATA IMACH(15) /       -256 /
C     DATA IMACH(16) /        255 /
C
C     MACHINE CONSTANTS FOR THE BURROUGHS 5700 SYSTEM
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          7 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         48 /
C     DATA IMACH( 6) /          6 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         39 /
C     DATA IMACH( 9) / O0007777777777777 /
C     DATA IMACH(10) /          8 /
C     DATA IMACH(11) /         13 /
C     DATA IMACH(12) /        -50 /
C     DATA IMACH(13) /         76 /
C     DATA IMACH(14) /         26 /
C     DATA IMACH(15) /        -50 /
C     DATA IMACH(16) /         76 /
C
C     MACHINE CONSTANTS FOR THE BURROUGHS 6700/7700 SYSTEMS
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          7 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         48 /
C     DATA IMACH( 6) /          6 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         39 /
C     DATA IMACH( 9) / O0007777777777777 /
C     DATA IMACH(10) /          8 /
C     DATA IMACH(11) /         13 /
C     DATA IMACH(12) /        -50 /
C     DATA IMACH(13) /         76 /
C     DATA IMACH(14) /         26 /
C     DATA IMACH(15) /     -32754 /
C     DATA IMACH(16) /      32780 /
C
C     MACHINE CONSTANTS FOR THE CDC 170/180 SERIES USING NOS/VE
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          7 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         64 /
C     DATA IMACH( 6) /          8 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         63 /
C     DATA IMACH( 9) / 9223372036854775807 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         47 /
C     DATA IMACH(12) /      -4095 /
C     DATA IMACH(13) /       4094 /
C     DATA IMACH(14) /         94 /
C     DATA IMACH(15) /      -4095 /
C     DATA IMACH(16) /       4094 /
C
C     MACHINE CONSTANTS FOR THE CDC 6000/7000 SERIES
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          7 /
C     DATA IMACH( 4) /    6LOUTPUT/
C     DATA IMACH( 5) /         60 /
C     DATA IMACH( 6) /         10 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         48 /
C     DATA IMACH( 9) / 00007777777777777777B /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         47 /
C     DATA IMACH(12) /       -929 /
C     DATA IMACH(13) /       1070 /
C     DATA IMACH(14) /         94 /
C     DATA IMACH(15) /       -929 /
C     DATA IMACH(16) /       1069 /
C
C     MACHINE CONSTANTS FOR THE CELERITY C1260
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          6 /
C     DATA IMACH( 4) /          0 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         31 /
C     DATA IMACH( 9) / Z'7FFFFFFF' /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -126 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         53 /
C     DATA IMACH(15) /      -1022 /
C     DATA IMACH(16) /       1023 /
C
C     MACHINE CONSTANTS FOR THE CONVEX
C     USING THE -fn COMPILER OPTION
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          7 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         31 /
C     DATA IMACH( 9) / 2147483647 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -127 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         53 /
C     DATA IMACH(15) /      -1023 /
C     DATA IMACH(16) /       1023 /
C
C     MACHINE CONSTANTS FOR THE CONVEX
C     USING THE -fi COMPILER OPTION
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          7 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         31 /
C     DATA IMACH( 9) / 2147483647 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -125 /
C     DATA IMACH(13) /        128 /
C     DATA IMACH(14) /         53 /
C     DATA IMACH(15) /      -1021 /
C     DATA IMACH(16) /       1024 /
C
C     MACHINE CONSTANTS FOR THE CONVEX
C     USING THE -p8 COMPILER OPTION
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          7 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         63 /
C     DATA IMACH( 9) / 9223372036854775807 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         53 /
C     DATA IMACH(12) /      -1023 /
C     DATA IMACH(13) /       1023 /
C     DATA IMACH(14) /        113 /
C     DATA IMACH(15) /     -16383 /
C     DATA IMACH(16) /      16383 /
C
C     MACHINE CONSTANTS FOR THE CONVEX
C     USING THE -pd8 COMPILER OPTION
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          7 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         63 /
C     DATA IMACH( 9) / 9223372036854775807 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         53 /
C     DATA IMACH(12) /      -1023 /
C     DATA IMACH(13) /       1023 /
C     DATA IMACH(14) /         53 /
C     DATA IMACH(15) /      -1023 /
C     DATA IMACH(16) /       1023 /
C
C     MACHINE CONSTANTS FOR THE CRAY
C     USING THE 46 BIT INTEGER COMPILER OPTION
C
C     DATA IMACH( 1) /        100 /
C     DATA IMACH( 2) /        101 /
C     DATA IMACH( 3) /        102 /
C     DATA IMACH( 4) /        101 /
C     DATA IMACH( 5) /         64 /
C     DATA IMACH( 6) /          8 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         46 /
C     DATA IMACH( 9) / 1777777777777777B /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         47 /
C     DATA IMACH(12) /      -8189 /
C     DATA IMACH(13) /       8190 /
C     DATA IMACH(14) /         94 /
C     DATA IMACH(15) /      -8099 /
C     DATA IMACH(16) /       8190 /
C
C     MACHINE CONSTANTS FOR THE CRAY
C     USING THE 64 BIT INTEGER COMPILER OPTION
C
C     DATA IMACH( 1) /        100 /
C     DATA IMACH( 2) /        101 /
C     DATA IMACH( 3) /        102 /
C     DATA IMACH( 4) /        101 /
C     DATA IMACH( 5) /         64 /
C     DATA IMACH( 6) /          8 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         63 /
C     DATA IMACH( 9) / 777777777777777777777B /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         47 /
C     DATA IMACH(12) /      -8189 /
C     DATA IMACH(13) /       8190 /
C     DATA IMACH(14) /         94 /
C     DATA IMACH(15) /      -8099 /
C     DATA IMACH(16) /       8190 /
C
C     MACHINE CONSTANTS FOR THE DATA GENERAL ECLIPSE S/200
C
C     DATA IMACH( 1) /         11 /
C     DATA IMACH( 2) /         12 /
C     DATA IMACH( 3) /          8 /
C     DATA IMACH( 4) /         10 /
C     DATA IMACH( 5) /         16 /
C     DATA IMACH( 6) /          2 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         15 /
C     DATA IMACH( 9) /      32767 /
C     DATA IMACH(10) /         16 /
C     DATA IMACH(11) /          6 /
C     DATA IMACH(12) /        -64 /
C     DATA IMACH(13) /         63 /
C     DATA IMACH(14) /         14 /
C     DATA IMACH(15) /        -64 /
C     DATA IMACH(16) /         63 /
C
C     MACHINE CONSTANTS FOR THE DEC RISC
C
c     DATA IMACH( 1) /          5 /
c     DATA IMACH( 2) /          6 /
c     DATA IMACH( 3) /          6 /
c     DATA IMACH( 4) /          6 /
c     DATA IMACH( 5) /         32 /
c     DATA IMACH( 6) /          4 /
c     DATA IMACH( 7) /          2 /
c     DATA IMACH( 8) /         31 /
c     DATA IMACH( 9) / 2147483647 /
c     DATA IMACH(10) /          2 /
c     DATA IMACH(11) /         24 /
c     DATA IMACH(12) /       -125 /
c     DATA IMACH(13) /        128 /
c     DATA IMACH(14) /         53 /
c     DATA IMACH(15) /      -1021 /
c     DATA IMACH(16) /       1024 /
C
C     MACHINE CONSTANTS FOR THE ELXSI 6400
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          6 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         32 /
C     DATA IMACH( 9) / 2147483647 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -126 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         53 /
C     DATA IMACH(15) /      -1022 /
C     DATA IMACH(16) /       1023 /
C
C     MACHINE CONSTANTS FOR THE HARRIS 220
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          0 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         24 /
C     DATA IMACH( 6) /          3 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         23 /
C     DATA IMACH( 9) /    8388607 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         23 /
C     DATA IMACH(12) /       -127 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         38 /
C     DATA IMACH(15) /       -127 /
C     DATA IMACH(16) /        127 /
C
C     MACHINE CONSTANTS FOR THE HONEYWELL 600/6000 SERIES
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /         43 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         36 /
C     DATA IMACH( 6) /          6 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         35 /
C     DATA IMACH( 9) / O377777777777 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         27 /
C     DATA IMACH(12) /       -127 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         63 /
C     DATA IMACH(15) /       -127 /
C     DATA IMACH(16) /        127 /
C
C     MACHINE CONSTANTS FOR THE HP 730
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          6 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         31 /
C     DATA IMACH( 9) / 2147483647 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -125 /
C     DATA IMACH(13) /        128 /
C     DATA IMACH(14) /         53 /
C     DATA IMACH(15) /      -1021 /
C     DATA IMACH(16) /       1024 /
C
C     MACHINE CONSTANTS FOR THE HP 2100
C     3 WORD DOUBLE PRECISION OPTION WITH FTN4
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          4 /
C     DATA IMACH( 4) /          1 /
C     DATA IMACH( 5) /         16 /
C     DATA IMACH( 6) /          2 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         15 /
C     DATA IMACH( 9) /      32767 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         23 /
C     DATA IMACH(12) /       -128 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         39 /
C     DATA IMACH(15) /       -128 /
C     DATA IMACH(16) /        127 /
C
C     MACHINE CONSTANTS FOR THE HP 2100
C     4 WORD DOUBLE PRECISION OPTION WITH FTN4
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          4 /
C     DATA IMACH( 4) /          1 /
C     DATA IMACH( 5) /         16 /
C     DATA IMACH( 6) /          2 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         15 /
C     DATA IMACH( 9) /      32767 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         23 /
C     DATA IMACH(12) /       -128 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         55 /
C     DATA IMACH(15) /       -128 /
C     DATA IMACH(16) /        127 /
C
C     MACHINE CONSTANTS FOR THE HP 9000
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          6 /
C     DATA IMACH( 4) /          7 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         32 /
C     DATA IMACH( 9) / 2147483647 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -126 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         53 /
C     DATA IMACH(15) /      -1015 /
C     DATA IMACH(16) /       1017 /
C
C     MACHINE CONSTANTS FOR THE IBM 360/370 SERIES,
C     THE XEROX SIGMA 5/7/9, THE SEL SYSTEMS 85/86, AND
C     THE PERKIN ELMER (INTERDATA) 7/32.
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          7 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         31 /
C     DATA IMACH( 9) /  Z7FFFFFFF /
C     DATA IMACH(10) /         16 /
C     DATA IMACH(11) /          6 /
C     DATA IMACH(12) /        -64 /
C     DATA IMACH(13) /         63 /
C     DATA IMACH(14) /         14 /
C     DATA IMACH(15) /        -64 /
C     DATA IMACH(16) /         63 /
C
C     MACHINE CONSTANTS FOR THE IBM PC
C
      DATA IMACH( 1) /          5 /
      DATA IMACH( 2) /          6 /
      DATA IMACH( 3) /          0 /
      DATA IMACH( 4) /          0 /
      DATA IMACH( 5) /         32 /
      DATA IMACH( 6) /          4 /
      DATA IMACH( 7) /          2 /
      DATA IMACH( 8) /         31 /
      DATA IMACH( 9) / 2147483647 /
      DATA IMACH(10) /          2 /
      DATA IMACH(11) /         24 /
      DATA IMACH(12) /       -125 /
      DATA IMACH(13) /        127 /
      DATA IMACH(14) /         53 /
      DATA IMACH(15) /      -1021 /
      DATA IMACH(16) /       1023 /
c
C     MACHINE CONSTANTS FOR THE IBM RS 6000
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          6 /
C     DATA IMACH( 4) /          0 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         31 /
C     DATA IMACH( 9) / 2147483647 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -125 /
C     DATA IMACH(13) /        128 /
C     DATA IMACH(14) /         53 /
C     DATA IMACH(15) /      -1021 /
C     DATA IMACH(16) /       1024 /
C
C     MACHINE CONSTANTS FOR THE INTEL i860
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          6 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         31 /
C     DATA IMACH( 9) / 2147483647 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -125 /
C     DATA IMACH(13) /        128 /
C     DATA IMACH(14) /         53 /
C     DATA IMACH(15) /      -1021 /
C     DATA IMACH(16) /       1024 /
C
C     MACHINE CONSTANTS FOR THE PDP-10 (KA PROCESSOR)
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          5 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         36 /
C     DATA IMACH( 6) /          5 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         35 /
C     DATA IMACH( 9) / "377777777777 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         27 /
C     DATA IMACH(12) /       -128 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         54 /
C     DATA IMACH(15) /       -101 /
C     DATA IMACH(16) /        127 /
C
C     MACHINE CONSTANTS FOR THE PDP-10 (KI PROCESSOR)
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          5 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         36 /
C     DATA IMACH( 6) /          5 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         35 /
C     DATA IMACH( 9) / "377777777777 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         27 /
C     DATA IMACH(12) /       -128 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         62 /
C     DATA IMACH(15) /       -128 /
C     DATA IMACH(16) /        127 /
C
C     MACHINE CONSTANTS FOR PDP-11 FORTRAN SUPPORTING
C     32-BIT INTEGER ARITHMETIC.
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          5 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         31 /
C     DATA IMACH( 9) / 2147483647 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -127 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         56 /
C     DATA IMACH(15) /       -127 /
C     DATA IMACH(16) /        127 /
C
C     MACHINE CONSTANTS FOR PDP-11 FORTRAN SUPPORTING
C     16-BIT INTEGER ARITHMETIC.
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          5 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         16 /
C     DATA IMACH( 6) /          2 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         15 /
C     DATA IMACH( 9) /      32767 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -127 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         56 /
C     DATA IMACH(15) /       -127 /
C     DATA IMACH(16) /        127 /
C
C     MACHINE CONSTANTS FOR THE SUN
C
c     DATA IMACH( 1) /          5 /
c     DATA IMACH( 2) /          6 /
c     DATA IMACH( 3) /          6 /
c     DATA IMACH( 4) /          6 /
c     DATA IMACH( 5) /         32 /
c     DATA IMACH( 6) /          4 /
c     DATA IMACH( 7) /          2 /
c     DATA IMACH( 8) /         31 /
c     DATA IMACH( 9) / 2147483647 /
c     DATA IMACH(10) /          2 /
c     DATA IMACH(11) /         24 /
c     DATA IMACH(12) /       -125 /
c     DATA IMACH(13) /        128 /
c     DATA IMACH(14) /         53 /
c     DATA IMACH(15) /      -1021 /
c     DATA IMACH(16) /       1024 /
C
C     MACHINE CONSTANTS FOR THE SUN
C     USING THE -r8 COMPILER OPTION
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          6 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         31 /
C     DATA IMACH( 9) / 2147483647 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         53 /
C     DATA IMACH(12) /      -1021 /
C     DATA IMACH(13) /       1024 /
C     DATA IMACH(14) /        113 /
C     DATA IMACH(15) /     -16381 /
C     DATA IMACH(16) /      16384 /
C
C     MACHINE CONSTANTS FOR THE UNIVAC 1100 SERIES FTN COMPILER
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          1 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         36 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         35 /
C     DATA IMACH( 9) / O377777777777 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         27 /
C     DATA IMACH(12) /       -128 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         60 /
C     DATA IMACH(15) /      -1024 /
C     DATA IMACH(16) /       1023 /
C
C     MACHINE CONSTANTS FOR THE VAX (D-FLOATING)
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          5 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         31 /
C     DATA IMACH( 9) / 2147483647 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -127 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         56 /
C     DATA IMACH(15) /       -127 /
C     DATA IMACH(16) /        127 /
C
C     MACHINE CONSTANTS FOR THE VAX (G-FLOATING)
C
C     DATA IMACH( 1) /          5 /
C     DATA IMACH( 2) /          6 /
C     DATA IMACH( 3) /          5 /
C     DATA IMACH( 4) /          6 /
C     DATA IMACH( 5) /         32 /
C     DATA IMACH( 6) /          4 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         31 /
C     DATA IMACH( 9) / 2147483647 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -127 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         53 /
C     DATA IMACH(15) /      -1023 /
C     DATA IMACH(16) /       1023 /
C
C     MACHINE CONSTANTS FOR THE Z80 MICROPROCESSOR
C
C     DATA IMACH( 1) /          1 /
C     DATA IMACH( 2) /          1 /
C     DATA IMACH( 3) /          0 /
C     DATA IMACH( 4) /          1 /
C     DATA IMACH( 5) /         16 /
C     DATA IMACH( 6) /          2 /
C     DATA IMACH( 7) /          2 /
C     DATA IMACH( 8) /         15 /
C     DATA IMACH( 9) /      32767 /
C     DATA IMACH(10) /          2 /
C     DATA IMACH(11) /         24 /
C     DATA IMACH(12) /       -127 /
C     DATA IMACH(13) /        127 /
C     DATA IMACH(14) /         56 /
C     DATA IMACH(15) /       -127 /
C     DATA IMACH(16) /        127 /
C
C***FIRST EXECUTABLE STATEMENT  I1MACH
      IF (I .LT. 1  .OR.  I .GT. 16) GO TO 10
C
      I1MACH = IMACH(I)
      RETURN
C
   10 CONTINUE
      WRITE (UNIT = OUTPUT, FMT = 9000)
 9000 FORMAT ('1ERROR    1 IN I1MACH - I OUT OF BOUNDS')
C
C     CALL FDUMP
C
      STOP
      END
      SUBROUTINE FDUMP
C***FIRST EXECUTABLE STATEMENT  FDUMP
      RETURN
      END
      SUBROUTINE XERHLT (MESSG)
      CHARACTER*(*) MESSG
C***FIRST EXECUTABLE STATEMENT  XERHLT
      STOP "PROGRAM STOP THROUGH CALL TO XERHLT"
      END
C *** END OF FILES REQUIRING USER EDIT ***
      FUNCTION J4SAVE (IWHICH, IVALUE, ISET)
      LOGICAL ISET
      INTEGER IPARAM(9)
      SAVE IPARAM
      DATA IPARAM(1),IPARAM(2),IPARAM(3),IPARAM(4)/0,2,0,10/
      DATA IPARAM(5)/1/
      DATA IPARAM(6),IPARAM(7),IPARAM(8),IPARAM(9)/0,0,0,0/
C***FIRST EXECUTABLE STATEMENT  J4SAVE
      J4SAVE = IPARAM(IWHICH)
      IF (ISET) IPARAM(IWHICH) = IVALUE
      RETURN
      END
      SUBROUTINE XGETUA (IUNITA, N)
      DIMENSION IUNITA(5)
C***FIRST EXECUTABLE STATEMENT  XGETUA
      N = J4SAVE(5,0,.FALSE.)
      DO 30 I=1,N
         INDEX = I+4
         IF (I.EQ.1) INDEX = 3
         IUNITA(I) = J4SAVE(INDEX,0,.FALSE.)
   30 CONTINUE
      RETURN
      END
      SUBROUTINE XERSVE (LIBRAR, SUBROU, MESSG, KFLAG, NERR, LEVEL,
     +   ICOUNT)
      PARAMETER (LENTAB=10)
      INTEGER LUN(5)
      CHARACTER*(*) LIBRAR, SUBROU, MESSG
      CHARACTER*8  LIBTAB(LENTAB), SUBTAB(LENTAB), LIB, SUB
      CHARACTER*20 MESTAB(LENTAB), MES
      DIMENSION NERTAB(LENTAB), LEVTAB(LENTAB), KOUNT(LENTAB)
      SAVE LIBTAB, SUBTAB, MESTAB, NERTAB, LEVTAB, KOUNT, KOUNTX, NMSG
      DATA KOUNTX/0/, NMSG/0/
C***FIRST EXECUTABLE STATEMENT  XERSVE
C
      IF (KFLAG.LE.0) THEN
C
C        Dump the table.
C
         IF (NMSG.EQ.0) RETURN
C
C        Print to each unit.
C
         CALL XGETUA (LUN, NUNIT)
         DO 20 KUNIT = 1,NUNIT
            IUNIT = LUN(KUNIT)
            IF (IUNIT.EQ.0) IUNIT = I1MACH(4)
C
C           Print the table header.
C
            WRITE (IUNIT,9000)
C
C           Print body of table.
C
            DO 10 I = 1,NMSG
               WRITE (IUNIT,9010) LIBTAB(I), SUBTAB(I), MESTAB(I),
     *            NERTAB(I),LEVTAB(I),KOUNT(I)
   10       CONTINUE
C
C           Print number of other errors.
C
            IF (KOUNTX.NE.0) WRITE (IUNIT,9020) KOUNTX
            WRITE (IUNIT,9030)
   20    CONTINUE
C
C        Clear the error tables.
C
         IF (KFLAG.EQ.0) THEN
            NMSG = 0
            KOUNTX = 0
         ENDIF
      ELSE
C
C        PROCESS A MESSAGE...
C        SEARCH FOR THIS MESSG, OR ELSE AN EMPTY SLOT FOR THIS MESSG,
C        OR ELSE DETERMINE THAT THE ERROR TABLE IS FULL.
C
         LIB = LIBRAR
         SUB = SUBROU
         MES = MESSG
         DO 30 I = 1,NMSG
            IF (LIB.EQ.LIBTAB(I) .AND. SUB.EQ.SUBTAB(I) .AND.
     *         MES.EQ.MESTAB(I) .AND. NERR.EQ.NERTAB(I) .AND.
     *         LEVEL.EQ.LEVTAB(I)) THEN
                  KOUNT(I) = KOUNT(I) + 1
                  ICOUNT = KOUNT(I)
                  RETURN
            ENDIF
   30    CONTINUE
C
         IF (NMSG.LT.LENTAB) THEN
C
C           Empty slot found for new message.
C
            NMSG = NMSG + 1
            LIBTAB(I) = LIB
            SUBTAB(I) = SUB
            MESTAB(I) = MES
            NERTAB(I) = NERR
            LEVTAB(I) = LEVEL
            KOUNT (I) = 1
            ICOUNT    = 1
         ELSE
C
C           Table is full.
C
            KOUNTX = KOUNTX+1
            ICOUNT = 0
         ENDIF
      ENDIF
      RETURN
C
C     Formats.
C
 9000 FORMAT ('0          ERROR MESSAGE SUMMARY' /
     +   ' LIBRARY    SUBROUTINE MESSAGE START             NERR',
     +   '     LEVEL     COUNT')
 9010 FORMAT (1X,A,3X,A,3X,A,3I10)
 9020 FORMAT ('0OTHER ERRORS NOT INDIVIDUALLY TABULATED = ', I10)
 9030 FORMAT (1X)
      END
      SUBROUTINE XERCNT (LIBRAR, SUBROU, MESSG, NERR, LEVEL, KONTRL)
      CHARACTER*(*) LIBRAR, SUBROU, MESSG
C***FIRST EXECUTABLE STATEMENT  XERCNT
      RETURN
      END
      SUBROUTINE XERPRN (PREFIX, NPREF, MESSG, NWRAP)
      CHARACTER*(*) PREFIX, MESSG
      INTEGER NPREF, NWRAP
      CHARACTER*148 CBUFF
      INTEGER IU(5), NUNIT
      CHARACTER*2 NEWLIN
      PARAMETER (NEWLIN = '$$')
C***FIRST EXECUTABLE STATEMENT  XERPRN
      CALL XGETUA(IU,NUNIT)
C
C       A ZERO VALUE FOR A LOGICAL UNIT NUMBER MEANS TO USE THE STANDARD
C       ERROR MESSAGE UNIT INSTEAD.  I1MACH(4) RETRIEVES THE STANDARD
C       ERROR MESSAGE UNIT.
C
      N = I1MACH(4)
      DO 10 I=1,NUNIT
         IF (IU(I) .EQ. 0) IU(I) = N
   10 CONTINUE
C
C       LPREF IS THE LENGTH OF THE PREFIX.  THE PREFIX IS PLACED AT THE
C       BEGINNING OF CBUFF, THE CHARACTER BUFFER, AND KEPT THERE DURING
C       THE REST OF THIS ROUTINE.
C
      IF ( NPREF .LT. 0 ) THEN
         LPREF = LEN(PREFIX)
      ELSE
         LPREF = NPREF
      ENDIF
      LPREF = MIN(16, LPREF)
      IF (LPREF .NE. 0) CBUFF(1:LPREF) = PREFIX
C
C       LWRAP IS THE MAXIMUM NUMBER OF CHARACTERS WE WANT TO TAKE AT ONE
C       TIME FROM MESSG TO PRINT ON ONE LINE.
C
      LWRAP = MAX(16, MIN(132, NWRAP))
C
C       SET LENMSG TO THE LENGTH OF MESSG, IGNORE ANY TRAILING BLANKS.
C
      LENMSG = LEN(MESSG)
      N = LENMSG
      DO 20 I=1,N
         IF (MESSG(LENMSG:LENMSG) .NE. ' ') GO TO 30
         LENMSG = LENMSG - 1
   20 CONTINUE
   30 CONTINUE
C
C       IF THE MESSAGE IS ALL BLANKS, THEN PRINT ONE BLANK LINE.
C
      IF (LENMSG .EQ. 0) THEN
         CBUFF(LPREF+1:LPREF+1) = ' '
         DO 40 I=1,NUNIT
            WRITE(IU(I), '(A)') CBUFF(1:LPREF+1)
   40    CONTINUE
         RETURN
      ENDIF
C
C       SET NEXTC TO THE POSITION IN MESSG WHERE THE NEXT SUBSTRING
C       STARTS.  FROM THIS POSITION WE SCAN FOR THE NEW LINE SENTINEL.
C       WHEN NEXTC EXCEEDS LENMSG, THERE IS NO MORE TO PRINT.
C       WE LOOP BACK TO LABEL 50 UNTIL ALL PIECES HAVE BEEN PRINTED.
C
C       WE LOOK FOR THE NEXT OCCURRENCE OF THE NEW LINE SENTINEL.  THE
C       INDEX INTRINSIC FUNCTION RETURNS ZERO IF THERE IS NO OCCURRENCE
C       OR IF THE LENGTH OF THE FIRST ARGUMENT IS LESS THAN THE LENGTH
C       OF THE SECOND ARGUMENT.
C
C       THERE ARE SEVERAL CASES WHICH SHOULD BE CHECKED FOR IN THE
C       FOLLOWING ORDER.  WE ARE ATTEMPTING TO SET LPIECE TO THE NUMBER
C       OF CHARACTERS THAT SHOULD BE TAKEN FROM MESSG STARTING AT
C       POSITION NEXTC.
C
C       LPIECE .EQ. 0   THE NEW LINE SENTINEL DOES NOT OCCUR IN THE
C                       REMAINDER OF THE CHARACTER STRING.  LPIECE
C                       SHOULD BE SET TO LWRAP OR LENMSG+1-NEXTC,
C                       WHICHEVER IS LESS.
C
C       LPIECE .EQ. 1   THE NEW LINE SENTINEL STARTS AT MESSG(NEXTC:
C                       NEXTC).  LPIECE IS EFFECTIVELY ZERO, AND WE
C                       PRINT NOTHING TO AVOID PRODUCING UNNECESSARY
C                       BLANK LINES.  THIS TAKES CARE OF THE SITUATION
C                       WHERE THE LIBRARY ROUTINE HAS A MESSAGE OF
C                       EXACTLY 72 CHARACTERS FOLLOWED BY A NEW LINE
C                       SENTINEL FOLLOWED BY MORE CHARACTERS.  NEXTC
C                       SHOULD BE INCREMENTED BY 2.
C
C       LPIECE .GT. LWRAP+1  REDUCE LPIECE TO LWRAP.
C
C       ELSE            THIS LAST CASE MEANS 2 .LE. LPIECE .LE. LWRAP+1
C                       RESET LPIECE = LPIECE-1.  NOTE THAT THIS
C                       PROPERLY HANDLES THE END CASE WHERE LPIECE .EQ.
C                       LWRAP+1.  THAT IS, THE SENTINEL FALLS EXACTLY
C                       AT THE END OF A LINE.
C
      NEXTC = 1
   50 LPIECE = INDEX(MESSG(NEXTC:LENMSG), NEWLIN)
      IF (LPIECE .EQ. 0) THEN
C
C       THERE WAS NO NEW LINE SENTINEL FOUND.
C
         IDELTA = 0
         LPIECE = MIN(LWRAP, LENMSG+1-NEXTC)
         IF (LPIECE .LT. LENMSG+1-NEXTC) THEN
            DO 52 I=LPIECE+1,2,-1
               IF (MESSG(NEXTC+I-1:NEXTC+I-1) .EQ. ' ') THEN
                  LPIECE = I-1
                  IDELTA = 1
                  GOTO 54
               ENDIF
   52       CONTINUE
         ENDIF
   54    CBUFF(LPREF+1:LPREF+LPIECE) = MESSG(NEXTC:NEXTC+LPIECE-1)
         NEXTC = NEXTC + LPIECE + IDELTA
      ELSEIF (LPIECE .EQ. 1) THEN
C
C       WE HAVE A NEW LINE SENTINEL AT MESSG(NEXTC:NEXTC+1).
C       DON'T PRINT A BLANK LINE.
C
         NEXTC = NEXTC + 2
         GO TO 50
      ELSEIF (LPIECE .GT. LWRAP+1) THEN
C
C       LPIECE SHOULD BE SET DOWN TO LWRAP.
C
         IDELTA = 0
         LPIECE = LWRAP
         DO 56 I=LPIECE+1,2,-1
            IF (MESSG(NEXTC+I-1:NEXTC+I-1) .EQ. ' ') THEN
               LPIECE = I-1
               IDELTA = 1
               GOTO 58
            ENDIF
   56    CONTINUE
   58    CBUFF(LPREF+1:LPREF+LPIECE) = MESSG(NEXTC:NEXTC+LPIECE-1)
         NEXTC = NEXTC + LPIECE + IDELTA
      ELSE
C
C       IF WE ARRIVE HERE, IT MEANS 2 .LE. LPIECE .LE. LWRAP+1.
C       WE SHOULD DECREMENT LPIECE BY ONE.
C
         LPIECE = LPIECE - 1
         CBUFF(LPREF+1:LPREF+LPIECE) = MESSG(NEXTC:NEXTC+LPIECE-1)
         NEXTC  = NEXTC + LPIECE + 2
      ENDIF
C
C       PRINT
C
      DO 60 I=1,NUNIT
         WRITE(IU(I), '(A)') CBUFF(1:LPREF+LPIECE)
   60 CONTINUE
C
      IF (NEXTC .LE. LENMSG) GO TO 50
      RETURN
      END
      SUBROUTINE XERMSG (LIBRAR, SUBROU, MESSG, NERR, LEVEL)
      CHARACTER*(*) LIBRAR, SUBROU, MESSG
      CHARACTER*8 XLIBR, XSUBR
      CHARACTER*72  TEMP
      CHARACTER*20  LFIRST
C***FIRST EXECUTABLE STATEMENT  XERMSG
      LKNTRL = J4SAVE (2, 0, .FALSE.)
      MAXMES = J4SAVE (4, 0, .FALSE.)
C
C       LKNTRL IS A LOCAL COPY OF THE CONTROL FLAG KONTRL.
C       MAXMES IS THE MAXIMUM NUMBER OF TIMES ANY PARTICULAR MESSAGE
C          SHOULD BE PRINTED.
C
C       WE PRINT A FATAL ERROR MESSAGE AND TERMINATE FOR AN ERROR IN
C          CALLING XERMSG.  THE ERROR NUMBER SHOULD BE POSITIVE,
C          AND THE LEVEL SHOULD BE BETWEEN 0 AND 2.
C
      IF (NERR.LT.-9999999 .OR. NERR.GT.99999999 .OR. NERR.EQ.0 .OR.
     *   LEVEL.LT.-1 .OR. LEVEL.GT.2) THEN
         CALL XERPRN (' ***', -1, 'FATAL ERROR IN...$$ ' //
     *      'XERMSG -- INVALID ERROR NUMBER OR LEVEL$$ '//
     *      'JOB ABORT DUE TO FATAL ERROR.', 72)
         CALL XERSVE (' ', ' ', ' ', 0, 0, 0, KDUMMY)
         CALL XERHLT (' ***XERMSG -- INVALID INPUT')
         RETURN
      ENDIF
C
C       RECORD THE MESSAGE.
C
      I = J4SAVE (1, NERR, .TRUE.)
      CALL XERSVE (LIBRAR, SUBROU, MESSG, 1, NERR, LEVEL, KOUNT)
C
C       HANDLE PRINT-ONCE WARNING MESSAGES.
C
      IF (LEVEL.EQ.-1 .AND. KOUNT.GT.1) RETURN
C
C       ALLOW TEMPORARY USER OVERRIDE OF THE CONTROL FLAG.
C
      XLIBR  = LIBRAR
      XSUBR  = SUBROU
      LFIRST = MESSG
      LERR   = NERR
      LLEVEL = LEVEL
      CALL XERCNT (XLIBR, XSUBR, LFIRST, LERR, LLEVEL, LKNTRL)
C
      LKNTRL = MAX(-2, MIN(2,LKNTRL))
      MKNTRL = ABS(LKNTRL)
C
C       SKIP PRINTING IF THE CONTROL FLAG VALUE AS RESET IN XERCNT IS
C       ZERO AND THE ERROR IS NOT FATAL.
C
      IF (LEVEL.LT.2 .AND. LKNTRL.EQ.0) GO TO 30
      IF (LEVEL.EQ.0 .AND. KOUNT.GT.MAXMES) GO TO 30
      IF (LEVEL.EQ.1 .AND. KOUNT.GT.MAXMES .AND. MKNTRL.EQ.1) GO TO 30
      IF (LEVEL.EQ.2 .AND. KOUNT.GT.MAX(1,MAXMES)) GO TO 30
C
C       ANNOUNCE THE NAMES OF THE LIBRARY AND SUBROUTINE BY BUILDING A
C       MESSAGE IN CHARACTER VARIABLE TEMP (NOT EXCEEDING 66 CHARACTERS)
C       AND SENDING IT OUT VIA XERPRN.  PRINT ONLY IF CONTROL FLAG
C       IS NOT ZERO.
C
      IF (LKNTRL .NE. 0) THEN
         TEMP(1:21) = 'MESSAGE FROM ROUTINE '
         I = MIN(LEN(SUBROU), 16)
         TEMP(22:21+I) = SUBROU(1:I)
         TEMP(22+I:33+I) = ' IN LIBRARY '
         LTEMP = 33 + I
         I = MIN(LEN(LIBRAR), 16)
         TEMP(LTEMP+1:LTEMP+I) = LIBRAR (1:I)
         TEMP(LTEMP+I+1:LTEMP+I+1) = '.'
         LTEMP = LTEMP + I + 1
         CALL XERPRN (' ***', -1, TEMP(1:LTEMP), 72)
      ENDIF
C
C       IF LKNTRL IS POSITIVE, PRINT AN INTRODUCTORY LINE BEFORE
C       PRINTING THE MESSAGE.  THE INTRODUCTORY LINE TELLS THE CHOICE
C       FROM EACH OF THE FOLLOWING THREE OPTIONS.
C       1.  LEVEL OF THE MESSAGE
C              'INFORMATIVE MESSAGE'
C              'POTENTIALLY RECOVERABLE ERROR'
C              'FATAL ERROR'
C       2.  WHETHER CONTROL FLAG WILL ALLOW PROGRAM TO CONTINUE
C              'PROG CONTINUES'
C              'PROG ABORTED'
C       3.  WHETHER OR NOT A TRACEBACK WAS REQUESTED.  (THE TRACEBACK
C           MAY NOT BE IMPLEMENTED AT SOME SITES, SO THIS ONLY TELLS
C           WHAT WAS REQUESTED, NOT WHAT WAS DELIVERED.)
C              'TRACEBACK REQUESTED'
C              'TRACEBACK NOT REQUESTED'
C       NOTICE THAT THE LINE INCLUDING FOUR PREFIX CHARACTERS WILL NOT
C       EXCEED 74 CHARACTERS.
C       WE SKIP THE NEXT BLOCK IF THE INTRODUCTORY LINE IS NOT NEEDED.
C
      IF (LKNTRL .GT. 0) THEN
C
C       THE FIRST PART OF THE MESSAGE TELLS ABOUT THE LEVEL.
C
         IF (LEVEL .LE. 0) THEN
            TEMP(1:20) = 'INFORMATIVE MESSAGE,'
            LTEMP = 20
         ELSEIF (LEVEL .EQ. 1) THEN
            TEMP(1:30) = 'POTENTIALLY RECOVERABLE ERROR,'
            LTEMP = 30
         ELSE
            TEMP(1:12) = 'FATAL ERROR,'
            LTEMP = 12
         ENDIF
C
C       THEN WHETHER THE PROGRAM WILL CONTINUE.
C
         IF ((MKNTRL.EQ.2 .AND. LEVEL.GE.1) .OR.
     *       (MKNTRL.EQ.1 .AND. LEVEL.EQ.2)) THEN
            TEMP(LTEMP+1:LTEMP+14) = ' PROG ABORTED,'
            LTEMP = LTEMP + 14
         ELSE
            TEMP(LTEMP+1:LTEMP+16) = ' PROG CONTINUES,'
            LTEMP = LTEMP + 16
         ENDIF
C
C       FINALLY TELL WHETHER THERE SHOULD BE A TRACEBACK.
C
         IF (LKNTRL .GT. 0) THEN
            TEMP(LTEMP+1:LTEMP+20) = ' TRACEBACK REQUESTED'
            LTEMP = LTEMP + 20
         ELSE
            TEMP(LTEMP+1:LTEMP+24) = ' TRACEBACK NOT REQUESTED'
            LTEMP = LTEMP + 24
         ENDIF
         CALL XERPRN (' ***', -1, TEMP(1:LTEMP), 72)
      ENDIF
C
C       NOW SEND OUT THE MESSAGE.
C
      CALL XERPRN (' *  ', -1, MESSG, 72)
C
C       IF LKNTRL IS POSITIVE, WRITE THE ERROR NUMBER AND REQUEST A
C          TRACEBACK.
C
      IF (LKNTRL .GT. 0) THEN
         WRITE (TEMP, '(''ERROR NUMBER = '', I8)') NERR
         DO 10 I=16,22
            IF (TEMP(I:I) .NE. ' ') GO TO 20
   10    CONTINUE
C
   20    CALL XERPRN (' *  ', -1, TEMP(1:15) // TEMP(I:23), 72)
         CALL FDUMP
      ENDIF
C
C       IF LKNTRL IS NOT ZERO, PRINT A BLANK LINE AND AN END OF MESSAGE.
C
      IF (LKNTRL .NE. 0) THEN
         CALL XERPRN (' *  ', -1, ' ', 72)
         CALL XERPRN (' ***', -1, 'END OF MESSAGE', 72)
         CALL XERPRN ('    ',  0, ' ', 72)
      ENDIF
C
C       IF THE ERROR IS NOT FATAL OR THE ERROR IS RECOVERABLE AND THE
C       CONTROL FLAG IS SET FOR RECOVERY, THEN RETURN.
C
   30 IF (LEVEL.LE.0 .OR. (LEVEL.EQ.1 .AND. MKNTRL.LE.1)) RETURN
C
C       THE PROGRAM WILL BE STOPPED DUE TO AN UNRECOVERED ERROR OR A
C       FATAL ERROR.  PRINT THE REASON FOR THE ABORT AND THE ERROR
C       SUMMARY IF THE CONTROL FLAG AND THE MAXIMUM ERROR COUNT PERMIT.
C
      IF (LKNTRL.GT.0 .AND. KOUNT.LT.MAX(1,MAXMES)) THEN
         IF (LEVEL .EQ. 1) THEN
            CALL XERPRN
     *         (' ***', -1, 'JOB ABORT DUE TO UNRECOVERED ERROR.', 72)
         ELSE
            CALL XERPRN(' ***', -1, 'JOB ABORT DUE TO FATAL ERROR.', 72)
         ENDIF
         CALL XERSVE (' ', ' ', ' ', -1, 0, 0, KDUMMY)
         CALL XERHLT (' ')
      ELSE
         CALL XERHLT (MESSG)
      ENDIF
      RETURN
      END
      REAL FUNCTION SDOT (N, SX, INCX, SY, INCY)
      REAL SX(*), SY(*)
C***FIRST EXECUTABLE STATEMENT  SDOT
      SDOT = 0.0E0
      IF (N .LE. 0) RETURN
      IF (INCX .EQ. INCY) IF (INCX-1) 5,20,60
C
C     Code for unequal or nonpositive increments.
C
    5 IX = 1
      IY = 1
      IF (INCX .LT. 0) IX = (-N+1)*INCX + 1
      IF (INCY .LT. 0) IY = (-N+1)*INCY + 1
      DO 10 I = 1,N
        SDOT = SDOT + SX(IX)*SY(IY)
        IX = IX + INCX
        IY = IY + INCY
   10 CONTINUE
      RETURN
C
C     Code for both increments equal to 1.
C
C     Clean-up loop so remaining vector length is a multiple of 5.
C
   20 M = MOD(N,5)
      IF (M .EQ. 0) GO TO 40
      DO 30 I = 1,M
        SDOT = SDOT + SX(I)*SY(I)
   30 CONTINUE
      IF (N .LT. 5) RETURN
   40 MP1 = M + 1
      DO 50 I = MP1,N,5
      SDOT = SDOT + SX(I)*SY(I) + SX(I+1)*SY(I+1) + SX(I+2)*SY(I+2) +
     1              SX(I+3)*SY(I+3) + SX(I+4)*SY(I+4)
   50 CONTINUE
      RETURN
C
C     Code for equal, positive, non-unit increments.
C
   60 NS = N*INCX
      DO 70 I = 1,NS,INCX
        SDOT = SDOT + SX(I)*SY(I)
   70 CONTINUE
      RETURN
      END
      DOUBLE PRECISION FUNCTION DDOT (N, DX, INCX, DY, INCY)
      DOUBLE PRECISION DX(*), DY(*)
C***FIRST EXECUTABLE STATEMENT  DDOT
      DDOT = 0.0D0
      IF (N .LE. 0) RETURN
      IF (INCX .EQ. INCY) IF (INCX-1) 5,20,60
C
C     Code for unequal or nonpositive increments.
C
    5 IX = 1
      IY = 1
      IF (INCX .LT. 0) IX = (-N+1)*INCX + 1
      IF (INCY .LT. 0) IY = (-N+1)*INCY + 1
      DO 10 I = 1,N
        DDOT = DDOT + DX(IX)*DY(IY)
        IX = IX + INCX
        IY = IY + INCY
   10 CONTINUE
      RETURN
C
C     Code for both increments equal to 1.
C
C     Clean-up loop so remaining vector length is a multiple of 5.
C
   20 M = MOD(N,5)
      IF (M .EQ. 0) GO TO 40
      DO 30 I = 1,M
         DDOT = DDOT + DX(I)*DY(I)
   30 CONTINUE
      IF (N .LT. 5) RETURN
   40 MP1 = M + 1
      DO 50 I = MP1,N,5
      DDOT = DDOT + DX(I)*DY(I) + DX(I+1)*DY(I+1) + DX(I+2)*DY(I+2) +
     1              DX(I+3)*DY(I+3) + DX(I+4)*DY(I+4)
   50 CONTINUE
      RETURN
C
C     Code for equal, positive, non-unit increments.
C
   60 NS = N*INCX
      DO 70 I = 1,NS,INCX
        DDOT = DDOT + DX(I)*DY(I)
   70 CONTINUE
      RETURN
      END
      SUBROUTINE SAXPY (N, SA, SX, INCX, SY, INCY)
      REAL SX(*), SY(*), SA
C***FIRST EXECUTABLE STATEMENT  SAXPY
      IF (N.LE.0 .OR. SA.EQ.0.0E0) RETURN
      IF (INCX .EQ. INCY) IF (INCX-1) 5,20,60
C
C     Code for unequal or nonpositive increments.
C
    5 IX = 1
      IY = 1
      IF (INCX .LT. 0) IX = (-N+1)*INCX + 1
      IF (INCY .LT. 0) IY = (-N+1)*INCY + 1
      DO 10 I = 1,N
        SY(IY) = SY(IY) + SA*SX(IX)
        IX = IX + INCX
        IY = IY + INCY
   10 CONTINUE
      RETURN
C
C     Code for both increments equal to 1.
C
C     Clean-up loop so remaining vector length is a multiple of 4.
C
   20 M = MOD(N,4)
      IF (M .EQ. 0) GO TO 40
      DO 30 I = 1,M
        SY(I) = SY(I) + SA*SX(I)
   30 CONTINUE
      IF (N .LT. 4) RETURN
   40 MP1 = M + 1
      DO 50 I = MP1,N,4
        SY(I) = SY(I) + SA*SX(I)
        SY(I+1) = SY(I+1) + SA*SX(I+1)
        SY(I+2) = SY(I+2) + SA*SX(I+2)
        SY(I+3) = SY(I+3) + SA*SX(I+3)
   50 CONTINUE
      RETURN
C
C     Code for equal, positive, non-unit increments.
C
   60 NS = N*INCX
      DO 70 I = 1,NS,INCX
        SY(I) = SA*SX(I) + SY(I)
   70 CONTINUE
      RETURN
      END
      SUBROUTINE DAXPY (N, DA, DX, INCX, DY, INCY)
      DOUBLE PRECISION DX(*), DY(*), DA
C***FIRST EXECUTABLE STATEMENT  DAXPY
      IF (N.LE.0 .OR. DA.EQ.0.0D0) RETURN
      IF (INCX .EQ. INCY) IF (INCX-1) 5,20,60
C
C     Code for unequal or nonpositive increments.
C
    5 IX = 1
      IY = 1
      IF (INCX .LT. 0) IX = (-N+1)*INCX + 1
      IF (INCY .LT. 0) IY = (-N+1)*INCY + 1
      DO 10 I = 1,N
        DY(IY) = DY(IY) + DA*DX(IX)
        IX = IX + INCX
        IY = IY + INCY
   10 CONTINUE
      RETURN
C
C     Code for both increments equal to 1.
C
C     Clean-up loop so remaining vector length is a multiple of 4.
C
   20 M = MOD(N,4)
      IF (M .EQ. 0) GO TO 40
      DO 30 I = 1,M
        DY(I) = DY(I) + DA*DX(I)
   30 CONTINUE
      IF (N .LT. 4) RETURN
   40 MP1 = M + 1
      DO 50 I = MP1,N,4
        DY(I) = DY(I) + DA*DX(I)
        DY(I+1) = DY(I+1) + DA*DX(I+1)
        DY(I+2) = DY(I+2) + DA*DX(I+2)
        DY(I+3) = DY(I+3) + DA*DX(I+3)
   50 CONTINUE
      RETURN
C
C     Code for equal, positive, non-unit increments.
C
   60 NS = N*INCX
      DO 70 I = 1,NS,INCX
        DY(I) = DA*DX(I) + DY(I)
   70 CONTINUE
      RETURN
      END
      SUBROUTINE XERBLA (SRNAME, INFO)
C
C     ..    Scalar Arguments ..
      INTEGER            INFO
      CHARACTER*6        SRNAME
      CHARACTER*2        XERN1
C
C***FIRST EXECUTABLE STATEMENT  XERBLA
C
      WRITE (XERN1, '(I2)') INFO
      CALL XERMSG ('SLATEC', SRNAME, 'On entry to '//SRNAME//
     $             ' parameter number '//XERN1//' had an illegal value',
     $             INFO,1)
C
      RETURN
C
C     End of XERBLA.
C
      END
      LOGICAL FUNCTION LSAME (CA, CB)
C     .. Scalar Arguments ..
      CHARACTER CA*1, CB*1
C     .. Local Scalars ..
      INTEGER IOFF
      LOGICAL FIRST
C     .. Intrinsic Functions ..
      INTRINSIC ICHAR
C     .. Save statement ..
      SAVE FIRST
C     .. Data statements ..
      DATA FIRST /.TRUE./
C***FIRST EXECUTABLE STATEMENT  LSAME
      IF (FIRST) THEN
        IOFF = ICHAR('a') - ICHAR('A')
      ENDIF
C
      FIRST = .FALSE.
C
C     Test if the characters are equal
C
      LSAME = CA .EQ. CB
C
C     Now test for equivalence
C
      IF (.NOT.LSAME) THEN
        LSAME = ICHAR(CA) - IOFF .EQ. ICHAR(CB)
      ENDIF
C
      RETURN
C
C  The following comments contain code for CDC systems using 6-12 bit
C  representations.
C
C     .. Parameters ..
C     INTEGER                ICIRFX
C     PARAMETER            ( ICIRFX=62 )
C     .. Scalar Arguments ..
C     CHARACTER*1            CB
C     .. Array Arguments ..
C     CHARACTER*1            CA(*)
C     .. Local Scalars ..
C     INTEGER                IVAL
C     .. Intrinsic Functions ..
C     INTRINSIC              ICHAR, CHAR
C     .. Executable Statements ..
C     INTRINSIC              ICHAR, CHAR
C
C     See if the first character in string CA equals string CB.
C
C     LSAME = CA(1) .EQ. CB .AND. CA(1) .NE. CHAR(ICIRFX)
C
C     IF (LSAME) RETURN
C
C     The characters are not identical. Now check them for equivalence.
C     Look for the 'escape' character, circumflex, followed by the
C     letter.
C
C     IVAL = ICHAR(CA(2))
C     IF (IVAL.GE.ICHAR('A') .AND. IVAL.LE.ICHAR('Z')) THEN
C        LSAME = CA(1) .EQ. CHAR(ICIRFX) .AND. CA(2) .EQ. CB
C     ENDIF
C
C     RETURN
C
C     End of LSAME.
C
      END
      FUNCTION INITS (OS, NOS, ETA)
      REAL OS(*)
C***FIRST EXECUTABLE STATEMENT  INITS
      IF (NOS .LT. 1) CALL XERMSG ('SLATEC', 'INITS',
     +   'Number of coefficients is less than 1', 2, 1)
C
      ERR = 0.
      DO 10 II = 1,NOS
        I = NOS + 1 - II
        ERR = ERR + ABS(OS(I))
        IF (ERR.GT.ETA) GO TO 20
   10 CONTINUE
C
   20 IF (I .EQ. NOS) CALL XERMSG ('SLATEC', 'INITS',
     +   'Chebyshev series too short for specified accuracy', 1, 1)
      INITS = I
C
      RETURN
      END
      SUBROUTINE SCOPY (N, SX, INCX, SY, INCY)
      REAL SX(*), SY(*)
C***FIRST EXECUTABLE STATEMENT  SCOPY
      IF (N .LE. 0) RETURN
      IF (INCX .EQ. INCY) IF (INCX-1) 5,20,60
C
C     Code for unequal or nonpositive increments.
C
    5 IX = 1
      IY = 1
      IF (INCX .LT. 0) IX = (-N+1)*INCX + 1
      IF (INCY .LT. 0) IY = (-N+1)*INCY + 1
      DO 10 I = 1,N
        SY(IY) = SX(IX)
        IX = IX + INCX
        IY = IY + INCY
   10 CONTINUE
      RETURN
C
C     Code for both increments equal to 1.
C
C     Clean-up loop so remaining vector length is a multiple of 7.
C
   20 M = MOD(N,7)
      IF (M .EQ. 0) GO TO 40
      DO 30 I = 1,M
        SY(I) = SX(I)
   30 CONTINUE
      IF (N .LT. 7) RETURN
   40 MP1 = M + 1
      DO 50 I = MP1,N,7
        SY(I) = SX(I)
        SY(I+1) = SX(I+1)
        SY(I+2) = SX(I+2)
        SY(I+3) = SX(I+3)
        SY(I+4) = SX(I+4)
        SY(I+5) = SX(I+5)
        SY(I+6) = SX(I+6)
   50 CONTINUE
      RETURN
C
C     Code for equal, positive, non-unit increments.
C
   60 NS = N*INCX
      DO 70 I = 1,NS,INCX
        SY(I) = SX(I)
   70 CONTINUE
      RETURN
      END
      FUNCTION CSEVL (X, CS, N)
      REAL B0, B1, B2, CS(*), ONEPL, TWOX, X
      LOGICAL FIRST
      SAVE FIRST, ONEPL
      DATA FIRST /.TRUE./
C***FIRST EXECUTABLE STATEMENT  CSEVL
      IF (FIRST) ONEPL = 1.0E0 + R1MACH(4)
      FIRST = .FALSE.
      IF (N .LT. 1) CALL XERMSG ('SLATEC', 'CSEVL',
     +   'NUMBER OF TERMS .LE. 0', 2, 2)
      IF (N .GT. 1000) CALL XERMSG ('SLATEC', 'CSEVL',
     +   'NUMBER OF TERMS .GT. 1000', 3, 2)
      IF (ABS(X) .GT. ONEPL) CALL XERMSG ('SLATEC', 'CSEVL',
     +   'X OUTSIDE THE INTERVAL (-1,+1)', 1, 1)
C
      B1 = 0.0E0
      B0 = 0.0E0
      TWOX = 2.0*X
      DO 10 I = 1,N
         B2 = B1
         B1 = B0
         NI = N + 1 - I
         B0 = TWOX*B1 - B2 + CS(NI)
   10 CONTINUE
C
      CSEVL = 0.5E0*(B0-B2)
C
      RETURN
      END
      DOUBLE PRECISION FUNCTION DCSEVL (X, CS, N)
      DOUBLE PRECISION B0, B1, B2, CS(*), ONEPL, TWOX, X, D1MACH
      LOGICAL FIRST
      SAVE FIRST, ONEPL
      DATA FIRST /.TRUE./
C***FIRST EXECUTABLE STATEMENT  DCSEVL
      IF (FIRST) ONEPL = 1.0D0 + D1MACH(4)
      FIRST = .FALSE.
      IF (N .LT. 1) CALL XERMSG ('SLATEC', 'DCSEVL',
     +   'NUMBER OF TERMS .LE. 0', 2, 2)
      IF (N .GT. 1000) CALL XERMSG ('SLATEC', 'DCSEVL',
     +   'NUMBER OF TERMS .GT. 1000', 3, 2)
      IF (ABS(X) .GT. ONEPL) CALL XERMSG ('SLATEC', 'DCSEVL',
     +   'X OUTSIDE THE INTERVAL (-1,+1)', 1, 1)
C
      B1 = 0.0D0
      B0 = 0.0D0
      TWOX = 2.0D0*X
      DO 10 I = 1,N
         B2 = B1
         B1 = B0
         NI = N + 1 - I
         B0 = TWOX*B1 - B2 + CS(NI)
   10 CONTINUE
C
      DCSEVL = 0.5D0*(B0-B2)
C
      RETURN
      END
      FUNCTION INITDS (OS, NOS, ETA)
      DOUBLE PRECISION OS(*)
C***FIRST EXECUTABLE STATEMENT  INITDS
      IF (NOS .LT. 1) CALL XERMSG ('SLATEC', 'INITDS',
     +   'Number of coefficients is less than 1', 2, 1)
C
      ERR = 0.
      DO 10 II = 1,NOS
        I = NOS + 1 - II
        ERR = ERR + ABS(REAL(OS(I)))
        IF (ERR.GT.ETA) GO TO 20
   10 CONTINUE
C
   20 IF (I .EQ. NOS) CALL XERMSG ('SLATEC', 'INITDS',
     +   'Chebyshev series too short for specified accuracy', 1, 1)
      INITDS = I
C
      RETURN
      END
      SUBROUTINE SSCAL (N, SA, SX, INCX)
      REAL SA, SX(*)
      INTEGER I, INCX, IX, M, MP1, N
C***FIRST EXECUTABLE STATEMENT  SSCAL
      IF (N .LE. 0) RETURN
      IF (INCX .EQ. 1) GOTO 20
C
C     Code for increment not equal to 1.
C
      IX = 1
      IF (INCX .LT. 0) IX = (-N+1)*INCX + 1
      DO 10 I = 1,N
        SX(IX) = SA*SX(IX)
        IX = IX + INCX
   10 CONTINUE
      RETURN
C
C     Code for increment equal to 1.
C
C     Clean-up loop so remaining vector length is a multiple of 5.
C
   20 M = MOD(N,5)
      IF (M .EQ. 0) GOTO 40
      DO 30 I = 1,M
        SX(I) = SA*SX(I)
   30 CONTINUE
      IF (N .LT. 5) RETURN
   40 MP1 = M + 1
      DO 50 I = MP1,N,5
        SX(I) = SA*SX(I)
        SX(I+1) = SA*SX(I+1)
        SX(I+2) = SA*SX(I+2)
        SX(I+3) = SA*SX(I+3)
        SX(I+4) = SA*SX(I+4)
   50 CONTINUE
      RETURN
      END
      DOUBLE PRECISION FUNCTION DNRM2 (N, DX, INCX)
      INTEGER NEXT
      DOUBLE PRECISION DX(*), CUTLO, CUTHI, HITEST, SUM, XMAX, ZERO,
     +                 ONE
      SAVE CUTLO, CUTHI, ZERO, ONE
      DATA ZERO, ONE /0.0D0, 1.0D0/
C
      DATA CUTLO, CUTHI /8.232D-11,  1.304D19/
C***FIRST EXECUTABLE STATEMENT  DNRM2
      IF (N .GT. 0) GO TO 10
         DNRM2  = ZERO
         GO TO 300
C
   10 ASSIGN 30 TO NEXT
      SUM = ZERO
      NN = N * INCX
C
C                                                 BEGIN MAIN LOOP
C
      I = 1
   20    GO TO NEXT,(30, 50, 70, 110)
   30 IF (ABS(DX(I)) .GT. CUTLO) GO TO 85
      ASSIGN 50 TO NEXT
      XMAX = ZERO
C
C                        PHASE 1.  SUM IS ZERO
C
   50 IF (DX(I) .EQ. ZERO) GO TO 200
      IF (ABS(DX(I)) .GT. CUTLO) GO TO 85
C
C                                PREPARE FOR PHASE 2.
C
      ASSIGN 70 TO NEXT
      GO TO 105
C
C                                PREPARE FOR PHASE 4.
C
  100 I = J
      ASSIGN 110 TO NEXT
      SUM = (SUM / DX(I)) / DX(I)
  105 XMAX = ABS(DX(I))
      GO TO 115
C
C                   PHASE 2.  SUM IS SMALL.
C                             SCALE TO AVOID DESTRUCTIVE UNDERFLOW.
C
   70 IF (ABS(DX(I)) .GT. CUTLO) GO TO 75
C
C                     COMMON CODE FOR PHASES 2 AND 4.
C                     IN PHASE 4 SUM IS LARGE.  SCALE TO AVOID OVERFLOW.
C
  110 IF (ABS(DX(I)) .LE. XMAX) GO TO 115
         SUM = ONE + SUM * (XMAX / DX(I))**2
         XMAX = ABS(DX(I))
         GO TO 200
C
  115 SUM = SUM + (DX(I)/XMAX)**2
      GO TO 200
C
C                  PREPARE FOR PHASE 3.
C
   75 SUM = (SUM * XMAX) * XMAX
C
C     FOR REAL OR D.P. SET HITEST = CUTHI/N
C     FOR COMPLEX      SET HITEST = CUTHI/(2*N)
C
   85 HITEST = CUTHI / N
C
C                   PHASE 3.  SUM IS MID-RANGE.  NO SCALING.
C
      DO 95 J = I,NN,INCX
      IF (ABS(DX(J)) .GE. HITEST) GO TO 100
   95    SUM = SUM + DX(J)**2
      DNRM2 = SQRT(SUM)
      GO TO 300
C
  200 CONTINUE
      I = I + INCX
      IF (I .LE. NN) GO TO 20
C
C              END OF MAIN LOOP.
C
C              COMPUTE SQUARE ROOT AND ADJUST FOR SCALING.
C
      DNRM2 = XMAX * SQRT(SUM)
  300 CONTINUE
      RETURN
      END
      REAL FUNCTION SNRM2 (N, SX, INCX)
      INTEGER NEXT
      REAL SX(*), CUTLO, CUTHI, HITEST, SUM, XMAX, ZERO, ONE
      SAVE CUTLO, CUTHI, ZERO, ONE
      DATA ZERO, ONE /0.0E0, 1.0E0/
C
      DATA CUTLO, CUTHI /4.441E-16,  1.304E19/
C***FIRST EXECUTABLE STATEMENT  SNRM2
      IF (N .GT. 0) GO TO 10
         SNRM2  = ZERO
         GO TO 300
C
   10 ASSIGN 30 TO NEXT
      SUM = ZERO
      NN = N * INCX
C
C                                                 BEGIN MAIN LOOP
C
      I = 1
   20    GO TO NEXT,(30, 50, 70, 110)
   30 IF (ABS(SX(I)) .GT. CUTLO) GO TO 85
      ASSIGN 50 TO NEXT
      XMAX = ZERO
C
C                        PHASE 1.  SUM IS ZERO
C
   50 IF (SX(I) .EQ. ZERO) GO TO 200
      IF (ABS(SX(I)) .GT. CUTLO) GO TO 85
C
C                                PREPARE FOR PHASE 2.
C
      ASSIGN 70 TO NEXT
      GO TO 105
C
C                                PREPARE FOR PHASE 4.
C
  100 I = J
      ASSIGN 110 TO NEXT
      SUM = (SUM / SX(I)) / SX(I)
  105 XMAX = ABS(SX(I))
      GO TO 115
C
C                   PHASE 2.  SUM IS SMALL.
C                             SCALE TO AVOID DESTRUCTIVE UNDERFLOW.
C
   70 IF (ABS(SX(I)) .GT. CUTLO) GO TO 75
C
C                     COMMON CODE FOR PHASES 2 AND 4.
C                     IN PHASE 4 SUM IS LARGE.  SCALE TO AVOID OVERFLOW.
C
  110 IF (ABS(SX(I)) .LE. XMAX) GO TO 115
         SUM = ONE + SUM * (XMAX / SX(I))**2
         XMAX = ABS(SX(I))
         GO TO 200
C
  115 SUM = SUM + (SX(I)/XMAX)**2
      GO TO 200
C
C                  PREPARE FOR PHASE 3.
C
   75 SUM = (SUM * XMAX) * XMAX
C
C     FOR REAL OR D.P. SET HITEST = CUTHI/N
C     FOR COMPLEX      SET HITEST = CUTHI/(2*N)
C
   85 HITEST = CUTHI / N
C
C                   PHASE 3.  SUM IS MID-RANGE.  NO SCALING.
C
      DO 95 J = I,NN,INCX
      IF (ABS(SX(J)) .GE. HITEST) GO TO 100
   95    SUM = SUM + SX(J)**2
      SNRM2 = SQRT( SUM )
      GO TO 300
C
  200 CONTINUE
      I = I + INCX
      IF (I .LE. NN) GO TO 20
C
C              END OF MAIN LOOP.
C
C              COMPUTE SQUARE ROOT AND ADJUST FOR SCALING.
C
      SNRM2 = XMAX * SQRT(SUM)
  300 CONTINUE
      RETURN
      END
      SUBROUTINE DSCAL (N, DA, DX, INCX)
      DOUBLE PRECISION DA, DX(*)
      INTEGER I, INCX, IX, M, MP1, N
C***FIRST EXECUTABLE STATEMENT  DSCAL
      IF (N .LE. 0) RETURN
      IF (INCX .EQ. 1) GOTO 20
C
C     Code for increment not equal to 1.
C
      IX = 1
      IF (INCX .LT. 0) IX = (-N+1)*INCX + 1
      DO 10 I = 1,N
        DX(IX) = DA*DX(IX)
        IX = IX + INCX
   10 CONTINUE
      RETURN
C
C     Code for increment equal to 1.
C
C     Clean-up loop so remaining vector length is a multiple of 5.
C
   20 M = MOD(N,5)
      IF (M .EQ. 0) GOTO 40
      DO 30 I = 1,M
        DX(I) = DA*DX(I)
   30 CONTINUE
      IF (N .LT. 5) RETURN
   40 MP1 = M + 1
      DO 50 I = MP1,N,5
        DX(I) = DA*DX(I)
        DX(I+1) = DA*DX(I+1)
        DX(I+2) = DA*DX(I+2)
        DX(I+3) = DA*DX(I+3)
        DX(I+4) = DA*DX(I+4)
   50 CONTINUE
      RETURN
      END
      SUBROUTINE CAXPY (N, CA, CX, INCX, CY, INCY)
      COMPLEX CX(*), CY(*), CA
C***FIRST EXECUTABLE STATEMENT  CAXPY
      IF (N.LE.0 .OR. CA.EQ.(0.0E0,0.0E0)) RETURN
      IF (INCX.EQ.INCY .AND. INCX.GT.0) GO TO 20
C
C     Code for unequal or nonpositive increments.
C
      KX = 1
      KY = 1
      IF (INCX .LT. 0) KX = 1+(1-N)*INCX
      IF (INCY .LT. 0) KY = 1+(1-N)*INCY
      DO 10 I = 1,N
        CY(KY) = CY(KY) + CA*CX(KX)
        KX = KX + INCX
        KY = KY + INCY
   10 CONTINUE
      RETURN
C
C     Code for equal, positive, non-unit increments.
C
   20 NS = N*INCX
      DO 30 I = 1,NS,INCX
        CY(I) = CA*CX(I) + CY(I)
   30 CONTINUE
      RETURN
      END
      INTEGER FUNCTION ISAMAX (N, SX, INCX)
      REAL SX(*), SMAX, XMAG
      INTEGER I, INCX, IX, N
C***FIRST EXECUTABLE STATEMENT  ISAMAX
      ISAMAX = 0
      IF (N .LE. 0) RETURN
      ISAMAX = 1
      IF (N .EQ. 1) RETURN
C
      IF (INCX .EQ. 1) GOTO 20
C
C     Code for increment not equal to 1.
C
      IX = 1
      IF (INCX .LT. 0) IX = (-N+1)*INCX + 1
      SMAX = ABS(SX(IX))
      IX = IX + INCX
      DO 10 I = 2,N
        XMAG = ABS(SX(IX))
        IF (XMAG .GT. SMAX) THEN
          ISAMAX = I
          SMAX = XMAG
        ENDIF
        IX = IX + INCX
   10 CONTINUE
      RETURN
C
C     Code for increments equal to 1.
C
   20 SMAX = ABS(SX(1))
      DO 30 I = 2,N
        XMAG = ABS(SX(I))
        IF (XMAG .GT. SMAX) THEN
          ISAMAX = I
          SMAX = XMAG
        ENDIF
   30 CONTINUE
      RETURN
      END
      SUBROUTINE DCOPY (N, DX, INCX, DY, INCY)
      DOUBLE PRECISION DX(*), DY(*)
C***FIRST EXECUTABLE STATEMENT  DCOPY
      IF (N .LE. 0) RETURN
      IF (INCX .EQ. INCY) IF (INCX-1) 5,20,60
C
C     Code for unequal or nonpositive increments.
C
    5 IX = 1
      IY = 1
      IF (INCX .LT. 0) IX = (-N+1)*INCX + 1
      IF (INCY .LT. 0) IY = (-N+1)*INCY + 1
      DO 10 I = 1,N
        DY(IY) = DX(IX)
        IX = IX + INCX
        IY = IY + INCY
   10 CONTINUE
      RETURN
C
C     Code for both increments equal to 1.
C
C     Clean-up loop so remaining vector length is a multiple of 7.
C
   20 M = MOD(N,7)
      IF (M .EQ. 0) GO TO 40
      DO 30 I = 1,M
        DY(I) = DX(I)
   30 CONTINUE
      IF (N .LT. 7) RETURN
   40 MP1 = M + 1
      DO 50 I = MP1,N,7
        DY(I) = DX(I)
        DY(I+1) = DX(I+1)
        DY(I+2) = DX(I+2)
        DY(I+3) = DX(I+3)
        DY(I+4) = DX(I+4)
        DY(I+5) = DX(I+5)
        DY(I+6) = DX(I+6)
   50 CONTINUE
      RETURN
C
C     Code for equal, positive, non-unit increments.
C
   60 NS = N*INCX
      DO 70 I = 1,NS,INCX
        DY(I) = DX(I)
   70 CONTINUE
      RETURN
      END
      INTEGER FUNCTION IDAMAX (N, DX, INCX)
      DOUBLE PRECISION DX(*), DMAX, XMAG
      INTEGER I, INCX, IX, N
C***FIRST EXECUTABLE STATEMENT  IDAMAX
      IDAMAX = 0
      IF (N .LE. 0) RETURN
      IDAMAX = 1
      IF (N .EQ. 1) RETURN
C
      IF (INCX .EQ. 1) GOTO 20
C
C     Code for increments not equal to 1.
C
      IX = 1
      IF (INCX .LT. 0) IX = (-N+1)*INCX + 1
      DMAX = ABS(DX(IX))
      IX = IX + INCX
      DO 10 I = 2,N
        XMAG = ABS(DX(IX))
        IF (XMAG .GT. DMAX) THEN
          IDAMAX = I
          DMAX = XMAG
        ENDIF
        IX = IX + INCX
   10 CONTINUE
      RETURN
C
C     Code for increments equal to 1.
C
   20 DMAX = ABS(DX(1))
      DO 30 I = 2,N
        XMAG = ABS(DX(I))
        IF (XMAG .GT. DMAX) THEN
          IDAMAX = I
          DMAX = XMAG
        ENDIF
   30 CONTINUE
      RETURN
      END
      COMPLEX FUNCTION CDOTC (N, CX, INCX, CY, INCY)
      COMPLEX CX(*),CY(*)
C***FIRST EXECUTABLE STATEMENT  CDOTC
      CDOTC = (0.0,0.0)
      IF (N .LE. 0) RETURN
      IF (INCX.EQ.INCY .AND. INCX.GT.0) GO TO 20
C
C     Code for unequal or nonpositive increments.
C
      KX = 1
      KY = 1
      IF (INCX .LT. 0) KX = 1+(1-N)*INCX
      IF (INCY .LT. 0) KY = 1+(1-N)*INCY
      DO 10 I = 1,N
        CDOTC = CDOTC + CONJG(CX(KX))*CY(KY)
        KX = KX + INCX
        KY = KY + INCY
   10 CONTINUE
      RETURN
C
C     Code for equal, positive increments.
C
   20 NS = N*INCX
      DO 30 I = 1,NS,INCX
      CDOTC = CDOTC + CONJG(CX(I))*CY(I)
   30 CONTINUE
      RETURN
      END
      SUBROUTINE SSWAP (N, SX, INCX, SY, INCY)
      REAL SX(*), SY(*), STEMP1, STEMP2, STEMP3
C***FIRST EXECUTABLE STATEMENT  SSWAP
      IF (N .LE. 0) RETURN
      IF (INCX .EQ. INCY) IF (INCX-1) 5,20,60
C
C     Code for unequal or nonpositive increments.
C
    5 IX = 1
      IY = 1
      IF (INCX .LT. 0) IX = (-N+1)*INCX + 1
      IF (INCY .LT. 0) IY = (-N+1)*INCY + 1
      DO 10 I = 1,N
        STEMP1 = SX(IX)
        SX(IX) = SY(IY)
        SY(IY) = STEMP1
        IX = IX + INCX
        IY = IY + INCY
   10 CONTINUE
      RETURN
C
C     Code for both increments equal to 1.
C
C     Clean-up loop so remaining vector length is a multiple of 3.
C
   20 M = MOD(N,3)
      IF (M .EQ. 0) GO TO 40
      DO 30 I = 1,M
        STEMP1 = SX(I)
        SX(I) = SY(I)
        SY(I) = STEMP1
   30 CONTINUE
      IF (N .LT. 3) RETURN
   40 MP1 = M + 1
      DO 50 I = MP1,N,3
        STEMP1 = SX(I)
        STEMP2 = SX(I+1)
        STEMP3 = SX(I+2)
        SX(I) = SY(I)
        SX(I+1) = SY(I+1)
        SX(I+2) = SY(I+2)
        SY(I) = STEMP1
        SY(I+1) = STEMP2
        SY(I+2) = STEMP3
   50 CONTINUE
      RETURN
C
C     Code for equal, positive, non-unit increments.
C
   60 NS = N*INCX
      DO 70 I = 1,NS,INCX
        STEMP1 = SX(I)
        SX(I) = SY(I)
        SY(I) = STEMP1
   70 CONTINUE
      RETURN
      END
      SUBROUTINE DSWAP (N, DX, INCX, DY, INCY)
      DOUBLE PRECISION DX(*), DY(*), DTEMP1, DTEMP2, DTEMP3
C***FIRST EXECUTABLE STATEMENT  DSWAP
      IF (N .LE. 0) RETURN
      IF (INCX .EQ. INCY) IF (INCX-1) 5,20,60
C
C     Code for unequal or nonpositive increments.
C
    5 IX = 1
      IY = 1
      IF (INCX .LT. 0) IX = (-N+1)*INCX + 1
      IF (INCY .LT. 0) IY = (-N+1)*INCY + 1
      DO 10 I = 1,N
        DTEMP1 = DX(IX)
        DX(IX) = DY(IY)
        DY(IY) = DTEMP1
        IX = IX + INCX
        IY = IY + INCY
   10 CONTINUE
      RETURN
C
C     Code for both increments equal to 1.
C
C     Clean-up loop so remaining vector length is a multiple of 3.
C
   20 M = MOD(N,3)
      IF (M .EQ. 0) GO TO 40
      DO 30 I = 1,M
        DTEMP1 = DX(I)
        DX(I) = DY(I)
        DY(I) = DTEMP1
   30 CONTINUE
      IF (N .LT. 3) RETURN
   40 MP1 = M + 1
      DO 50 I = MP1,N,3
        DTEMP1 = DX(I)
        DTEMP2 = DX(I+1)
        DTEMP3 = DX(I+2)
        DX(I) = DY(I)
        DX(I+1) = DY(I+1)
        DX(I+2) = DY(I+2)
        DY(I) = DTEMP1
        DY(I+1) = DTEMP2
        DY(I+2) = DTEMP3
   50 CONTINUE
      RETURN
C
C     Code for equal, positive, non-unit increments.
C
   60 NS = N*INCX
      DO 70 I = 1,NS,INCX
        DTEMP1 = DX(I)
        DX(I) = DY(I)
        DY(I) = DTEMP1
   70 CONTINUE
      RETURN
      END
      REAL FUNCTION PYTHAG (A, B)
      REAL A,B
C
      REAL P,Q,R,S,T
C***FIRST EXECUTABLE STATEMENT  PYTHAG
      P = MAX(ABS(A),ABS(B))
      Q = MIN(ABS(A),ABS(B))
      IF (Q .EQ. 0.0E0) GO TO 20
   10 CONTINUE
         R = (Q/P)**2
         T = 4.0E0 + R
         IF (T .EQ. 4.0E0) GO TO 20
         S = R/T
         P = P + 2.0E0*P*S
         Q = Q*S
      GO TO 10
   20 PYTHAG = P
      RETURN
      END
      DOUBLE PRECISION FUNCTION ZABS (ZR, ZI)
      DOUBLE PRECISION ZR, ZI, U, V, Q, S
C***FIRST EXECUTABLE STATEMENT  ZABS
      U = ABS(ZR)
      V = ABS(ZI)
      S = U + V
C-----------------------------------------------------------------------
C     S*1.0D0 MAKES AN UNNORMALIZED UNDERFLOW ON CDC MACHINES INTO A
C     TRUE FLOATING ZERO
C-----------------------------------------------------------------------
      S = S*1.0D+0
      IF (S.EQ.0.0D+0) GO TO 20
      IF (U.GT.V) GO TO 10
      Q = U/V
      ZABS = V*SQRT(1.D+0+Q*Q)
      RETURN
   10 Q = V/U
      ZABS = U*SQRT(1.D+0+Q*Q)
      RETURN
   20 ZABS = 0.0D+0
      RETURN
      END
      REAL FUNCTION SASUM (N, SX, INCX)
      REAL SX(*)
      INTEGER I, INCX, IX, M, MP1, N
C***FIRST EXECUTABLE STATEMENT  SASUM
      SASUM = 0.0E0
      IF (N .LE. 0) RETURN
C
      IF (INCX .EQ. 1) GOTO 20
C
C     Code for increment not equal to 1.
C
      IX = 1
      IF (INCX .LT. 0) IX = (-N+1)*INCX + 1
      DO 10 I = 1,N
        SASUM = SASUM + ABS(SX(IX))
        IX = IX + INCX
   10 CONTINUE
      RETURN
C
C     Code for increment equal to 1.
C
C     Clean-up loop so remaining vector length is a multiple of 6.
C
   20 M = MOD(N,6)
      IF (M .EQ. 0) GOTO 40
      DO 30 I = 1,M
        SASUM = SASUM + ABS(SX(I))
   30 CONTINUE
      IF (N .LT. 6) RETURN
   40 MP1 = M + 1
      DO 50 I = MP1,N,6
        SASUM = SASUM + ABS(SX(I)) + ABS(SX(I+1)) + ABS(SX(I+2)) +
     1          ABS(SX(I+3)) + ABS(SX(I+4)) + ABS(SX(I+5))
   50 CONTINUE
      RETURN
      END
      FUNCTION PIMACH (DUM)
C
C***FIRST EXECUTABLE STATEMENT  PIMACH
      PIMACH = 3.14159265358979
      RETURN
      END
c---------------------------------------------
c_________________________________________________________________
c     example program for polyfit; y=1+2x+3x**2
c     real ax(256),y(256),a(10)
c     ax(1)=0.
c     ax(2)=1.
c     ax(3)=2.
c     ax(4)=3.
c     ax(5)=4.
c     ax(6)=5.
c     ax(7)=6.
c     y(1)=1.
c     y(2)=6.
c     y(3)=17.
c     y(4)=34.
c     y(5)=34.
c     y(6)=34.
c     y(7)=34.
c     n=4
c     m=2
c     call polyfit(ax,y,n,a,m)
c     do 10 i=1,7
c        print *,'a',i,a(i),y(i)
c10   continue
c     end
c_________________________________________________________________
c_________________________________________________________________
c     subroutine polyfit(ax,Y,NDATA,A,ma)
cccc  ax     absissus values
cccc  Y      ordinate values
cccc  ndata  number of data pointe
cccc  A      polynomial coefficients
cccc  ma     order of polynomial
c
c     include "array_size.h"
c     parameter (MMAX=50,ns=array_dim)
c     dimension ax(ns),X(ns),Y(ns),SIG(ns),A(10),LISTA(10),
c    +          COVAR(10,10),BETA(MMAX),AFUNC(MMAX)
c---- NEW
c      real*8 ax(ns),X(ns),Y(ns),SIG(ns),A(10),
c    +          COVAR(10,10),BETA(MMAX),AFUNC(MMAX)
c      dimension axin(ns), Yin(ns),LISTA(10)
c     do 5 i=1,NDATA
c        ax(i) = dble(axin(i))
c         y(i)= dble(Yin(i))
c5     continue
c-----END NEW
cccc  SCALE X DATA FROM 0 TO 1
c     ma=ma+1
c     do 9 i=1,ndata
c       x(i)=(ax(i)-ax(1))/(ax(ndata)-ax(1))
c9    continue

c     ncvm=ma
c     mfit=ma
c     do 10 i=1,ndata
c       sig(i) = 1.
c10   continue
c     do 20 i=1,ma
c       lista(i)=i
c20   continue
c     KK=MFIT+1
c     DO 12 J=1,MA
c       IHIT=0
c       DO 11 K=1,MFIT
c         IF (LISTA(K).EQ.J) IHIT=IHIT+1
c11      CONTINUE
c       IF (IHIT.EQ.0) THEN
c          LISTA(KK)=J
c          KK=KK+1
c        ELSE IF (IHIT.GT.1) THEN
c          PRINT*,' IMPROPER SET IN LISTA '
c        ENDIF
c12    CONTINUE
c      IF (KK.NE.(MA+1)) PRINT *,'IMPROPER SET IN LISTA            '
c      DO 14 J=1,MFIT
c        DO 13 K=1,MFIT
c          COVAR(J,K)=0.
c13      CONTINUE
c        BETA(J)=0.
c14    CONTINUE
c      DO 18 I=1,NDATA
c        CALL fpoly(X(I),AFUNC,MA)
c        YM=Y(I)
c        IF(MFIT.LT.MA) THEN
c          DO 15 J=MFIT+1,MA
c            YM=YM-A(LISTA(J))*AFUNC(LISTA(J))
c15        CONTINUE
c        ENDIF
c        SIG2I=1./SIG(I)**2
c        DO 17 J=1,MFIT
c          WT=AFUNC(LISTA(J))*SIG2I
c          DO 16 K=1,J
c            COVAR(J,K)=COVAR(J,K)+WT*AFUNC(LISTA(K))
c16        CONTINUE
c          BETA(J)=BETA(J)+YM*WT
c17      CONTINUE
c18    CONTINUE
c      IF (MFIT.GT.1) THEN
c        DO 21 J=2,MFIT
c          DO 19 K=1,J-1
c            COVAR(K,J)=COVAR(J,K)
c19        CONTINUE
c21      CONTINUE
c      ENDIF
c      CALL GAUSSJ(COVAR,MFIT,NCVM,BETA,1,1)
c      DO 22 J=1,MFIT
c        A(LISTA(J))=BETA(J)
c22    CONTINUE
c      CHISQ=0.
c      DO 24 I=1,NDATA
c        CALL fpoly(X(I),AFUNC,MA)
c        SUM=0.
c        DO 23 J=1,MA
c          SUM=SUM+A(J)*AFUNC(J)
c23      CONTINUE
c        CHISQ=CHISQ+((Y(I)-SUM)/SIG(I))**2
c24    CONTINUE
c      ma = ma-1 !changed 22 April by J. Hubbert. 
cc     do 100 i=1,NDATA
cc        axin(i) =real(ax(i))
cc         yin(i)= real(Y(i))
cc100      continue
c
c      RETURN
c      END
c_______________________________________________________________________
c_______________________________________________________________________
c      subroutine gaussj(A,N,NP,B,M,MP)
c
c      PARAMETER (NMAX=50)
c     DIMENSION A(NP,NP),B(NP,MP),IPIV(NMAX),INDXR(NMAX),INDXC(NMAX)
c      DIMENSION A(10,10),B(10,MP),IPIV(NMAX),INDXR(NMAX),INDXC(NMAX)
c----NEW
c      real*8 A(10,10),B(10,MP)
c      dimension IPIV(NMAX),INDXR(NMAX),INDXC(NMAX)
c----END NEW
c      DO 11 J=1,N
c        IPIV(J)=0
c11    CONTINUE
c      DO 22 I=1,N
c        BIG=0.
c        DO 13 J=1,N
c          IF(IPIV(J).NE.1)THEN
c            DO 12 K=1,N
c              IF (IPIV(K).EQ.0) THEN
c                IF (ABS(A(J,K)).GE.BIG)THEN
c                  BIG=ABS(A(J,K))
c                  IROW=J
c                  ICOL=K
c                ENDIF
c              ELSE IF (IPIV(K).GT.1) THEN
c                PRINT*,'SINGULAR MATRIX '
c              ENDIF
c12          CONTINUE
c          ENDIF
c13      CONTINUE
c        IPIV(ICOL)=IPIV(ICOL)+1
c        IF (IROW.NE.ICOL) THEN
c          DO 14 L=1,N
c            DUM=A(IROW,L)
c            A(IROW,L)=A(ICOL,L)
c            A(ICOL,L)=DUM
c14        CONTINUE
c          DO 15 L=1,M
c            DUM=B(IROW,L)
c            B(IROW,L)=B(ICOL,L)
c            B(ICOL,L)=DUM
c15        CONTINUE
c        ENDIF
c        INDXR(I)=IROW
c        INDXC(I)=ICOL
c        IF (A(ICOL,ICOL).EQ.0.) PRINT*, 'SINGULAR MATRIX '
c        PIVINV=1./A(ICOL,ICOL)
c        A(ICOL,ICOL)=1.
c        DO 16 L=1,N
c          A(ICOL,L)=A(ICOL,L)*PIVINV
c16      CONTINUE
c        DO 17 L=1,M
c          B(ICOL,L)=B(ICOL,L)*PIVINV
c17      CONTINUE
c        DO 21 LL=1,N
c          IF(LL.NE.ICOL)THEN
c            DUM=A(LL,ICOL)
c            A(LL,ICOL)=0.
c            DO 18 L=1,N
c              A(LL,L)=A(LL,L)-A(ICOL,L)*DUM
c18          CONTINUE
c            DO 19 L=1,M
c              B(LL,L)=B(LL,L)-B(ICOL,L)*DUM
c19          CONTINUE
c          ENDIF
c21      CONTINUE
c22    CONTINUE
c      DO 24 L=N,1,-1
c        IF(INDXR(L).NE.INDXC(L))THEN
c          DO 23 K=1,N
c            DUM=A(K,INDXR(L))
c            A(K,INDXR(L))=A(K,INDXC(L))
c            A(K,INDXC(L))=DUM
c23        CONTINUE
c        ENDIF
c24    CONTINUE
c      RETURN
c      END
cc____________________________
c____________________________________________________________________
c
c      subroutine fpoly(X,P,NP)
c
c       dimension P(NP)
cc---NEW
cc     real*8, dimension(NP) ::  P
cc     real*8 x
cc----END NEW
c      P(1)=1.
c      if (np.eq.1)  return
c      DO 11 J=2,NP
c        P(J)=P(J-1)*X
c11    CONTINUE
c
c      RETURN
c      END
c______________________________________________________________________
c     example program for polyfit; y=1+2x+3x**2
c     real*8 ax(512),y(512),a(512),v(512),dd,c(0:512),e1
c     ax(1)=0.
c     ax(2)=1.
c     ax(3)=2.
c     ax(4)=3.
c     ax(5)=4.
c     ax(6)=5.
c     ax(7)=6.
c     y(1)=1.
c     y(2)=6.00
c     y(3)=17.
c     y(4)=34.
c     y(5)=57.
c     y(6)=86.
c     y(7)=121.
c     n=7
c     m=2
c     e1=0.0
c     call polyfit(ax,y,n,a,m)
c     call LS_POLY(m,e1,n,l,ax,y,c,dd,v)
c     do 10 i=1,7
c        print *,'a',i,c(i-1),y(i),v(i),dd
c10   continue
c     end
c_________________________________________________________________
c NEW DOUBLE PRESCISION ROUNTINE FOR PLOYNOMIAL FITTING
!*****************************************************************
!*         LEAST SQUARES POLYNOMIAL FITTING PROCEDURE            *
!* ------------------------------------------------------------- *
!* This program least squares fits a polynomial to input data.   *
!* forsythe orthogonal polynomials are used in the fitting.      *
!* The number of data points is n.                               *
!* The data is input to the subroutine in x[i], y[i] pairs.      *
!* The coefficients are returned in c[i],                        *
!* the smoothed data is returned in v[i],                        *
!* the order of the fit is specified by m.                       *
!* The standard deviation of the fit is returned in d.           *
!* There are two options available by use of the parameter e:    *
!*  1. if e = 0, the fit is to order m,                          *
!*  2. if e > 0, the order of fit increases towards m, but will  *
!*     stop if the relative standard deviation does not decrease *
!*     by more than e between successive fits.                   *
!* The order of the fit then obtained is l.                      *
!*****************************************************************
       Subroutine LS_POLY(m,e1,n,l,x,y,c,dd,v)
         integer SIZE
         parameter(SIZE=1025,ipoly=39)
  !Labels: 10,15,20,30,50
         real*8 x(SIZE),y(SIZE),v(SIZE),a(SIZE),b(SIZE)
         real*8 c(0:ipoly),d(SIZE),c2(SIZE),e(SIZE),f(SIZE)
           integer i,l,l2,m,n,n1
        real*8 a1,a2,b1,b2,c1,dd,d1,e1,f1,f2,v1,v2,w
c        print *,'IN LS_POLY******** ORDER ',m
        n1 = m + 1; l=0
        v1 = 1.d7
  ! Initialize the arrays
        do i=1, n1
          a(i) = 0.d0; b(i) = 0.d0; f(i) = 0.d0
        end do
        do i=1, n
          v(i) = 0.d0; d(i) = 0.d0
        end do
        d1 = dsqrt(dfloat(n)); w = d1;
        do i=1, n
          e(i) = 1.d0 / w
        end do
        f1 = d1; a1 = 0.d0
        do i=1, n
          a1 = a1 + x(i) * e(i) * e(i)
        end do
        c1 = 0.d0
        do i=1, n
          c1 = c1 + y(i) * e(i)
        end do
        b(1) = 1.d0 / f1; f(1) = b(1) * c1
        do i=1, n
          v(i) = v(i) + e(i) * c1
        end do
        m = 1
! Save latest results
10      do i=1, l
          c2(i) = c(i)
        end do
        l2 = l; v2 = v1; f2 = f1; a2 = a1; f1 = 0.d0
        do i=1, n
          b1 = e(i)
          e(i) = (x(i) - a2) * e(i) - f2 * d(i)
          d(i) = b1
          f1 = f1 + e(i) * e(i)
        end do
        f1 = dsqrt(f1)
        do i=1, n
          e(i) = e(i) / f1
        end do
        a1 = 0.d0
        do i=1, n
          a1 = a1 + x(i) * e(i) * e(i)
        end do
        c1 = 0.d0
        do i=1, n
          c1 = c1 + e(i) * y(i)
        end do
        m = m + 1; i = 0
15       l = m - i; b2 = b(l); d1 = 0.d0
        if (l > 1)  d1 = b(l - 1)
        d1 = d1 - a2 * b(l) - f2 * a(l)
        b(l) = d1 / f1; a(l) = b2; i = i + 1
        if (i.ne.m) goto 15
        do i=1, n
          v(i) = v(i) + e(i) * c1
        end do
        do i=1, n1
          f(i) = f(i) + b(i) * c1
          c(i) = f(i)
        end do
        vv = 0.d0
        do i=1, n
          vv = vv + (v(i) - y(i)) * (v(i) - y(i))
        end do
  !Note the division is by the number of degrees of freedom
        vv = dsqrt(vv / dfloat(n - l - 1)); l = m
        if (e1.eq.0.d0) goto 20
  !Test for minimal improvement
        if (dabs(v1 - vv) / vv < e1) goto 50
  !if error is larger, quit
        if (e1 * vv > e1 * v1) goto 50
        v1 = vv
20       if (m.eq.n1) goto 30
        goto 10
!Shift the c[i] down, so c(0) is the constant term
30       do i=1, l
          c(i - 1) = c(i)
        end do
        c(l) = 0.d0
  ! l is the order of the polynomial fitted
        l = l - 1; dd = vv
c
C return m to original input order
c
        m=m-1
c
        return
! Aborted sequence, recover last values
50       l = l2; vv = v2
        do i=1, l
          c(i) = c2(i)
        end do
        goto 30
      end

! End of file lsqrpoly.f90
