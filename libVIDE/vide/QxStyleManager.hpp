#ifndef VIDE_QXSTYLEMANAGER_HPP
#define VIDE_QXSTYLEMANAGER_HPP

#include <QMap>
#include <QWidget>
#include "UseFtl.hpp"
#include "QxStyle.hpp"

namespace vide
{

class QxControl;

class QxStyleManager: public QObject, public Instance
{
	Q_OBJECT
	
public:
	static QxStyleManager* instance();
	
	QxStyle* style(QString name) const;
	void setStyle(QString name, QxStyle* style);
	
	int constant(QString name) const;
	void setConstant(QString name, int value);
	
	QColor color(QString name) const;
	void setColor(QString name, QColor value);
	
	// layout helper
	QxControl* hl(QWidget* parent) const;
	QxControl* vl(QWidget* parent) const;
	QWidget* space(int w, int h, QWidget* parent);
	QWidget* hFill(QColor fill, int height, QWidget* parent);
	QWidget* hFill(QColor fill, QWidget* parent);
	
private slots:
	void disband();
	
private:
	QxStyleManager();
	
	static Ref<QxStyleManager, Owner> instance_;
	
	typedef QMap< QString, Ref<QxStyle, Owner> > Styles;
	Styles styles_;
	
	typedef QMap<QString, int> Constants;
	Constants constants_;
	
	typedef QMap<QString, QColor> Colors;
	Colors colors_;
};

QxStyleManager* styleManager();

} // namespace vide

#endif // VIDE_QXSTYLEMANAGER_HPP
