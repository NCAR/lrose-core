;GIL-3
(
(
	:type                   :base-window
	:name                   window1
	:owner                  nil
	:width                  653
	:height                 763
	:background-color       "Gray"
	:foreground-color       ""
	:label                  "Color Scale Editor"
	:label-type             :string
	:initial-state          :open
	:show-footer            nil
	:resizable              t
	:icon-file              "cscale.icon"
	:icon-label             ""
	:icon-mask-file         ""
	:event-handler          win_event_proc
	:user-data              ()
	:actions                (
		(
		:from                   (window1)
		:when                   (AnyEvent )
		:to                     (window1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (win_event_proc)
		)
	)
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  window1
	:help                   ""
	:x                      0
	:y                      0
	:width                  653
	:height                 116
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
	:name                   read_file_button
	:owner                  controls1
	:help                   "Pops up a window to get the directory and filename to be read in from disk."
	:x                      8
	:y                      8
	:width                  85
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Blue"
	:label                  "Read file... "
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         read_file
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 read_file_button)
		:when                   (Notify )
		:to                     (window1 read_file_button)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (read_file)
		)
	)
)
(
	:type                   :button
	:name                   delete_button
	:owner                  controls1
	:help                   "Deletes the highlighted field."
	:x                      115
	:y                      8
	:width                  84
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "blue"
	:label                  "Del Field   "
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         del_field_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 delete_button)
		:when                   (Notify )
		:to                     (window1 delete_button)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (del_field_proc)
		)
	)
)
(
	:type                   :button
	:name                   insert_above
	:owner                  controls1
	:help                   "Inserts a new field above the highlighted field."
	:x                      220
	:y                      8
	:width                  94
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "blue"
	:label                  "Insert Above"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         insert_above_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 insert_above)
		:when                   (Notify )
		:to                     (window1 insert_above)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (insert_above_proc)
		)
	)
)
(
	:type                   :button
	:name                   insert_below
	:owner                  controls1
	:help                   "Inserts a new field below the highlighted field."
	:x                      325
	:y                      8
	:width                  93
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Blue"
	:label                  "Insert Below"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         insert_below_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 insert_below)
		:when                   (Notify )
		:to                     (window1 insert_below)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (insert_below_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   fields
	:owner                  controls1
	:help                   "In a range of one to sixty-four.  Can either enter the number of fields by typing it in or by left clicking on the up or down buttons."
	:x                      440
	:y                      8
	:width                  139
	:height                 15
	:value-x                508
	:value-y                8
	:value-length           2
	:stored-length          2
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "# of flds:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              64
	:min-value              1
	:initial-value          0
	:initial-state          :active
	:read-only              nil
	:notify-handler         fields_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 fields)
		:when                   (Notify )
		:to                     (window1 fields)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (fields_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   max
	:owner                  controls1
	:help                   "Assigns the value entered to the maximum value of the highlighted field and the minimum value of the field above the highlighted field."
	:x                      440
	:y                      28
	:width                  118
	:height                 15
	:value-x                478
	:value-y                28
	:value-length           10
	:stored-length          10
	:rows                   3
	:foreground-color       "Black"
	:text-type              :alphanumeric
	:label                  "Max:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         max_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 max)
		:when                   (Notify )
		:to                     (window1 max)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (max_proc)
		)
	)
)
(
	:type                   :button
	:name                   write_file_button
	:owner                  controls1
	:help                   "Pops up a window to get the directory and filename of a file to be written out to disk."
	:x                      8
	:y                      35
	:width                  85
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "blue"
	:label                  "Write File..."
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         write_file
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 write_file_button)
		:when                   (Notify )
		:to                     (window1 write_file_button)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (write_file)
		)
	)
)
(
	:type                   :button
	:name                   sort_button
	:owner                  controls1
	:help                   "Sorts the colors on the lefthand canvas in order of the closest ones to the highlighted field on the righthand canvas."
	:x                      115
	:y                      35
	:width                  84
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "blue"
	:label                  "Sort Colors"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         sort_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 sort_button)
		:when                   (Notify )
		:to                     (window1 sort_button)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (sort_proc)
		)
	)
)
(
	:type                   :button
	:name                   quit_button
	:owner                  controls1
	:help                   "Exits the Color Scale Editor."
	:x                      592
	:y                      35
	:width                  45
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "red"
	:label                  "Quit"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         quit_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 quit_button)
		:when                   (Notify )
		:to                     (window1 quit_button)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (quit_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   min
	:owner                  controls1
	:help                   "Assigns the value entered to the minimum value of the highlighted field and the maximum value of the field below the highlighted field."
	:x                      440
	:y                      47
	:width                  117
	:height                 15
	:value-x                477
	:value-y                47
	:value-length           10
	:stored-length          10
	:rows                   3
	:foreground-color       "Black"
	:text-type              :alphanumeric
	:label                  "Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         min_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 min)
		:when                   (Notify )
		:to                     (window1 min)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (min_proc)
		)
	)
)
(
	:type                   :slider
	:name                   slider1
	:owner                  controls1
	:help                   "Allows one to view the entire file of possible RGB colors."
	:x                      235
	:y                      55
	:width                  159
	:height                 20
	:value-x                235
	:value-y                55
	:slider-width           100
	:ticks                  0
	:foreground-color       ""
	:label                  ""
	:label-type             :string
	:layout-type            :horizontal
	:orientation            :horizontal
	:show-endboxes          t
	:show-range             t
	:show-value             nil
	:min-value              0
	:max-value              5
	:min-value-string       ""
	:max-value-string       ""
	:min-tick-string        ""
	:max-tick-string        ""
	:initial-value          0
	:initial-state          :active
	:notify-handler         slide_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 slider1)
		:when                   (Notify )
		:to                     (window1 slider1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (slide_proc)
		)
	)
)
(
	:type                   :button
	:name                   value_setting
	:owner                  controls1
	:help                   "Pops up a window to allow one to easily create color scales with minimum and maximum values.  This window has a linear setting capability and an arbitrary setting capability."
	:x                      8
	:y                      63
	:width                  83
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "blue"
	:label                  "Values...    "
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         value_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 value_setting)
		:when                   (Notify )
		:to                     (window1 value_setting)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (value_proc)
		)
	)
)
(
	:type                   :button
	:name                   clear_button
	:owner                  controls1
	:help                   "Clears the color scale."
	:x                      115
	:y                      63
	:width                  82
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "Blue"
	:label                  "Clear        "
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         clear_button_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 clear_button)
		:when                   (Notify )
		:to                     (window1 clear_button)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (clear_button_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   color_label
	:owner                  controls1
	:help                   "Used for labeling what the various colors represent."
	:x                      440
	:y                      67
	:width                  193
	:height                 15
	:value-x                489
	:value-y                67
	:value-length           18
	:stored-length          18
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Label:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         label_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 color_label)
		:when                   (Notify )
		:to                     (window1 color_label)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (label_proc)
		)
	)
)
(
	:type                   :message
	:name                   color_message
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      96
	:width                  56
	:height                 13
	:foreground-color       ""
	:label                  "Color is:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   status_message
	:owner                  controls1
	:help                   "Displays a message for reading and writing files."
	:x                      235
	:y                      96
	:width                  46
	:height                 13
	:foreground-color       ""
	:label                  "Status:"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :canvas-pane
	:name                   canvas1
	:owner                  window1
	:help                   "RGB colors to be used to create color scales.  
Click on the left mouse button to copy a color onto the color scale in the highlighted field.  A click on the right mouse button just causes the color to be displayed on the top of the canvas, but does not copy the color to the color scale."
	:x                      0
	:y                      116
	:width                  468
	:height                 644
	:background-color       "Medium Slate Blue"
	:foreground-color       "white"
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       468
	:vertical-scrollbar     nil
	:scrollable-height      644
	:repaint-proc           can_repaint_proc
	:event-handler          can_event_proc
	:drawing-model          :xwindows
	:user-data              ()
	:actions                (
		(
		:from                   (window1 canvas1)
		:when                   (Repaint )
		:to                     (window1 canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (can_repaint_proc)
		)
		(
		:from                   (window1 canvas1)
		:when                   (AnyEvent )
		:to                     (window1 canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (can_event_proc)
		)
	)
)
(
	:type                   :canvas-pane
	:name                   canvas2
	:owner                  window1
	:help                   "Area for creating color scales or reading in old color scales in order to make changes.  Click on the left mouse button to highlight a field."
	:x                      479
	:y                      116
	:width                  174
	:height                 646
	:background-color       "Medium Slate Blue"
	:foreground-color       "White"
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       174
	:vertical-scrollbar     nil
	:scrollable-height      646
	:repaint-proc           can2_repaint_proc
	:event-handler          can2_event_proc
	:drawing-model          :xwindows
	:user-data              ()
	:actions                (
		(
		:from                   (window1 canvas2)
		:when                   (Repaint )
		:to                     (window1 canvas2)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (can2_repaint_proc)
		)
		(
		:from                   (window1 canvas2)
		:when                   (AnyEvent )
		:to                     (window1 canvas2)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (can2_event_proc)
		)
	)
)
)
