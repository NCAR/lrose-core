;GIL-3
(
(
	:type                   :menu
	:name                   menu1
	:help                   ""
	:columns                1
	:menu-type              :exclusive
	:menu-handler           select_field_proc
	:menu-title             "Show"
	:menu-item-labels       ("Ceiling" "Visibility" "Flight cat" "Temperature" "Wind Speed" "Wind Direction" "Humidity" "Pressure" )
	:menu-item-label-types  (:string :string :string :string :string :string :string :string )
	:menu-item-states       (:active :active :active :active :active :active :active :active )
	:menu-item-defaults     (nil nil nil nil nil nil nil nil )
	:initial-selections     (nil nil nil nil nil nil nil nil )
	:menu-item-handlers     (nil nil nil nil nil nil nil nil )
	:menu-item-menus        (nil nil nil nil nil nil nil nil )
	:menu-item-colors       ("" "" "" "" "" "" "" "" )
	:pinnable               nil
	:user-data              ()
	:actions                (
		(
		:from                   (menu1)
		:when                   (Notify )
		:to                     (menu1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (select_field_proc)
		)
	)
)
(
	:type                   :base-window
	:name                   win1
	:owner                  nil
	:width                  266
	:height                 234
	:background-color       ""
	:foreground-color       ""
	:label                  "Station Trends"
	:label-type             :string
	:initial-state          :open
	:show-footer            nil
	:resizable              t
	:icon-file              "gauge_strip.icon"
	:icon-label             ""
	:icon-mask-file         ""
	:event-handler          win1_event_proc
	:user-data              ()
	:actions                (
		(
		:from                   (win1)
		:when                   (AnyEvent )
		:to                     (win1)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (win1_event_proc)
		)
	)
)
(
	:type                   :canvas-pane
	:name                   canvas1
	:owner                  win1
	:help                   ""
	:x                      0
	:y                      0
	:width                  266
	:height                 234
	:background-color       ""
	:foreground-color       ""
	:initial-state          :visible
	:draggable              nil
	:droppable              nil
	:default-drop-site      nil
	:menu                   menu1
	:horizontal-scrollbar   nil
	:scrollable-width       266
	:vertical-scrollbar     nil
	:scrollable-height      234
	:repaint-proc           nil
	:event-handler          nil
	:drawing-model          :xwindows
	:user-data              ()
	:actions                ()
)
)
