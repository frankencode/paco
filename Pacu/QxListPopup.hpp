#ifndef PACU_QXLISTPOPUP_HPP
#define PACU_QXLISTPOPUP_HPP

#include <QLayout>
#include <pte/CustomList.hpp>
#include "UsePte.hpp"
#include "UseVide.hpp"

namespace pacu
{

class QxListPopup: public QxControl, public CustomListObserver< Ref<QxControl> >
{
	Q_OBJECT
	
public:
	QxListPopup(QxControl* combo, int align = Qt::AlignLeft);
	
	typedef CustomList< Ref<QxControl> > ItemList;
	ItemList* itemList() const;
	
	QxControl* selectedItem() const;
	
signals:
	void stopResponseTimer();
	
public slots:
	void show();
	void unselectAll();
	void rotateSelection();
	void backRotateSelection();
	
private:
	virtual bool eventFilter(QObject* watched, QEvent* event);
	
	QxControl* hl();
	QxControl* vl();
	
	virtual void afterPush(int i, int n);
	virtual bool beforePop(int i, int n);
	
	Ref<QxControl> combo_;
	int align_;
	Ref<ItemList, Owner> itemList_;
	Ref<ItemList, Owner> itemSeparators_;
	QVBoxLayout* itemLayout_;
	int maxItemWidth_;
};

} // namespace pacu

#endif // PACU_QXLISTPOPUP_HPP
