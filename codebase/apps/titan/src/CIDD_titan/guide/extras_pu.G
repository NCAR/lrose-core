;GIL-3
(
(
	:type                   :menu
	:name                   cont_mu
	:help                   ""
	:columns                1
	:menu-type              :command
	:menu-handler           cont_field_mu_gen
	:menu-title             "Data Field:"
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
		:from                   (cont_mu)
		:when                   (Notify )
		:to                     (cont_mu)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_field_mu_gen)
		)
	)
)
(
	:type                   :popup-window
	:name                   extras_pu
	:owner                  nil
	:width                  456
	:height                 539
	:background-color       ""
	:foreground-color       ""
	:label                  "Overlays"
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
	:owner                  extras_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  456
	:height                 539
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
	:name                   message5
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      0
	:width                  167
	:height                 13
	:foreground-color       ""
	:label                  "GRIDED DATA OVERLAYS"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   map_set_color_bt
	:owner                  controls1
	:help                   ""
	:x                      130
	:y                      8
	:width                  87
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Reset Color"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         set_ov_color_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu map_set_color_bt)
		:when                   (Notify )
		:to                     (extras_pu map_set_color_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_ov_color_proc)
		)
	)
)
(
	:type                   :message
	:name                   message1
	:owner                  controls1
	:help                   ""
	:x                      6
	:y                      12
	:width                  104
	:height                 13
	:foreground-color       ""
	:label                  "MAP OVERLAYS"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   ov_field_num_st
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      20
	:width                  166
	:height                 23
	:value-x                278
	:value-y                20
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Layer"
	:label-type             :string
	:notify-handler         ov_field_num_sel_proc
	:event-handler          nil
	:choices                ("1" "2" "3" "4" "5" )
	:choice-label-types     (:string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" )
	:initial-selections     (nil nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu ov_field_num_st)
		:when                   (Notify )
		:to                     (extras_pu ov_field_num_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (ov_field_num_sel_proc)
		)
	)
)
(
	:type                   :scrolling-list
	:name                   overlay_list
	:owner                  controls1
	:help                   ""
	:x                      6
	:y                      33
	:width                  200
	:height                 200
	:value-x                6
	:value-y                33
	:rows                   10
	:foreground-color       ""
	:label                  ""
	:title                  ""
	:label-type             :string
	:layout-type            :vertical
	:read-only              t
	:multiple-selections    t
	:selection-required     nil
	:initial-state          :active
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:notify-handler         over_select_proc
	:event-handler          nil
	:initial-list-values    ()
	:initial-list-glyphs    ()
	:initial-selections     ()
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu overlay_list)
		:when                   (Notify )
		:to                     (extras_pu overlay_list)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (over_select_proc)
		)
	)
)
(
	:type                   :button
	:name                   ov_fld_mu_bt
	:owner                  controls1
	:help                   ""
	:x                      368
	:y                      48
	:width                  62
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Field"
	:label-type             :string
	:initial-state          :active
	:menu                   cont_mu
	:notify-handler         nil
	:event-handler          set_ov_fld_proc
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu ov_fld_mu_bt)
		:when                   (AnyEvent )
		:to                     (extras_pu ov_fld_mu_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_ov_fld_proc)
		)
	)
)
(
	:type                   :setting
	:name                   over_fld_st
	:owner                  controls1
	:help                   "This will turn on an overlay field.
"
	:x                      232
	:y                      50
	:width                  52
	:height                 40
	:value-x                232
	:value-y                67
	:rows                   0
	:columns                1
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  "Field:"
	:label-type             :string
	:notify-handler         overlay_fld_on_proc
	:event-handler          nil
	:choices                ("On" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu over_fld_st)
		:when                   (Notify )
		:to                     (extras_pu over_fld_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (overlay_fld_on_proc)
		)
	)
)
(
	:type                   :message
	:name                   ov_fld_msg
	:owner                  controls1
	:help                   ""
	:x                      276
	:y                      50
	:width                  35
	:height                 13
	:foreground-color       ""
	:label                  "None"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   ov_fld_max_tx
	:owner                  controls1
	:help                   ""
	:x                      346
	:y                      96
	:width                  86
	:height                 15
	:value-x                384
	:value-y                96
	:value-length           6
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Max:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         overlay_fld_max_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu ov_fld_max_tx)
		:when                   (Notify )
		:to                     (extras_pu ov_fld_max_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (overlay_fld_max_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   ov_fld_min_tx
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      97
	:width                  85
	:height                 15
	:value-x                269
	:value-y                97
	:value-length           6
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         overlay_fld_min_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu ov_fld_min_tx)
		:when                   (Notify )
		:to                     (extras_pu ov_fld_min_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (overlay_fld_min_proc)
		)
	)
)
(
	:type                   :setting
	:name                   layer_mode_st
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      119
	:width                  172
	:height                 23
	:value-x                306
	:value-y                119
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Main  ON:"
	:label-type             :string
	:notify-handler         layer_mode_proc
	:event-handler          nil
	:choices                ("Bottom" "Top" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu layer_mode_st)
		:when                   (Notify )
		:to                     (extras_pu layer_mode_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (layer_mode_proc)
		)
	)
)
(
	:type                   :message
	:name                   message6
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      149
	:width                  198
	:height                 13
	:foreground-color       ""
	:label                  "*********************************"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   cont_bt
	:owner                  controls1
	:help                   ""
	:x                      368
	:y                      164
	:width                  62
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Field"
	:label-type             :string
	:initial-state          :active
	:menu                   cont_mu
	:notify-handler         nil
	:event-handler          set_cont_field_proc
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu cont_bt)
		:when                   (AnyEvent )
		:to                     (extras_pu cont_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_cont_field_proc)
		)
	)
)
(
	:type                   :message
	:name                   cont_label_msg
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      169
	:width                  80
	:height                 13
	:foreground-color       ""
	:label                  "CONTOURS:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   cont_layer_st
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      189
	:width                  166
	:height                 23
	:value-x                278
	:value-y                189
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Layer"
	:label-type             :string
	:notify-handler         cont_layer_set_proc
	:event-handler          nil
	:choices                ("1" "2" "3" "4" "5" )
	:choice-label-types     (:string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" )
	:initial-selections     (nil nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu cont_layer_st)
		:when                   (Notify )
		:to                     (extras_pu cont_layer_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_layer_set_proc)
		)
	)
)
(
	:type                   :message
	:name                   cont_msg
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      219
	:width                  35
	:height                 13
	:foreground-color       ""
	:label                  "None"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   cont_label_st
	:owner                  controls1
	:help                   ""
	:x                      320
	:y                      222
	:width                  106
	:height                 23
	:value-x                374
	:value-y                222
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  "Labels"
	:label-type             :string
	:notify-handler         cont_label_set_proc
	:event-handler          nil
	:choices                ("On" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu cont_label_st)
		:when                   (Notify )
		:to                     (extras_pu cont_label_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_label_set_proc)
		)
	)
)
(
	:type                   :setting
	:name                   cont_st
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      239
	:width                  52
	:height                 23
	:value-x                232
	:value-y                239
	:rows                   1
	:columns                0
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         cont_activate_proc
	:event-handler          nil
	:choices                ("On" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu cont_st)
		:when                   (Notify )
		:to                     (extras_pu cont_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_activate_proc)
		)
	)
)
(
	:type                   :setting
	:name                   ref_st
	:owner                  controls1
	:help                   ""
	:x                      6
	:y                      241
	:width                  123
	:height                 69
	:value-x                6
	:value-y                258
	:rows                   0
	:columns                1
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  "REFERENCE LINES "
	:label-type             :string
	:notify-handler         ref_mark_proc
	:event-handler          nil
	:choices                ("Range Rings" "Azmith Lines" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu ref_st)
		:when                   (Notify )
		:to                     (extras_pu ref_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (ref_mark_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   cont_int_tx
	:owner                  controls1
	:help                   ""
	:x                      345
	:y                      244
	:width                  76
	:height                 15
	:value-x                373
	:value-y                244
	:value-length           6
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "By:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         cont_int_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu cont_int_tx)
		:when                   (Notify )
		:to                     (extras_pu cont_int_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_int_proc)
		)
	)
)
(
	:type                   :message
	:name                   cont_color_msg
	:owner                  controls1
	:help                   ""
	:x                      367
	:y                      268
	:width                  36
	:height                 13
	:foreground-color       ""
	:label                  "Color"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          cont_color_proc
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu cont_color_msg)
		:when                   (AnyEvent )
		:to                     (extras_pu cont_color_msg)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_color_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   cont_fr_tx
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      269
	:width                  93
	:height                 15
	:value-x                277
	:value-y                269
	:value-length           6
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "From:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         cont_fr_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu cont_fr_tx)
		:when                   (Notify )
		:to                     (extras_pu cont_fr_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_fr_proc)
		)
	)
)
(
	:type                   :message
	:name                   ref_mark_color_msg
	:owner                  controls1
	:help                   ""
	:x                      130
	:y                      274
	:width                  36
	:height                 13
	:foreground-color       ""
	:label                  "Color"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          ref_color_proc
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu ref_mark_color_msg)
		:when                   (AnyEvent )
		:to                     (extras_pu ref_mark_color_msg)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (ref_color_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   cont_to_tx
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      291
	:width                  76
	:height                 15
	:value-x                260
	:value-y                291
	:value-length           6
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "To:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         cont_to_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu cont_to_tx)
		:when                   (Notify )
		:to                     (extras_pu cont_to_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_to_proc)
		)
	)
)
(
	:type                   :message
	:name                   message7
	:owner                  controls1
	:help                   ""
	:x                      6
	:y                      318
	:width                  216
	:height                 13
	:foreground-color       ""
	:label                  "************************************"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   message2
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      318
	:width                  192
	:height                 13
	:foreground-color       ""
	:label                  "********************************"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   vect_set_color_bt
	:owner                  controls1
	:help                   ""
	:x                      344
	:y                      328
	:width                  87
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Reset Color"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         vect_set_color_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu vect_set_color_bt)
		:when                   (Notify )
		:to                     (extras_pu vect_set_color_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (vect_set_color_proc)
		)
	)
)
(
	:type                   :message
	:name                   message4
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      333
	:width                  51
	:height                 13
	:foreground-color       ""
	:label                  "Vectors"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :slider
	:name                   prod_font_sl
	:owner                  controls1
	:help                   ""
	:x                      6
	:y                      339
	:width                  196
	:height                 20
	:value-x                136
	:value-y                339
	:slider-width           50
	:ticks                  0
	:foreground-color       ""
	:label                  "Product Label Size"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             nil
	:min-value              0
	:max-value              4
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         prod_font_sel_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu prod_font_sl)
		:when                   (Notify )
		:to                     (extras_pu prod_font_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_font_sel_proc)
		)
	)
)
(
	:type                   :scrolling-list
	:name                   wind_on_list
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      353
	:width                  200
	:height                 92
	:value-x                232
	:value-y                353
	:rows                   4
	:foreground-color       ""
	:label                  ""
	:title                  ""
	:label-type             :string
	:layout-type            :vertical
	:read-only              t
	:multiple-selections    t
	:selection-required     nil
	:initial-state          :active
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:notify-handler         vect_field_num_set_proc
	:event-handler          nil
	:initial-list-values    ()
	:initial-list-glyphs    ()
	:initial-selections     ()
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu wind_on_list)
		:when                   (Notify )
		:to                     (extras_pu wind_on_list)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (vect_field_num_set_proc)
		)
	)
)
(
	:type                   :slider
	:name                   prod_line_sl
	:owner                  controls1
	:help                   ""
	:x                      6
	:y                      367
	:width                  199
	:height                 20
	:value-x                139
	:value-y                367
	:slider-width           50
	:ticks                  0
	:foreground-color       ""
	:label                  "Product Line Width"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             nil
	:min-value              1
	:max-value              5
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         prod_line_width_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu prod_line_sl)
		:when                   (Notify )
		:to                     (extras_pu prod_line_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_line_width_proc)
		)
	)
)
(
	:type                   :message
	:name                   prod_tm_msg
	:owner                  controls1
	:help                   ""
	:x                      6
	:y                      395
	:width                  90
	:height                 13
	:foreground-color       ""
	:label                  "Product Time"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   prod_data_tm_msg
	:owner                  controls1
	:help                   ""
	:x                      6
	:y                      416
	:width                  162
	:height                 13
	:foreground-color       ""
	:label                  "Product Data Time:  N/A"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   wind_msg
	:owner                  controls1
	:help                   ""
	:x                      380
	:y                      428
	:width                  36
	:height                 13
	:foreground-color       ""
	:label                  "1 min"
	:label-type             :string
	:label-bold             nil
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   prod_tm_sel_st
	:owner                  controls1
	:help                   ""
	:x                      6
	:y                      437
	:width                  158
	:height                 40
	:value-x                6
	:value-y                454
	:rows                   1
	:columns                0
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Show Products Valid at:"
	:label-type             :string
	:notify-handler         prod_time_sel_proc
	:event-handler          nil
	:choices                ("Loop End" "Each Frame" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu prod_tm_sel_st)
		:when                   (Notify )
		:to                     (extras_pu prod_tm_sel_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_time_sel_proc)
		)
	)
)
(
	:type                   :slider
	:name                   wind_sl
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      452
	:width                  138
	:height                 20
	:value-x                273
	:value-y                452
	:slider-width           60
	:ticks                  0
	:foreground-color       ""
	:label                  "Scale"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          t
	:show-range             nil
	:show-value             nil
	:min-value              0
	:max-value              8
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         wind_scale_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu wind_sl)
		:when                   (Notify )
		:to                     (extras_pu wind_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (wind_scale_proc)
		)
	)
)
(
	:type                   :message
	:name                   message3
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      479
	:width                  192
	:height                 13
	:foreground-color       ""
	:label                  "********************************"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :slider
	:name                   prod_tm_wdth_sl
	:owner                  controls1
	:help                   ""
	:x                      6
	:y                      485
	:width                  209
	:height                 20
	:value-x                107
	:value-y                485
	:slider-width           50
	:ticks                  0
	:foreground-color       ""
	:label                  "Seconds Wide"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             t
	:min-value              0
	:max-value              3000
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :invisible
	:notify-handler         prod_time_width_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu prod_tm_wdth_sl)
		:when                   (Notify )
		:to                     (extras_pu prod_tm_wdth_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_time_width_proc)
		)
	)
)
(
	:type                   :message
	:name                   dim_msg
	:owner                  controls1
	:help                   ""
	:x                      246
	:y                      499
	:width                  103
	:height                 13
	:foreground-color       ""
	:label                  "IMAGE DIMMER"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :slider
	:name                   dim_sl
	:owner                  controls1
	:help                   ""
	:x                      232
	:y                      517
	:width                  157
	:height                 20
	:value-x                232
	:value-y                517
	:slider-width           120
	:ticks                  0
	:foreground-color       ""
	:label                  ""
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          t
	:show-range             nil
	:show-value             nil
	:min-value              0
	:max-value              16
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         dim_im_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu dim_sl)
		:when                   (Notify )
		:to                     (extras_pu dim_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (dim_im_proc)
		)
	)
)
(
	:type                   :group
	:name                   group4
	:x                      232
	:y                      499
	:group-type             nil
	:members                (dim_msg dim_sl )
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
	:name                   save_bt
	:owner                  controls1
	:help                   ""
	:x                      6
	:y                      513
	:width                  152
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Save Plan View Image"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         save_h_image_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (extras_pu save_bt)
		:when                   (Notify )
		:to                     (extras_pu save_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (save_h_image_proc)
		)
	)
)
)
