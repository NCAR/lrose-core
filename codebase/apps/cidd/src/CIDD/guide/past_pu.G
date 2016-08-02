;GIL-3
(
(
	:type                   :popup-window
	:name                   past_pu
	:owner                  nil
	:width                  78
	:height                 65
	:background-color       ""
	:foreground-color       ""
	:label                  "Past"
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
	:owner                  past_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  78
	:height                 65
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
	:name                   past_hr_st
	:owner                  controls1
	:help                   ""
	:x                      4
	:y                      6
	:width                  16
	:height                 23
	:value-x                4
	:value-y                6
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         past_hr_proc
	:event-handler          nil
	:choices                ()
	:choice-label-types     ()
	:choice-colors          ()
	:initial-selections     ()
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (past_pu past_hr_st)
		:when                   (Notify )
		:to                     (past_pu past_hr_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (past_hr_proc)
		)
	)
)
)
