#include <trmm_rsl/rsl.h>

typedef struct {
  char *s;
  int len;
} Charlen;

typedef struct {
 int   country;        /* 000 */
 char  namestr[20];    /* Berrima */
 int   station_id_no;  /* 63 */
 float lat;            /* 12.457 */
 float lon;            /* 130.925 */
 float height;         /* 40 (meters) */
 int   datno;          /* 02798  (jjjyy) */
 float hhmm;           /* 05.00 */
 char  yyyymoddhhmmss[20]; /* 1998012705001 */
 float versionNumber;  /* 10.01 */
 int   freq;           /* 5645 */
 int   prf;            /* 740 */
 float pulselen;       /* 0.9 */
 int   range_resolution; /* 500 (meters) */
 float anglerate;        /* 15.0 */
 char  clearair[4];      /* "ON" or "OFF" */
 float angle_resolution; /* 1.0 (degrees) */
 int   video_resolution; /* 256 */
 int   start_range;      /* 1000 (meters) */
 int   end_range;      /* 256000 (meters) */
 char  product_type[10]; /* VOLUMETRIC */
 int   scannum; /* Scan n of m */
 int   ofscans; /* m */
 char  imgfmt[20]; /* PPI, RHI, etc. */
 float elev;       /* 0.5 (degrees) */
 char  video[20];  /* Vel, Wid, ... */
 float vellvl;     /* 9.8 */
 float nyquist;    /* 9.8 */
 float ratio1, ratio2; /* x:y, None */

 int nbins; /* Number of values in Rapic_range vector. */
} Rapic_sweep_header;

typedef struct {
  float x; /* Dummy. */
} Rapic_range;

typedef struct {
  Rapic_sweep_header h;
  Rapic_range *r;
} Rapic_sweep;

void binprint(char *s, int n);
void rapic_decode(unsigned char *inbuf, int inbytes, unsigned char *outbuf, int *outbytes,
				  float *azim, float *elev, int *delta_time);
void rapic_fix_time (Ray *ray);
void rapic_load_ray_header(Rapic_sweep_header rh, int iray, int isweep, float elev, float azim, Ray_header *h);
void rapic_load_ray_data(unsigned char *buf, int bufsize, int ifield, Ray *ray);
Radar *fill_header(Radar *radar);

/* I want to have the rapic prefix in the yacc parser.
 * This hack is required. 
 * I got this list from automake.html.
 */
/*
#define yymaxdepth rapicmaxdepth
#define yyparse rapicparse
#define yylex   rapiclex
#define yyerror rapicerror
#define yylval  rapiclval
#define yychar  rapicchar
#define yydebug rapicdebug
#define yypact  rapicpact  
#define yyr1    rapicr1                    
#define yyr2    rapicr2                    
#define yydef   rapicdef           
#define yychk   rapicchk           
#define yypgo   rapicpgo           
#define yyact   rapicact           
#define yyexca  rapicexca
#define yyerrflag rapicerrflag
#define yynerrs rapicnerrs
#define yyps    rapicps
#define yypv    rapicpv
#define yys     rapics
#define yy_yys  rapicyys
#define yystate rapicstate
#define yytmp   rapictmp
#define yyv     rapicv
#define yy_yyv  rapicyyv
#define yyval   rapicval
#define yylloc  rapiclloc
#define yyreds  rapicreds
#define yytoks  rapictoks
#define yylhs   rapiclhs
#define yylen   rapiclen
#define yydefred rapicdefred
#define yydgoto rapicdgoto
#define yysindex rapicsindex
#define yyrindex rapicrindex
#define yygindex rapicgindex
#define yytable  rapictable
#define yycheck  rapiccheck

#define yyin  rapicin
#define yyout rapicout
#define yy_create_buffer rapic_create_buffer
#define yy_load_buffer_state rapic_load_buffer_state
#define yyrestart rapicrestart
#define yy_init_buffer rapic_init_buffer
#define yy_switch_to_buffer rapic_switch_to_buffer
#define yy_delete_buffer rapic_delete_buffer
#define yy_flush_buffer rapic_flush_buffer
#define yy_scan_buffer rapic_scan_buffer
#define yy_scan_string rapic_scan_string
#define yy_scan_bytes rapic_scan_bytes
*/
