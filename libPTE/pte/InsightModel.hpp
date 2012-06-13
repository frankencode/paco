#ifndef PTE_INSIGHTMODEL_HPP
#define PTE_INSIGHTMODEL_HPP

#include <QAbstractItemModel>
#include "UseFtl.hpp"
#include "Document.hpp" // to please moc
#include "InsightTree.hpp" // DEBUG

class QTreeView;

namespace pte
{

class Document;
class SequenceAdapter;

class InsightModel: public QAbstractItemModel, public Instance
{
	Q_OBJECT
	
public:
	InsightModel(Ref<Document> document);
	
	Ref<Document> document() const;
	void reload();
	
	virtual QModelIndex index(int row, int column, const QModelIndex& index = QModelIndex()) const;
	virtual bool hasChildren(const QModelIndex& index = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex& index) const;
	virtual int rowCount(const QModelIndex& index = QModelIndex()) const;
	virtual int columnCount(const QModelIndex& index = QModelIndex()) const;
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	
private slots:
	void takeNoticeOfDocumentChanged(Ref<Delta, Owner>);
	
private:
	void printTree(Ref<Token> tree, Ref<SequenceAdapter> source, int depth = 0); // DEBUG
	void printTree(Ref<InsightTree> tree, int depth = 0); // DEBUG
	void printNode(Ref<InsightTree> tree) const;
	
	Ref<Document> document_;
	Ref<InsightTree, Owner> insightTree_;
	bool needsUpdate_;
};

} // namespace pte

#endif // PTE_INSIGHTMODEL_HPP
