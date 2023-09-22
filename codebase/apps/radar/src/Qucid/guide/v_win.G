;GIL-3
(
(
	:type                   :menu
	:name                   v_field_mu
	:help                   ""
	:columns                1
	:menu-type              :exclusive
	:menu-handler           v_field_mu_gen_proc
	:menu-title             "Data Field"
	:menu-item-labels       ()
	:menu-item-label-types  ()
	:menu-item-states       ()
	:menu-item-defaults     ()
	:initial-selections     ()
	:menu-item-handlers     ()
	:menu-item-menus        ()
	:menu-item-colors       ()
	:pinnable               nil
	:user-data              ()
	:actions                (
		(
		:from                   (v_field_mu)
		:when                   (Notify )
		:to                     (v_field_mu)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (v_field_mu_gen_proc)
		)
	)
)
(
	:type                   :popup-window
	:name                   v_win_pu
	:owner                  nil
	:width                  797
	:height                 417
	:background-color       ""
	:foreground-color       ""
	:label                  "Route Cross Section"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            nil
	:resizable              t
	:pinned                 nil
	:done-handler           nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  v_win_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  797
	:height                 48
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
	:type                   :setting
	:name                   route_st
	:owner                  controls1
	:help                   ""
	:x                      22
	:y                      4
	:width                  66
	:height                 23
	:value-x                22
	:value-y                4
	:rows                   1
	:columns                0
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         show_route_pu_proc
	:event-handler          nil
	:choices                ("Routes..." )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (v_win_pu route_st)
		:when                   (Notify )
		:to                     (v_win_pu route_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (show_route_pu_proc)
		)
	)
)
(
	:type                   :setting
	:name                   route_labels_st
	:owner                  controls1
	:help                   ""
	:x                      104
	:y                      4
	:width                  251
	:height                 23
	:value-x                104
	:value-y                4
	:rows                   1
	:columns                0
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         route_labels_proc
	:event-handler          nil
	:choices                ("Label Waypoints" "Show Winds Text" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (v_win_pu route_labels_st)
		:when                   (Notify )
		:to                     (v_win_pu route_labels_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (route_labels_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   ht_top_tx
	:owner                  controls1
	:help                   ""
	:x                      512
	:y                      4
	:width                  116
	:height                 15
	:value-x                580
	:value-y                4
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Scale Top:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         scale_top_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (v_win_pu ht_top_tx)
		:when                   (Notify )
		:to                     (v_win_pu ht_top_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (scale_top_proc)
		)
	)
)
(
	:type                   :button
	:name                   v_unzoom_bt
	:owner                  controls1
	:help                   ""
	:x                      668
	:y                      12
	:width                  54
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Unzoom"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         v_unzoom_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (v_win_pu v_unzoom_bt)
		:when                   (Notify )
		:to                     (v_win_pu v_unzoom_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (v_unzoom_proc)
		)
	)
)
(
	:type                   :button
	:name                   dismiss_bt
	:owner                  controls1
	:help                   ""
	:x                      730
	:y                      12
	:width                  60
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Dismiss"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         v_panel_dismiss
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (v_win_pu dismiss_bt)
		:when                   (Notify )
		:to                     (v_win_pu dismiss_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (v_panel_dismiss)
		)
	)
)
(
	:type                   :text-field
	:name                   scale_base_tx
	:owner                  controls1
	:help                   ""
	:x                      512
	:y                      24
	:width                  122
	:height                 15
	:value-x                586
	:value-y                24
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Scale Base:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         scale_base_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (v_win_pu scale_base_tx)
		:when                   (Notify )
		:to                     (v_win_pu scale_base_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (scale_base_proc)
		)
	)
)
(
	:type                   :message
	:name                   route_msg
	:owner                  controls1
	:help                   ""
	:x                      4
	:y                      30
	:width                  84
	:height                 13
	:foreground-color       ""
	:label                  "Average Winds:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   hazard_msg
	:owner                  controls1
	:help                   ""
	:x                      94
	:y                      30
	:width                  96
	:height                 13
	:foreground-color       ""
	:label                  "Hazard Warnings:"
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
	:owner                  v_win_pu
	:help                   ""
	:x                      0
	:y                      48
	:width                  797
	:height                 369
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   v_field_mu
	:horizontal-scrollbar   nil
	:scrollable-width       797
	:vertical-scrollbar     nil
	:scrollable-height      369
	:repaint-proc           v_can_repaint
	:event-handler          v_can_events
	:drawing-model          :xwindows
	:user-data              ()
	:actions                (
		(
		:from                   (v_win_pu canvas1)
		:when                   (Repaint )
		:to                     (v_win_pu canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (v_can_repaint)
		)
		(
		:from                   (v_win_pu canvas1)
		:when                   (AnyEvent )
		:to                     (v_win_pu canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (v_can_events)
		)
	)
)
)
