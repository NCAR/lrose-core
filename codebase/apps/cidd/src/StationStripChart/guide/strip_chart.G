;GIL-3
(
(
	:type                   :menu
	:name                   menu1
	:help                   ""
	:columns                1
	:menu-type              :exclusive
	:menu-handler           select_field_proc
	:menu-title             "Show"
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
		:from                   (menu1)
		:when                   (Notify )
		:to                     (menu1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (select_field_proc)
		)
	)
)
(
	:type                   :base-window
	:name                   win1
	:owner                  nil
	:width                  266
	:height                 234
	:background-color       ""
	:foreground-color       ""
	:label                  "Station Trends"
	:label-type             :string
	:initial-state          :open
	:show-footer            nil
	:resizable              t
	:icon-file              "gauge_strip.icon"
	:icon-label             ""
	:icon-mask-file         ""
	:event-handler          win1_event_proc
	:user-data              ()
	:actions                (
		(
		:from                   (win1)
		:when                   (AnyEvent )
		:to                     (win1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (win1_event_proc)
		)
	)
)
(
	:type                   :canvas-pane
	:name                   canvas1
	:owner                  win1
	:help                   ""
	:x                      0
	:y                      0
	:width                  266
	:height                 234
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   menu1
	:horizontal-scrollbar   nil
	:scrollable-width       266
	:vertical-scrollbar     nil
	:scrollable-height      234
	:repaint-proc           nil
	:event-handler          nil
	:drawing-model          :xwindows
	:user-data              ()
	:actions                ()
)
(
	:type                   :popup-window
	:name                   config_pu
	:owner                  win1
	:width                  584
	:height                 253
	:background-color       ""
	:foreground-color       ""
	:label                  "Configuration"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            nil
	:resizable              nil
	:pinned                 nil
	:done-handler           nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  config_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  584
	:height                 253
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
	:name                   run_mode_st
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      6
	:width                  204
	:height                 23
	:value-x                63
	:value-y                6
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Modes:"
	:label-type             :string
	:notify-handler         run_mode_proc
	:event-handler          nil
	:choices                ("Archive" "Live" "Slave" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("" "" "" )
	:initial-selections     (nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu run_mode_st)
		:when                   (Notify )
		:to                     (config_pu run_mode_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (run_mode_proc)
		)
	)
)
(
	:type                   :setting
	:name                   units_st
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      6
	:width                  114
	:height                 23
	:value-x                232
	:value-y                6
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         set_units_proc
	:event-handler          nil
	:choices                ("Metric" "English" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu units_st)
		:when                   (Notify )
		:to                     (config_pu units_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_units_proc)
		)
	)
)
(
	:type                   :setting
	:name                   zone_st
	:owner                  controls1
	:help                   ""
	:x                      370
	:y                      6
	:width                  123
	:height                 23
	:value-x                370
	:value-y                6
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         zone_proc
	:event-handler          nil
	:choices                ("UTC" "Local Time" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu zone_st)
		:when                   (Notify )
		:to                     (config_pu zone_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (zone_proc)
		)
	)
)
(
	:type                   :button
	:name                   config_dismiss_bt
	:owner                  controls1
	:help                   ""
	:x                      514
	:y                      8
	:width                  57
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Yellow"
	:label                  "CLOSE"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         dismiss_config_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu config_dismiss_bt)
		:when                   (Notify )
		:to                     (config_pu config_dismiss_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (dismiss_config_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   archive_time_tx
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      41
	:width                  550
	:height                 15
	:value-x                270
	:value-y                41
	:value-length           36
	:stored-length          128
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Plot Ends at: (yyyy mm dd hh mm ss):"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          "2003 10 31 23 00 00 UTC"
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_arch_time_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu archive_time_tx)
		:when                   (Notify )
		:to                     (config_pu archive_time_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_arch_time_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   url_tx
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      64
	:width                  553
	:height                 15
	:value-x                65
	:value-y                64
	:value-length           62
	:stored-length          1024
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Source:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          "./"
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_url_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu url_tx)
		:when                   (Notify )
		:to                     (config_pu url_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_url_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   update_interval_tx
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      90
	:width                  162
	:height                 15
	:value-x                106
	:value-y                90
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Update Every"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         update_interval_tx
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu update_interval_tx)
		:when                   (Notify )
		:to                     (config_pu update_interval_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (update_interval_tx)
		)
	)
)
(
	:type                   :text-field
	:name                   delay_tx
	:owner                  controls1
	:help                   ""
	:x                      242
	:y                      90
	:width                  64
	:height                 15
	:value-x                242
	:value-y                90
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  ""
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         delay_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu delay_tx)
		:when                   (Notify )
		:to                     (config_pu delay_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (delay_proc)
		)
	)
)
(
	:type                   :message
	:name                   message2
	:owner                  controls1
	:help                   ""
	:x                      312
	:y                      91
	:width                  116
	:height                 13
	:foreground-color       ""
	:label                  "Seconds after :00"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   message1
	:owner                  controls1
	:help                   ""
	:x                      178
	:y                      92
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
	:name                   trace_width_tx
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      112
	:width                  187
	:height                 15
	:value-x                131
	:value-y                112
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Trace Line Width"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         trace_width_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu trace_width_tx)
		:when                   (Notify )
		:to                     (config_pu trace_width_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (trace_width_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   scale_tx
	:owner                  controls1
	:help                   ""
	:x                      312
	:y                      114
	:width                  196
	:height                 15
	:value-x                444
	:value-y                114
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Seconds per pixel:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         scale_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu scale_tx)
		:when                   (Notify )
		:to                     (config_pu scale_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (scale_proc)
		)
	)
)
(
	:type                   :setting
	:name                   field_st
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      136
	:width                  561
	:height                 110
	:value-x                12
	:value-y                136
	:rows                   0
	:columns                4
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         active_field_st
	:event-handler          nil
	:choices                ("Precip Rate" "Precip Accum" "Ceiling" "Visibility" "Flight Category" "Temperature" "Humidity" "Wind Speed" "Wind direction" "Barometer" "Precip Alternate" "Spare 1" "Spare 2" "Freezing Precip" "Dew Pt" "Wx Type" )
	:choice-label-types     (:string :string :string :string :string :string :string :string :string :string :string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" )
	:initial-selections     (nil nil nil nil nil nil nil nil nil nil nil nil nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu field_st)
		:when                   (Notify )
		:to                     (config_pu field_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (active_field_st)
		)
	)
)
)
