        subroutine kdpchill(corr_height)
        real corr_height(-5:1900)
 
c Add to clean Zdr and Rohv using the same criterion as for Kdp. Li Liu, 9/21/95
c Add to integrate Phidp (PDPder) from Kdp derived from Zh & Zdr. Li Liu, 10/18/95
c Modified by Yanting Wang 2000-01-08 yt_wang@hotmail.com
c Adapted from S-band to C-band by Bringi Feb 02
c   Log: Check test pulse by SNR and SD_PHIDP
c   Log: Connect good data region by slant lines.
c   Log: Arrange codes in std style and modify some codes(see inline comments)
c   Log: Combine attenuation correction & rain estimation from Dr. Bringi's version
c   Log: Speed the ZDR correction by eliminating the loop
c   Log: Trace hail region back in a multicells store for ZDR correction
c   Log: modified for CRL COBRA radar in Okinawa  Bringi Oct 2003
c   Log: modified for CRL COBRA data in Okinawa   Bringi Aug 2004
c        phidp now increases with range in rain
c        rhohv is now 'solid' so re introducind the rhv threshold
c        which was 'dropped' earlier 
c        changed call to basic atten correction using filtered phipd
c        rather than using May's integrated phidp to avoid problem
c        when its raining hard on radome...meas phidp is not right
c        changed first good data search to after 5 km (old near field was 
c        set at 1.5 km. This means that atten due to rain cell at < 5 km range
c        won't be accounted for.
c        changed unwrap amount from 180 deg (for swithced system) to 360 for
c        slant 45 trans (or hybrid mode)...seems like all COBRA data is in slant
c        45 deg mode. Bringi Aug 24. 2004. 
c        initialized plfile(i,41)=0 in this version...Bringi Aug 2004
c
cc        found that some data sets from 2005/06 had very poor rhohv data quality but phipd was OK
c        instead of altering the code one can simply set the variable thrcrr to a very
c        low value say 0.25 for Darwin which has been done
c        below...MUST change back to thrcrr=0.8 if rhohv data is good
c        getting rid of SNR threshold of 3 dB since I don't know precisely the radar constant
c        for CPOL plus its correlated with std dev of phidp Oct 2007

        integer firsttime,count,igood_data(-5:1900)
        integer ibegin_arr(20),iend_arr(20)
        integer irange,malpha,mopt
        real x(-90:1900),y(-90:1900),z(-200:1900),z_cns(1900)
        real xx(31),yy(31)
        real init0,init,thres,kdp,meanzdr_corr
        real Nw_testud
        real r1,r2,rr1,rr2,meanzdr,Ir1r0,meanzh_corr,No_testud
        real mean_HDR,mean_kdp 
        real kdp_orig(1900),AhAv_Sm(1900),zh_lin(1900),kdp_test(1900)
        real atten(1900,23),phidp_test(1900,23),gamma(23),sumerror(23)
        real error(1900,23),phidpn(1900,23),temp(23),valuen(1900,23)
        real zhlinn(1900,23),value(1900),phidp(1900)
        real r_begin_arr(20),r_end_arr(20)
        real diff_Zh,phinit0

c        common /SUN/ suncal,beta,offsetPB,inp,iflag_loc

        common /SUN/suncal,beta,malpha,mopt,gamma,phinit0,iflag_loc
        common/kdp_testud/xxmin_test,xxmax_test,itestud,iturnon,
     &               ibegin1,iend1,irange
        real*4 suncal(4),reduce_wt(20),increase_wt(20)
c       Add for Cons Kdp
        real phi_u(1900), phi_i(1900), kdp_c(1900)

        include 'comblk.h'
        data reduce_wt/.95,.9,.85,.8,.75,.7,.65,.6,.55,.5,.45,.4
     &  ,.35,.3,.25,.2,.15,.1,.05,0./
        data increase_wt/1.05,1.1,1.15,1.2,1.25,1.3,1.35,1.4,1.45
     &  ,1.5,1.55,1.6,1.65,1.7,1.75,1.8,1.85,1.9,1.95,2.0/
c 
c     gamma array defined in pltcpol_testud.f (main program)
c      data gamma/.04,.045,.05,.055,.06,.065,.07,.075,.08,.085,
c     & .09,.095,.1,.105,.11,.115,.12,.125,.13,.135,.14,.145,.15/


c Set in pltcpol_testud.f now 
c        malpha = 23    !loop for ZH atten corr
c        mopt = 9       !optimal value for gamma
c        phinit0 = -999 !automatically decide initial phase
c-------------------------

        nad1 = 31          ! label(31)='Pdpadap'
        nfl  = 32          ! label(32)='Pdpadfl'
        nad2 = 33          ! label(33)='Kdpadap'
        ndf  = 34          ! label(34)='Delta'
        iSNR = 11
        thres = 3.         ! number of degrees of deviation by which the filter checks
c       madflt = 2
c reducing madflt to 10 to speed up, 20 seems like overkill

c        madflt = 20        !severe convection over land

         madflt=10 ! max number of filtering iterations to filter phidp
c
c       thrreshold based on Cpol data 
c       reduce threshold for COBRA since Klystron
c       trial of 12 deg
c*********************************
c         thrs_phi=12   !     threshold for sd_dev of phidp
c       for King City radar trial 
c
c
c          thrs_phi=20   ! increased for C-pol    magnetron


c checked 10 sweeos of CPOL data (1 hr) to get histogram of phidp
c for CPOL from Jan 05 (build up event) and Jan 22, 2006 (monsoon rain).
c the mean and sigma for Jan 5 was [5.45,4] and for Jan 22 it was [3.5,3.12]
c setting threshold to ~ mean+2*sigma which lies in the inteval [13.5,10]
c for the two cases: thus taking the mean of the 2 choosing thrs_phi for
c CPOL to be 12 deg 5 Dec 2007 fro WG

           thrs_phi=12   ! CPOL 2005-06 rain season (XXXX 6 deg for CP2)

c Note: iflag_loc gives location flag
c       1: Darwin data  2: SCSMEX data
c Note: thrcrr is the rhohv threshold
c       htL/htU are the lower and upper heights
c       defining the melting layer , in km
c       alpha is defined by Ah=alpha*Kdp
c       beta is defined by  Ah-Av=beta*Kdp
c       The settings below will override the
c       default values set in plt.f main menu.

        if(iflag_loc.eq.1) then
c This refers to Darwin
          b_exp=0.76
c          thrcrr=0.8
c set low to handle some datsets from Darwin that had very poor rhohv data

c 2005/2006...don't know how many such datasets were affected.
c
          thrcrr=0.25 ! (XXXX threshold for rhohv)

c increased htL for Darwin from 3 to 3.5 km for
c WG applications
          htL=3.5
          htU=6.
        endif

          if(iflag_loc.eq.2) then
c This refers to SCSMX or Okinawa (similar)
c note: for COBRA not using rhh threshold for good data mask
c note: setting thrcrr will override threshcorr in option 9
c       of main plt program!! Bringi 10/18/04
           b_exp=0.7862
           thrcrr=0.9
           htL=4.
           htU=6.
          endif

c NOTE: hard wiring rhv threshold as above
c       will mean cannot change threshcorr in option 9
c       of main PLT program!! Bringi 10/18/04


c       The alpha and beta are valid at 5.5 GHz
c       beta=0.016 can be set in main menu but hardwired here
c       alpha=0.08

        beta=0.016
        alpha=gamma(mopt)
        print *,'adpm,lp,afa,bta,idx:',madflt,malpha,alpha,beta,mopt


c       removing pre filter
c       Pre filter raw Zh and raw Zv and then put in Zdr array
c       removing pre filter for checking Bringi 8/20/04
c        call ADflt('IIR5 ',1,1)
c        call ADflt('IIR5 ',7,7)
c        do i=1,length
cc
ccccc NOTE: zdrbias is included in ufread program
ccccc       else, must be included here
cc
c           plfile(i,2)=plfile(i,1)-plfile(i,7)
c        enddo
c

c XXXX need following

        inp = 3
        ikdpst = 5
        firsttime = 1
        mgood     = 10       ! number of good data to enter cell
        mbad      = 5        ! number of bad data to exit cell
        count     = 0
        kbegin    = length
        iend      = length
        loop      = 1
 
c XXXX i: gate number
c XXXX i: gate number
c XXXX length = number of gates

        do i=-5,length
           plfile(i,nad1)=0.0   ! preset Pdpadap XXXX
           plfile(i,nfl)=0.0    ! preset Pdpadfl XXXX
           plfile(i,nad2)=0.0   ! preset Kdpadap XXXX
           plfile(i,ndf)=0.0    ! preset Delta   XXXX
           plfile(i,16)=-100.
           plfile(i,25)=0.0     ! preset PDP derived from Testud
           plfile(i,48)=0.0     ! preset gamma_opt
           plfile(i,23)=0.      ! preset beta:axis ratio,Gorgucci
           plfile(i,14)=0.      ! preset Do, Gorgucci
           plfile(i,50)=0.      ! preset Nw, Gorgucci
c           plfile(i,24)=0.      ! preset R(Zh,Zdr,beta) Gorgucci
c           plfile(i,17)=0.0     ! preset R(Kdp,Zdr) Gorgucci
c           plfile(i,26)=0.0     ! preset R(Kdp) Gorgucci
           plfile(i,60)=0.0     ! preset R(Ah,Nw) Testud
           plfile(i,20)=plfile(i,1)     ! preset Zh_corr by Testud Method
           plfile(i,21)=plfile(i,2)     ! preset Zdr_corr by Smythe/Illing method
           plfile(i,27)=0.0     ! preset Ah by Testud Method
           plfile(i,38)=0.0     ! preset Ah-AV by Smythe/Illing method
           plfile(i,6)=0.       ! preset Cons Kdp
           plfile(i,22)=0.0     ! preset integrated phidp from cons Kdp
           plfile(i,34)=-500.   ! preset integrated phidp=f(Zh,Zdr)
           plfile(i,61)=0.0     ! preset smooth/corr zh
           plfile(i,62)=0.0     ! preset smooth/corr zdr
           igood_data(i)=0      ! start from - XXXX ????
           plfile(i,41)=0.0     ! must preset good data flage to 0.0 Bringi 8/16/2004  XXXX data mask
           plfile(i,43)=plfile(i,1) !     preset fixed alpha method of corr Zh
           plfile(i,44)=plfile(i,2) !     preset Adp=betaKdp**gamma method of gate by gate correction
           plfile(i,45)=0.0 ! temporary testing adp values
           plfile(i,46)=0.0 ! temp testing of int_adp
           plfile(i,39)=0   ! preset R from MTs Kdp
           plfile(i,51)=0.0 ! preset Do from MTs corr Zdr
           plfile(i,42)=0.0  ! preset smoothed MTs Zdr 
           plfile(i,52)=0.0 !  preset algo10(Nw) using R and Do from MT
           plfile(i,35)=0  ! testing sd of R in rainrate beta.f
           plfile(i,53)=0.0 ! preset new Do_polynomial for Zdr>-0.5 dB
        enddo


        tempcorr=0.0
        tempfix=0.0
        anitial=0.0 ! XXXX
        avrg2 = 0.0 ! XXXX
        j_arr=0  ! XXXX
        k_arr=0 ! XXXX
        epslon_over=0.1
        epslon_under=-0.1

        
c$$$$$$$ UNWRAP DIFF PHASE $$$$$$$$
        jj_test = 0
        inwrap = 0
        extra = 0
        avgpdp = 0
        
        jcount = 0
        if (phinit0 .ne. -999) then
           ibegin = 1
           avgbg = phinit0
        else
           ibeg = 0
           avgbg = 0
        endif  ! to adjuest the reference phase by observation (ytwang)

c set threshold for unfolding (set nominal as -60 deg)
c but needs to be validated : checked for all RAYs from SCSMEX sent
c by Feng Lei: Bringi 2/14/02
c increasing thrs_unfold to 90 for testing of CPOL 2005-2006 data

           thrs_unfold=90


c This is Yanting's modified unfold code 
        do 8 jj = 1,length

          plfile(jj,63) = plfile(jj,3)  ! save the raw phidp to checking unfold



c************************************************************
        if(inwrap.eq.1) then
c**********************************
           if(plfile(jj,3).lt.(avgbg-thrs_unfold+extra)) then
c changing unwrap amount from 180 to 360 for COBRA slant 45 trans
c Bringi 8/24/04
c 9/11/07 checking what to add for King City Radar based on May 15 2007 data
c seems like should add 180 and not 360 for King City radar eventhough its
c slant 45 system.
c OK checked it should be 180 deg for King City radar!! 

              plfile(jj,3)=plfile(jj,3)+180.
              print*,'ran/phi/extra::',plfile(jj,0),plfile(jj,3),extra
c               plfile(jj,3)=plfile(jj,3)+360.

           endif
c**********************************
              do jjj = 1,5
                 xx(jjj) = plfile(jj-jjj,0)
                 yy(jjj) = plfile(jj-jjj,3)
              enddo
              call LSE(phimn_slp,bb,xx,yy,5)
c***************
              if(phimn_slp.gt.-15.0.and.phimn_slp.lt.20.) then

                 extra=extra+(plfile(jj,0)-plfile(jj-1,0))*phimn_slp
              endif
c**************
         write(44,*) phimn_slp,extra,plfile(jj,3),
     +                   plfile(jj,63),plfile(jj,0)

c*************************************************************
        else  ! goes with if inwrap ne 1 

c SDEV over 10 gates

           do jjj = 0,mgood-1
              yy(jjj+1) = plfile(jj+jjj,inp)
           enddo
           call msr(phimn,sd_phidp,yy,mgood)

           if(ibegin.eq.1) then
c*********************
              if(sd_phidp.lt.thrs_phi) then 
                 avgpdp=(plfile(jj,3)+plfile(jj+1,3))/2.0
c fine tuning avgpdp-avgbg gt 50  seems like COBRA
c radar system phidp is around 15 deg and phidp folds
c when it reaches 90 deg for some reason...should fold only 
c when phdip reaches 180 deg.
c****************
                 if((avgpdp-avgbg).gt.40.0) then ! incr 40 -> 90 for SBand
                    inwrap=1
           print *,'ran/phi/::inwrap',plfile(jj,0),avgpdp,inwrap
                 endif
c***************
              endif
c**********************
           else
              if(sd_phidp.lt.thrs_phi
     +          .and.plfile(jj,0).ge.1.5) then ! 1.5 km = far field for CP2 antenna
                jcount=jcount+1
                if(jcount.eq.mgood) then
                   sum=0.
                   do ln=0,4
                     sum=sum+plfile(jj-ln,3)
                   enddo
                   avgbg=sum/5.0
                   ibegin=1
                   print *,'BEGIN ran/pbg/::',plfile(jj,0),avgbg
                endif
              else
                jcount=0
              endif   ! this goes with trying to find begin of good data
           endif
        endif
8       continue
c$$$$$$$$$$$$$$$$$$$$$$$$$$$$ - end of unwrap


c XXXX remove this VVVVVVV
c ---------------------------------------------------------------
c      Calculate Consensus Kdp, integrated Phi

c       Find out the gate_spacing for Cons Kdp
c       Set som constant for Cons Kdp
        n = length
        rinc = (plfile(101,0) - plfile(100,0))*1000
        mfl = 11
        nn = 13
        dev =1.5

        do i=1,n
          phi_u(i) = plfile(i,inp)
        enddo

c        call process_kdp(n,rinc,mfl,nn,dev,plfile(1,1),  !z,
c     +                  phi_u,kdp_c,x,phi_i,y)

        do i=1,n
          plfile(i,6) = kdp_c(i)
          plfile(i,22) = phi_i(i)
        enddo
c XXXX remove this ^^^^^^
c ---------------------------------------------------------------


cccc    x(i) -- IIR5 pre-filtered data;
cccc    z(i) -- updated profile;
cccc    y(i) -- filtered profile.
        ibegin = length
        do i=-5,length
           z(i)=0.0
           x(i)=0.0
        enddo

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12
c--------- FIND THE start AND stop BINs FOR FILTERING ---------------

c XXXX remove this VVVVVVV
        plfile(0,iSNR) = plfile(1,iSNR)
c XXXX remove this ^^^^^^

        do 1000 i=1,length

           do jjj = 0,mgood-1
c just for checking sd of Zdr in plfile (i,2)
c             yy(jjj+1) = plfile(i+jjj,2)

              yy(jjj+1) = plfile(i+jjj,inp) ! inp = 3, raw phidp
           enddo
           call msr(phimn,sd_phidp,yy,mgood)
           plfile(i,49) = sd_phidp ! 49 is sdev of phidp

c           if(plfile(i,0).gt.146.5 .or. loop.eq.0) then ! cannot find a robust way
c              if (loop.eq.2) then  ! In good data, seeking bad data
c                 iend = i-1
c                 k_arr=k_arr+1
c                 iend_arr(k_arr)=iend
c                 r_end_arr(k_arr)=plfile(iend,0)
cc                print *,'k_arr',k_arr,iend_arr(k_arr)
c              endif             ! else already in bad data
c              z(i) = avrg2
c              loop = 0
c              goto 1000
c           endif

           SNR = plfile(i,iSNR) ! XXX - don't need
           SNRslevel = 3. ! XXX - don't need
           goto (100,200),loop

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12
c ********** Find begin of GOOD data loop ***********
100        z(i) = avrg2
c
c removing the thrcrr criterion for detecting beginiinig
c of 'good' data...as COBRA rhohv falls rapidly at times 

c re-introduing the thrcrr criterion as COBRA rhohv 
c data from 2004 look quite solid...earlier data was from 2001
c and Dec 2003.. Bringi 8/16/2004
c delaying plfile(i,0) to 5 km for jun 8 COBRA rhi analysis
c Bringi 8/21/04
c changing min range to 1.5 km April 05
c changing min range to 2.5 km for CPOL 11 Dec 2007

           if (plfile(i,ikdpst).ge.thrcrr ! ikdpst = 5 = rhohv
     +     .and.plfile(i,49).lt.thrs_phi. ! 49 is sdev of phidp < 6 deg
     +      and.plfile(i,0).ge.2.5) then ! 0 is range, 2.5 -> 1.5 for CP2

c rhohv is really bad for 21 Jan 2006 KC radar so removing it from
c good data segment
c going back to older thresholds above    3 May 2007 Bringi
 

c            if(SNR.gt.SNRslevel.and.plfile(i,49).lt.thrs_phi.
c     +      and.plfile(i,0).ge.1.5) then

              count = count+1
              if (count.eq.mgood) then ! mgood gates = 10
                 do  il = 0, mgood-1
                    z(i-il) = plfile(i-il,inp) ! inp = 3 = phidp
                    plfile(i-il,16) = plfile(i-il,2) ! preserve good Zdr
                    zdr_test=plfile(i-il,2)
                    zh_test=plfile(i-il,1)
                 enddo

                 if (firsttime.eq.1) then
c increasing ibegin for COBRA only as it appears thatphipd is
c highly fluctuating from 0 to 8 km at times   Bringi 10/30/03
c going back to CPOL 
                    ibegin   = i-mgood+1 ! begin of the 1st encountered cell
                    j_arr=j_arr+1
                    ibegin_arr(j_arr)=ibegin
                    r_begin_arr(j_arr)=plfile(ibegin,0) ! ibegin = range of first good gate
c                   print *,'j_arr',j_arr,ibegin_arr(j_arr)
                    init0=(z(ibegin)+z(ibegin+1)+ ! z is phidp
     +              z(ibegin+2)+z(ibegin+3))/4. ! average 4 phidps together to get good value
                    init = init0    ! Recorded for local trend

                    flip   = 0.
                    delavg = init0 - phinit0

                 else
                    mc = i-mgood+1  ! begin of the successive encountered cells
                    j_arr=j_arr+1
                    ibegin_arr(j_arr)=mc
                    r_begin_arr(j_arr)=plfile(mc,0)
c                   print *,'j_arr',j_arr,ibegin_arr(j_arr)
                    avrg1 = (z(mc+1)+z(mc+2)+z(mc+3)+z(mc+4))/4.
                    delavg = avrg1-avrg2

                 endif

c                deal with the case the first good_data need flip (ytwang)
                 if (phinit0.ne.-999 .or. firsttime.eq.0) then

                    if (abs(delavg).gt.65.) then
            print *,'(ytwang) delavg:: ',delavg,' [',iend,'-',mc,']'
                       if (delavg.gt.250.) then
                          flip = -360.
                       elseif (delavg.lt.-250) then
                          flip = 360.
                       elseif (delavg.gt.140.) then
                          flip = -180.
                       elseif (delavg.lt.-140.) then
                          flip = 180.
                       elseif (delavg.gt.65) then ! (ytwang)
                          flip = -delavg !+phi_i(mc)-phi_i(iend)  !-90.
                       elseif (delavg.lt.-65) then
                          flip = -delavg !+phi_i(mc)-phi_i(iend)  !90.
                       endif
c    Notice: it is NOT robust for int_phi_dp because it makes all the decreasing flat
c    in the phidp. If there is a long segment of bad data, the phidp may decreasing 
c    first and then increasing by same amount. Hence int_phi_dp will introduces an 
c    additional error into the result. (ytwang) dec.18, 2001
                       do  kf = 0, mgood-1
                          z(i-kf) = z(i-kf) + flip
                       enddo
            print *,'(ytwang) flip:: ',flip,' @ ',plfile(i,0)
                    else
                       flip = 0.
                    endif

                endif


                if (firsttime.eq.1) then
                    firsttime = 0
                    init0 = init0 + flip
                    init = init0
                else
                    avrg1 = avrg1 + flip
c               ??? Tied the bad data accroding to the monotonous increasing property of Phidp
c                   why monotonous incressing? decide the bad data by local trend! Yanting
                    if (avrg1.lt.init.and.avrg2.gt.init) then
                       avrg1 = avrg2
                    elseif (avrg1.gt.init.and.avrg2.lt.init) then
                       avrg2 = init
                    elseif (abs(delavg).gt.15..and.avrg1.lt.avrg2) then
                       avrg1 = avrg2
                    endif
                    init = avrg1     ! Save local trend
                    rmc =plfile(mc,0)
                    rend=plfile(iend,0)
                    d1 = rmc-rend
                    d2 = (avrg1-avrg2)/d1
                    d3 = (rend*avrg1-rmc*avrg2)/d1
                    do 11 ij = iend+1, mc
                       rij   = plfile(ij,0)
                       z(ij) = rij*d2 - d3
11                  continue
                 endif


                 loop  = 2
                 count = 0
                 mgood = 10 
                 iend  = length
              endif
           else
              count = 0
           endif
           goto 1000

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12
c ************* Find END of GOOD DATA loop ****************  

200        z(i) = plfile(i,inp) + flip
           plfile(i,16) = plfile(i,2) ! preserve good Zdr

           if(i.eq.length) then
              iend=length
              go to 203
           endif

c remove the testing on rhohv for COBRA data 
c for 2001 and 2003 data
c re-introduicng rhohv threshold for 2004 data as it 
c looks quite good now. Bringi 8/16/2004
c removing rhohv threshold for 21 Jan KC case for now
c as it looks really bad
c going back to old way  3 May 2007 as 21 Jan 2006 KC data rho is bad!!

c           if (plfile(i,ikdpst).lt.thrcrr.or.SNR.lt.SNRslevel
c     +     .or.plfile(i,49).gt.thrs_phi) then

c for Cpol data from Nov 05-March 06 TRMM PMM Working Group
c
c getting rid of SNR threshold for CPOL 2005-2006 season

         if(plfile(i,49).gt.thrs_phi) then ! 49 - sdev of phidp

              count = count + 1

c**************************************************************

c remove section dealing with hail/BB as rhohv is not used         
c for COBRA

c re-introducing for 2004 COBRA data as rhohv thresold
c for bad data in done now Bringi 8/16/2004

              if (count.eq.mbad) then  !Insert test to preserve hail/BB signal.
                 zhmn = 0.
                 do 288 jj = 0, mbad-1 ! mbad = 5
                    zhmn = 10.**(0.1*plfile(i-jj,1))+zhmn ! 1 - dbzh
                    yy(jj+1) = plfile(i-jj,ikdpst) ! ikdpst = rhohv
                    xx(jj+1) = plfile(i-jj,inp) ! inp = phidp
288              continue
                 zhmn = zhmn/float(mbad)
                 zhmn_log=10.*alog10(zhmn)
ccc Changing zh_mean value in BB or hail to 30 dBZ; 2/1/02 Bringi
ccc NOTE: BB with mean Zh<30 dBZ may be classified as "bad" data
c Changing zhmn threshold to 20 dBZ for bright-band/hail boundary for KC radar
c 21 Jan 2006 case. Bringi 5/1/07.
                 if (zhmn_log.gt.20.) then ! maybe 30 for CP2
                    call msr(ymn,sd,yy,mbad) ! maybe not needed
                    call msr(amean,test_sd_phidp,xx,mbad) ! needed
ccc rhohv in BB could go as low as 0.6
ccc  checking mean rhohv in BB/hail region
c         if (ymn.ge.0.6.and.test_sd_phidp.lt.(thrs_phi+5)) then
c
c
c  removing rhohv check in BB as some darwin datsets from 05/06 had
c  unseable rhohv data...don't know how many such datasets are affected
c  need to check below adjustment for BB later!!! Oct 07

         if(test_sd_phidp.lt.(thrs_phi+7.5)) then ! 7.5 -> 6
c
ccc changing std dev of phidp threshold in "hail" region to thrs_phi+5!! Bringi 12/03/01
cc
          print *,'hail/BB; rhohv,zh',ymn,zhmn_log,test_sd_phidp
          print *,'range(mbad)', plfile(i-4,0)
cc
                       count = 0
                       goto 1000
                    endif
                 endif
c*************************************************************

                 iend = i-mbad
                 k_arr=k_arr+1
                 iend_arr(k_arr)=iend
                 r_end_arr(k_arr)=plfile(iend,0)
Cc                print *,'k_arr',k_arr,iend_arr(k_arr)
                 do 210 jj = 0, mbad-1  !Inserted to clean the bad Zdr & Rhv.
                    plfile(i-jj,16) = -100. ! bad value for Zdr
                    plfile(i-jj,15) = 0.    ! bad value for Rhv
                    kdp_test(i-jj)=0.0
210              continue
203              continue
                 avrg2=(z(iend)+z(iend-1)+z(iend-2)+z(iend-3))/4. ! averaging phidp over 4 gates
                 if (iend.eq.length) goto 1000
                 z(i)   = avrg2
                 z(i-1) = avrg2
                 loop = 1
                 count = 0
              endif
           else
              count = 0
           endif
1000    continue
c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12

c       Save max value of k_arr and j_arr
        k_max_arr=k_arr
        j_max_arr=j_arr
c       print *,'k_max_arr',k_max_arr
c       print *,'j_max_arr',j_max_arr
c---------------- END of FINDING start AND stop BINs -------------------

        if(ibegin.eq.length) then  !NO good data in whole ray. RETURN.
           print *, 'No good data in entire beam!'
c          Simply put "raw" values of Zh and Zdr in plfile(i,20,21)
C           do i=1,length
c             Do only if rhovh>thrcrr and SNR>3 dB.
c             remove test on rhohv data for COBRA
C              if(plfile(i,11).gt.5.) then
C                 plfile(i,20)=plfile(i,1)  !
C                 plfile(i,21)=plfile(i,2)
C              endif
C           enddo
           go to 600
        endif

        if (kbegin.eq.length) then  ! OF COURSE
           kbegin = ibegin
        endif

c extend data records for filtering
   
        ext = avrg2
        do i=1,89+kbegin   !Set the initial conditions
           z(kbegin-i) = init0
        enddo
        do i=1,n-iend+30   !Extend data record
           z(iend+i)=ext
        enddo
        do i=-90,length+30 !Adjust raw data array
           x(i)= z(i)
        enddo
 
        irl=0
c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12
c------------- MAIN LOOP of Phidp Adaptive Filtering --------------------
        do 9999 mloop=1,madflt
c          TIE DOWN THE INITIAL and EXTENDING DATA RECORD
           do i=irl,kbegin+89
              z(kbegin-i)=init0
           enddo
           do i=1,n-iend+30
              z(iend+i)=ext
           enddo
c          FIR3 FILTER SECTION  ( change back from FIR1 12/8/92. )
           do i=-5,n+5
              acc=0.0
              do j=0,fir3order ! firorder is 20 for 150 m gate spacing
                 acc=acc+fir3coef(j)*z(i-fir3order/2+j)
              enddo
              y(i)=acc*fir3gain
           enddo
c          END of FIR3 FILTERING
           do i=1,n
              delt=abs(x(i)-y(i))
              if (delt.ge.thres) then
                 z(i)=y(i)
              else
                 z(i)=x(i)
              endif
           enddo
9999    continue 
c*****************END LOOP for Phidp Adaptive Filtering****************************

c       PUT KDP,DELTA,PDPCORRECTED,PDPCORRECTED-FILTERED into PLFILE
        do 90 i=-5,n+3
           plfile(i,nad1)= z(i)
           plfile(i,nfl) = y(i)  ! ??? (ytwang) -flip
           plfile(i,ndf) = x(i)-y(i) 
90      continue

c       CALCULATE KDP
102     delta_2r=plfile(3,0)-plfile(1,0)
        do 103 i=1,n ! n = length
c          Set "good data" range mask based on new Zdr stored in plfile(i,16)
           if(plfile(i,16).gt.-100.) then
              igood_data(i)=1
              plfile(i,41)=1.
           else
              igood_data(i)=0
              plfile(i,41)=0.
           endif 


c          Check Zh range
c          default value for nadp is 10
           nadp=10
           if(i.gt.15.and.i.lt.n-15) then
              if(plfile(i,1).lt.35.) nadp=30
              if(plfile(i,1).ge.35.and.plfile(i,1).lt.45.)
     +                               nadp=20
              if(plfile(i,1).gt.45.) nadp=10
           endif
           do jj=0,nadp
              xx(jj+1)=plfile(i-nadp/2+jj,0)
              yy(jj+1)=plfile(i-nadp/2+jj,nfl) ! nfl = final filtered phidp
           enddo

c          Improved Kdp base on LSE fit to Adap flt Phidp
           call LSE(aa,bb,xx,yy,nadp+1)
           plfile(i,nad2) = aa/2.
103     continue
c*******************END Kdp CALCULATION******************************************
c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12

c Seek for the reasonable start point for attenuation correction AND
c Implement standard attenuation correction, i.e., using default alpha & beta

c j_max_arr refers to the good data region, no good data, then bypass (ytwang)
       if (j_max_arr.eq.0) then
          go to 600
       endif

c Checking for those instances where good data mask extends from last
c bad data segment through iend=length. In such a case make iend1=length!
       if ((j_max_arr-k_max_arr).eq.1) then
          k_max_arr = k_max_arr+1
          iend_arr(k_max_arr) = length
       endif
       print *,'iend=',iend,'; iend_arr(max)=',iend_arr(k_max_arr)   
       iend=min0(iend,iend_arr(k_max_arr)) ! x1 -> x ytwang
         
c Adjust ibegin:iend from  <<< ADAPTIVE FILTER MENU for Phidp, DFR and LDR >>>
c in the MAIN menu. IT will read in stkdadrng, endkdadrng.
       if(irange.eq.1) then  ! x1 -> x ytwang
          ibegin=int(stkdadrng/delta_r)
          iend=  int(endkdadrng/delta_r)
          print *, 'user input ibegin:iend',stkdadrng,endkdadrng
       endif

       cos_el2=cos(elevation*0.0174532)**2
       sin_el =sin(elevation*0.0174532)
       sin_el2=sin_el*sin_el

c Note: By definition phidp_cons starts from zero.
c To avoid large errors in phdip_cons near the radar
c First find 10 consecutive gates where pdp_sd < thrs_phi  deg 
c This should define the beginning of "good" sector.
       ib=0
       jcount=0
       do 601  i=1,ibegin   ! x1 -> x
          pdp_sd=plfile(i,49)

          if(pdp_sd.lt.thrs_phi) then
             jcount=jcount+1
             if(jcount.eq.10)then
                do il=0,9
                   z_cns(i-il)=plfile(i-il,22)
                enddo
                ib=i-9

c Compute avg of phidp_cons in 4 good gates. This will be the
c initial value of the phidp used later in atten correction.
                anitial=(z_cns(ib)+z_cns(ib+1)+
     +          z_cns(ib+2)+z_cns(ib+3))/4.
                go to 602
             else
                go to 601
             endif
          else
             jcount=0
          endif
601    continue
602    continue

c do standard attenuation correction, based on that result (1st guess)
c computer HDR for accurate correction in the case of large phidp (ytwang)
       print *,'pre-correction: standard correction first'
c****************************************************************************

c forcing ib=0...and not using consesuus estimator in plfile(i,22)
c for the phidp increase...also means that ibegin is determined by
c previous thresholds is real start of beam...have to do this for 
c COBRA phipd problem at close ranges when its raining on the radar
c eg rhi from 8 june 2004 at 0442-0600...large postive Kdp detected
c by May's eatimator (up to 6 deg/km) and this causes plfile(i,22) to
c increase too much from old ib to ibegin...around 20 deg due to sysem
c problems. So backing off and doing standard correction using the 
c plfile(i,32) below....Bringi 8/22/04

       ib=0 ! forced Bringi 8/22/04
       if (ib.gt.0) then
          ibasic = ib
       else
          ibasic = ibegin
c replaced below from plfile(i,22) to plfile(i,32) Bringi 8/22/04

          anitial=(plfile(ibegin,32)+plfile(ibegin+1,32)+
     +          plfile(ibegin+2,32)+plfile(ibegin+3,32))/4.
       endif
       call basic_zh_corr(corr_height,cos_el2,ibasic,anitial,alpha)
       call basic_zdr_corr(corr_height,cos_el2,ibasic,anitial,beta)
       
c      the  alpha=0.0672 is for Dartwin Joss DSDs analyzed by MT
c      Nov 2007; the Adp multiplicative coeff is 0.0113; exponent is
c      1.276 hard wired in the subroutine

c new_zh_corr good for S band

       call new_zh_corr(corr_height,cos_el2,ibasic,anitial,0.0672) ! good scheme
       call new_zdr_corr(corr_height,cos_el2,ibasic,anitial,0.0113)

c Smoothing  raw Zh and Zdr for attenuation correction later
c Used to set the desired value of Zdr for Adp correction  Bringi 12/10/01
        do 105 i=1,n
           sumzh=0.0
           sumzv=0.0
           jcount=0

           do jj=0,10
              index = i-5+jj
c note: converting to Zh and Zv to linear scale
c yy contains linear Zh and zz contains linear Zv
              if (igood_data(index).eq.1) then
                 sumzh=sumzh+10.**(0.1*plfile(index,20))
                 sumzv=sumzv+10.**
     +           (0.1*(plfile(index,20)-plfile(index,21)))
                 jcount=jcount+1
              endif
           enddo

           if (sumzh.gt.0.0 .and. igood_data(i).eq.1) then ! add 'and' (ytwang)
              zhmean_lin=sumzh/float(jcount)
              zvmean_lin=sumzv/float(jcount)
              zhmean_log =10.*log10(zhmean_lin)
              zdrmean_log =10.*log10(zhmean_lin/zvmean_lin)
           else
              zhmean_log = 0
              zdrmean_log = 0
           endif

c USe HDR to make sure of pure rain before setting zdr_avg
c Using rainline for oscillating drops, see Chap 7 of B & C book
c NOTE: HDR here is based on  Zh and Zdr values corrected
c       for attenuation in simple way. It is only a first estimate of HDR 
c 
c*****************************************************
c C-band Darwin/SCSMEX dsds show
          if(zdrmean_log.le.0.0) then
             f_zdr = 32.
          elseif(zdrmean_log.le.1.43) then
             f_zdr=19.5*zdrmean_log+32
          else
             f_zdr=60.
          endif
          HDR=zhmean_log-f_zdr
          plfile(i,9)=HDR
c*****************************************************

 105   continue

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12
c____________________________________________________________
c Next part is copied and attached from Dr. Bringi's version

c ytwang modify the following part
c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12
c*******BEGIN ADAPTIVE TESTUD MEHOD FOR ATTEN CORRECTION OF REFLECTIVITY********

c Note: delta_r is the actual range gate spacing!
       delta_r=plfile(2,0)-plfile(1,0)

       k_iter=j_max_arr
       k1_iter=1
       k2_iter=0

1001   continue
c working below for multicell storm by ytwang

      ibegin1 = max0(ibegin,ibegin_arr(k_iter))
      iend1 = min0(iend,iend_arr(k_iter))
      
      if ((iend1-ibegin1).lt.20) then !if(j_max_arr.eq.1) then
         ih_start = 0
      else  ! checking hail in this cell
               
c********Begin detection of hail using HDR>20 dB*******
c        For mgood consecutive good gates
         jcount_h=0
         ih_start=0
         do 603 i=ibegin1,iend1
            HDR=plfile(i,9)
            if(HDR.ge.20) then
               jcount_h=jcount_h+1
               if(jcount_h.eq.mgood)then
                  ih_start=i-mgood+1
                  go to 604
               else
                  go to 603
               endif
            else
               jcount_h=0
            endif
603      continue
604      continue
         if (ih_start.eq.0) go to 613 ! no hail in this cell
         
c********Find end of Hail region using HDR<10 dB for 
c        mbad      consecutive gates 
         jcount_h=0
         ih_end=0
         do 610 i=ih_start,iend1
            HDR=plfile(i,9)
            if(HDR.le.10) then
               jcount_h=jcount_h+1
               if(jcount_h.eq.mbad) then
                  ih_end=i-mbad+1
                  go to 611
               else
                  go to 610
               endif
            else
               jcount_h=0
            endif
610      continue
611      continue
         if (ih_end.eq.0) ih_end=iend1

       print *,'-HailInfo>> ray0-ray1  range0-range1  Hdr0-Hdr1'
       print '(2x,2(i5),5x,2(f6.2),5x,2(f8.3))',ih_start,ih_end,
     +         plfile(ih_start,0),plfile(ih_end,0),
     +         plfile(ih_start,9),plfile(ih_end,9)

c Find location index of min ZDr within the hail region
c and check if its negative, then use it as a reference value
c for Adp correction 
         test_zdr=plfile(ih_start,2)  ! upgraded by ytwang dec-19,01
         index_zdr=ih_start
         do i=ih_start+1,ih_end
            if(plfile(i,2).lt.test_zdr) then
               test_zdr=plfile(i,2)
               index_zdr=i
            endif
         enddo 

613      continue

      endif ! goes with if(delta_i>20) above     



c Compute delta_phi and meanzdr in last 10 good gates

       sum_final=0.0
       do 151 k=iend1-mgood+1,iend1
          sum_final=sum_final+plfile(k,32)
151    continue
       phidpfinal= sum_final/mgood

       sum_phidp=0.0
       do 152 k=ibegin,ibegin+mgood-1 ! x1 -> x ytwang
          sum_phidp=sum_phidp+plfile(k,32)
152    continue
       phidpinit= sum_phidp/mgood

c delta_phi is the change in phidp across the beam from ibegin to iend
       delta_phi=phidpfinal-phidpinit

       print *,'-GoodData>> gate0-gate1 range0-range1 phi0-phi1-delta'
       print '(2x,2(i5),5x,2(f6.2),5x,3(f8.3))',
     +        ibegin,iend1,plfile(ibegin,0),plfile(iend1,0),
     +        phidpinit,phidpfinal,delta_phi

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12

c if delta_phi<10 deg at C-band  then don't do any correction by Testud Method
c by default we use standard method as in call Basic ZH correction
c subroutine already invoked above from ibegin:length (Bringi Aug 2004)


       if(delta_phi.le.10.) then
          print*,'delta_phi<10: standard correction acceptable'
          go to 600
       endif

c****************NOTE: Beyond this delta_phi > 10 deg!! 
c if the height at ibegin exceeds htU then don't do any correction
c using the Testud method as we don't know what atten/diff atten
c is caused by ice. The testud method is only valid in rain

       height_ibegin=corr_height(ibegin) ! x1 -> x
       print *,'height_ibegin:', height_ibegin
       if(height_ibegin.ge.htU)  then
          print *,'height(ibegin)> htU, no correction for ibegin:iend'
          go to 600
       endif


c The C-pol radar's system phidp is close to -65 deg. Sometimes
c due to malfunction the system phidp rises to near 0 deg. Check
c if phidpinit is very different (by over 50 deg) from the system_phidp
c
c changing to COBRA's sys_phidp...could comment
c out next 5 lines    26 Oct 2003 Bringi
c syst_phipd forr COBRA seems to be between -5 and -15 degrees
c for sample data from jun 8 2004...need to check for other case

c        sys_phidp=15.0
c          if(abs(phidpinit-sys_phidp).gt.50.) then
c          print *,'initial phidp is more than 50 deg away from system'
c          go to 600
c          endif

c******************************************************************************
c NOT DOING THIS FOR COBRA 8 JUNE 2004 data when its raining on the
c radome some weird system problem causes phidp to decrease and the
c increase with range giving large negative/postive Kdp values which
c causes the consessus intgegrated phipd to be wrong in plfile(i,22)
c Now assuming that there is no intense rain cell within 5 km or
c of radar which can cause significant attenuation !! Bringi Aug 2004

c First do simple correction from 1:ibegin as there may be a cell here.
c Use phipd_consencus from May to do this. Stored in plfile(i,22)
c Note that phidp_cons is calculated in this routine (Huang implemented it)

       print *,'Start from r(ib):',plfile(ib,0),'; anitial:',anitial
     +         ,'; r(ibegin):',plfile(ibegin,0)
       ! change ibegin1 to ibegin around here (ytwang)
       tempcorr=0.0
       tempfix=0.0
       temp_final_zh=0.0
       temp_final_zdr=0.0
       if(ib.lt.ibegin.and.ib.gt.0) then

          do i=ib,ibegin
             pdp_sd=plfile(i,49)


c Set Kdp from May's estimate in plfile(i,33)
c May's estimator is stored in plfile(i,6)

             if(plfile(i,20).ge.35.and.plfile(i,6).gt.0.
     +       and.pdp_sd.lt.thrs_phi) then
                plfile(i,33)=plfile(i,6)
             endif
          enddo  ! ib:ibegin1
c tempcorr is the change in phidp from ib to ibegin
c based on concensus phidp 

          tempcorr=plfile(ibegin,22)-anitial

       endif

       print *, 'phidp increases by tempcorr:',tempcorr ! comes from inside the loop above
c Store final atten/diff atten in dB for use later on.
c force tempcorr to be 0 here...means there is no attenuating rain
c cell from 0 to range(ibegin)...normally begin of good data starts
c after 1.5 km from radar...but for COBAR 8 June case setting good data
c to start after 5 km delay...because rain in radome in 8 June case
c from 0440-0600 causes weired system phase changes in beginning from
c 0 to 5 km range for RHI and upto 15 km for PPI!! Aug 2004 Bringi 

c************************************************************************
       tempcorr=0 ! forced Aug 22 2004 Bringi 
c************************************************************************

       if(tempcorr.gt.0) then
          temp_final_zh=tempcorr*alpha/cos_el2
          temp_final_zdr=tempcorr*beta
       else
          temp_final_zh=0.0
          temp_final_zdr=0.0
       endif

       print *,'at point ibegin temp_final_zh:',temp_final_zh,
     +         '; temp_final_zdr:',temp_final_zdr

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12
c Now start Testud Method
c Note 1:n means entire beam within which ibegin and iend represent the "good" data

       do i=1,n
          zh_lin(i)=10.**(0.1*plfile(i,1))
          phidp(i)=plfile(i,32)
          kdp_orig(i)=plfile(i,33)
          value(i)=0.0
       enddo

c "b_exp" is defined in Ah=a(No^(1-b))*Zh^b where b=b_exp.
c From analysis of SCSMEX dsds ( for Darwin it is 0.76)
c values set at beginning under iflag_loc

c Initialize gamma-values where Ah=gamma*Kdp. It is same as alpha.
c Note that gAmma is defined in the data statement
c Make elevation angle correction since Testud's equations are
c only valid for elev=0. Note that Ah is assumed to be indpendent
c of elevation angle, but Kdp depends on elevation angle.

       loop = malpha ! set to 23 
       if (loop .eq. 1) gamma(1) = gamma(mopt)

       do 320 j=1,loop
          temp(j)=10.**(0.1*b_exp*gamma(j)*delta_phi/cos_el2)
          temp(j)=temp(j)-1

          do 321 k=1,n
             zhlinn(k,j)=zh_lin(k)
             phidpn(k,j)=phidp(k)
321       continue
320    continue


       do 302 i=ibegin,iend1-1
          sum=0.0
          ifix=i
          do 303 j=ifix,iend1
             sum=sum+(zh_lin(j)**b_exp)*b_exp*0.46*delta_r
303       continue
          value(i)=sum ! sum|value::I(r;rM)
302    continue
       Ir1r0=value(ibegin) ! ibegin1 -> ibegin
       value(iend1)=0.0
c Force value(iend1)=0 by definition 3/20/01 Bringi
c*******************************************************************

c Set the initial value for valuen, atten and phidp_test
c Combine the loop here and the two next (ytwang)
       do k=1,loop
          do i=ibegin,iend1
             valuen(i,k)=value(i)
c Find Ah at each gate and for each gamma-value
             atten(i,k)=temp(k)*(zhlinn(i,k)**b_exp)
             atten(i,k)=atten(i,k)/(Ir1r0+temp(k)*valuen(i,k))
c For each gamma-value compute the cumulative phidp by integrating
c Kdp=Ah/gamma with range. Do a correction for elevation angle
c dependence of Kdp since Kdp=Ah/gamma is valid at elev=0.
             sum = 0.0
             do j=ibegin,i
                sum=sum+atten(j,k)*delta_r
             enddo
             phidp_test(i,k)=phidpinit+(cos_el2*sum*2./gamma(k))
          enddo
          phidp_test(ibegin,k)=phidpinit ! Reset the initial one
       enddo

c For each gamma-value, find abs(deviation) between the measured
c phidp and the phidp calculated from Ah at each gate along the beam
c then find sum of these errors for the entire beam.

       do 310 j=1,loop
          sumerr=0.0
          do 311 i=ibegin,iend1
             error(i,j)=abs(phidp_test(i,j)-phidpn(i,j))
             sumerr=sumerr+error(i,j)
311       continue
          sumerror(j)=sumerr
c         print *,'sumerror(j)',sumerr
310    continue

c Find optimal gamma-value that gives minimum error. Not guaranteed
c that an optimal gamma can be found. If the minimum error can't be
c found in the gamma-interval then select default value
c of 0.08 

       jmin=1
       amintest=sumerror(1)
       do 312 j=1,loop
          if(sumerror(j).lt.amintest) then
             amintest=sumerror(j)
             jmin=j
          endif
312    continue


c check if first value of gamma  is chosen as the optimal
c value. This usually means that a minimum was not found.
c Also check if last value of gamma was chosen. In either
c case set gamma_opt=.08. This value is good for both SCSMX and Darwin dsd.

c set jmin=1 or gamma_opt=alpha=0.08 if delta_phi<30 deg. This
c is to ensure that there is substantial phidp across the rain cell
c before selecting an optimal gamma for the beam. The Ah profile
c is very sensitive to gamma_opt value and so is the derived
c rainrate. Since we are comparing the derived ohidp with the
c measured phidp to select the optimal gamma, it is prudent
c to make sure delta_phi >30 deg.


        if(delta_phi.lt.30) jmin=1

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12

       if(jmin.eq.1.or.jmin.eq.loop) then

c make sure that further down if default value of
c gamma =.08 is used that the index in the atten arrays
c are set correctly !!

          gamma_opt=alpha
          print *, 'Optimal gamma cannot be found'
          tempx=10.**(.1*b_exp*gamma_opt*delta_phi) ! Similar with 'temp'
          temp_Nw=1-10.**(-.1*b_exp*gamma_opt*delta_phi)
       else
          gamma_opt=gamma(jmin)
          print *, 'Optimal gamma value found', gamma_opt
          tempx=10.**(.1*b_exp*gamma_opt*delta_phi)
          temp_Nw=1-10.**(-.1*b_exp*gamma_opt*delta_phi)
       endif

c       a_Nw=3.9655E-07
c       Nw_testud=(temp_Nw/(Ir1r0*a_Nw))**(1/(1-b_exp))

c Store the optimum phipd calculated in plfile(i,25)

       do i=ibegin,iend1

c*********************CHECKING*********************
            plfile(i,48)=gamma_opt
c**************************************************

          if((jmin.eq.1.or.jmin.eq.loop).and.loop.gt.1) then

c If gamma values in data statement are changed
c then make sure the default value of gamma=.08
c points to the correct index in phidp_test and atten arrays!!

             plfile(i,25)=phidp_test(i,9)
             plfile(i,27)=atten(i,9)
          else
             plfile(i,25)=phidp_test(i,jmin)
             plfile(i,27)=atten(i,jmin)
          endif
c          plfile(i,60)=(10**(2.98))*(Nw_testud**(.09))
c          plfile(i,60)=plfile(i,60)*(plfile(i,27)**0.91)
       enddo


c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12

c Calculate corrected Zh at each gate
c Make sure that atten/diff atten from ib:ibegin-1 is added in.
       testfix=0.0
       test=0.0
       do 314 i=ibegin+1,iend1
          sum=0.0
          ifix=i
          do 315 k=ibegin,ifix
             if((jmin.eq.1.or.jmin.eq.loop).and.loop.gt.1) then
c*** If gamma values are changed in data statement
c*** then make sure default value of gamma=.08 points
c*** to correct index in atten array below!!
                sum=sum+atten(k,9)*delta_r
             else
                sum=sum+atten(k,jmin)*delta_r
             endif
315       continue

c Here, add in the attenuation from 1:ibegin previously
c stored in temp_final_zh. This is in dB, so is "sum" above.

          test=2.0*sum+temp_final_zh

c Do correction of Zh only if height of gate < htU where htU is set=6 km
c For gates located along the beam whose height is > 6 km the
c total attenuation is fixed by testfix below.

          height=corr_height(i)
          if(abs(height-htU).le.0.1) testfix=test
          if(height.le.htU) then
             plfile(i,20)=plfile(i,1)+test !10.*alog10(zh_lin(i)*test)
          else
             plfile(i,20)=plfile(i,1)+testfix !10.*alog10(zh_lin(i)*testfix)
          endif
314    continue

c Store the final dB values for later use
       test_dB=test
       testfix_dB=testfix

       print *,'The values below are in dB from 1:iend for Zh'
       print *,'test_dB:',test_dB,'; testfix_dB:',testfix_dB

c Complete the attenuation correction for gates from iend1+1 to n.
c Note that phidpfinal is the "initial" value of phidp at iend1!
c Use standard atten correction method here using filtered phidp
c stored in plfile(i,32).
c Make sure that test1 and test_dB are added in. Then add in
c any additional correction from iend1 to end of beam.
       print *,'complete zh correction after iend1'
       height_end1=corr_height(iend1+1)
       temp_end=0.0
       tempx_end=0.0
       print *,'height(iend1):',height_end1,'; k_iter:',k_iter

       do i=iend1+1,n
          height=corr_height(i)
          if(abs(height-htU).le.0.1) tempx_end=
     +    plfile(i,32)-phidpfinal
          temp_end=plfile(i,32)-phidpfinal

          if (height_end1.le.htU.and.height.le.htU) then
             plfile(i,20)=plfile(i,1)+test_dB
             if(temp_end.gt.0.) then
                plfile(i,20)=plfile(i,20)+temp_end*gamma_opt/cos_el2
             endif
          elseif(height.gt.htU.and.height_end1.le.htU) then
             plfile(i,20)=plfile(i,1)+test_dB
             if(tempx_end.gt.0.) then
                plfile(i,20)=plfile(i,20)+tempx_end*gamma_opt/cos_el2
             endif
          else
             plfile(i,20)=plfile(i,1)+testfix_dB
          endif
       enddo
c (ytwang) The ZH correction is completed here.

c Compute meanzh in last mgood "good" gates so that
c the desired value of Zdr there can be estimated.
c Compute mean HDR in last mgood gates but note that
c HDR is first guess since Zdr is corrected by simple method

1002   continue   ! ytwang checking the values in hail

       sum_zh=0.0
       sum_zv=0.0 
c using smoothed ZH and Zdr fields here, Bringi 12/10/01
       do k=iend1-mgood+1,iend1
          sum_zh=sum_zh+10.**(0.1*plfile(k,1))
          sum_zv=sum_zv+10.**(0.1*(plfile(k,1)-plfile(k,2)))
       enddo

       meanzdr=10.*alog10(sum_zh/sum_zv)

       sum_zh_corr=0.0
       sum_HDR=0.0
       sum_kdp=0.0
       do k=iend1-mgood+1,iend1
          sum_zh_corr=sum_zh_corr+10.**(0.1*plfile(k,20))
          sum_HDR=sum_HDR+10.**(0.1*plfile(k,9))
          sum_kdp=sum_kdp+plfile(k,33)
       enddo
       meanzh_corr=10.*alog10(sum_zh_corr/mgood)
       mean_HDR=10.*alog10(sum_HDR/mgood)
       mean_kdp=sum_kdp/mgood

c initializing tempzdr, and zdr_avg Bringi 8/17/04
        tempzdr=1.0
        zdr_avg=0.0
       

c find height agl of the gate where meanzh_corr is calculated

       height_meanzh=corr_height(iend1-mgood/2)

c Set up the desired  zdr_avg value in rain based upon Zh.
c Checked for SCSMEX dsd data: Bringi 2/14/02
c Huang checked against Joss Darwin data
c Used in Gematronik handbook Bringi 8/17/04
c made elevation angle correction for Rayleigh scattering
c Bringi 8/17/04

      if(meanzh_corr.le.20.) then
          zdr_avg=0.0

      elseif(meanzh_corr.gt.20.and.meanzh_corr.le.55.) then

          
          zdr_avg= .0366*meanzh_corr-0.5378
          zdr_avg_lin=10.**(.1*zdr_avg)  ! linear Zdr
          tszdr=(sqrt(zdr_avg_lin)*sin_el2+cos_el2)
          tszdr2=tszdr*tszdr
          tempzdr=zdr_avg_lin/tszdr2

c make elevation angle correction 
        if(tempzdr.gt.0)zdr_avg=10.*alog10(tempzdr)

       else
c added to take care if meanzh_corr>55 dBZ, put ceiling 
          zdr_avg=1.475
 
          zdr_avg_lin=10.**(.1*zdr_avg)  ! linear Zdr
          tszdr=(sqrt(zdr_avg_lin)*sin_el2+cos_el2)
          tszdr2=tszdr*tszdr
          tempzdr=zdr_avg_lin/tszdr2
c make elevation angle correction 
        if(tempzdr.gt.0)zdr_avg=10.*alog10(tempzdr)

       endif
c************************************************************




c if the above height exceeds the height of the freezing level
c then avg zdr there is forced to 0. This is assumed to be the
c same as the height above which no attenuation correction is done.
c Also, check if mean_HDR>5 dB, as this would signify hail region

       if(height_meanzh.ge.htL.or.mean_HDR.ge.5) then
          zdr_avg=0.0
          tempzdr=1.0
       endif

       print *,'MEAN_ZDR: ',meanzdr,'; DESIRED ZDR:',zdr_avg,
     +      '; MEAN_HDR:',mean_HDR, 'elev=', elevation,
     +       'tempzdr', tempzdr



c CHECK IF meanzdr is > desired Zdr!!! 
c Cannot us Ah-Av to reduce the raw Zdr , it can only increase raw Zdr
c and if meanZdr is already > desired Zdr then can't use this method.
c Decide to use default or simple method with no constraints!!

c simply go to 600 where the simple method is used,          
c Ah-Av=beta*delta_phi
c Inserted on 12/7/01, bringi
c NOTE: k_max_arr gives the number of "good" data segments

c next block was upgraded by ytwang
        if(meanzdr.gt.zdr_avg.and.delta_phi.ge.30.and
     +     .j_max_arr.ge.2) then ! should be j_ (means good data start)
           print *,'case of raw zdr > desired zdr: meanzdr:',meanzdr

           if (k1_iter.ne.k2_iter .and. ih_start.ne.0) then ! we have not check the hail
              iend1 = index_zdr+5
              k2_iter = k2_iter+1
              print *,'hail case: iend1:',iend1,'; ih_end:',ih_end
              go to 1002
           elseif (k_iter.gt.1) then ! even hail the condition not met
              k_iter = k_iter-1
              k2_iter = k1_iter
              k1_iter = k1_iter+1
              print *,'backward after checking hail; k1_iter:',k1_iter
              go to 1001
           else  ! have already reached to first cell
              go to 600
           endif
        elseif(meanzdr.lt.zdr_avg)then
           print *,'going to 902 continue'
           go to 902 
        else
           go to 600
        endif   ! goes with if(meanzdr.gt....) above


c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12

c*******************************************************************
c Now start correction for Zdr.

902    continue


c where Ah-Av=const_zdr*Ah , and beta is defined as Ah-Av=beta*Kdp


       const_zdr=1
       print *,'-Init>> meanzh_corr  meanzdr  weight  const_zdr'
       print '(2x,4(f8.3))',meanzh_corr,meanzdr,weightcoeff,const_zdr

       do 316 i=ibegin,iend1  ! ibegin1 -> ibegin
          height=corr_height(i)
          if(height.le.htL) then
 
c Normally set htL=4 km which is near the melting level
c for the tropics, but htL=1.5 km for Colorado
c Also, an adjustment is made of elevation angles > 0 to
c the relation Ah-Av=Ah*const_zdr as the const_zdr is strictly
c valid only for elev angle=0.0. The adjustment is valid for
c Rayleigh scattering.
             AhAv_Sm(i)=plfile(i,27)*const_zdr*cos_el2
             plfile(i,38)=AhAv_Sm(i) ! Adp

          elseif(height.gt.htL.and.height.le.htU) then

c A linearly decreasing weight is applied in the melting layer
c otherwise diff atten in bright band may be overestimated.
             AhAv_Sm(i)=plfile(i,27)*const_zdr*((htU-height)/
     +       (htU-htL))*cos_el2
             plfile(i,38)=AhAv_Sm(i)

c It is better to use the Smythe/Illin method between htL and htU
c in the melting layer. Note that we use weightcoeff=const_zdr*
c gamma_opt here.
             if(kdp_orig(i).ge.0.5)then
                AhAv_Sm(i)=const_zdr*gamma_opt*kdp_orig(i)
                plfile(i,38)=AhAv_Sm(i)
             endif

          else
             AhAv_Sm(i)=0.0
             plfile(i,38)=AhAv_Sm(i)
          endif
316    continue

c********************CHECK IF BIG DROP DETECTION NEEDED
c********************and local increase in Ah-Av needed??

       testfix1=0.0
       test1=0.0

       do 318 i=ibegin+1,iend1
          sum=0.0
          ifix=i
          do 319 k=ibegin,ifix
             sum=sum+AhAv_Sm(k)*delta_r
319       continue

c Add in the total diff atten from 1:ibegin-1 stored
c in temp_final_zdr which is in dB, as is "sum" above.
          test1=2.*sum+temp_final_zdr

c Store corrrected Zdr in plfile(i,21). Raw zdr is is plfile(i,2).
          height=corr_height(i)
          if(abs(height-htU).le.0.1) testfix1=test1
          if(height.le.htU) then
             plfile(i,21)=plfile(i,2)+test1
             if(plfile(i,16).gt.-100.)plfile(i,16)=plfile(i,21)
          else
             plfile(i,21)=plfile(i,2)+testfix1
             if(plfile(i,16).gt.-100.)plfile(i,16)=plfile(i,21)
          endif

318    continue
       print *,'total zdr correction dB from 1:iend is: ',test1

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12


c Compute meanzdr in last 10 "good" gates to see if
c desired value in zdr_avg was achieved.

       sum_zh=0.0
       sum_zv=0.0
       do 154 k=iend1-9,iend1
          sum_zh=sum_zh+10.**(0.1*plfile(k,20))
          sum_zv=sum_zv+10.**(0.1*(plfile(k,20)-plfile(k,21)))
154    continue
       meanzdr_corr=10.*alog10(sum_zh/sum_zv)

       print *, 'meanzdr_corr:', meanzdr_corr


c Iterate the Zdr correction so that the meanzdr_corr tends to
c zdr_avg which is the desired value . This was necessary since the
c calculated Ah-Av above tends to "over correct" the raw zdr esp


c This is for iterative Testud's method of deriving Ah-Av
       kcount_zdr=0
       kcount_zdr1=0
       weight_adj=weightcoeff

901    test_zdr=meanzdr_corr-zdr_avg

c The following takes care of "overshooting" the desired  zdr
c*****************IF, THEN, ELSEIF,ELSE, ENDIF*********
       if(test_zdr.gt.epslon_over) then
          kcount_zdr=kcount_zdr+1

          const_adj=(zdr_avg-meanzdr)/(meanzdr_corr-meanzdr)
          const_zdr=const_zdr*const_adj

c this code takes care of "undershooting" the desired zdr
       elseif(test_zdr.lt.epslon_under) then
          kcount_zdr1=kcount_zdr1+1

          const_adj=(zdr_avg-meanzdr)/(meanzdr_corr-meanzdr)
          const_zdr=const_zdr*const_adj

       else
          print *,'test_zdr satisfied for convergence: iter: ',
     +           kcount_zdr,kcount_zdr1
          go to 700
       endif
c***************************************************

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12

cTHIS IS  A LONG IF, THEN,ELSE,ENDIF  sectiom marked out 
c**************************************************
       if(kcount_zdr.gt.19.or.kcount_zdr1.gt.19) then
          print *,'Number of iter exceeded: ',kcount_zdr,kcount_zdr1
          go to 700
       else

          do i=ibegin,iend1
             height=corr_height(i)
             AhAv_Sm(i)=plfile(i,38)*const_adj ! plfile(i,27)*const_adj
             plfile(i,38)=AhAv_Sm(i)
          enddo

          testfix1=0.0
          test1=0.0
          do 418 i=ibegin+1,iend1
             sum=0.0
             ifix=i
             do 419 k=ibegin,ifix
                sum=sum+AhAv_Sm(k)*delta_r
419          continue

c add in any diff atten from 1:ibegin-1 computed earlier
             test1=2.*sum+temp_final_zdr
             
             height=corr_height(i)
             if(abs(height-htU).le.0.1) testfix1=test1
             if(height.le.htU) then
                plfile(i,21)=plfile(i,2)+test1
                if(plfile(i,16).gt.-100.) plfile(i,16)=plfile(i,21)
             else
                plfile(i,21)=plfile(i,2)+testfix1
                if(plfile(i,16).gt.-100.) plfile(i,16)=plfile(i,21)
             endif
418       continue

c Find meanzdr_corr in last 10 "good" gates.
          sum_zh=0.0
          sum_zv=0.0
          do k=iend1-9,iend1
             sum_zh=sum_zh+10.**(0.1*plfile(k,20))
             sum_zv=sum_zv+10.**(0.1*(plfile(k,20)-plfile(k,21)))
          enddo
          meanzdr_corr=10.*alog10(sum_zh/sum_zv)

          print*,'-Loop>> k_iter  const_adj  const_zdr  meanzdr_corr'
          print '(2x,i5,3x,3(f8.3))',k_iter,const_adj,
     +          const_zdr,meanzdr_corr


c The const_zdr "out of range" should be same at S or C bands
           if (const_zdr.le.0.125 .or. const_zdr.gt.1.5) then
              print *,'WARNING: const_zdr out of range! <0.125, 1.5>'
           endif

             go to 901

       endif
c*****************************************************************


700    continue ! Complete correction of  Zdr only  for gates from iend1+1:n
       print *,'complete zdr correction after iend1'

          height_end1=corr_height(iend1+1)
          temp_end=0.0
          tempx_end=0.0

          do i=iend1+1,n
             height=corr_height(i)
             if(abs(height-htU).le.0.1)
     +       tempx_end=plfile(i,32)-phidpfinal
             temp_end=plfile(i,32)-phidpfinal

             if (height_end1.le.htU.and.height.le.htU) then
                plfile(i,21)=plfile(i,2)+test1
                if(plfile(i,16).gt.-100.)plfile(i,16)=plfile(i,21)

                if(temp_end.gt.0.) then
                   plfile(i,21)=plfile(i,21)+temp_end*beta
                   if(plfile(i,16).gt.-100.)plfile(i,16)=plfile(i,21)
                endif

             elseif(height.gt.htU.and.height_end1.le.htU) then
                plfile(i,21)=plfile(i,2)+test1
                if(plfile(i,16).gt.-100.)plfile(i,16)=plfile(i,21)

                if(tempx_end.gt.0.) then
                   plfile(i,21)=plfile(i,21)+tempx_end*beta
                   if(plfile(i,16).gt.-100.)plfile(i,16)=plfile(i,21)
                endif

             else
                plfile(i,21)=plfile(i,2)+testfix1
                if(plfile(i,16).gt.-100.)plfile(i,16)=plfile(i,21)
             endif
          enddo


c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12

600    continue

c Call rainrate_beta here (at this point the Zh and Zdr 
c are corrected by the standard method (not Testud's)
       call rainrate_beta(igood_data,corr_height,ibegin_arr)
       return

       end
c end  kdpchill
c_____________________________________________________________


c ------------------------------------------------------------
c Subroutine: simple zh correction
c Changes: seperate subroutine for zh and zdr correction
c         
       subroutine basic_zh_corr(corr_height,cos_el2,ib,anitial,alpha)
       real corr_height(-5:1900),cos_el2,anitial,alpha
       integer ib
       include 'comblk.h'

c changing from using conseusus phidp in plfile(i,22) to 
c adap filtered phipd plfile(i,32) for standard correction
c this may be problem if raw phidp is not unwrapped properly
c Bringi 8/22/04 ...also min range is set at 5 km for
c COBRA 8 June data as when it rains over radar there
c is large swing in phidp from 0 to 5 km (normally I have
c set min range to be 1.5 km (farfield). 

       tempcorr=0.0
       tempfix=0.0
       temp_final_zh=0.0


c       if(ib.gt.0) then
          do i=ib,length
             height=corr_height(i)

             if(abs(height-htU).le.0.1) then
                tempfix=plfile(i,32)-anitial
             endif
             tempcorr=plfile(i,32)-anitial

             if (tempcorr.gt.0.and.height.le.htU) then
                plfile(i,20)=plfile(i,1)+tempcorr*alpha/cos_el2
             elseif(tempfix.gt.0.and.height.gt.htU) then
                plfile(i,20)=plfile(i,1)+tempfix*alpha/cos_el2
             else
                plfile(i,20)=plfile(i,1)
             endif
          enddo
        print *, 'std corr Zh  term', tempcorr*alpha/cos_el2,
     +            'anitial/delta _phi at end',anitial, tempcorr 
     +     ,'plfile(i,20)', plfile(i,20)
c       endif
       
       return
       end ! basic_zh_corr

c ------------------------------------------------------------
c Subroutine: simple zdr correction
c Changes: seperate subroutine for zh and zdr correction
c       
       subroutine basic_zdr_corr(corr_height,cos_el2,ib,anitial,beta)
       real corr_height(-5:1900),cos_el2,anitial,beta
       integer ib
       real conskdp
       include 'comblk.h'

c changing from using conseusus phidp in plfile(i,22) to 
c adap filtered phipd plfile(i,32) for standard correction
c this may be problem if raw phidp is not unwrapped properly
c Bringi 8/22/04


       tempcorr=0.0
       tempfix=0.0
       temp_final_zdr=0.0

c       if(ib.gt.0) then
          do i=ib,length
             height=corr_height(i)

             if(abs(height-htU).le.0.1) then
                tempfix=plfile(i,32)-anitial
             endif
             tempcorr=plfile(i,32)-anitial
  
             if (tempcorr.gt.0.and.height.le.htU) then
                plfile(i,21)=plfile(i,2)+tempcorr*beta
                if(plfile(i,16).gt.-100.)
     +          plfile(i,16)=plfile(i,2)+tempcorr*beta
                conskdp = plfile(i,6)
                if(conskdp.gt.0) plfile(i,38) = beta*conskdp ! for ldr correction
             elseif(tempfix.gt.0.and.height.gt.htU) then
                plfile(i,21)=plfile(i,2)+tempfix*beta
                if(plfile(i,16).gt.-100.)
     +          plfile(i,16)=plfile(i,2)+tempfix*beta
             else
                plfile(i,21)=plfile(i,2)
                if(plfile(i,16).gt.-100.)plfile(i,16)=plfile(i,2)
             endif
          enddo

        print *, 'total corr Zdr term', tempcorr*beta   

c      else
c       endif
       
       return
       end ! basic_zdr_corr

c ------------------------------------------------------------
c Subroutine: new zh correction
c         
       subroutine new_zh_corr(corr_height,cos_el2,ib,anitial,alpha)
       real corr_height(-5:1900),cos_el2,anitial,alpha
       integer ib
       include 'comblk.h'

c changing from using conseusus phidp in plfile(i,22) to
c adap filtered phipd plfile(i,32) for standard correction
c this may be problem if raw phidp is not unwrapped properly
c Bringi 8/22/04 ...also min range is set at 5 km for
c COBRA 8 June data as when it rains over radar there
c is large swing in phidp from 0 to 5 km (normally I have
c set min range to be 1.5 km (farfield).

       tempcorr=0.0
       tempfix=0.0
       temp_final_zh=0.0


c       if(ib.gt.0) then
          do i=ib,length
             height=corr_height(i)

             if(abs(height-htU).le.0.1) then
                tempfix=plfile(i,32)-anitial
             endif
             tempcorr=plfile(i,32)-anitial

             if (tempcorr.gt.0.and.height.le.htU) then
                plfile(i,43)=plfile(i,1)+tempcorr*alpha/cos_el2
             elseif(tempfix.gt.0.and.height.gt.htU) then
                plfile(i,43)=plfile(i,1)+tempfix*alpha/cos_el2
             else
                plfile(i,43)=plfile(i,1)
             endif
          enddo
        print *, 'new  corr Zh  term', tempcorr*alpha/cos_el2,
     +  'plfile(i,43)', plfile(i,43),'plfile(i,1)',plfile(i,1)
c       endif

       return
       end ! new_zh_corr

c ------------------------------------------------------------


c ------------------------------------------------------------
c Subroutine: new zdr correction
c       
c modified based on basic_zdr_corr
c Jason, May 6, 2005
       subroutine new_zdr_corr(corr_height,cos_el2,ib,anitial,beta)
       real corr_height(-5:1900),cos_el2,anitial,beta
       integer ib
       real conskdp
       real adp,int_adp,deltar
       include 'comblk.h'

c changing from using conseusus phidp in plfile(i,22) to 
c adap filtered phipd plfile(i,32) for standard correction
c this may be problem if raw phidp is not unwrapped properly
c Bringi 8/22/04


       tempcorr=0.0
       tempfix=0.0
       temp_final_zdr=0.0
       int_adp=0.0
       deltar=plfile(2,0)-plfile(1,0)
          do i=ib,length
c do only in good data segments Nov 2007

           if(plfile(i,41).gt.0.5) then  

c  exponent is for Darwin DSDs as analyzed by MT Nov 2007
             if (plfile(i,33).ge.0) then
          	 	adp=beta*(plfile(i,33)**1.2761)
                 int_adp=int_adp+2*adp*deltar
          	 else

c if Kdp<0 due to noise fluctuations then take abs value and  calculate
c adp

          	  adp=beta*abs(plfile(i,33))**1.2761
                 int_adp=int_adp-2*adp*deltar

          	 endif 
                 endif
               plfile(i,45)=adp
               plfile(i,46)=int_adp
          	 
             height=corr_height(i)

             if(abs(height-htU).le.0.1) then
                tempfix=int_adp
             endif
             tempcorr=int_adp
  
c             if (tempcorr.gt.0.and.height.le.htU) then

                 if(height.le.htU) then
                plfile(i,44)=plfile(i,2)+tempcorr

c             elseif(tempfix.gt.0.and.height.gt.htU) then


                elseif(height.gt.htU) then
                plfile(i,44)=plfile(i,2)+tempfix                
             else
                plfile(i,44)=plfile(i,2)
             endif
          enddo

        print *, 'total corr Zdr term (new)', tempcorr
      
       return
       end ! basic_zdr_corr
