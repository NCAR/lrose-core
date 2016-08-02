;GIL-3
(
(
	:type                   :base-window
	:name                   win1
	:owner                  nil
	:width                  266
	:height                 114
	:background-color       ""
	:foreground-color       ""
	:label                  "Registration Tool"
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
	:name                   controls1
	:owner                  win1
	:help                   ""
	:x                      0
	:y                      0
	:width                  266
	:height                 114
	:background-color       "Dim Gray"
	:foreground-color       "White"
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   start_bt
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      16
	:width                  47
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Start"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         start_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (win1 start_bt)
		:when                   (Notify )
		:to                     (win1 start_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (start_proc)
		)
	)
)
(
	:type                   :button
	:name                   comp_bt
	:owner                  controls1
	:help                   ""
	:x                      68
	:y                      16
	:width                  49
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Store"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         comp_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (win1 comp_bt)
		:when                   (Notify )
		:to                     (win1 comp_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (comp_proc)
		)
	)
)
(
	:type                   :message
	:name                   status_msg
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      45
	:width                  94
	:height                 13
	:foreground-color       "White"
	:label                  "Ready to Start"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   each_pt_msg
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      68
	:width                  68
	:height                 13
	:foreground-color       "White"
	:label                  "No Points."
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   avg_msg
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      91
	:width                  144
	:height                 13
	:foreground-color       "White"
	:label                  "X:               Y:               "
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
)
