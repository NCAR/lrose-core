;GIL-3
(
(
	:type                   :menu
	:name                   v_field_mu
	:help                   ""
	:columns                1
	:menu-type              :exclusive
	:menu-handler           v_field_mu_gen_proc
	:menu-title             "Data Field"
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
		:from                   (v_field_mu)
		:when                   (Notify )
		:to                     (v_field_mu)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (v_field_mu_gen_proc)
		)
	)
)
(
	:type                   :popup-window
	:name                   v_win_pu
	:owner                  nil
	:width                  435
	:height                 154
	:background-color       ""
	:foreground-color       ""
	:label                  "RHI"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            nil
	:resizable              t
	:pinned                 nil
	:done-handler           nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :canvas-pane
	:name                   canvas1
	:owner                  v_win_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  435
	:height                 154
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   v_field_mu
	:horizontal-scrollbar   nil
	:scrollable-width       435
	:vertical-scrollbar     nil
	:scrollable-height      154
	:repaint-proc           v_can_repaint
	:event-handler          v_can_events
	:drawing-model          :xwindows
	:user-data              ()
	:actions                (
		(
		:from                   (v_win_pu canvas1)
		:when                   (Repaint )
		:to                     (v_win_pu canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (v_can_repaint)
		)
		(
		:from                   (v_win_pu canvas1)
		:when                   (AnyEvent )
		:to                     (v_win_pu canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (v_can_events)
		)
	)
)
)
