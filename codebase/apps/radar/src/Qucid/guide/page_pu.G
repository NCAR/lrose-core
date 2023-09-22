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
	:name                   page_pu
	:owner                  nil
	:width                  305
	:height                 667
	:background-color       ""
	:foreground-color       ""
	:label                  " Display Page Configuration"
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
	:owner                  page_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  305
	:height                 667
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
	:type                   :setting
	:name                   over_fld_st
	:owner                  controls1
	:help                   "This will turn on an overlay field.
"
	:x                      214
	:y                      7
	:width                  66
	:height                 23
	:value-x                214
	:value-y                7
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         overlay_fld_on_proc
	:event-handler          nil
	:choices                ("Active" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu over_fld_st)
		:when                   (Notify )
		:to                     (page_pu over_fld_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (overlay_fld_on_proc)
		)
	)
)
(
	:type                   :setting
	:name                   ov_field_num_st
	:owner                  controls1
	:help                   ""
	:x                      132
	:y                      8
	:width                  44
	:height                 23
	:value-x                132
	:value-y                8
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         ov_field_num_sel_proc
	:event-handler          nil
	:choices                ("1" "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "12" "13" "14" "15" "16" )
	:choice-label-types     (:string :string :string :string :string :string :string :string :string :string :string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" )
	:choice-defaults        (nil nil nil nil nil nil nil nil nil nil nil nil nil nil nil nil )
	:initial-selections     (nil nil nil nil nil nil nil nil nil nil nil nil nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu ov_field_num_st)
		:when                   (Notify )
		:to                     (page_pu ov_field_num_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (ov_field_num_sel_proc)
		)
	)
)
(
	:type                   :message
	:name                   message5
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      12
	:width                  120
	:height                 13
	:foreground-color       ""
	:label                  "GRIDED DATA LAYER ->"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   ov_fld_mu_bt
	:owner                  controls1
	:help                   ""
	:x                      14
	:y                      36
	:width                  32
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  ""
	:label-type             :string
	:initial-state          :active
	:menu                   cont_mu
	:notify-handler         nil
	:event-handler          set_ov_fld_proc
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu ov_fld_mu_bt)
		:when                   (AnyEvent )
		:to                     (page_pu ov_fld_mu_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_ov_fld_proc)
		)
	)
)
(
	:type                   :message
	:name                   ov_fld_msg
	:owner                  controls1
	:help                   ""
	:x                      60
	:y                      38
	:width                  24
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
	:name                   layer_mode_st
	:owner                  controls1
	:help                   ""
	:x                      168
	:y                      56
	:width                  78
	:height                 63
	:value-x                168
	:value-y                73
	:rows                   0
	:columns                1
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Layers on:"
	:label-type             :string
	:notify-handler         layer_mode_proc
	:event-handler          nil
	:choices                ("     Top" "   Bottom  " )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu layer_mode_st)
		:when                   (Notify )
		:to                     (page_pu layer_mode_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (layer_mode_proc)
		)
	)
)
(
	:type                   :setting
	:name                   layer_legend_st
	:owner                  controls1
	:help                   ""
	:x                      22
	:y                      60
	:width                  96
	:height                 23
	:value-x                22
	:value-y                60
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         layer_legend_proc
	:event-handler          nil
	:choices                ("Show Legend" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu layer_legend_st)
		:when                   (Notify )
		:to                     (page_pu layer_legend_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (layer_legend_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   ov_fld_min_tx
	:owner                  controls1
	:help                   ""
	:x                      22
	:y                      84
	:width                  104
	:height                 15
	:value-x                90
	:value-y                84
	:value-length           6
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
	:notify-handler         overlay_fld_min_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu ov_fld_min_tx)
		:when                   (Notify )
		:to                     (page_pu ov_fld_min_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (overlay_fld_min_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   ov_fld_max_tx
	:owner                  controls1
	:help                   ""
	:x                      22
	:y                      102
	:width                  104
	:height                 15
	:value-x                90
	:value-y                102
	:value-length           6
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Max Value:"
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
		:from                   (page_pu ov_fld_max_tx)
		:when                   (Notify )
		:to                     (page_pu ov_fld_max_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (overlay_fld_max_proc)
		)
	)
)
(
	:type                   :message
	:name                   message6
	:owner                  controls1
	:help                   ""
	:x                      10
	:y                      125
	:width                  288
	:height                 13
	:foreground-color       "Medium Blue"
	:label                  "************************************************"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   cont_st
	:owner                  controls1
	:help                   ""
	:x                      230
	:y                      135
	:width                  66
	:height                 23
	:value-x                230
	:value-y                135
	:rows                   1
	:columns                0
	:layout-type            :vertical
	:foreground-color       "Medium Blue"
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         cont_activate_proc
	:event-handler          nil
	:choices                ("Active" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu cont_st)
		:when                   (Notify )
		:to                     (page_pu cont_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_activate_proc)
		)
	)
)
(
	:type                   :setting
	:name                   cont_layer_st
	:owner                  controls1
	:help                   ""
	:x                      142
	:y                      136
	:width                  44
	:height                 23
	:value-x                142
	:value-y                136
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       "Medium Blue"
	:setting-type           :stack
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         cont_layer_set_proc
	:event-handler          nil
	:choices                ("1" "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "12" "13" "14" "15" "16" )
	:choice-label-types     (:string :string :string :string :string :string :string :string :string :string :string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" "" "" "" "" "" "" "" "" "" "" )
	:choice-defaults        (nil nil nil nil nil nil nil nil nil nil nil nil nil nil nil nil )
	:initial-selections     (nil nil nil nil nil nil nil nil nil nil nil nil nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu cont_layer_st)
		:when                   (Notify )
		:to                     (page_pu cont_layer_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_layer_set_proc)
		)
	)
)
(
	:type                   :message
	:name                   cont_label_msg
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      140
	:width                  126
	:height                 13
	:foreground-color       "Medium Blue"
	:label                  "LINE CONTOUR LAYER ->"
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
	:x                      12
	:y                      156
	:width                  32
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Medium Blue"
	:label                  ""
	:label-type             :string
	:initial-state          :active
	:menu                   cont_mu
	:notify-handler         nil
	:event-handler          set_cont_field_proc
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu cont_bt)
		:when                   (AnyEvent )
		:to                     (page_pu cont_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_cont_field_proc)
		)
	)
)
(
	:type                   :message
	:name                   cont_msg
	:owner                  controls1
	:help                   ""
	:x                      62
	:y                      162
	:width                  24
	:height                 13
	:foreground-color       "Medium Blue"
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
	:name                   cont_fr_tx
	:owner                  controls1
	:help                   ""
	:x                      14
	:y                      184
	:width                  74
	:height                 15
	:value-x                52
	:value-y                184
	:value-length           6
	:stored-length          80
	:rows                   3
	:foreground-color       "Medium Blue"
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
		:from                   (page_pu cont_fr_tx)
		:when                   (Notify )
		:to                     (page_pu cont_fr_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_fr_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   cont_to_tx
	:owner                  controls1
	:help                   ""
	:x                      109
	:y                      184
	:width                  62
	:height                 15
	:value-x                135
	:value-y                184
	:value-length           6
	:stored-length          80
	:rows                   3
	:foreground-color       "Medium Blue"
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
		:from                   (page_pu cont_to_tx)
		:when                   (Notify )
		:to                     (page_pu cont_to_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_to_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   cont_int_tx
	:owner                  controls1
	:help                   ""
	:x                      187
	:y                      184
	:width                  62
	:height                 15
	:value-x                213
	:value-y                184
	:value-length           6
	:stored-length          80
	:rows                   3
	:foreground-color       "Medium Blue"
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
		:from                   (page_pu cont_int_tx)
		:when                   (Notify )
		:to                     (page_pu cont_int_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_int_proc)
		)
	)
)
(
	:type                   :setting
	:name                   cont_legend_st
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      200
	:width                  96
	:height                 23
	:value-x                16
	:value-y                200
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       "Medium Blue"
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         cont_legend_proc
	:event-handler          nil
	:choices                ("Show Legend" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu cont_legend_st)
		:when                   (Notify )
		:to                     (page_pu cont_legend_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_legend_proc)
		)
	)
)
(
	:type                   :message
	:name                   cont_color_msg
	:owner                  controls1
	:help                   ""
	:x                      140
	:y                      207
	:width                  30
	:height                 13
	:foreground-color       "Medium Blue"
	:label                  "Color"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          cont_color_proc
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu cont_color_msg)
		:when                   (AnyEvent )
		:to                     (page_pu cont_color_msg)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_color_proc)
		)
	)
)
(
	:type                   :setting
	:name                   cont_label_st
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      225
	:width                  96
	:height                 23
	:value-x                16
	:value-y                225
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       "Medium Blue"
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         cont_label_set_proc
	:event-handler          nil
	:choices                ("Line Labels" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu cont_label_st)
		:when                   (Notify )
		:to                     (page_pu cont_label_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cont_label_set_proc)
		)
	)
)
(
	:type                   :slider
	:name                   prod_font_sl
	:owner                  controls1
	:help                   ""
	:x                      126
	:y                      230
	:width                  109
	:height                 20
	:value-x                203
	:value-y                230
	:slider-width           16
	:ticks                  0
	:foreground-color       "Medium Blue"
	:label                  "Label Sze:  "
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
		:from                   (page_pu prod_font_sl)
		:when                   (Notify )
		:to                     (page_pu prod_font_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_font_sel_proc)
		)
	)
)
(
	:type                   :message
	:name                   message3
	:owner                  controls1
	:help                   ""
	:x                      10
	:y                      257
	:width                  282
	:height                 13
	:foreground-color       "Dark Green"
	:label                  "***********************************************"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   wind_legend_st
	:owner                  controls1
	:help                   ""
	:x                      222
	:y                      264
	:width                  72
	:height                 23
	:value-x                222
	:value-y                264
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       "Dark Green"
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         wind_legend_proc
	:event-handler          nil
	:choices                ("Legends" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu wind_legend_st)
		:when                   (Notify )
		:to                     (page_pu wind_legend_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (wind_legend_proc)
		)
	)
)
(
	:type                   :button
	:name                   vect_set_color_bt
	:owner                  controls1
	:help                   ""
	:x                      116
	:y                      268
	:width                  90
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "dark green"
	:label                  "Change Color"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         vect_set_color_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu vect_set_color_bt)
		:when                   (Notify )
		:to                     (page_pu vect_set_color_bt)
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
	:x                      8
	:y                      270
	:width                  72
	:height                 13
	:foreground-color       "Dark Green"
	:label                  "WINDS LAYERS"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :slider
	:name                   density_sl
	:owner                  controls1
	:help                   ""
	:x                      10
	:y                      294
	:width                  89
	:height                 20
	:value-x                33
	:value-y                294
	:slider-width           50
	:ticks                  0
	:foreground-color       "Dark Green"
	:label                  "Num"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             nil
	:min-value              5
	:max-value              1000
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         set_wind_density_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu density_sl)
		:when                   (Notify )
		:to                     (page_pu density_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_wind_density_proc)
		)
	)
)
(
	:type                   :slider
	:name                   prod_line_sl
	:owner                  controls1
	:help                   ""
	:x                      98
	:y                      294
	:width                  66
	:height                 20
	:value-x                133
	:value-y                294
	:slider-width           15
	:ticks                  0
	:foreground-color       "Dark Green"
	:label                  "Width"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
	:show-range             nil
	:show-value             nil
	:min-value              1
	:max-value              3
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
		:from                   (page_pu prod_line_sl)
		:when                   (Notify )
		:to                     (page_pu prod_line_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_line_width_proc)
		)
	)
)
(
	:type                   :slider
	:name                   wind_sl
	:owner                  controls1
	:help                   ""
	:x                      172
	:y                      294
	:width                  77
	:height                 20
	:value-x                213
	:value-y                294
	:slider-width           20
	:ticks                  0
	:foreground-color       "Dark Green"
	:label                  "Length"
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          nil
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
		:from                   (page_pu wind_sl)
		:when                   (Notify )
		:to                     (page_pu wind_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (wind_scale_proc)
		)
	)
)
(
	:type                   :message
	:name                   wind_msg
	:owner                  controls1
	:help                   ""
	:x                      251
	:y                      294
	:width                  30
	:height                 13
	:foreground-color       "Dark Green"
	:label                  "1 min"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :scrolling-list
	:name                   wind_on_list
	:owner                  controls1
	:help                   ""
	:x                      14
	:y                      312
	:width                  260
	:height                 92
	:value-x                14
	:value-y                312
	:rows                   4
	:foreground-color       "Dark Green"
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
		:from                   (page_pu wind_on_list)
		:when                   (Notify )
		:to                     (page_pu wind_on_list)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (vect_field_num_set_proc)
		)
	)
)
(
	:type                   :message
	:name                   message2
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      412
	:width                  288
	:height                 13
	:foreground-color       "Firebrick"
	:label                  "************************************************"
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
	:x                      160
	:y                      430
	:width                  90
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Firebrick"
	:label                  "Change Color"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         set_ov_color_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu map_set_color_bt)
		:when                   (Notify )
		:to                     (page_pu map_set_color_bt)
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
	:x                      10
	:y                      432
	:width                  72
	:height                 13
	:foreground-color       "Firebrick"
	:label                  "MAP OVERLAYS"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :scrolling-list
	:name                   overlay_list
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      452
	:width                  260
	:height                 92
	:value-x                12
	:value-y                452
	:rows                   4
	:foreground-color       "Firebrick"
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
		:from                   (page_pu overlay_list)
		:when                   (Notify )
		:to                     (page_pu overlay_list)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (over_select_proc)
		)
	)
)
(
	:type                   :setting
	:name                   ref_st
	:owner                  controls1
	:help                   ""
	:x                      13
	:y                      548
	:width                  96
	:height                 23
	:value-x                13
	:value-y                548
	:rows                   0
	:columns                1
	:layout-type            :vertical
	:foreground-color       "Firebrick"
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         ref_mark_proc
	:event-handler          nil
	:choices                ("Range Rings" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu ref_st)
		:when                   (Notify )
		:to                     (page_pu ref_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (ref_mark_proc)
		)
	)
)
(
	:type                   :message
	:name                   ref_mark_color_msg
	:owner                  controls1
	:help                   ""
	:x                      121
	:y                      554
	:width                  30
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
		:from                   (page_pu ref_mark_color_msg)
		:when                   (AnyEvent )
		:to                     (page_pu ref_mark_color_msg)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (ref_color_proc)
		)
	)
)
(
	:type                   :setting
	:name                   land_use_st
	:owner                  controls1
	:help                   ""
	:x                      13
	:y                      573
	:width                  78
	:height                 23
	:value-x                13
	:value-y                573
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       "Firebrick"
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         land_use_activate
	:event-handler          nil
	:choices                ("Land Use" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu land_use_st)
		:when                   (Notify )
		:to                     (page_pu land_use_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (land_use_activate)
		)
	)
)
(
	:type                   :slider
	:name                   dim_sl
	:owner                  controls1
	:help                   ""
	:x                      105
	:y                      579
	:width                  166
	:height                 20
	:value-x                165
	:value-y                579
	:slider-width           64
	:ticks                  0
	:foreground-color       "Firebrick"
	:label                  "Brightness"
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
		:from                   (page_pu dim_sl)
		:when                   (Notify )
		:to                     (page_pu dim_sl)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (dim_im_proc)
		)
	)
)
(
	:type                   :setting
	:name                   topo_st
	:owner                  controls1
	:help                   ""
	:x                      13
	:y                      599
	:width                  138
	:height                 23
	:value-x                13
	:value-y                599
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       "Firebrick"
	:setting-type           :check
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         topo_onoff_proc
	:event-handler          nil
	:choices                ("Terrain Masking On" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu topo_st)
		:when                   (Notify )
		:to                     (page_pu topo_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (topo_onoff_proc)
		)
	)
)
(
	:type                   :button
	:name                   save_bt
	:owner                  controls1
	:help                   ""
	:x                      38
	:y                      632
	:width                  204
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Magenta"
	:label                  "Save Plan View Image to File..."
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         save_h_image_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (page_pu save_bt)
		:when                   (Notify )
		:to                     (page_pu save_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (save_h_image_proc)
		)
	)
)
)
