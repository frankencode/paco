#ifndef PACU_QXCOMMANDPREFERENCES_HPP
#define PACU_QXCOMMANDPREFERENCES_HPP

#include <QWidget>
#include <QTreeView>
#include <QToolButton>

namespace pacu
{

class QxPreferences;

class QxCommandsPreferences: public QWidget
{
public:
	QxCommandsPreferences(QWidget* parent = 0);
	
private:
	friend class QxPreferences;
	
	QTreeView* view_;
	QToolButton* add_;
	QToolButton* del_;
	QToolButton* edit_;
	QToolButton* import_;
	QToolButton* export_;
};

} // namespace pacu

#endif // PACU_QXCOMMANDPREFERENCES_HPP
