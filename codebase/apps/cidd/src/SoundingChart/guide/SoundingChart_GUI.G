;GIL-3
(
(
	:type                   :base-window
	:name                   window1
	:owner                  nil
	:width                  661
	:height                 597
	:background-color       ""
	:foreground-color       ""
	:label                  "Base Window"
	:label-type             :string
	:initial-state          :open
	:show-footer            nil
	:resizable              t
	:icon-file              ""
	:icon-label             ""
	:icon-mask-file         ""
	:event-handler          window_event_func
	:user-data              ()
	:actions                (
		(
		:from                   (window1)
		:when                   (AnyEvent )
		:to                     (window1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (window_event_func)
		)
	)
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  window1
	:help                   ""
	:x                      0
	:y                      0
	:width                  661
	:height                 32
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :slider
	:name                   width_sl
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      8
	:width                  249
	:height                 20
	:value-x                120
	:value-y                8
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  "Sounding Width"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          t
	:show-range             nil
	:show-value             nil
	:min-value              0
	:max-value              100
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         adust_width_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 width_sl)
		:when                   (Notify )
		:to                     (window1 width_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (adust_width_proc)
		)
	)
)
(
	:type                   :message
	:name                   time_msg
	:owner                  controls1
	:help                   ""
	:x                      550
	:y                      8
	:width                  79
	:height                 13
	:foreground-color       ""
	:label                  "TIME String"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :canvas-pane
	:name                   canvas1
	:owner                  window1
	:help                   ""
	:x                      0
	:y                      32
	:width                  661
	:height                 565
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       661
	:vertical-scrollbar     nil
	:scrollable-height      565
	:repaint-proc           canvas_repaint
	:event-handler          canvas_event_func
	:drawing-model          :xview
	:user-data              ()
	:actions                (
		(
		:from                   (window1 canvas1)
		:when                   (Repaint )
		:to                     (window1 canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (canvas_repaint)
		)
		(
		:from                   (window1 canvas1)
		:when                   (AnyEvent )
		:to                     (window1 canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (canvas_event_func)
		)
	)
)
(
	:type                   :popup-window
	:name                   config_pu
	:owner                  window1
	:width                  314
	:height                 231
	:background-color       ""
	:foreground-color       ""
	:label                  "Sounding Chart Configuration"
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
	:name                   controls2
	:owner                  config_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  314
	:height                 231
	:background-color       ""
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
	:name                   data_msg
	:owner                  controls2
	:help                   ""
	:x                      98
	:y                      8
	:width                  95
	:height                 13
	:foreground-color       ""
	:label                  "Data Message"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   grid_wind_st
	:owner                  controls2
	:help                   ""
	:x                      10
	:y                      40
	:width                  292
	:height                 23
	:value-x                79
	:value-y                40
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  "Model     "
	:label-type             :string
	:notify-handler         show_grid_wind
	:event-handler          nil
	:choices                ("Colors" "Plot" "Winds" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:initial-selections     (nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu grid_wind_st)
		:when                   (Notify )
		:to                     (config_pu grid_wind_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (show_grid_wind)
		)
	)
)
(
	:type                   :setting
	:name                   Grid_st
	:owner                  controls2
	:help                   ""
	:x                      84
	:y                      68
	:width                  116
	:height                 23
	:value-x                133
	:value-y                68
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "FIELD:"
	:label-type             :string
	:notify-handler         grid_select
	:event-handler          nil
	:choices                ("None" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:choice-defaults        (nil )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu Grid_st)
		:when                   (Notify )
		:to                     (config_pu Grid_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (grid_select)
		)
	)
)
(
	:type                   :setting
	:name                   sound_wind_st
	:owner                  controls2
	:help                   ""
	:x                      8
	:y                      96
	:width                  295
	:height                 23
	:value-x                80
	:value-y                96
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  "Sounding"
	:label-type             :string
	:notify-handler         show_sound_winds
	:event-handler          nil
	:choices                ("Colors" "Plot" "Winds" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:initial-selections     (nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu sound_wind_st)
		:when                   (Notify )
		:to                     (config_pu sound_wind_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (show_sound_winds)
		)
	)
)
(
	:type                   :setting
	:name                   sound_st
	:owner                  controls2
	:help                   ""
	:x                      80
	:y                      128
	:width                  116
	:height                 23
	:value-x                129
	:value-y                128
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "FIELD:"
	:label-type             :string
	:notify-handler         sounding_select
	:event-handler          nil
	:choices                ("None" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:choice-defaults        (nil )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu sound_st)
		:when                   (Notify )
		:to                     (config_pu sound_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (sounding_select)
		)
	)
)
(
	:type                   :setting
	:name                   refs_st
	:owner                  controls2
	:help                   ""
	:x                      8
	:y                      172
	:width                  194
	:height                 23
	:value-x                184
	:value-y                172
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  "Output Reference Points"
	:label-type             :string
	:notify-handler         set_ref_points_proc
	:event-handler          nil
	:choices                ("" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu refs_st)
		:when                   (Notify )
		:to                     (config_pu refs_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_ref_points_proc)
		)
	)
)
(
	:type                   :button
	:name                   clear_bt
	:owner                  controls2
	:help                   ""
	:x                      8
	:y                      204
	:width                  153
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Clear reference points"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         clear_refs_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu clear_bt)
		:when                   (Notify )
		:to                     (config_pu clear_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (clear_refs_proc)
		)
	)
)
)
