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
	:label                  "Color Scale Editor: Read File"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            t
	:resizable              nil
	:pinned                 nil
	:done-handler           read_file_done
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup1)
		:when                   (Done )
		:to                     (popup1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (read_file_done)
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
	:name                   read_directory
	:owner                  controls1
	:help                   "The directory where the file to be read in resides."
	:x                      12
	:y                      12
	:width                  354
	:height                 15
	:value-x                86
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
	:event-handler          read_dir_events
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 read_directory)
		:when                   (AnyEvent )
		:to                     (popup1 read_directory)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (read_dir_events)
		)
	)
)
(
	:type                   :text-field
	:name                   read_filename
	:owner                  controls1
	:help                   "The name of the file to be read in."
	:x                      12
	:y                      40
	:width                  354
	:height                 15
	:value-x                86
	:value-y                40
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
	:event-handler          read_filename_events
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 read_filename)
		:when                   (AnyEvent )
		:to                     (popup1 read_filename)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (read_filename_events)
		)
	)
)
(
	:type                   :setting
	:name                   read_format
	:owner                  controls1
	:help                   "The type of format the file is in.  There are three choices of name, rgb, or hexval."
	:x                      12
	:y                      64
	:width                  206
	:height                 23
	:value-x                70
	:value-y                64
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Format:"
	:label-type             :string
	:notify-handler         read_format_proc
	:event-handler          nil
	:choices                ("Name" "RGB" "Hexval" )
	:choice-label-types     (:string :string :string )
	:choice-colors          ("blue" "blue" "blue" )
	:initial-selections     (nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 read_format)
		:when                   (Notify )
		:to                     (popup1 read_format)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (read_format_proc)
		)
	)
)
(
	:type                   :button
	:name                   read_button
	:owner                  controls1
	:help                   "Causes the file named by filename to be read in."
	:x                      172
	:y                      116
	:width                  48
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "red"
	:label                  "Read"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         read_button_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 read_button)
		:when                   (Notify )
		:to                     (popup1 read_button)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (read_button_proc)
		)
	)
)
)
