#include <QVBoxLayout>
#include <QHBoxLayout>
#include "QxCommandsPreferences.hpp"

namespace pacu
{

QxCommandsPreferences::QxCommandsPreferences(QWidget* parent)
	: QWidget(parent)
{
	QVBoxLayout* col = new QVBoxLayout;
	{
		view_ = new QTreeView(this);
		col->addWidget(view_);
	}
	{
		QHBoxLayout* row = new QHBoxLayout;
		row->setMargin(0);
		row->setSpacing(1);
		add_ = new QToolButton(this);
		add_->setText(tr("Add"));
		add_->setSizePolicy(QSizePolicy::Expanding, add_->sizePolicy().verticalPolicy());
		del_ = new QToolButton(this);
		del_->setText(tr("Del"));
		del_->setSizePolicy(QSizePolicy::Expanding, del_->sizePolicy().verticalPolicy());
		edit_ = new QToolButton(this);
		edit_->setText(tr("Edit"));
		edit_->setSizePolicy(QSizePolicy::Expanding, edit_->sizePolicy().verticalPolicy());
		import_ = new QToolButton(this);
		import_->setText(tr("Import"));
		import_->setSizePolicy(QSizePolicy::Expanding, import_->sizePolicy().verticalPolicy());
		export_ = new QToolButton(this);
		export_->setText(tr("Export"));
		export_->setSizePolicy(QSizePolicy::Expanding, import_->sizePolicy().verticalPolicy());
		row->addWidget(add_);
		row->addWidget(del_);
		row->addWidget(edit_);
		row->addWidget(import_);
		row->addWidget(export_);
		col->addLayout(row);
	}
	setLayout(col);
}

} // namespace
