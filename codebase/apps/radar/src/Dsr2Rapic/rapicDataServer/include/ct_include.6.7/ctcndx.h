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

#ifndef __CTCNDX_H
#define __CTCNDX_H
#ifndef ctPORTH
#include "ctstdr.h"
#include "ctoptn.h"
#include "ctstrc.h"
#endif /* ~ctPORH */

#ifdef ctCLIENT
#define getcndxmem(a)   mballc(1,(UINT)(a))
#define putcndxmem(a)   mbfree((pVOID)(a))
#define putcndxmemn(a)  mbfren((ppVOID)(a))
#define RES_msgsuppress 0
#define gcidx(a,b,c)    GETCRES(a,b,c)
#else
#define getcndxmem(a)   ctgetmem((VRLEN)(a))
#define putcndxmem(a)   ctputmem((pTEXT)(a))
#define putcndxmemn(a)  ctputmemn((ppVOID)(a))
#define RES_msgsuppress RES_SRVR
#define gcidx(a,b,c)    iGETCRES(a,b,c,NO)
#endif

#define	YYLMAX		100		/* token and pushback buffer size */
#ifndef yy_state_t
#define yy_state_t NINT
#endif

struct LEXINST_S
{
	unsigned char* yyInput;
	int		yyInputSize;
	int		yyPointer;
	int		yylineno;
	int		yyleng;
	unsigned char	yytext[YYLMAX+1];
	yy_state_t	yy_sbuf[YYLMAX+1];
	int		yy_end;
	int		yy_start;
	int		yy_lastc;
	unsigned char 	yy_save;
};

#define CNDX_MAX_UINT           "4294967295"
#define CNDX_MAX_INT            "2147483674"
#define CNDX_MAX_TEXT           255
#define CNDX_MAX_STACK          512
#define CNDX_DEBUG              0

#define ERY_NONE                0       /* No errors detected */
#define ERY_SYNTAX              1       /* Syntax error detected */
#define ERY_TYPE                2       /* Invalid type mixup */
#define ERY_FIELD               3       /* Unknown field name */
#define ERY_INTERN              4       /* Internal yacc error */
#define ERY_MEMORY              5       /* Memory allocation failed */
#define ERY_OVERFLOW            6       /* Stack overflow */
#define ERY_UNDERFLOW           7       /* Stack Underflow */
#define ERY_EXEC                8       /* Invalid execution node */
#define ERY_DIVISION            9       /* Division by zero */
#define ERY_NOSCHEMA            10
#define ERY_NORECBUF            11

enum PT_NODE_TYPES
{
	PT_NONE,

	/*
	 * Data Fields
	 */
	PT_FIELD,
	
	/*
	 * Terminators
	 */
	PT_SIGNED,
	PT_UNSIGN,
	PT_REAL,
	PT_TEXT,

	/*
	 * Actions
	 */
	PT_CAST_SR,             /* Cast signed to double             */
	PT_CAST_SU,             /* Cast signed to unsigned           */
	PT_CAST_UR,             /* Cast unsgined to double           */
	PT_CAST_US,             /* Cast unsigned to signed           */
	PT_CAST_RS,             /* Cast double to signed             */
	PT_CAST_RU,             /* Cast double to unsigned           */
	PT_EXPR_PAIR,           /* Place holder for expression pair  */

	PT_STRNICMP,
	PT_STRNCMP,
	PT_STRICMP,
	PT_STRCMP,
	PT_STRLEN,
	PT_FMOD,
	PT_FLOOR,
	PT_CEIL,
	PT_FABS,
	PT_LABS,
	PT_ABS,
	PT_ATOL,
	PT_ATOI,
	PT_ATOF,

	PT_MINUS_S,             /* Signed unary minus                   */
	PT_MINUS_R,             /* Real unary minus                     */
	PT_BINNOT,              /* Binary NOT - works only on signed    */
	PT_LOGNOT_S,            /* Logical NOT on signed                */
	PT_LOGNOT_U,            /* Logical NOT on unsgined              */
	PT_LOGNOT_R,            /* Logical NOT on double                */
	PT_LOGNOT_T,            /* Logical NOT on text                  */
	PT_ADD_S,               /* Add Signed                           */
	PT_ADD_U,               /* Add Unsigned                         */
	PT_ADD_R,               /* Add real                             */
	PT_SUB_S,               /* Subtract Signed                      */
	PT_SUB_U,               /* Subtract Unsigned                    */
	PT_SUB_R,               /* Subtract Real                        */
	PT_MUL_S,               /* Multiply Signed                      */
	PT_MUL_U,               /* Multiply Unsigned                    */
	PT_MUL_R,               /* Multiply Real                        */
	PT_DIV_S,               /* Divide Signed                        */
	PT_DIV_U,               /* Divide Unsigned                      */
	PT_DIV_R,               /* Divide Real                          */
	PT_MOD_S,               /* Modulus Signed                       */
	PT_MOD_U,               /* Modulus Unsigned                     */
	PT_MOD_R,               /* Modulus Real                         */
	PT_LT_S,                /* Relational Less Than on signed       */
	PT_LT_U,                /* Relational Less Than on unsigned     */
	PT_LT_R,                /* Relational Less Than on real         */
	PT_LE_S,                /* Relational Less or Equal on signed   */
	PT_LE_U,                /* Relational Less or Equal on unsigned */
	PT_LE_R,                /* Relational Less or Equal on real     */
	PT_EQ_S,                /* Relational Equal on signed           */
	PT_EQ_U,                /* Relational Equal on unsigned         */
	PT_EQ_R,                /* Relational Equal on real             */
	PT_NE_S,                /* Relational Not Equal on signed       */
	PT_NE_U,                /* Relational Not Equal on unsigned     */
	PT_NE_R,                /* Relational Not Equal on real         */
	PT_GT_S,                /* Relational Greater Than on signed    */
	PT_GT_U,                /* Relational Greater Than on unsigned  */
	PT_GT_R,                /* Relational Greater Than on real      */
	PT_GE_S,                /* Relational Greater or Equal on signed*/
	PT_GE_U,                /* Relational Greater or Equal on unsign*/
	PT_GE_R,                /* Relational Greater or Equal on real  */
	PT_BINAND_S,            /* Binary AND on signed                 */
	PT_BINAND_U,            /* Binary AND on unsined                */
	PT_BINOR_S,             /* Binary OR on signed                  */
	PT_BINOR_U,             /* Binary OR on unsigned                */
	PT_BINXOR_S,            /* Binary XOR on signed                 */
	PT_BINXOR_U,            /* Binary XOR on unsigned               */
	PT_LOGAND_S,            /* Logical AND on signed                */
	PT_LOGAND_U,            /* Logical AND on unsigned              */
	PT_LOGAND_R,            /* Logical AND on Real                  */
	PT_LOGAND_T,            /* Logical AND on Text                  */
	PT_LOGOR_S,             /* Logical OR on signed                 */
	PT_LOGOR_U,             /* Logical OR on unsigned               */
	PT_LOGOR_R,             /* Logical OR on real                   */
	PT_LOGOR_T,             /* Logical OR on Text                   */

	PT_TEST_S,
	PT_TEST_U,
	PT_TEST_R,
	PT_TEST_T,

	PT_LAST
};

typedef struct LEXINST_S*       LEXINST;
typedef struct PTREE_S*         PTREE;

typedef union PLEAF_U
{
	LONG    Signed;
	ULONG   Unsign;
	double  Real;
	pTEXT   Text;
	UINT    Offset;
} PLEAF;

typedef struct PNODE_S
{
	PTREE   Left;
	PTREE   Right;
} PNODE;

typedef union PBRANCH_U
{
	PLEAF   Leaf;
	PNODE   Node;
} PBRANCH;

struct PTREE_S
{
	COUNT   NodeType;
	COUNT   ExprType;
	PBRANCH Branch;
};

#ifdef ctCLIENT
#define ptAlloc(a)              mballc(1, (UINT)(a))
#define ptFree(a)               mbfree((pVOID)(a))
#else
#define ptAlloc(a)              ctgetmem((VRLEN)(a))
#define ptFree(a)               ctputmem((pTEXT)(a))
#endif

#ifdef PROTOTYPE

NINT	testparse(COUNT datno,pTEXT condexpr,PTREE *ptree);
pTEXT	ctgetnames(COUNT datno);
pTEXT   TextDup( pTEXT Text );
LONG    HexDigit( TEXT Char );
ULONG   atox( pTEXT Text );
LONG    Text2Escape( TEXT Char );
pTEXT   Quote2Text( pTEXT Text );

NINT ctcxparse( pConvMap Schema, pTEXT Names, pTEXT InputText, NINT InputTextSize, PTREE* pTree );

VOID ptFreeMem( pVOID Memory );
PTREE ptCreate( COUNT NodeType, COUNT ExprType );
VOID ptDestroy( PTREE Tree );
PTREE ptPutSigned( LONG Value );
PTREE ptPutUnsign( ULONG Value );
PTREE ptPutReal( double Value );
PTREE ptPutText( pTEXT Value );
PTREE ptPutField( COUNT NodeType, COUNT ExprType, UINT Value );
PTREE ptPutNode( COUNT NodeType, COUNT ExprType, PTREE Left, PTREE Right );
COUNT ptGetNodeType( PTREE Tree );
COUNT ptGetExprType( PTREE Tree );
LONG ptGetSigned( PTREE Tree );
ULONG ptGetUnsign( PTREE Tree );
double ptGetReal( PTREE Tree );
pTEXT ptGetText( PTREE Tree );
UINT ptGetField( PTREE Tree );
PTREE ptGetLeft( PTREE Tree );
PTREE ptGetRight( PTREE Tree );

PTREE TriplexCall( COUNT NodeType, COUNT ExprType, COUNT PairType, PTREE Arg1, PTREE Arg2, PTREE Arg3 );
PTREE DuplexCall( COUNT NodeType, COUNT ExprType, PTREE Arg1, PTREE Arg2 );
PTREE SimplexCall( COUNT NodeType, COUNT ExprType, PTREE Arg1 );
PTREE FieldName( pConvMap Schema, pTEXT Names, pTEXT Ident );

PTREE CastToSigned( PTREE Tree );
PTREE CastToUnsign( PTREE Tree );
PTREE CastToReal( PTREE Tree );

COUNT DuplexType( PTREE Left, PTREE Right );
PTREE DuplexMath( COUNT SignedOp, COUNT UnsignOp, COUNT RealOp, PTREE Left, PTREE Right );
PTREE DuplexRelational( COUNT SignedOp, COUNT UnsignOp, COUNT RealOp, PTREE Left, PTREE Right );
PTREE DuplexBinary( COUNT SignedOp, COUNT UnsignOp, PTREE Left, PTREE Right );
PTREE DuplexLogical( COUNT SignedOp, COUNT UnsignOp, COUNT RealOp, PTREE Left, PTREE Right );

PTREE cndxparse(  pConvMap Schema, pTEXT Names, pTEXT InputText,  NINT InputTextSize );
COUNT cndxeval( PTREE Tree, pVOID Recptr, pConvMap Schema );
VOID cndxfree( PTREE Tree );

#ifdef ctGVARH
COUNT cndxrun( PTREE Tree, pVOID Recptr, pConvMap Schema pinHan);
#endif

#else

NINT	testparse();
pTEXT	ctgetnames();
pTEXT   TextDup();
LONG    HexDigit();
ULONG   atox();
LONG    Text2Escape();
pTEXT   Quote2Text();

NINT ctcxparse();

VOID ptFreeMem();
PTREE ptCreate();
VOID ptDestroy();
PTREE ptPutSigned();
PTREE ptPutUnsign();
PTREE ptPutReal();
PTREE ptPutText();
PTREE ptPutField();
PTREE ptPutNode();
COUNT ptGetNodeType();
COUNT ptGetExprType();
LONG ptGetSigned();
ULONG ptGetUnsign();
double ptGetReal();
pTEXT ptGetText();
UINT ptGetField();
PTREE ptGetLeft();
PTREE ptGetRight();

PTREE TriplexCall();
PTREE DuplexCall();
PTREE SimplexCall();
PTREE FieldName();

PTREE CastToSigned();
PTREE CastToUnsign();
PTREE CastToReal();

COUNT DuplexType();
PTREE DuplexMath();
PTREE DuplexRelational();
PTREE DuplexBinary();
PTREE DuplexLogical();

COUNT cndxrun();
PTREE cndxparse();
COUNT cndxeval();
VOID cndxfree();

#endif  /* PROTOTYPE */

#endif  /* __CTCNDX_H */
