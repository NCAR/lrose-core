;GIL-3
(
(
	:type                   :popup-window
	:name                   prod_pu
	:owner                  nil
	:width                  218
	:height                 237
	:background-color       ""
	:foreground-color       ""
	:label                  "Products"
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
	:owner                  prod_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  218
	:height                 237
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
	:name                   prod_st
	:owner                  controls1
	:help                   ""
	:x                      2
	:y                      4
	:width                  92
	:height                 23
	:value-x                2
	:value-y                4
	:rows                   0
	:columns                1
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :nonexclusive
	:selection-required     t
	:label                  ""
	:label-type             :string
	:notify-handler         prod_select_proc
	:event-handler          nil
	:choices                ("NO Products" )
	:choice-label-types     (:string )
	:choice-colors          ("" )
	:initial-selections     (nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (prod_pu prod_st)
		:when                   (Notify )
		:to                     (prod_pu prod_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_select_proc)
		)
	)
)
(
	:type                   :scrolling-list
	:name                   prod_lst
	:owner                  controls1
	:help                   ""
	:x                      0
	:y                      28
	:width                  200
	:height                 200
	:value-x                0
	:value-y                28
	:rows                   10
	:foreground-color       ""
	:label                  ""
	:title                  ""
	:label-type             :string
	:layout-type            :vertical
	:read-only              t
	:multiple-selections    t
	:selection-required     nil
	:initial-state          :invisible
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:notify-handler         prod_list_proc
	:event-handler          nil
	:initial-list-values    ()
	:initial-list-glyphs    ()
	:initial-selections     ()
	:user-data              ()
	:actions                (
		(
		:from                   (prod_pu prod_lst)
		:when                   (Notify )
		:to                     (prod_pu prod_lst)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (prod_list_proc)
		)
	)
)
)
