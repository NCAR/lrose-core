!****************************************************
!*      Program to demonstrate least squares        *
!*         polynomial fitting subroutine            *
!* ------------------------------------------------ *
!* Reference: BASIC Scientific Subroutines, Vol. II *
!* By F.R. Ruckdeschel, BYTE/McGRAWW-HILL, 1981 [1].*
!*                                                  *
!*                F90 version by J-P Moreau, Paris  *
!*                        (www.jpmoreau.fr)         *
!* ------------------------------------------------ *
!* SAMPLE RUN:                                      *
!*                                                  *
!* LEAST SQUARES POLYNOMIAL FITTING                 *
!*                                                  *
!* What is the order of the fit      : 4            *
!* What is the error reduction factor: 0            *
!* How many data pooints are there   : 11           *
!*                                                  *
!* Input the data points as prompted:               *
!*                                                  *
!*  1   X, Y = 1,1                                  *
!*  2   X, Y = 2,2                                  *
!*  3   X, Y = 3,3                                  *
!*  4   X, Y = 4,4                                  *
!*  5   X, Y = 5,5                                  *
!*  6   X, Y = 6,6                                  *
!*  7   X, Y = 7,7                                  *
!*  8   X, Y = 8,8                                  *
!*  9   X, Y = 9,9                                  *
!*  10   X, Y = 0,0                                 *
!*  11   X, Y = 1,1                                 *
!*                                                  *
!* Coefficients are:                                *
!*  0   -0.000000                                   *
!*  1   1.000000                                    *
!*  2   -0.000000                                   *
!*  3   0.000000                                    *
!*  4   -0.000000                                   *
!*                                                  *
!* Standard deviation = 0.000000                    *
!*                                                  *
!****************************************************
PROGRAM DEMO_LS_POLY

parameter(SIZE=25)

integer i,l,m,n
real*8  dd,e1,vv

real*8 c(0:SIZE),x(SIZE),y(SIZE) 

  print *,' '
  print *,'LEAST SQUARES POLYNOMIAL FITTING'
  
  write(*,"(' What is the order of the fit      : ')",advance='no')
  read *, m
  write(*,"(' What is the error reduction factor: ')",advance='no')
  read *, e1
  write(*,"(' How many data pooints are there   : ')",advance='no')
  read *, n
  
  print *,' '
  print *,'Input the data points as prompted:'
  
  do i=1, n
    write(*,50,advance='no') i
    read *, x(i), y(i)
  end do
  print *,' '

  call LS_POLY(m,e1,n,l,x,y,c,dd)

  print *,'Coefficients are:'
  print *,' '
  do i=0, l
    write(*,60)  i, c(i) 
  end do
  
  write(*,70)  dd
  stop 

50 format('  ',i2,'   X  Y = ')
60 format('  ',i2,'   ',f9.6)
70 format(' Standard deviation: ',f9.6//) 

end

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
Subroutine LS_POLY(m,e1,n,l,x,y,c,dd)
  parameter(SIZE=25)
  !Labels: 10,15,20,30,50
  real*8 x(SIZE),y(SIZE),v(SIZE),a(SIZE),b(SIZE)
  real*8 c(0:SIZE),d(SIZE),c2(SIZE),e(SIZE),f(SIZE)
  integer i,l,l2,m,n,n1
  real*8 a1,a2,b1,b2,c1,dd,d1,e1,f1,f2,v1,v2,w
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
10 do i=1, l
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
15 l = m - i; b2 = b(l); d1 = 0.d0
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
20 if (m.eq.n1) goto 30
  goto 10
!Shift the c[i] down, so c(0) is the constant term
30 do i=1, l  
    c(i - 1) = c(i)
  end do
  c(l) = 0.d0
  ! l is the order of the polynomial fitted
  l = l - 1; dd = vv
  return
! Aborted sequence, recover last values
50 l = l2; vv = v2
  do i=1, l  
    c(i) = c2(i)
  end do
  goto 30
end

! End of file lsqrpoly.f90
