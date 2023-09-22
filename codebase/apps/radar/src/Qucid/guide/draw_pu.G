;GIL-3
(
(
	:type                   :popup-window
	:name                   draw_pu
	:owner                  nil
	:width                  378
	:height                 130
	:background-color       ""
	:foreground-color       ""
	:label                  "Drawn Products"
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
	:name                   controls2
	:owner                  draw_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  378
	:height                 130
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
	:name                   type_st
	:owner                  controls2
	:help                   ""
	:x                      14
	:y                      6
	:width                  165
	:height                 23
	:value-x                108
	:value-y                6
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :stack
	:selection-required     t
	:label                  "Select  Type:"
	:label-type             :string
	:notify-handler         select_draw_type_proc
	:event-handler          nil
	:choices                ("stub1" "stub2" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:choice-defaults        (nil nil )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (draw_pu type_st)
		:when                   (Notify )
		:to                     (draw_pu type_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (select_draw_type_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   serialno_tx
	:owner                  controls2
	:help                   ""
	:x                      8
	:y                      34
	:width                  130
	:height                 15
	:value-x                55
	:value-y                34
	:value-length           6
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "ID no:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              999999
	:min-value              0
	:initial-value          1
	:initial-state          :active
	:read-only              nil
	:notify-handler         draw_serialno_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (draw_pu serialno_tx)
		:when                   (Notify )
		:to                     (draw_pu serialno_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (draw_serialno_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   validtm_tx
	:owner                  controls2
	:help                   ""
	:x                      206
	:y                      34
	:width                  83
	:height                 15
	:value-x                249
	:value-y                34
	:value-length           5
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Valid"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         draw_validtm_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (draw_pu validtm_tx)
		:when                   (Notify )
		:to                     (draw_pu validtm_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (draw_validtm_proc)
		)
	)
)
(
	:type                   :message
	:name                   minute_msg
	:owner                  controls2
	:help                   ""
	:x                      312
	:y                      35
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
	:type                   :text-field
	:name                   label_tx
	:owner                  controls2
	:help                   ""
	:x                      8
	:y                      60
	:width                  161
	:height                 15
	:value-x                57
	:value-y                60
	:value-length           14
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Label:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         draw_label_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (draw_pu label_tx)
		:when                   (Notify )
		:to                     (draw_pu label_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (draw_label_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   time_tx
	:owner                  controls2
	:help                   ""
	:x                      180
	:y                      62
	:width                  190
	:height                 15
	:value-x                226
	:value-y                62
	:value-length           18
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Time:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_draw_time_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (draw_pu time_tx)
		:when                   (Notify )
		:to                     (draw_pu time_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_draw_time_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   url_tx
	:owner                  controls2
	:help                   ""
	:x                      8
	:y                      85
	:width                  358
	:height                 15
	:value-x                46
	:value-y                85
	:value-length           40
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
	:notify-handler         draw_url_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (draw_pu url_tx)
		:when                   (Notify )
		:to                     (draw_pu url_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (draw_url_proc)
		)
	)
)
(
	:type                   :button
	:name                   cancel_bt
	:owner                  controls2
	:help                   ""
	:x                      232
	:y                      106
	:width                  59
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Cancel"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         draw_cancel_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (draw_pu cancel_bt)
		:when                   (Notify )
		:to                     (draw_pu cancel_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (draw_cancel_proc)
		)
	)
)
(
	:type                   :button
	:name                   ok_bt
	:owner                  controls2
	:help                   ""
	:x                      314
	:y                      106
	:width                  52
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "  OK  "
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         draw_ok_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (draw_pu ok_bt)
		:when                   (Notify )
		:to                     (draw_pu ok_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (draw_ok_proc)
		)
	)
)
)
