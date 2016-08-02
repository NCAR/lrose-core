/* 16 bit Windows 3.1x */
#define ctLP
#define ctEXCLCREAT
#define ct_NULL (char *) 0

#ifdef _MT  /* From stddef.h for multi-thread */
#define OWNER	(*_threadid)
#define apOWNER (*_threadid)
#else
#define OWNER	(1)
#define apOWNER (1)
#endif

#define ctLSCAN		/* implies standard model fscanf/printf */

#ifndef SIZEOF
#define SIZEOF 	sizeof
#endif

#define ctrt_fclose		fclose
#define ctrt_strcpy(a,b)	_fstrcpy((pTEXT) (a),(pTEXT) (b))
#define ctrt_strcat(a,b)	_fstrcat((pTEXT) (a),(pTEXT) (b))
#define ctrt_strcmp(a,b)	_fstrcmp((pTEXT) (a),(pTEXT) (b))
#define ctrt_strncmp(a,b,n) 	_fstrncmp((pTEXT) (a), (pTEXT)(b), n)
#define ctrt_strlen(a)		_fstrlen((pTEXT) (a))
#define ctrt_strncpy(a,b,n)	_fstrncpy((pTEXT) (a),(pTEXT) (b),n)
#define ctrt_memcmp(a,b,n)	_fmemcmp((pVOID) (a),(pVOID) (b),n)
#define ctrt_memchr(a,b,n)	_fmemchr((pVOID) (a),(NINT) (b),n)
#define ctrt_putc(a,b)          putc((int) a, (FILE *) b);
#define ctrt_toupper          	toupper
#if (defined(NOTFORCE) && defined(TRANPROC))
#define ctrt_getcwd		ctGetCwd /* For Qualifing TRANPROC LOG Paths */
#endif

#define memmove                 _fmemmove
#define ctALLOC
extern void _far * ct_alloc(unsigned int n, unsigned int size);
#define BIGDELM

#define icpylng(dp,sp,n) *dp++ = *sp++; *dp++ = *sp++; *dp++ = *sp++; *dp++ = *sp++
#define cpylng(dp,sp,n)  cpy4bf(dp,sp)
#define cpy4bf(dp,sp)    _fmemcpy((pVOID) (dp),(pVOID) (sp),4)
#define cpybuf(dp,sp,n)  _fmemcpy((pVOID) (dp),(pVOID) (sp),(UINT) n)
#define ctsfill(dp,ch,n) _fmemset((pVOID) (dp),ch,n)

#ifndef __WATCOMC__
#define FASTCOMP
#define BIGCHECK
#endif

#define ctcfill          ctcbill
#define FASTRIGHT

#ifndef ctCONV
#define ctCONV
#endif

#ifndef ctDECL
#define ctDECL WINAPI _loadds 
#endif

#ifndef ctDECLV
#define ctDECLV _loadds 
#endif

#define ctMEM  far
#define ctFILE_ACCESS /* Drive Access control */

void cpynbuf(char far *,char far *,unsigned int);
void cpybig(char far *,char far *,long);
void ctafree(void far *);
int ctcfill(char far *,int,long);

extern ctCONV void  	ctDECL	ctrt_exit( int ret );
extern ctCONV int   	ctDECLV	ctrt_fscanf( FILE *fp, char far *fmt, ... );
extern ctCONV void  	ctDECLV	ctrt_printf( char far *fmt, ... );
extern ctCONV void  	ctDECLV	ctrt_sprintf( char far *, char far *, ... );
extern ctCONV void  	ctDECLV	ctrt_fprintf( FILE *fp, char far *fmt, ... );
extern ctCONV FILE *	ctDECL	ctrt_fopen(char far *,char far *);
extern ctCONV char far* ctDECL	ctrt_tmpnam( char far *);

/****************************************************************************/
/* ntree needs */
#define NETFRMAT

#define WNDNET
#ifndef DllLoadDS
#define DllLoadDS 	_loadds
#endif
#define cpymbuf		cpybuf
#define cpymsg		cpybuf
#define cpysrcm		cpysrcl

/* Banyan Vines */
#ifdef VINES
#define ctPROTBYTES
#define ctBAN5
#define	NWPTR	FAR *
#endif
/****************************************************************************/

/* end of ctcmpl_w.h */
