#ifndef VK_RA_PIXMAP_H
#define VK_RA_PIXMAP_H


// These take a char** xpm programmatic description

Pixmap VkCreateXPMPixmap(Widget w, char **description, char **resources = NULL);

void VkSetHighlightingPixmap(Widget w, 
			     char **xpmPixmapDesc, 
			     const char *resource = NULL);

void VkSetHighlightingPixmap(Widget w, 
			     char **xpmPixmapDesc,
			     char **xpmInsensitivePixmapDesc,			     
			     const char *resource);

// These take a char* xpm in-memory buffer, or a file name

void VkSetHighlightingPixmap(Widget w, 
			     char *xpmBufferOrFile, 
			     const char *resource = NULL);

void VkSetHighlightingPixmap(Widget w, 
			     char *xpmBufferOrFile,
			     char *xpmInsensitiveBufferOrFile,			     
			     const char *resource);

Pixmap VkCreateXPMPixmap(Widget w, char *fileOrBuffer, char **resources = NULL);

#endif    
