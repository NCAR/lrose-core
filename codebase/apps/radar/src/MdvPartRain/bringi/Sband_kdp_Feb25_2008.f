c _____________________________________________________________
c	 
        subroutine csu_kdp(length,range,zh,zdr,phidp,
     +                     rhohv,snr_s,kdp_final)

c      Main program to use and test csu_kdp_version5 routine

c      COMMON DATA DECLARATION
       parameter (ndim=5000)
       real zh(ndim),zdr(ndim),phidp(ndim),rhohv(ndim)
       real range(ndim),phi_int(ndim),kdp(ndim)
       real plfile(-5:ndim,0:70),kdp_final(ndim),snr_s(ndim)
       integer length
C       common /PACK/ plfile
c file 'testray.dat' contains an ascii test file of
c input data: range in km, raw Zh, raw Zdr, raw phidp and raw rhohv
c The sample testray.dat provided is for CPOL beam from 21 Jan 1999

	  integer firsttime,count
        integer ibegin_arr(20),iend_arr(20)
        integer fir3order
        real x(-90:ndim),y(-90:ndim),z(-200:ndim),z_cns(ndim)
c  **************  changes made Feb 25 2008 ******************
        real xx(31),yy(31),fir3coef(0:30)
c  **************  end changes made Feb 25 2008 ******************
        real init0,init,thres
        real fir3gain,elevation
        real r_begin_arr(20),r_end_arr(20)
        real phinit0,r_min
C        common /FIR/ fir3order,fir3gain,fir3coef

c NEXT THREE DATA STATEMENTS CONTAIN THE FIR FILTER ORDER, GAIN AND COEFICIENTS
c The specification of FIR filter coeficients is set for gate spacing of 150 meters.
c
c  **************  changes made Feb 25 2008 ******************
       data fir3order/30/
       data fir3gain/1.00000/
       data (fir3coef(i), i=0,30)
     +   /0.01040850049, 0.0136551033,  0.01701931136,
     +	0.0204494327,  0.0238905658,  0.02728575662,
     +	0.03057723021, 0.03370766631, 0.03662148602,
     +	0.03926611662, 0.04159320123, 0.04355972181,
     +	0.04512900539, 0.04627158699, 0.04696590613,
     +	0.04719881804, 0.04696590613, 0.04627158699,
     +	0.04512900539, 0.04355972181, 0.04159320123,
     +	0.03926611662, 0.03662148602, 0.03370766631,
     +	0.03057723021, 0.02728575662, 0.0238905658,
     +	0.0204494327,  0.01701931136, 0.0136551033,
     +	0.01040850049/ 

c  **************  end changes made Feb 25 2008 ******************
      
c First set up the initialization routine

        do i=1,length
          plfile(i,0)=range(i)
          plfile(i,1)=zh(i)
          plfile(i,2)=zdr(i)
c Generate Zv field

          plfile(i,7)=plfile(i,1)-plfile(i,2)
          plfile(i,3)=phidp(i)
          plfile(i,5)=rhohv(i)
		plfile(i,11)=snr_s(i)

c Estimate SNR using radar constant of 72 and noise floor of -115 dBm
c NEED TO FINE TUNE FOR CP2!!! if exact SNR is required


          r = plfile(i,0)
        enddo

c Now call the major routine that calculates Kdp,
c does attenuation correction of Zh and Zdr, calculates
c gamma dsd paramters, 'pol tuned' Z-R, and other rainrate
c estimates


        phinit0 = -999 !automatically decide initial phase

	r_min = 5	! the minimum range in km depends on radar system, e.g. beyond far field.

        nad1 = 31          ! label(31)='Pdpadap'
        nfl  = 32          ! label(32)='Pdpadfl'
        nad2 = 33          ! label(33)='Kdpadap'
        ndf  = 34          ! label(34)='Delta'

c  **************  changes made Feb 25 2008 ******************

        thres = 4.	! this value depends on the expected backscatter diff phase
        madflt = 20
        thrs_phi=12 ! this value for CP2
	  thrcrr=0.9   ! set low since not used
c  
c  **************  end changes made Feb 25 2008 ******************

c
        inp = 3
        ikdpst = 5
        firsttime = 1
        mgood     = 10       ! number of good data to enter cell
        mbad      = 5        ! number of bad data to exit cell
        count     = 0
        kbegin    = length
        iend      = length
        loop      = 1
 
        do i=-5,length
           plfile(i,nad1)=0.0   ! preset Pdpadap
           plfile(i,nfl)=0.0    ! preset Pdpadfl
           plfile(i,nad2)=0.0   ! preset Kdpadap
	     plfile(i,ndf)=0.0    ! preset Delta
c A 'good' data mask in set in plfile(i,41) as real 0.0 (bad) and 1.0 (good)
           plfile(i,41)=0.0     ! preset good data mask =0 (all bad)
        enddo


        tempcorr=0.0
        tempfix=0.0
        anitial=0.0
        avrg2 = 0.0
        j_arr=0
        k_arr=0

        
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
        end if  ! to adjuest the reference phase by observation (ytwang)

           thrs_unfold=90   ! must be checked for CP2

c This is Yanting's modified unfold code 
        do 8 jj = 1,length

          plfile(jj,63) = plfile(jj,3)  ! save the raw phidp to checking unfold

        if(inwrap.eq.1) then
           if(plfile(jj,3).lt.(avgbg-thrs_unfold+extra)) then

c Depending on the pulsing mode, if alternate H/V, the folding value should be 
c changed to 180 degrees; if simultaneous transmission mode (slant 45 degree), 
c then the folding value should be 360 degree.

              plfile(jj,3)=plfile(jj,3)+360.  

           end if
              do jjj = 1,5
                 xx(jjj) = plfile(jj-jjj,0)
                 yy(jjj) = plfile(jj-jjj,3)
              enddo
              call LSE(phimn_slp,bb,xx,yy,5)
              if(phimn_slp.gt.-5.0.and.phimn_slp.lt.20.) then

                 extra=extra+(plfile(jj,0)-plfile(jj-1,0))*phimn_slp
              end if

        else

           do jjj = 0,mgood-1
              yy(jjj+1) = plfile(jj+jjj,inp)
           enddo
           call msr(phimn,sd_phidp,yy,mgood)

           if(ibegin.eq.1) then
              if(sd_phidp.lt.thrs_phi) then 
                 avgpdp=(plfile(jj,3)+plfile(jj+1,3))/2.0
                 if((avgpdp-avgbg).gt.40.0) then 
                    inwrap=1
                 end if
              end if
           else

              if(sd_phidp.lt.thrs_phi
     +          .and.plfile(jj,0).ge.r_min) then

                jcount=jcount+1
                if(jcount.eq.5) then
                   sum=0.
                   do ln=0,4
                     sum=sum+plfile(jj-ln,3)
                   enddo
                   avgbg=sum/5.0
                   ibegin=1
                   print *,'BEGIN ran/pbg/::',plfile(jj,0),avgbg
                end if
              else
                jcount=0
              end if
           end if
        end if
8       continue
c$$$$$$$$$$$$$$$$$$$$$$$$$$$$
        n = length
cccc    x(i) -- input unfolded raw phidp data;
cccc    z(i) -- updated profile;
cccc    y(i) -- filtered profile.
        ibegin = length
        do i=-5,length
           z(i)=0.0
           x(i)=0.0
        enddo

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12
c--------- FIND THE start AND stop BINs FOR FILTERING ---------------
        plfile(0,11) = plfile(1,11)
        do 1000 i=1,length

           do jjj = 0,mgood-1
              yy(jjj+1) = plfile(i+jjj,inp)
           enddo
           call msr(phimn,sd_phidp,yy,mgood)
           plfile(i,49) = sd_phidp


           SNR = plfile(i,11)
           SNRslevel = 3.
           goto (100,200),loop

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12
c ********** Find begin of GOOD data loop ***********
100        z(i) = avrg2

           if (plfile(i,ikdpst).ge.thrcrr
     +     .and.plfile(i,49).lt.thrs_phi.
     +      and.plfile(i,0).ge.1.5) then 

              count = count+1
              if (count.eq.mgood) then
                 do  il = 0, mgood-1
                    z(i-il) = plfile(i-il,inp)
                    plfile(i-il,41) = 1 ! good data mask
                 enddo

                 if (firsttime.eq.1) then
                    ibegin   = i-mgood+1 ! begin of the 1st encountered cell
                    j_arr=j_arr+1
                    ibegin_arr(j_arr)=ibegin
                    r_begin_arr(j_arr)=plfile(ibegin,0)
                    init0=(z(ibegin)+z(ibegin+1)+
     +              z(ibegin+2)+z(ibegin+3))/4.
                    init = init0    ! Recorded for local trend

                    delavg = init0 - phinit0

                 else
                    mc = i-mgood+1  ! begin of the successive encountered cells
                    j_arr=j_arr+1
                    ibegin_arr(j_arr)=mc
                    r_begin_arr(j_arr)=plfile(mc,0)
c                   print *,'j_arr',j_arr,ibegin_arr(j_arr)
                    avrg1 = (z(mc+1)+z(mc+2)+z(mc+3)+z(mc+4))/4.
                    delavg = avrg1-avrg2

                 end if


                if (firsttime.eq.1) then
                    firsttime = 0
                    init = init0
                else
                    if (avrg1.lt.init.and.avrg2.gt.init) then
                       avrg1 = avrg2
                    else if (avrg1.gt.init.and.avrg2.lt.init) then
                       avrg2 = init
                    else if (abs(delavg).gt.15..and.avrg1.lt.avrg2) then
                       avrg1 = avrg2
                    end if
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
                 end if


                 loop  = 2
                 count = 0
                 mgood = 10 
                 iend  = length
              end if
           else
              count = 0
           end if
           goto 1000

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12
c ************* Find END of GOOD DATA loop ****************  

200        z(i) = plfile(i,inp)
           plfile(i,41) = 1 ! in this branch, the gate locates on good data segment
			    ! as we are looking for next bad data.

           if(i.eq.length) then
              iend=length
              avrg2=(z(iend)+z(iend-1)+z(iend-2)+z(iend-3))/4.
              go to 1000
           end if

c  **************  changes made Feb 25 2008 ******************
           if(plfile(i,49).gt.thrs_phi .or. 
     +        plfile(i,ikdpst) .lt. thrcrr) then
c  **************  end changes made Feb 25 2008 ******************
              count = count + 1
              if (count.eq.mbad) then  !Insert test to preserve hail/BB signal.
                 zhmn = 0.
                 do 288 jj = 0, mbad-1
                    zhmn = 10.**(0.1*plfile(i-jj,1))+zhmn
                    yy(jj+1) = plfile(i-jj,ikdpst)
                    xx(jj+1) = plfile(i-jj,inp)
288              continue
                 zhmn = zhmn/float(mbad)
                 zhmn_log=10.*alog10(zhmn)
c Changing zh_mean value in BB or hail to 30 dBZ; 2/1/02 Bringi
c NOTE: BB with mean Zh<30 dBZ may be classified as "bad" data
                 if (zhmn_log.ge.25.) then
                    call msr(ymn,sd,yy,mbad)
                    call msr(amean,test_sd_phidp,xx,mbad)
c rhohv in BB could go as low as 0.6
c  checking mean rhohv in BB/hail region

c  **************  changes made Feb 25 2008 ******************
           if(ymn .ge. 0.6 .and. test_sd_phidp.lt.(thrs_phi+5.0)) then
c  **************  end changes made Feb 25 2008 ******************
                       count = 0
                       goto 1000
                    end if
                 end if
                 iend = i-mbad
                 k_arr=k_arr+1
                 iend_arr(k_arr)=iend
                 r_end_arr(k_arr)=plfile(iend,0)
c                print *,'k_arr',k_arr,iend_arr(k_arr)
                 do 210 jj = 0, mbad-1  !Inserted to clean the bad Zdr & Rhv.
                    plfile(i-jj,41) = 0. ! bad value 
210              continue
203              continue
      avrg2=(z(iend)+z(iend-1)+z(iend-2)+z(iend-3))/4.
                 if (iend.eq.length) goto 1000
                 z(i)   = avrg2
                 z(i-1) = avrg2
                 loop = 1
                 count = 0
              end if
           else
              count = 0
           end if
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
c 
           go to 600
        end if

        if (kbegin.eq.length) then
           kbegin = ibegin
        end if
   
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
c          FIR FILTER SECTION
           do i=-5,n+5
              acc=0.0
              do j=0,fir3order
                 acc=acc+fir3coef(j)*z(i-fir3order/2+j)
              enddo
              y(i)=acc*fir3gain
           enddo
c          END of FIR FILTERING
           do i=1,n
              delt=abs(x(i)-y(i))
              if (delt.ge.thres) then
                 z(i)=y(i)
              else
                 z(i)=x(i)
              end if
           enddo
9999    continue 
c*****************END LOOP for Phidp Adaptive Filtering****************************

c       PUT KDP,DELTA,PDPCORRECTED,PDPCORRECTED-FILTERED into PLFILE
        do 90 i=-5,n+3
           plfile(i,nad1)= z(i)
           plfile(i,nfl) = y(i)  
           plfile(i,ndf) = x(i)-y(i) 
90      continue

c       CALCULATE KDP
102     delta_2r=plfile(3,0)-plfile(1,0)
        do 103 i=1,n

c          Check Zh range
c          default value for nadp is 10
           nadp=10
c  **************  changes made Feb 25 2008 ******************
           if(i.gt.10.and.i.lt.n-10) then
              if(plfile(i,1).lt.20.) nadp=15
              if(plfile(i,1).ge.20.and.plfile(i,1).lt.35.)
     +                               nadp=8
              if(plfile(i,1).ge.35.) nadp=2
           end if
c  **************  end changes made Feb 25 2008 ******************
c
           do jj=0,nadp
              xx(jj+1)=plfile(i-nadp/2+jj,0)
              yy(jj+1)=plfile(i-nadp/2+jj,nfl)
           enddo

c          Improved Kdp base on LSE fit to Adap flt Phidp
           call LSE(aa,bb,xx,yy,nadp+1)
           plfile(i,nad2) = aa/2.
103     continue
c*******************END Kdp CALCULATION******************************************
c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12

600    continue

c end  kdpchill
c_____________________________________________________________

c	open(3,file='testoutp.ray',status='unknown')
c      write(6,*)'write result'

c write output file for test beam
	do k=1,length
	kdp_final(k)=plfile(k,33)
c  95 write(3,710) plfile(k, 0),plfile(k,33),plfile(k, 3)

c  710 format(3(3x,f9.4)) 
	enddo	
	return
       end
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
      end if
10    continue
      det = total * xxsum - xsum**2
      a = ( total*xysum - xsum*ysum ) / det
      b = ( ysum*xxsum - xsum*xysum ) / det
        return
      end
c-----------------------------------------------------------------

      subroutine msr(ymn,sd,y,n)

cccc  To calculate the mean (ymn) and standard deviation (sd, or,
cccc  mean square root, msr) of the array y(i) (i=1,...,n).
cccc                                               Li Liu  Sep. 19, 95

      real y(500),ymn,sd

      ysum  = 0.
      yysum = 0.
      total = float(n)
      do 10 i = 1,n
      if (abs(y(i)).gt.1.e35) then
         total = total-1.
      else
         ysum =  ysum + y(i)
      end if
10      continue
      ymn = ysum/total

      do 20 i = 1,n
      if (abs(y(i)).lt.1.e35) then
         yysum =  yysum + (y(i)-ymn)**2
      end if
20      continue
      sd = sqrt(yysum/total)

        return
      end
