/********************************************************************
 *           YaST2-GTK - http://en.opensuse.org/YaST2-GTK           *
 ********************************************************************/

#include <config.h>
#include <ycp/y2log.h>
#include <YGUI.h>
#include "YGWidget.h"

#include "YProgressBar.h"

class YGProgressBar : public YProgressBar, public YGLabeledWidget
{
public:
	YGProgressBar (YWidget *parent, const string &label, int maxValue)
	: YProgressBar (NULL, label, maxValue)
		// NOTE: its label widget is positionated at the vertical, because its label
		// may change often and so will its size, which will look odd (we may want
		// to make the label widget to only grow).
	, YGLabeledWidget (this, parent, label, YD_VERT, true,
	                   GTK_TYPE_PROGRESS_BAR, NULL)
	{}

	// YProgressBar
	virtual void setValue (int value)
	{
		IMPL
		float fraction = ((float) value) / maxValue();
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (getWidget()), fraction);
        YProgressBar::setValue (value);
	}

	YGWIDGET_IMPL_COMMON
	YGLABEL_WIDGET_IMPL_SET_LABEL_CHAIN (YProgressBar)
};

YProgressBar *YGWidgetFactory::createProgressBar (YWidget *parent, const string &label,
                                                  int maxValue)
{
	IMPL
	return new YGProgressBar (parent, label, maxValue);
}

#include "YDownloadProgress.h"

class YGDownloadProgress : public YDownloadProgress, public YGLabeledWidget
{
	guint timeout_id;

public:
	YGDownloadProgress (YWidget *parent, const string &label,
	                    const string &filename, YFileSize_t expectedFileSize)
	: YDownloadProgress (NULL, label, filename, expectedFileSize)
	, YGLabeledWidget (this, parent, label, YD_HORIZ, true,
	                   GTK_TYPE_PROGRESS_BAR, NULL)
	{
		timeout_id = g_timeout_add (250, timeout_cb, this);
	}

	virtual ~YGDownloadProgress()
	{
		g_source_remove (timeout_id);
	}

	virtual void setExpectedSize (YFileSize_t size)
	{
		YDownloadProgress::setExpectedSize (size);
		timeout_cb (this);  // force an update
	}

	static gboolean timeout_cb (void *pData)
	{
		YGDownloadProgress *pThis = (YGDownloadProgress*) pData;
		float fraction = ((float) pThis->currentFileSize()) / pThis->expectedSize();
		if (fraction > 1) fraction = 1;
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (pThis->getWidget()), fraction);
		return TRUE;
	}

	YGWIDGET_IMPL_COMMON
	YGLABEL_WIDGET_IMPL_SET_LABEL_CHAIN (YDownloadProgress)
};

YDownloadProgress *YGOptionalWidgetFactory::createDownloadProgress (YWidget *parent,
		const string &label, const string &filename, YFileSize_t expectedFileSize)
{
	IMPL
	return new YGDownloadProgress (parent, label, filename, expectedFileSize);
}

#include "ygtkratiobox.h"
#include "YMultiProgressMeter.h"

class YGMultiProgressMeter : public YMultiProgressMeter, public YGWidget
{
public:
	YGMultiProgressMeter (YWidget *parent, YUIDimension dim, const vector<float> &maxValues)
	: YMultiProgressMeter (NULL, dim, maxValues)
	, YGWidget (this, parent, true,
	            horizontal() == YD_HORIZ ? YGTK_TYPE_RATIO_HBOX : YGTK_TYPE_RATIO_VBOX, NULL)
	{
//		ygtk_ratio_box_set_homogeneous (YGTK_RATIO_BOX (getWidget()), TRUE);
		ygtk_ratio_box_set_spacing (YGTK_RATIO_BOX (getWidget()), 2);
		for (int s = 0; s < segments(); s++) {
			GtkWidget* bar = gtk_progress_bar_new();
			gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (bar),
				horizontal() ? GTK_PROGRESS_LEFT_TO_RIGHT : GTK_PROGRESS_BOTTOM_TO_TOP);
			// Progress bars would ask for too much size with weight...
			const int min_size = 5;
			if (horizontal())
				gtk_widget_set_size_request (bar, min_size, -1);
			else
				gtk_widget_set_size_request (bar, -1, min_size);
			ygtk_ratio_box_pack (YGTK_RATIO_BOX (getWidget()), bar,
			                     getSegmentWeight (s), TRUE, TRUE, 0);
		}

		ygtk_adj_size_set_max (YGTK_ADJ_SIZE (m_adj_size), horizontal() ? 200 : 0,
		                                                   horizontal() ? 0 : 200);
		gtk_widget_show_all (getWidget());
	}

	virtual void doUpdate()
	{
		GList* children = gtk_container_get_children (GTK_CONTAINER (getWidget()));
		int s = 0;
		for (GList *i = children; i && s < segments(); i = i->next, s++) {
			GtkProgressBar *bar = GTK_PROGRESS_BAR (i->data);
			gtk_progress_bar_set_fraction (bar, getSegmentValue (s));
		}
		g_list_free (children);
	}

	int getSegmentWeight (int n)
	{
		if (vertical())
			n = (segments() - n) - 1;
		return maxValue (n);
	}
	float getSegmentValue (int n)
	{
		if (vertical())
			n = (segments() - n) - 1;
		if (currentValue (n) == -1)
			return 0;
		return 1.0 - (((float) currentValue (n)) / maxValue (n));
	}

	YGWIDGET_IMPL_COMMON
};

YMultiProgressMeter *YGOptionalWidgetFactory::createMultiProgressMeter (YWidget *parent,
		YUIDimension dim, const vector<float> &maxValues)
{
	return new YGMultiProgressMeter (parent, dim, maxValues);
}

