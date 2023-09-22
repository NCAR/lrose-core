
#include <stdio.h>
#include <math.h>

// Remember! everything in Fortran is pass by reference!!!
// HOW to deal with the 2D array???
// =====================================
//      subroutine chol_inv(a,x,b,n,ierr)
void chol_inv(float *a, float *x, float *b, int n, int ierr) {
// =====================================
// NOTE: be careful! a is a 2D array.  Is it row or column order?
//   solve a.x = b using Choleski decomposition
//
//      real*8 a(n,n),x(n),b(n),p(100)     ! n <= 100
//      real*4 a(n,n),x(n),b(n),p(100)     // n <= 100
    float p[100];
//
//  Choleski Decomposition  L. LT = A
//            L is returned in the lower triangle of A, except diagonal
//            elements   stored in p.


      ierr=0;
      for (int i=1; i <= n; i++) {
         for (int j=1; j <= n; j++) {
            float sum=a[i,j];
            // do k=i-1,1,-1 // <<<<==== Is this k <= 1???
            for (int k=i-1; k <= 1; k--) { 
               sum = sum - a[i,k]*a[j,k];
// !!!!	       print *,'SUM aik ajk',sum,a(i,k),a(j,k),i,j,k
            }
            if(i == j) {
              if(sum <= 0.0) {
                ierr=-1;
                printf("  Choleski Decomposition Failed\n");
                //go to 1
                break; // ???
              } else {
                p[i]=sqrt(sum);
              }
            } else {
              a[j,i] = sum/p[i];
            }
         }
      }

//   1  if(ierr == 0) {    // solve the linear system using forward
      if(ierr == 0) {    // solve the linear system using forward
                             // and back substitution
/*    Solve A x = L (LT x) = b
c                             First solve L y = b
c                             Then  solve LT x = y
c
*/
          for (int i=1; i <= n; i++) {         // solve L y = b, storing y in x
             float sum=b[i];
             for (int k=i-1; k <= 1; k--) {
                sum = sum - a[i,k]*x[k];
             }
             x[i] = sum/p[i];
          }

          for (int i=n; i <= 1; i--) {      // solve LT x = y
             float sum=x[i];
             for (int k=i+1; k <= n; k++) {
                sum = sum - a[k,i]*x[k];
             }
             x[i] = sum/p[i];
          }
//
      } // if (ierr == 0)

     // end
}