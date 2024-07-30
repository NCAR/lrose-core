/***************************************************
 *      Program to demonstrate least squares        *
 *         polynomial fitting subroutine            *
 * ------------------------------------------------ *
 * Reference: BASIC Scientific Subroutines, Vol. II *
 * By F.R. Ruckdeschel, BYTE/McGRAWW-HILL, 1981 [1].*
 *                                                  *
 *                C++ version by J-P Moreau, Paris  *
 *                       (www.jpmoreau.fr)          *
 * ------------------------------------------------ *
 * SAMPLE RUN:                                      *
 *                                                  *
 * LEAST SQUARES POLYNOMIAL FITTING                 *
 *                                                  *
 * What is the order of the fit      : 4            *
 * What is the error reduction factor: 0            *
 * How many data pooints are there   : 11           *
 *                                                  *
 * Input the data points as prompted:               *
 *                                                  *
 *  1   X, Y = 1,1                                  *
 *  2   X, Y = 2,2                                  *
 *  3   X, Y = 3,3                                  *
 *  4   X, Y = 4,4                                  *
 *  5   X, Y = 5,5                                  *
 *  6   X, Y = 6,6                                  *
 *  7   X, Y = 7,7                                  *
 *  8   X, Y = 8,8                                  *
 *  9   X, Y = 9,9                                  *
 *  10   X, Y = 0,0                                 *
 *  11   X, Y = 1,1                                 *
 *                                                  *
 * Coefficients are:                                *
 *  0   -0.000000                                   *
 *  1   1.000000                                    *
 *  2   -0.000000                                   *
 *  3   0.000000                                    *
 *  4   -0.000000                                   *
 *                                                  *
 * Standard deviation = 0.000000                    *
 *                                                  *
 ***************************************************/
#include <stdio.h>
#include <math.h>

#define SIZE 25

typedef double TAB[SIZE+1];

int    i,l,m,n;
double dd,e1,vv;

TAB    x,y,v,a,b,c,d,c2,e,f; 

/****************************************************************
 *         LEAST SQUARES POLYNOMIAL FITTING SUBROUTINE           *
 * ------------------------------------------------------------- *
 * This program least squares fits a polynomial to input data.   *
 * forsythe orthogonal polynomials are used in the fitting.      *
 * The number of data points is n.                               *
 * The data is input to the subroutine in x[i], y[i] pairs.      *
 * The coefficients are returned in c[i],                        *
 * the smoothed data is returned in v[i],                        *
 * the order of the fit is specified by m.                       *
 * The standard deviation of the fit is returned in d.           *
 * There are two options available by use of the parameter e:    *
 *  1. if e = 0, the fit is to order m,                          *
 *  2. if e > 0, the order of fit increases towards m, but will  *
 *     stop if the relative standard deviation does not decrease *
 *     by more than e between successive fits.                   *
 * The order of the fit then obtained is l.                      *
 ****************************************************************/
void LS_POLY()  {

  //Labels: e10,e15,e20,e30,e50,fin;
  int i,l2,n1;
  double a1,a2,b1,b2,c1,d1,f1,f2,v1,v2,w;
  n1 = m + 1;
  v1 = 1e7;
  // Initialize the arrays
  for (i = 1; i < n1+1; i++) {
    a[i] = 0; b[i] = 0; f[i] = 0;
  };
  for (i = 1; i < n+1; i++) {
    v[i] = 0; d[i] = 0;
  }
  d1 = sqrt(n); w = d1;
  for (i = 1; i < n+1; i++) {
    e[i] = 1 / w;
  }
  f1 = d1; a1 = 0;
  for (i = 1; i < n+1; i++) {
    a1 = a1 + x[i] * e[i] * e[i];
  }
  c1 = 0;
  for (i = 1; i < n+1; i++) {
    c1 = c1 + y[i] * e[i];
  }
  b[1] = 1 / f1; f[1] = b[1] * c1;
  for (i = 1; i < n+1; i++) {
    v[i] = v[i] + e[i] * c1;
  }
  m = 1;
 e10: // Save latest results
  for (i = 1; i < l+1; i++)  c2[i] = c[i];
  l2 = l; v2 = v1; f2 = f1; a2 = a1; f1 = 0;
  for (i = 1; i < n+1; i++) {
    b1 = e[i];
    e[i] = (x[i] - a2) * e[i] - f2 * d[i];
    d[i] = b1;
    f1 = f1 + e[i] * e[i];
  }
  f1 = sqrt(f1);
  for (i = 1; i < n+1; i++)  e[i] = e[i] / f1;
  a1 = 0;
  for (i = 1; i < n+1; i++)  a1 = a1 + x[i] * e[i] * e[i];
  c1 = 0;
  for (i = 1; i < n+1; i++)  c1 = c1 + e[i] * y[i]; 
  m = m + 1; i = 0;
 e15: l = m - i; b2 = b[l]; d1 = 0;
  if (l > 1)  d1 = b[l - 1];
  d1 = d1 - a2 * b[l] - f2 * a[l];
  b[l] = d1 / f1; a[l] = b2; i = i + 1;
  if (i != m) goto e15;
  for (i = 1; i < n+1; i++)  v[i] = v[i] + e[i] * c1; 
  for (i = 1; i < n1+1; i++) {
    f[i] = f[i] + b[i] * c1;
    c[i] = f[i];
  }
  vv = 0;
  for (i = 1; i < n+1; i++)
    vv = vv + (v[i] - y[i]) * (v[i] - y[i]); 
  //Note the division is by the number of degrees of freedom
  vv = sqrt(vv / (n - l - 1)); l = m;
  if (e1 == 0) goto e20;
  //Test for minimal improvement
  if (fabs(v1 - vv) / vv < e1) goto e50;
  //if error is larger, quit
  if (e1 * vv > e1 * v1) goto e50;
  v1 = vv;
 e20: if (m == n1) goto e30;
  goto e10;
 e30: //Shift the c[i] down, so c(0) is the constant term
  for (i = 1; i < l+1; i++)  c[i - 1] = c[i];
  c[l] = 0;
  //l is the order of the polynomial fitted
  l = l - 1; dd = vv;
  goto fin;
 e50: // Aborted sequence, recover last values
  l = l2; vv = v2;
  for (i = 1; i < l+1; i++)  c[i] = c2[i];
  goto e30;
 fin: ;
}


int main(int argc, char **argv)  {

  printf(" LEAST SQUARES POLYNOMIAL FITTING\n");
  
  printf("\n What is the order of the fit      : "); scanf("%d",&m);
  printf("\n What is the error reduction factor: "); scanf("%lf",&e1);
  printf("\n How many data pooints are there   : "); scanf("%d",&n);
  
  printf("\n\n Input the data points as prompted:\n");
  
  for (i = 1; i < n+1; i++) {
    printf("  %d   X  Y = ",i); scanf("%lf %lf",&x[i], &y[i]);
  }
  printf("\n");

  LS_POLY();  // CALL LEAST SQUARES POLYNOMIAL FITTING SUBROUTINE

  printf("\n Coefficients are:\n");
  for (i = 0; i < l+1; i++) {
    printf("  %d   %f\n",i,c[i]); 
  }
  
  printf("\n\n Standard deviation: %f\n\n",dd); 

}

// End of file lsqply.cpp
