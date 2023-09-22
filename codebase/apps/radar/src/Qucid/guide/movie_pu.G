;GIL-3
(
(
	:type                   :popup-window
	:name                   movie_pu
	:owner                  nil
	:width                  811
	:height                 152
	:background-color       ""
	:foreground-color       ""
	:label                  "MOVIE CONTROLS"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            nil
	:resizable              nil
	:pinned                 t
	:done-handler           nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   movie_cp
	:owner                  movie_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  811
	:height                 79
	:background-color       "Gray"
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   fr_begin_msg
	:owner                  movie_cp
	:help                   ""
	:x                      12
	:y                      4
	:width                  86
	:height                 13
	:foreground-color       ""
	:label                  "FRAME TIME:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   fcast_per_tx
	:owner                  movie_cp
	:help                   ""
	:x                      288
	:y                      4
	:width                  151
	:height                 15
	:value-x                403
	:value-y                4
	:value-length           3
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Forecast Period:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :invisible
	:read-only              nil
	:notify-handler         set_fcast_period_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (movie_pu fcast_per_tx)
		:when                   (Notify )
		:to                     (movie_pu fcast_per_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_fcast_period_proc)
		)
	)
)
(
	:type                   :message
	:name                   facst_time_usits_msg
	:owner                  movie_cp
	:help                   ""
	:x                      448
	:y                      4
	:width                  23
	:height                 13
	:foreground-color       ""
	:label                  "hrs"
	:label-type             :string
	:label-bold             t
	:initial-state          :invisible
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   start_time_tx
	:owner                  movie_cp
	:help                   ""
	:x                      524
	:y                      4
	:width                  278
	:height                 15
	:value-x                674
	:value-y                4
	:value-length           16
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Movie Loop Starts at:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         start_time_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (movie_pu start_time_tx)
		:when                   (Notify )
		:to                     (movie_pu start_time_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (start_time_proc)
		)
	)
)
(
	:type                   :setting
	:name                   start_st
	:owner                  movie_cp
	:help                   ""
	:x                      8
	:y                      24
	:width                  88
	:height                 23
	:value-x                8
	:value-y                24
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         movie_start_proc
	:event-handler          nil
	:choices                ("Stop" "Start" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (movie_pu start_st)
		:when                   (Notify )
		:to                     (movie_pu start_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (movie_start_proc)
		)
	)
)
(
	:type                   :setting
	:name                   movie_type_st
	:owner                  movie_cp
	:help                   ""
	:x                      126
	:y                      25
	:width                  138
	:height                 23
	:value-x                126
	:value-y                25
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         movie_type_proc
	:event-handler          nil
	:choices                ("Real Time" "Archive" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (movie_pu movie_type_st)
		:when                   (Notify )
		:to                     (movie_pu movie_type_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (movie_type_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   end_frame_tx
	:owner                  movie_cp
	:help                   ""
	:x                      304
	:y                      26
	:width                  202
	:height                 15
	:value-x                442
	:value-y                26
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Number of Frames:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          "1"
	:initial-state          :active
	:read-only              nil
	:notify-handler         end_frame_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (movie_pu end_frame_tx)
		:when                   (Notify )
		:to                     (movie_pu end_frame_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (end_frame_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   time_interval_tx
	:owner                  movie_cp
	:help                   ""
	:x                      524
	:y                      28
	:width                  236
	:height                 15
	:value-x                680
	:value-y                28
	:value-length           10
	:stored-length          20
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Movie Frame Interval:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         time_interv_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (movie_pu time_interval_tx)
		:when                   (Notify )
		:to                     (movie_pu time_interval_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (time_interv_proc)
		)
	)
)
(
	:type                   :message
	:name                   min_msg
	:owner                  movie_cp
	:help                   ""
	:x                      772
	:y                      30
	:width                  27
	:height                 13
	:foreground-color       ""
	:label                  "min"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   save_loop_bt
	:owner                  movie_cp
	:help                   ""
	:x                      312
	:y                      52
	:width                  94
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Output Loop"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         save_loop_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (movie_pu save_loop_bt)
		:when                   (Notify )
		:to                     (movie_pu save_loop_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (save_loop_proc)
		)
	)
)
(
	:type                   :setting
	:name                   sweep_st
	:owner                  movie_cp
	:help                   ""
	:x                      440
	:y                      52
	:width                  74
	:height                 23
	:value-x                440
	:value-y                52
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         set_sweep_proc
	:event-handler          nil
	:choices                ("Sweep" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (movie_pu sweep_st)
		:when                   (Notify )
		:to                     (movie_pu sweep_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_sweep_proc)
		)
	)
)
(
	:type                   :slider
	:name                   movie_frame_sl
	:owner                  movie_cp
	:help                   ""
	:x                      50
	:y                      56
	:width                  250
	:height                 20
	:value-x                98
	:value-y                56
	:slider-width           80
	:ticks                  0
	:foreground-color       ""
	:label                  "Frame:"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          t
	:show-range             t
	:show-value             t
	:min-value              0
	:max-value              128
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         movie_frame_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (movie_pu movie_frame_sl)
		:when                   (Notify )
		:to                     (movie_pu movie_frame_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (movie_frame_proc)
		)
	)
)
(
	:type                   :slider
	:name                   movie_speed_sl
	:owner                  movie_cp
	:help                   ""
	:x                      524
	:y                      56
	:width                  83
	:height                 20
	:value-x                524
	:value-y                56
	:slider-width           20
	:ticks                  0
	:foreground-color       ""
	:label                  ""
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             nil
	:min-value              0
	:max-value              30
	:min-value-string       "Speed"
	:max-value-string       " "
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         movie_speed_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (movie_pu movie_speed_sl)
		:when                   (Notify )
		:to                     (movie_pu movie_speed_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (movie_speed_proc)
		)
	)
)
(
	:type                   :slider
	:name                   movie_dwell_sl
	:owner                  movie_cp
	:help                   ""
	:x                      608
	:y                      56
	:width                  80
	:height                 20
	:value-x                608
	:value-y                56
	:slider-width           20
	:ticks                  0
	:foreground-color       ""
	:label                  ""
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             t
	:show-value             nil
	:min-value              0
	:max-value              10
	:min-value-string       "Delay"
	:max-value-string       " "
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         movie_delay_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (movie_pu movie_dwell_sl)
		:when                   (Notify )
		:to                     (movie_pu movie_dwell_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (movie_delay_proc)
		)
	)
)
(
	:type                   :message
	:name                   prod_title_msg
	:owner                  movie_cp
	:help                   ""
	:x                      712
	:y                      62
	:width                  59
	:height                 13
	:foreground-color       ""
	:label                  "Products"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   grid_title_msg
	:owner                  movie_cp
	:help                   ""
	:x                      2
	:y                      64
	:width                  36
	:height                 13
	:foreground-color       ""
	:label                  "Grids"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :canvas-pane
	:name                   time_can
	:owner                  movie_pu
	:help                   ""
	:x                      0
	:y                      79
	:width                  811
	:height                 73
	:background-color       "Black"
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       811
	:vertical-scrollbar     nil
	:scrollable-height      73
	:repaint-proc           time_canvas_repaint
	:event-handler          time_canvas_event_proc
	:drawing-model          :xwindows
	:user-data              ()
	:actions                (
		(
		:from                   (movie_pu time_can)
		:when                   (Repaint )
		:to                     (movie_pu time_can)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (time_canvas_repaint)
		)
		(
		:from                   (movie_pu time_can)
		:when                   (AnyEvent )
		:to                     (movie_pu time_can)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (time_canvas_event_proc)
		)
	)
)
)
