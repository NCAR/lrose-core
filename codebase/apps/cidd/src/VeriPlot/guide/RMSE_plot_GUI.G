;GIL-3
(
(
	:type                   :base-window
	:name                   window1
	:owner                  nil
	:width                  934
	:height                 492
	:background-color       ""
	:foreground-color       ""
	:label                  "Verification Plotter"
	:label-type             :string
	:initial-state          :open
	:show-footer            nil
	:resizable              t
	:icon-file              ""
	:icon-label             ""
	:icon-mask-file         ""
	:event-handler          window_event_func
	:user-data              ()
	:actions                (
		(
		:from                   (window1)
		:when                   (AnyEvent )
		:to                     (window1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (window_event_func)
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
	:width                  934
	:height                 63
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
	:name                   plot_end_tx
	:owner                  controls1
	:help                   ""
	:x                      8
	:y                      8
	:width                  326
	:height                 15
	:value-x                94
	:value-y                8
	:value-length           40
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Plot Ends at:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   plot_width_tx
	:owner                  controls1
	:help                   ""
	:x                      356
	:y                      8
	:width                  193
	:height                 15
	:value-x                478
	:value-y                8
	:value-length           3
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :numeric
	:label                  "Plot Length (days):"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:max-value              732
	:min-value              1
	:initial-value          1
	:initial-state          :active
	:read-only              nil
	:notify-handler         plot_width_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (window1 plot_width_tx)
		:when                   (Notify )
		:to                     (window1 plot_width_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (plot_width_proc)
		)
	)
)
(
	:type                   :setting
	:name                   field_st
	:owner                  controls1
	:help                   ""
	:x                      12
	:y                      30
	:width                  566
	:height                 23
	:value-x                56
	:value-y                30
	:rows                   1
	:columns                0
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :exclusive
	:selection-required     t
	:label                  "Field:"
	:label-type             :string
	:notify-handler         set_field_proc
	:event-handler          nil
	:choices                ("Temperature" "Dew Point" "Relative Humidity" "Pressure" "Wind Speed" "Wind Direction" "U" "V" )
	:choice-label-types     (:string :string :string :string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" "" "" "" )
	:initial-selections     (nil nil nil nil nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (window1 field_st)
		:when                   (Notify )
		:to                     (window1 field_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_field_proc)
		)
	)
)
(
	:type                   :message
	:name                   time_msg
	:owner                  controls1
	:help                   ""
	:x                      846
	:y                      40
	:width                  66
	:height                 13
	:foreground-color       ""
	:label                  "TIME String"
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
	:help                   ""
	:x                      0
	:y                      59
	:width                  934
	:height                 433
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   nil
	:horizontal-scrollbar   nil
	:scrollable-width       934
	:vertical-scrollbar     nil
	:scrollable-height      433
	:repaint-proc           canvas_repaint
	:event-handler          canvas_event_func
	:drawing-model          :xview
	:user-data              ()
	:actions                (
		(
		:from                   (window1 canvas1)
		:when                   (Repaint )
		:to                     (window1 canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (canvas_repaint)
		)
		(
		:from                   (window1 canvas1)
		:when                   (AnyEvent )
		:to                     (window1 canvas1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (canvas_event_func)
		)
	)
)
(
	:type                   :popup-window
	:name                   config_pu
	:owner                  window1
	:width                  370
	:height                 346
	:background-color       ""
	:foreground-color       ""
	:label                  "Verification Data Configuration"
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
	:name                   controls2
	:owner                  config_pu
	:help                   ""
	:x                      0
	:y                      0
	:width                  370
	:height                 346
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
	:type                   :message
	:name                   data_msg
	:owner                  controls2
	:help                   ""
	:x                      16
	:y                      4
	:width                  72
	:height                 13
	:foreground-color       ""
	:label                  "Data Message"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   del_bt
	:owner                  controls2
	:help                   ""
	:x                      220
	:y                      4
	:width                  54
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "DELETE"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         nil
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   close_bt
	:owner                  controls2
	:help                   ""
	:x                      298
	:y                      4
	:width                  48
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "CLose"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         close_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu close_bt)
		:when                   (Notify )
		:to                     (config_pu close_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (close_proc)
		)
	)
)
(
	:type                   :scrolling-list
	:name                   model_lst
	:owner                  controls2
	:help                   ""
	:x                      16
	:y                      28
	:width                  300
	:height                 98
	:value-x                16
	:value-y                28
	:rows                   3
	:foreground-color       ""
	:label                  ""
	:title                  "MODELS"
	:label-type             :string
	:layout-type            :horizontal
	:read-only              nil
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
(
	:type                   :text-field
	:name                   url_tx
	:owner                  controls2
	:help                   ""
	:x                      16
	:y                      132
	:width                  326
	:height                 15
	:value-x                42
	:value-y                132
	:value-length           50
	:stored-length          80
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "URL"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          ""
	:initial-state          :active
	:read-only              nil
	:notify-handler         set_url_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu url_tx)
		:when                   (Notify )
		:to                     (config_pu url_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_url_proc)
		)
	)
)
(
	:type                   :button
	:name                   add_url_bt
	:owner                  controls2
	:help                   ""
	:x                      268
	:y                      160
	:width                  36
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Add"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         add_url_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu add_url_bt)
		:when                   (Notify )
		:to                     (config_pu add_url_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (add_url_proc)
		)
	)
)
(
	:type                   :message
	:name                   color_msg
	:owner                  controls2
	:help                   ""
	:x                      22
	:y                      162
	:width                  72
	:height                 13
	:foreground-color       ""
	:label                  "Color: Black"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu color_msg)
		:when                   (Select )
		:to                     (config_pu color_msg)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_color_proc)
		)
	)
)
(
	:type                   :setting
	:name                   plot_st
	:owner                  controls2
	:help                   ""
	:x                      14
	:y                      208
	:width                  235
	:height                 81
	:value-x                46
	:value-y                208
	:rows                   0
	:columns                2
	:layout-type            :horizontal
	:foreground-color       ""
	:setting-type           :check
	:selection-required     t
	:label                  "Plot"
	:label-type             :string
	:notify-handler         set_plots_proc
	:event-handler          nil
	:choices                ("Temperature" "Dew Pt" "Wind Dir" "Wind Speed" "Pressure" )
	:choice-label-types     (:string :string :string :string :string )
	:choice-colors          ("" "" "" "" "" )
	:initial-selections     (nil nil nil nil nil )
	:initial-state          :active
	:user-data              ()
	:actions                (
		(
		:from                   (config_pu plot_st)
		:when                   (Notify )
		:to                     (config_pu plot_st)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (set_plots_proc)
		)
	)
)
)
