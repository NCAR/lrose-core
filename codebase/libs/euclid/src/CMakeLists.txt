cmake_minimum_required(VERSION 2.8)

set (CMAKE_VERBOSE_MAKEFILE "ON")
set (CMAKE_CXX_FLAGS "-Wall -g -O")

project(euclid)

include_directories("./include" $ENV{GTG_INC_DIR} $ENV{LOCAL_INC_DIR})

add_subdirectory(include/euclid)

set (Lib_sources
    Polygon_simplify/DPbasic.cc  
    alloc/alloc.c
    boundary/adj_star.c
    boundary/alloc_nodes.c
    boundary/bbox.c
    boundary/bdry_graph.c
    boundary/boundary_intervals.c
    boundary/fill_boundary.c
    boundary/find_extreme_pts.c
    boundary/free_clump_info.c
    boundary/gen_bdry.c
    boundary/init_ray.c
    boundary/make_star.c
    boundary/obdry_graph.c
    boundary/oboundary_intervals.c
    boundary/ogen_bdry.c
    boundary/otraverse_bdry_graph.c
    boundary/translate_bdry.c
    boundary/traverse_bdry_graph.c
    clump/adjust_intervals.c
    clump/alloc_clumps.c
    clump/alloc_rowh.c
    clump/clump_grid.c
    clump/clump_intervals.c
    clump/clump_volume.c
    clump/erode_clump.c
    clump/euclid_dist.c
    clump/extend_clump.c
    clump/extend_intervals.c
    clump/find_intervals.c
    clump/find_intervals_float.c
    clump/find_overlap.c
    clump/free_interval.c
    clump/get_intervals.c
    clump/get_intervals_float.c
    clump/intervals_to_array.c
    clump/link_intervals.c
    clump/oclump_intervals.c
    clump/ofree_clump_info.c
    clump/oprint_intervals.c
    clump/overlap.c
    clump/overlap_plane.c
    clump/overlap_rows.c
    clump/overlap_volume.c
    clump/print_intervals.c
    clump/reset_arrays.c
    clump/reset_clump_id.c
    clump/row_hdr.c
    clump/scan_interval.c
    clump/set_intervals.c
    clump/set_rp.c
    clump/small_clumps.c
    clump/sort_intervals.c
    clump/split_clump.c
    clump/stack_interval.c
    clump/transform_intervals.c
    clump/translate_array.c
    clump/translate_clump.c
    clump/translate_intervals.c
    clump/union_intervals.c
    clump/zero_clump.c
    geometry/convex_hull.c
    geometry/coord_system.c
    geometry/create_box.c
    geometry/create_trapezoid.c
    geometry/equidistant_pts.c
    geometry/fill_polygon.c
    geometry/half_plane.c
    geometry/hypot.c
    geometry/inside_poly.c
    geometry/is_rectangle.c
    geometry/lat_long.c
    geometry/linear_comb.c
    geometry/line_intersect.c
    geometry/line_point.c
    geometry/line_through_ellipse.c
    geometry/perp_projection.c
    geometry/point_in_ellipse.c
    geometry/point_in_polygon.c
    geometry/point_on_segment.c
    geometry/polygon_area.c
    geometry/polygon_centroid.c
    geometry/polygon_intersect.c
    geometry/random_bdry.c
    geometry/segment_intersect.c
    geometry/unit_vector.c
    geometry/vect_add.c
    link/link.c
    )

ADD_LIBRARY(euclid
    ${Lib_sources}
    )

INSTALL(FILES libeuclid.a
        DESTINATION $ENV{GTG_LIB_DIR}
	)

