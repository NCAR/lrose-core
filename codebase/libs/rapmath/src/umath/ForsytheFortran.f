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

      Subroutine LS_POLY(m,e1,n,l,x,y,c,sdev)

      integer SIZE, SIZE1
      parameter (SIZE = 1024)
      parameter (SIZE1 = 1025)
      
      real*8 x(SIZE),y(SIZE),v(SIZE),a(SIZE),b(SIZE) 
      real*8 c(SIZE1),d(SIZE),c2(SIZE),e(SIZE),f(SIZE)
      integer i,l,l2,m,n,mm1
      real*8 a1,a2,b1,b2,c1,sdev,d1,e1,f1,f2,v1,v2,w
      
      write(6, *) "ffffffffff m: ", m
      write(6, *) "ffffffffff SIZE, SIZE1: ", SIZE, SIZE1
      write(6, *) "ffffffffff e1, x[1], y[1]: ", e1, x(1), y(1)

      do i=1, n
         write(6, *) "ffffffffff ii, x[ii], y[ii]: ", i, x(i), y(i)
      end do

      mm1 = m + 1
      l = 0
      v1 = 1d7
      
!*    Initialize the arrays *!
      
      do i=1, mm1
         b(i) = 0.d0
         f(i) = 0.d0                                                            
      end do
      do i=1, n
         v(i) = 0.d0
         d(i) = 0.d0                                                                         
      end do
      
      d1 = dsqrt(dfloat(n))
      w = d1                                                                    
      do i=1, n
         e(i) = 1.d0 / w
      end do

      f1 = d1
      a1 = 0.d0                                                                                 
      do i=1, n
         a1 = a1 + x(i) * e(i) * e(i)
      end do
      c1 = 0.d0
                                                                                          
      do i=1, n
         c1 = c1 + y(i) * e(i)
      end do                                                                                            
 
      b(1) = 1.d0 / f1
      f(1) = b(1) * c1
      
      do i=1, n
         v(i) = v(i) + e(i) * c1
      end do
 
      m = 1
                                                                                              
!*    Save latest results *!

10    do i=1, l
         c2(i) = c(i)
      end do

      write(6, *) "mmmmmmmmmmmm m: ", m

      l2 = l
      v2 = v1
      f2 = f1
      a2 = a1
      f1 = 0.d0

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
         c1 = c1 + e(i) * y(i)
      end do
                                                                                             
      m = m + 1
      i = 0                                                                                   
15    l = m - i
      b2 = b(l)
      d1 = 0.d0                                                                   
      if (l > 1)  d1 = b(l - 1)
                                                                          
      d1 = d1 - a2 * b(l) - f2 * a(l)
                                                                    
      b(l) = d1 / f1
      a(l) = b2
      i = i + 1                                                               
      if (i.ne.m) goto 15
                                                                                
      do i=1, n
         v(i) = v(i) + e(i) * c1
      end do
      
      do i=1, mm1
         f(i) = f(i) + b(i) * c1
         c(i) = f(i)
         write(6, *) "cccccccccccccc i, c(i): ", i, c(i)
      end do
                                                                                             
      vv = 0.d0
      
      do i=1, n
         vv = vv + (v(i) - y(i)) * (v(i) - y(i))
      end do
                                                                                             
!*     Note the division is by the number of degrees of freedom
 
      vv = dsqrt(vv / dfloat(n - l - 1))
      l = m
      if (e1.eq.0.d0) goto 20
      
!*     Test for minimal improvement

      if (dabs(v1 - vv) / vv < e1) goto 50
      
!*    if error is larger, quit

      if (e1 * vv > e1 * v1) goto 50
      
      v1 = vv
      
20    if (m.eq.mm1) goto 30
      
      goto 10
      
!*    Shift the c[i] down, so c(0) is the constant term
                                                   
30    do i=1, l                                                                                         
        c(i - 1) = c(i)
      end do
      
      c(l) = 0.d0
      
!*    l is the order of the polynomial fitted
      l = l - 1
      sdev = vv
      return
                                                                                             
!*     Aborted sequence, recover last values                                                              
50    l = l2
      vv = v2                                                                                   
      do i=1, l
         c(i) = c2(i)
      end do
                                                                                             
      goto 30
                                                                                            
      end
                                                                                                  
                                                                                                     
!c_________________________________________________________________
!c NEW DOUBLE PRECISION ROUTINE FOR POLYNOMIAL FITTING
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
!     * The order of the fit then obtained is l.                      *
!*****************************************************************

      Subroutine LS_POLY2(m,e1,n,l,x,y,c,dd)

      integer SIZE
      parameter(SIZE=1025,ipoly=39)

!     Labels: 10,15,20,30,50

      real*8 x(SIZE),y(SIZE),v(SIZE),a(SIZE),b(SIZE)
      real*8 c(0:ipoly),d(SIZE),c2(SIZE),e(SIZE),f(SIZE)
      integer i,l,l2,m,n,n1
      real*8 a1,a2,b1,b2,c1,dd,d1,e1,f1,f2,v1,v2,w
      
!c     print *,'IN LS_POLY******** ORDER ',m

      n1 = m + 1; l=0
      v1 = 1.d7

!     Initialize the arrays

      do i=1, n1
         a(i) = 0.d0
         b(i) = 0.d0
         f(i) = 0.d0
      end do

      do i=1, n
         v(i) = 0.d0
         d(i) = 0.d0
      end do

      d1 = dsqrt(dfloat(n)); w = d1;

      do i=1, n
         e(i) = 1.d0 / w
      end do

      f1 = d1
      a1 = 0.d0

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

!     Save latest results

 10   do i=1, l
         c2(i) = c(i)
      end do

      l2 = l
      v2 = v1
      f2 = f1
      a2 = a1
      f1 = 0.d0

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

 15   l = m - i; b2 = b(l); d1 = 0.d0

      if (l > 1)  d1 = b(l - 1)

      d1 = d1 - a2 * b(l) - f2 * a(l)

      b(l) = d1 / f1
      a(l) = b2
      i = i + 1

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

      vv = dsqrt(vv / dfloat(n - l - 1))
      l = m

      if (e1.eq.0.d0) goto 20

!Test for minimal improvement

      if (dabs(v1 - vv) / vv < e1) goto 50

!if error is larger, quit

      if (e1 * vv > e1 * v1) goto 50

      v1 = vv

 20   if (m.eq.n1) goto 30

      goto 10

!     Shift the c[i] down, so c(0) is the constant term

 30   do i=1, l
         c(i - 1) = c(i)
      end do

      c(l) = 0.d0

! l is the order of the polynomial fitted

      l = l - 1
      dd = vv

!C     return m to original input order

!c     
      m=m-1

      return

!     Aborted sequence, recover last values

 50   l = l2
      vv = v2
      
      do i=1, l
         c(i) = c2(i)
      end do

      goto 30

      end




! End of file lsqrpoly.f90
