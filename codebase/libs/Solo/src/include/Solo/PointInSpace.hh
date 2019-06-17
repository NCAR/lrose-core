class PointInSpace {

public:

  /* set this bit in state if there is a lat/lon/alt/earthr */
  static const long PISP_EARTH =            0x00000001;

  /* set this bit in state if x,y,z vals relative to lat/lon */
  static const long PISP_XYZ =              0x00000002;

  /* set this bit in state if az,el,rng relative to lat/lon */
    static const long PISP_AZELRG =         0x00000004;

  /* set this bit in state for aircraft postitioning relative to lat/lon                     
   * rotation angle, tilt, range, roll, pitch, drift, heading                                
   */
    static const long PISP_AIR =            0x00000008;
    static const long PISP_PLOT_RELATIVE =  0x00000010;
    static const long PISP_TIME_SERIES =    0x00000020;



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

