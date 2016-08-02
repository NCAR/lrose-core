;GIL-3
(
(
	:type                   :popup-window
	:name                   save_im_pu
	:owner                  nil
	:width                  409
	:height                 101
	:background-color       ""
	:foreground-color       ""
	:label                  "Save CIDD Image"
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
	:name                   save_cp
	:owner                  save_im_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  409
	:height                 101
	:background-color       "Wheat"
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
	:owner                  save_cp
	:help                   ""
	:x                      2
	:y                      8
	:width                  352
	:height                 15
	:value-x                34
	:value-y                8
	:value-length           40
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
	:notify-handler         set_dir_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (save_im_pu dir_tx)
		:when                   (Notify )
		:to                     (save_im_pu dir_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_dir_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   fname_tx
	:owner                  save_cp
	:help                   ""
	:x                      2
	:y                      33
	:width                  356
	:height                 15
	:value-x                38
	:value-y                33
	:value-length           40
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
	:notify-handler         set_fname_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (save_im_pu fname_tx)
		:when                   (Notify )
		:to                     (save_im_pu fname_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_fname_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   cmd_tx
	:owner                  save_cp
	:help                   ""
	:x                      2
	:y                      58
	:width                  399
	:height                 15
	:value-x                81
	:value-y                58
	:value-length           40
	:stored-length          256
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Command:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_command_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (save_im_pu cmd_tx)
		:when                   (Notify )
		:to                     (save_im_pu cmd_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_command_proc)
		)
	)
)
(
	:type                   :button
	:name                   save_bt
	:owner                  save_cp
	:help                   ""
	:x                      6
	:y                      76
	:width                  46
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Save"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         save_image_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (save_im_pu save_bt)
		:when                   (Notify )
		:to                     (save_im_pu save_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (save_image_proc)
		)
	)
)
(
	:type                   :button
	:name                   cancel_bt
	:owner                  save_cp
	:help                   ""
	:x                      340
	:y                      78
	:width                  59
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Cancel"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         cancel_save_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (save_im_pu cancel_bt)
		:when                   (Notify )
		:to                     (save_im_pu cancel_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cancel_save_proc)
		)
	)
)
(
	:type                   :message
	:name                   save_pu_msg
	:owner                  save_cp
	:help                   ""
	:x                      58
	:y                      80
	:width                  210
	:height                 13
	:foreground-color       ""
	:label                  "Window Information goes here"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
)
