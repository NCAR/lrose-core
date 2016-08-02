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
	:type                   :popup-window
	:name                   fields_pu
	:owner                  nil
	:width                  444
	:height                 377
	:background-color       ""
	:foreground-color       ""
	:label                  "Data Field Configuration"
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
	:width                  444
	:height                 377
	:background-color       "Gold"
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :scrolling-list
	:name                   display_list
	:owner                  fields_controls
	:help                   ""
	:x                      8
	:y                      8
	:width                  200
	:height                 361
	:value-x                8
	:value-y                25
	:rows                   18
	:foreground-color       ""
	:label                  "Displayable Fields"
	:title                  ""
	:label-type             :string
	:layout-type            :vertical
	:read-only              nil
	:multiple-selections    t
	:selection-required     nil
	:initial-state          :active
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
	:type                   :button
	:name                   f_config_sel_bt
	:owner                  fields_controls
	:help                   ""
	:x                      360
	:y                      14
	:width                  65
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Field:"
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
	:x                      238
	:y                      16
	:width                  111
	:height                 13
	:foreground-color       ""
	:label                  "Field Properties:"
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
	:x                      238
	:y                      33
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
	:type                   :message
	:name                   f_config_msg3
	:owner                  fields_controls
	:help                   ""
	:x                      238
	:y                      49
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
	:type                   :message
	:name                   f_config_msg4
	:owner                  fields_controls
	:help                   ""
	:x                      238
	:y                      65
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
	:type                   :text-field
	:name                   f_config_tx1
	:owner                  fields_controls
	:help                   ""
	:x                      238
	:y                      81
	:width                  196
	:height                 15
	:value-x                330
	:value-y                81
	:value-length           13
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Source Host:"
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
	:name                   f_config_tx2
	:owner                  fields_controls
	:help                   ""
	:x                      238
	:y                      99
	:width                  198
	:height                 15
	:value-x                324
	:value-y                99
	:value-length           14
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Server Port:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_port_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu f_config_tx2)
		:when                   (Notify )
		:to                     (fields_pu f_config_tx2)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_port_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   f_config_tx3
	:owner                  fields_controls
	:help                   ""
	:x                      238
	:y                      117
	:width                  199
	:height                 15
	:value-x                341
	:value-y                117
	:value-length           12
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Field Number:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_field_no_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu f_config_tx3)
		:when                   (Notify )
		:to                     (fields_pu f_config_tx3)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_field_no_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   f_config_tx6
	:owner                  fields_controls
	:help                   ""
	:x                      238
	:y                      135
	:width                  195
	:height                 15
	:value-x                305
	:value-y                135
	:value-length           16
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Subtype:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_subtype_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu f_config_tx6)
		:when                   (Notify )
		:to                     (fields_pu f_config_tx6)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_subtype_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   f_config_tx5
	:owner                  fields_controls
	:help                   ""
	:x                      238
	:y                      153
	:width                  198
	:height                 15
	:value-x                308
	:value-y                153
	:value-length           16
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Instance:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_instance_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu f_config_tx5)
		:when                   (Notify )
		:to                     (fields_pu f_config_tx5)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_instance_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   f_config_tx4
	:owner                  fields_controls
	:help                   ""
	:x                      238
	:y                      171
	:width                  152
	:height                 15
	:value-x                318
	:value-y                171
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
	:type                   :message
	:name                   f_config_msg6
	:owner                  fields_controls
	:help                   ""
	:x                      400
	:y                      174
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
	:name                   f_config_tx9
	:owner                  fields_controls
	:help                   ""
	:x                      238
	:y                      190
	:width                  150
	:height                 15
	:value-x                332
	:value-y                190
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
	:type                   :message
	:name                   f_config_msg7
	:owner                  fields_controls
	:help                   ""
	:x                      400
	:y                      192
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
	:name                   group2
	:x                      238
	:y                      14
	:group-type             nil
	:members                (f_config_sel_bt f_config_msg1 f_config_msg2 f_config_msg3 f_config_msg4 f_config_tx1 f_config_tx2 f_config_tx3 f_config_tx6 f_config_tx5 f_config_tx4 f_config_msg6 f_config_tx9 f_config_msg7 )
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
	:x                      238
	:y                      206
	:width                  191
	:height                 23
	:value-x                238
	:value-y                206
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         set_ren_meth_proc
	:event-handler          nil
	:choices                ("Render as FIlled Contours" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
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
	:type                   :setting
	:name                   update_st
	:owner                  fields_controls
	:help                   ""
	:x                      238
	:y                      224
	:width                  192
	:height                 23
	:value-x                238
	:value-y                224
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
	:choices                ("Auto Update/ Store Image" )
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
	:x                      238
	:y                      242
	:width                  183
	:height                 23
	:value-x                238
	:value-y                242
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
	:choices                ("Request Composite Data" )
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
	:name                   use_servmap_st
	:owner                  fields_controls
	:help                   ""
	:x                      238
	:y                      260
	:width                  148
	:height                 23
	:value-x                238
	:value-y                260
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         set_servmap_proc
	:event-handler          nil
	:choices                ("Use Server Mapper" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (fields_pu use_servmap_st)
		:when                   (Notify )
		:to                     (fields_pu use_servmap_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_servmap_proc)
		)
	)
)
(
	:type                   :group
	:name                   group1
	:x                      238
	:y                      206
	:group-type             nil
	:members                (ren_meth_st update_st max_st use_servmap_st )
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
	:name                   f_config_msg8
	:owner                  fields_controls
	:help                   ""
	:x                      238
	:y                      284
	:width                  139
	:height                 13
	:foreground-color       ""
	:label                  "Colorscale Mapping:"
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
	:x                      238
	:y                      306
	:width                  184
	:height                 15
	:value-x                318
	:value-y                306
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
	:x                      238
	:y                      326
	:width                  175
	:height                 15
	:value-x                285
	:value-y                326
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
	:type                   :button
	:name                   f_config_color_apply_bt
	:owner                  fields_controls
	:help                   ""
	:x                      238
	:y                      346
	:width                  177
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Apply New Color Mapping"
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
	:type                   :group
	:name                   group3
	:x                      238
	:y                      284
	:group-type             nil
	:members                (f_config_msg8 f_config_tx7 f_config_tx8 f_config_color_apply_bt )
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
	:name                   group4
	:x                      238
	:y                      14
	:group-type             nil
	:members                (group2 group1 group3 )
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
)
