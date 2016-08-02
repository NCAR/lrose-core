;GIL-3
(
(
	:type                   :base-window
	:name                   window1
	:owner                  nil
	:width                  260
	:height                 288
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
	:width                  260
	:height                 32
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
	:name                   button1
	:owner                  controls1
	:help                   ""
	:x                      18
	:y                      8
	:width                  37
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "GO"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         go_func
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 button1)
		:when                   (Notify )
		:to                     (window1 button1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (go_func)
		)
	)
)
(
	:type                   :canvas-pane
	:name                   canvas1
	:owner                  window1
	:help                   ""
	:x                      0
	:y                      32
	:width                  256
	:height                 256
	:background-color       "Black"
	:foreground-color       "White"
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       256
	:vertical-scrollbar     nil
	:scrollable-height      256
	:repaint-proc           nil
	:event-handler          nil
	:drawing-model          :xwindows
	:user-data              ()
	:actions                ()
)
)
