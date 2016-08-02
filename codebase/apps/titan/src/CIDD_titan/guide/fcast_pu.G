;GIL-3
(
(
	:type                   :popup-window
	:name                   fcast_pu
	:owner                  nil
	:width                  92
	:height                 125
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
	:name                   cp
	:owner                  fcast_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  92
	:height                 125
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
	:name                   fcast_st
	:owner                  cp
	:help                   ""
	:x                      -2
	:y                      2
	:width                  94
	:height                 23
	:value-x                -2
	:value-y                2
	:rows                   0
	:columns                1
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         set_fcast_tm_proc
	:event-handler          nil
	:choices                ("OFF - (NOW)" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (fcast_pu fcast_st)
		:when                   (Notify )
		:to                     (fcast_pu fcast_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_fcast_tm_proc)
		)
	)
)
)
