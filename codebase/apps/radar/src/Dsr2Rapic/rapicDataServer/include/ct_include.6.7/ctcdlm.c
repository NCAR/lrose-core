/*
 *      OFFICIAL NOTIFICATION: the following CONFIDENTIAL and PROPRIETARY 
 * 	property legend shall not be removed from this source code module 
 * 	for any reason.
 *
 *	This program is the CONFIDENTIAL and PROPRIETARY property 
 *	of FairCom(R) Corporation. Any unauthorized use, reproduction or
 *	transfer of this computer program is strictly prohibited.
 *
 *      Copyright (c) 1984 - 1997 FairCom Corporation.
 *	This is an unpublished work, and is subject to limited distribution and
 *	restricted disclosure only. ALL RIGHTS RESERVED.
 *
 *			RESTRICTED RIGHTS LEGEND
 *	Use, duplication, or disclosure by the Government is subject to
 *	restrictions set forth in subparagraph (c)(1)(ii) of the Rights in
 * 	Technical Data and Computer Software clause at DFARS 252.227-7013.
 *	FairCom Corporation, 4006 West Broadway, Columbia, MO 65203.
 *
 *	c-tree PLUS(tm)	Version 6.7
 *			Release A2
 *			August 1, 1997
 */

/*
** find delimeter
*/

#ifndef HA_OFFSET
#define HA_OFFSET(vp)	((((pUCOUNT) &(vp)))[0])
#define HA_SELCTR(vp)	((((pUCOUNT) &(vp)))[1])
#define BLOCK		((UCOUNT) 63488)
#endif

#ifdef ctrt_memchr
#ifdef MSWINDOWS
#ifndef WIN32
extern pVOID _fmemchr(const void far *,int,size_t);
#endif
#else
#ifndef PROTOTYPE
/* see ctopt2.h for ctMEMCHRtxt default */
/* extern ctMEMCHRtxt ctrt_memchr(); */
#endif
#endif
#endif

#ifdef BIGDELM
#ifndef ctrt_memchr
HIGH-SPEED BIGDELM CANNOT BE USED UNLESS ctrt_memchr is defined
#endif

#ifdef PROTOTYPE
VRLEN ctEXPORT ctDECL ctcdelm(pTEXT tp,NINT ch,VRLEN len)
#else
VRLEN ctEXPORT ctDECL ctcdelm(tp,ch,len)
pTEXT tp;
NINT ch;
VRLEN len;
#endif
{
	UCOUNT d_off,smlen;
	VRLEN  prvpos  = 0L;
	VRLEN  delmpos = 0L;
	pTEXT  delmadr;

	/* returns zero if no delimiter, or length including delimiter */

loop:
	d_off = HA_OFFSET(tp);
	if (len < 65536L) {
		smlen = (UCOUNT) len;
		if (!d_off || (d_off + smlen) >= d_off) {
			if (delmadr = ctrt_memchr(tp,ch,smlen))
				delmpos  = (delmadr - tp) + 1 + prvpos;
		} else {
			d_off = (~d_off) + 1;
			if (delmadr = ctrt_memchr(tp,ch,d_off)) {
				delmpos  = (delmadr - tp) + 1 + prvpos;
			} else {
				prvpos += d_off;
				HA_SELCTR(tp) += cthshift;
				HA_OFFSET(tp)  = 0;
				if (delmadr = ctrt_memchr(tp,ch,smlen - d_off))
					delmpos  = (delmadr - tp) + 1 + prvpos;
			}
		}
		return(delmpos);
	}

	if (d_off) {
		d_off = (~d_off) + 1;
		if (delmadr = ctrt_memchr(tp,ch,d_off))
			return((delmadr - tp) + 1 + prvpos);
		else
			prvpos += d_off;
		HA_SELCTR(tp) += cthshift;
		HA_OFFSET(tp)  = 0;
		len	      -= d_off;
	} else {
		if (delmadr = ctrt_memchr(tp,ch,BLOCK))
			return((delmadr - tp) + 1 + prvpos);
		else
			prvpos += BLOCK;
		HA_OFFSET(tp) = BLOCK;
		len	     -= BLOCK;
	}
	goto loop;

}
#else /* BIGDELM */
#ifdef ctLP

#ifdef PROTOTYPE
VRLEN ctEXPORT ctDECL ctcdelm(pTEXT tp,NINT ch,VRLEN len)
#else
VRLEN ctEXPORT ctDECL ctcdelm(tp,ch,len)
pTEXT tp;
NINT ch;
VRLEN len;
#endif
{
	pTEXT	sp = tp;
	ULONG	retval;
	char huge *hsp;
	char huge *htp;

	if (len < 65536L) {
#ifdef ctrt_memchr
		tp = (pTEXT)ctrt_memchr(tp,ch,(ctSIZET) len);
		if (tp)
			retval = (tp - sp) + 1;
		else
			retval = 0L;
#else
		while (len > 0 && *tp++ != ch)
			--len;
		if (len)
			retval = tp - sp;
		else
			retval = 0L;
#endif
		return(retval);
	}

	hsp = htp = (char huge *) tp;
	while (len > 0 && *htp++ != ch)
		--len;
	if (len)
		retval = htp - hsp;
	else
		retval = 0L;
	return(retval);
}
#else /* ctLP */

#ifdef PROTOTYPE
VRLEN ctEXPORT ctDECL ctcdelm(pTEXT tp,NINT ch,VRLEN len)
#else
VRLEN ctEXPORT ctDECL ctcdelm(tp,ch,len)
pTEXT tp;
NINT ch;
VRLEN len;
#endif
{
	pTEXT	sp = tp;
	ULONG	retval;

#ifdef ctrt_memchr
	tp = (pTEXT)ctrt_memchr(tp,ch,(ctSIZET) len);
	if (tp)
		retval = (tp - sp) + 1;
	else
		retval = 0L;
#else
	while (len > 0 && *tp++ != ch)
		--len;
	if (len)
		retval = tp - sp;
	else
		retval = 0L;
#endif
	return(retval);
}
#endif /* ctLP */
#endif /* BIGDELM */

/* end of ctcdlm.c */
