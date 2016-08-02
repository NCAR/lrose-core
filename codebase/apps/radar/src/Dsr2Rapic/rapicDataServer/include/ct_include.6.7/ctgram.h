typedef union {
	COUNT   yInt;
	LONG    ySigned;
	ULONG   yUnsign;
	pTEXT   yText;
	double  yReal;
	PTREE   yTree;
} YYSTYPE;
#define IDENTIFIER	257
#define TEXTCONST	258
#define SIGNEDCONST	259
#define UNSIGNCONST	260
#define REALCONST	261
#define L_AND	262
#define L_OR	263
#define R_LT	264
#define R_LE	265
#define R_EQ	266
#define R_NE	267
#define R_GT	268
#define R_GE	269
#define T_INT	270
#define T_UNSIGNED	271
#define T_LONG	272
#define T_DOUBLE	273
#define F_ATOI	274
#define F_ATOL	275
#define F_ABS	276
#define F_LABS	277
#define F_FABS	278
#define F_ATOF	279
#define F_CEIL	280
#define F_FLOOR	281
#define F_FMOD	282
#define F_STRLEN	283
#define F_STRCMP	284
#define F_STRICMP	285
#define F_STRNCMP	286
#define F_STRNICMP	287
extern YYSTYPE yylval;
