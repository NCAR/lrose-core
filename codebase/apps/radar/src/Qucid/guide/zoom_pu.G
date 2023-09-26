;GIL-3
(
(
	:type                   :popup-window
	:name                   zoom_pu
	:owner                  nil
	:width                  137
	:height                 151
	:background-color       ""
	:foreground-color       ""
	:label                  " "
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
	:name                   controls1
	:owner                  zoom_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  137
	:height                 151
	:background-color       "Gray"
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
	:name                   domain_st
	:owner                  controls1
	:help                   ""
	:x                      4
	:y                      0
	:width                  16
	:height                 23
	:value-x                4
	:value-y                0
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         set_domain_proc
	:event-handler          nil
	:choices                ()
	:choice-label-types     ()
	:choice-colors          ()
	:initial-selections     ()
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (zoom_pu domain_st)
		:when                   (Notify )
		:to                     (zoom_pu domain_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_domain_proc)
		)
	)
)
(
	:type                   :popup-window
	:name                   bookmk_pu
	:owner                  nil
	:width                  108
	:height                 152
	:background-color       ""
	:foreground-color       ""
	:label                  "Bookmarks"
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
	:name                   controls2
	:owner                  bookmk_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  108
	:height                 152
	:background-color       "Gray"
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
	:name                   bookmk_st
	:owner                  controls2
	:help                   ""
	:x                      0
	:y                      0
	:width                  16
	:height                 23
	:value-x                0
	:value-y                0
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         bookmark_proc
	:event-handler          nil
	:choices                ()
	:choice-label-types     ()
	:choice-colors          ()
	:initial-selections     ()
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (bookmk_pu bookmk_st)
		:when                   (Notify )
		:to                     (bookmk_pu bookmk_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (bookmark_proc)
		)
	)
)
)
