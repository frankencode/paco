#ifndef PACU_QXCOMMANDEDITOR_HPP
#define PACU_QXCOMMANDEDITOR_HPP

#include <QPointer>
#include "QxDialog.hpp"
#include "QxCommand.hpp"
#include "UsePte.hpp"

class QToolButton;
class QPushButton;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QLabel;
class QTextEdit;

namespace pte {
	class EditWidget;
	class Edit;
}

namespace pacu
{

class QxIconSelector;

class QxCommandEditor: public QxDialog
{
	Q_OBJECT
	
public:
	QxCommandEditor(QWidget* parent);
	
	QxCommand* command() const;
	void setCommand(QxCommand* cmd);
	
	Edit* scriptEdit() const;
	
private slots:
	void updateKey(int);
	void updateTarget(int target);
	void loadIcon();
	void resetIcon();
	void updateCommand();
	void substTriggered(QAction* action);
	
private:
	QToolButton* icon_;
	QToolButton* iconReset_;
	QString noDescription_;
	QLineEdit* description_;
	QComboBox* modifier_;
	QComboBox* keyGroup_;
	QComboBox* key_;
	QComboBox* target_;
	QCheckBox* autoSaveFile_;
	QCheckBox* autoOpenNextLink_;
	QLabel* scriptLabel_;
	EditWidget* script_;
	QxIconSelector* iconSelector_;
	QPointer<QxCommand> command_;
};

} // namespace pacu

#endif // PACU_QXCOMMANDEDITOR_HPP
