#include <QDebug> // DEBUG
#include <QFileDialog>
#include <QApplication>
#include <QFormLayout>
#include <QLayout>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>
#include <QMenu>
#include <pte/EditWidget.hpp>
#include <QDialogButtonBox>
#include "QxIconSelector.hpp"
#include "QxCommandEditor.hpp"

namespace pacu
{

QxCommandEditor::QxCommandEditor(QWidget* parent)
	: QxDialog(parent),
	  iconSelector_(new QxIconSelector(parent)),
	  command_(0)
{
	setWindowTitle(qApp->applicationName() + tr("- Edit Command"));
	
	QVBoxLayout* col = new QVBoxLayout;
	{
		QFormLayout* form = new QFormLayout;
		form->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
		{
			icon_ = new QToolButton(this);
			icon_->setText("...");
			icon_->setIconSize(QSize(24, 24));
			iconReset_ = new QToolButton(this);
			iconReset_->setText(tr("Reset"));
			connect(icon_, SIGNAL(clicked()), this, SLOT(loadIcon()));
			connect(iconReset_, SIGNAL(clicked()), this, SLOT(resetIcon()));
		
			QHBoxLayout* row = new QHBoxLayout;
			row->addWidget(icon_);
			row->addWidget(iconReset_);
			row->addStretch();
			form->addRow(tr("Icon"), row);
		}
		{
			noDescription_ = tr("<optional text>");
			description_ = new QLineEdit(this);
			description_->setText(noDescription_);
			form->addRow(tr("Description"), description_);
		}
		{
			modifier_ = new QComboBox(this);
			modifier_->addItem(tr("None"));
			modifier_->addItem(tr("Alt"), Qt::Key_Alt);
			modifier_->addItem(tr("Control"), Qt::Key_Control);
			modifier_->addItem(tr("Meta"), Qt::Key_Meta);
			keyGroup_ = new QComboBox(this);
			keyGroup_->addItem(tr("F1-F12"));
			keyGroup_->addItem(tr("0-9"));
			key_ = new QComboBox(this);
			connect(keyGroup_, SIGNAL(currentIndexChanged(int)), this, SLOT(updateKey(int)));
			updateKey(0);
			
			QHBoxLayout* row = new QHBoxLayout;
			row->addWidget(modifier_);
			row->addWidget(keyGroup_);
			row->addWidget(key_);
			row->addStretch();
			form->addRow(tr("Short key"), row);
		}
		{
			scriptLabel_ = new QLabel(tr("Script"), this);
			target_ = new QComboBox(this);
			target_->addItem(tr("Paste into current terminal"), QxCommand::ActiveTerminal);
			target_->addItem(tr("Paste into new terminal"), QxCommand::NewTerminal);
			// target_->addItem(tr("Paste into editor"), QxCommand::TextEditor);
			target_->addItem(tr("Open by default application"), QxCommand::WebBrowser);
			connect(target_, SIGNAL(currentIndexChanged(int)), this, SLOT(updateTarget(int)));
			QHBoxLayout* row = new QHBoxLayout;
			row->addWidget(target_);
			row->addStretch();
			form->addRow(scriptLabel_, row);
		}
		{
			autoSaveFile_ = new QCheckBox(tr("Automatically save current file"), this);
			autoOpenNextLink_ = new QCheckBox(tr("Automatically open next link"), this);
			QVBoxLayout* col = new QVBoxLayout();
			col->addWidget(autoSaveFile_);
			col->addWidget(autoOpenNextLink_);
			form->addRow(tr("Options"), col);
		}
		col->addLayout(form);
	}
	{
		QFrame* frame = new QFrame(this);
		frame->setFrameStyle(QFrame::Sunken|QFrame::StyledPanel);
		
		script_ = new EditWidget(frame);
		script_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		
		QHBoxLayout* cell = new QHBoxLayout;
		cell->setSpacing(0);
		cell->setMargin(0);
		cell->addWidget(script_);
		frame->setLayout(cell);
		
		QMenu* editMenu = script_->edit()->contextMenu();
		editMenu->addSeparator();
		QMenu* substMenu = editMenu->addMenu(tr("Insert $SUBST"));
		const char* substNames[] = {
			"$DIRPATH", "$DIRNAME",
			"$FILEPATH", "$FILENAME", "$FILEID", "$FILEEXT",
			"$TEXT", "$WORD", "$CURSOR_LINE", "$CURSOR_COLUMN", "$OFFSET"
		};
		for (int i = 0, n = sizeof(substNames) / sizeof(const char*); i < n; ++i)
			substMenu->addAction(substNames[i]);
		connect(substMenu, SIGNAL(triggered(QAction*)), this, SLOT(substTriggered(QAction*)));
		
		col->addWidget(frame);
	}
	{
		QDialogButtonBox* box = new QDialogButtonBox(
			QDialogButtonBox::Ok | QDialogButtonBox::Cancel
		);
		connect(box, SIGNAL(accepted()), this, SLOT(accept()));
		connect(box, SIGNAL(rejected()), this, SLOT(reject()));
		col->addWidget(box);
	}
	setLayout(col);
	
	// design HACK
	resize(parent->size() * 1.2);
	
	connect(this, SIGNAL(accepted()), this, SLOT(updateCommand()));
}

QxCommand* QxCommandEditor::command() const { return command_; }

void QxCommandEditor::setCommand(QxCommand* cmd)
{
	command_ = cmd;
	icon_->setIcon(cmd->icon_);
	if (cmd->description_ != QString()) {
		description_->setText(cmd->description_);
	}
	else {
		description_->setText(noDescription_);
		// description_->selectAll();
	}
		
	{
		QString s = cmd->keysToString();
		
		if (s.contains("Alt"))
			modifier_->setCurrentIndex(1);
		else if (s.contains("Ctrl"))
			modifier_->setCurrentIndex(2);
		else if (s.contains("Meta"))
			modifier_->setCurrentIndex(3);
		
		bool found = false;
		
		for (int k = 11; k >= 0; --k) {
			if (s.contains(QString("F%1").arg(k + 1))) {
				keyGroup_->setCurrentIndex(0);
				key_->setCurrentIndex(k);
				found = true;
				break;
			}
		}
		
		if (!found)
			for (int k = 0; k < 10; ++k) {
				if (s.contains(QString("%1").arg((k + 1) % 10))) {
					keyGroup_->setCurrentIndex(1);
					key_->setCurrentIndex(k);
					break;
				}
			}
	}
	target_->setCurrentIndex(cmd->target_);
	autoSaveFile_->setChecked(cmd->autoSaveFile_);
	autoOpenNextLink_->setChecked(cmd->autoOpenNextLink_);
	script_->setText(cmd->script_);
}

Edit* QxCommandEditor::scriptEdit() const
{
	return script_->edit();
}

void QxCommandEditor::updateKey(int)
{
	if (keyGroup_->currentIndex() == 0) {
		key_->clear();
		for (int i = 0; i < 12; ++i)
			key_->addItem(QString("F%1").arg(i + 1), Qt::Key_F1 + i);
	}
	else if (keyGroup_->currentIndex() == 1) {
		if (modifier_->itemData(modifier_->currentIndex()) == QVariant::Invalid)
			modifier_->setCurrentIndex(1);
		key_->clear();
		for (int i = 0; i < 10; ++i)
			key_->addItem(QString("%1").arg((i + 1) % 10), Qt::Key_0 + (i + 1) % 10);
	}
}

void QxCommandEditor::updateTarget(int target)
{
	if ((target == QxCommand::ActiveTerminal) || (target == QxCommand::NewTerminal))
		scriptLabel_->setText(tr("Script"));
	/*else if (target == QxCommand::TextEditor)
		scriptLabel_->setText(tr("Snippet"));*/
	else if (target == QxCommand::WebBrowser)
		scriptLabel_->setText(tr("URL"));
}

void QxCommandEditor::loadIcon()
{
	if (iconSelector_->exec() == QDialog::Accepted)
		icon_->setIcon(iconSelector_->icon());
}

void QxCommandEditor::resetIcon()
{
	icon_->setIcon(QIcon());
}

void QxCommandEditor::updateCommand()
{
	command_->icon_ = icon_->icon().pixmap(icon_->iconSize());
	command_->description_ = (description_->text() != noDescription_) ? description_->text() : QString();
	
	if (modifier_->itemData(modifier_->currentIndex()) != QVariant::Invalid) {
		int key1 = modifier_->itemData(modifier_->currentIndex()).toInt();
		int key2 = key_->itemData(key_->currentIndex()).toInt();
		command_->keys_ = QKeySequence(key1, key2);
	}
	else {
		int key2 = key_->itemData(key_->currentIndex()).toInt();
		command_->keys_ = QKeySequence(key2);
	}
	
	command_->target_ = target_->itemData(target_->currentIndex()).toInt();
	command_->autoSaveFile_ = autoSaveFile_->isChecked();
	command_->autoOpenNextLink_ = autoOpenNextLink_->isChecked();
	command_->script_ = script_->text();
}

void QxCommandEditor::substTriggered(QAction* action)
{
	script_->edit()->paste(action->text());
}

} // namespace pacu
