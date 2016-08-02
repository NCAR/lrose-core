
#include <math.h> /* for log */
#include <stdio.h>

/* File scope. */

float pr_thte(float p, float tc, float tdc);
float pr_tmst( float thte, float pres, float tguess);
float max(float x, float y);

/* Code taken from Fortran, Niles Oien Jan 1998 */

float calc_li(float p,
	      float tc, float tdc, float tliK,
	      float pres_li, float bad)
	{

	float Calc_li;
	float theta_e,tlift;

      if ( (p==bad)     || (tc == bad) ||
           (tdc == bad) || (tliK == bad) ) {
        Calc_li = bad;
      } else {

        theta_e = pr_thte(p,tc,tdc);
        tlift = pr_tmst(theta_e, pres_li, tliK);

        if ( tlift > 0.0 ) {
           Calc_li =  tliK - tlift;
        } else {
           Calc_li = bad;
        }
      }

      return Calc_li;

      }

/*--------------------------------------*/

float pr_thte(float p, float tc, float tdc)
      {

    float mixr,thtam,tlcl;
	float tk,tdk;
/*
c.. calculate theta_e. thtam is theta of moist air.
c.. tlcl is the temperature at the Lifted Condensation Level.
c.. from page 19-62 and 19-63 of the GEMPAK users guide.
*/

      tk  = tc  + 273.15;
      tdk = tdc + 273.15;

      tlcl = (1.0/(1.0/(tdk-56.0)+(float)log(tk/tdk)/800.))+56.0;

      mixr=1000.0*6.11*
         (2.87e6/4.615e6)/p*exp(2.5e10*(tdk-273.16)/(tdk*273.16*4.615e6));

      thtam = pow(tk*(1000/p),(.288*(1.-(.28*.001*mixr))));

      return thtam*exp((3.376/tlcl-.00254)*(mixr*(1+.81*.001*mixr)));
 
      }

/*
C************************************************************************
C* PR_TMST 								*
C*									*
C* This function computes TMST from THTE, PRES, TGUESS.  TMST is the	*
C* parcel temperature at level PRES on a specified moist adiabat 	*
C* (THTE).  The computation is an iterative Newton-Raphson technique	*
C* of the form:								*
C*	                                                                *
C*   x = x(guess) + [ f( x ) - f( x(guess) ) ] / f'( x(guess) )		*
C*	                                                                *
C*     f' is approximated with finite differences			*
C*     f' = [ f( x(guess) + 1 ) - f( x(guess) ) ] / 1			*
C*	                                                                *
C* If TGUESS is 0, a reasonable first guess will be made.		*
C*									*
C* Convergence is not guaranteed for extreme input values.  If the	*
C* computation does not converge after 100 iterations, the missing	*
C* data value will be returned.						*

I set missing to -1000.0 here - it was in a common. Niles.

C*	                                                                *
C* PR_TMST  ( THTE, PRES, TGUESS )                            		*
C*									*
C* Input parameters:							*
C*	THTE   		REAL		Equivalent potential temp in K	*
C*	PRES       	REAL    	Pressure in millibars		*
C*	TGUESS   	REAL    	First guess temperature in K	*
C*									*
C* Output parameters:							*
C*	PR_TMST		REAL		Parcel temperature in Kelvin	*
C**									*
C* Log:									*
C* P. Kocin		1980    Orginal source				*
C* M. Goodman/RDS	 9/84	Modified prologue			*
C* G. Huffman/GSC	 8/88	Recode (retain Newton-Raphson concept);	*
C*				use Mark Handel (MIT) first guess	*
C* M. desJardins/GSFC	 8/90	Move tguess into temp variable		*
C************************************************************************
C------------------------------------------------------------------------
C
*/

float pr_tmst( float thte, float pres, float tguess)

{

	float tg,epsi,tgnu,tgnup,tenu,tenup,cor;
	int i;

/*	Move tguess into another variable.
*/

	tg = tguess;
/*
C*	If TGUESS is passed as 0. it is computed from an MIT scheme.
*/
	if  ( tg == 0.0 ) {
         tg =(thte-.5*pow((max(thte-270.,0.)),1.05))*pow((pres/1000.),.2);
	}
/*
C*	Set convergence and initial guess in degrees C.
*/
	epsi = .01;
	tgnu = tg - 273.16;
/*
C*	Set a limit of 100 iterations.  Compute tenu, tenup, the
C*	THTE's at, one degree above the guess temperature.
*/
	for(i=0;i<100;i++){
	  tgnup = tgnu + 1.0;
	  tenu  = pr_thte ( pres, tgnu, tgnu );
	  tenup = pr_thte ( pres, tgnup, tgnup );

/*
C*	  Check that the THTE's exist.
C
C
C*	  Compute the correction, DELTG; return on convergence.
C
*/
	  cor  = ( thte - tenu ) / ( tenup - tenu );
	  tgnu = tgnu + cor;
	  if ( ( cor < epsi ) && ( -cor < epsi ) )  {
	      return tgnu + 273.16;
          }
	}

/*
C*	Failed to converge - return missing.
*/
        return -1000.0;
        
}

/*----------------------------------------------*/

float max(float x, float y)
{

	if (x>y) return x;
	return y;
}


