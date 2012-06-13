#ifndef PTE_QXTEXTLAYOUT_HPP
#define PTE_QXTEXTLAYOUT_HPP

#include <QTextLayout>

namespace pte
{

class QxTextLayout: public QTextLayout, public Instance
{
public:
	QxTextLayout(const QString& text, const QFont& font, QPaintDevice* device);
};

} // namespace pte

#endif // PTE_QXTEXTLAYOUT_HPP
