;GIL-3
(
(
	:type                   :popup-window
	:name                   popup1
	:owner                  nil
	:width                  221
	:height                 90
	:background-color       ""
	:foreground-color       ""
	:label                  "Save CLASS File"
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
	:width                  221
	:height                 90
	:background-color       "Light Steel Blue"
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   dir_tx
	:owner                  controls1
	:help                   ""
	:x                      15
	:y                      12
	:width                  192
	:height                 15
	:value-x                47
	:value-y                12
	:value-length           20
	:stored-length          256
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Dir:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_dir
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 dir_tx)
		:when                   (Notify )
		:to                     (popup1 dir_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_dir)
		)
	)
)
(
	:type                   :text-field
	:name                   fname_tx
	:owner                  controls1
	:help                   ""
	:x                      13
	:y                      37
	:width                  196
	:height                 15
	:value-x                49
	:value-y                37
	:value-length           20
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "File:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_fname
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 fname_tx)
		:when                   (Notify )
		:to                     (popup1 fname_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_fname)
		)
	)
)
(
	:type                   :button
	:name                   load_pu_bt
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      62
	:width                  199
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Generate and Save CLASS File"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         ld_sv_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 load_pu_bt)
		:when                   (Notify )
		:to                     (popup1 load_pu_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (ld_sv_proc)
		)
	)
)
)
