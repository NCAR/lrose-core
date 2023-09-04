;GIL-3
(
(
	:type                   :popup-window
	:name                   data_pu
	:owner                  nil
	:width                  191
	:height                 158
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
	:name                   data_pu_cp
	:owner                  data_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  191
	:height                 158
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
	:type                   :scrolling-list
	:name                   group_list
	:owner                  data_pu_cp
	:help                   ""
	:x                      0
	:y                      4
	:width                  150
	:height                 127
	:value-x                0
	:value-y                21
	:rows                   5
	:foreground-color       ""
	:label                  "Data Groups"
	:title                  ""
	:label-type             :string
	:layout-type            :vertical
	:read-only              nil
	:multiple-selections    t
	:selection-required     nil
	:initial-state          :invisible
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:notify-handler         set_group_proc
	:event-handler          nil
	:initial-list-values    ()
	:initial-list-glyphs    ()
	:initial-selections     ()
	:user-data              ()
	:actions                (
		(
		:from                   (data_pu group_list)
		:when                   (Notify )
		:to                     (data_pu group_list)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_group_proc)
		)
	)
)
(
	:type                   :setting
	:name                   data_st
	:owner                  data_pu_cp
	:help                   ""
	:x                      4
	:y                      106
	:width                  16
	:height                 23
	:value-x                4
	:value-y                106
	:rows                   0
	:columns                1
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         set_data_proc
	:event-handler          nil
	:choices                ()
	:choice-label-types     ()
	:choice-colors          ()
	:initial-selections     ()
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (data_pu data_st)
		:when                   (Notify )
		:to                     (data_pu data_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_data_proc)
		)
	)
)
)
