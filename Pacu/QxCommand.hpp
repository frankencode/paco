#ifndef PACU_QXCOMMAND_HPP
#define PACU_QXCOMMAND_HPP

#include <QString>
#include <QPixmap>
#include <QAction>
#include <QKeySequence>
#include <QPointer>
#include "UseFtl.hpp"

namespace pacu
{

class QxCommand: public QObject
{
	Q_OBJECT
	
public:
	enum Target {
		ActiveTerminal,
		NewTerminal,
		// TextEditor,
		WebBrowser
	};
	
	QxCommand(QObject* parent);
	~QxCommand();
	
	void assignAction(QAction* action);
	void updateAction();
	QAction* action() const;
	
	void debug();
	
	QPixmap icon_;
	QString description_;
	QKeySequence keys_;
	int target_;
	bool autoSaveFile_;
	bool autoOpenNextLink_;
	QString script_;
	
	static int numAttributes();
	static QString attributeName(int k);
	QVariant attribute(int k) const;
	
	QString keysToString(QString sep = "+", bool reverse = false) const;
	
	QVariantList save() const;
	void restore(QVariantList list);
	
signals:
	void triggered(QxCommand* cmd);
	
private slots:
	void strike();
	
private:
	QPointer<QAction> action_;
};

} // namespace pacu

#endif // PACU_QXCOMMAND_HPP
