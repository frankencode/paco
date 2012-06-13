#ifndef PTE_INSIGHTPARSER_HPP
#define PTE_INSIGHTPARSER_HPP

#include <ftl/List.hpp>
#include <ftl/Token.hpp>
#include "UseFtl.hpp"
#include "InsightTree.hpp"
#include "SequenceAdapter.hpp"

namespace pte
{

class InsightParser: public Instance
{
public:
	InsightParser(int definition);
	void addEntity(int category, int rule, int indentifierRule);
	
	Ref<InsightTree, Owner> parse(Ref<Token> token, Ref<SequenceAdapter> source, Ref<InsightTree> insightTree = 0);
	
	int definition_;
	
	class Entity {
	public:
		Entity()
			: category_(InsightTree::Undefined),
			  rule_(-1),
			  identifierRule_(-1)
		{}
		Entity(int category, int rule, int identifierRule)
			: category_(category),
			  rule_(rule),
			  identifierRule_(identifierRule)
		{}
		int category_;
		int rule_;
		int identifierRule_;
	};
	
	typedef List<Entity> Entities;
	Ref<Entities> entities_;
};

} // namespace pte

#endif // PTE_INSIGHTPARSER_HPP
