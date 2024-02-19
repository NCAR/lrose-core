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

    replaceDict["gd.syprod_P->debug"] = "_params.symprod_debug"
    replaceDict["gd.syprod_P->short_requests"] = "_params.symprod_short_requests"
    replaceDict["gd.syprod_P->gzip_requests"] = "_params.symprod_gzip_requests"
    replaceDict["gd.syprod_P->prod_info"] = "_params.symprod_prod_info"

########################################################################
# set up replace dictionary

def createReplaceDict_v0():

    global replaceDict
    replaceDict = {}

    replaceDict["gd.idle_reset_seconds"] = "_params.idle_reset_seconds"
    
    replaceDict["gd.model_run_list_hours"] = "_params.model_run_list_hours"
    replaceDict["gd.model_run_list_hours"] = "_params.model_run_list_hours"
    replaceDict["gd.close_popups"] = "_params.close_popups"
    replaceDict["gd.disable_pick_mode"] = "_params.disable_pick_mode"
    replaceDict["gd.clip_overlay_fields"] = "_params.clip_overlay_fields"
    replaceDict["gd.output_geo_xml"] = "_params.output_geo_xml"
    replaceDict["gd.use_latlon_in_geo_xml"] = "_params.use_latlon_in_geo_xml"
    replaceDict["gd.replace_underscores"] = "_params.replace_underscores"
    replaceDict["gd.image_dir"] = "_params.image_dir"

    replaceDict["gd.html_mode"] = "_params.html_mode"
    replaceDict["gd.run_once_and_exit"] = "_params.run_once_and_exit"
  
    replaceDict["gd.transparent_images"] = "_params.transparent_images"
  
    replaceDict["gd.save_images_to_day_subdir"] = "_params.save_images_to_day_subdir"
  
    replaceDict["gd.horiz_image_fname"] = "_params.horiz_image_fname"
    replaceDict["gd.horiz_image_command"] = "_params.horiz_image_command"
  
    replaceDict["gd.vert_image_fname"] = "_params.vert_image_fname"
    replaceDict["gd.vert_image_command"] = "_params.vert_image_command"
  
    replaceDict["gd.image_ext"] = "_params.image_ext"
  
    replaceDict["gd.image_horiz_prefix"] = "_params.image_horiz_prefix"
    replaceDict["gd.image_vert_prefix"] = "_params.image_vert_prefix"
    replaceDict["gd.image_name_separator"] = "_params.image_name_separator"
  
    replaceDict["gd.image_convert_script"] = "_params.image_convert_script"
  
    replaceDict["gd.series_convert_script"] = "_params.series_convert_script"

    replaceDict["gd.complex_command_timeout_secs"] = "_params.complex_command_timeout_secs"
    replaceDict["gd.print_script"] = "_params.print_script"
    replaceDict["gd.simple_command_timeout_secs"] = "_params.simple_command_timeout_secs"

    replaceDict["gd.label_time_format"] = "_params.label_time_format"

    replaceDict["gd.moviestart_time_format"] = "_params.moviestart_time_format"

    replaceDict["gd.frame_range_time_format"] = "_params.frame_range_time_format"

    replaceDict["gd.movieframe_time_format"] = "_params.movieframe_time_format"

    replaceDict["gd.movieframe_time_mode"] = "_params.movieframe_time_mode"

    replaceDict["gd.layer_legends_on"] = "_params.layer_legends_on"
    replaceDict["gd.cont_legends_on"] = "_params.cont_legends_on"
    replaceDict["gd.wind_legends_on"] = "_params.wind_legends_on"
    replaceDict["gd.contour_line_width"] = "_params.contour_line_width"
    replaceDict["gd.smooth_contours"] = "_params.smooth_contours"
    replaceDict["gd.use_alt_contours"] = "_params.use_alt_contours"
    replaceDict["gd.add_noise"] = "_params.add_noise"
    replaceDict["gd.special_contour_value"] = "_params.special_contour_value"
    replaceDict["gd.map_bad_to_min_value"] = "_params.map_bad_to_min_value"
    replaceDict["gd.map_missing_to_min_value"] = "_params.map_missing_to_min_value"

    replaceDict["gd.draw_main_on_top"] = "_params.draw_main_on_top"

    replaceDict["gd.mark_latest_click_location"] = "_params.mark_latest_click_location"

    # replaceDict["gd.drawing_mode"] = "_params.drawing_mode"

    replaceDict["gd.products_on"] = "_params.products_on"
    replaceDict["gd.product_line_width"] = "_params.product_line_width"
    replaceDict["gd.product_font_size"] = "_params.product_font_size"
  
    replaceDict["gd.always_get_full_domain"] = "_params.always_get_full_domain"
    replaceDict["gd.do_not_clip_on_mdv_request"] = "_params.do_not_clip_on_mdv_request"
    replaceDict["gd.do_not_decimate_on_mdv_request"] = "_params.do_not_decimate_on_mdv_request"
     
    replaceDict["gd.range_ring_follows_data"] = "_params.range_ring_follows_data"
    replaceDict["gd.range_ring_for_radar_only"] = "_params.range_ring_for_radar_only"
  
    replaceDict["gd.domain_follows_data"] = "_params.domain_follows_data"

    replaceDict["gd.help_command"] = "_params.help_command"
    replaceDict["gd.http_tunnel_url"] = "_params.http_tunnel_url"
    replaceDict["gd.datamap_host"] = "_params.datamap_host"
    replaceDict["gd.http_proxy_url"] = "_params.http_proxy_url"

    replaceDict["gd.bookmark_command"] = "_params.bookmark_command"
    replaceDict["gd.num_bookmarks"] = "_params.num_bookmarks"

    replaceDict["gd.origin_latitude"] = "_params.origin_latitude"
    replaceDict["gd.origin_longitude"] = "_params.origin_longitude"

    replaceDict["gd.reset_click_latitude"] = "_params.reset_click_latitude"
    replaceDict["gd.reset_click_longitude"] = "_params.reset_click_longitude"

    replaceDict["gd.latlon_mode"] = "_params.latlon_mode"
    replaceDict["gd.north_angle"] = "_params.north_angle"
    replaceDict["gd.lambert_lat1"] = "_params.lambert_lat1"
    replaceDict["gd.lambert_lat2"] = "_params.lambert_lat2"
    replaceDict["gd.tangent_lat"] = "_params.tangent_lat"
    replaceDict["gd.tangent_lon"] = "_params.tangent_lon"
    replaceDict["gd.central_scale"] = "_params.central_scale"

    replaceDict["gd.aspect_ratio"] = "_params.aspect_ratio"

    replaceDict["gd.scale_units_per_km"] = "_params.scale_units_per_km"
    replaceDict["gd.scale_units_label"] = "_params.scale_units_label"
  
    replaceDict["gd.projection_type"] = "_params.projection_type"

    replaceDict["gd.movie_on"] = "_params.movie_on"
    replaceDict["gd.movie_magnify_factor"] = "_params.movie_magnify_factor"
    replaceDict["gd.time_interval"] = "_params.time_interval"
    replaceDict["gd.frame_span"] = "_params.frame_span"
    replaceDict["gd.starting_movie_frames"] = "_params.starting_movie_frames"
    replaceDict["gd.reset_frames"] = "_params.reset_frames"
    replaceDict["gd.movie_delay"] = "_params.movie_delay"
    replaceDict["gd.forecast_interval"] = "_params.forecast_interval"
    replaceDict["gd.past_interval"] = "_params.past_interval"
    replaceDict["gd.time_search_stretch_factor"] = "_params.time_search_stretch_factor"
    replaceDict["gd.climo_mode"] = "_params.climo_mode"
    replaceDict["gd.temporal_rounding"] = "_params.temporal_rounding"
    replaceDict["gd.movie_speed_msec"] = "_params.movie_speed_msec"

    replaceDict["gd.check_clipping"] = "_params.check_clipping"
    replaceDict["gd.max_time_list_span"] = "_params.max_time_list_span"
    replaceDict["gd.show_clock"] = "_params.show_clock"
    replaceDict["gd.show_data_messages"] = "_params.show_data_messages"
    replaceDict["gd.display_labels"] = "_params.display_labels"
    replaceDict["gd.display_ref_lines"] = "_params.display_ref_lines"
    replaceDict["gd.enable_status_window"] = "_params.enable_status_window"
    replaceDict["gd.enable_save_image_panel"] = "_params.enable_save_image_panel"
    replaceDict["gd.draw_clock_local"] = "_params.draw_clock_local"
    replaceDict["gd.use_local_timestamps"] = "_params.use_local_timestamps"
    replaceDict["gd.show_height_sel"] = "_params.show_height_sel"

    replaceDict["gd.use_cosine_correction"] = "_params.use_cosine_correction"
    replaceDict["gd.demo_time"] = "_params.demo_time"
    replaceDict["gd.gather_data_mode"] = "_params.gather_data_mode"
  
    replaceDict["gd.image_fill_threshold"] = "_params.image_fill_threshold"

    replaceDict["gd.dynamic_contour_threshold"] = "_params.dynamic_contour_threshold"

    replaceDict["gd.image_inten"] = "_params.image_inten"
    replaceDict["gd.inten_levels"] = "_params.inten_levels"
    replaceDict["gd.data_inten"] = "_params.data_inten"
  
    replaceDict["gd.data_timeout_secs"] = "_params.data_timeout_secs"
  
    replaceDict["gd.request_compressed_data"] = "_params.request_compressed_data"
    replaceDict["gd.request_gzip_vol_compression"] = "_params.request_gzip_vol_compression"

    replaceDict["gd.add_frame_num_to_filename"] = "_params.add_frame_num_to_filename"
    replaceDict["gd.add_button_name_to_filename"] = "_params.add_button_name_to_filename"
    replaceDict["gd.add_height_to_filename"] = "_params.add_height_to_filename"
    replaceDict["gd.add_frame_time_to_filename"] = "_params.add_frame_time_to_filename"
    replaceDict["gd.add_gen_time_to_filename"] = "_params.add_gen_time_to_filename"
     
    replaceDict["gd.add_valid_time_to_filename"] = "_params.add_valid_time_to_filename"
     
    replaceDict["gd.font_display_mode"] = "_params.font_display_mode"

    replaceDict["gd.label_contours"] = "_params.label_contours"

    replaceDict["gd.top_margin_render_style"] = "_params.top_margin_render_style"

    replaceDict["gd.bot_margin_render_style"] = "_params.bot_margin_render_style"

    replaceDict["gd.num_field_menu_cols"] = "_params.num_field_menu_cols"

    replaceDict["gd.num_cache_zooms"] = "_params.num_cache_zooms"
  
    replaceDict["gd.num_zoom_levels"] = "_params.num_zoom_levels"
    replaceDict["gd.start_zoom_level"] = "_params.start_zoom_level"
  
    replaceDict["gd.zoom_limits_in_latlon"] = "_params.zoom_limits_in_latlon"
    replaceDict["gd.domain_limit_min_x"] = "_params.domain_limit_min_x"
    replaceDict["gd.domain_limit_max_x"] = "_params.domain_limit_max_x"
    replaceDict["gd.domain_limit_min_y"] = "_params.domain_limit_min_y"
    replaceDict["gd.domain_limit_max_y"] = "_params.domain_limit_max_y"
  
    replaceDict["gd.min_ht"] = "_params.min_ht"
    replaceDict["gd.max_ht"] = "_params.max_ht"
    replaceDict["gd.start_ht"] = "_params.start_ht"
    replaceDict["gd.planview_start_page"] = "_params.planview_start_page"
    replaceDict["gd.xsect_start_page"] = "_params.xsect_start_page"

    replaceDict["gd.ideal_x_vects"] = "_params.ideal_x_vects"
    replaceDict["gd.ideal_y_vects"] = "_params.ideal_y_vects"
    replaceDict["gd.wind_head_size"] = "_params.wind_head_size"
    replaceDict["gd.wind_head_angle"] = "_params.wind_head_angle"
    replaceDict["gd.barb_shaft_len"] = "_params.barb_shaft_len"
  
    replaceDict["gd.all_winds_on"] = "_params.all_winds_on"
    replaceDict["gd.wind_mode"] = "_params.wind_mode"
  
    replaceDict["gd.wind_time_scale_interval"] = "_params.wind_time_scale_interval"

    replaceDict["gd.wind_scaler"] = "_params.wind_scaler"

    replaceDict["gd.azmith_lines"] = "_params.azmith_lines"
    replaceDict["gd.azmith_radius"] = "_params.azmith_radius"
    replaceDict["gd.azmith_interval"] = "_params.azmith_interval"

    replaceDict["gd.wind_marker_type"] = "_params.wind_marker_type"
    replaceDict["gd.wind_reference_speed"] = "_params.wind_reference_speed"
    replaceDict["gd.wind_units_label"] = "_params.wind_units_label"
    replaceDict["gd.wind_w_scale_factor"] = "_params.wind_w_scale_factor"
    replaceDict["gd.wind_units_scale_factor"] = "_params.wind_units_scale_factor"

    replaceDict["gd.map_file_subdir"] = "_params.map_file_subdir"
    replaceDict["gd.locator_margin_km"] = "_params.locator_margin_km"
    replaceDict["gd.station_loc_url"] = "_params.station_loc_url"
  
    replaceDict["gd.remote_ui_url"] = "_params.remote_ui_url"

    replaceDict["gd.num_fonts"] = "_params.num_fonts"
  
    replaceDict["gd.contour_font_num"] = "_params.contour_font_num"
    replaceDict["gd.n_ideal_contour_labels"] = "_params.n_ideal_contour_labels"
  
    replaceDict["gd.rotate_coarse_adjust"] = "_params.rotate_coarse_adjust"
    replaceDict["gd.rotate_medium_adjust"] = "_params.rotate_medium_adjust"
    replaceDict["gd.rotate_fine_adjust"] = "_params.rotate_fine_adjust"

    replaceDict["gd.min_zoom_threshold"] = "_params.min_zoom_threshold"

    replaceDict["gd.no_data_message"] = "_params.no_data_message"
  
    replaceDict["gd.horiz_top_margin"] = "_params.horiz_top_margin"
    replaceDict["gd.horiz_bot_margin"] = "_params.horiz_bot_margin"
    replaceDict["gd.horiz_left_margin"] = "_params.horiz_left_margin"
    replaceDict["gd.horiz_right_margin"] = "_params.horiz_right_margin"
  
    replaceDict["gd.vert_top_margin"] = "_params.vert_top_margin"
    replaceDict["gd.vert_bot_margin"] = "_params.vert_bot_margin"
    replaceDict["gd.vert_left_margin"] = "_params.vert_left_margin"
    replaceDict["gd.vert_right_margin"] = "_params.vert_right_margin"
  
    replaceDict["gd.horiz_legends_start_x"] = "_params.horiz_legends_start_x"
    replaceDict["gd.horiz_legends_start_y"] = "_params.horiz_legends_start_y"
    replaceDict["gd.horiz_legends_delta_y"] = "_params.horiz_legends_delta_y"
  
    replaceDict["gd.vert_legends_start_x"] = "_params.vert_legends_start_x"
    replaceDict["gd.vert_legends_start_y"] = "_params.vert_legends_start_y"
    replaceDict["gd.vert_legends_delta_y"] = "_params.vert_legends_delta_y"
  
    replaceDict["gd.horiz_min_height"] = "_params.horiz_min_height"
    replaceDict["gd.horiz_min_width"] = "_params.horiz_min_width"
    replaceDict["gd.horiz_default_height"] = "_params.horiz_default_height"
    replaceDict["gd.horiz_default_width"] = "_params.horiz_default_width"

    replaceDict["gd.vert_min_height"] = "_params.vert_min_height"
    replaceDict["gd.vert_min_width"] = "_params.vert_min_width"
    replaceDict["gd.vert_default_height"] = "_params.vert_default_height"
    replaceDict["gd.vert_default_width"] = "_params.vert_default_width"

    replaceDict["gd.wsddm_mode "] = "_params.wsddm_mode "
    replaceDict["gd.one_click_rhi "] = "_params.one_click_rhi "
    replaceDict["gd.click_posn_rel_to_origin "] = "_params.click_posn_rel_to_origin "
    replaceDict["gd.report_clicks_in_status_window"] = "_params.report_clicks_in_status_window"
    replaceDict["gd.report_clicks_in_degM_and_nm"] = "_params.report_clicks_in_degM_and_nm"
    replaceDict["gd.magnetic_variation_deg"] = "_params.magnetic_variation_deg"
    replaceDict["gd.check_data_times"] = "_params.check_data_times"

    replaceDict["gd.frame_label"] = "_params.frame_label"
    replaceDict["gd.status_info_file"] = "_params.status_info_file"

    replaceDict["gd.horiz_default_y_pos"] = "_params.horiz_default_y_pos"
    replaceDict["gd.horiz_default_x_pos"] = "_params.horiz_default_x_pos"

    replaceDict["gd.vert_default_x_pos"] = "_params.vert_default_x_pos"
    replaceDict["gd.vert_default_y_pos"] = "_params.vert_default_y_pos"
  
    replaceDict["gd.ideal_x_vectors"] = "_params.ideal_x_vectors"
    replaceDict["gd.ideal_y_vectors"] = "_params.ideal_y_vectors"
    replaceDict["gd.azmith_lines"] = "_params.azimuth_lines"
    replaceDict["gd.azmith_interval"] = "_params.azimuth_interval"
    replaceDict["gd.azmith_radius"] = "_params.azimuth_radius"
    replaceDict["gd.latest_click_mark_size"] = "_params.latest_click_mark_size"
    replaceDict["gd.range_ring_x_space"] = "_params.range_ring_x_space"
    replaceDict["gd.range_ring_y_space"] = "_params.range_ring_y_space"
    replaceDict["gd.range_ring_spacing"] = "_params.range_ring_spacing"
    replaceDict["gd.max_ring_range"] = "_params.max_ring_range"
    replaceDict["gd.range_ring_labels"] = "_params.range_ring_labels"

    replaceDict["gd.scale_constant"] = "_params.scale_constant"

    replaceDict["gd.redraw_interval"] = "_params.redraw_interval"
    replaceDict["gd.update_interval"] = "_params.update_interval"

########################################################################
# Run - entry point

if __name__ == "__main__":
    main()
