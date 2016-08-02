;GIL-3
(
(
	:type                   :base-window
	:name                   window1
	:owner                  nil
	:width                  624
	:height                 727
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
	:width                  624
	:height                 102
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
	:type                   :button
	:name                   now_bt
	:owner                  controls1
	:help                   ""
	:x                      10
	:y                      6
	:width                  139
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Set to Current Time"
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
	:type                   :text-field
	:name                   source_tx
	:owner                  controls1
	:help                   ""
	:x                      300
	:y                      8
	:width                  262
	:height                 15
	:value-x                402
	:value-y                8
	:value-length           20
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Source String:"
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
	:x                      300
	:y                      26
	:width                  188
	:height                 23
	:value-x                397
	:value-y                26
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Product Type"
	:label-type             :string
	:notify-handler         prod_typ_proc
	:event-handler          nil
	:choices                ("Generic" "Standard" )
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
	:x                      300
	:y                      54
	:width                  145
	:height                 15
	:value-x                381
	:value-y                54
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Product ID:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
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
	:type                   :setting
	:name                   req_typ_st
	:owner                  controls1
	:help                   ""
	:x                      300
	:y                      74
	:width                  289
	:height                 23
	:value-x                366
	:value-y                74
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
	:choices                ("Interval" "Closest" "Before" "After" )
	:choice-label-types     (:string :string :string :string )
	:choice-colors          ("" "" "" "" )
	:initial-selections     (nil nil nil nil )
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
	:type                   :group
	:name                   group1
	:x                      300
	:y                      8
	:group-type             nil
	:members                (source_tx prod_typ_st prod_id_tx req_typ_st )
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
	:name                   end_tx
	:owner                  controls1
	:help                   ""
	:x                      10
	:y                      28
	:width                  264
	:height                 15
	:value-x                114
	:value-y                28
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
	:type                   :text-field
	:name                   time_delta_ntx
	:owner                  controls1
	:help                   ""
	:x                      10
	:y                      48
	:width                  136
	:height                 15
	:value-x                63
	:value-y                48
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
	:x                      154
	:y                      50
	:width                  56
	:height                 13
	:foreground-color       ""
	:label                  "Minutes"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   submit_bt
	:owner                  controls1
	:help                   ""
	:x                      10
	:y                      72
	:width                  116
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "SUBMIT Request"
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
	:type                   :button
	:name                   clear_bt
	:owner                  controls1
	:help                   ""
	:x                      136
	:y                      72
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
	:y                      102
	:width                  624
	:height                 625
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
