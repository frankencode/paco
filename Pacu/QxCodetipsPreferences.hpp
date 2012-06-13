#ifndef PACU_QXCODETIPSPREFERENCES_HPP
#define PACU_QXCODETIPSPREFERENCES_HPP

#include <QWidget>
#include <QPointer>
#include <ftl/Map.hpp>
#include <codetips/codetips>
#include "UseFtl.hpp"

class QComboBox;
class QLabel;
class QLineEdit;
class QStackedLayout;

namespace pacu
{

class QxVideoTerminal;
class QxPreferences;

class QxCodetipsPreferences: public QWidget
{
	Q_OBJECT
	
public:
	QxCodetipsPreferences(QxPreferences* preferences, QWidget* parent = 0);
	
private slots:
	void selectLanguage(int index);
	void selectAssistant(int index);
	void pathEditingFinished(QLineEdit* edit = 0);
	void pathChooser();
	
private:
	String locator(String language, String assistant);
	String locator(String language, String assistant, String resource);
	void generateResourceStack();
	QWidget* pathEdit(Variant value);
	
	QPointer<QxPreferences> preferences_;
	QComboBox* language_;
	QComboBox* assistant_;
	QStackedLayout* resourceStack_;
	QxVideoTerminal* log_;
	
	typedef Map<int, Ref<codetips::Assistant> > AssistantByStackIndex;
	typedef Map<String, int> StackIndexByLocator;
	typedef Map<QWidget*, String> LocatorByWidget;
	
	Ref<AssistantByStackIndex, Owner> assistantByStackIndex_;
	Ref<StackIndexByLocator, Owner> stackIndexByLocator_;
	Ref<LocatorByWidget, Owner> locatorByWidget_;
};

} // namespace pacu

#endif // PACU_QXCODETIPSPREFERENCES_HPP
