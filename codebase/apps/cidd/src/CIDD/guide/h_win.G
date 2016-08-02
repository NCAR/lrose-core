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
	:label                  "Configurable Interactive Data Display (CIDD)"
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
	:name                   main_st
	:owner                  cp
	:help                   ""
	:x                      2
	:y                      4
	:width                  44
	:height                 23
	:value-x                2
	:value-y                4
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     nil
	:label                  ""
	:label-type             :string
	:notify-handler         main_st_proc
	:event-handler          nil
	:choices                ("Loop" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw main_st)
		:when                   (Notify )
		:to                     (horiz_bw main_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (main_st_proc)
		)
	)
)
(
	:type                   :setting
	:name                   im_cache_st
	:owner                  cp
	:help                   ""
	:x                      470
	:y                      26
	:width                  80
	:height                 23
	:value-x                470
	:value-y                26
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       "Black"
	:setting-type           :exclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         im_cache_proc
	:event-handler          nil
	:choices                (" " " " " " " " )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:initial-selections     (nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (horiz_bw im_cache_st)
		:when                   (Notify )
		:to                     (horiz_bw im_cache_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (im_cache_proc)
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
	:x                      130
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
	:x                      380
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
