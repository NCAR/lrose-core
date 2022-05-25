//#include <Solo/OneBoundary.hh>
#include <Solo/BoundaryPointMap.hh>
#include <Solo/PointInSpace.hh>
#include <Solo/SoloFunctions.hh>
#include <Solo/GeneralDefinitions.hh>

//void SoloFunctionsApi::CreateBoundary() {
//
//}

//SoloFunctionsApi::SoloFunctionsApi() {}


// get the boundary mask for a given:
// - list of boundary vertices
// - ray geometry
//
//short *SoloFunctionsApi::GetBoundaryMask(OneBoundary *boundaryList,
//                                           PointInSpace *radar_origin,
//                                           PointInSpace *boundary_origin,
void se_get_boundary_mask(long *xpoints, long *ypoints, int npoints,
					 //float radar_origin_x,
           //  float radar_origin_y,
           //  float radar_origin_z,
             float radar_origin_latitude,
             float radar_origin_longitude,
             float radar_origin_altitude,
             float boundary_origin_tilt,
             // float boundary_origin_x,
             // float boundary_origin_y,
             // float boundary_origin_z,
             float boundary_origin_latitude,
             float boundary_origin_longitude,
             float boundary_origin_altitude,
             int nGates,
             float gateSize,
             float distanceToCellNInMeters,
             float azimuth,
             int radar_scan_mode,
             int radar_type,
             float tilt_angle,
			       bool *boundary_mask_out) {


  // map flat data to internal data structures ...
  PointInSpace *radar_origin = new PointInSpace;
  PointInSpace *boundary_origin = new PointInSpace;

  // insert xy-points into OneBoundary structure
  OneBoundary *boundary = new OneBoundary();
  BoundaryPointMap map;

  // Fill the OneBoundary object with the boundary points
  for (int i=0; i<npoints; i++) {
    map.xse_add_bnd_pt(xpoints[i], ypoints[i], boundary);
  }

  //  radar_origin->x = radar_origin_x;
  //  radar_origin->y = radar_origin_y;
  //  radar_origin->z = radar_origin_z;
  radar_origin->latitude = radar_origin_latitude;
  radar_origin->longitude = radar_origin_longitude;
  radar_origin->altitude = radar_origin_altitude;

  //boundary_origin->x = boundary_origin_x;
  //boundary_origin->y = boundary_origin_y;
  //boundary_origin->z = boundary_origin_z;
  boundary_origin->latitude = boundary_origin_latitude;
  boundary_origin->longitude = boundary_origin_longitude;
  boundary_origin->altitude = boundary_origin_altitude;
  boundary_origin->tilt = boundary_origin_tilt;

  //  send the filled boundary object to the function that will
  // create the mask for the current ray geometry
  map.get_boundary_mask(  
  boundary,
    // bool new_sweep,  // assume new_sweep                                                                       
    //        bool operate_outside_bnd,                                                                           
    //bool shift_bnd,  // always shift                                                                            
    radar_origin,
    boundary_origin,
    nGates,
    gateSize,
    distanceToCellNInMeters,
    azimuth,    // TODO: are azimuth & rotation_angle the same thing? YES                                   
    radar_scan_mode,
    radar_type,
    tilt_angle,
    boundary_mask_out);

  delete boundary;
  delete radar_origin;
  delete boundary_origin;

  //return boundary_mask;
} 

