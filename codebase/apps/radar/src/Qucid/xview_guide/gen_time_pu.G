;GIL-3
(
(
	:type                   :popup-window
	:name                   popup1
	:owner                  nil
	:width                  92
	:height                 107
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
	:owner                  popup1
	:help                   ""
	:x                      0
	:y                      0
	:width                  92
	:height                 107
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
	:name                   gen_time_st
	:owner                  controls1
	:help                   ""
	:x                      4
	:y                      4
	:width                  52
	:height                 23
	:value-x                4
	:value-y                4
	:rows                   0
	:columns                1
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         gen_time_proc
	:event-handler          nil
	:choices                ("Latest" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 gen_time_st)
		:when                   (Notify )
		:to                     (popup1 gen_time_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gen_time_proc)
		)
	)
)
)
