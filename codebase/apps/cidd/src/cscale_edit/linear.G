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
	:label                  "Color Scale Editor:  Linear scale"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            t
	:resizable              t
	:pinned                 nil
	:done-handler           linear_done
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup1)
		:when                   (Done )
		:to                     (popup1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (linear_done)
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
	:name                   linear_min
	:owner                  controls1
	:help                   "Minimum value that the first field is set to.  Only used with the linear setting."
	:x                      8
	:y                      12
	:width                  85
	:height                 15
	:value-x                45
	:value-y                12
	:value-length           6
	:stored-length          6
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          lin_min_events
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 linear_min)
		:when                   (AnyEvent )
		:to                     (popup1 linear_min)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (lin_min_events)
		)
	)
)
(
	:type                   :setting
	:name                   linear_arbitrary
	:owner                  controls1
	:help                   "Exclusive choice of linear or arbitrary setting of minimum and maximum values on the color scale."
	:x                      210
	:y                      12
	:width                  123
	:height                 40
	:value-x                210
	:value-y                29
	:rows                   1
	:columns                0
	:layout-type            :vertical
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Linear/Arbitrary"
	:label-type             :string
	:notify-handler         lin_arb_proc
	:event-handler          nil
	:choices                ("Linear" "Arbitrary" )
	:choice-label-types     (:string :string )
	:choice-colors          ("" "" )
	:initial-selections     (nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 linear_arbitrary)
		:when                   (Notify )
		:to                     (popup1 linear_arbitrary)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (lin_arb_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   fields
	:owner                  controls1
	:help                   "The number of fields to be used on the color scale.  Only used with the linear setting."
	:x                      8
	:y                      42
	:width                  134
	:height                 15
	:value-x                71
	:value-y                42
	:value-length           2
	:stored-length          2
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "# fields:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              64
	:min-value              1
	:initial-value          1
	:initial-state          :active
	:read-only              nil
	:notify-handler         lin_field_notify
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 fields)
		:when                   (Notify )
		:to                     (popup1 fields)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (lin_field_notify)
		)
	)
)
(
	:type                   :text-field
	:name                   color_scale_min
	:owner                  controls1
	:help                   "Sets the currently highlighted field's minimum value to this value.  Only used with the arbitrary setting."
	:x                      185
	:y                      70
	:width                  165
	:height                 15
	:value-x                302
	:value-y                70
	:value-length           6
	:stored-length          6
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Color Scale Min:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :inactive
	:read-only              nil
	:notify-handler         cscale_min_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 color_scale_min)
		:when                   (Notify )
		:to                     (popup1 color_scale_min)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cscale_min_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   delta
	:owner                  controls1
	:help                   "Amount to offset each value of min or max as it proceeds up the color scale.  Only used with linear setting."
	:x                      8
	:y                      72
	:width                  95
	:height                 15
	:value-x                55
	:value-y                72
	:value-length           6
	:stored-length          6
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Delta:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          lin_delta_events
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 delta)
		:when                   (AnyEvent )
		:to                     (popup1 delta)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (lin_delta_events)
		)
	)
)
(
	:type                   :button
	:name                   apply_button
	:owner                  controls1
	:help                   "Sets the bottom field's minimum value to min and creates the number of fields specified by adding delta to min for each field.  Only used with linear setting."
	:x                      75
	:y                      105
	:width                  53
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       "red"
	:label                  "Apply"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         apply_button
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 apply_button)
		:when                   (Notify )
		:to                     (popup1 apply_button)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (apply_button)
		)
	)
)
(
	:type                   :text-field
	:name                   color_scale_max
	:owner                  controls1
	:help                   "Sets the top fields maximum value to this value.
Only used with the arbitrary setting."
	:x                      185
	:y                      105
	:width                  166
	:height                 15
	:value-x                303
	:value-y                105
	:value-length           6
	:stored-length          6
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Color Scale Max:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :inactive
	:read-only              nil
	:notify-handler         cscale_max_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (popup1 color_scale_max)
		:when                   (Notify )
		:to                     (popup1 color_scale_max)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (cscale_max_proc)
		)
	)
)
)
