;GIL-3
(
(
	:type                   :base-window
	:name                   window1
	:owner                  nil
	:width                  170
	:height                 128
	:background-color       ""
	:foreground-color       ""
	:label                  "Base Window"
	:label-type             :string
	:initial-state          :open
	:show-footer            t
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
	:width                  170
	:height                 128
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          control_panel_proc
	:user-data              ()
	:actions                (
		(
		:from                   (window1 controls1)
		:when                   (AnyEvent )
		:to                     (window1 controls1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (control_panel_proc)
		)
	)
)
(
	:type                   :button
	:name                   button1
	:owner                  controls1
	:help                   ""
	:x                      56
	:y                      56
	:width                  49
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "QUIT"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         quit_bt_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 button1)
		:when                   (Notify )
		:to                     (window1 button1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (quit_bt_proc)
		)
	)
)
)
