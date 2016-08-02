/* 	$Id: dd_utils.hh,v 1.2 2009/11/25 22:01:22 dixon Exp $	 */

# ifndef DD_UTILS_HH
# define DD_UTILS_HH

# include <time.h>
# include <libgen.h>
# include <dirent.h>
# include <string.h>

# include <radar/dorade/WhichFloat.hh>
# include <radar/dorade/MedianFilter.hh>
# include <radar/dorade/AvrgList.hh>
# include <radar/dorade/dd_defines.h>

double AngDiff(float a1, float a2);

double time_from_file_name( char * fname, char * prefix );

int CleanDir( char * ref_dir, struct solo_list_mgmt * rlist, int rcount
	, int time_span, char * prefix, char * suffix
	      , int not_this_suffix );

int CleanDir2( char * ref_dir, struct solo_list_mgmt * rlist, int rcount
		 , int t_span, char * prefix );
extern "C"
{
    char *solo_list_entry( struct solo_list_mgmt *, int );
    int generic_sweepfiles
    ( const char *dir, struct solo_list_mgmt *lm, const char *prefix, 
      const char *suffix, int not_this_suffix );
}

/* c------------------------------------------------------------------------ */

# ifndef SOLO_LIST_MGMT
# define SOLO_LIST_MGMT
# define SLM_CODE

struct solo_list_mgmt {
    int num_entries;
    int sizeof_entries;
    int max_entries;
    char **list;
};
/* c------------------------------------------------------------------------ */

struct solo_str_mgmt {
    struct solo_str_mgmt *last;
    struct solo_str_mgmt *next;
    char *at;
};
# endif

// c---------------------------------------------------------------------------

union X4swapU {
    int i4;
    char byte[4];
} ;

union X2swapU {
    short i2;
    char byte[2];
} ;

enum FB_IO_State { FB_IO_EOF, FB_IO_OK, FB_IO_FAIL, FB_IO_EOT };


// c---------------------------------------------------------------------------

class basic_node {
private:
    basic_node * prev;
    basic_node * next;
public:
    basic_node()  { prev = next = NULL; }
    void set_prev( basic_node * new_prev ) { this->prev = new_prev; }
    void set_next( basic_node * new_next ) { this->next = new_next; }
    basic_node * return_prev() { return prev; }
    basic_node * return_next() { return next; }
};

// c---------------------------------------------------------------------------

class heir_node : public basic_node {
private:
public:

    heir_node() : basic_node() {}

    void set_prev( heir_node * new_prev )
    { basic_node::set_prev(( basic_node *) new_prev); }

    void set_next( heir_node * new_next )
    { basic_node::set_next(( basic_node *) new_next); }

    heir_node * return_next()
    { return (heir_node *)basic_node::return_next(); }

    heir_node * return_prev()
    { return (heir_node *)basic_node::return_prev(); }
};

// c---------------------------------------------------------------------------

class limits_node : public basic_node {

private:

    double limit1;
    double limit2;
    int hits;
    char id[16];

    inline int inSector( double ang, double ang1, double ang2 )
    {
	// assumes sector defined from ang1 clockwise to ang2
	
	if( ang1 > ang2 )		// crosses 360.
	    { return( ang >= ang1 || ang < ang2 ); }
	
	return( ang >= ang1 && ang < ang2 );
    }

public:

  limits_node() : basic_node()
  { limit1 = limit2 = 0; hits = 0; *id = '\0'; }

    inline void set_limit1( double limit ) { limit1 = limit; }
    inline void set_limit2( double limit ) { limit2 = limit; }
    inline double return_limit1() { return limit1; }
    inline double return_limit2() { return limit2; }

    int inThisSector( double angle ) {
      if( inSector( angle, limit1, limit2 )) {
	hits++;
	return YES;
      }
      return NO;
    }

    inline int withinLimits( double value )
    {
	return ( value >= limit1 && value < limit2 ) ? 1 : 0;
    }

  void reset_hit_count() { hits = 0; }
  int return_hit_count() { return hits; }
  void set_id( char * the_id ) { strcpy( id, the_id ); }
  const char * return_id() { return id; }

    void set_prev( limits_node * new_prev )
    {
	basic_node::set_prev(( basic_node *) new_prev);
    }

    void set_next( limits_node * new_next )
    {
	basic_node::set_next(( basic_node *) new_next);
    }

    limits_node * return_next()
    {
	return (limits_node *)basic_node::return_next();
    }

    limits_node * return_prev()
    {
	return (limits_node *)basic_node::return_prev();
    }
};

// c---------------------------------------------------------------------------

template < class type_node >
class basic_linked_list {

private:

    type_node * top;
    type_node * bottom;
    type_node * current;
    type_node * loop_marker;

    int num_links;
    int loop_count;

public:

    basic_linked_list()
    {
	num_links = loop_count = 0;
	top = bottom = current = loop_marker = NULL;
    }

    ~basic_linked_list()
    {
	if( !num_links )
	    { return; }
	type_node * prev_node = this->loop_reset();
	type_node * node;

	for(; num_links--; ) {
	    node = prev_node->return_next();
	    delete prev_node;
	    prev_node = node;
	}
    }

    int return_num_links() { return num_links; }

    int list_of_size( int nn ) {
	for(; nn--; ) {
	    if( !this->add_node() )
		{ return num_links; }
	}
	return num_links;
    }

    type_node * loop_reset()
    {
	if( !num_links )
	    { return NULL; }

	current = top;
	loop_count = num_links -1;
	return current;
    }

    type_node * next_node()
    {
	if( !loop_count ) {
	    return NULL;
	}
	current = current->return_next();
	loop_count--;
	return current;
    }

    type_node * add_node()
    {
	type_node * node = new type_node();

	if( !node) {
	    return NULL;
	}

	if( !num_links ) {
	    top = bottom = current = loop_marker = node;
	    top->set_prev( top );
	    top->set_next( top );
	    num_links = 1;
	    return node;
	}
	// otherwise append to list of nodes
	//
	node->set_prev( bottom );
	node->set_next( top );
	top->set_prev( node );
	bottom->set_next( node );
	bottom = node;
	current = node;
	num_links++;
	return node;
    }

    type_node * add_node( type_node * before )
    {
	if( !before )
	    { return NULL; }

	type_node * node = new type_node();

	if( !node)
	    { return NULL; }

	if( !num_links ) {
	    this->add_node( node );
	    return node;
	}
	// otherwise insert before indicated node
	//
	node->set_prev( before->return_prev() );
	node->set_next( before );
	before->set_prev( node );
	before->return_prev()->set_next( node );
	if( before == top )
	    { top = node; }
	current = node;
	num_links++;
	return node;
    }

    type_node * remove_node( type_node * node )
    {
	if( !num_links || !node )
	    { return NULL; }

	if( num_links > 1 ) {
	    if( node == current ) {
		current = node->return_next();
	    }
	    if( node == loop_marker ) {
		loop_marker = node->return_next();
	    }
	}

	if( num_links == 1 ) {
	    top = bottom = current = loop_marker = NULL;
	    num_links = 0;
	    return node;
	}
	else if( node == top ) {
	    if( current == node ) {
		current = node->return_next();
	    }
	    top = top->return_next();
	    top->set_prev( bottom );
	    bottom->set_next( top );
	    num_links--;
	    return node;
	}
	else if( node == bottom ) {
	    if( current == bottom ) {
		current = bottom->return_prev();
	    }
	    bottom = bottom->return_prev();
	    bottom->set_next( top );
	    top->set_prev( bottom );
	    num_links--;
	    return node;
	}
	node->return_prev()->set_next( node->return_next() );
	node->return_next()->set_prev( node->return_prev() );
	num_links--;
	return node;
    }
};

// c---------------------------------------------------------------------------

class special_sectors {

private:

    basic_linked_list< class limits_node >  * fxd_limits;
    basic_linked_list< class limits_node >  * az_limits;
    int single;

public:
    special_sectors( int az_only )
    {
        single = YES;
	az_limits = new basic_linked_list< class limits_node >();
    }
    special_sectors()
    {
        single = NO;
	fxd_limits = new basic_linked_list< class limits_node >();
	az_limits = new basic_linked_list< class limits_node >();
    }

    ~special_sectors()
    {
    }

  void reset_hits();
  int return_hits( char * sector_id );
  int return_hits( int sector_num );
  const char * return_sector_id( int sector_num );
  const char * max_hits_id();

  int inOneSector( float az );
  int inOneSector( float fxd, float az );

  int sector_count()
  {
    return az_limits->return_num_links();
  }

  void add_a_sector( float az1, float az2, char * id );
  void add_a_sector( float fx1, float fx2, float az1, float az2 );

};

// c---------------------------------------------------------------------------

// c---------------------------------------------------------------------------

class piraqPktXfer {
private:
    int residual;
    unsigned int residualVal;
public:
    piraqPktXfer() { residual = 0; }
    ~piraqPktXfer();
    void reset() { residual = 0; }
    int Xfer(char *, char *, int);
};


// c---------------------------------------------------------------------------

typedef double DTime;
    const DTime Bad_Time = - 1.e22;
    const DTime Seconds_To_Days =     1.157407407e-5;
    const DTime Seconds_To_Hours  =   2.777777778e-4;
    const DTime Seconds_To_Minutes = .01666666667;

    const int the_Seconds_Per_Day = 86400;
    const int Max_Years = 100;
    const int Wild_Guess = 1992;
    const int Reference_Year = 1970;

class DD_Time {

private:

    DTime reg_year[13];	//num secs in each month 
    DTime leap_year[13]; 

    DTime *accumulated_months[Max_Years + 1];
    DTime accumulated_years[Max_Years + 1];
    
    char the_ascii_time[32];

    DTime the_time_stamp;
    DTime the_additional_seconds;
    int   the_julian_day;
    int   the_year;
    int   the_month;
    int   the_day;
    int   the_hour;
    int   the_minute;
    int   the_second;
    int   the_millisecond;


public:

    DD_Time();
    ~DD_Time() { };

    DTime   stamp_the_time();
    /*
     * "stamp_the_time()" expects year, month, day, and
     * seconds past midnight to be set.
     * It will NOT use the hour, min, sec, and ms info
     *
     * Fold the julian day into "the_additional_seconds"
     * e.g.
     * x.set_additional_seconds(seconds_past_midnight +
     *     (julian_day - 1) * x.Seconds_Per_Day());
     * and make sure the month and day are zero by doing a "reset()" first
     * or setting them both to zero
     *
     * This routine gives a time similar if not the same as unix time
     * but does not worry about time zones and daylight saving time.
     */
    void    reset();
    void    unstamp_the_time();
    char *  ascii_time();
    char *  ascii_numeric_time();

    DTime   time_stamp() { return(the_time_stamp); }
    time_t  unix_time() { return (time_t)the_time_stamp; }
    int     microsecond() { return (time_t)
				 ((the_time_stamp -
				   (time_t)the_time_stamp) * 1.e6); }
    int     year() { return(the_year); }
    int     julian_day() { return(the_julian_day); }
    int     month() { return(the_month); }
    int     day() { return(the_day); }
    int     hour() { return(the_hour); }
    int     minute() { return(the_minute); }
    int     second() { return(the_second); }
    int     millisecond() { return(the_millisecond); }
    int     Seconds_Per_Day() { return(the_Seconds_Per_Day); }

    void    accept_time_stamp(DTime dt) { the_time_stamp = dt; }
    void    set_year(int yy) { the_year = yy > 1900 ? yy : yy +1900; }
    void    set_month(int mm) { the_month = mm; }
    void    set_day(int dd) { the_day = dd; }
    void    set_additional_seconds(DTime dt) { the_additional_seconds = dt; }
};

// c---------------------------------------------------------------------------

typedef struct param_pair {
    struct param_pair *next;
    char *id;
    double val;
} XTRA_PARAM_PAIR;

// c---------------------------------------------------------------------------


class extra_parameters {
private:
    struct param_pair *first_pair;
public:
    extra_parameters();
    int  there_is_a(char *);	// verifies existence
    double return_val_for(char *); // returns val associated with char string
    ~extra_parameters();	// deletes all parameter pairs
    void add(char *, double);	// adds a new (id,val) pair
};

// c---------------------------------------------------------------------------

// c---------------------------------------------------------------------------

# endif // DD_UTILS_HH





