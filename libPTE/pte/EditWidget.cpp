#include <QBoxLayout>
#include "Context.hpp"
#include "EditWidget.hpp"

namespace pte
{

EditWidget::EditWidget(QWidget* parent)
	: QWidget(parent)
{
	edit_ = new Edit(this);
	edit_->setDocument(new Document);
	edit_->document()->setHistoryEnabled(true);
	edit_->setContextMenu(Edit::setupActions(edit_));
	
	scrollBar_ = new QScrollBar(Qt::Vertical, this);
	edit_->setVerticalScrollBar(scrollBar_);
	
	QHBoxLayout* row = new QHBoxLayout;
	row->setSpacing(0);
	row->setMargin(0);
	row->addWidget(edit_);
	row->addWidget(scrollBar_);
	setLayout(row);
}

Edit* EditWidget::edit() const { return edit_; }
QScrollBar* EditWidget::scrollBar() const { return scrollBar_; }

QString EditWidget::text() const {
	return edit_->document()->text();
}

void EditWidget::setText(const QString& text) {
	edit_->document()->setText(text);
	edit_->moveCursor(0, 0);
	edit_->update();
}

} // namespace pte
