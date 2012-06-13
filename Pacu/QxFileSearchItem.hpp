#ifndef PACU_QXFILESEARCHITEM_HPP
#define PACU_QXFILESEARCHITEM_HPP

#include "UseVide.hpp"

namespace pacu
{

class QxFileSearchItem: public QxControl
{
	Q_OBJECT
	
public:
	QxFileSearchItem(QWidget* parent, QPixmap icon, QString label, QString details);
	
	QString details() const;
	
signals:
	void selected(QString label);
	void submitted(QString details);
	
private slots:
	void activate();
	void forward();
	void submit();
	
private:
	QString label_;
	QString details_;
	bool ignoreFirstTime_;
};

} // namespace pacu

#endif // PACU_FILESEARCHITEM_HPP
