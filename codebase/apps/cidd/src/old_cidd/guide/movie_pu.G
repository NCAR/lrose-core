;GIL-3
(
(
	:type                   :popup-window
	:name                   movie_pu
	:owner                  nil
	:width                  286
	:height                 182
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
	:width                  286
	:height                 182
	:background-color       "Green Yellow"
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   start_time_tx
	:owner                  movie_cp
	:help                   ""
	:x                      1
	:y                      4
	:width                  278
	:height                 15
	:value-x                151
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
	:type                   :text-field
	:name                   time_interval_tx
	:owner                  movie_cp
	:help                   ""
	:x                      1
	:y                      22
	:width                  251
	:height                 15
	:value-x                196
	:value-y                22
	:value-length           7
	:stored-length          20
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Movie Frame Time Interval:"
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
	:x                      254
	:y                      24
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
	:type                   :text-field
	:name                   fcast_per_tx
	:owner                  movie_cp
	:help                   ""
	:x                      1
	:y                      40
	:width                  247
	:height                 15
	:value-x                192
	:value-y                40
	:value-length           7
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Realtime-Look into Future:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
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
	:x                      254
	:y                      42
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
	:type                   :text-field
	:name                   end_frame_tx
	:owner                  movie_cp
	:help                   ""
	:x                      1
	:y                      58
	:width                  206
	:height                 15
	:value-x                136
	:value-y                58
	:value-length           3
	:stored-length          32
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Number of Frames"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              128
	:min-value              1
	:initial-value          0
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
	:name                   st_frame_tx
	:owner                  movie_cp
	:help                   ""
	:x                      194
	:y                      60
	:width                  86
	:height                 15
	:value-x                209
	:value-y                60
	:value-length           1
	:stored-length          32
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "F"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              128
	:min-value              1
	:initial-value          0
	:initial-state          :invisible
	:read-only              nil
	:notify-handler         start_frame_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (movie_pu st_frame_tx)
		:when                   (Notify )
		:to                     (movie_pu st_frame_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (start_frame_proc)
		)
	)
)
(
	:type                   :message
	:name                   msg1
	:owner                  movie_cp
	:help                   ""
	:x                      2
	:y                      76
	:width                  276
	:height                 13
	:foreground-color       ""
	:label                  "**********************************************"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   movie_type_st
	:owner                  movie_cp
	:help                   ""
	:x                      8
	:y                      88
	:width                  138
	:height                 23
	:value-x                8
	:value-y                88
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
	:type                   :setting
	:name                   start_st
	:owner                  movie_cp
	:help                   ""
	:x                      176
	:y                      88
	:width                  88
	:height                 23
	:value-x                176
	:value-y                88
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
	:type                   :slider
	:name                   movie_dwell_sl
	:owner                  movie_cp
	:help                   ""
	:x                      8
	:y                      115
	:width                  117
	:height                 20
	:value-x                8
	:value-y                115
	:slider-width           30
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
	:min-value-string       "End Delay"
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
	:type                   :slider
	:name                   movie_speed_sl
	:owner                  movie_cp
	:help                   ""
	:x                      152
	:y                      115
	:width                  116
	:height                 20
	:value-x                152
	:value-y                115
	:slider-width           40
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
	:min-value-string       "Slow"
	:max-value-string       "Fast"
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
	:name                   movie_frame_sl
	:owner                  movie_cp
	:help                   ""
	:x                      8
	:y                      139
	:width                  267
	:height                 20
	:value-x                53
	:value-y                139
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Frame"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          t
	:show-range             t
	:show-value             t
	:min-value              0
	:max-value              100
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
	:type                   :message
	:name                   fr_begin_msg
	:owner                  movie_cp
	:help                   ""
	:x                      8
	:y                      163
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
	:type                   :group
	:name                   group2
	:x                      3
	:y                      88
	:group-type             nil
	:members                (movie_type_st start_st movie_dwell_sl movie_speed_sl movie_frame_sl fr_begin_msg )
	:rows                   0
	:row-alignment          :top-edges
	:columns                0
	:column-alignment       :left-edges
	:horizontal-spacing     10
	:vertical-spacing       10
	:anchor-object          nil
	:anchor-point           :north-west
	:reference-point        :north-west
	:horizontal-offset      0
	:vertical-offset        0
	:user-data              ()
	:actions                ()
)
)
