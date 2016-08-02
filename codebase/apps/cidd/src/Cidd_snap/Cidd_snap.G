;GIL-3
(
(
	:type                   :base-window
	:name                   window1
	:owner                  nil
	:width                  465
	:height                 405
	:background-color       ""
	:foreground-color       ""
	:label                  "CIDD Snapshot"
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
	:width                  465
	:height                 25
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
	:type                   :message
	:name                   msg1
	:owner                  controls1
	:help                   ""
	:x                      18
	:y                      6
	:width                  60
	:height                 13
	:foreground-color       ""
	:label                  "Message"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :canvas-pane
	:name                   canvas1
	:owner                  window1
	:help                   ""
	:x                      0
	:y                      25
	:width                  465
	:height                 380
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   t
	:scrollable-width       465
	:vertical-scrollbar     t
	:scrollable-height      380
	:repaint-proc           nil
	:event-handler          can_event_func
	:drawing-model          :xwindows
	:user-data              ()
	:actions                (
		(
		:from                   (window1 canvas1)
		:when                   (Resize )
		:to                     (window1 canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (resize_can_proc)
		)
		(
		:from                   (window1 canvas1)
		:when                   (AnyEvent )
		:to                     (window1 canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (can_event_func)
		)
	)
)
)
