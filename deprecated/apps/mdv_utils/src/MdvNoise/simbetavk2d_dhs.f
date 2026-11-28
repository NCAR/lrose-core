cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c simulate 2D backscatter field with Von Karman spectrum and outer scale
c with mean value of unity and generate output for plots
c
c Wed May 11 14:27:41 MDT 2005
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
      program simbetavk2d_dhs

      integer nwv,nwh		!Maximum dimension for wsave
      integer npv,nph,np	!Maximum dimensions for 2D simulation
      parameter(npv=4096,nph=4096,np=8192)
      parameter(nwv=4*npv+15,nwh=4*nph+15)
      real spcwt(npv,nph)	!Spectral weights
      real beta(npv,nph)	!beta array/<beta>
      real wsavev(nwv)		!Working space for FFT in vertical direction
      real wsaveh(nwh)		!Working space for FFT in horizontal direction
      complex cwork(np)		!Working space for 2D complex FFT
      complex x(npv,nph)	!Complex array for 2D simulation
      external GVK		!Normalized covariance for Von Karman spectrum

      character*20 p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

c  **  read parameter list from command line
       if( iargc() .eq. 7 ) then
                call getarg( 1, p1 )
                call getarg( 2, p2 )
                call getarg( 3, p3 )
                call getarg( 4, p4 )
                call getarg( 5, p5 )
                call getarg( 6, p6 )
                call getarg( 7, p7 )
c               call getarg( 8, p8 )
c               call getarg( 9, p9 )
       else
c ______________________________________________________________________
      print   *,'USAGE: simbetavk2d_dhs msimv msimh deltav deltah si L0
     +idum > output'
      print*,' (p1)msimv      - (I)simulation length in vertical'
      print*,' (p2)msimh      - (I)simulation length in horizontal'
      print*,' (p3)deltav     - (R)grid spacing in vertical (m)'
      print*,' (p4)deltah     - (R)grid spacing in horizontal(m)'
      print*,' (p5)si         - (R)sigma_beta/<beta>'
      print*,' (p6)L0         - (R)outer scale (m)'
      print*,' (p7)idum       - (I)random Seed (0 or large neg integer)'
      print*,' '
      stop
 
       endif
 
C   CONVERT PARAMETERS
      read(p1,"(i9)") msimv
      read(p2,"(i9)") msimh
      read(p3,"(g14.6)") deltav
      read(p4,"(g14.6)") deltah
      read(p5,"(g14.6)") si
      read(p6,"(g14.6)") xl0
      read(p7,"(i9)") idum

      if ((msimv .gt. npv) .or. (msimh .gt. nph)) then
      print *, 'Array dimensions exceeded.'
      end if

c     idum=0			!Random number seed
c     msimv=64			!Simulation dimension in vertical
c     msimh=128			!Simulation dimension in horizontal
c     deltav=10.0		!Vertical grid point spacing
c     deltah=100.0		!Horizontal grid point spacing
c     epsilon=0.001		!Energy dissipation rate
c     xl0=2000.0		!Outer scale of turbulence

c Initialize Parameters and calculate needed arrays
      msv=msimv
      msh=msimh
      ckh=deltah/xl0
      ckv=deltav/xl0
      scv=ckv*ckv
      sch=ckh*ckh
      si2=si*si			!VAR[beta]/<beta>^2
      var=alogn(si2)		!VAR[ln(beta)]
      scon=exp(-0.5*var)

c     write(*,*)' varv1d=',varv1d,' scv=',scv,' sch=',sch

      call cffti(msv,wsavev)	!Initialize FFT of length msv
      call cffti(msh,wsaveh)	!Initialize FFT of length msv

c Spectral weights
      call spcwgt_2dg(x,spcwt,cwork,npv,nph,msv,msh,wsavev,wsaveh,
     +si2,scv,sch,GVK)
c     write(*,*)(spcwt(i,1),i=1,msv)

      write(*,*)'# Output from program simbetavk2d_dhs.f'
      write(*,'(a,i6,a,i6,a,f6.1,a,f6.1,a)')
     +'# msimv=',msimv,' msimh=',msimh,' deltav=',deltav,
     +' m  deltah=',deltah,' m' 
      write(*,'(a,f5.1,a,f6.1,a)')'# sigma/<beta>=',si,' L0=',xl0,' m'

      call sim2dg(x,spcwt,npv,nph,msv,msh,cwork,wsavev,wsaveh,idum)	!Simulation

      do k=1,msimh
      do l=1,msimv
      beta(l,k)=scon*exp(real(x(l,k)))
      write(*,*) beta(l,k)
      enddo
      enddo

 111  format(i6,f12.0,f12.3,200f10.4)

      END

      function GVK(X)
C Normalized Atmospheric Covariance Function for Von Karman Scalar Spectrum
c C(r)= variance * G(r/L0)
      PARAMETER(CC=0.5925485,ONETH=1./3.)
      IF(X.GT.84.0) then
      GVK=0.0
      return
      endif
      IF(X.EQ.0.0) then
      GVK=1.0
      return
      else
      call RKBESL(X,ONETH,1,1,Y,ncalc)
c     write(*,*)' X  RKBESL',x,Y
      GVK=CC*X**ONETH*Y
      endif
      RETURN
      END
 
      function HVK(X)
C Normalized Atmospheric Scalar Structure Function for Von Karman Spectrum
c D(r)= 2 * variance * H(r/L0)
      PARAMETER(ONETH=1./3.,TWOTH=2./3.,C1=0.95527483,C2=.375,
     +C3=.179114,c4=.028125,c5=.009595394462,cc=.5925485)
      IF(X.GT.84.0) then
      HVK=1.0
      return
      endif
      IF(X.LT.0.1) then         !Taylor series expansion
      x2=x*x
      xpl=x**TWOTH
      HVK=xpl*(c1+x2*(c3+x2*c5))-x2*(c2+x2*c4)
      return
      else
      call RKBESL(X,ONETH,1,1,Y,ncalc)
      HVK=1.0-CC*X**ONETH*Y
      endif
      RETURN
      END

      subroutine spcwgt_2dg(a,p,cw,npv,nph,nv,nh,wsavev,wsaveh,
     +si2,scv,sch,covnorm)
c calculate spectral weights for simulation of log-normal beta field
c with a Von Karman spatial covariance B(x)=sigbeta2*G(x/L_0) and 
c sigbeta2 is the variance of beta
c simulated beta(x) = <beta> exp[-sig2/2+y(x)]
c where y(x) is N(0,sig2) and sig2=ln[1+si2] is the variance of y
c B_y(x)=ln[1+B(x)/<beta>^2]=ln[1+si2*G(x)]
c B_y(x)=sig2*ln[1+si2*G(x)]/sig2
c a(npv,nph) - 2D complex work array
c p(npv,nph) - 2D real array for spectral weights
c cw         - 1D complex work array
c nv         - dimensions of ouput array in vertical direction
c nh         - dimensions of output array in horitzontal direction
c wsavev     - array for fft of length nv
c wsaveh     - array for fft of length nh
c si2        - sigma_beta^2/<beta>^2
c scv        - (delta v/L_0)^2
c sch        - (delta h/L_0)^2
c delta v    - grid spacing in vertical
c delta h    - grid spacing in horizontal
c L_0        - outer scale
c covnorm    - function for normalized covariance 
c              C(k,l) = var covnorm(sqrt[scv*(k-1)**2+sch*(l-1)**2])
c var        - variance = ln(1+si2)
c 

      complex a(npv,nph), cw(*)
      dimension p(npv,nph), wsavev(*), wsaveh(*)
      external covnorm

c test array bounds
      if(nv.gt.npv) write(*,*)' nv > npv in simwind2dg'
      if(nh.gt.nph) write(*,*)' nh > nph in simwind2dg'

      var=alogn(si2)

      call covgeng(a,npv,nph,nv,nh,si2,scv,sch,covnorm)

c     do j=1,nh
c     write(*,111) (real(a(k,j)),k=1,nv)
c     do k=1,nv
c     if(.not.(real(a(k,j)).gt.-1.0e30.and.real(a(k,j)).lt.1.0e30))
c    +write(*,*)k,j,a(k,j)
c     enddo
c     enddo

 111  format(8f10.4)

      call twodfftg(nv,nh,npv,nph,a,wsavev,wsaveh,cw)

c     do j=1,nh
c     write(*,111) (real(a(k,j)),k=1,nv)
c     enddo

      con=sqrt(var/(float(nv)*float(nh)))

c calculate spectral weights
      sum=0.0
      do k=1,nv
      do j=1,nh
      ss=real(a(k,j))
      if(ss.lt.0.0) write(*,*) 'L0 too large',k,j,ss
      if(ss.lt.0.0) ss=0.0
      p(k,j)=sqrt(abs(ss))*con
      sum=sum+p(k,j)*p(k,j)
      enddo
      enddo

      return
      end

      subroutine covgeng(x,npv,nph,nv,nh,si2,scv,sch,func)
c Generate covariance function C(k,l) for log-normal simulation
c C(k,l) = ln[1+si2* func(sqrt[scv*(k-1)**2+sch*(l-1)**2])]/ln[1+si2]
c x(npv,nph)- storage space general complex 2D array
c nv        - dimensions of ouput array in vertical direction
c nh        - dimensions of output array in horitzontal direction
c si2       - sigma_beta^2/<beta>^2
c var       - variance = ln(1+si2)
c scv       - (delta v/L_0)^2
c sch       - (delta h/L_0)^2
c delta v   - grid spacing in vertical
c delta h   - grid spacing in horizontal
c L_0       - outer scale
      external func
      complex x(npv,nph)

c test array bounds
      if(nv.gt.npv) write(*,*)' nv > npv in simwind2dg'
      if(nh.gt.nph) write(*,*)' nh > nph in simwind2dg'

      var=alogn(si2)
c Calculate covariance function
      nmv=nv/2+1
      nmh=nh/2+1
      do 5 k=1,nv
      do 5 j=1,nh
      r=sqrt(scv*float(im(k,nv,nmv)**2)+sch*float(im(j,nh,nmh)**2))
      x(k,j)=alogn(si2*func(r))/var
  5   continue
      return
      end
 
      function im(k,n,nm)
c convert FFT index to negative domain 
c if k<n/2+1 im=k-1 else im=n-k+1
c nm=n/2+1
      if(k.le.nm) then
      im=k-1
      else
      im=n-k+1
      endif
      return
      end

      function alogn(x)
c ln(1+x)
      if(x.lt.0.005) then
      alogn=x*(1.0+x*(-0.5+x*0.33333333333))
      else
      alogn=alog(1.0+x)
      endif
      return
      end

      subroutine sim2dg(x,spcwt,npv,nph,nv,nh,cwork,wsavev,wsaveh,idum)
c simulate 2D scalar field
c x(npv,nph)- storage space general complex 2D array
c nv        - dimensions of ouput array in vertical direction
c nh        - dimensions of output array in horitzontal direction
c x         - two dimensional complex array for wind field
c           - real and imaginary part of x are statistically independent
c             realizations of the wind field
c spcwt     - real array of spectral weights for simulation from spcwgt_2dg
c nv        - dimensions of ouput array in vertical direction
c nh        - dimensions of output array in horitzontal direction
c cwork     - complex working space of dimensions at least max(npv,nph)
c wsavev    - array for fft of length nv
c wsaveh    - array for fft of length nh
c idum  - random number seed (0 or large negative number)
      complex x(npv,nph), cwork(*)
      dimension spcwt(npv,nph), wsavev(*), wsaveh(*)

c test array bounds
      if(nv.gt.npv) write(*,*)' nv > npv in simwind2dg'
      if(nh.gt.nph) write(*,*)' nh > nph in simwind2dg'

c Generate zero-mean complex Gaussian spectral weights

      do 10 k=1,nv
      do 10 l=1,nh
      CALL GASCMP(IDUM,V1,V2)		!Gaussian random variables
      x(k,l)=CMPLX(V1,V2)*spcwt(k,l)	!Spectral weights
 10   continue

c produce simulation with general 2D fft
      call twodfftbg(nv,nh,npv,nph,x,wsavev,wsaveh,cwork)

      return
      end

c     include 'routines/rndo.f'
      FUNCTION rndo(IDUM)
c uniform (0,1) random number generator rand3 modified from Numerical Recipes
c     DIMENSION MA(55)
c      the lines below can be use to convert the routine to all floating
c      point for machines w/ 16 bit or less integer.
c      IMPLICIT REAL*4(M)
c      PARAMETER (MBIG=4000000.,MSEED=1618033.,MZ=0.,FAC=1./MBIG)
      common /RR/INEXTP,INEXT,MJ,MK,MA(55)      !required to preserve memory locations
      PARAMETER (MBIG=1000000000,MSEED=1661803398,MZ=0,FAC=1./MBIG)
      DATA IFF /0/

c initialize shuffle array MA

      IF (IDUM.LT.0.OR.IFF.EQ.0) THEN
        IFF=1
        MJ=MSEED-IABS(IDUM)
        MJ=MOD(MJ,MBIG)
        MA(55)=MJ
        MK=1
        DO 11 I=1,54
           II=MOD(21*I,55)
           MA(II)=MK
           MK=MJ-MK
           IF(MK.LT.MZ) MK=MK+MBIG
           MJ=MA(II)
  11    CONTINUE
        DO 13 K=1,4
           DO 12 I=1,55
              MA(I)=MA(I)-MA(1+MOD(I+30,55))
              IF(MA(I).LT.MZ) MA(I)=MA(I)+MBIG
  12    CONTINUE
  13    CONTINUE
        INEXT=0
        INEXTP=31
        IDUM=1
      ENDIF

c calculate random number

      INEXT=INEXT+1
      IF(INEXT.EQ.56) INEXT=1
      INEXTP=INEXTP+1
      IF(INEXTP.EQ.56) INEXTP=1
      MJ=MA(INEXT)-MA(INEXTP)
      IF(MJ.LT.MZ) MJ=MJ+MBIG
      MA(INEXT)=MJ
      rndo=MJ*FAC
      RETURN
      END
c     include 'routines/gascmp.f'
      subroutine gascmp(IDUM,V1,V2)
c generate zero mean Gaussian random variables V1, V2 
c with unit variance using Box-Muller transformation
 1    V1=2.0*RNDO(IDUM)-1.0
      V2=2.0*RNDO(IDUM)-1.0
      R=V1*V1+V2*V2
      IF(R.GE.1.0.or.R.EQ.0.0) GOTO 1
      FAC=SQRT(-2.0*ALOG(R)/R)
      V1=V1*FAC
      V2=V2*FAC
      RETURN
      END

c     include 'routines/tdfftc.f'
      subroutine twodfft(n,np,x,wsave,ww)
c 2D complex forward FFT
c
c x(j)=the sum from k=1,...,n of  x(k)*exp(-I*(j-1)*(k-1)*2*pi/n)
c
c n - dimensions of FFT
c np- storage space for array
c x - complex data array (n,n)
c wsave(4*n+15)- storage space for output of  CFFTI(N,WSAVE)
c ww(n)- storage space
      complex x(np,np), ww(np)
      dimension wsave(*)
c fft of columns
      iw1=n+n+1
      iw2=iw1+n+n
      do 10 k=1,n
      call cfftf1(n,x(1,k),wsave,wsave(iw1),wsave(iw2))
 10   continue
c fft of rows
      do 20 k=1,n
      do 30 i=1,n
      ww(i)=x(k,i)
  30  continue
      call cfftf1(n,ww,wsave,wsave(iw1),wsave(iw2))
      do 40 j=1,n
      x(k,j)=ww(j)
  40  continue
  20  continue
      return
      end
    
      subroutine twodfftb(n,np,x,wsave,ww)
c 2D complex backward FFT
c
c x(j)=the sum from k=1,...,n of  x(k)*exp(I*(j-1)*(k-1)*2*pi/n)
c
c n - dimensions of FFT
c np- storage space for array
c x - complex data array (n,n)
c wsave(4*n+15)- storage space for output of  CFFTI(N,WSAVE)
c ww(n)- storage space
      complex x(np,np), ww(np)
      dimension wsave(*)
      iw1=n+n+1
      iw2=iw1+n+n
c fft of columns
      do 10 k=1,n
      call cfftb1(n,x(1,k),wsave,wsave(iw1),wsave(iw2))
 10   continue
c fft of rows
      do 20 k=1,n
      do 30 i=1,n
      ww(i)=x(k,i)
  30  continue
      call cfftb1(n,ww,wsave,wsave(iw1),wsave(iw2))
      do 40 j=1,n
      x(k,j)=ww(j)
  40  continue
  20  continue
      return
      end
    
C     SUBROUTINE CFFTF(N,C,WSAVE)                                               
C                                                                               
C     SUBROUTINE CFFTF COMPUTES THE FORWARD COMPLEX DISCRETE FOURIER            
C     TRANSFORM (THE FOURIER ANALYSIS). EQUIVALENTLY , CFFTF COMPUTES           
C     THE FOURIER COEFFICIENTS OF A COMPLEX PERIODIC SEQUENCE.                  
C     THE TRANSFORM IS DEFINED BELOW AT OUTPUT PARAMETER C.                     
C                                                                               
C     THE TRANSFORM IS NOT NORMALIZED. TO OBTAIN A NORMALIZED TRANSFORM         
C     THE OUTPUT MUST BE DIVIDED BY N. OTHERWISE A CALL OF CFFTF                
C     FOLLOWED BY A CALL OF CFFTB WILL MULTIPLY THE SEQUENCE BY N.              
C                                                                               
C     THE ARRAY WSAVE WHICH IS USED BY SUBROUTINE CFFTF MUST BE                 
C     INITIALIZED BY CALLING SUBROUTINE CFFTI(N,WSAVE).                         
C                                                                               
C     INPUT PARAMETERS                                                          
C                                                                               
C                                                                               
C     N      THE LENGTH OF THE COMPLEX SEQUENCE C. THE METHOD IS                
C            MORE EFFICIENT WHEN N IS THE PRODUCT OF SMALL PRIMES. N            
C                                                                               
C     C      A COMPLEX ARRAY OF LENGTH N WHICH CONTAINS THE SEQUENCE            
C                                                                               
C     WSAVE   A REAL WORK ARRAY WHICH MUST BE DIMENSIONED AT LEAST 4N+15        
C             IN THE PROGRAM THAT CALLS CFFTF. THE WSAVE ARRAY MUST BE          
C             INITIALIZED BY CALLING SUBROUTINE CFFTI(N,WSAVE) AND A            
C             DIFFERENT WSAVE ARRAY MUST BE USED FOR EACH DIFFERENT             
C             VALUE OF N. THIS INITIALIZATION DOES NOT HAVE TO BE               
C             REPEATED SO LONG AS N REMAINS UNCHANGED THUS SUBSEQUENT           
C             TRANSFORMS CAN BE OBTAINED FASTER THAN THE FIRST.                 
C             THE SAME WSAVE ARRAY CAN BE USED BY CFFTF AND CFFTB.              
C                                                                               
C     OUTPUT PARAMETERS                                                         
C                                                                               
C     C      FOR J=1,...,N                                                      
C                                                                               
C                C(J)=THE SUM FROM K=1,...,N OF                                 
C                                                                               
C                      C(K)*EXP(-I*(J-1)*(K-1)*2*PI/N)                          
C                                                                               
C                            WHERE I=SQRT(-1)                                   
C                                                                               
C     WSAVE   CONTAINS INITIALIZATION CALCULATIONS WHICH MUST NOT BE            
C             DESTROYED BETWEEN CALLS OF SUBROUTINE CFFTF OR CFFTB              
C                                                                               
      SUBROUTINE CFFTF (N,C,WSAVE)                                              
      DIMENSION       C(*)       ,WSAVE(*)                                      
C                                                                               
      IF (N .EQ. 1) RETURN                                                      
      IW1 = N+N+1                                                               
      IW2 = IW1+N+N                                                             
      CALL CFFTF1 (N,C,WSAVE,WSAVE(IW1),WSAVE(IW2))                             
      RETURN                                                                    
      END                                                                       
 
      SUBROUTINE CFFTF1 (N,C,CH,WA,IFAC)                                        
      DIMENSION       CH(*)      ,C(*)       ,WA(*)      ,IFAC(*)               
      NF = IFAC(2)                                                              
      NA = 0                                                                    
      L1 = 1                                                                    
      IW = 1                                                                    
      DO 116 K1=1,NF                                                            
         IP = IFAC(K1+2)                                                        
         L2 = IP*L1                                                             
         IDO = N/L2                                                             
         IDOT = IDO+IDO                                                         
         IDL1 = IDOT*L1                                                         
         IF (IP .NE. 4) GO TO 103                                               
         IX2 = IW+IDOT                                                          
         IX3 = IX2+IDOT                                                         
         IF (NA .NE. 0) GO TO 101                                               
         CALL PASSF4 (IDOT,L1,C,CH,WA(IW),WA(IX2),WA(IX3))                      
         GO TO 102                                                              
  101    CALL PASSF4 (IDOT,L1,CH,C,WA(IW),WA(IX2),WA(IX3))                      
  102    NA = 1-NA                                                              
         GO TO 115                                                              
  103    IF (IP .NE. 2) GO TO 106                                               
         IF (NA .NE. 0) GO TO 104                                               
         CALL PASSF2 (IDOT,L1,C,CH,WA(IW))                                      
         GO TO 105                                                              
  104    CALL PASSF2 (IDOT,L1,CH,C,WA(IW))                                      
  105    NA = 1-NA                                                              
         GO TO 115                                                              
  106    IF (IP .NE. 3) GO TO 109                                               
         IX2 = IW+IDOT                                                          
         IF (NA .NE. 0) GO TO 107                                               
         CALL PASSF3 (IDOT,L1,C,CH,WA(IW),WA(IX2))                              
         GO TO 108                                                              
  107    CALL PASSF3 (IDOT,L1,CH,C,WA(IW),WA(IX2))                              
  108    NA = 1-NA                                                              
         GO TO 115                                                              
  109    IF (IP .NE. 5) GO TO 112                                               
         IX2 = IW+IDOT                                                          
         IX3 = IX2+IDOT                                                         
         IX4 = IX3+IDOT                                                         
         IF (NA .NE. 0) GO TO 110                                               
         CALL PASSF5 (IDOT,L1,C,CH,WA(IW),WA(IX2),WA(IX3),WA(IX4))              
         GO TO 111                                                              
  110    CALL PASSF5 (IDOT,L1,CH,C,WA(IW),WA(IX2),WA(IX3),WA(IX4))              
  111    NA = 1-NA                                                              
         GO TO 115                                                              
  112    IF (NA .NE. 0) GO TO 113                                               
         CALL PASSF (NAC,IDOT,IP,L1,IDL1,C,C,C,CH,CH,WA(IW))                    
         GO TO 114                                                              
  113    CALL PASSF (NAC,IDOT,IP,L1,IDL1,CH,CH,CH,C,C,WA(IW))                   
  114    IF (NAC .NE. 0) NA = 1-NA                                              
  115    L1 = L2                                                                
         IW = IW+(IP-1)*IDOT                                                    
  116 CONTINUE                                                                  
      IF (NA .EQ. 0) RETURN                                                     
      N2 = N+N                                                                  
      DO 117 I=1,N2                                                             
         C(I) = CH(I)                                                           
  117 CONTINUE                                                                  
      RETURN                                                                    
      END                                                                       
 
      SUBROUTINE PASSF (NAC,IDO,IP,L1,IDL1,CC,C1,C2,CH,CH2,WA)                  
      DIMENSION       CH(IDO,L1,IP)          ,CC(IDO,IP,L1)          ,          
     1                C1(IDO,L1,IP)          ,WA(*)      ,C2(IDL1,IP),          
     2                CH2(IDL1,IP)                                              
      IDOT = IDO/2                                                              
      NT = IP*IDL1                                                              
      IPP2 = IP+2                                                               
      IPPH = (IP+1)/2                                                           
      IDP = IP*IDO                                                              
C                                                                               
      IF (IDO .LT. L1) GO TO 106                                                
      DO 103 J=2,IPPH                                                           
         JC = IPP2-J                                                            
         DO 102 K=1,L1                                                          
            DO 101 I=1,IDO                                                      
               CH(I,K,J) = CC(I,J,K)+CC(I,JC,K)                                 
               CH(I,K,JC) = CC(I,J,K)-CC(I,JC,K)                                
  101       CONTINUE                                                            
  102    CONTINUE                                                               
  103 CONTINUE                                                                  
      DO 105 K=1,L1                                                             
         DO 104 I=1,IDO                                                         
            CH(I,K,1) = CC(I,1,K)                                               
  104    CONTINUE                                                               
  105 CONTINUE                                                                  
      GO TO 112                                                                 
  106 DO 109 J=2,IPPH                                                           
         JC = IPP2-J                                                            
         DO 108 I=1,IDO                                                         
            DO 107 K=1,L1                                                       
               CH(I,K,J) = CC(I,J,K)+CC(I,JC,K)                                 
               CH(I,K,JC) = CC(I,J,K)-CC(I,JC,K)                                
  107       CONTINUE                                                            
  108    CONTINUE                                                               
  109 CONTINUE                                                                  
      DO 111 I=1,IDO                                                            
         DO 110 K=1,L1                                                          
            CH(I,K,1) = CC(I,1,K)                                               
  110    CONTINUE                                                               
  111 CONTINUE                                                                  
  112 IDL = 2-IDO                                                               
      INC = 0                                                                   
      DO 116 L=2,IPPH                                                           
         LC = IPP2-L                                                            
         IDL = IDL+IDO                                                          
         DO 113 IK=1,IDL1                                                       
            C2(IK,L) = CH2(IK,1)+WA(IDL-1)*CH2(IK,2)                            
            C2(IK,LC) = -WA(IDL)*CH2(IK,IP)                                     
  113    CONTINUE                                                               
         IDLJ = IDL                                                             
         INC = INC+IDO                                                          
         DO 115 J=3,IPPH                                                        
            JC = IPP2-J                                                         
            IDLJ = IDLJ+INC                                                     
            IF (IDLJ .GT. IDP) IDLJ = IDLJ-IDP                                  
            WAR = WA(IDLJ-1)                                                    
            WAI = WA(IDLJ)                                                      
            DO 114 IK=1,IDL1                                                    
               C2(IK,L) = C2(IK,L)+WAR*CH2(IK,J)                                
               C2(IK,LC) = C2(IK,LC)-WAI*CH2(IK,JC)                             
  114       CONTINUE                                                            
  115    CONTINUE                                                               
  116 CONTINUE                                                                  
      DO 118 J=2,IPPH                                                           
         DO 117 IK=1,IDL1                                                       
            CH2(IK,1) = CH2(IK,1)+CH2(IK,J)                                     
  117    CONTINUE                                                               
  118 CONTINUE                                                                  
      DO 120 J=2,IPPH                                                           
         JC = IPP2-J                                                            
         DO 119 IK=2,IDL1,2                                                     
            CH2(IK-1,J) = C2(IK-1,J)-C2(IK,JC)                                  
            CH2(IK-1,JC) = C2(IK-1,J)+C2(IK,JC)                                 
            CH2(IK,J) = C2(IK,J)+C2(IK-1,JC)                                    
            CH2(IK,JC) = C2(IK,J)-C2(IK-1,JC)                                   
  119    CONTINUE                                                               
  120 CONTINUE                                                                  
      NAC = 1                                                                   
      IF (IDO .EQ. 2) RETURN                                                    
      NAC = 0                                                                   
      DO 121 IK=1,IDL1                                                          
         C2(IK,1) = CH2(IK,1)                                                   
  121 CONTINUE                                                                  
      DO 123 J=2,IP                                                             
         DO 122 K=1,L1                                                          
            C1(1,K,J) = CH(1,K,J)                                               
            C1(2,K,J) = CH(2,K,J)                                               
  122    CONTINUE                                                               
  123 CONTINUE                                                                  
      IF (IDOT .GT. L1) GO TO 127                                               
      IDIJ = 0                                                                  
      DO 126 J=2,IP                                                             
         IDIJ = IDIJ+2                                                          
         DO 125 I=4,IDO,2                                                       
            IDIJ = IDIJ+2                                                       
            DO 124 K=1,L1                                                       
               C1(I-1,K,J) = WA(IDIJ-1)*CH(I-1,K,J)+WA(IDIJ)*CH(I,K,J)          
               C1(I,K,J) = WA(IDIJ-1)*CH(I,K,J)-WA(IDIJ)*CH(I-1,K,J)            
  124       CONTINUE                                                            
  125    CONTINUE                                                               
  126 CONTINUE                                                                  
      RETURN                                                                    
  127 IDJ = 2-IDO                                                               
      DO 130 J=2,IP                                                             
         IDJ = IDJ+IDO                                                          
         DO 129 K=1,L1                                                          
            IDIJ = IDJ                                                          
            DO 128 I=4,IDO,2                                                    
               IDIJ = IDIJ+2                                                    
               C1(I-1,K,J) = WA(IDIJ-1)*CH(I-1,K,J)+WA(IDIJ)*CH(I,K,J)          
               C1(I,K,J) = WA(IDIJ-1)*CH(I,K,J)-WA(IDIJ)*CH(I-1,K,J)            
  128       CONTINUE                                                            
  129    CONTINUE                                                               
  130 CONTINUE                                                                  
      RETURN                                                                    
      END                                                                       
 
      SUBROUTINE PASSF2 (IDO,L1,CC,CH,WA1)                                      
      DIMENSION       CC(IDO,2,L1)           ,CH(IDO,L1,2)           ,          
     1                WA1(*)                                                    
      IF (IDO .GT. 2) GO TO 102                                                 
      DO 101 K=1,L1                                                             
         CH(1,K,1) = CC(1,1,K)+CC(1,2,K)                                        
         CH(1,K,2) = CC(1,1,K)-CC(1,2,K)                                        
         CH(2,K,1) = CC(2,1,K)+CC(2,2,K)                                        
         CH(2,K,2) = CC(2,1,K)-CC(2,2,K)                                        
  101 CONTINUE                                                                  
      RETURN                                                                    
  102 DO 104 K=1,L1                                                             
         DO 103 I=2,IDO,2                                                       
            CH(I-1,K,1) = CC(I-1,1,K)+CC(I-1,2,K)                               
            TR2 = CC(I-1,1,K)-CC(I-1,2,K)                                       
            CH(I,K,1) = CC(I,1,K)+CC(I,2,K)                                     
            TI2 = CC(I,1,K)-CC(I,2,K)                                           
            CH(I,K,2) = WA1(I-1)*TI2-WA1(I)*TR2                                 
            CH(I-1,K,2) = WA1(I-1)*TR2+WA1(I)*TI2                               
  103    CONTINUE                                                               
  104 CONTINUE                                                                  
      RETURN                                                                    
      END                                                                       
 
      SUBROUTINE PASSF3 (IDO,L1,CC,CH,WA1,WA2)                                  
      DIMENSION       CC(IDO,3,L1)           ,CH(IDO,L1,3)           ,          
     1                WA1(*)     ,WA2(*)                                        
      DATA TAUR,TAUI /-.5,-.866025403784439/                                    
      IF (IDO .NE. 2) GO TO 102                                                 
      DO 101 K=1,L1                                                             
         TR2 = CC(1,2,K)+CC(1,3,K)                                              
         CR2 = CC(1,1,K)+TAUR*TR2                                               
         CH(1,K,1) = CC(1,1,K)+TR2                                              
         TI2 = CC(2,2,K)+CC(2,3,K)                                              
         CI2 = CC(2,1,K)+TAUR*TI2                                               
         CH(2,K,1) = CC(2,1,K)+TI2                                              
         CR3 = TAUI*(CC(1,2,K)-CC(1,3,K))                                       
         CI3 = TAUI*(CC(2,2,K)-CC(2,3,K))                                       
         CH(1,K,2) = CR2-CI3                                                    
         CH(1,K,3) = CR2+CI3                                                    
         CH(2,K,2) = CI2+CR3                                                    
         CH(2,K,3) = CI2-CR3                                                    
  101 CONTINUE                                                                  
      RETURN                                                                    
  102 DO 104 K=1,L1                                                             
         DO 103 I=2,IDO,2                                                       
            TR2 = CC(I-1,2,K)+CC(I-1,3,K)                                       
            CR2 = CC(I-1,1,K)+TAUR*TR2                                          
            CH(I-1,K,1) = CC(I-1,1,K)+TR2                                       
            TI2 = CC(I,2,K)+CC(I,3,K)                                           
            CI2 = CC(I,1,K)+TAUR*TI2                                            
            CH(I,K,1) = CC(I,1,K)+TI2                                           
            CR3 = TAUI*(CC(I-1,2,K)-CC(I-1,3,K))                                
            CI3 = TAUI*(CC(I,2,K)-CC(I,3,K))                                    
            DR2 = CR2-CI3                                                       
            DR3 = CR2+CI3                                                       
            DI2 = CI2+CR3                                                       
            DI3 = CI2-CR3                                                       
            CH(I,K,2) = WA1(I-1)*DI2-WA1(I)*DR2                                 
            CH(I-1,K,2) = WA1(I-1)*DR2+WA1(I)*DI2                               
            CH(I,K,3) = WA2(I-1)*DI3-WA2(I)*DR3                                 
            CH(I-1,K,3) = WA2(I-1)*DR3+WA2(I)*DI3                               
  103    CONTINUE                                                               
  104 CONTINUE                                                                  
      RETURN                                                                    
      END                                                                       
 
      SUBROUTINE PASSF4 (IDO,L1,CC,CH,WA1,WA2,WA3)                              
      DIMENSION       CC(IDO,4,L1)           ,CH(IDO,L1,4)           ,          
     1                WA1(*)     ,WA2(*)     ,WA3(*)                            
      IF (IDO .NE. 2) GO TO 102                                                 
      DO 101 K=1,L1                                                             
         TI1 = CC(2,1,K)-CC(2,3,K)                                              
         TI2 = CC(2,1,K)+CC(2,3,K)                                              
         TR4 = CC(2,2,K)-CC(2,4,K)                                              
         TI3 = CC(2,2,K)+CC(2,4,K)                                              
         TR1 = CC(1,1,K)-CC(1,3,K)                                              
         TR2 = CC(1,1,K)+CC(1,3,K)                                              
         TI4 = CC(1,4,K)-CC(1,2,K)                                              
         TR3 = CC(1,2,K)+CC(1,4,K)                                              
         CH(1,K,1) = TR2+TR3                                                    
         CH(1,K,3) = TR2-TR3                                                    
         CH(2,K,1) = TI2+TI3                                                    
         CH(2,K,3) = TI2-TI3                                                    
         CH(1,K,2) = TR1+TR4                                                    
         CH(1,K,4) = TR1-TR4                                                    
         CH(2,K,2) = TI1+TI4                                                    
         CH(2,K,4) = TI1-TI4                                                    
  101 CONTINUE                                                                  
      RETURN                                                                    
  102 DO 104 K=1,L1                                                             
         DO 103 I=2,IDO,2                                                       
            TI1 = CC(I,1,K)-CC(I,3,K)                                           
            TI2 = CC(I,1,K)+CC(I,3,K)                                           
            TI3 = CC(I,2,K)+CC(I,4,K)                                           
            TR4 = CC(I,2,K)-CC(I,4,K)                                           
            TR1 = CC(I-1,1,K)-CC(I-1,3,K)                                       
            TR2 = CC(I-1,1,K)+CC(I-1,3,K)                                       
            TI4 = CC(I-1,4,K)-CC(I-1,2,K)                                       
            TR3 = CC(I-1,2,K)+CC(I-1,4,K)                                       
            CH(I-1,K,1) = TR2+TR3                                               
            CR3 = TR2-TR3                                                       
            CH(I,K,1) = TI2+TI3                                                 
            CI3 = TI2-TI3                                                       
            CR2 = TR1+TR4                                                       
            CR4 = TR1-TR4                                                       
            CI2 = TI1+TI4                                                       
            CI4 = TI1-TI4                                                       
            CH(I-1,K,2) = WA1(I-1)*CR2+WA1(I)*CI2                               
            CH(I,K,2) = WA1(I-1)*CI2-WA1(I)*CR2                                 
            CH(I-1,K,3) = WA2(I-1)*CR3+WA2(I)*CI3                               
            CH(I,K,3) = WA2(I-1)*CI3-WA2(I)*CR3                                 
            CH(I-1,K,4) = WA3(I-1)*CR4+WA3(I)*CI4                               
            CH(I,K,4) = WA3(I-1)*CI4-WA3(I)*CR4                                 
  103    CONTINUE                                                               
  104 CONTINUE                                                                  
      RETURN                                                                    
      END                                                                       
 
      SUBROUTINE PASSF5 (IDO,L1,CC,CH,WA1,WA2,WA3,WA4)                          
      DIMENSION       CC(IDO,5,L1)           ,CH(IDO,L1,5)           ,          
     1                WA1(*)     ,WA2(*)     ,WA3(*)     ,WA4(*)                
      DATA TR11,TI11,TR12,TI12 /.309016994374947,-.951056516295154,             
     1-.809016994374947,-.587785252292473/                                      
      IF (IDO .NE. 2) GO TO 102                                                 
      DO 101 K=1,L1                                                             
         TI5 = CC(2,2,K)-CC(2,5,K)                                              
         TI2 = CC(2,2,K)+CC(2,5,K)                                              
         TI4 = CC(2,3,K)-CC(2,4,K)                                              
         TI3 = CC(2,3,K)+CC(2,4,K)                                              
         TR5 = CC(1,2,K)-CC(1,5,K)                                              
         TR2 = CC(1,2,K)+CC(1,5,K)                                              
         TR4 = CC(1,3,K)-CC(1,4,K)                                              
         TR3 = CC(1,3,K)+CC(1,4,K)                                              
         CH(1,K,1) = CC(1,1,K)+TR2+TR3                                          
         CH(2,K,1) = CC(2,1,K)+TI2+TI3                                          
         CR2 = CC(1,1,K)+TR11*TR2+TR12*TR3                                      
         CI2 = CC(2,1,K)+TR11*TI2+TR12*TI3                                      
         CR3 = CC(1,1,K)+TR12*TR2+TR11*TR3                                      
         CI3 = CC(2,1,K)+TR12*TI2+TR11*TI3                                      
         CR5 = TI11*TR5+TI12*TR4                                                
         CI5 = TI11*TI5+TI12*TI4                                                
         CR4 = TI12*TR5-TI11*TR4                                                
         CI4 = TI12*TI5-TI11*TI4                                                
         CH(1,K,2) = CR2-CI5                                                    
         CH(1,K,5) = CR2+CI5                                                    
         CH(2,K,2) = CI2+CR5                                                    
         CH(2,K,3) = CI3+CR4                                                    
         CH(1,K,3) = CR3-CI4                                                    
         CH(1,K,4) = CR3+CI4                                                    
         CH(2,K,4) = CI3-CR4                                                    
         CH(2,K,5) = CI2-CR5                                                    
  101 CONTINUE                                                                  
      RETURN                                                                    
  102 DO 104 K=1,L1                                                             
         DO 103 I=2,IDO,2                                                       
            TI5 = CC(I,2,K)-CC(I,5,K)                                           
            TI2 = CC(I,2,K)+CC(I,5,K)                                           
            TI4 = CC(I,3,K)-CC(I,4,K)                                           
            TI3 = CC(I,3,K)+CC(I,4,K)                                           
            TR5 = CC(I-1,2,K)-CC(I-1,5,K)                                       
            TR2 = CC(I-1,2,K)+CC(I-1,5,K)                                       
            TR4 = CC(I-1,3,K)-CC(I-1,4,K)                                       
            TR3 = CC(I-1,3,K)+CC(I-1,4,K)                                       
            CH(I-1,K,1) = CC(I-1,1,K)+TR2+TR3                                   
            CH(I,K,1) = CC(I,1,K)+TI2+TI3                                       
            CR2 = CC(I-1,1,K)+TR11*TR2+TR12*TR3                                 
            CI2 = CC(I,1,K)+TR11*TI2+TR12*TI3                                   
            CR3 = CC(I-1,1,K)+TR12*TR2+TR11*TR3                                 
            CI3 = CC(I,1,K)+TR12*TI2+TR11*TI3                                   
            CR5 = TI11*TR5+TI12*TR4                                             
            CI5 = TI11*TI5+TI12*TI4                                             
            CR4 = TI12*TR5-TI11*TR4                                             
            CI4 = TI12*TI5-TI11*TI4                                             
            DR3 = CR3-CI4                                                       
            DR4 = CR3+CI4                                                       
            DI3 = CI3+CR4                                                       
            DI4 = CI3-CR4                                                       
            DR5 = CR2+CI5                                                       
            DR2 = CR2-CI5                                                       
            DI5 = CI2-CR5                                                       
            DI2 = CI2+CR5                                                       
            CH(I-1,K,2) = WA1(I-1)*DR2+WA1(I)*DI2                               
            CH(I,K,2) = WA1(I-1)*DI2-WA1(I)*DR2                                 
            CH(I-1,K,3) = WA2(I-1)*DR3+WA2(I)*DI3                               
            CH(I,K,3) = WA2(I-1)*DI3-WA2(I)*DR3                                 
            CH(I-1,K,4) = WA3(I-1)*DR4+WA3(I)*DI4                               
            CH(I,K,4) = WA3(I-1)*DI4-WA3(I)*DR4                                 
            CH(I-1,K,5) = WA4(I-1)*DR5+WA4(I)*DI5                               
            CH(I,K,5) = WA4(I-1)*DI5-WA4(I)*DR5                                 
  103    CONTINUE                                                               
  104 CONTINUE                                                                  
      RETURN                                                                    
      END                                                                       
 
C     SUBROUTINE CFFTI(N,WSAVE)                                                 
C                                                                               
C     SUBROUTINE CFFTI INITIALIZES THE ARRAY WSAVE WHICH IS USED IN             
C     BOTH CFFTF AND CFFTB. THE PRIME FACTORIZATION OF N TOGETHER WITH          
C     A TABULATION OF THE TRIGONOMETRIC FUNCTIONS ARE COMPUTED AND              
C     STORED IN WSAVE.                                                          
C                                                                               
C     INPUT PARAMETER                                                           
C                                                                               
C     N       THE LENGTH OF THE SEQUENCE TO BE TRANSFORMED                      
C                                                                               
C     OUTPUT PARAMETER                                                          
C                                                                               
C     WSAVE   A WORK ARRAY WHICH MUST BE DIMENSIONED AT LEAST 4*N+15            
C             THE SAME WORK ARRAY CAN BE USED FOR BOTH CFFTF AND CFFTB          
C             AS LONG AS N REMAINS UNCHANGED. DIFFERENT WSAVE ARRAYS            
C             ARE REQUIRED FOR DIFFERENT VALUES OF N. THE CONTENTS OF           
C             WSAVE MUST NOT BE CHANGED BETWEEN CALLS OF CFFTF OR CFFTB.        
C                                                                               
      SUBROUTINE CFFTI (N,WSAVE)                                                
      DIMENSION       WSAVE(*)                                                  
C                                                                               
      IF (N .EQ. 1) RETURN                                                      
      IW1 = N+N+1                                                               
      IW2 = IW1+N+N                                                             
      CALL CFFTI1 (N,WSAVE(IW1),WSAVE(IW2))                                     
      RETURN                                                                    
      END                                                                       
 
      SUBROUTINE CFFTI1 (N,WA,IFAC)                                             
      DIMENSION       WA(*)      ,IFAC(*)    ,NTRYH(4)                          
      DATA NTRYH(1),NTRYH(2),NTRYH(3),NTRYH(4)/3,4,2,5/                         
      NL = N                                                                    
      NF = 0                                                                    
      J = 0                                                                     
  101 J = J+1                                                                   
      IF (J-4) 102,102,103                                                      
  102 NTRY = NTRYH(J)                                                           
      GO TO 104                                                                 
  103 NTRY = NTRY+2                                                             
  104 NQ = NL/NTRY                                                              
      NR = NL-NTRY*NQ                                                           
      IF (NR) 101,105,101                                                       
  105 NF = NF+1                                                                 
      IFAC(NF+2) = NTRY                                                         
      NL = NQ                                                                   
      IF (NTRY .NE. 2) GO TO 107                                                
      IF (NF .EQ. 1) GO TO 107                                                  
      DO 106 I=2,NF                                                             
         IB = NF-I+2                                                            
         IFAC(IB+2) = IFAC(IB+1)                                                
  106 CONTINUE                                                                  
      IFAC(3) = 2                                                               
  107 IF (NL .NE. 1) GO TO 104                                                  
      IFAC(1) = N                                                               
      IFAC(2) = NF                                                              
c     TPI = 2.*PIMACH(DUM)                                                      
      TPI = 8.*ATAN(1.0)
      ARGH = TPI/FLOAT(N)                                                       
      I = 2                                                                     
      L1 = 1                                                                    
      DO 110 K1=1,NF                                                            
         IP = IFAC(K1+2)                                                        
         LD = 0                                                                 
         L2 = L1*IP                                                             
         IDO = N/L2                                                             
         IDOT = IDO+IDO+2                                                       
         IPM = IP-1                                                             
         DO 109 J=1,IPM                                                         
            I1 = I                                                              
            WA(I-1) = 1.                                                        
            WA(I) = 0.                                                          
            LD = LD+L1                                                          
            FI = 0.                                                             
            ARGLD = FLOAT(LD)*ARGH                                              
            DO 108 II=4,IDOT,2                                                  
               I = I+2                                                          
               FI = FI+1.                                                       
               ARG = FI*ARGLD                                                   
               WA(I-1) = COS(ARG)                                               
               WA(I) = SIN(ARG)                                                 
  108       CONTINUE                                                            
            IF (IP .LE. 5) GO TO 109                                            
            WA(I1-1) = WA(I-1)                                                  
            WA(I1) = WA(I)                                                      
  109    CONTINUE                                                               
         L1 = L2                                                                
  110 CONTINUE                                                                  
      RETURN                                                                    
      END                                                                       
 
c     FUNCTION PIMACH (DUM)                                                     
C     PI=3.1415926535897932384626433832795028841971693993751058209749446        
C                                                                               
c     PIMACH = 4.*ATAN(1.0)                                                     
c     RETURN                                                                    
c     END                                                                       
 
C     SUBROUTINE CFFTB(N,C,WSAVE)                                               
C                                                                               
C     SUBROUTINE CFFTB COMPUTES THE BACKWARD COMPLEX DISCRETE FOURIER           
C     TRANSFORM (THE FOURIER SYNTHESIS). EQUIVALENTLY , CFFTB COMPUTES          
C     A COMPLEX PERIODIC SEQUENCE FROM ITS FOURIER COEFFICIENTS.                
C     THE TRANSFORM IS DEFINED BELOW AT OUTPUT PARAMETER C.                     
C                                                                               
C     A CALL OF CFFTF FOLLOWED BY A CALL OF CFFTB WILL MULTIPLY THE             
C     SEQUENCE BY N.                                                            
C                                                                               
C     THE ARRAY WSAVE WHICH IS USED BY SUBROUTINE CFFTB MUST BE                 
C     INITIALIZED BY CALLING SUBROUTINE CFFTI(N,WSAVE).                         
C                                                                               
C     INPUT PARAMETERS                                                          
C                                                                               
C                                                                               
C     N      THE LENGTH OF THE COMPLEX SEQUENCE C. THE METHOD IS                
C            MORE EFFICIENT WHEN N IS THE PRODUCT OF SMALL PRIMES.              
C                                                                               
C     C      A COMPLEX ARRAY OF LENGTH N WHICH CONTAINS THE SEQUENCE            
C                                                                               
C     WSAVE   A REAL WORK ARRAY WHICH MUST BE DIMENSIONED AT LEAST 4N+15        
C             IN THE PROGRAM THAT CALLS CFFTB. THE WSAVE ARRAY MUST BE          
C             INITIALIZED BY CALLING SUBROUTINE CFFTI(N,WSAVE) AND A            
C             DIFFERENT WSAVE ARRAY MUST BE USED FOR EACH DIFFERENT             
C             VALUE OF N. THIS INITIALIZATION DOES NOT HAVE TO BE               
C             REPEATED SO LONG AS N REMAINS UNCHANGED THUS SUBSEQUENT           
C             TRANSFORMS CAN BE OBTAINED FASTER THAN THE FIRST.                 
C             THE SAME WSAVE ARRAY CAN BE USED BY CFFTF AND CFFTB.              
C                                                                               
C     OUTPUT PARAMETERS                                                         
C                                                                               
C     C      FOR J=1,...,N                                                      
C                                                                               
C                C(J)=THE SUM FROM K=1,...,N OF                                 
C                                                                               
C                      C(K)*EXP(I*(J-1)*(K-1)*2*PI/N)                           
C                                                                               
C                            WHERE I=SQRT(-1)                                   
C                                                                               
C     WSAVE   CONTAINS INITIALIZATION CALCULATIONS WHICH MUST NOT BE            
C             DESTROYED BETWEEN CALLS OF SUBROUTINE CFFTF OR CFFTB              
C
      SUBROUTINE CFFTB (N,C,WSAVE)                                              
      DIMENSION       C(*)       ,WSAVE(*)                                      
C                                                                               
      IF (N .EQ. 1) RETURN                                                      
      IW1 = N+N+1                                                               
      IW2 = IW1+N+N                                                             
      CALL CFFTB1 (N,C,WSAVE,WSAVE(IW1),WSAVE(IW2))                             
      RETURN                                                                    
      END                                                                       
 
      SUBROUTINE CFFTB1 (N,C,CH,WA,IFAC)                                        
      DIMENSION       CH(*)      ,C(*)       ,WA(*)      ,IFAC(*)               
      NF = IFAC(2)                                                              
      NA = 0                                                                    
      L1 = 1                                                                    
      IW = 1                                                                    
      DO 116 K1=1,NF                                                            
         IP = IFAC(K1+2)                                                        
         L2 = IP*L1                                                             
         IDO = N/L2                                                             
         IDOT = IDO+IDO                                                         
         IDL1 = IDOT*L1                                                         
         IF (IP .NE. 4) GO TO 103                                               
         IX2 = IW+IDOT                                                          
         IX3 = IX2+IDOT                                                         
         IF (NA .NE. 0) GO TO 101                                               
         CALL PASSB4 (IDOT,L1,C,CH,WA(IW),WA(IX2),WA(IX3))                      
         GO TO 102                                                              
  101    CALL PASSB4 (IDOT,L1,CH,C,WA(IW),WA(IX2),WA(IX3))                      
  102    NA = 1-NA                                                              
         GO TO 115                                                              
  103    IF (IP .NE. 2) GO TO 106                                               
         IF (NA .NE. 0) GO TO 104                                               
         CALL PASSB2 (IDOT,L1,C,CH,WA(IW))                                      
         GO TO 105                                                              
  104    CALL PASSB2 (IDOT,L1,CH,C,WA(IW))                                      
  105    NA = 1-NA                                                              
         GO TO 115                                                              
  106    IF (IP .NE. 3) GO TO 109                                               
         IX2 = IW+IDOT                                                          
         IF (NA .NE. 0) GO TO 107                                               
         CALL PASSB3 (IDOT,L1,C,CH,WA(IW),WA(IX2))                              
         GO TO 108                                                              
  107    CALL PASSB3 (IDOT,L1,CH,C,WA(IW),WA(IX2))                              
  108    NA = 1-NA                                                              
         GO TO 115                                                              
  109    IF (IP .NE. 5) GO TO 112                                               
         IX2 = IW+IDOT                                                          
         IX3 = IX2+IDOT                                                         
         IX4 = IX3+IDOT                                                         
         IF (NA .NE. 0) GO TO 110                                               
         CALL PASSB5 (IDOT,L1,C,CH,WA(IW),WA(IX2),WA(IX3),WA(IX4))              
         GO TO 111                                                              
  110    CALL PASSB5 (IDOT,L1,CH,C,WA(IW),WA(IX2),WA(IX3),WA(IX4))              
  111    NA = 1-NA                                                              
         GO TO 115                                                              
  112    IF (NA .NE. 0) GO TO 113                                               
         CALL PASSB (NAC,IDOT,IP,L1,IDL1,C,C,C,CH,CH,WA(IW))                    
         GO TO 114                                                              
  113    CALL PASSB (NAC,IDOT,IP,L1,IDL1,CH,CH,CH,C,C,WA(IW))                   
  114    IF (NAC .NE. 0) NA = 1-NA                                              
  115    L1 = L2                                                                
         IW = IW+(IP-1)*IDOT                                                    
  116 CONTINUE                                                                  
      IF (NA .EQ. 0) RETURN                                                     
      N2 = N+N                                                                  
      DO 117 I=1,N2                                                             
         C(I) = CH(I)                                                           
  117 CONTINUE                                                                  
      RETURN                                                                    
      END                                                                       
 
      SUBROUTINE PASSB (NAC,IDO,IP,L1,IDL1,CC,C1,C2,CH,CH2,WA)                  
      DIMENSION       CH(IDO,L1,IP)          ,CC(IDO,IP,L1)          ,          
     1                C1(IDO,L1,IP)          ,WA(*)      ,C2(IDL1,IP),          
     2                CH2(IDL1,IP)                                              
      IDOT = IDO/2                                                              
      NT = IP*IDL1                                                              
      IPP2 = IP+2                                                               
      IPPH = (IP+1)/2                                                           
      IDP = IP*IDO                                                              
C                                                                               
      IF (IDO .LT. L1) GO TO 106                                                
      DO 103 J=2,IPPH                                                           
         JC = IPP2-J                                                            
         DO 102 K=1,L1                                                          
            DO 101 I=1,IDO                                                      
               CH(I,K,J) = CC(I,J,K)+CC(I,JC,K)                                 
               CH(I,K,JC) = CC(I,J,K)-CC(I,JC,K)                                
  101       CONTINUE                                                            
  102    CONTINUE                                                               
  103 CONTINUE                                                                  
      DO 105 K=1,L1                                                             
         DO 104 I=1,IDO                                                         
            CH(I,K,1) = CC(I,1,K)                                               
  104    CONTINUE                                                               
  105 CONTINUE                                                                  
      GO TO 112                                                                 
  106 DO 109 J=2,IPPH                                                           
         JC = IPP2-J                                                            
         DO 108 I=1,IDO                                                         
            DO 107 K=1,L1                                                       
               CH(I,K,J) = CC(I,J,K)+CC(I,JC,K)                                 
               CH(I,K,JC) = CC(I,J,K)-CC(I,JC,K)                                
  107       CONTINUE                                                            
  108    CONTINUE                                                               
  109 CONTINUE                                                                  
      DO 111 I=1,IDO                                                            
         DO 110 K=1,L1                                                          
            CH(I,K,1) = CC(I,1,K)                                               
  110    CONTINUE                                                               
  111 CONTINUE                                                                  
  112 IDL = 2-IDO                                                               
      INC = 0                                                                   
      DO 116 L=2,IPPH                                                           
         LC = IPP2-L                                                            
         IDL = IDL+IDO                                                          
         DO 113 IK=1,IDL1                                                       
            C2(IK,L) = CH2(IK,1)+WA(IDL-1)*CH2(IK,2)                            
            C2(IK,LC) = WA(IDL)*CH2(IK,IP)                                      
  113    CONTINUE                                                               
         IDLJ = IDL                                                             
         INC = INC+IDO                                                          
         DO 115 J=3,IPPH                                                        
            JC = IPP2-J                                                         
            IDLJ = IDLJ+INC                                                     
            IF (IDLJ .GT. IDP) IDLJ = IDLJ-IDP                                  
            WAR = WA(IDLJ-1)                                                    
            WAI = WA(IDLJ)                                                      
            DO 114 IK=1,IDL1                                                    
               C2(IK,L) = C2(IK,L)+WAR*CH2(IK,J)                                
               C2(IK,LC) = C2(IK,LC)+WAI*CH2(IK,JC)                             
  114       CONTINUE                                                            
  115    CONTINUE                                                               
  116 CONTINUE                                                                  
      DO 118 J=2,IPPH                                                           
         DO 117 IK=1,IDL1                                                       
            CH2(IK,1) = CH2(IK,1)+CH2(IK,J)                                     
  117    CONTINUE                                                               
  118 CONTINUE                                                                  
      DO 120 J=2,IPPH                                                           
         JC = IPP2-J                                                            
         DO 119 IK=2,IDL1,2                                                     
            CH2(IK-1,J) = C2(IK-1,J)-C2(IK,JC)                                  
            CH2(IK-1,JC) = C2(IK-1,J)+C2(IK,JC)                                 
            CH2(IK,J) = C2(IK,J)+C2(IK-1,JC)                                    
            CH2(IK,JC) = C2(IK,J)-C2(IK-1,JC)                                   
  119    CONTINUE                                                               
  120 CONTINUE                                                                  
      NAC = 1                                                                   
      IF (IDO .EQ. 2) RETURN                                                    
      NAC = 0                                                                   
      DO 121 IK=1,IDL1                                                          
         C2(IK,1) = CH2(IK,1)                                                   
  121 CONTINUE                                                                  
      DO 123 J=2,IP                                                             
         DO 122 K=1,L1                                                          
            C1(1,K,J) = CH(1,K,J)                                               
            C1(2,K,J) = CH(2,K,J)                                               
  122    CONTINUE                                                               
  123 CONTINUE                                                                  
      IF (IDOT .GT. L1) GO TO 127                                               
      IDIJ = 0                                                                  
      DO 126 J=2,IP                                                             
         IDIJ = IDIJ+2                                                          
         DO 125 I=4,IDO,2                                                       
            IDIJ = IDIJ+2                                                       
            DO 124 K=1,L1                                                       
               C1(I-1,K,J) = WA(IDIJ-1)*CH(I-1,K,J)-WA(IDIJ)*CH(I,K,J)          
               C1(I,K,J) = WA(IDIJ-1)*CH(I,K,J)+WA(IDIJ)*CH(I-1,K,J)            
  124       CONTINUE                                                            
  125    CONTINUE                                                               
  126 CONTINUE                                                                  
      RETURN                                                                    
  127 IDJ = 2-IDO                                                               
      DO 130 J=2,IP                                                             
         IDJ = IDJ+IDO                                                          
         DO 129 K=1,L1                                                          
            IDIJ = IDJ                                                          
            DO 128 I=4,IDO,2                                                    
               IDIJ = IDIJ+2                                                    
               C1(I-1,K,J) = WA(IDIJ-1)*CH(I-1,K,J)-WA(IDIJ)*CH(I,K,J)          
               C1(I,K,J) = WA(IDIJ-1)*CH(I,K,J)+WA(IDIJ)*CH(I-1,K,J)            
  128       CONTINUE                                                            
  129    CONTINUE                                                               
  130 CONTINUE                                                                  
      RETURN                                                                    
      END                                                                       
 
      SUBROUTINE PASSB2 (IDO,L1,CC,CH,WA1)                                      
      DIMENSION       CC(IDO,2,L1)           ,CH(IDO,L1,2)           ,          
     1                WA1(1)                                                    
      IF (IDO .GT. 2) GO TO 102                                                 
      DO 101 K=1,L1                                                             
         CH(1,K,1) = CC(1,1,K)+CC(1,2,K)                                        
         CH(1,K,2) = CC(1,1,K)-CC(1,2,K)                                        
         CH(2,K,1) = CC(2,1,K)+CC(2,2,K)                                        
         CH(2,K,2) = CC(2,1,K)-CC(2,2,K)                                        
  101 CONTINUE                                                                  
      RETURN                                                                    
  102 DO 104 K=1,L1                                                             
         DO 103 I=2,IDO,2                                                       
            CH(I-1,K,1) = CC(I-1,1,K)+CC(I-1,2,K)                               
            TR2 = CC(I-1,1,K)-CC(I-1,2,K)                                       
            CH(I,K,1) = CC(I,1,K)+CC(I,2,K)                                     
            TI2 = CC(I,1,K)-CC(I,2,K)                                           
            CH(I,K,2) = WA1(I-1)*TI2+WA1(I)*TR2                                 
            CH(I-1,K,2) = WA1(I-1)*TR2-WA1(I)*TI2                               
  103    CONTINUE                                                               
  104 CONTINUE                                                                  
      RETURN                                                                    
      END                                                                       
 
      SUBROUTINE PASSB3 (IDO,L1,CC,CH,WA1,WA2)                                  
      DIMENSION       CC(IDO,3,L1)           ,CH(IDO,L1,3)           ,          
     1                WA1(*)     ,WA2(*)                                        
      DATA TAUR,TAUI /-.5,.866025403784439/                                     
      IF (IDO .NE. 2) GO TO 102                                                 
      DO 101 K=1,L1                                                             
         TR2 = CC(1,2,K)+CC(1,3,K)                                              
         CR2 = CC(1,1,K)+TAUR*TR2                                               
         CH(1,K,1) = CC(1,1,K)+TR2                                              
         TI2 = CC(2,2,K)+CC(2,3,K)                                              
         CI2 = CC(2,1,K)+TAUR*TI2                                               
         CH(2,K,1) = CC(2,1,K)+TI2                                              
         CR3 = TAUI*(CC(1,2,K)-CC(1,3,K))                                       
         CI3 = TAUI*(CC(2,2,K)-CC(2,3,K))                                       
         CH(1,K,2) = CR2-CI3                                                    
         CH(1,K,3) = CR2+CI3                                                    
         CH(2,K,2) = CI2+CR3                                                    
         CH(2,K,3) = CI2-CR3                                                    
  101 CONTINUE                                                                  
      RETURN                                                                    
  102 DO 104 K=1,L1                                                             
         DO 103 I=2,IDO,2                                                       
            TR2 = CC(I-1,2,K)+CC(I-1,3,K)                                       
            CR2 = CC(I-1,1,K)+TAUR*TR2                                          
            CH(I-1,K,1) = CC(I-1,1,K)+TR2                                       
            TI2 = CC(I,2,K)+CC(I,3,K)                                           
            CI2 = CC(I,1,K)+TAUR*TI2                                            
            CH(I,K,1) = CC(I,1,K)+TI2                                           
            CR3 = TAUI*(CC(I-1,2,K)-CC(I-1,3,K))                                
            CI3 = TAUI*(CC(I,2,K)-CC(I,3,K))                                    
            DR2 = CR2-CI3                                                       
            DR3 = CR2+CI3                                                       
            DI2 = CI2+CR3                                                       
            DI3 = CI2-CR3                                                       
            CH(I,K,2) = WA1(I-1)*DI2+WA1(I)*DR2                                 
            CH(I-1,K,2) = WA1(I-1)*DR2-WA1(I)*DI2                               
            CH(I,K,3) = WA2(I-1)*DI3+WA2(I)*DR3                                 
            CH(I-1,K,3) = WA2(I-1)*DR3-WA2(I)*DI3                               
  103    CONTINUE                                                               
  104 CONTINUE                                                                  
      RETURN                                                                    
      END                                                                       
 
      SUBROUTINE PASSB4 (IDO,L1,CC,CH,WA1,WA2,WA3)                              
      DIMENSION       CC(IDO,4,L1)           ,CH(IDO,L1,4)           ,          
     1                WA1(*)     ,WA2(*)     ,WA3(*)                            
      IF (IDO .NE. 2) GO TO 102                                                 
      DO 101 K=1,L1                                                             
         TI1 = CC(2,1,K)-CC(2,3,K)                                              
         TI2 = CC(2,1,K)+CC(2,3,K)                                              
         TR4 = CC(2,4,K)-CC(2,2,K)                                              
         TI3 = CC(2,2,K)+CC(2,4,K)                                              
         TR1 = CC(1,1,K)-CC(1,3,K)                                              
         TR2 = CC(1,1,K)+CC(1,3,K)                                              
         TI4 = CC(1,2,K)-CC(1,4,K)                                              
         TR3 = CC(1,2,K)+CC(1,4,K)                                              
         CH(1,K,1) = TR2+TR3                                                    
         CH(1,K,3) = TR2-TR3                                                    
         CH(2,K,1) = TI2+TI3                                                    
         CH(2,K,3) = TI2-TI3                                                    
         CH(1,K,2) = TR1+TR4                                                    
         CH(1,K,4) = TR1-TR4                                                    
         CH(2,K,2) = TI1+TI4                                                    
         CH(2,K,4) = TI1-TI4                                                    
  101 CONTINUE                                                                  
      RETURN                                                                    
  102 DO 104 K=1,L1                                                             
         DO 103 I=2,IDO,2                                                       
            TI1 = CC(I,1,K)-CC(I,3,K)                                           
            TI2 = CC(I,1,K)+CC(I,3,K)                                           
            TI3 = CC(I,2,K)+CC(I,4,K)                                           
            TR4 = CC(I,4,K)-CC(I,2,K)                                           
            TR1 = CC(I-1,1,K)-CC(I-1,3,K)                                       
            TR2 = CC(I-1,1,K)+CC(I-1,3,K)                                       
            TI4 = CC(I-1,2,K)-CC(I-1,4,K)                                       
            TR3 = CC(I-1,2,K)+CC(I-1,4,K)                                       
            CH(I-1,K,1) = TR2+TR3                                               
            CR3 = TR2-TR3                                                       
            CH(I,K,1) = TI2+TI3                                                 
            CI3 = TI2-TI3                                                       
            CR2 = TR1+TR4                                                       
            CR4 = TR1-TR4                                                       
            CI2 = TI1+TI4                                                       
            CI4 = TI1-TI4                                                       
            CH(I-1,K,2) = WA1(I-1)*CR2-WA1(I)*CI2                               
            CH(I,K,2) = WA1(I-1)*CI2+WA1(I)*CR2                                 
            CH(I-1,K,3) = WA2(I-1)*CR3-WA2(I)*CI3                               
            CH(I,K,3) = WA2(I-1)*CI3+WA2(I)*CR3                                 
            CH(I-1,K,4) = WA3(I-1)*CR4-WA3(I)*CI4                               
            CH(I,K,4) = WA3(I-1)*CI4+WA3(I)*CR4                                 
  103    CONTINUE                                                               
  104 CONTINUE                                                                  
      RETURN                                                                    
      END                                                                       
 
      SUBROUTINE PASSB5 (IDO,L1,CC,CH,WA1,WA2,WA3,WA4)                          
      DIMENSION       CC(IDO,5,L1)           ,CH(IDO,L1,5)           ,          
     1                WA1(*)     ,WA2(*)     ,WA3(*)     ,WA4(*)                
      DATA TR11,TI11,TR12,TI12 /.309016994374947,.951056516295154,              
     1-.809016994374947,.587785252292473/                                       
      IF (IDO .NE. 2) GO TO 102                                                 
      DO 101 K=1,L1                                                             
         TI5 = CC(2,2,K)-CC(2,5,K)                                              
         TI2 = CC(2,2,K)+CC(2,5,K)                                              
         TI4 = CC(2,3,K)-CC(2,4,K)                                              
         TI3 = CC(2,3,K)+CC(2,4,K)                                              
         TR5 = CC(1,2,K)-CC(1,5,K)                                              
         TR2 = CC(1,2,K)+CC(1,5,K)                                              
         TR4 = CC(1,3,K)-CC(1,4,K)                                              
         TR3 = CC(1,3,K)+CC(1,4,K)                                              
         CH(1,K,1) = CC(1,1,K)+TR2+TR3                                          
         CH(2,K,1) = CC(2,1,K)+TI2+TI3                                          
         CR2 = CC(1,1,K)+TR11*TR2+TR12*TR3                                      
         CI2 = CC(2,1,K)+TR11*TI2+TR12*TI3                                      
         CR3 = CC(1,1,K)+TR12*TR2+TR11*TR3                                      
         CI3 = CC(2,1,K)+TR12*TI2+TR11*TI3                                      
         CR5 = TI11*TR5+TI12*TR4                                                
         CI5 = TI11*TI5+TI12*TI4                                                
         CR4 = TI12*TR5-TI11*TR4                                                
         CI4 = TI12*TI5-TI11*TI4                                                
         CH(1,K,2) = CR2-CI5                                                    
         CH(1,K,5) = CR2+CI5                                                    
         CH(2,K,2) = CI2+CR5                                                    
         CH(2,K,3) = CI3+CR4                                                    
         CH(1,K,3) = CR3-CI4                                                    
         CH(1,K,4) = CR3+CI4                                                    
         CH(2,K,4) = CI3-CR4                                                    
         CH(2,K,5) = CI2-CR5                                                    
  101 CONTINUE                                                                  
      RETURN                                                                    
  102 DO 104 K=1,L1                                                             
         DO 103 I=2,IDO,2                                                       
            TI5 = CC(I,2,K)-CC(I,5,K)                                           
            TI2 = CC(I,2,K)+CC(I,5,K)                                           
            TI4 = CC(I,3,K)-CC(I,4,K)                                           
            TI3 = CC(I,3,K)+CC(I,4,K)                                           
            TR5 = CC(I-1,2,K)-CC(I-1,5,K)                                       
            TR2 = CC(I-1,2,K)+CC(I-1,5,K)                                       
            TR4 = CC(I-1,3,K)-CC(I-1,4,K)                                       
            TR3 = CC(I-1,3,K)+CC(I-1,4,K)                                       
            CH(I-1,K,1) = CC(I-1,1,K)+TR2+TR3                                   
            CH(I,K,1) = CC(I,1,K)+TI2+TI3                                       
            CR2 = CC(I-1,1,K)+TR11*TR2+TR12*TR3                                 
            CI2 = CC(I,1,K)+TR11*TI2+TR12*TI3                                   
            CR3 = CC(I-1,1,K)+TR12*TR2+TR11*TR3                                 
            CI3 = CC(I,1,K)+TR12*TI2+TR11*TI3                                   
            CR5 = TI11*TR5+TI12*TR4                                             
            CI5 = TI11*TI5+TI12*TI4                                             
            CR4 = TI12*TR5-TI11*TR4                                             
            CI4 = TI12*TI5-TI11*TI4                                             
            DR3 = CR3-CI4                                                       
            DR4 = CR3+CI4                                                       
            DI3 = CI3+CR4                                                       
            DI4 = CI3-CR4                                                       
            DR5 = CR2+CI5                                                       
            DR2 = CR2-CI5                                                       
            DI5 = CI2-CR5                                                       
            DI2 = CI2+CR5                                                       
            CH(I-1,K,2) = WA1(I-1)*DR2-WA1(I)*DI2                               
            CH(I,K,2) = WA1(I-1)*DI2+WA1(I)*DR2                                 
            CH(I-1,K,3) = WA2(I-1)*DR3-WA2(I)*DI3                               
            CH(I,K,3) = WA2(I-1)*DI3+WA2(I)*DR3                                 
            CH(I-1,K,4) = WA3(I-1)*DR4-WA3(I)*DI4                               
            CH(I,K,4) = WA3(I-1)*DI4+WA3(I)*DR4                                 
            CH(I-1,K,5) = WA4(I-1)*DR5-WA4(I)*DI5                               
            CH(I,K,5) = WA4(I-1)*DI5+WA4(I)*DR5                                 
  103    CONTINUE                                                               
  104 CONTINUE                                                                  
      RETURN                                                                    
      END                                                                       
c     include 'routines/tdfftg.f'
      subroutine twodfftg(nk,nl,npk,npl,x,wsavek,wsavel,ww)
c 2D complex forward FFT
c
c x(i,j)= sum k=1,nk sum l=1,nl  x(k,l)*exp(-I*(i-1)*(k-1)*2*pi/nk)*exp(-I*(j-1)*(l-1)*2*pi/nl)
c
c nk - k dimensions of FFT
c nl - l dimensions of FFT
c npk- k storage space for array
c npl- l storage space for array
c x  - complex data array (nk,nl)
c wsavek(4*nk+15)- storage space for FFT of length nk output of  CFFTI(NK,WSAVEK)
c wsavek(4*nl+15)- storage space for FFT of length nl output of  CFFTI(NL,WSAVEL)
c ww(n)- storage space
      complex x(npk,npl), ww(*)
      dimension wsavek(*), wsavel(*)
c fft of columns
      iw1=nk+nk+1
      iw2=iw1+nk+nk
      do 10 l=1,nl
      call cfftf1(nk,x(1,l),wsavek,wsavek(iw1),wsavek(iw2))
 10   continue

c fft of rows
      iw1=nl+nl+1
      iw2=iw1+nl+nl
      do 20 k=1,nk
      do 30 i=1,nl
      ww(i)=x(k,i)
  30  continue
      call cfftf1(nl,ww,wsavel,wsavel(iw1),wsavel(iw2))
      do 40 j=1,nl
      x(k,j)=ww(j)
  40  continue
  20  continue
      return
      end

      subroutine twodfftbg(nk,nl,npk,npl,x,wsavek,wsavel,ww)
c 2D complex forward FFT
c
c x(i,j)= sum k=1,nk sum l=1,nl  x(k,l)*exp(I*(i-1)*(k-1)*2*pi/nk)*exp(I*(j-1)*(l-1)*2*pi/nl)
c
c nk - k dimensions of FFT
c nl - l dimensions of FFT
c npk- k storage space for array
c npl- l storage space for array
c x  - complex data array (nk,nl)
c wsavek(4*nk+15)- storage space for FFT of length nk output of  CFFTI(NK,WSAVEK)
c wsavek(4*nl+15)- storage space for FFT of length nl output of  CFFTI(NL,WSAVEL)
c ww(n)- storage space
      complex x(npk,npl), ww(*)
      dimension wsavek(*), wsavel(*)
c fft of columns
      iw1=nk+nk+1
      iw2=iw1+nk+nk
      do 10 l=1,nl
      call cfftb1(nk,x(1,l),wsavek,wsavek(iw1),wsavek(iw2))
 10   continue

c fft of rows
      iw1=nl+nl+1
      iw2=iw1+nl+nl
      do 20 k=1,nk
      do 30 i=1,nl
      ww(i)=x(k,i)
  30  continue
      call cfftb1(nl,ww,wsavel,wsavel(iw1),wsavel(iw2))
      do 40 j=1,nl
      x(k,j)=ww(j)
  40  continue
  20  continue
      return
      end
c     include 'routines/RKBESL.f'
      SUBROUTINE RKBESL(X,ALPHA,NB,IZE,BK,NCALC)
C-------------------------------------------------------------------
C
C  This FORTRAN 77 routine calculates modified Bessel functions
C  of the second kind, K SUB(N+ALPHA) (X), for non-negative
C  argument X, and non-negative order N+ALPHA, with or without
C  exponential scaling.
C
C  Explanation of variables in the calling sequence
C
C  Description of output values ..
C
C X     - Working precision non-negative real argument for which
C         K's or exponentially scaled K's (K*EXP(X))
C         are to be calculated.  If K's are to be calculated,
C         X must not be greater than XMAX (see below).
C ALPHA - Working precision fractional part of order for which 
C         K's or exponentially scaled K's (K*EXP(X)) are
C         to be calculated.  0 .LE. ALPHA .LT. 1.0.
C NB    - Integer number of functions to be calculated, NB .GT. 0.
C         The first function calculated is of order ALPHA, and the 
C         last is of order (NB - 1 + ALPHA).
C IZE   - Integer type.  IZE = 1 if unscaled K's are to be calculated,
C         and 2 if exponentially scaled K's are to be calculated.
C BK    - Working precision output vector of length NB.  If the
C         routine terminates normally (NCALC=NB), the vector BK
C         contains the functions K(ALPHA,X), ... , K(NB-1+ALPHA,X),
C         or the corresponding exponentially scaled functions.
C         If (0 .LT. NCALC .LT. NB), BK(I) contains correct function
C         values for I .LE. NCALC, and contains the ratios
C         K(ALPHA+I-1,X)/K(ALPHA+I-2,X) for the rest of the array.
C NCALC - Integer output variable indicating possible errors.
C         Before using the vector BK, the user should check that 
C         NCALC=NB, i.e., all orders have been calculated to
C         the desired accuracy.  See error returns below.
C
C
C*******************************************************************
C*******************************************************************
C
C Explanation of machine-dependent constants
C
C   beta   = Radix for the floating-point system
C   minexp = Smallest representable power of beta
C   maxexp = Smallest power of beta that overflows
C   EPS    = The smallest positive floating-point number such that 
C            1.0+EPS .GT. 1.0
C   XMAX   = Upper limit on the magnitude of X when IZE=1;  Solution 
C            to equation:
C               W(X) * (1-1/8X+9/128X**2) = beta**minexp
C            where  W(X) = EXP(-X)*SQRT(PI/2X)
C   SQXMIN = Square root of beta**minexp
C   XINF   = Largest positive machine number; approximately
C            beta**maxexp
C   XMIN   = Smallest positive machine number; approximately
C            beta**minexp
C
C
C     Approximate values for some important machines are:
C
C                          beta       minexp      maxexp      EPS
C
C  CRAY-1        (S.P.)      2        -8193        8191    7.11E-15
C  Cyber 180/185 
C    under NOS   (S.P.)      2         -975        1070    3.55E-15
C  IEEE (IBM/XT,
C    SUN, etc.)  (S.P.)      2         -126         128    1.19E-7
C  IEEE (IBM/XT,
C    SUN, etc.)  (D.P.)      2        -1022        1024    2.22D-16
C  IBM 3033      (D.P.)     16          -65          63    2.22D-16
C  VAX           (S.P.)      2         -128         127    5.96E-8
C  VAX D-Format  (D.P.)      2         -128         127    1.39D-17
C  VAX G-Format  (D.P.)      2        -1024        1023    1.11D-16
C
C
C                         SQXMIN       XINF        XMIN      XMAX
C
C CRAY-1        (S.P.)  6.77E-1234  5.45E+2465  4.59E-2467 5674.858
C Cyber 180/855
C   under NOS   (S.P.)  1.77E-147   1.26E+322   3.14E-294   672.788
C IEEE (IBM/XT,
C   SUN, etc.)  (S.P.)  1.08E-19    3.40E+38    1.18E-38     85.337
C IEEE (IBM/XT,
C   SUN, etc.)  (D.P.)  1.49D-154   1.79D+308   2.23D-308   705.342
C IBM 3033      (D.P.)  7.35D-40    7.23D+75    5.40D-79    177.852
C VAX           (S.P.)  5.42E-20    1.70E+38    2.94E-39     86.715
C VAX D-Format  (D.P.)  5.42D-20    1.70D+38    2.94D-39     86.715
C VAX G-Format  (D.P.)  7.46D-155   8.98D+307   5.57D-309   706.728
C
C*******************************************************************
C*******************************************************************
C
C Error returns
C
C  In case of an error, NCALC .NE. NB, and not all K's are
C  calculated to the desired accuracy.
C
C  NCALC .LT. -1:  An argument is out of range. For example,
C       NB .LE. 0, IZE is not 1 or 2, or IZE=1 and ABS(X) .GE.
C       XMAX.  In this case, the B-vector is not calculated,
C       and NCALC is set to MIN0(NB,0)-2  so that NCALC .NE. NB.
C  NCALC = -1:  Either  K(ALPHA,X) .GE. XINF  or
C       K(ALPHA+NB-1,X)/K(ALPHA+NB-2,X) .GE. XINF.  In this case,
C       the B-vector is not calculated.  Note that again 
C       NCALC .NE. NB.
C
C  0 .LT. NCALC .LT. NB: Not all requested function values could
C       be calculated accurately.  BK(I) contains correct function
C       values for I .LE. NCALC, and contains the ratios
C       K(ALPHA+I-1,X)/K(ALPHA+I-2,X) for the rest of the array.
C
C
C Intrinsic functions required are:
C
C     ABS, AINT, EXP, INT, LOG, MAX, MIN, SINH, SQRT
C
C
C Acknowledgement
C
C  This program is based on a program written by J. B. Campbell
C  (2) that computes values of the Bessel functions K of real
C  argument and real order.  Modifications include the addition
C  of non-scaled functions, parameterization of machine
C  dependencies, and the use of more accurate approximations
C  for SINH and SIN.
C
C References: "On Temme's Algorithm for the Modified Bessel
C              Functions of the Third Kind," Campbell, J. B.,
C              TOMS 6(4), Dec. 1980, pp. 581-586.
C
C             "A FORTRAN IV Subroutine for the Modified Bessel
C              Functions of the Third Kind of Real Order and Real
C              Argument," Campbell, J. B., Report NRC/ERB-925,
C              National Research Council, Canada.
C
C  Latest modification: May 30, 1989
C
C  Modified by: W. J. Cody and L. Stoltz
C               Applied Mathematics Division
C               Argonne National Laboratory
C               Argonne, IL  60439
C
C-------------------------------------------------------------------
      INTEGER I,IEND,ITEMP,IZE,J,K,M,MPLUS1,NB,NCALC
      REAL
CD    DOUBLE PRECISION  
     1    A,ALPHA,BLPHA,BK,BK1,BK2,C,D,DM,D1,D2,D3,ENU,EPS,ESTF,ESTM,
     2    EX,FOUR,F0,F1,F2,HALF,ONE,P,P0,Q,Q0,R,RATIO,S,SQXMIN,T,TINYX,
     3    TWO,TWONU,TWOX,T1,T2,WMINF,X,XINF,XMAX,XMIN,X2BY4,ZERO
      DIMENSION BK(1),P(8),Q(7),R(5),S(4),T(6),ESTM(6),ESTF(7)
C---------------------------------------------------------------------
C  Mathematical constants
C    A = LOG(2.D0) - Euler's constant
C    D = SQRT(2.D0/PI)
C---------------------------------------------------------------------
      DATA HALF,ONE,TWO,ZERO/0.5E0,1.0E0,2.0E0,0.0E0/
      DATA FOUR,TINYX/4.0E0,1.0E-10/
      DATA A/ 0.11593151565841244881E0/,D/0.797884560802865364E0/
CD    DATA HALF,ONE,TWO,ZERO/0.5D0,1.0D0,2.0D0,0.0D0/
CD    DATA FOUR,TINYX/4.0D0,1.0D-10/
CD    DATA A/ 0.11593151565841244881D0/,D/0.797884560802865364D0/
C---------------------------------------------------------------------
C  Machine dependent parameters
C---------------------------------------------------------------------
      DATA EPS/1.19E-7/,SQXMIN/1.08E-19/,XINF/3.40E+38/
      DATA XMIN/1.18E-38/,XMAX/85.337E0/
CD    DATA EPS/2.22D-16/,SQXMIN/1.49D-154/,XINF/1.79D+308/
CD    DATA XMIN/2.23D-308/,XMAX/705.342D0/
C---------------------------------------------------------------------
C  P, Q - Approximation for LOG(GAMMA(1+ALPHA))/ALPHA
C                                         + Euler's constant
C         Coefficients converted from hex to decimal and modified
C         by W. J. Cody, 2/26/82
C  R, S - Approximation for (1-ALPHA*PI/SIN(ALPHA*PI))/(2.D0*ALPHA)
C  T    - Approximation for SINH(Y)/Y
C---------------------------------------------------------------------
      DATA P/ 0.805629875690432845E00,    0.204045500205365151E02,
     1        0.157705605106676174E03,    0.536671116469207504E03,
     2        0.900382759291288778E03,    0.730923886650660393E03,
     3        0.229299301509425145E03,    0.822467033424113231E00/
      DATA Q/ 0.294601986247850434E02,    0.277577868510221208E03,
     1        0.120670325591027438E04,    0.276291444159791519E04,
     2        0.344374050506564618E04,    0.221063190113378647E04,
     3        0.572267338359892221E03/
      DATA R/-0.48672575865218401848E+0,  0.13079485869097804016E+2,
     1       -0.10196490580880537526E+3,  0.34765409106507813131E+3,
     2        0.34958981245219347820E-3/
      DATA S/-0.25579105509976461286E+2,  0.21257260432226544008E+3,
     1       -0.61069018684944109624E+3,  0.42269668805777760407E+3/
      DATA T/ 0.16125990452916363814E-9, 0.25051878502858255354E-7,
     1        0.27557319615147964774E-5, 0.19841269840928373686E-3,
     2        0.83333333333334751799E-2, 0.16666666666666666446E+0/
      DATA ESTM/5.20583E1, 5.7607E0, 2.7782E0, 1.44303E1, 1.853004E2,
     1          9.3715E0/
      DATA ESTF/4.18341E1, 7.1075E0, 6.4306E0, 4.25110E1, 1.35633E0,
     1          8.45096E1, 2.0E1/
CD    DATA P/ 0.805629875690432845D00,    0.204045500205365151D02,
CD   1        0.157705605106676174D03,    0.536671116469207504D03,
CD   2        0.900382759291288778D03,    0.730923886650660393D03,
CD   3        0.229299301509425145D03,    0.822467033424113231D00/
CD    DATA Q/ 0.294601986247850434D02,    0.277577868510221208D03,
CD   1        0.120670325591027438D04,    0.276291444159791519D04,
CD   2        0.344374050506564618D04,    0.221063190113378647D04,
CD   3        0.572267338359892221D03/
CD    DATA R/-0.48672575865218401848D+0,  0.13079485869097804016D+2,
CD   1       -0.10196490580880537526D+3,  0.34765409106507813131D+3,
CD   2        0.34958981245219347820D-3/
CD    DATA S/-0.25579105509976461286D+2,  0.21257260432226544008D+3,
CD   1       -0.61069018684944109624D+3,  0.42269668805777760407D+3/
CD    DATA T/ 0.16125990452916363814D-9, 0.25051878502858255354D-7,
CD   1        0.27557319615147964774D-5, 0.19841269840928373686D-3,
CD   2        0.83333333333334751799D-2, 0.16666666666666666446D+0/
CD    DATA ESTM/5.20583D1, 5.7607D0, 2.7782D0, 1.44303D1, 1.853004D2,
CD   1          9.3715D0/
CD    DATA ESTF/4.18341D1, 7.1075D0, 6.4306D0, 4.25110D1, 1.35633D0,
CD   1          8.45096D1, 2.0D1/
C---------------------------------------------------------------------
      EX = X
      ENU = ALPHA
      NCALC = MIN(NB,0)-2
      IF ((NB .GT. 0) .AND. ((ENU .GE. ZERO) .AND. (ENU .LT. ONE))
     1     .AND. ((IZE .GE. 1) .AND. (IZE .LE. 2)) .AND.
     2     ((IZE .NE. 1) .OR. (EX .LE. XMAX)) .AND.
     3     (EX .GT. ZERO))  THEN
            K = 0
            IF (ENU .LT. SQXMIN) ENU = ZERO
            IF (ENU .GT. HALF) THEN
                  K = 1
                  ENU = ENU - ONE
            END IF
            TWONU = ENU+ENU
            IEND = NB+K-1
            C = ENU*ENU
            D3 = -C
            IF (EX .LE. ONE) THEN
C---------------------------------------------------------------------
C  Calculation of P0 = GAMMA(1+ALPHA) * (2/X)**ALPHA
C                 Q0 = GAMMA(1-ALPHA) * (X/2)**ALPHA
C---------------------------------------------------------------------
                  D1 = ZERO
                  D2 = P(1)
                  T1 = ONE
                  T2 = Q(1)
                  DO 10 I = 2,7,2
                     D1 = C*D1+P(I)
                     D2 = C*D2+P(I+1)
                     T1 = C*T1+Q(I)
                     T2 = C*T2+Q(I+1)
   10             CONTINUE
                  D1 = ENU*D1
                  T1 = ENU*T1
                  F1 = LOG(EX)
                  F0 = A+ENU*(P(8)-ENU*(D1+D2)/(T1+T2))-F1
                  Q0 = EXP(-ENU*(A-ENU*(P(8)+ENU*(D1-D2)/(T1-T2))-F1))
                  F1 = ENU*F0
                  P0 = EXP(F1)
C---------------------------------------------------------------------
C  Calculation of F0 = 
C---------------------------------------------------------------------
                  D1 = R(5)
                  T1 = ONE
                  DO 20 I = 1,4
                     D1 = C*D1+R(I)
                     T1 = C*T1+S(I)
   20             CONTINUE
                  IF (ABS(F1) .LE. HALF) THEN
                        F1 = F1*F1
                        D2 = ZERO
                        DO 30 I = 1,6
                           D2 = F1*D2+T(I)
   30                   CONTINUE
                        D2 = F0+F0*F1*D2
                     ELSE
                        D2 = SINH(F1)/ENU
                  END IF
                  F0 = D2-ENU*D1/(T1*P0)
                  IF (EX .LE. TINYX) THEN
C--------------------------------------------------------------------
C  X.LE.1.0E-10
C  Calculation of K(ALPHA,X) and X*K(ALPHA+1,X)/K(ALPHA,X)
C--------------------------------------------------------------------
                        BK(1) = F0+EX*F0
                        IF (IZE .EQ. 1) BK(1) = BK(1)-EX*BK(1)
                        RATIO = P0/F0
                        C = EX*XINF
                        IF (K .NE. 0) THEN
C--------------------------------------------------------------------
C  Calculation of K(ALPHA,X) and X*K(ALPHA+1,X)/K(ALPHA,X),
C  ALPHA .GE. 1/2
C--------------------------------------------------------------------
                              NCALC = -1
                              IF (BK(1) .GE. C/RATIO) GO TO 500
                              BK(1) = RATIO*BK(1)/EX
                              TWONU = TWONU+TWO
                              RATIO = TWONU
                        END IF
                        NCALC = 1
                        IF (NB .EQ. 1) GO TO 500
C--------------------------------------------------------------------
C  Calculate  K(ALPHA+L,X)/K(ALPHA+L-1,X),  L  =  1, 2, ... , NB-1
C--------------------------------------------------------------------
                        NCALC = -1
                        DO 80 I = 2,NB
                           IF (RATIO .GE. C) GO TO 500
                           BK(I) = RATIO/EX
                           TWONU = TWONU+TWO
                           RATIO = TWONU
   80                   CONTINUE
                        NCALC = 1
                        GO TO 420
                     ELSE
C--------------------------------------------------------------------
C  1.0E-10 .LT. X .LE. 1.0
C--------------------------------------------------------------------
                        C = ONE
                        X2BY4 = EX*EX/FOUR
                        P0 = HALF*P0
                        Q0 = HALF*Q0
                        D1 = -ONE
                        D2 = ZERO
                        BK1 = ZERO
                        BK2 = ZERO
                        F1 = F0
                        F2 = P0
  100                   D1 = D1+TWO
                        D2 = D2+ONE
                        D3 = D1+D3
                        C = X2BY4*C/D2
                        F0 = (D2*F0+P0+Q0)/D3
                        P0 = P0/(D2-ENU)
                        Q0 = Q0/(D2+ENU)
                        T1 = C*F0
                        T2 = C*(P0-D2*F0)
                        BK1 = BK1+T1
                        BK2 = BK2+T2
                        IF ((ABS(T1/(F1+BK1)) .GT. EPS) .OR.
     1                     (ABS(T2/(F2+BK2)) .GT. EPS))  GO TO 100
                        BK1 = F1+BK1
                        BK2 = TWO*(F2+BK2)/EX
                        IF (IZE .EQ. 2) THEN
                              D1 = EXP(EX)
                              BK1 = BK1*D1
                              BK2 = BK2*D1
                        END IF
                        WMINF = ESTF(1)*EX+ESTF(2)
                  END IF
               ELSE IF (EPS*EX .GT. ONE) THEN
C--------------------------------------------------------------------
C  X .GT. ONE/EPS
C--------------------------------------------------------------------
                  NCALC = NB
                  BK1 = ONE / (D*SQRT(EX))
                  DO 110 I = 1, NB
                     BK(I) = BK1
  110             CONTINUE
                  GO TO 500
               ELSE
C--------------------------------------------------------------------
C  X .GT. 1.0
C--------------------------------------------------------------------
                  TWOX = EX+EX
                  BLPHA = ZERO
                  RATIO = ZERO
                  IF (EX .LE. FOUR) THEN
C--------------------------------------------------------------------
C  Calculation of K(ALPHA+1,X)/K(ALPHA,X),  1.0 .LE. X .LE. 4.0
C--------------------------------------------------------------------
                        D2 = AINT(ESTM(1)/EX+ESTM(2))
                        M = INT(D2)
                        D1 = D2+D2
                        D2 = D2-HALF
                        D2 = D2*D2
                        DO 120 I = 2,M
                           D1 = D1-TWO
                           D2 = D2-D1
                           RATIO = (D3+D2)/(TWOX+D1-RATIO)
  120                   CONTINUE
C--------------------------------------------------------------------
C  Calculation of I(|ALPHA|,X) and I(|ALPHA|+1,X) by backward
C    recurrence and K(ALPHA,X) from the wronskian
C--------------------------------------------------------------------
                        D2 = AINT(ESTM(3)*EX+ESTM(4))
                        M = INT(D2)
                        C = ABS(ENU)
                        D3 = C+C
                        D1 = D3-ONE
                        F1 = XMIN
                        F0 = (TWO*(C+D2)/EX+HALF*EX/(C+D2+ONE))*XMIN
                        DO 130 I = 3,M
                           D2 = D2-ONE
                           F2 = (D3+D2+D2)*F0
                           BLPHA = (ONE+D1/D2)*(F2+BLPHA)
                           F2 = F2/EX+F1
                           F1 = F0
                           F0 = F2
  130                   CONTINUE
                        F1 = (D3+TWO)*F0/EX+F1
                        D1 = ZERO
                        T1 = ONE
                        DO 140 I = 1,7
                           D1 = C*D1+P(I)
                           T1 = C*T1+Q(I)
  140                   CONTINUE
                        P0 = EXP(C*(A+C*(P(8)-C*D1/T1)-LOG(EX)))/EX
                        F2 = (C+HALF-RATIO)*F1/EX
                        BK1 = P0+(D3*F0-F2+F0+BLPHA)/(F2+F1+F0)*P0
                        IF (IZE .EQ. 1) BK1 = BK1*EXP(-EX)
                        WMINF = ESTF(3)*EX+ESTF(4)
                     ELSE
C--------------------------------------------------------------------
C  Calculation of K(ALPHA,X) and K(ALPHA+1,X)/K(ALPHA,X), by backward
C  recurrence, for  X .GT. 4.0
C--------------------------------------------------------------------
                        DM = AINT(ESTM(5)/EX+ESTM(6))
                        M = INT(DM)
                        D2 = DM-HALF
                        D2 = D2*D2
                        D1 = DM+DM
                        DO 160 I = 2,M
                           DM = DM-ONE
                           D1 = D1-TWO
                           D2 = D2-D1
                           RATIO = (D3+D2)/(TWOX+D1-RATIO)
                           BLPHA = (RATIO+RATIO*BLPHA)/DM
  160                   CONTINUE
                        BK1 = ONE/((D+D*BLPHA)*SQRT(EX))
                        IF (IZE .EQ. 1) BK1 = BK1*EXP(-EX)
                        WMINF = ESTF(5)*(EX-ABS(EX-ESTF(7)))+ESTF(6)
                  END IF
C--------------------------------------------------------------------
C  Calculation of K(ALPHA+1,X) from K(ALPHA,X) and
C    K(ALPHA+1,X)/K(ALPHA,X)
C--------------------------------------------------------------------
                  BK2 = BK1+BK1*(ENU+HALF-RATIO)/EX
            END IF
C--------------------------------------------------------------------
C  Calculation of 'NCALC', K(ALPHA+I,X), I  =  0, 1, ... , NCALC-1,
C  K(ALPHA+I,X)/K(ALPHA+I-1,X), I  =  NCALC, NCALC+1, ... , NB-1
C--------------------------------------------------------------------
            NCALC = NB
            BK(1) = BK1
            IF (IEND .EQ. 0) GO TO 500
            J = 2-K
            IF (J .GT. 0) BK(J) = BK2
            IF (IEND .EQ. 1) GO TO 500
            M = MIN(INT(WMINF-ENU),IEND)
            DO 190 I = 2,M
               T1 = BK1
               BK1 = BK2
               TWONU = TWONU+TWO
               IF (EX .LT. ONE) THEN
                     IF (BK1 .GE. (XINF/TWONU)*EX) GO TO 195
                     GO TO 187
                  ELSE 
                     IF (BK1/EX .GE. XINF/TWONU) GO TO 195
               END IF
  187          CONTINUE
               BK2 = TWONU/EX*BK1+T1
               ITEMP = I
               J = J+1
               IF (J .GT. 0) BK(J) = BK2
  190       CONTINUE
  195       M = ITEMP
            IF (M .EQ. IEND) GO TO 500
            RATIO = BK2/BK1
            MPLUS1 = M+1
            NCALC = -1
            DO 410 I = MPLUS1,IEND
               TWONU = TWONU+TWO
               RATIO = TWONU/EX+ONE/RATIO
               J = J+1
               IF (J .GT. 1) THEN
                     BK(J) = RATIO
                  ELSE
                     IF (BK2 .GE. XINF/RATIO) GO TO 500
                     BK2 = RATIO*BK2
               END IF
  410       CONTINUE
            NCALC = MAX(MPLUS1-K,1)
            IF (NCALC .EQ. 1) BK(1) = BK2
            IF (NB .EQ. 1) GO TO 500
  420       J = NCALC+1
            DO 430 I = J,NB
               IF (BK(NCALC) .GE. XINF/BK(I)) GO TO 500
               BK(I) = BK(NCALC)*BK(I)
               NCALC = I
  430       CONTINUE
      END IF
  500 RETURN
C---------- Last line of RKBESL ----------
      END


