       program S_band_Kdp_CP2
	 parameter (ndim=5000)
	 real zh(ndim),zdr(ndim),phidp(ndim),rhohv(ndim)
	 real range(ndim),phi_int(ndim),kdp(ndim),kdp_final(ndim)
       real snr_s(ndim)
       integer length

c  **************  changes made Feb 25 2008 ******************
c  no need to have 'real plfile(-5:ndim,0:70)' in the main part
c  **************  end changes made Feb 25 2008 ******************

	 open(55,file='test_input.dat') 
c	 open(56,file='test_output.dat')

c File 'testcase.ray' cotains the input data plus output of csu_kdp_version5
      
c NOTE: length can change depending on size of testray file.

       length=950   ! number of gates in a beam

c read in the testray input data:range in km, raw Zh, raw Zdr, raw phidp
c and raw rhohv

	do i=1,length
          read(55,1001) range(i),zh(i),zdr(i),phidp(i),rhohv(i)
1001      format(5(2x,f10.4))

c    SNR is calculated below for C-POL, but needs input separately for CP2.
c
	  if(range(i).gt.0.) then
          snr_s(i)=zh(i)+41.25-20.*alog10(range(i))
          else
          snr_s(i)=zh(i)+41.25
        endif
       enddo

c  **************  changes made Feb 25 2008 ******************
c the name 'csu_driver' subroutine has been changed to 'csu_kdp'	
c
	call csu_kdp(length,range,zh,zdr,phidp,rhohv,snr_s,kdp_final)

c  **************  end changes made Feb 25 2008 ******************
c   ... printing results ....
		do i=1,length
	      print *,'range, zh, kdp_final =  ',range(i),zh(i),
     +             kdp_final(i)
c		  write(56,*),range(i),zh(i),kdp_final(i),phidp(i)
		enddo
	end
