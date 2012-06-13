#ifndef PTE_MEMBERTIPMODEL_HPP
#define PTE_MEMBERTIPMODEL_HPP

#include <QAbstractItemModel>
#include <codetips/codetips>
#include <ftl/Array.hpp>
#include "UseFtl.hpp"

namespace pte
{

class MembersTipModel: public QAbstractItemModel
{
	Q_OBJECT
	
public:
	MembersTipModel(Ref<codetips::Members> members, QObject* parent = 0);
	
signals:
	void memberInfo(const QString& text);
	
public slots:
	void currentChanged(const QModelIndex& index);
	
private:
	virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex& index) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
	virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	
	Ref<codetips::Members, Owner> members_;
};

} // namespace pte

#endif // PTE_MEMBERTIPMODEL_HPP
