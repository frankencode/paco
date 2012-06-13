#ifndef PACU_QXCOMBOBOX_HPP
#define PACU_QXCOMBOBOX_HPP

#include <QComboBox>
#include <QEvent>

namespace pacu
{

class QxComboBox: public QComboBox
{
	Q_OBJECT
	
public:
	QxComboBox(QWidget* parent = 0);
/*#ifdef Q_WS_MAC
#ifndef QT_MAC_USE_COCOA
	virtual QSize sizeHint() const;
#endif
#endif*/
/*#ifdef Q_WS_MAC
	virtual QSize minimumSizeHint() const;
#endif*/
	bool event(QEvent* event);
	
signals:
	void escape();
};

} // namespace pacu

#endif // PACU_QXCOMBOBOX_HPP
