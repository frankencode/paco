#ifndef PTE_TEXTELIDER_HPP
#define PTE_TEXTELIDER_HPP

#include <QWidget>
#include <QPointer>

namespace pte
{

/** Elides single lines of text to fit into a widget.
  * For instance useful in connection with a QLabel.
  */
class TextElider: public QObject
{
	Q_OBJECT
	
public:
	TextElider(QWidget* widget, Qt::TextElideMode mode = Qt::ElideRight);
	
signals:
	void textChanged(const QString& s);
	
public slots:
	void setText(const QString& s);
	
private:
	QPointer<QWidget> widget_;
	Qt::TextElideMode mode_;
};

} // namespace pte

#endif // PTE_TEXTELIDER_HPP
