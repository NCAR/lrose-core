;GIL-3
(
(
	:type                   :menu
	:name                   field_mu
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           field_mu_gen_proc
	:menu-title             "Active Fields"
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
		:from                   (field_mu)
		:when                   (Notify )
		:to                     (field_mu)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (field_mu_gen_proc)
		)
	)
)
(
	:type                   :menu
	:name                   wind_mu
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           wind_menu_gen_proc
	:menu-title             "Wind Set"
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
		:from                   (wind_mu)
		:when                   (Notify )
		:to                     (wind_mu)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (wind_menu_gen_proc)
		)
	)
)
(
	:type                   :menu
	:name                   prod_mu
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           prod_mu_gen_proc
	:menu-title             "Symbolic Product"
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
		:from                   (prod_mu)
		:when                   (Notify )
		:to                     (prod_mu)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_mu_gen_proc)
		)
	)
)
(
	:type                   :popup-window
	:name                   fields_pu
	:owner                  nil
	:width                  705
	:height                 356
	:background-color       ""
	:foreground-color       ""
	:label                  "Data Source Configuration"
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
	:name                   fields_controls
	:owner                  fields_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  705
	:height                 356
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
	:type                   :button
	:name                   f_config_sel_bt
	:owner                  fields_controls
	:help                   ""
	:x                      62
	:y                      8
	:width                  32
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  ""
	:label-type             :string
	:initial-state          :active
	:menu                   field_mu
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   f_config_msg1
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      10
	:width                  36
	:height                 13
	:foreground-color       ""
	:label                  "GRID:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   f_config_msg2
	:owner                  fields_controls
	:help                   ""
	:x                      118
	:y                      10
	:width                  41
	:height                 13
	:foreground-color       ""
	:label                  "Label:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :group
	:name                   group5
	:x                      8
	:y                      8
	:group-type             nil
	:members                (f_config_sel_bt f_config_msg1 f_config_msg2 )
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
(
	:type                   :text-field
	:name                   f_config_tx1
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      29
	:width                  678
	:height                 15
	:value-x                46
	:value-y                29
	:value-length           80
	:stored-length          256
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "URL:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_host_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu f_config_tx1)
		:when                   (Notify )
		:to                     (fields_pu f_config_tx1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_host_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   f_config_tx4
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      46
	:width                  152
	:height                 15
	:value-x                88
	:value-y                46
	:value-length           9
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Time Slop:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_stretch_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu f_config_tx4)
		:when                   (Notify )
		:to                     (fields_pu f_config_tx4)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_stretch_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   f_config_tx9
	:owner                  fields_controls
	:help                   ""
	:x                      200
	:y                      46
	:width                  150
	:height                 15
	:value-x                294
	:value-y                46
	:value-length           7
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Time Offset:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_time_offset_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu f_config_tx9)
		:when                   (Notify )
		:to                     (fields_pu f_config_tx9)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_time_offset_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   f_config_tx3
	:owner                  fields_controls
	:help                   ""
	:x                      400
	:y                      46
	:width                  193
	:height                 15
	:value-x                513
	:value-y                46
	:value-length           10
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Altitude Offset:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_alt_offset
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu f_config_tx3)
		:when                   (Notify )
		:to                     (fields_pu f_config_tx3)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_alt_offset)
		)
	)
)
(
	:type                   :message
	:name                   f_config_msg6
	:owner                  fields_controls
	:help                   ""
	:x                      162
	:y                      47
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
	:type                   :message
	:name                   f_config_msg7
	:owner                  fields_controls
	:help                   ""
	:x                      352
	:y                      47
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
	:type                   :group
	:name                   group3
	:x                      8
	:y                      46
	:group-type             nil
	:members                (f_config_tx4 f_config_tx9 f_config_tx3 f_config_msg6 f_config_msg7 )
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
(
	:type                   :button
	:name                   f_config_color_apply_bt
	:owner                  fields_controls
	:help                   ""
	:x                      468
	:y                      63
	:width                  130
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Change Color Map"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         set_cscale_apply_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu f_config_color_apply_bt)
		:when                   (Notify )
		:to                     (fields_pu f_config_color_apply_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_cscale_apply_proc)
		)
	)
)
(
	:type                   :message
	:name                   f_config_msg8
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      67
	:width                  44
	:height                 13
	:foreground-color       ""
	:label                  "Colors"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   f_config_tx7
	:owner                  fields_controls
	:help                   ""
	:x                      60
	:y                      67
	:width                  184
	:height                 15
	:value-x                140
	:value-y                67
	:value-length           13
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Min Value:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_cscale_min_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu f_config_tx7)
		:when                   (Notify )
		:to                     (fields_pu f_config_tx7)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_cscale_min_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   f_config_tx8
	:owner                  fields_controls
	:help                   ""
	:x                      262
	:y                      67
	:width                  175
	:height                 15
	:value-x                309
	:value-y                67
	:value-length           16
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Delta:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_cscale_max_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu f_config_tx8)
		:when                   (Notify )
		:to                     (fields_pu f_config_tx8)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_cscale_max_proc)
		)
	)
)
(
	:type                   :group
	:name                   group4
	:x                      8
	:y                      63
	:group-type             nil
	:members                (f_config_color_apply_bt f_config_msg8 f_config_tx7 f_config_tx8 )
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
(
	:type                   :setting
	:name                   update_st
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      84
	:width                  110
	:height                 23
	:value-x                8
	:value-y                84
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         set_update_proc
	:event-handler          nil
	:choices                ("Auto Update" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu update_st)
		:when                   (Notify )
		:to                     (fields_pu update_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_update_proc)
		)
	)
)
(
	:type                   :setting
	:name                   max_st
	:owner                  fields_controls
	:help                   ""
	:x                      120
	:y                      84
	:width                  151
	:height                 23
	:value-x                120
	:value-y                84
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         set_composite_proc
	:event-handler          nil
	:choices                ("Request Composite" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu max_st)
		:when                   (Notify )
		:to                     (fields_pu max_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_composite_proc)
		)
	)
)
(
	:type                   :setting
	:name                   autoscale_st
	:owner                  fields_controls
	:help                   ""
	:x                      273
	:y                      84
	:width                  92
	:height                 23
	:value-x                273
	:value-y                84
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         autoscale_proc
	:event-handler          nil
	:choices                ("Autoscale" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu autoscale_st)
		:when                   (Notify )
		:to                     (fields_pu autoscale_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (autoscale_proc)
		)
	)
)
(
	:type                   :message
	:name                   f_config_msg3
	:owner                  fields_controls
	:help                   ""
	:x                      367
	:y                      89
	:width                  79
	:height                 13
	:foreground-color       ""
	:label                  "Color scale:"
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
	:x                      8
	:y                      84
	:group-type             nil
	:members                (update_st max_st autoscale_st f_config_msg3 )
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
(
	:type                   :setting
	:name                   ren_meth_st
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      109
	:width                  424
	:height                 23
	:value-x                88
	:value-y                109
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Render As:"
	:label-type             :string
	:notify-handler         set_ren_meth_proc
	:event-handler          nil
	:choices                ("Polygons" "Contours" "Auto Contours" "Line Contours" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:initial-selections     (nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu ren_meth_st)
		:when                   (Notify )
		:to                     (fields_pu ren_meth_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_ren_meth_proc)
		)
	)
)
(
	:type                   :message
	:name                   f_config_msg4
	:owner                  fields_controls
	:help                   ""
	:x                      456
	:y                      113
	:width                  40
	:height                 13
	:foreground-color       ""
	:label                  "Units:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :group
	:name                   group1
	:x                      8
	:y                      109
	:group-type             nil
	:members                (ren_meth_st f_config_msg4 )
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
(
	:type                   :group
	:name                   group6
	:x                      8
	:y                      8
	:group-type             nil
	:members                (group5 f_config_tx1 group3 group4 group2 group1 )
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
(
	:type                   :scrolling-list
	:name                   display_list
	:owner                  fields_controls
	:help                   ""
	:x                      516
	:y                      84
	:width                  150
	:height                 55
	:value-x                516
	:value-y                101
	:rows                   1
	:foreground-color       ""
	:label                  "Display Fields In Menu"
	:title                  ""
	:label-type             :string
	:layout-type            :vertical
	:read-only              nil
	:multiple-selections    t
	:selection-required     nil
	:initial-state          :invisible
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:notify-handler         field_display_proc
	:event-handler          nil
	:initial-list-values    ()
	:initial-list-glyphs    ()
	:initial-selections     ()
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu display_list)
		:when                   (Notify )
		:to                     (fields_pu display_list)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (field_display_proc)
		)
	)
)
(
	:type                   :message
	:name                   separator1
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      134
	:width                  678
	:height                 13
	:foreground-color       "Dark Green"
	:label                  "*****************************************************************************************************************"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   w_config_sel_bt
	:owner                  fields_controls
	:help                   ""
	:x                      56
	:y                      149
	:width                  32
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Dark Green"
	:label                  ""
	:label-type             :string
	:initial-state          :active
	:menu                   wind_mu
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   w_config_msg1
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      152
	:width                  37
	:height                 13
	:foreground-color       "Dark Green"
	:label                  "WIND"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   w_config_msg2
	:owner                  fields_controls
	:help                   ""
	:x                      104
	:y                      152
	:width                  41
	:height                 13
	:foreground-color       "Dark Green"
	:label                  "Label:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   w_config_color_msg
	:owner                  fields_controls
	:help                   ""
	:x                      556
	:y                      152
	:width                  36
	:height                 13
	:foreground-color       "Dark Green"
	:label                  "Color"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu w_config_color_msg)
		:when                   (Select )
		:to                     (fields_pu w_config_color_msg)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (wind_color_proc)
		)
	)
)
(
	:type                   :group
	:name                   group7
	:x                      8
	:y                      149
	:group-type             nil
	:members                (w_config_sel_bt w_config_msg1 w_config_msg2 w_config_color_msg )
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
(
	:type                   :text-field
	:name                   w_config_url_tx
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      170
	:width                  678
	:height                 15
	:value-x                46
	:value-y                170
	:value-length           80
	:stored-length          256
	:rows                   3
	:foreground-color       "Dark Green"
	:text-type              :alphanumeric
	:label                  "URL:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         wind_url_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu w_config_url_tx)
		:when                   (Notify )
		:to                     (fields_pu w_config_url_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (wind_url_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   w_config_u_tx
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      187
	:width                  136
	:height                 15
	:value-x                72
	:value-y                187
	:value-length           9
	:stored-length          80
	:rows                   3
	:foreground-color       "Dark Green"
	:text-type              :alphanumeric
	:label                  "U Name:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         u_name_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu w_config_u_tx)
		:when                   (Notify )
		:to                     (fields_pu w_config_u_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (u_name_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   w_config_v_tx
	:owner                  fields_controls
	:help                   ""
	:x                      160
	:y                      187
	:width                  135
	:height                 15
	:value-x                223
	:value-y                187
	:value-length           9
	:stored-length          80
	:rows                   3
	:foreground-color       "Dark Green"
	:text-type              :alphanumeric
	:label                  "V Name:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         v_name_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu w_config_v_tx)
		:when                   (Notify )
		:to                     (fields_pu w_config_v_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (v_name_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   w_config_w_tx
	:owner                  fields_controls
	:help                   ""
	:x                      306
	:y                      187
	:width                  130
	:height                 15
	:value-x                372
	:value-y                187
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       "Dark Green"
	:text-type              :alphanumeric
	:label                  "W Name:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         w_name_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu w_config_w_tx)
		:when                   (Notify )
		:to                     (fields_pu w_config_w_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (w_name_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   w_config_line_width_tx
	:owner                  fields_controls
	:help                   ""
	:x                      460
	:y                      187
	:width                  155
	:height                 15
	:value-x                544
	:value-y                187
	:value-length           2
	:stored-length          80
	:rows                   3
	:foreground-color       "Dark Green"
	:text-type              :numeric
	:label                  "Line Width:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              10
	:min-value              1
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         w_line_width_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu w_config_line_width_tx)
		:when                   (Notify )
		:to                     (fields_pu w_config_line_width_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (w_line_width_proc)
		)
	)
)
(
	:type                   :group
	:name                   group8
	:x                      8
	:y                      187
	:group-type             nil
	:members                (w_config_u_tx w_config_v_tx w_config_w_tx w_config_line_width_tx )
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
(
	:type                   :setting
	:name                   w_style_tx
	:owner                  fields_controls
	:help                   ""
	:x                      421
	:y                      204
	:width                  141
	:height                 23
	:value-x                466
	:value-y                204
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       "Dark Green"
	:setting-type           :stack
	:selection-required     t
	:label                  "Style:"
	:label-type             :string
	:notify-handler         wind_style_proc
	:event-handler          nil
	:choices                ("Arrows" "Tufts" "Barbs" "Vectors" "T Vectors" "L Barbs" "Met Barbs" "SH Barbs" "LSH Barbs" )
	:choice-label-types     (:string :string :string :string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" "" "" "" )
	:choice-defaults        (nil nil nil nil nil nil nil nil nil )
	:initial-selections     (nil nil nil nil nil nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu w_style_tx)
		:when                   (Notify )
		:to                     (fields_pu w_style_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (wind_style_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   textfield2
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      208
	:width                  120
	:height                 15
	:value-x                88
	:value-y                208
	:value-length           5
	:stored-length          80
	:rows                   3
	:foreground-color       "Dark Green"
	:text-type              :alphanumeric
	:label                  "Time Slop:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         wind_time_slop_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu textfield2)
		:when                   (Notify )
		:to                     (fields_pu textfield2)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (wind_time_slop_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   textfield1
	:owner                  fields_controls
	:help                   ""
	:x                      130
	:y                      208
	:width                  134
	:height                 15
	:value-x                224
	:value-y                208
	:value-length           5
	:stored-length          80
	:rows                   3
	:foreground-color       "Dark Green"
	:text-type              :alphanumeric
	:label                  "Time Offset:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         wind_time_offset_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu textfield1)
		:when                   (Notify )
		:to                     (fields_pu textfield1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (wind_time_offset_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   wind_alt_tx
	:owner                  fields_controls
	:help                   ""
	:x                      266
	:y                      208
	:width                  153
	:height                 15
	:value-x                379
	:value-y                208
	:value-length           5
	:stored-length          80
	:rows                   3
	:foreground-color       "Dark Green"
	:text-type              :alphanumeric
	:label                  "Altitude Offset:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         wind_alt_offset_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu wind_alt_tx)
		:when                   (Notify )
		:to                     (fields_pu wind_alt_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (wind_alt_offset_proc)
		)
	)
)
(
	:type                   :group
	:name                   group9
	:x                      8
	:y                      204
	:group-type             nil
	:members                (w_style_tx textfield2 textfield1 wind_alt_tx )
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
(
	:type                   :group
	:name                   group10
	:x                      8
	:y                      134
	:group-type             nil
	:members                (separator1 group7 w_config_url_tx group8 group9 )
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
(
	:type                   :message
	:name                   separator2
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      229
	:width                  672
	:height                 13
	:foreground-color       "Navy"
	:label                  "****************************************************************************************************************"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   render_type_st
	:owner                  fields_controls
	:help                   ""
	:x                      366
	:y                      241
	:width                  310
	:height                 23
	:value-x                457
	:value-y                241
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       "navy"
	:setting-type           :stack
	:selection-required     t
	:label                  "Constraints:"
	:label-type             :string
	:notify-handler         prod_render_type_proc
	:event-handler          nil
	:choices                ("Render All" "Render All Valid" "Render Valid in Last Frame" "Render Latest in Frame" "Render Latest in Loop" "Render First Before Data Time" "Render First After Data Time" "Render All Before Data Time" "Render All After Data Time" )
	:choice-label-types     (:string :string :string :string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" "" "" "" )
	:choice-defaults        (nil nil nil nil nil nil nil nil nil )
	:initial-selections     (nil nil nil nil nil nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu render_type_st)
		:when                   (Notify )
		:to                     (fields_pu render_type_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_render_type_proc)
		)
	)
)
(
	:type                   :button
	:name                   p_config_sel_bt
	:owner                  fields_controls
	:help                   ""
	:x                      84
	:y                      242
	:width                  32
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "navy"
	:label                  ""
	:label-type             :string
	:initial-state          :active
	:menu                   prod_mu
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   prod_config_msg1
	:owner                  fields_controls
	:help                   ""
	:x                      12
	:y                      245
	:width                  60
	:height                 13
	:foreground-color       "navy"
	:label                  "Symprod"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   prod_config_msg2
	:owner                  fields_controls
	:help                   ""
	:x                      130
	:y                      245
	:width                  41
	:height                 13
	:foreground-color       "navy"
	:label                  "Label:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :group
	:name                   group11
	:x                      12
	:y                      242
	:group-type             nil
	:members                (p_config_sel_bt prod_config_msg1 prod_config_msg2 )
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
(
	:type                   :group
	:name                   group13
	:x                      12
	:y                      241
	:group-type             nil
	:members                (render_type_st group11 )
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
(
	:type                   :text-field
	:name                   p_config_url_tx
	:owner                  fields_controls
	:help                   ""
	:x                      12
	:y                      265
	:width                  662
	:height                 15
	:value-x                50
	:value-y                265
	:value-length           78
	:stored-length          256
	:rows                   3
	:foreground-color       "navy"
	:text-type              :alphanumeric
	:label                  "URL:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         prod_url_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu p_config_url_tx)
		:when                   (Notify )
		:to                     (fields_pu p_config_url_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_url_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   p_config_type_tx
	:owner                  fields_controls
	:help                   ""
	:x                      12
	:y                      282
	:width                  160
	:height                 15
	:value-x                92
	:value-y                282
	:value-length           10
	:stored-length          80
	:rows                   3
	:foreground-color       "navy"
	:text-type              :alphanumeric
	:label                  "Data Type:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         prod_data_type_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu p_config_type_tx)
		:when                   (Notify )
		:to                     (fields_pu p_config_type_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_data_type_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   allow_before_tx
	:owner                  fields_controls
	:help                   ""
	:x                      210
	:y                      282
	:width                  138
	:height                 15
	:value-x                308
	:value-y                282
	:value-length           5
	:stored-length          80
	:rows                   3
	:foreground-color       "navy"
	:text-type              :alphanumeric
	:label                  "Allow Before:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         allow_before_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu allow_before_tx)
		:when                   (Notify )
		:to                     (fields_pu allow_before_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (allow_before_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   allow_after_tx
	:owner                  fields_controls
	:help                   ""
	:x                      350
	:y                      282
	:width                  85
	:height                 15
	:value-x                395
	:value-y                282
	:value-length           5
	:stored-length          80
	:rows                   3
	:foreground-color       "navy"
	:text-type              :alphanumeric
	:label                  "After:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         allow_after_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu allow_after_tx)
		:when                   (Notify )
		:to                     (fields_pu allow_after_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (allow_after_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   text_thresh_tx
	:owner                  fields_controls
	:help                   ""
	:x                      496
	:y                      282
	:width                  159
	:height                 15
	:value-x                607
	:value-y                282
	:value-length           4
	:stored-length          80
	:rows                   3
	:foreground-color       "navy"
	:text-type              :alphanumeric
	:label                  "Text Threshold"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         test_thresh_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu text_thresh_tx)
		:when                   (Notify )
		:to                     (fields_pu text_thresh_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (test_thresh_proc)
		)
	)
)
(
	:type                   :message
	:name                   p_config_msg3
	:owner                  fields_controls
	:help                   ""
	:x                      437
	:y                      283
	:width                  27
	:height                 13
	:foreground-color       "navy"
	:label                  "min"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :group
	:name                   group12
	:x                      12
	:y                      282
	:group-type             nil
	:members                (p_config_type_tx allow_before_tx allow_after_tx text_thresh_tx p_config_msg3 )
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
(
	:type                   :group
	:name                   group15
	:x                      8
	:y                      229
	:group-type             nil
	:members                (separator2 group13 p_config_url_tx group12 )
	:rows                   1
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
(
	:type                   :message
	:name                   separator3
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      299
	:width                  672
	:height                 13
	:foreground-color       "Firebrick"
	:label                  "****************************************************************************************************************"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   topo_url_tx
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      314
	:width                  681
	:height                 15
	:value-x                81
	:value-y                314
	:value-length           76
	:stored-length          80
	:rows                   3
	:foreground-color       "Firebrick"
	:text-type              :alphanumeric
	:label                  "Topo URL"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         topo_url_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu topo_url_tx)
		:when                   (Notify )
		:to                     (fields_pu topo_url_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (topo_url_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   land_use_url_tx
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      331
	:width                  678
	:height                 15
	:value-x                110
	:value-y                331
	:value-length           72
	:stored-length          80
	:rows                   3
	:foreground-color       "Firebrick"
	:text-type              :alphanumeric
	:label                  "Land Use URL"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         land_use_url_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu land_use_url_tx)
		:when                   (Notify )
		:to                     (fields_pu land_use_url_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (land_use_url_proc)
		)
	)
)
(
	:type                   :group
	:name                   group14
	:x                      8
	:y                      299
	:group-type             nil
	:members                (separator3 topo_url_tx land_use_url_tx )
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
