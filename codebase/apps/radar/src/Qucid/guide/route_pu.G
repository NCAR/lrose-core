;GIL-3
(
(
	:type                   :popup-window
	:name                   route_pu
	:owner                  nil
	:width                  50
	:height                 31
	:background-color       ""
	:foreground-color       ""
	:label                  "Popup Window"
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
	:owner                  route_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  50
	:height                 31
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
	:name                   route_st
	:owner                  controls1
	:help                   ""
	:x                      2
	:y                      4
	:width                  44
	:height                 23
	:value-x                2
	:value-y                4
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         set_route_proc
	:event-handler          nil
	:choices                ("Item" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (route_pu route_st)
		:when                   (Notify )
		:to                     (route_pu route_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_route_proc)
		)
	)
)
)
