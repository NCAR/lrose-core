########################################################################
# parameter file for rctable_generate
#
# Summer scan sequence, 220 grids in x and y, each 1.25 km square
#
# Mike Dixon RAP NCAR Boulder CO Jan 1991
#########################################################################

rctable_generate.debug: false
rctable_generate.malloc_debug_level: 0

#
# target file names
#

rctable_generate.rc_table_file: $(RTEST_HOME)/tables/rc_table.polar
rctable_generate.slave_table_file: $(RTEST_HOME)/tables/slave_table.polar

#
# mode - type of volume to create
#
# Options are:
#  cart -  Cartesian volume grid       (km x km x km) (default)
#  ppi  -  Cartesian ppi grid          (deg x km x km)
#  polar - polar grid (el x az x gate) (deg x deg x km)
#

rctable_generate.mode: polar

#
# option to create the slave table - not needed if you are
# not going to run cart_slave to feed realtime data to
# the dobson_server
#

rctable_generate.create_slave_table: true

#
# scan strategy
#

rctable_generate.nelevations: 11
rctable_generate.elev_target: 0.5 1.2 2.5 4.0 5.5 7.0 8.5 10.0 13.0 17.0 22.0
rctable_generate.nazimuths: 360
rctable_generate.delta_azimuth: 1.0
rctable_generate.start_azimuth: 0.0

#
# use azimuth table
#

rctable_generate.use_azimuth_table: false
rctable_generate.azimuth_table_file: tass_scan_table

#
# beam params
#

rctable_generate.beam_width: 0.95
rctable_generate.ngates: 660
rctable_generate.gate_spacing: 0.225
rctable_generate.start_range: 0.1125

#
# radar location
#

rctable_generate.radar_latitude: 39.87823
rctable_generate.radar_longitude: -104.75900
rctable_generate.radarz: 1.604

#
# grid origin location
#

rctable_generate.cart_latitude: 39.87823
rctable_generate.cart_longitude: -104.75900
rctable_generate.cart_rotation: 0.0

#
# grid size
#

rctable_generate.nx: 240
rctable_generate.ny: 240
rctable_generate.nz: 20

rctable_generate.minx: -119.5
rctable_generate.miny: -119.5
rctable_generate.minz: 2.0

rctable_generate.dx: 1.0
rctable_generate.dy: 1.0
rctable_generate.dz: 1.0

#
# if extend_below is true, data from the bottom beam is used
# to fill the space below the beam.
#

rctable_generate.extend_below: false


