;GIL-3
(
(
	:type                   :base-window
	:name                   em_bw
	:owner                  nil
	:width                  537
	:height                 449
	:background-color       ""
	:foreground-color       ""
	:label                  "Event Manager"
	:label-type             :string
	:initial-state          :open
	:show-footer            nil
	:resizable              nil
	:icon-file              ""
	:icon-label             ""
	:icon-mask-file         ""
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   em_cp
	:owner                  em_bw
	:help                   ""
	:x                      0
	:y                      0
	:width                  537
	:height                 271
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
	:name                   goto_bt
	:owner                  em_cp
	:help                   ""
	:x                      10
	:y                      8
	:width                  114
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Cornflower Blue"
	:label                  "GO TO Event Time"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         goto_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw goto_bt)
		:when                   (Notify )
		:to                     (em_bw goto_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (goto_proc)
		)
	)
)
(
	:type                   :button
	:name                   rt_bt
	:owner                  em_cp
	:help                   ""
	:x                      302
	:y                      8
	:width                  90
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Cornflower Blue"
	:label                  "GO REAL-TIME"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         gort_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw rt_bt)
		:when                   (Notify )
		:to                     (em_bw rt_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gort_proc)
		)
	)
)
(
	:type                   :button
	:name                   help_bt
	:owner                  em_cp
	:help                   ""
	:x                      428
	:y                      8
	:width                  42
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Help"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         help_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw help_bt)
		:when                   (Notify )
		:to                     (em_bw help_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (help_proc)
		)
	)
)
(
	:type                   :button
	:name                   close_bt
	:owner                  em_cp
	:help                   ""
	:x                      480
	:y                      8
	:width                  48
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Close"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         close_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw close_bt)
		:when                   (Notify )
		:to                     (em_bw close_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (close_proc)
		)
	)
)
(
	:type                   :scrolling-list
	:name                   event_list
	:owner                  em_cp
	:help                   ""
	:x                      268
	:y                      38
	:width                  250
	:height                 224
	:value-x                268
	:value-y                38
	:rows                   10
	:foreground-color       ""
	:label                  ""
	:title                  "Events"
	:label-type             :string
	:layout-type            :vertical
	:read-only              nil
	:multiple-selections    nil
	:selection-required     nil
	:initial-state          :active
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:notify-handler         ev_list_proc
	:event-handler          nil
	:initial-list-values    ()
	:initial-list-glyphs    ()
	:initial-selections     ()
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw event_list)
		:when                   (Notify )
		:to                     (em_bw event_list)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (ev_list_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   fname_tx
	:owner                  em_cp
	:help                   ""
	:x                      8
	:y                      40
	:width                  240
	:height                 32
	:value-x                8
	:value-y                57
	:value-length           40
	:stored-length          1024
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Event File:"
	:label-type             :string
	:layout-type            :vertical
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         load_file_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw fname_tx)
		:when                   (Notify )
		:to                     (em_bw fname_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (load_file_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   summary_tx
	:owner                  em_cp
	:help                   ""
	:x                      8
	:y                      86
	:width                  188
	:height                 15
	:value-x                52
	:value-y                86
	:value-length           24
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Title:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         summary_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw summary_tx)
		:when                   (Notify )
		:to                     (em_bw summary_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (summary_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   date_tx
	:owner                  em_cp
	:help                   ""
	:x                      8
	:y                      118
	:width                  188
	:height                 15
	:value-x                52
	:value-y                118
	:value-length           24
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Start:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         start_time_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw date_tx)
		:when                   (Notify )
		:to                     (em_bw date_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (start_time_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   end_time_tx
	:owner                  em_cp
	:help                   ""
	:x                      8
	:y                      148
	:width                  182
	:height                 15
	:value-x                46
	:value-y                148
	:value-length           24
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "End: "
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         end_time_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw end_time_tx)
		:when                   (Notify )
		:to                     (em_bw end_time_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (end_time_proc)
		)
	)
)
(
	:type                   :button
	:name                   add_bt
	:owner                  em_cp
	:help                   ""
	:x                      8
	:y                      182
	:width                  72
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Sea Green"
	:label                  "New Event"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         add_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw add_bt)
		:when                   (Notify )
		:to                     (em_bw add_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (add_proc)
		)
	)
)
(
	:type                   :button
	:name                   del_bt
	:owner                  em_cp
	:help                   ""
	:x                      158
	:y                      182
	:width                  90
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Red"
	:label                  "Delete Event"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         del_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw del_bt)
		:when                   (Notify )
		:to                     (em_bw del_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (del_proc)
		)
	)
)
(
	:type                   :button
	:name                   save_bt
	:owner                  em_cp
	:help                   ""
	:x                      8
	:y                      220
	:width                  90
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Red"
	:label                  "Save Changes"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         save_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw save_bt)
		:when                   (Notify )
		:to                     (em_bw save_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (save_proc)
		)
	)
)
(
	:type                   :button
	:name                   mailto_bt
	:owner                  em_cp
	:help                   ""
	:x                      168
	:y                      220
	:width                  78
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Mail to..."
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         mail_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw mailto_bt)
		:when                   (Notify )
		:to                     (em_bw mailto_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (mail_proc)
		)
	)
)
(
	:type                   :message
	:name                   msg1
	:owner                  em_cp
	:help                   ""
	:x                      160
	:y                      248
	:width                  78
	:height                 13
	:foreground-color       ""
	:label                  "Event  Notes:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-pane
	:name                   em_tp
	:owner                  em_bw
	:help                   ""
	:x                      0
	:y                      271
	:width                  537
	:height                 178
	:background-color       "White"
	:foreground-color       "Black"
	:initial-state          :visible
	:show-border            t
	:read-only              nil
	:event-handler          comment_text_proc
	:user-data              ()
	:actions                (
		(
		:from                   (em_bw em_tp)
		:when                   (AnyEvent )
		:to                     (em_bw em_tp)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (comment_text_proc)
		)
	)
)
)
