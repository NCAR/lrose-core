;GIL-3
;	@(#)gfm.G	2.13 91/10/16 Copyright 1990 Sun Microsystems
(
(
	:type                   :popup-window
	:name                   popup
	:owner                  nil
	:width                  313
	:height                 306
	:background-color       ""
	:foreground-color       ""
	:label                  "File Chooser"
	:label-type             :string
	:initial-state          :invisible
	:show-footer            t
	:resizable              t
	:pinned                 nil
	:done-handler           nil
	:event-handler          gfm_resize
	:events                 ()
	:user-data              ()
	:actions                (
		(
		:from                   (popup)
		:when                   (AnyEvent )
		:to                     (popup)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gfm_resize)
		)
	)
)
(
	:type                   :control-area
	:name                   controls
	:owner                  popup
	:help                   "Devguide: File Chooser:

Choose a file to be loaded or saved by selecting
it in the scrolling list and pressing the load/save
button."
	:x                      0
	:y                      0
	:width                  313
	:height                 306
	:background-color       ""
	:foreground-color       ""
	:initial-state          :active
	:show-border            nil
	:menu                   nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
	:actions                ()
)
(
	:type                   :text-field
	:name                   directory
	:owner                  controls
	:help                   "Directory text field:

The Directory text field displays the name
of the current directory whose contents are
displayed in the scrolling list."
	:x                      10
	:y                      10
	:width                  298
	:height                 15
	:value-x                84
	:value-y                10
	:value-length           28
	:stored-length          1024
	:rows                   3
	:foreground-color       ""
	:text-type              :alphanumeric
	:label                  "Directory:"
	:label-type             :string
	:layout-type            :horizontal
	:value-underlined       nil
	:initial-value          ""
	:initial-state          :active
	:read-only              t
	:notify-handler         nil
	:event-handler          nil
	:events                 ()
	:user-data              ()
	:actions                ()
)
(
	:type                   :scrolling-list
	:name                   list
	:owner                  controls
	:help                   "File scrolling list:

The scrolling list shows all the files available
in the current directory.  Click SELECT on a 
file and choose the Load/Save button or double
click SELECT on a file or directory.  If a 
directory is selected, that directory will be 
opened and the contents will be listed.  Otherwise
the selected file will opened.
"
	:x                      10
	:y                      35
	:width                  275
	:height                 200
	:value-x                10
	:value-y                35
	:rows                   10
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
	:notify-handler         gfm_list_proc
	:event-handler          nil
	:events                 ()
	:initial-list-values    ()
	:initial-list-glyphs    ()
	:initial-selections     ()
	:user-data              ()
	:actions                (
		(
		:from                   (popup list)
		:when                   (Notify )
		:to                     (popup list)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gfm_list_proc)
		)
	)
)
(
	:type                   :text-field
	:name                   file
	:owner                  controls
	:help                   "Name text field:

The Name text field displays the name of the
currently selected file or directory.  Enter
the name of a file or directory here and
click SELECT on Load/Save or enter RETURN 
to have that file or directory loaded.
Environment variables and tildes will be
properly expanded."
	:x                      10
	:y                      245
	:width                  290
	:height                 15
	:value-x                60
	:value-y                245
	:value-length           30
	:stored-length          1024
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
	:notify-handler         gfm_file_notify
	:event-handler          nil
	:events                 ()
	:user-data              ()
	:actions                (
		(
		:from                   (popup file)
		:when                   (Notify )
		:to                     (popup file)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gfm_file_notify)
		)
	)
)
(
	:type                   :group
	:name                   file_list_group
	:x                      10
	:y                      10
	:group-type             :column
	:members                (directory list file )
	:rows                   0
	:row-alignment          :top-edges
	:columns                1
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
	:type                   :button
	:name                   load
	:owner                  controls
	:help                   "The Load button:

The Load button (may also be Save, Import, etc)
will load (or save, import, etc) the file or
directory that is currently selected.  Click
SELECT on it to load.
"
	:x                      136
	:y                      280
	:width                  46
	:height                 19
	:constant-width         nil
	:button-type            :normal
	:foreground-color       ""
	:label                  "Load"
	:label-type             :string
	:initial-state          :active
	:menu                   nil
	:notify-handler         gfm_open
	:event-handler          nil
	:events                 ()
	:user-data              ()
	:actions                (
		(
		:from                   (popup load)
		:when                   (Notify )
		:to                     (popup load)
		:function_type          CallFunction
		:arg_type               ()
		:action                 (gfm_open)
		)
	)
)
(
	:type                   :group
	:name                   props_group
	:x                      10
	:y                      10
	:group-type             :column
	:members                (file_list_group load )
	:rows                   0
	:row-alignment          :top-edges
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
