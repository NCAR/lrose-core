#!/usr/bin/env python3

# ========================================================================== #
#
# Replace global strings in files.
#
# ========================================================================== #

import os
import sys
import shutil
import subprocess

import string
from os.path import join, getsize
import subprocess
from optparse import OptionParser
from sys import platform

def main():

    global options
    global debug
    global replaceDict

    global thisScriptName
    thisScriptName = os.path.basename(__file__)

    # parse the command line

    usage = "usage: %prog [options]: replaces strings in file"

    parser = OptionParser(usage)
    parser.add_option('--debug',
                      dest='debug', default=False,
                      action="store_true",
                      help='Set debugging on')
    parser.add_option('--path',
                      dest='path', default='./junk',
                      help='Path for file to be modified')

    (options, args) = parser.parse_args()

    if (options.debug):
        print("  Running " + thisScriptName, file=sys.stderr)
        print("  Options:", file=sys.stderr)
        print("    debug: ", options.debug, file=sys.stderr)
        print("    path: ", options.path, file=sys.stderr)

    # create the replacement dictionary

    createReplaceDict()

    # Opening file in read only 
    # mode using the open() function 

    with open(options.path, 'r') as file: 
  
        # Reading the content of the file 
        # using the read() function and storing 
        # them in a new variable 
        data = file.read() 
  
        # Searching and replacing the text 
        # using the replace() function
        for x, y in replaceDict.items():
            data = data.replace(x, y) 
        
    # Opening our text file in write only 
    # mode to write the replaced content 
    with open(options.path, 'w') as file: 
            
        # Writing the replaced data in our 
        # text file 
        file.write(data) 
  
    # done

    sys.exit(0)

########################################################################
# set up replace dictionary

def createReplaceDict():

    global replaceDict
    replaceDict = {}

    replaceDict["gd.idle_reset_seconds"] = "_params.idle_reset_seconds"

  gd.model_run_list_hours = gd.uparams->getLong("cidd.model_run_list_hours",24);
  gd.model_run_list_hours = gd.uparams->getLong("cidd.model_run_list_hours",24);
  gd.close_popups =   gd.uparams->getLong("cidd.close_popups", 0);
  gd.disable_pick_mode =   gd.uparams->getLong("cidd.disable_pick_mode", 1);
  gd.clip_overlay_fields =   gd.uparams->getLong("cidd.clip_overlay_fields", 0);
  gd.output_geo_xml =   gd.uparams->getLong("cidd.output_geo_xml", 0);
  gd.use_latlon_in_geo_xml =   gd.uparams->getLong("cidd.use_latlon_in_geo_xml", 0);
  gd.replace_underscores =   gd.uparams->getLong("cidd.replace_underscores", 1);
  gd.image_dir = gd.uparams->getString("cidd.image_dir", "/tmp/image_dir");

  gd.html_mode = gd.uparams->getLong("cidd.html_mode", 0);
  gd.run_once_and_exit = gd.uparams->getLong("cidd.run_once_and_exit",0);
  
  gd.transparent_images = gd.uparams->getLong("cidd.transparent_images", 0);
  
  gd.save_images_to_day_subdir = gd.uparams->getLong("cidd.save_images_to_day_subdir", 0);
  
  gd.horiz_image_fname = gd.uparams->getString("cidd.horiz_image_fname", "cidd_horiz_view.png");
  gd.horiz_image_command = gd.uparams->getString("cidd.horiz_image_command", "");
  
  gd.vert_image_fname = gd.uparams->getString("cidd.vert_image_fname", "cidd_vert_view.png");
  gd.vert_image_command = gd.uparams->getString("cidd.vert_image_command", "");
  
  gd.image_ext = gd.uparams->getString("cidd.image_ext", "png");
  
  gd.image_horiz_prefix = gd.uparams->getString("cidd.image_horiz_prefix", "CP");
  gd.image_vert_prefix = gd.uparams->getString("cidd.image_vert_prefix", "CV");
  gd.image_name_separator = gd.uparams->getString("cidd.image_name_separator", "_");
  
  gd.image_convert_script = gd.uparams->getString("cidd.image_convert_script", "convert_image.csh");
  
  gd.series_convert_script = gd.uparams->getString("cidd.series_convert_script", "make_anim.csh");

  gd.complex_command_timeout_secs =  gd.uparams->getLong("cidd.complex_command_timeout_secs",180);
  gd.print_script = gd.uparams->getString("cidd.print_script", "");
  gd.simple_command_timeout_secs =  gd.uparams->getLong("cidd.simple_command_timeout_secs",30);

  gd.label_time_format = gd.uparams->getString("cidd.label_time_format", "%m/%d/%y %H:%M:%S");

  gd.moviestart_time_format = gd.uparams->getString("cidd.moviestart_time_format", "%H:%M %m/%d/%Y");

  gd.frame_range_time_format = gd.uparams->getString("cidd.frame_range_time_format", "%H:%M");

  gd.movieframe_time_format = gd.uparams->getString("cidd.movieframe_time_format", "%H%M");

  gd.movieframe_time_mode = gd.uparams->getLong("cidd.movieframe_time_mode", 0);

  gd.layer_legends_on = (gd.uparams->getLong("cidd.layer_legends_on", 1) & 1);
  gd.cont_legends_on = (gd.uparams->getLong("cidd.cont_legends_on", 1) & 1);
  gd.wind_legends_on = (gd.uparams->getLong("cidd.wind_legends_on", 1) & 1);
  gd.contour_line_width = gd.uparams->getLong("cidd.contour_line_width", 1);
  gd.smooth_contours = gd.uparams->getLong("cidd.smooth_contours", 0);
  gd.use_alt_contours = gd.uparams->getLong("cidd.use_alt_contours", 0);
  gd.add_noise = gd.uparams->getLong("cidd.add_noise", 0);
  gd.special_contour_value = gd.uparams->getDouble("cidd.special_contour_value", 0.0);
  gd.map_bad_to_min_value =  gd.uparams->getLong("cidd.map_bad_to_min_value", 0);
  gd.map_missing_to_min_value =  gd.uparams->getLong("cidd.map_missing_to_min_value", 0);

  gd.draw_main_on_top = (gd.uparams->getLong("cidd.draw_main_on_top", 0) & 1);

  gd.mark_latest_click_location = gd.uparams->getLong("cidd.mark_latest_click_location", 0);

  gd.drawing_mode = 0;

  // products
  
  gd.products_on = gd.uparams->getLong("cidd.products_on", 1);
  gd.product_line_width = gd.uparams->getLong("cidd.product_line_width", 1);
  gd.product_font_size = gd.uparams->getLong("cidd.product_font_size", 1);
  
  gd.always_get_full_domain = gd.uparams->getLong("cidd.always_get_full_domain", 0);
  gd.do_not_clip_on_mdv_request = gd.uparams->getLong("cidd.do_not_clip_on_mdv_request", 0);
  gd.do_not_decimate_on_mdv_request = gd.uparams->getLong("cidd.do_not_decimate_on_mdv_request", 0);
     
  gd.range_ring_follows_data = gd.uparams->getLong("cidd.range_ring_follows_data", 0);
  gd.range_ring_for_radar_only = gd.uparams->getLong("cidd.range_ring_for_radar_only", 0);
  
  gd.domain_follows_data = gd.uparams->getLong("cidd.domain_follows_data", 0);

  gd.help_command = gd.uparams->getString("cidd.help_command", "");
  gd.http_tunnel_url = gd.uparams->getString("cidd.http_tunnel_url", "");
  gd.datamap_host = gd.uparams->getString("cidd.datamap_host", "");
  gd.http_proxy_url = gd.uparams->getString("cidd.http_proxy_url", "");

  gd.bookmark_command = gd.uparams->getString("cidd.bookmark_command", "");
  gd.num_bookmarks = gd.uparams->getLong("cidd.num_bookmarks", 0);

  gd.origin_latitude = gd.uparams->getDouble("cidd.origin_latitude", 0.0);
  gd.origin_longitude = gd.uparams->getDouble("cidd.origin_longitude", 0.0);

  gd.reset_click_latitude = gd.uparams->getDouble("cidd.reset_click_latitude", gd.origin_latitude);
  gd.reset_click_longitude = gd.uparams->getDouble("cidd.reset_click_longitude", gd.origin_longitude);

  gd.latlon_mode = gd.uparams->getLong("cidd.latlon_mode",0);
  gd.north_angle = gd.uparams->getDouble("cidd.north_angle",0.0);
  gd.lambert_lat1 = gd.uparams->getDouble("cidd.lambert_lat1",20.0);
  gd.lambert_lat2 = gd.uparams->getDouble("cidd.lambert_lat2",60.0);
  gd.tangent_lat = gd.uparams->getDouble("cidd.tangent_lat",90.0);
  gd.tangent_lon = gd.uparams->getDouble("cidd.tangent_lon",0.0);
  gd.central_scale = gd.uparams->getDouble("cidd.central_scale",1.0);

  gd.aspect_ratio = gd.uparams->getDouble("cidd.aspect_ratio", 1.0);

  gd.scale_units_per_km = gd.uparams->getDouble("cidd.scale_units_per_km",1.0);
  gd.scale_units_label = gd.uparams->getString("cidd.scale_units_label", "km");
  
  gd.projection_type = gd.uparams->getString("cidd.projection_type", "CARTESIAN");

  gd.movie_on = gd.uparams->getLong("cidd.movie_on", 0);
  gd.movie_magnify_factor = gd.uparams->getDouble("cidd.movie_magnify_factor",1.0);
  gd.time_interval = gd.uparams->getDouble("cidd.time_interval",10.0);
  gd.frame_span = gd.uparams->getDouble("cidd.frame_span", gd.time_interval);
  gd.starting_movie_frames = gd.uparams->getLong("cidd.starting_movie_frames", 12);
  gd.reset_frames = gd.uparams->getLong("cidd.reset_frames", 0);
  gd.movie_delay = gd.uparams->getLong("cidd.movie_delay",3000);
  gd.forecast_interval = gd.uparams->getDouble("cidd.forecast_interval", 0.0);
  gd.past_interval = gd.uparams->getDouble("cidd.past_interval", 0.0);
  gd.time_search_stretch_factor = gd.uparams->getDouble("cidd.stretch_factor", 1.5);
  gd.climo_mode = gd.uparams->getString("cidd.climo_mode", "regular");
  gd.temporal_rounding = gd.uparams->getLong("cidd.temporal_rounding", 300);
  gd.movie_speed_msec = gd.uparams->getLong("cidd.movie_speed_msec", 75);

  gd.check_clipping = gd.uparams->getLong("cidd.check_clipping", 0);
  gd.max_time_list_span = gd.uparams->getLong("cidd.max_time_list_span", 365);
  gd.show_clock = gd.uparams->getLong("cidd.show_clock", 0);
  gd.show_data_messages = gd.uparams->getLong("cidd.show_data_messages", 1);
  gd.display_labels = gd.uparams->getLong("cidd.display_labels", 1);
  gd.display_ref_lines = gd.uparams->getLong("cidd.display_ref_lines", 1);
  gd.enable_status_window = gd.uparams->getLong("cidd.enable_status_window", 0);
  gd.enable_save_image_panel = gd.uparams->getLong("cidd.enable_save_image_panel", 0);
  gd.draw_clock_local = gd.uparams->getLong("cidd.draw_clock_local", 0);
  gd.use_local_timestamps = gd.uparams->getLong("cidd.use_local_timestamps", 0);
  gd.show_height_sel = gd.uparams->getLong("cidd.show_height_sel", 1);

  gd.use_cosine_correction = gd.uparams->getLong("cidd.use_cosine_correction", use_cosine);
  gd.demo_time = gd.uparams->getString("cidd.demo_time", "");
  gd.gather_data_mode = gd.uparams->getLong("cidd.gather_data_mode", CLOSEST_TO_FRAME_CENTER);
  
  gd.image_fill_threshold = gd.uparams->getLong("cidd.image_fill_threshold", 120000);

  gd.dynamic_contour_threshold = gd.uparams->getLong("cidd.dynamic_contour_threshold", 160000);

  gd.image_inten = gd.uparams->getDouble("cidd.image_inten", 0.8);
  gd.inten_levels = gd.uparams->getLong("cidd.inten_levels", 32);
  gd.data_inten = gd.uparams->getDouble("cidd.data_inten", 1.0);
  
  gd.data_timeout_secs = gd.uparams->getLong("cidd.data_timeout_secs", 10);
  
  gd.request_compressed_data = gd.uparams->getLong("cidd.request_compressed_data",0);
  gd.request_gzip_vol_compression = gd.uparams->getLong("cidd.request_gzip_vol_compression",0);

  gd.add_frame_num_to_filename = gd.uparams->getLong("cidd.add_frame_num_to_filename",1);
  gd.add_button_name_to_filename = gd.uparams->getLong("cidd.add_button_name_to_filename",0);
  gd.add_height_to_filename = gd.uparams->getLong("cidd.add_height_to_filename",0);
  gd.add_frame_time_to_filename = gd.uparams->getLong("cidd.add_frame_time_to_filename",1);
  gd.add_gen_time_to_filename = gd.uparams->getLong("cidd.add_gen_time_to_filename",0);
     
  gd.add_valid_time_to_filename = gd.uparams->getLong("cidd.add_valid_time_to_filename",0);
     
  gd.font_display_mode = gd.uparams->getLong("cidd.font_display_mode",1);

  gd.label_contours = gd.uparams->getLong("cidd.label_contours",1);

  gd.top_margin_render_style = gd.uparams->getLong("cidd.top_margin_render_style", 1);

  gd.bot_margin_render_style = gd.uparams->getLong("cidd.bot_margin_render_style", 1);

  gd.num_field_menu_cols = gd.uparams->getLong("cidd.num_field_menu_cols",0);

  gd.num_cache_zooms = gd.uparams->getLong("cidd.num_cache_zooms",1);
  
  gd.num_zoom_levels =  gd.uparams->getLong("cidd.num_zoom_levels",1);
  gd.start_zoom_level =  gd.uparams->getLong("cidd.start_zoom_level",1);
  
  gd.zoom_limits_in_latlon =  gd.uparams->getLong("cidd.zoom_limits_in_latlon",0);
  gd.domain_limit_min_x = gd.uparams->getDouble("cidd.domain_limit_min_x",-10000);
  gd.domain_limit_max_x = gd.uparams->getDouble("cidd.domain_limit_max_x",10000);
  gd.domain_limit_min_y = gd.uparams->getDouble("cidd.domain_limit_min_y",-10000);
  gd.domain_limit_max_y = gd.uparams->getDouble("cidd.domain_limit_max_y",10000);
  
  gd.min_ht = gd.uparams->getDouble("cidd.min_ht", 0.0);
  gd.max_ht = gd.uparams->getDouble("cidd.max_ht", 30.0);
  gd.start_ht = gd.uparams->getDouble("cidd.start_ht", 0.0);
  gd.planview_start_page = gd.uparams->getLong("cidd.planview_start_page", 1) -1;
  gd.xsect_start_page = gd.uparams->getLong("cidd.xsect_start_page", 1) -1;

  gd.ideal_x_vects = gd.uparams->getLong("cidd.ideal_x_vectors", 20);
  gd.ideal_y_vects = gd.uparams->getLong("cidd.ideal_y_vectors", 20);
  gd.wind_head_size = gd.uparams->getLong("cidd.wind_head_size", 5);
  gd.wind_head_angle = gd.uparams->getDouble("cidd.wind_head_angle", 45.0);
  gd.barb_shaft_len = gd.uparams->getLong("cidd.barb_shaft_len", 33);
  
  gd.all_winds_on = gd.uparams->getLong("cidd.all_winds_on", 1);
  gd.wind_mode = gd.uparams->getLong("cidd.wind_mode", 0);
  
  gd.wind_time_scale_interval = gd.uparams->getDouble("cidd.wind_time_scale_interval", 10.0);

  gd.wind_scaler = gd.uparams->getLong("cidd.wind_scaler", 3);

  gd.azmith_lines = gd.uparams->getLong("cidd.azmith_lines", 0);
  gd.azmith_radius = gd.uparams->getLong("cidd.azmith_lines", 0);
  gd.azmith_interval = gd.uparams->getLong("cidd.azmith_lines", 0);

  gd.wind_marker_type = gd.uparams->getString("cidd.wind_marker_type", "arrow");
  gd.wind_reference_speed = gd.uparams->getDouble("cidd.wind_reference_speed", 10.0);
  gd.wind_units_label = gd.uparams->getString("cidd.wind_units_label", "m/sec");
  gd.wind_w_scale_factor = gd.uparams->getDouble("cidd.wind_w_scale_factor", 10.0);
  gd.wind_units_scale_factor = gd.uparams->getDouble("cidd.wind_units_scale_factor", 1.0);

  gd.map_file_subdir =  gd.uparams->getString("cidd.map_file_subdir", "maps");
  gd.locator_margin_km = gd.uparams->getDouble("cidd.locator_margin_km", 50.0);
  gd.station_loc_url = gd.uparams->getString("cidd.station_loc_url", "");
  
  gd.remote_ui_url = gd.uparams->getString("cidd.remote_ui_url", "");

  gd.num_fonts = gd.uparams->getLong("cidd.num_fonts", 1);
  
  gd.contour_font_num = gd.uparams->getLong("contour_font_num", 6);
  gd.n_ideal_contour_labels = gd.uparams->getLong("n_ideal_contour_labels", 5);
  
  gd.rotate_coarse_adjust = gd.uparams->getDouble("cidd.rotate_coarse_adjust",6.0);
  gd.rotate_medium_adjust = gd.uparams->getDouble("cidd.rotate_medium_adjust",2.0);
  gd.rotate_fine_adjust = gd.uparams->getDouble("cidd.rotate_fine_adjust", 0.5);

  gd.min_zoom_threshold = gd.uparams->getDouble("cidd.min_zoom_threshold", 5.0);

  gd.no_data_message = gd.uparams->getString("cidd.no_data_message", "NO DATA FOUND (in this area at the selected time)");
  
  gd.horiz_top_margin =  gd.uparams->getLong("cidd.horiz_top_margin", 20);
  gd.horiz_bot_margin =  gd.uparams->getLong("cidd.horiz_bot_margin", 20);
  gd.horiz_left_margin = gd.uparams->getLong("cidd.horiz_left_margin", 20);
  gd.horiz_right_margin = gd.uparams->getLong("cidd.horiz_right_margin", 80);
  
  gd.vert_top_margin =  gd.uparams->getLong("cidd.vert_top_margin", 20);
  gd.vert_bot_margin =  gd.uparams->getLong("cidd.vert_bot_margin", 20);
  gd.vert_left_margin = gd.uparams->getLong("cidd.vert_left_margin", 20);
  gd.vert_right_margin = gd.uparams->getLong("cidd.vert_right_margin", 80);
  
  gd.horiz_legends_start_x = gd.uparams->getLong("cidd.horiz_legends_start_x", 0);
  gd.horiz_legends_start_y = gd.uparams->getLong("cidd.horiz_legends_start_y", 0);
  gd.horiz_legends_delta_y = gd.uparams->getLong("cidd.horiz_legends_delta_y", 0);
  
  gd.vert_legends_start_x = gd.uparams->getLong("cidd.vert_legends_start_x", 0);
  gd.vert_legends_start_y = gd.uparams->getLong("cidd.vert_legends_start_y", 0);
  gd.vert_legends_delta_y = gd.uparams->getLong("cidd.vert_legends_delta_y", 0);
  
  gd.horiz_min_height = gd.uparams->getLong("cidd.horiz_min_height", 400);
  gd.horiz_min_width = gd.uparams->getLong("cidd.horiz_min_width", 600);
  gd.horiz_default_height = gd.uparams->getLong("cidd.horiz_default_height", 600);
  gd.horiz_default_width = gd.uparams->getLong("cidd.horiz_default_width", 800);

  gd.vert_min_height = gd.uparams->getLong("cidd.vert_min_height", 400);
  gd.vert_min_width = gd.uparams->getLong("cidd.vert_min_width", 600);
  gd.vert_default_height = gd.uparams->getLong("cidd.vert_default_height", 400);
  gd.vert_default_width = gd.uparams->getLong("cidd.vert_default_width", 600);

  gd.wsddm_mode  = gd.uparams->getLong("cidd.wsddm_mode", 0);
  gd.one_click_rhi  = gd.uparams->getLong("cidd.one_click_rhi", 0);
  gd.click_posn_rel_to_origin  = gd.uparams->getLong("cidd.click_posn_rel_to_origin", 0);
  gd.report_clicks_in_status_window = gd.uparams->getLong("cidd.report_clicks_in_status_window", 0);
  gd.report_clicks_in_degM_and_nm = gd.uparams->getLong("cidd.report_clicks_in_degM_and_nm", 0);
  gd.magnetic_variation_deg = gd.uparams->getLong("cidd.magnetic_variation_deg", 0);
  gd.check_data_times = gd.uparams->getLong("cidd.check_data_times", 0);

  gd.frame_label = gd.uparams->getString("cidd.horiz_frame_label", "Qucid");
  gd.status_info_file = gd.uparams->getString("cidd.status_info_file", "");

  gd.horiz_default_y_pos = gd.uparams->getLong("cidd.horiz_default_y_pos",0);
  gd.horiz_default_x_pos = gd.uparams->getLong("cidd.horiz_default_x_pos",0);

  gd.vert_default_x_pos = gd.uparams->getLong("cidd.vert_default_x_pos", 0);
  gd.vert_default_y_pos = gd.uparams->getLong("cidd.vert_default_y_pos", 0);
  
  gd.ideal_x_vectors = gd.uparams->getLong("cidd.ideal_x_vectors", 20);
  gd.ideal_y_vectors = gd.uparams->getLong("cidd.ideal_y_vectors", 20);
  gd.azimuth_interval = gd.uparams->getDouble("cidd.azmith_interval", 30.0);
  gd.azimuth_interval = gd.uparams->getDouble("cidd.azimuth_interval", gd.azimuth_interval);
  gd.azimuth_radius = gd.uparams->getDouble("cidd.azmith_radius", 200.0);
  gd.azimuth_radius = gd.uparams->getDouble("cidd.azimuth_radius", gd.azimuth_radius);
  gd.latest_click_mark_size = gd.uparams->getLong("cidd.latest_click_mark_size", 11);
  gd.range_ring_x_space = gd.uparams->getLong("cidd.range_ring_x_space", 50);
  gd.range_ring_y_space = gd.uparams->getLong("cidd.range_ring_y_space", 15);
  gd.range_ring_spacing = gd.uparams->getDouble("cidd.range_ring_spacing", -1.0);
  gd.max_ring_range = gd.uparams->getDouble("cidd.max_ring_range", 1000.0);
  gd.range_ring_labels = gd.uparams->getLong("cidd.range_ring_labels", 1);

  gd.scale_constant = gd.uparams->getDouble("cidd.scale_constant", 300.0);

  gd.redraw_interval = gd.uparams->getLong("cidd.redraw_interval", REDRAW_INTERVAL);
  gd.update_interval = gd.uparams->getLong("cidd.update_interval", UPDATE_INTERVAL);

########################################################################
# Run - entry point

if __name__ == "__main__":
    main()
