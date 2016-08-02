;GIL-3
(
(
	:type                   :base-window
	:name                   horiz_bw
	:owner                  nil
	:width                  554
	:height                 489
	:background-color       ""
	:foreground-color       ""
	:label                  "Cartesian Interactive Data Display (CIDD)"
	:label-type             :string
	:initial-state          :open
	:show-footer            nil
	:resizable              t
	:icon-file              "cidd.icon"
	:icon-label             ""
	:icon-mask-file         ""
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   cp
	:owner                  horiz_bw
	:help                   ""
	:x                      0
	:y                      0
	:width                  554
	:height                 52
	:background-color       "Gray"
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          h_pan_event_proc
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw cp)
		:when                   (AnyEvent )
		:to                     (horiz_bw cp)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (h_pan_event_proc)
		)
	)
)
(
	:type                   :setting
	:name                   movie_start_st
	:owner                  cp
	:help                   ""
	:x                      2
	:y                      2
	:width                  44
	:height                 23
	:value-x                2
	:value-y                2
	:rows                   1
	:columns                0
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         movie_start_proc
	:event-handler          nil
	:choices                ("Loop" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw movie_start_st)
		:when                   (Notify )
		:to                     (horiz_bw movie_start_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (movie_start_proc)
		)
	)
)
(
	:type                   :setting
	:name                   field_st
	:owner                  cp
	:help                   ""
	:x                      47
	:y                      2
	:width                  46
	:height                 23
	:value-x                47
	:value-y                2
	:rows                   0
	:columns                1
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         field_proc
	:event-handler          nil
	:choices                ("Field" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw field_st)
		:when                   (Notify )
		:to                     (horiz_bw field_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (field_proc)
		)
	)
)
(
	:type                   :setting
	:name                   zoom_st
	:owner                  cp
	:help                   ""
	:x                      94
	:y                      2
	:width                  46
	:height                 23
	:value-x                94
	:value-y                2
	:rows                   0
	:columns                1
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         zoom_proc
	:event-handler          nil
	:choices                ("View" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw zoom_st)
		:when                   (Notify )
		:to                     (horiz_bw zoom_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (zoom_proc)
		)
	)
)
(
	:type                   :setting
	:name                   movie_st
	:owner                  cp
	:help                   ""
	:x                      141
	:y                      2
	:width                  52
	:height                 23
	:value-x                141
	:value-y                2
	:rows                   1
	:columns                0
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         movie_ctl_proc
	:event-handler          nil
	:choices                ("Movie" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw movie_st)
		:when                   (Notify )
		:to                     (horiz_bw movie_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (movie_ctl_proc)
		)
	)
)
(
	:type                   :setting
	:name                   overlay_st
	:owner                  cp
	:help                   ""
	:x                      194
	:y                      2
	:width                  69
	:height                 23
	:value-x                194
	:value-y                2
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         overlay_ctl_proc
	:event-handler          nil
	:choices                ("Overlays" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw overlay_st)
		:when                   (Notify )
		:to                     (horiz_bw overlay_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (overlay_ctl_proc)
		)
	)
)
(
	:type                   :setting
	:name                   product_st
	:owner                  cp
	:help                   ""
	:x                      264
	:y                      2
	:width                  72
	:height                 23
	:value-x                264
	:value-y                2
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         product_ctl_proc
	:event-handler          nil
	:choices                ("Warnings" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw product_st)
		:when                   (Notify )
		:to                     (horiz_bw product_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (product_ctl_proc)
		)
	)
)
(
	:type                   :setting
	:name                   export_st
	:owner                  cp
	:help                   ""
	:x                      337
	:y                      2
	:width                  48
	:height                 23
	:value-x                337
	:value-y                2
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         export_ctl_proc
	:event-handler          nil
	:choices                ("Draw" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw export_st)
		:when                   (Notify )
		:to                     (horiz_bw export_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (export_ctl_proc)
		)
	)
)
(
	:type                   :setting
	:name                   x_sect_st
	:owner                  cp
	:help                   ""
	:x                      386
	:y                      2
	:width                  41
	:height                 23
	:value-x                386
	:value-y                2
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         x_sect_ctl_proc
	:event-handler          nil
	:choices                ("Vert" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw x_sect_st)
		:when                   (Notify )
		:to                     (horiz_bw x_sect_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (x_sect_ctl_proc)
		)
	)
)
(
	:type                   :setting
	:name                   field_sel_st
	:owner                  cp
	:help                   ""
	:x                      428
	:y                      2
	:width                  56
	:height                 23
	:value-x                428
	:value-y                2
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         field_sel_proc
	:event-handler          nil
	:choices                ("Config" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw field_sel_st)
		:when                   (Notify )
		:to                     (horiz_bw field_sel_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (field_sel_proc)
		)
	)
)
(
	:type                   :setting
	:name                   vector_st
	:owner                  cp
	:help                   ""
	:x                      484
	:y                      2
	:width                  60
	:height                 23
	:value-x                484
	:value-y                2
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         set_vectors_proc
	:event-handler          nil
	:choices                ("Vectors" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw vector_st)
		:when                   (Notify )
		:to                     (horiz_bw vector_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_vectors_proc)
		)
	)
)
(
	:type                   :slider
	:name                   movie_spd_sl
	:owner                  cp
	:help                   ""
	:x                      116
	:y                      28
	:width                  87
	:height                 20
	:value-x                116
	:value-y                28
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
	:max-value              100
	:min-value-string       "Speed"
	:max-value-string       "  "
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :invisible
	:notify-handler         movie_speed_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw movie_spd_sl)
		:when                   (Notify )
		:to                     (horiz_bw movie_spd_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (movie_speed_proc)
		)
	)
)
(
	:type                   :slider
	:name                   slice_sl
	:owner                  cp
	:help                   ""
	:x                      290
	:y                      28
	:width                  140
	:height                 20
	:value-x                290
	:value-y                28
	:slider-width           50
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
	:max-value              21
	:min-value-string       "Surface"
	:max-value-string       "Top"
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         sl_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw slice_sl)
		:when                   (Notify )
		:to                     (horiz_bw slice_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (sl_proc)
		)
	)
)
(
	:type                   :message
	:name                   movie_frame_msg
	:owner                  cp
	:help                   ""
	:x                      10
	:y                      32
	:width                  49
	:height                 13
	:foreground-color       ""
	:label                  "Frame: "
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   cur_ht_msg
	:owner                  cp
	:help                   ""
	:x                      204
	:y                      32
	:width                  77
	:height                 13
	:foreground-color       ""
	:label                  "Height Msg"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   cur_time_msg
	:owner                  cp
	:help                   ""
	:x                      464
	:y                      32
	:width                  35
	:height                 13
	:foreground-color       ""
	:label                  "Time"
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
	:owner                  horiz_bw
	:help                   ""
	:x                      0
	:y                      52
	:width                  554
	:height                 437
	:background-color       "Black"
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       554
	:vertical-scrollbar     nil
	:scrollable-height      437
	:repaint-proc           can_repaint
	:event-handler          can_event_proc
	:drawing-model          :xwindows
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw canvas1)
		:when                   (Repaint )
		:to                     (horiz_bw canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (can_repaint)
		)
		(
		:from                   (horiz_bw canvas1)
		:when                   (AnyEvent )
		:to                     (horiz_bw canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (can_event_proc)
		)
	)
)
)
