
/*  A Bison parser, made from rapic.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse rapicparse
#define yylex rapiclex
#define yyerror rapicerror
#define yylval rapiclval
#define yychar rapicchar
#define yydebug rapicdebug
#define yynerrs rapicnerrs
#define	IMAGE	257
#define	IMAGESCANS	258
#define	IMAGESIZE	259
#define	IMAGEEND	260
#define	SCAN	261
#define	IMAGEHEADEREND	262
#define	NUMBER	263
#define	ALPHA	264
#define	FLOATNUMBER	265
#define	BRACKETNUM	266
#define	COUNTRY	267
#define	NAME	268
#define	STNID	269
#define	LATITUDE	270
#define	LONGITUDE	271
#define	HEIGHT	272
#define	DATE	273
#define	TIME	274
#define	TIMESTAMP	275
#define	VERS	276
#define	FREQUENCY	277
#define	PRF	278
#define	PULSELENGTH	279
#define	RNGRES	280
#define	ANGRES	281
#define	ANGLERATE	282
#define	CLEARAIR	283
#define	ON	284
#define	OFF	285
#define	VIDRES	286
#define	STARTRNG	287
#define	ENDRNG	288
#define	PRODUCT	289
#define	PASS	290
#define	IMGFMT	291
#define	ELEV	292
#define	VIDEO	293
#define	VELLVL	294
#define	NYQUIST	295
#define	UNFOLDING	296
#define	AT	297
#define	VOLUMETRIC	298
#define	NORMAL	299
#define	OF	300
#define	REFL	301
#define	VEL	302
#define	UNCORREFL	303
#define	ZDR	304
#define	WID	305
#define	NONE	306
#define	RAYDATA	307
#define	ENDRADARIMAGE	308

#line 23 "rapic.y"

#define USE_RSL_VARS
#include "rapic_routines.h"
#include <trmm_rsl/rsl.h> 
#include <stdlib.h>
#include <math.h>
#include <string.h>

int rapicerror(char *s);
int rapicwrap(char *s);
int rapicwrap(char *s);
int yywrap(char *s);
int rapiclex(void);

int nsweep = 0;
float angres;
Radar *radar, *rapic_radar = NULL;
Volume *volume;
Sweep *sweep;
Ray *ray;

/* Rapic format declarations. */
Rapic_sweep_header rh;
Rapic_sweep *rs;

unsigned char outbuf[2000000];
int outbytes;
float azim, elev;
float save_elev;
int delta_time;
int nray = 0;
int ifield;
int ivolume, isweep, iray;
int station_id;

int sweepcount[5];

extern int radar_verbose_flag;
float rapic_nyquist;
  
#line 326 "rapic.y"
typedef union {
  Charlen token;
} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		152
#define	YYFLAG		-32768
#define	YYNTBASE	56

#define YYTRANSLATE(x) ((unsigned)(x) <= 308 ? yytranslate[x] : 108)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    55,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,    54
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     4,     6,     9,    13,    15,    19,    22,    24,    27,
    28,    32,    35,    38,    40,    52,    54,    57,    58,    60,
    62,    65,    66,    69,    72,    75,    78,    81,    84,    87,
    90,    93,    96,    99,   102,   105,   108,   111,   114,   117,
   120,   123,   126,   130,   132,   135,   138,   141,   144,   147,
   150,   153,   155,   157,   159,   161,   163,   165,   167,   169,
   171,   173,   175,   177,   179,   181,   183,   185,   187,   189,
   191,   193,   195,   197,   199,   201,   203,   205,   207,   209,
   211,   213,   215,   217,   219,   221,   223,   227,   229,   231,
   233,   235,   237,   239,   241,   243,   245,   247,   249
};

static const short yyrhs[] = {    61,
    57,    60,     0,    58,     0,    57,    58,     0,    59,    65,
    54,     0,    67,     0,     6,    71,    73,     0,    62,     8,
     0,    63,     0,    62,    63,     0,     0,     3,    71,    73,
     0,     4,    70,     0,     5,    70,     0,    64,     0,     7,
    72,    55,    71,    74,    75,    76,    77,    75,    78,    79,
     0,    66,     0,    65,    66,     0,     0,    53,     0,    68,
     0,    67,    68,     0,     0,    14,    82,     0,    13,    81,
     0,    15,    83,     0,    16,    84,     0,    17,    85,     0,
    18,    86,     0,    19,    80,     0,    20,    87,     0,    21,
    88,     0,    22,    89,     0,    23,    90,     0,    24,    91,
     0,    25,    92,     0,    26,    93,     0,    28,    95,     0,
    29,    96,     0,    27,    94,     0,    32,    97,     0,    33,
    98,     0,    34,    98,     0,    35,    99,    12,     0,    35,
     0,    36,   100,     0,    38,    76,     0,    40,   105,     0,
    41,   106,     0,    39,   104,     0,    37,   103,     0,    42,
   107,     0,    70,     0,    11,     0,     9,     0,    70,     0,
    70,     0,    70,     0,    70,     0,    69,     0,    10,     0,
    69,     0,    70,     0,    70,     0,    70,     0,    70,     0,
    70,     0,    10,     0,    70,     0,    69,     0,    69,     0,
    69,     0,    69,     0,    70,     0,    69,     0,    70,     0,
    70,     0,    69,     0,    70,     0,    69,     0,    69,     0,
    30,     0,    31,     0,    70,     0,    70,     0,    44,     0,
    45,     0,   101,    46,   102,     0,    70,     0,    70,     0,
    10,     0,    47,     0,    48,     0,    49,     0,    50,     0,
    51,     0,    69,     0,    69,     0,    52,     0,    70,    55,
    70,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   334,   346,   347,   349,   359,   372,   374,   413,   414,   415,
   418,   429,   430,   437,   440,   450,   451,   452,   455,   507,
   508,   509,   515,   516,   517,   518,   519,   520,   521,   522,
   523,   524,   525,   526,   527,   528,   529,   530,   531,   532,
   533,   534,   535,   536,   537,   538,   539,   540,   545,   546,
   547,   550,   551,   553,   555,   556,   557,   558,   559,   560,
   561,   562,   563,   564,   565,   567,   568,   569,   570,   571,
   572,   573,   575,   576,   578,   579,   580,   581,   582,   583,
   584,   585,   586,   587,   588,   589,   591,   592,   593,   594,
   596,   597,   598,   599,   600,   602,   603,   605,   606
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","IMAGE",
"IMAGESCANS","IMAGESIZE","IMAGEEND","SCAN","IMAGEHEADEREND","NUMBER","ALPHA",
"FLOATNUMBER","BRACKETNUM","COUNTRY","NAME","STNID","LATITUDE","LONGITUDE","HEIGHT",
"DATE","TIME","TIMESTAMP","VERS","FREQUENCY","PRF","PULSELENGTH","RNGRES","ANGRES",
"ANGLERATE","CLEARAIR","ON","OFF","VIDRES","STARTRNG","ENDRNG","PRODUCT","PASS",
"IMGFMT","ELEV","VIDEO","VELLVL","NYQUIST","UNFOLDING","AT","VOLUMETRIC","NORMAL",
"OF","REFL","VEL","UNCORREFL","ZDR","WID","NONE","RAYDATA","ENDRADARIMAGE","':'",
"rapic_recognized","sweeps","sweep","sweepheader","imageend","complete_header",
"imageheader","imageheader_item","scanlist","rays","ray","scanheader","scanheaditem",
"real","number","seqno","scanno","imgno","datetime","dc","elev","fieldno","offset",
"size","datno","code","namestr","idno","lat","lon","alt","hhmm","yyyymoddhhmmss",
"versionNumber","freq","prf","len","gatewidth","angle","anglerate","clearair",
"res","rng","typeid","noofnscans","no","nscans","type","field","level","nyq",
"ratio", NULL
};
#endif

static const short yyr1[] = {     0,
    56,    57,    57,    58,    59,    60,    61,    62,    62,    62,
    63,    63,    63,    63,    64,    65,    65,    65,    66,    67,
    67,    67,    68,    68,    68,    68,    68,    68,    68,    68,
    68,    68,    68,    68,    68,    68,    68,    68,    68,    68,
    68,    68,    68,    68,    68,    68,    68,    68,    68,    68,
    68,    69,    69,    70,    71,    72,    73,    74,    75,    75,
    76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
    86,    87,    88,    89,    90,    91,    92,    93,    94,    95,
    96,    96,    97,    98,    99,    99,   100,   101,   102,   103,
   104,   104,   104,   104,   104,   105,   106,   107,   107
};

static const short yyr2[] = {     0,
     3,     1,     2,     3,     1,     3,     2,     1,     2,     0,
     3,     2,     2,     1,    11,     1,     2,     0,     1,     1,
     2,     0,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     3,     1,     2,     2,     2,     2,     2,     2,
     2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     3,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     3
};

static const short yydefact[] = {    10,
     0,     0,     0,     0,    22,     0,     8,    14,    54,    55,
     0,    12,    13,    56,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    44,     0,     0,     0,     0,
     0,     0,     0,    22,     2,    18,     5,    20,     7,     9,
    57,    11,     0,    66,    24,    67,    23,    68,    25,    53,
    69,    52,    26,    70,    27,    71,    28,    65,    29,    72,
    30,    73,    31,    74,    32,    75,    33,    76,    34,    77,
    35,    78,    36,    79,    39,    80,    37,    81,    82,    38,
    83,    40,    84,    41,    42,    85,    86,     0,    88,    45,
     0,    90,    50,    61,    46,    91,    92,    93,    94,    95,
    49,    96,    47,    97,    48,    98,     0,    51,     0,     3,
     1,    19,     0,    16,    21,     0,    43,     0,     0,     0,
     4,    17,    58,     0,    89,    87,    99,     6,    60,    59,
     0,     0,    62,     0,     0,    63,     0,    64,    15,     0,
     0,     0
};

static const short yydefgoto[] = {   150,
    44,    45,    46,   121,     5,     6,     7,     8,   123,   124,
    47,    48,   104,    62,    11,    15,    52,   134,   141,   105,
   144,   147,   149,    69,    55,    57,    59,    63,    65,    67,
    71,    73,    75,    77,    79,    81,    83,    85,    87,    90,
    92,    94,    98,   100,   101,   136,   103,   111,   113,   115,
   118
};

static const short yypact[] = {   116,
    -3,    -3,    -3,    -3,    70,    36,-32768,-32768,-32768,-32768,
    -3,-32768,-32768,-32768,   -35,    -3,    12,    -3,     3,     3,
     3,    -3,     3,    -3,     3,    -3,    -3,     3,    -3,     3,
     3,     7,    -3,    -3,    -3,     5,    -3,    14,     3,    66,
     3,     3,    -4,    40,-32768,   -26,    70,-32768,-32768,-32768,
-32768,-32768,    -3,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,    19,-32768,-32768,
   -11,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   -10,-32768,    -3,-32768,
-32768,-32768,    47,-32768,-32768,    -3,-32768,    -3,    -3,    -3,
-32768,-32768,-32768,   124,-32768,-32768,-32768,-32768,-32768,-32768,
     3,    -3,-32768,   124,    -3,-32768,    -3,-32768,-32768,    51,
    71,-32768
};

static const short yypgoto[] = {-32768,
-32768,    80,-32768,-32768,-32768,-32768,    41,-32768,-32768,     8,
-32768,    79,   -12,    -1,   -49,-32768,     0,-32768,    -8,    -2,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   102,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768
};


#define	YYLAST		146


static const short yytable[] = {    10,
    12,    13,    14,   126,     9,     9,    61,    64,    66,    51,
    70,     9,    74,    60,    54,    80,    58,    84,    86,    53,
    68,    56,    72,   102,    76,    78,   122,    82,   112,   114,
   127,    91,    93,    93,   128,    99,    88,    89,     1,     2,
     3,   117,     4,    49,   129,   119,    50,   116,    96,    97,
   151,    10,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,   130,
   152,    33,    34,    35,    36,    37,    38,    39,    40,    41,
    42,    43,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,   122,
   131,    33,    34,    35,    36,    37,    38,    39,    40,    41,
    42,    43,   106,   107,   108,   109,   110,    10,     1,     2,
     3,   140,     4,   120,   133,   125,   135,   137,    51,   138,
   132,   140,     9,   139,    60,   145,    95,     0,   142,     0,
   143,     0,     0,   146,     0,   148
};

static const short yycheck[] = {     1,
     2,     3,     4,    53,     9,     9,    19,    20,    21,    11,
    23,     9,    25,    11,    16,    28,    18,    30,    31,    55,
    22,    10,    24,    10,    26,    27,    53,    29,    41,    42,
    12,    33,    34,    35,    46,    37,    30,    31,     3,     4,
     5,    43,     7,     8,    55,     6,     6,    52,    44,    45,
     0,    53,    13,    14,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,   119,
     0,    32,    33,    34,    35,    36,    37,    38,    39,    40,
    41,    42,    13,    14,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    28,    29,    53,
    54,    32,    33,    34,    35,    36,    37,    38,    39,    40,
    41,    42,    47,    48,    49,    50,    51,   119,     3,     4,
     5,   134,     7,    44,   126,    47,   128,   129,   130,   130,
   123,   144,     9,    10,    11,   144,    35,    -1,   141,    -1,
   142,    -1,    -1,   145,    -1,   147
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  char *f = from;
  char *t = to;
  int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  char *t = to;
  char *f = from;
  int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  int yystate;
  int yyn;
  short *yyssp;
  YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 335 "rapic.y"
{
  if (radar_verbose_flag) fprintf(stderr, "SUCCESSFUL parse\n");
  sprintf(radar->h.name, "%s", rh.namestr);
  sprintf(radar->h.radar_name, "%s", rh.namestr);

  radar = fill_header(radar);
  radar = RSL_prune_radar(radar);
  rapic_radar = radar;
  YYACCEPT;
;
    break;}
case 4:
#line 350 "rapic.y"
{
  /* Attach the sweep to the volume. */
  if (radar_verbose_flag) fprintf(stderr, "Attach the sweep %d to the volume %d.\n",
		  isweep, ivolume);
  radar->v[ivolume]->sweep[isweep] = sweep;
  radar->v[ivolume]->h.f    = sweep->h.f;
  radar->v[ivolume]->h.invf = sweep->h.invf;
;
    break;}
case 5:
#line 360 "rapic.y"
{
  /*  float c =  RSL_SPEED_OF_LIGHT; */
  if (rh.angle_resolution != 0) 
	sweep = RSL_new_sweep((int)(360.0/rh.angle_resolution+0.5));
  if (fabs(rh.elev - save_elev) > .5) { /* New sweep elevation. */
	isweep++;
	save_elev = rh.elev;
  }
  nray = 0;
  /* rapic_nyquist = c*((float)rh.prf/10.)/(4.*(float)rh.freq*100000.0); */
;
    break;}
case 7:
#line 375 "rapic.y"
{
  if (radar_verbose_flag) fprintf(stderr, "sweepcount[0] = %d\n", sweepcount[0]);
  if (sweepcount[0] > 0) {
	radar->v[DZ_INDEX] = RSL_new_volume(sweepcount[0]);
	radar->v[DZ_INDEX]->h.type_str = strdup("Reflectivity");
  }
  if (radar_verbose_flag) fprintf(stderr, "sweepcount[1] = %d\n", sweepcount[1]);
  if (sweepcount[1] > 0) {
	volume = radar->v[VR_INDEX] = RSL_new_volume(sweepcount[1]);
	volume->h.type_str = strdup("Velocity");
	volume->h.calibr_const = 0.0;
  }
  if (radar_verbose_flag) fprintf(stderr, "sweepcount[2] = %d\n", sweepcount[2]);
  if (sweepcount[2] > 0) {
	radar->v[SW_INDEX] = RSL_new_volume(sweepcount[2]);
	volume->h.type_str = strdup("Spectral Width");
	volume->h.calibr_const = 0.0;
  }
  if (radar_verbose_flag) fprintf(stderr, "sweepcount[3] = %d\n", sweepcount[3]);
  if (sweepcount[3] > 0) {
	radar->v[ZD_INDEX] = RSL_new_volume(sweepcount[3]);
	volume->h.type_str = strdup("Reflectivity Depolarization Ratio");
	volume->h.calibr_const = 0.0;
  }
  if (radar_verbose_flag) fprintf(stderr, "sweepcount[4] = %d\n", sweepcount[4]);
  if (sweepcount[4] > 0) {
	radar->v[ZT_INDEX] = RSL_new_volume(sweepcount[4]);
	volume->h.type_str = strdup("Total Reflectivity");
	volume->h.calibr_const = 0.0;
  }
  isweep = -1; /* It keeps track of the sweep number across all field
                * types; volumes.  It is immediately bumped to 0 when
                * the sweepheader is parsed.
				*/
  save_elev = 99999;
;
    break;}
case 11:
#line 419 "rapic.y"
{
  radar = RSL_new_radar(MAX_RADAR_VOLUMES);
  sweepcount[0] = 0;
  sweepcount[1] = 0;
  sweepcount[2] = 0;
  sweepcount[3] = 0;
  sweepcount[4] = 0;
  radar->h.number = atoi(yyvsp[-1].token.s);
;
    break;}
case 13:
#line 431 "rapic.y"
{
  if (atoi(yyvsp[0].token.s) <= 0) {
	fprintf(stderr, "RAPIC: /IMAGESIZE == %d.  RAPIC ingest returning NULL.\n", atoi(yyvsp[0].token.s));
	YYERROR;
  }
;
    break;}
case 15:
#line 441 "rapic.y"
{
  ifield = atoi(yyvsp[-3].token.s);
  sweepcount[ifield]++;
;
    break;}
case 19:
#line 456 "rapic.y"
{

   /*   fprintf(stderr, "YACC len=%d text=<", yylval.token.len); */
   /*   binprint(yylval.token.s, yylval.token.len); */
   /*   fprintf(stderr, ">\n"); */

   /* Quiet the compilier, because I only use the rsl_f_list and rsl_invf_list. */
   RSL_ftype[0] = RSL_ftype[0];

   /* Use yylval.token.s and yylval.token.len */
   memset(outbuf, 0, sizeof(outbuf));
   rapic_decode((unsigned char *)yylval.token.s, yylval.token.len, outbuf, &outbytes,
				&azim, &elev, &delta_time);
   /*   fprintf(stderr, "RAYDATA: ray %d, ivol %d, isweep %d, azim %f, elev %f, dtime %d, size=%d\n", nray, ivolume, isweep, azim, elev, delta_time, outbytes); */

   ray = RSL_new_ray(outbytes);
   rapic_load_ray_header(rh, nray, isweep, elev, azim, &ray->h); /* Mostly from the scanheader (rh). */
   ray->h.azimuth = azim;
   /*    if (39<azim && azim <40) { */
   ray->h.elev = elev;
   ray->h.sec += delta_time;
   ray->h.f    = RSL_f_list[ivolume]; /* Data conversion function. f(x). */
   ray->h.invf = RSL_invf_list[ivolume]; /* invf(x). */

   rapic_fix_time(ray);
   rapic_load_ray_data(outbuf, outbytes, ivolume, ray);
#define DODO
#undef DODO
#ifdef DODO
   if (ray->h.ray_num == 0 && ivolume == 1 && isweep == 0)
	 { int i;
   fprintf(stderr, "RAYDATA: ray %d, ivol %d, isweep %d, azim %f, elev %f, dtime %d, size=%d\n", nray, ivolume, isweep, azim, elev, delta_time, outbytes);
	 for (i=0; i<ray->h.nbins; i++) {
	   fprintf(stderr,"YACCray->range[%d] = %d  %f\n", i, (int)ray->range[i],
			   ray->h.f(ray->range[i]));
	 }
	 }
#endif
   /* Attach the ray to the sweep. */
   sweep->ray[nray]      = ray;
   sweep->h.beam_width   = ray->h.beam_width;
   sweep->h.vert_half_bw = sweep->h.beam_width / 2.0;
   sweep->h.horz_half_bw = sweep->h.beam_width / 2.0;
   sweep->h.sweep_num    = isweep;
   sweep->h.elev         = ray->h.elev;
   sweep->h.f            = ray->h.f;
   sweep->h.invf         = ray->h.invf;
   nray++;
   /*   } */
;
    break;}
case 23:
#line 515 "rapic.y"
{ memmove(rh.namestr,yyvsp[0].token.s,yyvsp[0].token.len); ;
    break;}
case 24:
#line 516 "rapic.y"
{ rh.country       = atoi(yyvsp[0].token.s); ;
    break;}
case 25:
#line 517 "rapic.y"
{ rh.station_id_no = atoi(yyvsp[0].token.s); ;
    break;}
case 26:
#line 518 "rapic.y"
{ rh.lat           = atof(yyvsp[0].token.s); ;
    break;}
case 27:
#line 519 "rapic.y"
{ rh.lon           = atof(yyvsp[0].token.s); ;
    break;}
case 28:
#line 520 "rapic.y"
{ rh.height        = atof(yyvsp[0].token.s); ;
    break;}
case 29:
#line 521 "rapic.y"
{ rh.datno         = atoi(yyvsp[0].token.s); ;
    break;}
case 30:
#line 522 "rapic.y"
{ rh.hhmm          = atof(yyvsp[0].token.s); ;
    break;}
case 31:
#line 523 "rapic.y"
{ memmove(rh.yyyymoddhhmmss,yyvsp[0].token.s,yyvsp[0].token.len); ;
    break;}
case 32:
#line 524 "rapic.y"
{ rh.versionNumber    = atof(yyvsp[0].token.s); ;
    break;}
case 33:
#line 525 "rapic.y"
{ rh.freq             = atoi(yyvsp[0].token.s); ;
    break;}
case 34:
#line 526 "rapic.y"
{ rh.prf              = atoi(yyvsp[0].token.s); ;
    break;}
case 35:
#line 527 "rapic.y"
{ rh.pulselen         = atof(yyvsp[0].token.s); ;
    break;}
case 36:
#line 528 "rapic.y"
{ rh.range_resolution = atoi(yyvsp[0].token.s); ;
    break;}
case 37:
#line 529 "rapic.y"
{ rh.anglerate        = atof(yyvsp[0].token.s); ;
    break;}
case 38:
#line 530 "rapic.y"
{ memmove(rh.clearair,yyvsp[0].token.s,yyvsp[0].token.len);;
    break;}
case 39:
#line 531 "rapic.y"
{ rh.angle_resolution = atof(yyvsp[0].token.s); ;
    break;}
case 40:
#line 532 "rapic.y"
{ rh.video_resolution = atoi(yyvsp[0].token.s); ;
    break;}
case 41:
#line 533 "rapic.y"
{ rh.start_range      = atoi(yyvsp[0].token.s); ;
    break;}
case 42:
#line 534 "rapic.y"
{ rh.end_range        = atoi(yyvsp[0].token.s); ;
    break;}
case 43:
#line 535 "rapic.y"
{ memmove(rh.product_type,yyvsp[-1].token.s,yyvsp[-1].token.len); ;
    break;}
case 46:
#line 538 "rapic.y"
{ rh.elev    = atof(yyvsp[0].token.s); ;
    break;}
case 47:
#line 539 "rapic.y"
{ rh.vellvl  = atof(yyvsp[0].token.s); ;
    break;}
case 48:
#line 541 "rapic.y"
{
  rh.nyquist = atof(yyvsp[0].token.s);
  rapic_nyquist = rh.nyquist;
;
    break;}
case 49:
#line 545 "rapic.y"
{ memmove(rh.video,yyvsp[0].token.s,yyvsp[0].token.len); ;
    break;}
case 50:
#line 546 "rapic.y"
{ memmove(rh.imgfmt,yyvsp[0].token.s,yyvsp[0].token.len); ;
    break;}
case 88:
#line 592 "rapic.y"
{rh.scannum = atoi(yyvsp[0].token.s);;
    break;}
case 89:
#line 593 "rapic.y"
{rh.ofscans = atoi(yyvsp[0].token.s);;
    break;}
case 91:
#line 596 "rapic.y"
{ivolume = DZ_INDEX; volume = radar->v[ivolume];;
    break;}
case 92:
#line 597 "rapic.y"
{ivolume = VR_INDEX; volume = radar->v[ivolume];;
    break;}
case 93:
#line 598 "rapic.y"
{ivolume = ZT_INDEX; volume = radar->v[ivolume];;
    break;}
case 94:
#line 599 "rapic.y"
{ivolume = ZD_INDEX; volume = radar->v[ivolume];;
    break;}
case 95:
#line 600 "rapic.y"
{ivolume = SW_INDEX; volume = radar->v[ivolume];;
    break;}
case 98:
#line 605 "rapic.y"
{rh.ratio1 = 0; rh.ratio2 = 0;;
    break;}
case 99:
#line 606 "rapic.y"
{rh.ratio1 = atoi(yyvsp[-2].token.s); rh.ratio2 = atoi(yyvsp[0].token.s);;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 610 "rapic.y"


int rapicerror(char *s)
{
  fprintf(stderr, "RAPIC ERROR: <%s> on token <", s);
  binprint(yylval.token.s, yylval.token.len);
  fprintf(stderr, ">\n");
  return 1;
}

int rapicwrap(char *s)
{
  yywrap(s);
  return 1;
}
