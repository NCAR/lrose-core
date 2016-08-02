/*
	pointdata.h

	point data class header file

*/

#ifndef __POINTDATA_H
#define __POINTDATA_H

enum e_metadatatype { metadata, pointdata, metar, ltning };

class obs_site {
public:
    char *short_name, *long_name;
    int site_id;
    float lat, long, ht;
    };

class metadata {
public:
    e_metadatatype ptype;
    time_t obs_time;
    int	site_id;
    bool valid;
    metadata();
    };

class point_data public metadata {
public:
	point_data(char *text_line = NULL);
	virtual bool read_text_line(char *text_line);
	};

class metar public point_data {
public:
    float wnd_dir,
	wnd_spd,
	max_wind_gust,
	air_temp,
	dwpt,
	relh,
	sea_press,
	q10mnt_prcp,
	q9am_prcp,
	vis,
	cavok;
    metar(char *text_line = NULL);
    virtual bool read_text_line(char *text_line);
    };

#endif
