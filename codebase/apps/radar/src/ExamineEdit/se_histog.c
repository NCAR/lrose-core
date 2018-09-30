/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define NEW_ALLOC_SCHEME

/*
 * 
 * se_histo_fin_irreg_areas
 * se_histo_fin_irreg_counts
 * se_histo_fin_areas
 * se_histo_fin_counts
 * 
 * se_histo_init
 * se_histo_output
 * se_histo_ray
 * se_histog_setup
 */

# include <dorade_headers.h>
# include "solo_editor_structs.h"
# include <ui.h>
# include <ui_error.h>
# include <dd_math.h>
# include <function_decl.h>
# include <dgi_func_decl.h>
# include <glib.h>

extern GString *gs_complaints;

# define SQM2KM 1.e-6
# define   SE_HST_NOCOPY 0
# define     SE_HST_COPY 1

static struct solo_edit_stuff *seds=NULL;
static struct solo_str_mgmt *se_spair_strings=NULL;
static FILE *histo_stream;
static FILE *x_y_stream;
static double areaXval;


/* external routines */
void se_print_ssm_strings();	/* se_utils.c */
void se_fix_comment();		/* se_utils.c */
void se_append_string();	/* se_utils.c */
void se_push_all_ssms();	/* se_utils.c */

/* internal routines */
int se_histo_output();
void se_histo_fin_irreg_areas();
void se_histo_fin_irreg_counts();
void se_histo_fin_areas();
void se_histo_fin_counts();
void se_histo_init();
int  se_histog_setup();


/* c------------------------------------------------------------------------ */

int se_histo_output()
{
   struct solo_str_mgmt *ssm;
   struct solo_edit_stuff *seds, *return_sed_stuff();
   char str[256], mess[256];

   seds = return_sed_stuff();	/* solo editing struct */
   se_print_ssm_strings(seds->h_output);

   if(seds->histo_output_key == SE_HST_COPY) {
      if(!histo_stream) {
	 slash_path(str, seds->histo_directory);
	 strcat(str, seds->histo_filename);
	 if(strlen(seds->histo_comment)) {
	    strcat(str, ".");
	    se_fix_comment(seds->histo_comment);
	    strcat(str, seds->histo_comment);
	 }
	 if(!(histo_stream = fopen(str, "w"))) {
	    sprintf(mess, "Could not open histogram file : %s\n"
		    , str);
	    sii_message (mess);
	    return(0);
	 }
      }
      else {
	 fprintf(histo_stream, "\n\n");
      }
      ssm = seds->h_output;

      for(; ssm; ssm=ssm->next) {
	 fprintf(histo_stream, "%s", ssm->at);
      }
   }
   if(seds->histo_flush && histo_stream) {
      fflush(histo_stream);
   }
   return(1);
}
/* c------------------------------------------------------------------------ */

void se_histo_fin_irreg_areas()
{
    int ii, nn, mark, min, max;
    float f, f_val, f_sum=0, rcp_inc, f_min, f_max, f_mode, f_median;
    float f_mean, f_sdev, f_sum2, f_inc;
    double d, sqrt();
    char *a, *b, *c, *e, H='H', str[88], number[16];
    struct se_pairs *pair;
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();	/* solo editing struct */
    f_inc = seds->histo_increment;

    pair = seds->h_pairs;

# ifdef obsolete
    f_val = seds->histo_low;
    for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=seds->histo_increment) {
	uii_printf("%8.1f %8.1f\n", f_val, *(seds->areas_array+ii) * SQM2KM);
    }
# endif

    for(; pair; pair=pair->next) {
	f_val = seds->histo_low + .5*f_inc;
	pair->f_sum = 0;
	for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=f_inc) {
	    if(f_val < pair->f_low)
		  continue;
	    if(f_val > pair->f_high)
		  break;
	    pair->f_sum += *(seds->areas_array+ii) * SQM2KM;
	}	
	f_sum += pair->f_sum;
# ifdef obsolete
	uii_printf("From %6.1f to %6.1f: %8.1f sq.km.\n"
		  , pair->f_low, pair->f_high, f_sum);
# endif
    }
    ssm = se_pop_spair_string();
    pair = seds->h_pairs;
    f_min = f_max = pair->f_sum;

    for(; pair; pair=pair->next) {
	if(pair->f_sum >= f_max) {
	    f_max = pair->f_sum;
	    sprintf(ssm->at,
 " From %7.1f to %7.1f contains the largest area: %8.1f sq.km.\n"
		    , pair->f_low, pair->f_high, f_max);
	}
	if(pair->f_sum < f_min)
	      f_min = pair->f_sum;
    }
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "      Low area: %8.1f sq.km.\n", seds->low_area * SQM2KM);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "     High area: %8.1f sq.km.\n", seds->high_area * SQM2KM);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "Histogram area: %8.1f sq.km.\n", f_sum);
    se_append_string(&seds->h_output, ssm);
    
    f_inc = (f_max -f_min)/50.;
    rcp_inc = 1./f_inc;;
    sprintf(str, "%7.1f to %7.1f ", f_min, f_max); e = str +strlen(str);

    memset(str, ' ', sizeof(str));
    for(f_val=f_min,c=e-8; f_val <= f_max; f_val += 10*f_inc, c+=10) {
	sprintf(number, "%9.1f", f_val);
	strncpy(c, number, strlen(number));
    }
    *c++ = '\n'; *c++ = '\0';
    ssm = se_pop_spair_string();
    strcpy(ssm->at, str);
    se_append_string(&seds->h_output, ssm);

    memset(str, ' ', sizeof(str));
    for(f_val=f_min,c=e; f_val <= f_max; f_val += 10*f_inc, c+=10) {
	*c = '|';
    }
    *c++ = '\n'; *c++ = '\0';
    ssm = se_pop_spair_string();
    strcpy(ssm->at, str);
    se_append_string(&seds->h_output, ssm);

    pair = seds->h_pairs;

    for(; pair; pair=pair->next) {
	ssm = se_pop_spair_string();
	c = ssm->at;
	nn = (pair->f_sum -f_min)/f_inc;
	sprintf(c, "%7.1f to %7.1f ", pair->f_low, pair->f_high); 
	c += strlen(c);

	if( nn <= 0  && f_sum )
	    { nn = 1; }
	for(; nn--; *c++ = H);
	*c++ = '\n'; *c++ = '\0';
	se_append_string(&seds->h_output, ssm);
    }
    return;
}
/* c------------------------------------------------------------------------ */

void se_histo_fin_irreg_counts()
{
    int ii, kk, mm, nn=0, mark, min, max, med;
    float f, f_val, f_sum, ff_sum, f_inc, rcp_inc, f_mode, f_mean, f_sdev;
    float f_median;
    double d, sqrt();
    char *a, *b, *c, *e, H='H', str[88], number[16];
    struct se_pairs *pair;
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();	/* solo editing struct */

    pair = seds->h_pairs;
    f_inc = seds->histo_increment;

    for(; pair; pair=pair->next) {
	f_val = seds->histo_low + 0.5 * f_inc;
	pair->sum = 0;
	for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=f_inc) {
	    if(f_val < pair->f_low)
		  continue;
	    if(f_val > pair->f_high)
		  break;
	    pair->sum += *(seds->counts_array+ii);
	}	
# ifdef obsolete
	uii_printf("From %6.1f to %6.1f: %8.1f sq.km.\n"
		  , pair->f_low, pair->f_high, f_sum);
# endif
    }
    pair = seds->h_pairs;

    min = max = pair->sum;
    for(; pair; pair=pair->next) {
	if(pair->sum >= max) {
	    max = pair->sum;
	    f_mode = .5*(pair->f_low +pair->f_high);
	}
	if(pair->sum < min)
	      min = pair->sum;
    }
    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	"  Mode: %8.2f\n", f_mode);
    se_append_string(&seds->h_output, ssm);

    f_inc = (max -min)/50.;
    if(f_inc < 1./50.) {
	f_inc = 1./50.;
    }
    rcp_inc = 1./f_inc;
    sprintf(str, "%8.2f to %8.2f", f_inc, f_inc); e = str +strlen(str);

    memset(str, ' ', sizeof(str));
    for(ii=min,c=e-6; ii <= max; ii += 10*f_inc+.5, c+=10) {
	sprintf(number, "%7d", ii);
	strncpy(c, number, strlen(number));
    }
    *c++ = '\n'; *c++ = '\0';
    ssm = se_pop_spair_string();
    strcpy(ssm->at, str);
    se_append_string(&seds->h_output, ssm);

    memset(str, ' ', sizeof(str));
    for(ii=min,c=e+1; ii <= max; ii += 10*f_inc +.5, c+=10) {
	*c = '|';
    }
    *c++ = '\n'; *c++ = '\0';
    ssm = se_pop_spair_string();
    strcpy(ssm->at, str);
    se_append_string(&seds->h_output, ssm);
    pair = seds->h_pairs;
    
    for(; pair; pair=pair->next) {
	ssm = se_pop_spair_string();
	c = ssm->at;
	nn = (pair->sum -min)/f_inc;
	sprintf(c, "%8.2f to %8.2f ", pair->f_low, pair->f_high); 
	c += strlen(c);
	if( nn <= 0 && pair->sum > 0 ) 
	    { nn = 1; }
	for(; nn--; *c++ = H);
	*c++ = '\n'; *c++ = '\0';
	se_append_string(&seds->h_output, ssm);
    }
    return;
}
/* c------------------------------------------------------------------------ */

void se_histo_fin_areas(name)
  char *name;
{
    int ii, nn, mark, min, max;
    float f, f_val, f_sum, rcp_inc, f_min, f_max, f_mode, f_median;
    float f_mean, f_sdev, f_sum2, f_inc;
    char *dts_print();
    DD_TIME dts, *d_unstamp_time();
    double d, sqrt();
    char *a, *b, *c, *e, H='H', str[88], number[16];
    struct se_pairs *pair;
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();	/* solo editing struct */

    se_push_all_ssms(&seds->h_output);

    ssm = se_pop_spair_string();
    b = (H_IRREG SET_IN seds->histo_key) ? "irregular intervals of " :
	  "regular intervals of ";
    sprintf(ssm->at, "Areas Histogram in %s%s\n"
	    , b, seds->histogram_field);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string(); a = ssm->at;
    dts.time_stamp = seds->histo_start_time;
    sprintf(a, "From %s to ",  dts_print(d_unstamp_time(&dts)));
    a += strlen(a);
    dts.time_stamp = seds->histo_stop_time;
    sprintf(a, "%s for %s at %.1f deg.\n"
	    , dts_print(d_unstamp_time(&dts)), seds->histo_radar_name
	    , seds->histo_fixed_angle);

    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "       Missing: %8d gates\n", seds->histo_missing_count);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "   Points below:%8d gates\n", seds->low_count);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "   Points above:%8d gates\n", seds->high_count);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "Points between: %8d gates\n", seds->medium_count);
    se_append_string(&seds->h_output, ssm);

# ifdef obsolete
    nn = seds->low_count + seds->high_count +seds->medium_count;
    f_mean = seds->histo_sum/nn;
    f_sdev = sqrt((((double)nn*seds->histo_sum_sq -f_mean*f_mean)
			   /((double)nn*(nn-1))));
# else
    nn = seds->medium_count;
    f_mean = seds->histo_sum/nn;
    f_sdev = sqrt((((double)seds->histo_sum_sq -nn*f_mean*f_mean)
			   /((double)(nn-1))));
# endif
    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "  Mean: %7.1f\n", f_mean);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "  Sdev: %7.1f\n", f_sdev);
    se_append_string(&seds->h_output, ssm);

    if(nn <= 1) {
	mark = 0;
	return;
    }

    f_inc = seds->histo_increment;
    f_mode = f_val = seds->histo_low + 0.5 * f_inc;
    f_min = f_max = *seds->areas_array;
    f_sum = 0;

    for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=f_inc) {
	f = *(seds->areas_array+ii);
	f_sum += f;

	if(f > f_max) {
	    f_max = f;
	    f_mode = f_val;
	}
	if(f < f_min) {
	    f_min = f;
	}
    }

    f_sum2 = 0;
    f_val = seds->histo_low + 0.5 * f_inc;
    for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=f_inc) {
	f = *(seds->areas_array+ii);
	if((f_sum2 += f) >= .5*f_sum) {
	    f_median = f_val;
	    break;
	}
    }
    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "Median: %7.1f\n", f_median);
    se_append_string(&seds->h_output, ssm);

    if(H_IRREG SET_IN seds->histo_key) {
	se_histo_fin_irreg_areas();
	return;
    }
    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	"  Mode: %7.1f\n", f_mode);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "      Low area: %8.1f sq.km.\n", seds->low_area * SQM2KM);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "     High area: %8.1f sq.km.\n", seds->high_area * SQM2KM);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "Histogram area: %8.1f sq.km.      Volume: %.6f\n"
	    , f_sum * SQM2KM, areaXval * .001 * 1.e-6 );
    se_append_string(&seds->h_output, ssm);
    
    f_inc = (f_max -f_min)/50.;
    rcp_inc = 1./f_inc;;
    sprintf(str, "%7.1f ", f_val); e = str +strlen(str);

    memset(str, ' ', sizeof(str));
    for(f_val=f_min,c=e-8; f_val <= f_max; f_val += 10*f_inc, c+=10) {
	sprintf(number, "%9.1f", f_val*SQM2KM);
	strncpy(c, number, strlen(number));
    }
    *c++ = '\n'; *c++ = '\0';
    ssm = se_pop_spair_string();
    strcpy(ssm->at, str);
    se_append_string(&seds->h_output, ssm);

    memset(str, ' ', sizeof(str));
    for(f_val=f_min,c=e; f_val <= f_max; f_val += 10*f_inc, c+=10) {
	*c = '|';
    }
    *c++ = '\n'; *c++ = '\0';
    ssm = se_pop_spair_string();
    strcpy(ssm->at, str);
    se_append_string(&seds->h_output, ssm);

    f_val = seds->histo_low + 0.5 * seds->histo_increment;

    for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=seds->histo_increment) {
	nn = (*(seds->areas_array+ii) -f_min)/f_inc;
	sprintf(str, "%7.1f ", f_val); c = str +strlen(str);
	if( nn <= 0 && *(seds->areas_array+ii) > 0 )
	    { nn = 1; }
	for(; nn--; *c++ = H);
	*c++ = '\n'; *c++ = '\0';
	ssm = se_pop_spair_string();
	strcpy(ssm->at, str);
	se_append_string(&seds->h_output, ssm);
    }
    return;
}
/* c------------------------------------------------------------------------ */

void se_histo_fin_counts()
{
    int ii, kk, mm, nn=0, mark, min, max, med;
    float f, f_val, f_sum, ff_sum, f_inc, rcp_inc, f_mode, f_mean, f_sdev;
    float f_median;
    char *dts_print();
    DD_TIME dts, *d_unstamp_time();
    double d, sqrt();
    char *a, *b, *c, *e, H='H', str[88], number[16];
    struct se_pairs *pair;
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    struct solo_edit_stuff *seds, *return_sed_stuff();

    seds = return_sed_stuff();	/* solo editing struct */

    se_push_all_ssms(&seds->h_output);

# ifdef obsolete
    f_val = seds->histo_low + 0.5 * seds->histo_increment;
    for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=seds->histo_increment) {
	uii_printf("%7.1f %7d counts\n", f_val, *(seds->counts_array+ii));
    }
# endif

    ssm = se_pop_spair_string();
    b = (H_IRREG SET_IN seds->histo_key) ? "irregular intervals of " :
	  "regular intervals of ";
    sprintf(ssm->at, "Counts Histogram in %s%s\n"
	    , b, seds->histogram_field);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string(); a = ssm->at;
    dts.time_stamp = seds->histo_start_time;
    sprintf(a, "From %s to ",  dts_print(d_unstamp_time(&dts)));
    a += strlen(a);
    dts.time_stamp = seds->histo_stop_time;
    sprintf(a, "%s for %s at %.1f deg.\n"
	    , dts_print(d_unstamp_time(&dts)), seds->histo_radar_name
	    , seds->histo_fixed_angle);

    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "       Missing: %8d gates\n", seds->histo_missing_count);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "   Points below:%8d gates\n", seds->low_count);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "   Points above:%8d gates\n", seds->high_count);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "Points between: %8d gates\n", seds->medium_count);
    se_append_string(&seds->h_output, ssm);

# ifdef obsolete
    nn = seds->low_count + seds->high_count +seds->medium_count;
    f_mean = seds->histo_sum/nn;
    f_sdev = sqrt((((double)nn*seds->histo_sum_sq -f_mean*f_mean)
			   /((double)nn*(nn-1))));
# else
    nn = seds->medium_count;
    f_mean = seds->histo_sum/nn;
    f_sdev = sqrt((((double)seds->histo_sum_sq -nn*f_mean*f_mean)
			   /((double)(nn-1))));
# endif
    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "  Mean: %8.2f\n", f_mean);
    se_append_string(&seds->h_output, ssm);

    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "  Sdev: %8.2f\n", f_sdev);
    se_append_string(&seds->h_output, ssm);

    if(nn <= 1) {
	mark = 0;
	return;
    }

    f_mode = f_val = seds->histo_low + 0.5 * seds->histo_increment;
    min = max = *seds->counts_array;
    nn = seds->medium_count;
    f_sum = ff_sum = kk = mm = med = 0;


    for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=seds->histo_increment) {
	kk = *(seds->counts_array+ii);
	f_sum += f_val*kk;
	ff_sum += f_val*f_val*kk;

	if(kk > max) {
	    max = kk;
	    f_mode = f_val;
	}
	if(kk < min) min = kk;
	mm += kk;
	if(!med && mm >= nn/2) {
	    med = YES;
	    f_median = f_val;
	}
    }
    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	    "Median: %8.2f\n", f_median);
    se_append_string(&seds->h_output, ssm);

    if(H_IRREG SET_IN seds->histo_key) {
	se_histo_fin_irreg_counts();
	return;
    }
    ssm = se_pop_spair_string();
    sprintf(ssm->at,
	"  Mode: %8.2f\n", f_mode);
    se_append_string(&seds->h_output, ssm);

    f_val = seds->histo_low + 0.5 * seds->histo_increment;
    f_inc = (max -min)/50.;
    rcp_inc = 1./f_inc;;
    sprintf(str, "%8.2f ", f_val); e = str +strlen(str);

    memset(str, ' ', sizeof(str));
    for(ii=min,c=e-5; ii <= max; ii += 10*f_inc+.5, c+=10) {
	sprintf(number, "%7d", ii);
	strncpy(c, number, strlen(number));
    }
    *c++ = '\n'; *c++ = '\0';
    ssm = se_pop_spair_string();
    strcpy(ssm->at, str);
    se_append_string(&seds->h_output, ssm);

    memset(str, ' ', sizeof(str));
    for(ii=min,c=e; ii <= max; ii += 10*f_inc +.5, c+=10) {
	*c = '|';
    }
    *c++ = '\n'; *c++ = '\0';
    ssm = se_pop_spair_string();
    strcpy(ssm->at, str);
    se_append_string(&seds->h_output, ssm);

    for(ii=0; ii < seds->histo_num_bins; ii++, f_val+=seds->histo_increment) {
	nn = (*(seds->counts_array+ii) -min)/f_inc;
	sprintf(str, "%8.2f ", f_val); c = str +strlen(str);
	if( nn <= 0 && *(seds->counts_array+ii) > 0 )
	    { nn = 1; }
	for(; nn--; *c++ = H);
	*c++ = '\n'; *c++ = '\0';
	ssm = se_pop_spair_string();
	strcpy(ssm->at, str);
	se_append_string(&seds->h_output, ssm);
    }

    return;
}
/* c------------------------------------------------------------------------ */

void se_histo_init(seds)
  struct solo_edit_stuff *seds;
{
    int nb;
    struct se_pairs *pair;

    seds->low_count =
	  seds->medium_count =
		seds->high_count =
		      seds->histo_sum =
			    seds->histo_sum_sq = 0;

    if(!(H_AREAS SET_IN seds->histo_key || H_BINS SET_IN seds->histo_key)) {
	seds->histo_key |= H_BINS;
    }

    if(H_IRREG SET_IN seds->histo_key) { /* find min and max */
	nb = H_BIN_MAX;
	pair = seds->h_pairs;
	seds->histo_low = pair->f_low;
	seds->histo_high = pair->f_high;

	for(pair=pair->next; pair; pair=pair->next) {
	    if(pair->f_low < seds->histo_low)
		  seds->histo_low = pair->f_low;
	    if(pair->f_high > seds->histo_high)
		  seds->histo_high = pair->f_high;
	}
	seds->histo_increment = (seds->histo_high -seds->histo_low)/nb;
    }
    else {
	nb = (seds->histo_high -seds->histo_low)/seds->histo_increment;
    }

    if(H_AREAS SET_IN seds->histo_key) {	/* set up for areas */
	if(nb != seds->histo_num_bins) {
	    if(seds->areas_array)
		  free(seds->areas_array);
	    seds->areas_array = (float *)malloc((nb+1)*sizeof(float));
	}
	memset(seds->areas_array, 0, (nb+1)*sizeof(float));
	seds->low_area = seds->high_area = 0;
    }
    else {			/* counts! */
	if(nb != seds->histo_num_bins) {
	    if(seds->counts_array)
		  free(seds->counts_array);
	    seds->counts_array = (int *)malloc((nb+1)*sizeof(int));
	}
	memset(seds->counts_array, 0, (nb+1)*sizeof(int));
    }
    seds->histo_num_bins = nb;
    seds->histo_missing_count = 0;
    return;
}
/* c------------------------------------------------------------------------ */

int se_histo_ray(arg, cmds)	/* #do-histogram# */
  int arg;
  struct ui_command *cmds;	
{
    /* 
     * this routine does a histogram for this pass
     * on the specified field 
     * using the boundary mask
     */
# define PIOVR360 .00872664626
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    static int first_time;
    int ii, jj, kk, mm, nn, pn, sn, bad, mark;
    int nc, ndx_ss;
    double d, dr, r1, r2;
    struct dd_ray_sector *ddrc, *dd_ray_coverage();
    float rcp_inc;
    float scale, rcp_scale, bias, area, *rr;
    float *low_area, *high_area, *between_area;
    int *low_counts, *high_counts, *between_counts;
    int scaled_low, scaled_high, scaled_inc;
    char *name, radar[12], *str_terminate(), histo_title[88];
    char *dd_radar_namec();
    static char file_name[128];

    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    struct dds_structs *dds;

    short *ss, *zz, *bnd, xx;
    /* boundary mask is set to 1 if the corresponding cell satisfies
     * conditions of the boundaries
     */

    seds = return_sed_stuff();	/* solo editing struct */
    name = seds->histogram_field;
    if(seds->finish_up) {
	if(H_BINS SET_IN seds->histo_key) 
	      se_histo_fin_counts(name);
	else
	      se_histo_fin_areas(name);
	se_histo_output();
	return(1);
    }
    if(seds->process_ray_count == 1) {
	first_time = YES;
    }
    if(seds->use_boundary && seds->boundary_exists && !seds->num_segments) {
	return(1);		/* ray does not intersect the boundary */
    }
    sn = strlen(name);
    bnd = (short *) seds->boundary_mask;
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;

    if((pn = dd_find_field(dgi, name)) < 0) {
	uii_printf("Source parameter %s not found for copy\n", name);
	seds->punt = YES;
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[pn];
# else
    ss = (short *)((char *)dds->rdat[pn] +sizeof(struct paramdata_d));
# endif
    bad = dds->parm[pn]->bad_data;
    scale = dds->parm[pn]->parameter_scale;
    rcp_scale = 1./scale;
    bias = dds->parm[pn]->parameter_bias;
    rr = dds->celv->dist_cells;
    dr = *(rr+1) - (*rr);

    if(first_time) {
	first_time = NO;
	seds->histo_start_time = dgi->time;
	seds->histo_fixed_angle = dgi->dds->swib->fixed_angle;
	strcpy(seds->histo_radar_name
	       , dd_radar_namec(dgi->dds->radd->radar_name));
	/*
	 * set up for this run
	 * create a file name from the first ray
	 */
	if(!(*seds->histo_directory)) {
	    strcpy(seds->histo_directory, seds->sfic->directory_text);
	}
	dd_file_name("hgm", (long)seds->histo_start_time
		     , seds->histo_radar_name, 0, seds->histo_filename);
	se_histo_init(seds);
	areaXval = 0;
    }
    scaled_low = DD_SCALE(seds->histo_low, scale, bias);
    scaled_high = DD_SCALE(seds->histo_high, scale, bias);
    scaled_inc = DD_SCALE(seds->histo_increment, scale, bias);
    rcp_inc = 1./scaled_inc;
    nc = dgi->clip_gate +1;
    seds->histo_stop_time = dgi->time;

    /*
     * loop through the data
     */

    for(ndx_ss=0; ndx_ss < nc; ndx_ss++) {

	if(!(*(bnd+ndx_ss)))
	      continue;

	if((xx = *(ss+ndx_ss)) == bad ) {
	    seds->histo_missing_count++;
	    continue;
	}

	if(H_BINS SET_IN seds->histo_key) {
	    if(xx < scaled_low) {
		seds->low_count++;
	    }
	    else if(xx >= scaled_high) {
		seds->high_count++;
	    }
	    else {
		seds->medium_count++;
		kk = (xx-scaled_low)*rcp_inc;
# ifdef obsolete
		if(kk < 0 || kk >= seds->histo_num_bins) {
		   mark = 0;
		}
# endif
		(*(seds->counts_array+kk))++;
		d = DD_UNSCALE(xx, rcp_scale, bias);
		seds->histo_sum += d;
		seds->histo_sum_sq += d*d;
	    }
	}
	else {			/* areas! */
	    if(ndx_ss) {
# ifdef obsolete
		dr = *(rr+ndx_ss) -(*(rr+ndx_ss-1));
# endif
		r1 = *(rr+ndx_ss-1);
		r2 = *(rr+ndx_ss);
	    }
	    else {
		r1 = 0;
		r2 = *rr;
	    }
	    ddrc = dd_ray_coverage
		  (dgi, dgi->source_rat, seds->sweep_ray_count, 0);
# ifdef obsolete
	    area = fabs(ddrc->sector) * PIOVR360 *
		  (2.* (*(rr+ndx_ss))*dr +SQ(dr));
# endif
	    area = fabs(ddrc->sector) * PIOVR360 * ( SQ(r2) - SQ(r1) );
		
	    if(xx < scaled_low) {
		seds->low_count++;
		seds->low_area += area;
	    }
	    else if(xx >= scaled_high) {
		seds->high_count++;
		seds->high_area += area;
	    }
	    else {
		seds->medium_count++;
		areaXval += area * DD_UNSCALE( xx, rcp_scale, bias );
		kk = (xx-scaled_low)*rcp_inc;
		*(seds->areas_array+kk) += area;
		d = DD_UNSCALE(xx, rcp_scale, bias);
		seds->histo_sum += d;
		seds->histo_sum_sq += d*d;
	    }
	}
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_histog_setup(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
    /* #area-histogram# */
    /* #count-histogram# */
    /* #irregular-histogram-bin# */
    /* #regular-histogram-parameters# */
    /* #histogram-comment# */
    /* #histogram-directory# */
    /* #new-histogram-file# */
    /* #append-histogram-to-file# */
    /* #dont-append-histogram-to-file# */
    /* #map-boundary# */
    /* #site-list# */
    /* #show-site-values# */
    /* ## */
    /* ## */
    /* ## */
    /* ## */
    /* ## */

    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    int ii, nn, mark, size, ww, skip;
    char *a, *bb, *gg, *get_tagged_string(), str[256], *se_unquote_string();
    static char gauge_dir[256], *gauge_file;
    char *buf, *absorb_zmap_bnd(), *slash_path();
    struct solo_edit_stuff *seds, *return_sed_stuff();
    float f, f_low, f_high, f_inc;
    struct se_pairs *hp, *hpn;
    struct solo_str_mgmt *ssm, *se_pop_spair_string();
    struct zmap_points *zmpc;
    static int site_list_count = 0;


    seds = return_sed_stuff();

    if(strncmp(cmds->uc_text, "area-histogram", 9) == 0 ||
       strncmp(cmds->uc_text, "count-histogram", 9) == 0) {
	strcpy(seds->histogram_field, cmdq->uc_text);
	seds->num_irreg_bins = 0;
	for(hp=seds->h_pairs; hp;) { /* free irreg pairs if any */
	    hpn = hp->next;
	    free(hp);
	    hp = hpn;
	}	      
	seds->h_pairs = NULL;
	seds->histo_key = strncmp(cmds->uc_text, "area-histogram", 9) == 0
	      ? H_AREAS : H_BINS;
    }
    else if(strncmp(cmds->uc_text, "irregular-histogram-bin", 11) == 0) {
	seds->histo_key |= H_IRREG;
	seds->num_irreg_bins++;
	hp = (struct se_pairs *)malloc(sizeof(struct se_pairs));
	memset(hp, 0, sizeof(struct se_pairs));
	if(!seds->h_pairs) {
	    seds->h_pairs = hp;
	}
	else {
	    seds->h_pairs->last->next = hp;
	}
	seds->h_pairs->last = hp;
	hp->f_low = (cmdq++)->uc_v.us_v_float;
	hp->f_high = (cmdq++)->uc_v.us_v_float;
    }
    else if(strncmp(cmds->uc_text, "regular-histogram-parameters", 11) == 0) {
	seds->histo_key |= H_REG;
	seds->histo_low = (cmdq++)->uc_v.us_v_float;
	seds->histo_high = (cmdq++)->uc_v.us_v_float;
	seds->histo_increment = (cmdq++)->uc_v.us_v_float;
    }
    else if(strncmp(cmds->uc_text, "histogram-comment", 15) == 0) {
	se_unquote_string(seds->histo_comment, cmdq->uc_text);
	seds->histo_output_key = SE_HST_COPY;
    }
    else if(strncmp(cmds->uc_text, "histogram-directory", 15) == 0 ||
	    strncmp(cmds->uc_text, "xy-directory", 6) == 0) {
	se_unquote_string(seds->histo_directory, cmdq->uc_text);
	seds->histo_output_key = SE_HST_COPY;
    }
    else if(strncmp(cmds->uc_text, "histogram-flush", 15) == 0) {
       seds->histo_flush = YES;
    }
    else if(strncmp(cmds->uc_text, "new-histogram-file", 11) == 0) {
	if(histo_stream) {
	    fclose(histo_stream);
	    histo_stream = NULL;
	}
	seds->histo_output_key = SE_HST_COPY;
    }
    else if(strncmp(cmds->uc_text, "append-histogram-to-file", 11) == 0) {
	seds->histo_output_key = SE_HST_COPY;
    }
    else if(strncmp(cmds->uc_text, "dont-append-histogram-to-file", 11) == 0) {
	seds->histo_output_key = SE_HST_NOCOPY;
    }
    else if(strncmp(cmds->uc_text, "map-boundary", 7) == 0) {
       skip = 1;
       se_unquote_string(str, cmdq->uc_text);
       bb = str;
       if( strcmp( str, "test" ) == 0 ) {
	  bb = "/scr/hotlips/oye/spol/cases/bnd.square";
	  bb = "/scr/hotlips/oye/src/spol/misc/tfb.txt";
       }
       mark = 0;
       if( strcmp( str, "cases" ) == 0 ) {
	  bb = "/scr/hotlips/oye/spol/cases/walnut10";
	  skip = 5;
       }
	buf = absorb_zmap_bnd( bb, skip, &size );
	if( buf ) {
	    se_unpack_bnd_buf( buf, size );
	    free( buf );
	}
    }
    else if(strncmp(cmds->uc_text, "site-list", 9) == 0) {
	if( !site_list_count++ ) {
	    if( ( gg = getenv( "GAUGES_DIR" )) ||
		( gg = getenv( "RAIN_GAUGES_DIR" )) ) {
		slash_path( gauge_dir, gg );
	    }
	    else {
		slash_path( gauge_dir, "/scr/hotlips/oye/spol/cases" );
	    }
	    gauge_file = gauge_dir + strlen( gauge_dir );
	}

	se_unquote_string(str, cmdq->uc_text);
	bb = str;

	if( strcmp( str, "test" ) == 0 ) {
	    bb = "/scr/hotlips/oye/spol/cases/walnut_gauges";
	    bb = "/scr/hotlips/oye/src/spol/misc/precip98_gauges.txt";
	}
	else if( !( gg = strchr( str, '/' ))) {
	    /* we do not have a full path name
	     */
	    strcpy( gauge_file, str );
	    bb = gauge_dir;
	}
	/* code for these routines is in "se_bnd.c"
	 */
	absorb_zmap_pts( &seds->top_zmap_list, bb );
    }
    else if(strncmp(cmds->uc_text, "show-site-values", 7) == 0) {
	if( !seds->top_zmap_list ) {
	    uii_printf( "No zmap lists exist!\n" );
	    return(1);
	}
	if( !seds->curr_zmap_list ) {
	    seds->curr_zmap_list = seds->top_zmap_list;
	}
	bb = (cmdq++)->uc_text;	/* field name */
	if(cmdq->uc_ctype == UTT_END ) {
	    ww = 0;
	}
	else {
	    ww = (cmdq)->uc_v.us_v_int; /* frame number */
	    if( ww < 0 )
		{ ww = 0; }
	    else if( ww > 0 && ww <= 6 )
		{ ww--; }
	    else
		{ ww = 0; }
	}
# ifdef obsolte
	ww = (cmdq++)->uc_v.us_v_float -1; /* frame number */
# endif
	list_zmap_values( bb, ww );
    }
    else if(strncmp(cmds->uc_text, "select-site-list", 7) == 0) {
	bb = (cmdq++)->uc_text;	/* field name */
	for( zmpc = seds->top_zmap_list; zmpc ; zmpc = zmpc->next_list ) {
	    if( !strcmp( zmpc->list_id, bb ))
		{ break; }
	}
	if( zmpc ) {
	    seds->curr_zmap_list = zmpc;
	}
	else {
	    uii_printf( "zmap-list: %s cannot be found!\n", bb );
	    return( 1 );
	}
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

int se_xy_stuff(arg, cmds)
  int arg;
  struct ui_command *cmds;	
{
    /*
     *
     */
    struct ui_command *cmdq=cmds+1; /* point to the first argument */
    struct solo_edit_stuff *seds, *return_sed_stuff();
    struct dd_general_info *dgi, *dd_window_dgi();
    char *src_name, *dst_name;
    struct dds_structs *dds;
    int ii, jj, fn, fns, fnd, size=0, mark, ns, nd, nc, bad, rescale=NO;
    char *a=NULL, *b=NULL, str[256];
    unsigned short *bnd, *flag;
    short *ss, *tt, *zz;
    float srs_scale, rcp_scale, srs_bias, dst_scale, dst_bias, x, y;
    float rcp_yscale;
    DD_TIME dts;

    src_name = (cmdq++)->uc_text;
    dst_name = (cmdq++)->uc_text;
    ns = strlen(src_name);
    nd = strlen(dst_name);

    seds = return_sed_stuff();
    dgi = dd_window_dgi(seds->se_frame);
    dds = dgi->dds;

    if(seds->finish_up) {
       if(strncmp(cmds->uc_text, "xy-listing", 3) == 0) { 
# ifdef obsolete

	  /* dump end time and close file
	   */
	  dts.time_stamp = seds->histo_stop_time;
	  fprintf(x_y_stream, "#xy-stop-time: ");
	  d_unstamp_time(&dts);
	  fprintf(x_y_stream, "%02d%02d%02d%02d%02d%02d\n"
		  , dts.year -1900
		  , dts.month
		  , dts.day
		  , dts.hour
		  , dts.minute
		  , dts.second
		  );
# endif
	  fclose(x_y_stream);
       }
       return(1);
    }

    if(seds->process_ray_count == 1) {
       if(strncmp(cmds->uc_text, "xy-listing", 3) == 0) { 

	  /* open file and write headers
	   */
	  dts.time_stamp = seds->histo_start_time = dgi->time;
	  strcpy(seds->histo_radar_name
		 , dd_radar_namec(dgi->dds->radd->radar_name));
	  if(!(*seds->histo_directory)) {
	     strcpy(seds->histo_directory, seds->sfic->directory_text);
	  }
	  slash_path(str, seds->histo_directory);
	  dd_file_name("xyp", (long)seds->histo_start_time
		       , seds->histo_radar_name, 0, str+strlen(str));
	  if(strlen(seds->histo_comment)) {
	     strcat(str, ".");
	     se_fix_comment(seds->histo_comment);
	     strcat(str, seds->histo_comment);
	  }
	  /* tack on the variable names
	   */
	  sprintf(str+strlen(str), ",%s,%s", src_name, dst_name);

	  if(!(x_y_stream = fopen(str, "w"))) {
	     uii_printf("Could not open xy-listing file : %s\n"
		       , str);
	     seds->punt = YES;
	     return(0);
	  }
# ifdef obsolete
	  fprintf(x_y_stream, "#xy-radar: %s\n", seds->histo_radar_name);
	  fprintf(x_y_stream, "#xy-start-time: ");
	  d_unstamp_time(&dts);
	  fprintf(x_y_stream, "%02d%02d%02d%02d%02d%02d\n"
		  , dts.year -1900
		  , dts.month
		  , dts.day
		  , dts.hour
		  , dts.minute
		  , dts.second
		  );
	  fprintf(x_y_stream, "#xfield: %s\n", src_name);
	  fprintf(x_y_stream, "#yfield: %s\n", dst_name);
# endif
       }
    }
    seds->histo_stop_time = dgi->time;
    nc = dds->celv->number_cells;
    bnd = seds->boundary_mask;

    if((fns = dd_find_field(dgi, src_name)) < 0) {
	uii_printf("Source parameter %s not found for xy-listing\n", src_name);
	seds->punt = YES;
	return(-1);
    }
    if((fnd = dd_find_field(dgi, dst_name)) < 0) {
	uii_printf("Source parameter %s not found for xy-listing\n", dst_name);
	seds->punt = YES;
	return(-1);
    }
    rcp_scale = 1./dgi->dds->parm[fns]->parameter_scale;
    rcp_yscale = 1./dgi->dds->parm[fnd]->parameter_scale;
    srs_bias = dgi->dds->parm[fns]->parameter_bias;
    dst_bias = dgi->dds->parm[fnd]->parameter_bias;
    ss = (short *)dds->qdat_ptrs[fns];
    tt = (short *)dds->qdat_ptrs[fnd];
    bad = dds->parm[fnd]->bad_data;
    zz = ss +nc;

    /*
     * do the work!
     */
    if(strncmp(cmds->uc_text, "xy-listing", 3) == 0) { 

       for(; ss < zz; ss++,tt++,bnd++) {
	  if(*bnd && *ss != bad && *tt != bad) {
	     x = DD_UNSCALE((float)(*ss), rcp_scale, srs_bias);
	     y = DD_UNSCALE((float)(*tt), rcp_yscale, dst_bias);
	     fprintf(x_y_stream, "%.2f %.2f\n", x, y);
	  }
       }
    }
    return(1);
}  
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

