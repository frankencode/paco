#ifndef PACU_QXTABLEMODELWRAPPER_HPP
#define PACU_QXTABLEMODELWRAPPER_HPP

#include <QAbstractTableModel>
#include <pte/CustomListObserver.hpp>
#include "UsePte.hpp"

namespace pacu
{

template<class T>
class QxTableModelWrapper: public QAbstractTableModel, public CustomListObserver<T>
{
public:
	virtual QVariant attribute(int i, int k) const = 0;
	virtual QString attributeName(int k) const = 0;
	virtual int numAttributes() const = 0;
	virtual int length() const = 0;
	
private:
	virtual void afterPush(int i, int n) {
		if (n <= 0) return;
		beginInsertRows(QModelIndex(), i, i + n - 1);
		endInsertRows();
	}
	virtual void afterPop(int i, int n) {
		if (n <= 0) return;
		beginRemoveRows(QModelIndex(), i, i + n - 1);
		endRemoveRows();
	}
	virtual void afterWrite(int i, int n) {
		dataChanged(index(i, 0), index(i + n - 1, numAttributes() - 1));
	}
	
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const {
		QVariant value;
		if (role == Qt::DisplayRole)
			value = attribute(index.row(), index.column());
		return value;
	}
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const {
		QVariant value;
		if ((role == Qt::DisplayRole) && (orientation == Qt::Horizontal))
			value = attributeName(section);
		return value;
	}
	virtual int rowCount(const QModelIndex& parent) const {
		return parent.isValid() ? 0 : length();
	}
	virtual int columnCount(const QModelIndex& parent) const {
		return parent.isValid() ? 0 : numAttributes();
	}
};

} // namespace pacu

#endif // PACU_QXTABLEMODELWRAPPER_HPP
