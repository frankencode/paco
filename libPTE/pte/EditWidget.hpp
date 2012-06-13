#ifndef PTE_EDITWIDGET_HPP
#define PTE_EDITWIDGET_HPP

#include <QWidget>
#include <QScrollBar>
#include "Edit.hpp"
#include "Document.hpp"

namespace pte
{

class EditWidget: public QWidget
{
	Q_OBJECT
	
public:
	EditWidget(QWidget* parent = 0);
	
	Edit* edit() const;
	QScrollBar* scrollBar() const;
	
	QString text() const;
	void setText(const QString& text);
	
private:
	Edit* edit_;
	QScrollBar* scrollBar_;
};

} // namespace pte

#endif // PTE_EDITWIDGET_HPP
