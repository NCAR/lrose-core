;GIL-3
(
(
	:type                   :base-window
	:name                   window1
	:owner                  nil
	:width                  683
	:height                 755
	:background-color       ""
	:foreground-color       ""
	:label                  "XSPDB_CLIENT"
	:label-type             :string
	:initial-state          :open
	:show-footer            nil
	:resizable              t
	:icon-file              ""
	:icon-label             ""
	:icon-mask-file         ""
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  window1
	:help                   ""
	:x                      0
	:y                      0
	:width                  683
	:height                 137
	:background-color       "Light Steel Blue"
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
	:name                   source_tx
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      12
	:width                  646
	:height                 15
	:value-x                46
	:value-y                12
	:value-length           76
	:stored-length          512
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
	:notify-handler         source_string_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 source_tx)
		:when                   (Notify )
		:to                     (window1 source_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (source_string_proc)
		)
	)
)
(
	:type                   :setting
	:name                   prod_typ_st
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      26
	:width                  217
	:height                 23
	:value-x                134
	:value-y                26
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Select Product ID:"
	:label-type             :string
	:notify-handler         prod_typ_proc
	:event-handler          nil
	:choices                ("ANY" "Standard" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:choice-defaults        (nil nil )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (window1 prod_typ_st)
		:when                   (Notify )
		:to                     (window1 prod_typ_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_typ_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   prod_id_tx
	:owner                  controls1
	:help                   ""
	:x                      368
	:y                      32
	:width                  138
	:height                 15
	:value-x                410
	:value-y                32
	:value-length           12
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Type"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          "0"
	:initial-state          :active
	:read-only              nil
	:notify-handler         prod_id_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 prod_id_tx)
		:when                   (Notify )
		:to                     (window1 prod_id_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_id_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   data2_tx
	:owner                  controls1
	:help                   ""
	:x                      510
	:y                      34
	:width                  146
	:height                 15
	:value-x                560
	:value-y                34
	:value-length           12
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Type2"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          "0"
	:initial-state          :active
	:read-only              nil
	:notify-handler         datatype2_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 data2_tx)
		:when                   (Notify )
		:to                     (window1 data2_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (datatype2_proc)
		)
	)
)
(
	:type                   :button
	:name                   now_bt
	:owner                  controls1
	:help                   ""
	:x                      280
	:y                      56
	:width                  84
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Set to Now"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         set_now_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 now_bt)
		:when                   (Notify )
		:to                     (window1 now_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_now_proc)
		)
	)
)
(
	:type                   :button
	:name                   del_bt
	:owner                  controls1
	:help                   ""
	:x                      390
	:y                      56
	:width                  108
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Delete Product"
	:label-type             :string
	:initial-state          :inactive
	:menu                   nil
	:notify-handler         delete_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 del_bt)
		:when                   (Notify )
		:to                     (window1 del_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (delete_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   end_tx
	:owner                  controls1
	:help                   ""
	:x                      10
	:y                      58
	:width                  264
	:height                 15
	:value-x                114
	:value-y                58
	:value-length           20
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Interest Time:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         end_tm_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 end_tx)
		:when                   (Notify )
		:to                     (window1 end_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (end_tm_proc)
		)
	)
)
(
	:type                   :button
	:name                   submit_bt
	:owner                  controls1
	:help                   ""
	:x                      534
	:y                      58
	:width                  135
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "SUMBMIT REQUEST"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         submit_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 submit_bt)
		:when                   (Notify )
		:to                     (window1 submit_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (submit_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   time_delta_ntx
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      80
	:width                  136
	:height                 15
	:value-x                61
	:value-y                80
	:value-length           6
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Within"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              1000000000
	:min-value              0
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         time_delta_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 time_delta_ntx)
		:when                   (Notify )
		:to                     (window1 time_delta_ntx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (time_delta_proc)
		)
	)
)
(
	:type                   :message
	:name                   min_msg
	:owner                  controls1
	:help                   ""
	:x                      146
	:y                      82
	:width                  29
	:height                 13
	:foreground-color       ""
	:label                  "Min."
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   Instruct_msg
	:owner                  controls1
	:help                   ""
	:x                      224
	:y                      84
	:width                  335
	:height                 13
	:foreground-color       ""
	:label                  "Get Products valid at Interest time +/- N minutes"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :setting
	:name                   req_typ_st
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      104
	:width                  551
	:height                 23
	:value-x                74
	:value-y                104
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Request:"
	:label-type             :string
	:notify-handler         reqest_type_proc
	:event-handler          nil
	:choices                ("Exact" "Closest" "Interval" "Valid at" "Latest" "1st Before" "1st After" "Times" )
	:choice-label-types     (:string :string :string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" "" "" )
	:initial-selections     (nil nil nil nil nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (window1 req_typ_st)
		:when                   (Notify )
		:to                     (window1 req_typ_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (reqest_type_proc)
		)
	)
)
(
	:type                   :button
	:name                   clear_bt
	:owner                  controls1
	:help                   ""
	:x                      574
	:y                      106
	:width                  87
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "CLEAR Text"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         clear_text_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 clear_bt)
		:when                   (Notify )
		:to                     (window1 clear_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (clear_text_proc)
		)
	)
)
(
	:type                   :text-pane
	:name                   textpane1
	:owner                  window1
	:help                   ""
	:x                      0
	:y                      137
	:width                  683
	:height                 618
	:background-color       "white"
	:foreground-color       "black"
	:initial-state          :visible
	:show-border            t
	:read-only              nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
)
