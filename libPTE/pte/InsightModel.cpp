// #include <QDebug> // DEBUG
#include <ftl/streams> // DEBUG
#include "SequenceAdapter.hpp"
// #include "Document.hpp"
#include "LanguageLayer.hpp"
#include "LanguageStack.hpp"
#include "InsightParser.hpp"
#include "InsightModel.hpp"

namespace pte
{

InsightModel::InsightModel(Ref<Document> document)
	: document_(document),
	  needsUpdate_(true)
{
	connect(document, SIGNAL(changed(Ref<Delta, Owner>)), this, SLOT(takeNoticeOfDocumentChanged(Ref<Delta, Owner>)));
	reload();
}

Ref<Document> InsightModel::document() const
{
	return document_;
}

void InsightModel::reload()
{
	if (!needsUpdate_) return;
	needsUpdate_ = false;
	
	beginResetModel();
	document_->highlighterYield();
	
	Ref<LanguageStack> stack = document_->highlighter()->languageStack();
	Ref<Token, Owner> matchTree;
	for (int depth = 0; depth < stack->numLayers(); ++depth) {
		Ref<LanguageLayer> layer = stack->layer(depth);
		SequenceAdapter source(document_, false);
		Ref<Token, Owner> newTree = layer->syntax()->match(&source, 0, 0, 0, &source);
		
		if (matchTree)
			Token::meld(newTree, matchTree);
		matchTree = newTree;
	}
	
	/*#ifndef NDEBUG
	{
		SequenceAdapter source(document_, false);
		source.def(0);
		printTree(matchTree, &source);
	}
	#endif*/
	
	{
		SequenceAdapter source(document_, false);
		insightTree_ = stack->insightParser()->parse(matchTree, &source);
	}
	
	/*#ifndef NDEBUG
	if (insightTree_)
		printTree(insightTree_);
	#endif*/
	
	document_->highlighterResume();
	endResetModel();
}

void InsightModel::printTree(Ref<Token> tree, Ref<SequenceAdapter> source, int depth)
{
	for (int i = 0; i < depth; ++i) print(" ");
	print("[%%, %%:%%] ", tree->rule(), tree->i0(), tree->i1());
	if (tree->firstChild())
		print("\n");
	else
		print("\"%%\"\n", source->copy(tree).toUtf8().constData());
	Ref<Token> child = tree->firstChild();
	while (child) {
		printTree(child, source, depth + 1);
		child = child->nextSibling();
	}
}

void InsightModel::printTree(Ref<InsightTree> tree, int depth)
{
	for (int i = 0; i < depth; ++i) print("  ");
	printNode(tree);
	Ref<InsightTree> node = tree->firstChild();
	while (node) {
		printTree(node, depth + 1);
		node = node->nextSibling();
	}
}

void InsightModel::printNode(Ref<InsightTree> tree) const
{
	String categoryName = "undefined";
	if (tree->category_ == InsightTree::Variable)
		categoryName = "variable";
	else if (tree->category_ == InsightTree::Function)
		categoryName = "function";
	else if (tree->category_ == InsightTree::Class)
		categoryName = "class";
	else if (tree->category_ == InsightTree::Module)
		categoryName = "module";
	print("[%%: %% \"%%\"]\n", tree->yHint_ + 1, categoryName, tree->identifier_.toUtf8().constData());
}

QModelIndex InsightModel::index(int row, int column, const QModelIndex& index) const
{
	// print("InsightModel::index(): row, column, ptr = %%, %%, %%\n", row, column, index.internalPointer());
	if (!insightTree_) return QModelIndex();
	if (!hasIndex(row, column, index)) return QModelIndex();
	if (!index.isValid()) return createIndex(row, column, insightTree_);
	if (column != 0) return QModelIndex();
	InsightTree* tree = static_cast<InsightTree*>(index.internalPointer());
	if (!tree->children()) return QModelIndex();
	if ((row < 0) || (tree->children()->size() <= row)) return QModelIndex();
	InsightTree* child = tree->children()->at(row);
	// print("child = "); printNode(child);
	return createIndex(row, column, child);
}

bool InsightModel::hasChildren(const QModelIndex& index) const
{
	if (!insightTree_) return false;
	InsightTree* tree = insightTree_;
	if (index.isValid())
		tree = static_cast<InsightTree*>(index.internalPointer());
	return tree->children();
}

QModelIndex InsightModel::parent(const QModelIndex& index) const
{
	// print("InsightModel::parent(): row, column, ptr = %%, %%, %%\n", index.row(), index.column(), index.internalPointer());
	if (!insightTree_) return QModelIndex();
	if (!index.isValid()) return QModelIndex();
	InsightTree* tree = static_cast<InsightTree*>(index.internalPointer());
	Ref<InsightTree> parent = tree->parent();
	if (!parent) return QModelIndex();
	Ref<InsightTree> grandParent = parent->parent();
	if (grandParent) grandParent->children();
	// if (tree) print("tree = "); printNode(tree);
	// if (parent) print("parent = "); printNode(parent);
	return createIndex(parent->index_, 0, parent);
}

int InsightModel::rowCount(const QModelIndex& index) const
{
	// print("InsightModel::rowCount(): row, column, ptr = %%, %%, %%\n", index.row(), index.column(), index.internalPointer());
	if (!insightTree_) return 0;
	if (index.column() > 0) return 0;
	if (!index.isValid()) return 1;
	InsightTree* tree = static_cast<InsightTree*>(index.internalPointer());
	return tree->children() ? tree->children()->size() : 0;
}

int InsightModel::columnCount(const QModelIndex& index) const
{
	// print("InsightModel::columnCount(): row, column, ptr = %%, %%, %%\n", index.row(), index.column(), index.internalPointer());
	if (!insightTree_) return 0;
	return 1;
}

QVariant InsightModel::data(const QModelIndex& index, int role) const
{
	// print("InsightModel::data(): row, column, ptr = %%, %%, %%\n", index.row(), index.column(), index.internalPointer());
	if (!insightTree_) return QVariant();
	if (!index.isValid()) return QVariant();
	InsightTree* tree = static_cast<InsightTree*>(index.internalPointer());
	if (index.column() != 0) return QVariant();
	if (role == Qt::DisplayRole) {
		if (tree->category_ == InsightTree::Function)
			return tree->identifier_ + "()";
		return tree->identifier_;
	}
	return QVariant();
}

void InsightModel::takeNoticeOfDocumentChanged(Ref<Delta, Owner>)
{
	needsUpdate_ = true;
}

} // namespace pte
