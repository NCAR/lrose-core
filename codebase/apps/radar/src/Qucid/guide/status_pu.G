;GIL-3
(
(
	:type                   :popup-window
	:name                   status_pu
	:owner                  nil
	:width                  543
	:height                 435
	:background-color       ""
	:foreground-color       ""
	:label                  "CIDD Status"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            nil
	:resizable              t
	:pinned                 t
	:done-handler           nil
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (status_pu)
		:when                   (Resize )
		:to                     (status_pu)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (status_resize_proc)
		)
	)
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  status_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  543
	:height                 435
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
	:type                   :button
	:name                   dismiss_bt
	:owner                  controls1
	:help                   ""
	:x                      466
	:y                      6
	:width                  65
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Dismiss"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         status_dismiss_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (status_pu dismiss_bt)
		:when                   (Notify )
		:to                     (status_pu dismiss_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (status_dismiss_proc)
		)
	)
)
(
	:type                   :button
	:name                   clear_bt
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      8
	:width                  50
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Clear"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         status_clear_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (status_pu clear_bt)
		:when                   (Notify )
		:to                     (status_pu clear_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (status_clear_proc)
		)
	)
)
(
	:type                   :scrolling-list
	:name                   status_list
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      36
	:width                  500
	:height                 380
	:value-x                16
	:value-y                36
	:rows                   20
	:foreground-color       ""
	:label                  ""
	:title                  ""
	:label-type             :string
	:layout-type            :vertical
	:read-only              t
	:multiple-selections    nil
	:selection-required     nil
	:initial-state          :active
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:initial-list-values    ()
	:initial-list-glyphs    ()
	:initial-selections     ()
	:user-data              ()
	:actions                ()
)
)
