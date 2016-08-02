;GIL-3
(
(
	:type                   :base-window
	:name                   c2s_bw
	:owner                  nil
	:width                  162
	:height                 118
	:background-color       "Dim Gray"
	:foreground-color       ""
	:label                  "Cidd SkewT"
	:label-type             :string
	:initial-state          :open
	:show-footer            nil
	:resizable              nil
	:icon-file              "cidd_skewt.icon"
	:icon-label             ""
	:icon-mask-file         ""
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :control-area
	:name                   controls1
	:owner                  c2s_bw
	:help                   ""
	:x                      0
	:y                      0
	:width                  162
	:height                 118
	:background-color       "Dim Gray"
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
	:name                   info_msg
	:owner                  controls1
	:help                   ""
	:x                      16
	:y                      0
	:width                  133
	:height                 13
	:foreground-color       "White"
	:label                  "Select point on plot"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :message
	:name                   info_msg2
	:owner                  controls1
	:help                   ""
	:x                      23
	:y                      18
	:width                  119
	:height                 13
	:foreground-color       "White"
	:label                  "then push button"
	:label-type             :string
	:label-bold             t
	:initial-state          :active
	:event-handler          nil
	:user-data              ()
	:actions                ()
)
(
	:type                   :button
	:name                   def_gen_bt
	:owner                  controls1
	:help                   ""
	:x                      6
	:y                      42
	:width                  149
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Generate Skew T Plot"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         gen_plot_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (c2s_bw def_gen_bt)
		:when                   (Notify )
		:to                     (c2s_bw def_gen_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gen_plot_proc)
		)
	)
)
(
	:type                   :button
	:name                   gen_skewt_file_bt
	:owner                  controls1
	:help                   ""
	:x                      10
	:y                      69
	:width                  140
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Generate CLASS File"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         gen_file_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (c2s_bw gen_skewt_file_bt)
		:when                   (Notify )
		:to                     (c2s_bw gen_skewt_file_bt)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gen_file_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   slp_tx
	:owner                  controls1
	:help                   ""
	:x                      6
	:y                      98
	:width                  149
	:height                 15
	:value-x                91
	:value-y                98
	:value-length           8
	:stored-length          80
	:rows                   3
	:foreground-color       "Light Gray"
	:text-type              :alphanumeric
	:label                  "SLP (mbar):"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       t
	:initial-value          "1000"
	:initial-state          :active
	:read-only              nil
	:notify-handler         slp_tx_proc
	:event-handler          nil
	:user-data              ()
	:actions                (
		(
		:from                   (c2s_bw slp_tx)
		:when                   (Notify )
		:to                     (c2s_bw slp_tx)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (slp_tx_proc)
		)
	)
)
)
