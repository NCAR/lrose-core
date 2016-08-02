       program S_band_Kdp_CP2
	 parameter (ndim=5000)
	 real zh(ndim),zdr(ndim),phidp(ndim),rhohv(ndim)
	 real range(ndim),phi_int(ndim),kdp(ndim),kdp_final(ndim)
       real plfile(-5:ndim,0:70),snr_s(ndim)
       integer length

	 open(55,file='testcase.ray') 

C File 'testcase.ray' cotains the input data plus output of csu_kdp_version5
      
C NOTE: length can change depending on size of testray file.

       length=512   ! number of gates in a beam

C read in the testray input data:range in km, raw Zh, raw Zdr, raw phidp
C and raw rhohv

	do i=1,length
          read(55,1001) range(i),zh(i),zdr(i),phidp(i),rhohv(i)
1001      format(5(2x,f10.4))

C    SNR is calculated below for C-POL, but needs input separately for CP2.

	  if(range(i).gt.0.) then
          snr_s(i)=zh(i)+41.25-20.*alog10(range(i))
          else
          snr_s(i)=zh(i)+41.25
        end if
       enddo

	
       call compute_kdp(length,range,zh,zdr,phidp,
     +      rhohv,snr_s,kdp_final)

C   ... printing results ....
		do i=1,length
	      print *,'i-1, kdp_final =  ',i-1,kdp_final(i)
		enddo
	end
