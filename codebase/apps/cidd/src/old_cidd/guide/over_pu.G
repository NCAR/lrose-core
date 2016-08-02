;GIL-3
(
(
	:type                   :popup-window
	:name                   over_pu
	:owner                  nil
	:width                  115
	:height                 133
	:background-color       ""
	:foreground-color       ""
	:label                  "Maps"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            nil
	:resizable              nil
	:pinned                 t
	:done-handler           nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   over_cp
	:owner                  over_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  115
	:height                 133
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
	:name                   over_pu_st
	:owner                  over_cp
	:help                   ""
	:x                      2
	:y                      2
	:width                  16
	:height                 23
	:value-x                2
	:value-y                2
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         over_pu_proc
	:event-handler          nil
	:choices                ()
	:choice-label-types     ()
	:choice-colors          ()
	:initial-selections     ()
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (over_pu over_pu_st)
		:when                   (Notify )
		:to                     (over_pu over_pu_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (over_pu_proc)
		)
	)
)
)
