;GIL-3
(
(
	:type                   :popup-window
	:name                   popup1
	:owner                  nil
	:width                  375
	:height                 145
	:background-color       ""
	:foreground-color       ""
	:label                  "Color Scale Editor: Write File"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            t
	:resizable              nil
	:pinned                 nil
	:done-handler           write_file_done
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup1)
		:when                   (Done )
		:to                     (popup1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (write_file_done)
		)
	)
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  popup1
	:help                   ""
	:x                      0
	:y                      0
	:width                  375
	:height                 145
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
	:type                   :text-field
	:name                   popup_directory
	:owner                  controls1
	:help                   "The directory where the file will be written."
	:x                      8
	:y                      12
	:width                  354
	:height                 15
	:value-x                82
	:value-y                12
	:value-length           35
	:stored-length          35
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Directory:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          write_dir_events
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 popup_directory)
		:when                   (AnyEvent )
		:to                     (popup1 popup_directory)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (write_dir_events)
		)
	)
)
(
	:type                   :text-field
	:name                   popup_filename
	:owner                  controls1
	:help                   "The name of the file to be written to disk."
	:x                      8
	:y                      44
	:width                  354
	:height                 15
	:value-x                82
	:value-y                44
	:value-length           35
	:stored-length          35
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Filename:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          write_filename_events
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 popup_filename)
		:when                   (AnyEvent )
		:to                     (popup1 popup_filename)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (write_filename_events)
		)
	)
)
(
	:type                   :setting
	:name                   popup_format
	:owner                  controls1
	:help                   "The type of format to write the file in.  There are three choices, namely: name, rgb, or hexval."
	:x                      8
	:y                      80
	:width                  206
	:height                 23
	:value-x                66
	:value-y                80
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Format:"
	:label-type             :string
	:notify-handler         write_format_proc
	:event-handler          nil
	:choices                ("Name" "RGB" "Hexval" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("blue" "blue" "Blue" )
	:initial-selections     (nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 popup_format)
		:when                   (Notify )
		:to                     (popup1 popup_format)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (write_format_proc)
		)
	)
)
(
	:type                   :button
	:name                   popup_write_button
	:owner                  controls1
	:help                   "Causes the file to be written to disk."
	:x                      176
	:y                      120
	:width                  50
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Red"
	:label                  "Write"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         write_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 popup_write_button)
		:when                   (Notify )
		:to                     (popup1 popup_write_button)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (write_proc)
		)
	)
)
)
