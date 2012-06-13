#ifndef PTE_QUICKHELP_HPP
#define PTE_QUICKHELP_HPP

#include <QFrame>
#include <QPointer>
#include <QEvent>

namespace pte
{

class QuickHelp: public QFrame
{
	Q_OBJECT
	
public:
	QuickHelp(QWidget* widget, QPoint pos, QString text);

private slots:
	void openLink(const QString& path);
	
private:
	bool eventFilter(QObject* obj, QEvent* event);
	
	QPointer<QWidget> widget_;
	QPointer<QWidget> window_;
};

} // namespace pte

#endif // PTE_QUICKHELP_HPP
