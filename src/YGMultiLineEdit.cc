/* Yast GTK ... */

#include <config.h>
#include <ycp/y2log.h>
#include <YGUI.h>
#include <string>
#include "YGWidget.h"

class YGTextView : public YGScrolledWidget
{
int maxChars;

public:
	YGTextView (YWidget *y_widget, YGWidget *parent,
	            const YWidgetOpt &opt,
	            const YCPString &label,
	            const YCPString &text)
		: YGScrolledWidget (y_widget, parent, label, YD_VERT, true,
		                    GTK_TYPE_TEXT_VIEW, NULL)
	{
		IMPL
		maxChars = -1;
		setText (text);
		g_signal_connect (G_OBJECT (getBuffer()), "changed",
		                  G_CALLBACK (text_changed_cb), this);
	}

	virtual ~YGTextView() {}

	GtkTextBuffer* getBuffer()
	{ return gtk_text_view_get_buffer (GTK_TEXT_VIEW (getWidget())); }

	int getCharsNb()
	{
		IMPL
		return gtk_text_buffer_get_char_count (getBuffer());
	}

	void setCharsNb (int max_chars)
	{
		IMPL
		maxChars = max_chars;
		if (maxChars != -1 && getCharsNb() > maxChars)
			truncateText (maxChars);
	}

	void truncateText(int pos)
	{
		IMPL
		GtkTextIter start_it, end_it;
		gtk_text_buffer_get_iter_at_offset (getBuffer(), &start_it, pos);
		gtk_text_buffer_get_end_iter (getBuffer(), &end_it);

		g_signal_handlers_block_by_func (getWidget(), (gpointer) text_changed_cb, this);
		gtk_text_buffer_delete (getBuffer(), &start_it, &end_it);
		g_signal_handlers_unblock_by_func (getWidget(), (gpointer) text_changed_cb, this);
	}

	void setText (const YCPString &text)
	{
		IMPL
		gtk_text_buffer_set_text (getBuffer(), text->value_cstr(), -1);
	}

	YCPString getText()
	{
		IMPL
		GtkTextIter start_it, end_it;
		gtk_text_buffer_get_bounds (getBuffer(), &start_it, &end_it);

		gchar* text = gtk_text_buffer_get_text (getBuffer(), &start_it, &end_it, FALSE);
		YCPString str (text);
		g_free (text);
		return str;
	}

	void scrollDown()
	{
		GtkTextIter end_iter;
		gtk_text_buffer_get_end_iter (getBuffer(), &end_iter);
		GtkTextMark *end_mark;
		end_mark = gtk_text_buffer_create_mark
		               (getBuffer(), NULL, &end_iter, FALSE);
		gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (getWidget()),
		               end_mark, 0.0, FALSE, 0, 0);
		gtk_text_buffer_delete_mark (getBuffer(), end_mark);
	}

	// Event callbacks
	static void text_changed_cb (GtkTextBuffer *buffer, YGTextView *pThis)
	{
		if (pThis->maxChars != -1 && pThis->getCharsNb() > pThis->maxChars) {
			pThis->truncateText (pThis->maxChars);
			gdk_beep();
		}
		pThis->emitEvent (YEvent::ValueChanged);
	}
};

#include "YMultiLineEdit.h"

class YGMultiLineEdit : public YMultiLineEdit, public YGTextView
{
public:
	YGMultiLineEdit (const YWidgetOpt &opt,
	                 YGWidget *parent,
	                 const YCPString &label,
	                 const YCPString &text)
	: YMultiLineEdit (opt, label)
	, YGTextView (this, parent, opt, label, text)
	{
	}

	virtual ~YGMultiLineEdit() {}

	// YMultiLineEdit
	virtual void setText (const YCPString &text)
	{ YGTextView::setText (text); }

	virtual YCPString text()
	{ return YGTextView::getText(); }

	virtual void setInputMaxLength (const YCPInteger &numberOfChars)
	{ YGTextView::setCharsNb (numberOfChars->asInteger()->value()); }

	// YWidget
	YGWIDGET_IMPL_NICESIZE
	YGWIDGET_IMPL_SET_ENABLING
	YGWIDGET_IMPL_SET_SIZE
	YGWIDGET_IMPL_KEYBOARD_FOCUS
	// YGLabelWidget
	YGLABEL_WIDGET_IMPL_SET_LABEL_CHAIN(YMultiLineEdit)
};

YWidget *
YGUI::createMultiLineEdit (YWidget *parent, YWidgetOpt & opt,
                           const YCPString & label, const YCPString & text)
{
	return new YGMultiLineEdit (opt, YGWidget::get (parent), label, text);
}

#include "YLogView.h"

class YGLogView : public YLogView, public YGTextView
{
public:
	YGLogView (const YWidgetOpt &opt,
	           YGWidget *parent,
	           const YCPString &label,
	           int visibleLines, int maxLines)
	: YLogView (opt, label, visibleLines, maxLines)
	, YGTextView (this, parent, opt, label, YCPString(""))
	{
		gtk_scrolled_window_set_policy
				(GTK_SCROLLED_WINDOW (YGLabeledWidget::getWidget()),
				 GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
		gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (getWidget()), GTK_WRAP_WORD);
	}

	virtual ~YGLogView() {}

	// YLogView
	virtual void setLogText (const YCPString &text)
	{
		setText (text);
		scrollDown();
	}

	// YWidget
	YGWIDGET_IMPL_SET_ENABLING
	YGWIDGET_IMPL_KEYBOARD_FOCUS
	YGWIDGET_IMPL_SET_SIZE
	virtual long nicesize (YUIDimension dim)
	{
		IMPL
		if (dim == YD_HORIZ) return getNiceSize (dim);

		PangoFontDescription *font = gtk_widget_get_style (getWidget())->font_desc;
		gint font_size = pango_font_description_get_size(font);
		if (!pango_font_description_get_size_is_absolute(font))
			font_size /= PANGO_SCALE;

		return MAX (getNiceSize (dim), font_size * visibleLines());
	}

	// YGLabelWidget
	YGLABEL_WIDGET_IMPL_SET_LABEL_CHAIN(YLogView)
};

YWidget *
YGUI::createLogView (YWidget *parent, YWidgetOpt &opt,
                     const YCPString &label, int visibleLines,
                     int maxLines)
{
	return new YGLogView (opt, YGWidget::get (parent),
	                      label, visibleLines, maxLines);
}