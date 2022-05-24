#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <zlib.h>

typedef unsigned char  uchar;
typedef unsigned short ushort;


typedef struct
        {
        
        char signature[8]; /* string "ROS2_V" (ROS2 Volume)            */
        time_t date;       /* unix timestamp                           */
        float  rad_lat;    /* radar latitude                           */
        float  rad_lon;    /* radar latitude                           */
        float  rad_alt;    /* radar latitude                           */

        char   name[64];   /* name of scan (if available)              */
        char   type[8];    /* type of scan (CW=0, CCW=1, EL=2, AZ=3)   */
        float  PRF;        /* pulse repetition frequency               */
        int    l_pulse;    /* lenght of pulse [nanosec]                */
	float  l_bin;      /* lenght of range bin [m]                  */

        float  nyquist_v;  /* nyquist (unabiguous) velocity [m/s]      */
	float  freq ;      /* transmitted frequency [megahertz]        */

	char   Z;          /* reflectivity                             */
	char   Z_pos;      /* position of Z field in the radar beam    */
	char   D;          /* differential reflectivity                */
	char   D_pos;      /* position of D field in the radar beam    */
	char   P;          /* differential phase shift                 */
	char   P_pos;      /* position of P field in the radar beam    */
	char   R;          /* rho                                      */
	char   R_pos;      /* position of R field in the radar beam    */
        char   L;	   /* linear depolarization ratio              */
	char   L_pos;      /* position of L field in the radar beam    */
        char   V;          /* doppler velocity                         */
	char   V_pos;      /* position of V field in the radar beam    */
        char   S;	   /* spread of doppler velocity               */
	char   S_pos;      /* position of S field in the radar beam    */
        char   IQ;	   /* IQ data                                  */
	char   IQ_pos;     /* position of IQ field in the radar beam   */
        char   uZ;	   /* uncorrected reflectivity                 */
	char   uZ_pos;     /* position of uZ field in the radar beam   */
        char   uV;	   /* uncorrected doppler velocity             */
	char   uV_pos;     /* position of uV field in the radar beam   */
        char   uS;	   /* uncorrected spread of doppler velocity   */
	char   uS_pos;     /* position of uS field in the radar beam   */
        char   PART;	   /* particle                                 */
	char   PART_pos;   /* position of PART field in the radar beam */
        char   RR;	   /* rain (polarimetric relationship)         */
	char   RR_pos;     /* position of RR field in the radar beam   */

        float  tz_height;  /* thermal zero height (NaN=not available)  */

        /* added at 25Feb2011 */
        char   K;	   /* Kdp, specific differential phase         */
	char   K_pos;      /* position of Kdp field in the radar beam  */
	
	char   spare[510]; /* for future implementations               */                
        } VOLUME_HEADER;

typedef struct
        {
	short int sweep;    /* sweep_id (0 - n). Used as EOV flag if 
	                       set to -1                              */
        double    time;     /* UNIX timestamp                         */
        float     el;
	float     az;
	short int n_bins;   /* number of range bins                   */
        short int n_pulses; /* number of integrated pulses            */
        short int n_values; /* number of following float values       */

        char compression;   /* 0=nessuna, 1=Zlib                      */
        int  beam_lenght;   /* lenght of following data section.      */
        char data_type;     /* 1=uchar, 2=float, 3=ushort, 4=half     */
        } BEAM_HEADER;


int Uncompress(uchar* in,int n_in,uchar* out,int n_out)
    {
    /* prova a decomprimere (zlib) il contenuto di dell'area di memoria puntata da in, di dimensione n_in.
       Il risultato viene scritto nell'area di memoria puntata da *out, di dimensione *n_out assegnata. 
       Se la decompressione ha successo la funzione restituisce il valore 1, altrimenti l'ingresso viene
       semplicemente copiato sull'uscita e viene resttituito il valore 0. */

    z_stream z;

    z.next_in =Z_NULL;
    z.avail_in=Z_NULL;
    z.zalloc  =Z_NULL;
    z.zfree   =Z_NULL;
    z.opaque  =Z_NULL;
    inflateInit(&z);

    z.next_in   = in;
    z.avail_in  = n_in;
    z.next_out  = out;
    z.avail_out = n_out;

    if (inflate(&z,Z_FINISH)==Z_STREAM_END)
       {
       inflateEnd(&z);
       return(1);
       }
    else
       {
       inflateEnd(&z);
       return(0);
       }
    }

void PrintValues(int type,int position,int n_bins,char* beam,FILE* out)
     {
     int i, offset=position*n_bins;
     
     switch (type)
            {
	    case 1: /* uchar */   for (i=0;i<n_bins;i++) fprintf(out," %03d" ,*((uchar*) beam+offset+i)); break;
	    case 2: /* float */   for (i=0;i<n_bins;i++) fprintf(out," %6.2f",*((float*) beam+offset+i)); break;
	    case 3: /* ushort */  for (i=0;i<n_bins;i++) fprintf(out," %05d" ,*((ushort*)beam+offset+i)); break;
	    case 4: /* half */      	                                                                  break;
	    }           
     fprintf(out,"\n");
     return;
     }

int main (int argc,char* argv[])
         {
	 char *buf=NULL,*beam=NULL,date[32];
	 int n=0,sweep=-1,data_type=0;
	 FILE *in=NULL,*out=NULL;
         VOLUME_HEADER	vh; 
         BEAM_HEADER	bh; 
	 
	 int  Uncompress (uchar*,int,uchar*,int);
	 void PrintValues(int,int,int,char*,FILE*);

	 if      (argc< 2) {printf("USO: %s sorgente [destinazione]\n",argv[0]); return(0);     }
	 else if (argc==2) {n=strlen(argv[1])+7; buf=malloc(n); sprintf(buf,"%s.ASCII",argv[1]);}
	 else              {n=strlen(argv[2])+1; buf=malloc(n); strcpy(buf,argv[2]);            }
	 
	 if ((in=fopen(argv[1],"r")))
	    {
	    if ((out=fopen(buf,"w")))
	       {
	       if (fread(&vh,sizeof(VOLUME_HEADER),1,in) && !strcmp(vh.signature,"ROS2_V"))
 	          {
                  if (vh.Z) fprintf(out,"Z: REFLECTIVITY\n");			   
                  if (vh.D) fprintf(out,"D: DIFFERENTIAL REFLECTIVITY\n");			   
                  if (vh.P) fprintf(out,"P: DIFFERENTIAL PHASE SHIFT\n");				   
                  if (vh.R) fprintf(out,"R: COEFFICIENT OF CORRELATION\n");				   
                  if (vh.L) fprintf(out,"L: LINEAR DEPOLARIZATION RATIO\n");				   
                  if (vh.V) fprintf(out,"V: DOPPLER VELOCITY\n");				   
                  if (vh.S) fprintf(out,"S: SPREAD OF DOPPLER VELOCITY\n");				   

                  strcpy(date,ctime(&(vh.date))); date[strlen(date)-1]=0;
		  fprintf(out,"\nVOLUME: time=%.ld (%s)   rad_lat=%.4f deg   rad_lon=%.4f deg   rad_alt=%.0f m   range_bin=%.1f m   nyquist_velocity=%.2f m/s",vh.date,date,vh.rad_lat,vh.rad_lon,vh.rad_alt,vh.l_bin,vh.nyquist_v); 


                  while (1)
		        {
			if      (!fread(&bh,sizeof(BEAM_HEADER),1,in)) {printf("ERRORE: impossibile leggere l'header di un beam ROS2\n"); break;}
                        else if (bh.sweep<0)                                                                                              break;

                        if (!data_type) {data_type=bh.data_type; fprintf(out,"   data_type=%hd\n",data_type);}


		        fprintf(out,"\nBEAM: t=%.2lf   el=%.1f   az=%.1f  n_bins=%hd\n",bh.time,bh.el,bh.az,bh.n_bins); 

			if (sweep!=bh.sweep) {sweep=bh.sweep; beam=realloc(beam,bh.n_values*sizeof(float)); /* ad inizio sweep il numero di range bin potrebbe variare */ }
			
			if (n<bh.beam_lenght) {n=bh.beam_lenght; buf=realloc(buf,n);}
			if (!fread(buf,bh.beam_lenght,1,in)) {printf("ERRORE: impossibile leggere la sezione dati di un beam ROS2\n"); break;}
			
                        if (bh.compression)
			   {
			   if (!Uncompress((uchar*)buf,bh.beam_lenght,(uchar*)beam,bh.n_values*sizeof(float))) {printf("ERRORE: impossibile decomprimere la sezione dati di un beam ROS2\n"); break;}
			   }
                        else
                           memcpy(beam,buf,bh.beam_lenght);
			   
			/* ora beam contiene i dati radar decompressi, che posso stampare  */   
	  
	                if (vh.Z) {fprintf(out,"Z:"); PrintValues(bh.data_type,vh.Z_pos,bh.n_bins,beam,out);}
                        if (vh.D) {fprintf(out,"D:"); PrintValues(bh.data_type,vh.D_pos,bh.n_bins,beam,out);}			   
                        if (vh.P) {fprintf(out,"P:"); PrintValues(bh.data_type,vh.P_pos,bh.n_bins,beam,out);}				   
                        if (vh.R) {fprintf(out,"R:"); PrintValues(bh.data_type,vh.R_pos,bh.n_bins,beam,out);}				   
                        if (vh.L) {fprintf(out,"L:"); PrintValues(bh.data_type,vh.L_pos,bh.n_bins,beam,out);}				   
                        if (vh.V) {fprintf(out,"V:"); PrintValues(bh.data_type,vh.V_pos,bh.n_bins,beam,out);}				   
                        if (vh.S) {fprintf(out,"S:"); PrintValues(bh.data_type,vh.S_pos,bh.n_bins,beam,out);}				   

			}
		  }
	        else  printf("ERRORE: impossibile leggere un header di volume ROS2\n");
	       }
	    else printf("ERRORE: impossibile aprire il file destinazione \"%s\" in scrittura\n",buf);  
	    }
	 else printf("ERRORE: impossibile aprire il file sorgente \"%s\" in lettura\n",argv[0]); 
	 
	 if (beam) free(beam);
	 if (out)  fclose(out);
	 if (in)   fclose(in);
	 if (buf)  free(buf);
	 
	 return(1);
	 }


