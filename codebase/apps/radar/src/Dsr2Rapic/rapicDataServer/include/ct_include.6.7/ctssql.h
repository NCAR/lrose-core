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

#ifndef __CTSSQL_H__
#define __CTSSQL_H__

/*************************************************************************
 *  this section includes some header files for some common compilers.   *
 *  If your compiler is not here, add the required declarations and      *
 *  includes to the final #else section.  If your system uses the old    *
 *  UNIX style variable argument list instead of the ANSI variety, you   *
 *  should #define SYSTEM_V_VARARGS.  Some compilers, especially in the  *
 *  16 bit DOS environment, work more efficiently when you #define       *
 *  STACK_VARARGS.  However, it is your responsibility to determine      *
 *  whether this works for your environment, and be aware that the       *
 *  method may stop working with a new version of the compiler or some   *
 *  other change to your environment.                                    *
 *************************************************************************/

#ifdef ctNONSQL
#define NOEXITREPLACEMENT
#endif

#ifndef ctNONSQL

#ifdef   ctPortQNX
#include <float.h>
#endif

#ifdef _MSC_VER

#include <malloc.h>
#include <float.h>
#define ANSI_VARARGS
#define ANSIFUNCS

#endif

#ifdef __IBMC__

#include <malloc.h>
#define ANSI_VARARGS
#define ANSIFUNCS

#endif

#ifdef __ZTC__

#include <stdlib.h>
#define ANSI_VARARGS
#define ANSIFUNCS

#endif

#ifdef __BORLANDC__

#include <malloc.h>
#define ANSI_VARARGS
#define ANSIFUNCS

#endif

#ifdef __WATCOMC__

#include <malloc.h>
#define ANSI_VARARGS
#define ANSIFUNCS

#endif

#ifdef __GNUC__

#include <stdlib.h>
#include <float.h>
#include <limits.h>

#ifndef FLT_MIN
#define FLT_MIN MINFLOAT
#endif

#ifndef DBL_MIN
#define DBL_MIN MINDOUBLE
#endif

#define ANSI_VARARGS
#define ANSIFUNCS

#endif

#ifdef __BSD__

/*  define using -D__BSD__ on command line  */
#define SYSTEM_V_VARARGS
#define FLT_MIN MINFLOAT
#define DBL_MIN MINDOUBLE

#endif

/*  you'll have to define this with a -D on the command line  */
#ifdef __UNIX__

#define SYSTEM_V_VARARGS

#endif

#ifndef ANSI_VARARGS
#ifndef SYSTEM_V_VARARGS

/*  you need to add a #define for your compiler or place a -D on the  */
/*  compile line!                                                     */
deliberate error.  You need to either add a ifdef/endif for your compiler,
or use a -D on the command line to select one of the defined environments.

Note that for UNIX, you should use either -D__BSD__ or -D__UNIX__ for BSD
or System V UNIX, respectively.

#endif  /*  SYSTEM_V_VARARGS  */
#endif  /*  ANSI_VARARGS      */


#ifdef ANSI_VARARGS
#ifndef ctPortQNX
#include <stdarg.h>
#endif
#endif

#ifdef SYSTEM_V_VARARGS
#include <varargs.h>
#endif

#ifndef EOF
#define EOF -1
#endif

#endif /* ~ctNONSQL */

#define SQL_MAX_NAME_LENGTH 20
#define SQL_MAX_CURSORS 50
#define SQL_NULLDISPLAY ((pSQL_C)"?")

#define SQL_EXTERN extern
#define SQL_GLOBAL /**/
#define SQL_LOCAL static
#define SQL_VOID void

typedef void *pSQL_VOID;
typedef unsigned char SQL_C;
typedef SQL_C *pSQL_C;
typedef char SQL_SC;
typedef SQL_SC *pSQL_SC;
typedef char SQL_NC;
typedef SQL_NC *pSQL_NC;
typedef int SQL_NI;
typedef SQL_NI *pSQL_NI;
typedef unsigned int SQL_UNI;
typedef SQL_UNI *pSQL_UNI;
typedef short SQL_I;
typedef SQL_I *pSQL_I;
typedef unsigned short SQL_UI;
typedef SQL_UI *pSQL_UI;
typedef long SQL_L;
typedef SQL_L *pSQL_L;
typedef unsigned long SQL_UL;
typedef SQL_UL *pSQL_UL;
typedef float SQL_F;
typedef SQL_F *pSQL_F;
typedef double SQL_D;
typedef SQL_D *pSQL_D;
typedef void *SQL_ANYPTR;
typedef SQL_ANYPTR *SQL_POINTER_ARRAY;
typedef long SQL_RECID;
typedef SQL_RECID *pSQL_RECID;
typedef unsigned int SQL_BFT;

#ifndef ctNONSQL

#ifndef SKIP_SQLDA

enum SQL_COLUMN_TYPES
{
	SQL_CHAR,
	SQL_SMALLINT,
	SQL_INTEGER,
	SQL_FLOAT,
	SQL_DOUBLE,
	SQL_VCHAR,
	SQL_DATE,
	SQL_DECIMAL,
	SQL_BOOLEAN
};

#endif  /*  SKIP_SQLDA  */

/*  host variable types  */
enum HOST_TYPE
{
	HOST_SHORT_INTEGER,
	HOST_LONG_INTEGER,
	HOST_FLOAT,
	HOST_DOUBLE,
	HOST_CHAR_ARRAY,
	HOST_CHAR_1,
	HOST_DATE_TIME,
	HOST_DECIMAL
};

#define SQL_DTPAT ((pSQL_C)"MM-DD-YYYY")
#define SQL_DECPAT ((pSQL_C)"FFFFFFFFFFFFFFFF`FFFFFFFFFFFFFFF")

typedef struct _varspec
{
	enum HOST_TYPE type;
	SQL_UNI len;
	SQL_ANYPTR ptr;
	pSQL_C pat;
	SQL_NI isindic;
} VARSPEC, *pVARSPEC;

#endif /* ~ctNONSQL */

#ifdef __CQL_H__

#define SQL_DEC NEWDEC
#define pSQL_DEC pNEWDEC

#else

typedef struct _sql_dec
{
	SQL_SC sign;
	SQL_SC scale;
	SQL_UNI dig0 : 4;
	SQL_UNI dig1 : 4;
	SQL_UNI dig2 : 4;
	SQL_UNI dig3 : 4;
	SQL_UNI dig4 : 4;
	SQL_UNI dig5 : 4;
	SQL_UNI dig6 : 4;
	SQL_UNI dig7 : 4;
	SQL_UNI dig8 : 4;
	SQL_UNI dig9 : 4;
	SQL_UNI dig10 : 4;
	SQL_UNI dig11 : 4;
	SQL_UNI dig12 : 4;
	SQL_UNI dig13 : 4;
	SQL_UNI dig14 : 4;
	SQL_UNI dig15 : 4;
} SQL_DEC, *pSQL_DEC;

#endif

#ifdef __CQL_H__

#define SQL_D_TIME D_TIME
#define pSQL_D_TIME pD_TIME

#else

typedef struct _sql_d_time
{
	SQL_UNI second : 6;
	SQL_UNI minute : 6;
	SQL_UNI hour : 5;
	SQL_UNI day : 5;
	SQL_UNI month : 4;
	SQL_UNI year : 14 ;
} SQL_D_TIME, *pSQL_D_TIME;

#endif

#ifndef ctNONSQL

struct _sqlda;

#ifndef SKIP_SQLDA

typedef struct _sqlda_col
{
	pSQL_C name;
	enum SQL_COLUMN_TYPES type;
	SQL_UNI len;
	SQL_UNI scale;
	SQL_UNI precision;
	SQL_NI not_null;
	SQL_NI unique;
	SQL_NI nullval;
	union
	{
		pSQL_C cp;
		pSQL_I ip;
		pSQL_L lp;
		pSQL_F fp;
		pSQL_D dp;
		pSQL_DEC dec;
		pSQL_D_TIME dt;
	} storage;
} SQLDA_COL, *pSQLDA_COL;

typedef struct _sqlda_seg
{
	SQL_UNI columnNumber;
} SQLDA_SEG, *pSQLDA_SEG;

typedef struct _sqlda_idx
{
	SQL_UNI keyLength;
	SQL_NI duplicatesAllowed;
	SQL_UNI nsegs;
	pSQLDA_SEG segs;
} SQLDA_IDX, *pSQLDA_IDX;

typedef struct _sqlda_tbl
{
	pSQL_C name;
	pSQL_C auth;
	SQL_NI vlflag;
	SQL_L reclen;

	/*  these members filled in during sql_TableDescribe  */
	pSQL_C fileName;
	SQL_UNI nidxs;
	pSQLDA_IDX idxs;

	struct _sqlda *tableSqlda;
	/*  this is a hook filled in on client side for decoding r_fetch  */
} SQLDA_TBL, *pSQLDA_TBL;

typedef struct _sqlda
{
	SQL_UNI ncols;
	SQL_UNI ntbls;
	SQL_UNI nltbls;
	SQL_UNI bufsiz;
	SQL_UNI stringSize;

	/*  nidxs, nsegs, and stringSpaceOffset are valid only for         */
	/*  sql_TableDescribe                                              */
	SQL_UNI nidxs;
	SQL_UNI nsegs;

	SQL_NI vlflag;
	SQL_UL LogRecNum;
	SQL_UL NumberOfRecords;
	pSQL_C tbuf;
	pSQLDA_COL cols;
	pSQLDA_TBL tbls;
} SQLDA, *pSQLDA;

#endif

typedef struct _SQL_INIT_PARAMS
{
	SQL_UNI
		maxalc,
		tbufsz,
		maxcsr,
		maxrpf,
		inbfsz,
		maxset,
		defmpc,
		defrpp,
		defmxc;

	SQL_BFT
		cqlLockingStrict : 1,
		cqlLockingRelaxed : 1,
		cqlLockingOff : 1,
		outputToTemporarySpace : 1,
		externalTransactionProcessing : 1,
		sqlSuperfiles : 1,
		libraryMode : 1;

	SQL_NI
		skipRecovery;

	pSQL_C
		authorization,
		cqldir,
		datadir,
		scriptdir,
		password,
		serverId;

} SQL_INIT_PARAMS, *pSQL_INIT_PARAMS;

typedef struct _SQL_CURSOR
{
	pSQL_VOID
		sqlHandle;

	SQL_UNI
		cursorId;

	struct _sqlda
		*sqlda;

} SQL_CURSOR, *pSQL_CURSOR;

typedef struct
{
	char c;
	SQL_C c2;
} SQL_CHAR_TEST;

typedef struct
{
	char c;
	SQL_I t1;
} SQL_SMALLINT_TEST;

typedef struct
{
	char c;
	SQL_L t1;
} SQL_INTEGER_TEST;

typedef struct
{
	char c;
	SQL_DEC t1;
} SQL_DECIMAL_TEST;

typedef struct
{
	char c;
	SQL_F t1;
} SQL_FLOAT_TEST;

typedef struct
{
	char c;
	SQL_D t1;
} SQL_DOUBLE_TEST;

typedef struct
{
	char c;
	SQL_D_TIME t1;
} SQL_DATE_TIME_TEST;

#define SQL_CHAR_PAD (sizeof(SQL_CHAR_TEST)-sizeof(SQL_C))
#define SQL_SMALLINT_PAD (sizeof(SQL_SMALLINT_TEST)-sizeof(SQL_I))
#define SQL_INTEGER_PAD (sizeof(SQL_INTEGER_TEST)-sizeof(SQL_L))
#define SQL_DECIMAL_PAD (sizeof(SQL_DECIMAL_TEST)-sizeof(SQL_DEC))
#define SQL_FLOAT_PAD (sizeof(SQL_FLOAT_TEST)-sizeof(SQL_F))
#define SQL_DOUBLE_PAD (sizeof(SQL_DOUBLE_TEST)-sizeof(SQL_D))
#define SQL_DATE_TIME_PAD (sizeof(SQL_DATE_TIME_TEST)-sizeof(SQL_D_TIME))

/* marks for output during server FETCH */

#define BEG_MARK ((pSQL_C)"^^^")
#define FLD_MARK ((pSQL_C)"@@@")
#define END_MARK ((pSQL_C)"$$$")

#ifdef TCC
#include <alloc.h>
#include <float.h>
#endif

#ifdef XV
#include <malloc.h>
#include <float.h>
#endif

#ifdef WATCOM
#include <malloc.h>
#endif

#ifdef UNIX
#include <malloc.h>
#include <float.h>
#endif


/********************************************************
 *  if any of these are undefined replace with the      *
 *  appropriate constant.  For example, on the SUN,     *
 *  replace FLT_MIN with MINFLOAT, defined in values.h  *
 ********************************************************/

#define SQL_SMLNULL SHRT_MIN
#define SQL_INTNULL LONG_MIN
#define SQL_SCALE_NULL 0xfe
#define SQL_DIGS_NULL 017
#define SQL_FLOATNULL ((SQL_F)FLT_MIN)

#define SQL_DBLNULL ((SQL_D)DBL_MIN)

#define SQL_D_TIME_NULL 0x3fff

#ifndef NOEXITREPLACEMENT

SQL_LOCAL pSQL_C sql_tbuf[50];
SQL_LOCAL SQL_UNI sql_tbufno;
SQL_LOCAL SQL_NI sql_temp;
SQL_EXTERN pSQL_VOID globalHandle;

#ifdef ctSRVR
#define exit(n) sql_exit(globalHandle, n);STPUSR();
#else
#define exit(n) sql_exit(globalHandle, n)
#endif
#define AUTHID(x) sql_initid(&globalHandle, x)

#endif

#ifdef ANSIFUNCS

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __CQL_H__
pFILSPEC sql_GetDictionaryHandle( pSQL_VOID db );
pGLOBALS sql_GetGlobals( pSQL_VOID db );
#endif

SQL_NI sql_init( pSQL_VOID *db );
SQL_NI sql_error( pSQL_VOID db, pSQL_NI error, pSQL_NI position,
				 pSQL_NI fileSystemError );
SQL_NI sql_initid( pSQL_VOID *db, pSQL_C name );
SQL_NI sql_init_extended( pSQL_VOID *db, pSQL_INIT_PARAMS ip );
SQL_VOID sql_terminate( pSQL_VOID db );
SQL_VOID sql_exit( pSQL_VOID, SQL_NI );
SQL_NI sql1( pSQL_VOID db, pSQL_C stmnt);
SQL_NI sql_DeclareCursor( pSQL_VOID db, pSQL_C stmnt, pSQL_CURSOR *cursor );
SQL_NI sql_OpenCursor( pSQL_CURSOR cursor );
SQL_NI sql_CloseCursor( pSQL_CURSOR cursor );
SQL_NI sql_ReleaseCursor( pSQL_CURSOR cursor );
SQL_NI sql_CommitWork( pSQL_VOID db );
SQL_NI sql_RollbackWork( pSQL_VOID db );
SQL_NI sql_MakeAccessible( pSQL_VOID, pSQL_C, pSQL_C );

#ifdef SQL_OLD_FUNCTION_NAMES

#define describe sql_describe
#define destbuf sql_destbuf
#define destsiz sql_destsiz
#define setdest sql_setdest
#define clsdest sql_clsdest
#define destfil sql_destfil
#define r_fetch sql_r_fetch
#define r_fetchp sql_r_fetchp
#define r_insert sql_r_insert
#define r_fastins sql_r_fastins
#define r_update sql_r_update
#define GetLogRecNum sql_GetLogRecNum
#define PositionCursor sql_PositionCursor
#define DisableLocking sql_DisableLocking

#endif  /*  SQL_OLD_FUNCTION_NAMES  */

#ifdef SYSTEM_V_VARARGS
SQL_NI sql2();
SQL_NI sql_Fetch();
SQL_NI sql_FetchPrevious();
#else
SQL_NI sql2( pSQL_VOID db, pSQL_C stmnt, SQL_UNI nv, pVARSPEC v1, ... );
SQL_NI sql_Fetch( pSQL_CURSOR cursor, SQL_UNI nv, pVARSPEC v1, ... );
SQL_NI sql_FetchPrevious( pSQL_CURSOR cursor, SQL_UNI nv, pVARSPEC v1, ... );
#endif

#ifndef SKIP_SQLDA
SQL_NI sql_describe( pSQL_CURSOR, pSQLDA, SQL_UNI, pSQL_NI, pSQL_UNI );
SQL_NI sql_TableDescribe( pSQL_VOID, pSQL_C, pSQL_C, pSQLDA, SQL_UNI,
						  pSQL_NI, pSQL_UNI );
SQL_NI sqlda_fetch( pSQL_CURSOR cursor, pSQLDA sqlda );
SQL_NI sqlda_fetchp( pSQL_CURSOR cursor, pSQLDA sqlda );
SQL_NI sqlda_sfetch( pSQL_CURSOR cursor, pSQLDA sqlda );
SQL_NI sqlda_sfetchp( pSQL_CURSOR cursor, pSQLDA sqlda );
#endif

SQL_NI cqlprnt( pSQL_VOID db, pSQL_C stmnt );
SQL_VOID sql_destbuf( pSQL_VOID db, pSQL_C addr, SQL_UNI bufsiz );
SQL_UNI sql_destsiz( pSQL_VOID );
SQL_NI sql_setdest( pSQL_VOID db, pSQL_C filename );
SQL_VOID sql_clsdest( pSQL_VOID );
SQL_VOID sql_destfil( pSQL_VOID );
SQL_NI cql1( pSQL_VOID, pSQL_C, SQL_ANYPTR, SQL_UNI, pSQL_UNI );
SQL_NI cql2( pSQL_VOID,SQL_ANYPTR,SQL_UNI,pSQL_UNI,pSQL_C,SQL_UNI,pVARSPEC* );
SQL_NI cql( pSQL_VOID, pSQL_C, SQL_ANYPTR, SQL_UNI, pSQL_UNI );
SQL_NI sql_r_fetch( pSQL_CURSOR cursor, SQL_POINTER_ARRAY DataPointers);
SQL_NI sql_ServerFetch( pSQL_CURSOR, pSQL_UNI );
SQL_NI sql_ServerFetchPrevious( pSQL_CURSOR, pSQL_UNI );
SQL_NI sql_ServerRFetch( pSQL_CURSOR cursor, pSQL_UNI outputLength );
SQL_NI sql_r_fetchp( pSQL_CURSOR cursor, SQL_POINTER_ARRAY DataPointers );
SQL_NI sql_ServerRFetchPrevious( pSQL_CURSOR cursor, pSQL_UNI outputLength );
SQL_NI sql_r_insert( pSQL_CURSOR cursor, SQL_POINTER_ARRAY DataPointers );
SQL_NI sql_ServerRInsert( pSQL_CURSOR cursor, pSQL_C data, SQL_UNI length );
SQL_NI sql_r_fastins( pSQL_CURSOR cursor, SQL_POINTER_ARRAY DataPointers );
SQL_NI sql_r_update( pSQL_CURSOR cursor, SQL_POINTER_ARRAY DataPointers );
SQL_NI sql_ServerRUpdate( pSQL_CURSOR cursor, pSQL_C data, SQL_UNI length );
SQL_NI sql_GetLogRecNum( pSQL_CURSOR cursor, pSQL_UL recnum );
SQL_NI sql_PositionCursor( pSQL_CURSOR cursor, SQL_UL LogRecNum );
SQL_VOID sql_DisableLocking( pSQL_VOID );
SQL_VOID sql_StrictLocking( pSQL_VOID );
SQL_VOID sql_RelaxedLocking( pSQL_VOID );

#ifndef __PSDECL_H__
SQL_ANYPTR xalloc(SQL_UNI size);
#endif

#ifdef __cplusplus
}
#endif

#else

SQL_NI sql_init();
SQL_NI sql_error();
SQL_VOID sql_exit();
SQL_NI sql1();
SQL_NI sql2();
SQL_NI describe();
SQL_NI sqlda_fetch();
SQL_NI sqlda_fetchp();
SQL_NI sqlda_sfetch();
SQL_NI sqlda_sfetchp();
SQL_NI cqlprnt();
SQL_VOID destbuf();
SQL_UNI destsiz();
SQL_NI setdest();
SQL_VOID clsdest();
SQL_VOID destfil();
SQL_NI cql1();
SQL_NI cql2();
SQL_NI cql();
SQL_NI r_fetch();
SQL_NI r_fetchp();
SQL_NI r_insert();
SQL_NI r_fastins();
SQL_NI r_update();
SQL_NI GetLogRecNum();
SQL_NI PositionCursor();
SQL_VOID DisableLocking();
SQL_NI sql_MakeAccessible();

#ifndef __PSDECL_H__
SQL_ANYPTR xalloc();
#endif  /*  __PSDECL_H__  */

#endif  /*  ANSIFUNCS  */

#endif /* ~ctNONSQL */

#endif  /*  __CTSSQL_H__  */
