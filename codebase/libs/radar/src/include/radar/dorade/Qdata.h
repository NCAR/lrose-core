/* 	$Id: Qdata.h,v 1.1 2008/01/24 22:22:32 rehak Exp $	 */

# ifndef INCQdatah
# define INCQdatah

struct qparamdata_d {		/* new for 1995 */

    char pdata_desc[4];		/* parameter data descriptor identifier: ASCII
				 * characters "QDAT" for a block that contains
				 * the data plus some supplemental and
				 * identifying information */
    
    long  pdata_length;		/* parameter data descriptor length in bytes.
				 * this represents the size of this header
				 * information plus the data
				 *
				 * for this data block the start of the data
				 * is determined by using "offset_to_data"
				 * in the corresponding parameter descriptor
				 * "struct parameter_d"
				 * the offset is from the beginning of
				 * this descriptor/block
				 */
    
    char  pdata_name[8];	/* name of parameter */

    long  extension_num;
    long  config_num;		/* facilitates indexing into an array
				 * of radar descriptors where the radar
				 * characteristics of each ray and each
				 * parameter might be unique such as phased
				 * array antennas
				 */
    short first_cell[4];
    short num_cells[4];
				/* first cell and num cells demark
				 * some feature in the data and it's
				 * relation to the cell vector
				 * first_cell[n] = 0 implies the first datum
				 * present corresponds to "dist_cells[0]
				 * in "struct cell_d"
				 * for TRMM data this would be the
				 * nominal sample where the cell vector is
				 * at 125 meter resolution instead of 250 meters
				 * and identified segments might be the
				 * rain echo oversample "RAIN_ECH" and the
				 * surface oversample "SURFACE"
				 */
    float criteria_value[4];	/* criteria value associated
				 * with a criteria name
				 * in "struct parameter_d"
				 */
}; /* End of Structure */



typedef struct qparamdata_d qparamdata_d;
typedef struct qparamdata_d QPARAMDATA;

# endif /* INCQdatah */






