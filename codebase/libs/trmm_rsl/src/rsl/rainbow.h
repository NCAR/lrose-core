/* Control characters used in Rainbow data header */

#define SOH 0x01    /* Start of header: first character in Rainbow file. */
#define ETX 0x03    /* End of text:     end of header.                   */
#define  CR 0x0d    /* Carriage return: end of line, same as '\r'.       */
#define ETB 0x17    /* End of transmission block: marks header sections. */

/* Other constants */

#define SCAN_DATA   2
#define VOLUME_SCAN 100

/* Declarations */

struct elev_params {
    float elev_angle;
    float az_rate;
    int prf_high;
    int prf_low;
    float maxrange;
};

typedef struct {
    int filetype;
    int product;
    int datatype;
    int compressed;
    char radarname[9];
    int month;
    int day;
    int year;
    int hour;
    int minute;
    int sec;
    float lat;          /* radar latitude, degrees  */
    float lon;          /* radar longitude, degrees */
    int nsweeps;
    int az_start;       /* degrees */
    int az_stop;        /* degrees */      
    float az_step;      /* degrees */
    float range_start;  /* km */
    float range_stop;   /* km */
    float range_step;   /* km */
    int nbins;
    float bin_resolution;  /* km */
    int nvalues;
    int pulse_width_code;
    int zdata;
    int vdata;
    int wdata;
    int unfolding;
    int cdata;
    int ddata;
    int uzdata;
    struct elev_params *elev_params[20];
} Rainbow_hdr;

/* Function prototypes */

Radar *RSL_rainbow_to_radar(char *infile);
int rainbow_data_to_radar(Radar *radar, Rainbow_hdr rainbow_hdr, FILE *fp);
