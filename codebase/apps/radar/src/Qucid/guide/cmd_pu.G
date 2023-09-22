;GIL-3
(
(
	:type                   :popup-window
	:name                   cmd_pu
	:owner                  nil
	:width                  79
	:height                 71
	:background-color       ""
	:foreground-color       ""
	:label                  "Command Menu"
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
	:owner                  cmd_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  79
	:height                 71
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
	:name                   cmd_st
	:owner                  controls1
	:help                   ""
	:x                      0
	:y                      4
	:width                  44
	:height                 23
	:value-x                0
	:value-y                4
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         cmd_proc
	:event-handler          nil
	:choices                ("Item" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (cmd_pu cmd_st)
		:when                   (Notify )
		:to                     (cmd_pu cmd_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cmd_proc)
		)
	)
)
)
