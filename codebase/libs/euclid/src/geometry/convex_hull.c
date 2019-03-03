/*
 * Ken Clarkson wrote this.  Copyright (c) 1996 by AT&T..
 * Permission to use, copy, modify, and distribute this software for any
 * purpose without fee is hereby granted, provided that this entire notice
 * is included in all copies of any software which is or includes a copy
 * or modification of this software and in all copies of the supporting
 * documentation for such software.
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */


/*
 * two-dimensional convex hull
 * read points from stdin,
 *      one point per line, as two numbers separated by whitespace
 * on stdout, points on convex hull in order around hull, given
 *      by their numbers in input order
 * the results should be "robust", and not return a wildly wrong hull,
 *	despite using floating point
 * works in O(n log n); I think a bit faster than Graham scan;
 * 	somewhat like Procedure 8.2 in Edelsbrunner's "Algorithms in Combinatorial
 *	Geometry".
 */


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <euclid/geometry.h>

/* don't reset this typedef without changing geometry.h */

char input_format[] = "%lf%lf";

/* static coord points[N][2], *P[N+1];  */

int EG_read_points(double **P, double points[][2], int N) {
	int n = 0;
	char buf[100];
	while (fgets(buf, sizeof(buf), stdin)) {
                int ret = sscanf(buf, input_format, &points[n][0], &points[n][1]);
		assert(2==ret);
		P[n] = points[n];
                ++n;
		assert(n <= N);
	}
	return n;
}


#ifdef NOTNOW
void print_hull(coord **P, int m) {
	int i;
	for (i=0; i<m; i++) 
		printf("%d ", (P[i]-points[0])/2);
	printf("\n");
}
#endif

void EG_print_hull(double **P, double *start, int m) {
	int i;
	for (i=0; i<m; i++) 
		printf("%ld ", (P[i]-start)/2);
	printf("\n");
}



int ccw(double **P, int i, int j, int k) {
	double	a = P[i][0] - P[j][0],
		b = P[i][1] - P[j][1],
		c = P[k][0] - P[j][0],
		d = P[k][1] - P[j][1];
	return a*d - b*c <= 0;	   /* true if points i, j, k counterclockwise */
}


#define CMPM(c,A,B) \
	v = (*(double**)A)[c] - (*(double**)B)[c];\
	if (v>0) return 1;\
	if (v<0) return -1;

int cmpl(const void *a, const void *b) {
	double v; 
	CMPM(0,a,b);
	CMPM(1,b,a);
	return 0;
}

int cmph(const void *a, const void *b) {return cmpl(b,a);}


int make_chain(double** V, int n, int (*cmp)(const void*, const void*)) {
	int i, j, s = 1;
	double* t;

	qsort(V, n, sizeof(double*), cmp);
	for (i=2; i<n; i++) {
		for (j=s; j>=1 && ccw(V, i, j, j-1); j--){}
		s = j+1;
		t = V[s]; V[s] = V[i]; V[i] = t;
	}
	return s;
}

/* This function computes the convex hull. Note that P is an array of
   pointers to elements in the array points defined above. ch2d
   returns the number of points in the hull which we will call
   num_hp. The indices of the actual points in the hull are listed by
   the expression (P[i]-points[0])/2 for i = 0 through num_hp. */
int EG_ch2d(double **P, int n)  {
	int u = make_chain(P, n, cmpl);		/* make lower hull */
	if (!n) return 0;
	P[n] = P[0];
	return u+make_chain(P+u, n-u+1, cmph);	/* make upper hull */
}


/* Similar to EG_ch2d but also returns the indices of the vertices in the hull */
int EG_chull(double **P, double *start, int n, int *indices)
{
  int i, ret;
  int u = make_chain(P, n, cmpl);		/* make lower hull */

  if (!n) return 0;
  P[n] = P[0];

  ret = u+make_chain(P+u, n-u+1, cmph);	/* make upper hull */
  for (i=0; i<ret; i++) 
    indices[i] = (P[i]-start)/2;

    return ret;
}

#ifdef TEST_MAIN
int main(int argc, char** argv) {
  int ret = EG_read_points(P, points, N);
  ret = EG_ch2d(P, ret);
  EG_print_hull(P, points[0], ret);
  return(0);
}
#endif
