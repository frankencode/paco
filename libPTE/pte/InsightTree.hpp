#ifndef PTE_INSIGHTTREE_HPP
#define PTE_INSIGHTTREE_HPP

#include <QString>
#include <ftl/Tree.hpp>
#include <ftl/Array.hpp>
#include "UseFtl.hpp"

namespace pte
{

class InsightTree;

class InsightTree: public Tree<InsightTree>
{
public:
	enum Category {
		Void,
		Function,
		Variable,
		Class,
		Module,
		Undefined
	};
	
	InsightTree(int category)
		: category_(category),
		  yHint_(0),
		  index_(0)
	{}
	
	int category_;
	QString identifier_;
	int yHint_;
	
	typedef Array< Ref<InsightTree> > Children;
	Ref<Children, Owner> children_;
	int index_;
	
	Ref<Children> children();
};

} // namespace pte

#endif // PTE_INSIGHTTREE_HPP
