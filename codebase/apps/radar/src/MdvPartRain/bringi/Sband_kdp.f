      subroutine compute_kdp
     +     (length,range,zh,zdr,phidp,
     +     rhohv,snr_s,kdp_final)

C      COMMON DATA DECLARATION
       parameter (ndim=5000)
       real zh(ndim),zdr(ndim),phidp(ndim),rhohv(ndim)
       real range(ndim),phi_int(ndim),kdp(ndim)
       real plfile(-5:ndim,0:70),kdp_final(ndim),snr_s(ndim)
       integer length
C       common /PACK/ plfile
C file 'testray.dat' contains an ascii test file of
C input data: range in km, raw Zh, raw Zdr, raw phidp and raw rhohv
C The sample testray.dat provided is for CPOL beam from 21 Jan 1999

	  integer firsttime,count
        integer ibegin_arr(20),iend_arr(20)
        integer fir3order
        real x(-90:ndim),y(-90:ndim),z(-200:ndim),z_cns(ndim)
        real xx(31),yy(31),fir3coef(0:20)
        real init0,init,thres
        real fir3gain,elevation
        real r_begin_arr(20),r_end_arr(20)
        real phinit0,r_min
C        common /FIR/ fir3order,fir3gain,fir3coef

C NEXT THREE DATA STATEMENTS CONTAIN THE FIR FILTER ORDER, GAIN AND COEFICIENTS
C The specification of FIR filter coeficients is set for gate spacing of 150 meters.

       data fir3order/20/
       data fir3gain/1.044222/
       data (fir3coef(i), i=0,20)
     +  /1.625807356e-2, 2.230852545e-2, 2.896372364e-2,
     +   3.595993808e-2, 4.298744446e-2, 4.971005447e-2,
     +   5.578764970e-2, 6.089991897e-2, 6.476934523e-2,
     +   6.718151185e-2, 6.80010000e-2,  6.718151185e-2,
     +   6.476934523e-2, 6.089991897e-2, 5.578764970e-2,
     +   4.971005447e-2, 4.298744446e-2, 3.595993808e-2,
     +   2.896372364e-2, 2.230852545e-2, 1.625807356e-2/ 

      
C First set up the initialization routine

        do i=1,length
          plfile(i,0)=range(i)
          plfile(i,1)=zh(i)
          plfile(i,2)=zdr(i)
C Generate Zv field

          plfile(i,7)=plfile(i,1)-plfile(i,2)
          plfile(i,3)=phidp(i)
          plfile(i,5)=rhohv(i)
          plfile(i,11)=snr_s(i)

C Estimate SNR using radar constant of 72 and noise floor of -115 dBm
C NEED TO FINE TUNE FOR CP2!!! if exact SNR is required


          r = plfile(i,0)
        enddo

C        do jj = 1,length
C           print *,'i,range,phidp',jj,plfile(jj,0),plfile(jj,3)
C        enddo

C Now call the major routine that calculates Kdp,
C does attenuation correction of Zh and Zdr, calculates
C gamma dsd paramters, 'pol tuned' Z-R, and other rainrate
C estimates


        phinit0 = -999 !automatically decide initial phase

	r_min = 5	! the minimum range in km depends on radar system, e.g. beyond far field.

        nad1 = 31          ! label(31)='Pdpadap'
        nfl  = 32          ! label(32)='Pdpadfl'
        nad2 = 33          ! label(33)='Kdpadap'
        ndf  = 34          ! label(34)='Delta'
        thres = 3.	! this value depends on the expected backscatter diff phase
        madflt = 10
        thrs_phi=6 ! this value for CP2
	  thrcrr=0.25   ! set low since not used
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
C A 'good' data mask in set in plfile(i,41) as real 0.0 (bad) and 1.0 (good)
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

C This is Yanting's modified unfold code 
        do 8 jj = 1,length

C           print *,"11111111 jj: ",jj

          plfile(jj,63) = plfile(jj,3)  ! save the raw phidp to checking unfold

        if(inwrap.eq.1) then
           if(plfile(jj,3).lt.(avgbg-thrs_unfold+extra)) then

C Depending on the pulsing mode, if alternate H/V, the folding value should be 
C changed to 180 degrees; if simultaneous transmission mode (slant 45 degree), 
C then the folding value should be 360 degree.

              plfile(jj,3)=plfile(jj,3)+360.  

           end if
              do jjj = 1,5
                 xx(jjj) = plfile(jj-jjj,0)
                 yy(jjj) = plfile(jj-jjj,3)
              enddo
              call LSE(phimn_slp,bb,xx,yy,5)
C              print *, "5555 phimn_slp, bb: ",phimn_slp,bb
              if(phimn_slp.gt.-5.0.and.phimn_slp.lt.20.) then

                 extra=extra+(plfile(jj,0)-plfile(jj-1,0))*phimn_slp
              end if

        else

           do jjj = 0,mgood-1
              yy(jjj+1) = plfile(jj+jjj,inp)
C              print *,"3333 yy: ",yy(jjj+1)
           enddo
           call msr(phimn,sd_phidp,yy,mgood)
C           print *,"4444 phimn,sd_phidp: ",phimn,sd_phidp

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
C                   print *,'BEGIN ran/pbg/::',plfile(jj,0),avgbg
                end if
              else
                jcount=0
              end if
           end if
        end if
8       continue

c$$$$$$$$$$$$$$$$$$$$$$$$$$$$
        n = length
C    x(i) -- input unfolded raw phidp data;
C    z(i) -- updated profile;
C    y(i) -- filtered profile.
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
C              print *,"2222 i,jjj,yy: ",i,jjj,yy(jjj+1)
           enddo
           call msr(phimn,sd_phidp,yy,mgood)
           plfile(i,49) = sd_phidp


           SNR = plfile(i,11)
           SNRslevel = 3.
           goto (100,200),loop

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12
C ********** Find begin of GOOD data loop ***********
100        z(i) = avrg2

           if (plfile(i,ikdpst).ge.thrcrr
     +     .and.plfile(i,49).lt.thrs_phi.
     +      and.plfile(i,0).ge.1.5) then 

              count = count+1

C              print *,'count,rhohv,sdphipdp,range:',
C     +                 count,plfile(i,ikdpst),plfile(i,49),plfile(i,0)

              if (count.eq.mgood) then
                 do  il = 0, mgood-1
                    z(i-il) = plfile(i-il,inp)
                    plfile(i-il,41) = 1 ! good data mask
C                    print*,'AAAA i,il,i-il,z: ',i,il,i-il,z(i-il)
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
C                    print *,"7777 init0,delavg: ",init0,delavg
C                    print *,"7777 j_arr, ibegin: ",j_arr,ibegin


                 else
                    mc = i-mgood+1  ! begin of the successive encountered cells
                    j_arr=j_arr+1
                    ibegin_arr(j_arr)=mc
                    r_begin_arr(j_arr)=plfile(mc,0)
                    avrg1 = (z(mc+1)+z(mc+2)+z(mc+3)+z(mc+4))/4.
                    delavg = avrg1-avrg2
C                    print *,'8888 j_arr,mc',j_arr,mc
C                    print *,"8888 avrg1,avrg2,delavg: ",
C     +                       avrg1,avrg2,delavg

                 end if


                if (firsttime.eq.1) then
                    firsttime = 0
                    init = init0
                else
C                    print *,"BBBB init,delavg,avrg1,avrg2: ",init,delavg,avrg1,avrg2
                    if (avrg1.lt.init.and.avrg2.gt.init) then
                       avrg1 = avrg2
                    else if (avrg1.gt.init.and.avrg2.lt.init) then
                       avrg2 = init
                    else if (abs(delavg).gt.15..and.avrg1.lt.avrg2) then
                       avrg1 = avrg2
                    end if
C                    print *,"CCCC init,avrg1,avrg2: ",init,avrg1,avrg2
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
C ************* Find END of GOOD DATA loop ****************  

200        z(i) = plfile(i,inp)
           plfile(i,41) = 1 ! in this branch, the gate locates on good data segment
			    ! as we are looking for next bad data.

           if(i.eq.length) then
              iend=length
              avrg2=(z(iend)+z(iend-1)+z(iend-2)+z(iend-3))/4.
C              print *,"DDDD avrg2:",avrg2
              go to 1000
           end if

C           print *,"GGGG i,z,sdphidp:",i,z(i),plfile(i,49)
C           print *,"thrs_phi: ",thrs_phi

           if(plfile(i,49).gt.thrs_phi) then
C              print*,"HHHH, count,mbad",count,mbad
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
C Changing zh_mean value in BB or hail to 30 dBZ; 2/1/02 Bringi
C NOTE: BB with mean Zh<30 dBZ may be classified as "bad" data
                 if (zhmn_log.ge.25.) then
                    call msr(ymn,sd,yy,mbad)
                    call msr(amean,test_sd_phidp,xx,mbad)
C rhohv in BB could go as low as 0.6
C  checking mean rhohv in BB/hail region
           if(test_sd_phidp.lt.(thrs_phi+7.5)) then

                       count = 0
                       goto 1000
                    end if
                 end if
                 iend = i-mbad
C                 print *,"FFFF mbad,iend:",mbad,iend
                 k_arr=k_arr+1
                 iend_arr(k_arr)=iend
                 r_end_arr(k_arr)=plfile(iend,0)
C                print *,'k_arr',k_arr,iend_arr(k_arr)
                 do 210 jj = 0, mbad-1  !Inserted to clean the bad Zdr & Rhv.
                    plfile(i-jj,41) = 0. ! bad value 
210              continue
                 avrg2=(z(iend)+z(iend-1)+z(iend-2)+z(iend-3))/4.
C                 print *,"EEEE iend,avrg2:",iend,avrg2
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

        do i=1,length
           if (plfile(i,41).gt.0) then
C              print*,"i,goodmask: ",i,plfile(i,41)
           else
C              print*,"9999 i,********: ",i,plfile(i,41)
           end if
        enddo

c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12

C       Save max value of k_arr and j_arr
        k_max_arr=k_arr
        j_max_arr=j_arr
C       print *,'k_max_arr',k_max_arr
C       print *,'j_max_arr',j_max_arr
c---------------- END of FINDING start AND stop BINs -------------------

        if(ibegin.eq.length) then  !NO good data in whole ray. RETURN.
C           print *, 'No good data in entire beam!'
C 
           go to 600
        end if

        if (kbegin.eq.length) then
           kbegin = ibegin
        end if
   
        ext = avrg2

C        print*,"JJJJ ibegin,kbegin,ext: ",ibegin,kbegin,ext
C        print*,"init0:",init0

        do i=1,89+kbegin   !Set the initial conditions
           z(kbegin-i) = init0
        enddo
        do i=1,n-iend+30   !Extend data record
           z(iend+i)=ext
        enddo
        do i=-90,length+30 !Adjust raw data array
           x(i)= z(i)
        enddo
 
        irl=1
c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12
c------------- MAIN LOOP of Phidp Adaptive Filtering --------------------
C        print*,"iend:",iend
        
C        do i = -90,length+30
C           print *,'i,z:',i,z(i)
C        enddo
        do 9999 mloop=1,madflt
C          TIE DOWN THE INITIAL and EXTENDING DATA RECORD
           do i=irl,kbegin+89
              z(kbegin-i)=init0
           enddo
           do i=1,n-iend+30
              z(iend+i)=ext
           enddo

C        do i = -90,length+30
C           print *,'222 i,z:',i,z(i)
C        enddo

C          FIR FILTER SECTION
           do i=-5,n+5
              acc=0.0
              do j=0,fir3order
                 jjj = i-fir3order/2+j
                 acc=acc+fir3coef(j)*z(jjj)
C                 print*,"PPPP,j,jjj,acc,coeff,z: ",
C     +                  j,jjj,acc,fir3coef(j),z(jjj)
              enddo
              y(i)=acc*fir3gain
C              print *,"QQQQ mloop,i,y(i):",mloop,i,y(i)
           enddo
C          END of FIR FILTERING
           do i=1,n
              delt=abs(x(i)-y(i))
              if (delt.ge.thres) then
                 z(i)=y(i)
              else
                 z(i)=x(i)
              end if
           enddo
9999    continue 

C        do i = -90,length+30
C           print *,'i,z:',i,z(i)
C        enddo

C        print*,"KKKK"

c*****************END LOOP for Phidp Adaptive Filtering****************************

C       PUT KDP,DELTA,PDPCORRECTED,PDPCORRECTED-FILTERED into PLFILE
        do 90 i=-5,n+3
           plfile(i,nad1)= z(i)
           plfile(i,nfl) = y(i)  
           plfile(i,ndf) = x(i)-y(i)
90      continue

C        do i = -5,length+3
C           print *,'YYYY i,y:',i,y(i)
C        enddo

C       CALCULATE KDP
102     delta_2r=plfile(3,0)-plfile(1,0)
        do 103 i=1,n

C          Check Zh range
C          default value for nadp is 10
           nadp=10
           if(i.gt.15.and.i.lt.n-15) then
              if(plfile(i,1).lt.35.) nadp=30
              if(plfile(i,1).ge.35.and.plfile(i,1).lt.45.)
     +                               nadp=20
              if(plfile(i,1).gt.45.) nadp=10
           end if
C           print*,"i,zh,nadp:",i,plfile(i,1),nadp
           do jj=0,nadp
              xx(jj+1)=plfile(i-nadp/2+jj,0)
              yy(jj+1)=plfile(i-nadp/2+jj,nfl)
C              print*,"jj,xx,yy:",jj,xx(jj+1),yy(jj+1)
           enddo

C          Improved Kdp base on LSE fit to Adap flt Phidp
           call LSE(aa,bb,xx,yy,nadp+1)
C           print*,"nadp,aa,bb:",nadp,aa,bb
           plfile(i,nad2) = aa/2.
103     continue
c*******************END Kdp CALCULATION******************************************
c23456789-123456789-123456789-123456789-123456789-123456789-123456789-12

600    continue

C end  kdpchill
c_____________________________________________________________

c	open(3,file='testoutp.ray',status='unknown')
C      write(6,*)'write result'

C write output file for test beam
	do k=1,length
	kdp_final(k)=plfile(k,33)
C  95 write(3,710) plfile(k, 0),plfile(k,33),plfile(k, 3)

C  710 format(3(3x,f9.4)) 
	enddo	
	return
       end
c_____________________________________________________________

      subroutine LSE(a,b,x,y,n)

C    This is a Linear Least Square Estimate subroutine to fit a linear
C    equation for (xi,yi) (i=1,...,n), so that
C                            yi = a * xi + b
C    INPUTs: x(i), y(i), n, (i=1,...,n ).
C    OUTPUTs: a ( slope ), b ( intercept ).
C                                                Li Liu   Sep. 23, 92

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
C      print *, "6666 total,xsum,ysum,xxsum,xysum: ",
C     + total,xsum,ysum,xxsum,xysum
C      print *,"det,a,b: ",det,a,b
        return
      end
c-----------------------------------------------------------------

      subroutine msr(ymn,sd,y,n)

C  To calculate the mean (ymn) and standard deviation (sd, or,
C  mean square root, msr) of the array y(i) (i=1,...,n).
C                                               Li Liu  Sep. 19, 95

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
