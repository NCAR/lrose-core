c                                                                     c
c*********************************************************************c
c                                                                     c
      subroutine thecalc(prs,tmk,qvp,the,miy,mjx,mkzh)
c
      dimension prs(miy,mjx,mkzh),tmk(miy,mjx,mkzh),qvp(miy,mjx,mkzh),
     &   the(miy,mjx,mkzh)
c
      include 'comconst'
c
      do 1000 k = 1, mkzh
      do 1000 j = 1, mjx-1
      do 1000 i = 1, miy-1
         gammam=gamma*(1.+gammamd*qvp(i,j,k))
         the(i,j,k)=tmk(i,j,k)*(1000./prs(i,j,k))**gammam
 1000 continue
      return
      end
