#include "QxTexture.hpp"

namespace vide
{

QxTexture::QxTexture()
{}

QxTexture::QxTexture(const QBrush& brush)
	: brush_(brush)
{}

void QxTexture::draw(QxPainter* p, int x, int y, int w, int h)
{
	if (brush_ != QBrush())
		p->fillRect(x, y, w, h, brush_);
}

} // namespace vide
