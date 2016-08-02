/* 	$Id: super_SWIB.h,v 1.1 2008/01/24 22:22:32 rehak Exp $	 */


# ifndef INCsuper_SWIBh
# define INCsuper_SWIBh

# define   MAX_KEYS 8

# define      KEYED_BY_TIME 1
# define   KEYED_BY_ROT_ANG 2
# define  SOLO_EDIT_SUMMARY 3

# define        NDX_ROT_ANG 0
# define           NDX_SEDS 1

# define SWIB_NEW_VOL 0x1

struct key_table_info {
    long offset;
    long size;
    long type;
};

struct super_SWIB {
    char name_struct[4];	/* "SSWB" */
    long sizeof_struct;
    /* parameters from the first version */
    long last_used;		/* Unix time */
    long start_time;
    long stop_time;
    long sizeof_file;
    long compression_flag;
    long volume_time_stamp;	/* to reference current volume */
    long num_params;		/* number of parameters */

    /* end of first version parameters */

    char radar_name[8];

    double d_start_time;
    double d_stop_time;
    /*
     * "last_used" is an age off indicator where > 0 implies Unix time
     * of the last access and
     * 0 implies this sweep should not be aged off
     */
    long version_num;
    long num_key_tables;
    long status;
    long place_holder[7];
    struct key_table_info key_table[MAX_KEYS];
    /*
     * offset and key info to a table containing key value such as
     * the rot. angle and the offset to the corresponding ray
     * in the disk file
     */
};

struct super_SWIB_v0 {
    char name_struct[4];	/* "SSWB" */
    int sizeof_struct;

    long last_used;		/* Unix time */
    /*
     * "last_used" is an age off indicator where > 0 implies Unix time
     * of the last access and
     * 0 implies this sweep should not be aged off
     */
    long start_time;
    long stop_time;
    int sizeof_file;
    int compression_flag;
    long volume_time_stamp;	/* to reference current volume */
    int num_params;		/* number of parameters */
};

typedef struct super_SWIB super_SWIB;
typedef struct super_SWIB SUPERSWIB;

# endif  /* INCsuper_SWIBh */

