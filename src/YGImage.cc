/********************************************************************
 *           YaST2-GTK - http://en.opensuse.org/YaST2-GTK           *
 ********************************************************************/

#define YUILogComponent "gtk"
#include <yui/Libyui_config.h>
#include "ygdkmngloader.h"
#include "YGUI.h"
#include "YGWidget.h"
#include "YImage.h"
#include "ygtkimage.h"
#include <string.h>

static inline bool endsWith (const std::string &str1, const char *str2)
{
	size_t len = strlen (str2);
	if (str1.size() < len) return false;
	return str1.compare (str1.size()-len, len, str2, len) == 0;
}

class YGImage : public YImage, public YGWidget
{
public:
	YGImage (YWidget *parent, const std::string &filename, bool animated)
	: YImage (NULL, filename, animated),
	  YGWidget (this, parent, YGTK_TYPE_IMAGE, NULL)
	{
		YGtkImage *image = YGTK_IMAGE (getWidget());
		const char *stock = NULL;
		if (endsWith (filename, "/msg_question.png"))
			stock = GTK_STOCK_DIALOG_QUESTION;
		else if (endsWith (filename, "/msg_info.png"))
			stock = GTK_STOCK_DIALOG_INFO;
		else if (endsWith (filename, "/msg_warning.png"))
			stock = GTK_STOCK_DIALOG_WARNING;
		else if (endsWith (filename, "/msg_error.png"))
			stock = GTK_STOCK_DIALOG_ERROR;

                GtkStyleContext *ctx;
                ctx = gtk_widget_get_style_context(m_widget);

		if (stock && gtk_style_context_lookup_icon_set (ctx, stock)) {
			GdkPixbuf *pixbuf = gtk_widget_render_icon_pixbuf (m_widget, stock, GTK_ICON_SIZE_DIALOG);
			ygtk_image_set_from_pixbuf (image, pixbuf);
		}
		else
			ygtk_image_set_from_file (image, filename.c_str(), animated);
	}

	virtual void setAutoScale (bool scale)
	{
		YGtkImageAlign align = CENTER_IMAGE_ALIGN;
		if (scale)
			align = SCALE_IMAGE_ALIGN;
		ygtk_image_set_props (YGTK_IMAGE (getWidget()), align, NULL);
	}

	YGWIDGET_IMPL_COMMON (YImage)
};

YImage *YGWidgetFactory::createImage (YWidget *parent, const std::string &filename, bool animated)
{ return new YGImage (parent, filename, animated); }

