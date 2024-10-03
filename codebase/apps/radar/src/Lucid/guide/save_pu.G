;GIL-3
(
(
	:type                   :popup-window
	:name                   save_im_pu
	:owner                  nil
	:width                  496
	:height                 90
	:background-color       ""
	:foreground-color       ""
	:label                  "Save/Print  Image"
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
	:width                  496
	:height                 90
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
	:type                   :text-field
	:name                   fname_tx
	:owner                  save_cp
	:help                   ""
	:x                      4
	:y                      6
	:width                  482
	:height                 15
	:value-x                54
	:value-y                6
	:value-length           54
	:stored-length          120
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Name:"
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
	:name                   dir_tx
	:owner                  save_cp
	:help                   ""
	:x                      4
	:y                      24
	:width                  478
	:height                 15
	:value-x                82
	:value-y                24
	:value-length           50
	:stored-length          256
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Image Dir:"
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
	:name                   cmd_tx
	:owner                  save_cp
	:help                   ""
	:x                      2
	:y                      42
	:width                  483
	:height                 15
	:value-x                165
	:value-y                42
	:value-length           40
	:stored-length          256
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Post Processing Script:"
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
	:name                   gen_name_bt
	:owner                  save_cp
	:help                   ""
	:x                      4
	:y                      60
	:width                  112
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Generate Name"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         gen_fname_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (save_im_pu gen_name_bt)
		:when                   (Notify )
		:to                     (save_im_pu gen_name_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gen_fname_proc)
		)
	)
)
(
	:type                   :button
	:name                   cancel_bt
	:owner                  save_cp
	:help                   ""
	:x                      120
	:y                      60
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
	:x                      182
	:y                      64
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
(
	:type                   :button
	:name                   save_bt
	:owner                  save_cp
	:help                   ""
	:x                      440
	:y                      64
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
)
