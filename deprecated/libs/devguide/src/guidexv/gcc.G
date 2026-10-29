;GIL-3
;	@(#)gcc.G	2.10 91/10/16 Copyright 1989 Sun Microsystems
(
(
	:type                   :popup-window
	:name                   gccwin
	:owner                  nil
	:width                  270
	:height                 338
	:background-color       ""
	:foreground-color       ""
	:label                  ""
	:label-type             :string
	:initial-state          :invisible
	:show-footer            t
	:resizable              nil
	:pinned                 t
	:done-handler           nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   controls
	:owner                  gccwin
	:help                   "Devguide: Color Chooser:

This property window is for setting color.
Choose a color by making a selection in the 
scrolling list.  Pressing the apply button will
associate the chosen color with the interface
object listed in the property window footer."
	:x                      0
	:y                      0
	:width                  270
	:height                 338
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
	:actions                ()
)
(
	:type                   :scrolling-list
	:name                   color_list
	:owner                  controls
	:help                   "Colors scrolling list:

The colors scrolling list shows all the
colors available to you in X11r3.  Click
SELECT on the name of the color you want.
The Color text field below changes to show the
color you chose."
	:x                      25
	:y                      10
	:width                  200
	:height                 236
	:value-x                25
	:value-y                10
	:rows                   12
	:foreground-color       ""
	:label                  ""
	:title                  ""
	:label-type             :string
	:layout-type            :horizontal
	:read-only              t
	:multiple-selections    nil
	:selection-required     t
	:initial-state          :active
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:notify-handler         gcc_list_proc
	:event-handler          nil
	:events                 ()
	:initial-list-values    ()
	:initial-list-glyphs    ()
	:initial-selections     ()
	:user-data              ()
	:actions                (
		(
		:from                   (gccwin color_list)
		:when                   (Notify )
		:to                     (gccwin color_list)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gcc_list_proc)
		)
	)
)
(
	:type                   :message
	:name                   color_blot
	:owner                  controls
	:help                   "Color chip:

The Color chip shows the color that has been
selected in the scrolling list.  This can
be used to preview the color.
"
	:x                      227
	:y                      256
	:width                  32
	:height                 32
	:foreground-color       ""
	:label                  "black32.pr"
	:label-type             :glyph
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:events                 ()
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   color_name
	:owner                  controls
	:help                   "Color text field:

The Color text field shows the selected color
in the scrolling list above.  To enter a
color name without choosing it in the
scrolling list, click SELECT on the text
field and enter a proper X11 color name in
the text field."
	:x                      10
	:y                      273
	:width                  207
	:height                 15
	:value-x                57
	:value-y                273
	:value-length           20
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Color:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
	:actions                ()
)
(
	:type                   :group
	:name                   group1
	:x                      10
	:y                      256
	:group-type             :row
	:members                (color_name color_blot )
	:rows                   1
	:row-alignment          :bottom-edges
	:columns                0
	:column-alignment       :left-edges
	:horizontal-spacing     10
	:vertical-spacing       10
	:anchor-object          nil
	:anchor-point           :north-west
	:reference-point        :north-west
	:horizontal-offset      0
	:vertical-offset        0
	:user-data              ()
	:actions                ()
)
(
	:type                   :group
	:name                   group3
	:x                      10
	:y                      10
	:group-type             :column
	:members                (color_list group1 )
	:rows                   0
	:row-alignment          :bottom-edges
	:columns                1
	:column-alignment       :vertical-centers
	:horizontal-spacing     10
	:vertical-spacing       10
	:anchor-object          nil
	:anchor-point           :north-west
	:reference-point        :north-west
	:horizontal-offset      0
	:vertical-offset        0
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   apply
	:owner                  controls
	:help                   "The Apply button:

The Apply button applies any color changes
you've made when you click SELECT on it."
	:x                      77
	:y                      308
	:width                  53
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Apply"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         gcc_apply
	:event-handler          nil
	:events                 ()
	:user-data              ()
	:actions                (
		(
		:from                   (gccwin apply)
		:when                   (Notify )
		:to                     (gccwin apply)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gcc_apply)
		)
	)
)
(
	:type                   :button
	:name                   reset
	:owner                  controls
	:help                   "The Reset button:

The Reset button returns any changed color to
its original state as it was when you last
used the Apply button or when you selected a
new color in the colors scrolling list (which
has the same effect as choosing the Apply
button.)  Click SELECT on the Reset button to
reset a color."
	:x                      140
	:y                      308
	:width                  51
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Reset"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         gcc_reset
	:event-handler          nil
	:events                 ()
	:user-data              ()
	:actions                (
		(
		:from                   (gccwin reset)
		:when                   (Notify )
		:to                     (gccwin reset)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gcc_reset)
		)
	)
)
(
	:type                   :group
	:name                   group2
	:x                      77
	:y                      308
	:group-type             :row
	:members                (apply reset )
	:rows                   1
	:row-alignment          :top-edges
	:columns                0
	:column-alignment       :vertical-centers
	:horizontal-spacing     10
	:vertical-spacing       10
	:anchor-object          nil
	:anchor-point           :north-west
	:reference-point        :north-west
	:horizontal-offset      0
	:vertical-offset        0
	:user-data              ()
	:actions                ()
)
(
	:type                   :group
	:name                   group4
	:x                      10
	:y                      10
	:group-type             :column
	:members                (group3 group2 )
	:rows                   0
	:row-alignment          :bottom-edges
	:columns                1
	:column-alignment       :vertical-centers
	:horizontal-spacing     10
	:vertical-spacing       20
	:anchor-object          controls
	:anchor-point           :north-west
	:reference-point        :north-west
	:horizontal-offset      10
	:vertical-offset        10
	:user-data              ()
	:actions                ()
)
)
