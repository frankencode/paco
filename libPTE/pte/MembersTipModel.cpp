#include <ftl/PrintDebug.hpp> // DEBUG
#include <QStringList>
#include "MembersTipModel.hpp"

namespace pte
{

using namespace codetips;

MembersTipModel::MembersTipModel(Ref<Members> members, QObject* parent)
	: QAbstractItemModel(parent),
	  members_(members)
{}

void MembersTipModel::currentChanged(const QModelIndex& index)
{
	Ref<Member> member = members_->at(index.row());
	Ref<Type> type = member->type();
	if (type)
		emit memberInfo(type->displayString().stripTags()->data());
	else
		emit memberInfo("");
}

QModelIndex MembersTipModel::index(int row, int column, const QModelIndex& parent) const
{
	if ((column != 0) || (row < 0) || (members_->size() <= row) || parent.isValid())
		return QModelIndex();
		/** NB: The underlying logic implies that there are multiple roots, which is contradicting
		  * the notion of a tree model and indeed this is meant be an array model! QAbstractItemModel
		  * exposes tree logic in its pure virtual interface, not because it expects a tree logic,
		  * but to enable it, if required. Therefore giving a valid parent must result into an invalid
		  * model index.
		  */
	return createIndex(row, column, &members_->at(column));
}

QModelIndex MembersTipModel::parent(const QModelIndex& index) const
{
	return QModelIndex();
}

int MembersTipModel::rowCount(const QModelIndex& parent) const
{
	return (parent.isValid()) ? 0 : members_->size();
}

int MembersTipModel::columnCount(const QModelIndex& parent) const
{
	return (parent.isValid()) ? 0 : 1;
}

bool MembersTipModel::hasChildren(const QModelIndex& parent) const
{
    return parent.isValid() ? false : (rowCount() > 0);
}

QVariant MembersTipModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid()) return QVariant();
	Ref<Member> member = members_->at(index.row());
	QVariant value;
	if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
		value = member->name()->data();
	// else if (role == Qt::DecorationRole)
	//   NB: it is important not to deliver a string as decoration role
	return value;
}

} // namespace pte
