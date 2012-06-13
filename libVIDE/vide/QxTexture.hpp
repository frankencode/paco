#ifndef VIDE_QXTEXTURE_HPP
#define VIDE_QXTEXTURE_HPP

#include "QxPainter.hpp"
#include "UseFtl.hpp"

namespace vide
{

class QxTexture: public QObject, public Instance
{
	Q_OBJECT
	
public:
	QxTexture();
	QxTexture(const QBrush& brush);
	
	virtual void draw(QxPainter* p, int x, int y, int w, int h);
	
private:
	QBrush brush_;
};

} // namespace vide

#endif // VIDE_QXTEXTURE_HPP
