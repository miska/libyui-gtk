/********************************************************************
 *           YaST2-GTK - http://en.opensuse.org/YaST2-GTK           *
 ********************************************************************/

/* YGtkHtmlWrap is a wrapper around either GtkHtml and YGtkRichText, as
   set at compilation time. GtkHtml is superior, but to avoid dependencies,
   we use our customized in case it isn't installed.
   If you flag it as plain_text, it will simply wrap GtkTextView.
*/

#ifndef YGTK_HTML_WRAP_H
#define YGTK_HTML_WRAP_H

#include <gtk/gtkwidget.h>
G_BEGIN_DECLS

GtkWidget *ygtk_html_wrap_new (void);

GType ygtk_html_wrap_get_type (void);
void ygtk_html_wrap_init (GtkWidget *widget);  // if you use g_object_new(), call this

void ygtk_html_wrap_set_text (GtkWidget *widget, const gchar* text);
void ygtk_html_wrap_scroll (GtkWidget *widget, gboolean top /* or bottom */);

// callback of type: void (GtkWidget *htmlwrap, const gchar *url, gpointer data)
void ygtk_html_wrap_connect_link_clicked (GtkWidget *widget, GCallback callback, gpointer data);

// not supported on plain text
gboolean ygtk_html_wrap_search (GtkWidget *widget, const gchar *text);
gboolean ygtk_html_wrap_search_next (GtkWidget *widget, const gchar *text);  // F3

// set a background image -- to be done on (or after) realize
void ygtk_html_wrap_set_background (GtkWidget *widget, const char *image);

G_END_DECLS
#endif /* YGTK_HTML_WRAP_H */

