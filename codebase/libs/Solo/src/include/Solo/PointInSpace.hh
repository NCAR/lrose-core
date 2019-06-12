class PointInSpace {

public:

    char name_struct[4];        /* "PISP" */
    long sizeof_struct;

    double time;                /* unix time */
    double earth_radius;
    double latitude;
    double longitude;
    double altitude;

    float roll;
    float pitch;
    float drift;
    float heading;
    float x;
    float y;
    float z;
    float azimuth;
    float elevation;
    float range;
    float rotation_angle;
    float tilt;

    long cell_num;
    long ndx_last;
    long ndx_next;
    long state;                 /* which bits are set */

    char id[16];

  void print();
};

