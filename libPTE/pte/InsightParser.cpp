#include "InsightParser.hpp"

namespace pte
{

InsightParser::InsightParser(int definition)
	: definition_(definition),
	  entities_(new Entities)
{}

void InsightParser::addEntity(int category, int rule, int identifierRule)
{
	entities_->append(Entity(category, rule, identifierRule));
}

Ref<InsightTree, Owner> InsightParser::parse(Ref<Token> token, Ref<SequenceAdapter> source, Ref<InsightTree> insightTree)
{
	Ref<InsightTree, Owner> insightRoot;
	
	for (int i = 0; i < entities_->length(); ++i)
	{
		Entity e = entities_->at(i);
		
		if ((token->definition() == definition_) && (token->rule() == e.rule_))
		{
			Ref<InsightTree, Owner> insightNode = new InsightTree(e.category_);
			if (!insightTree)
				insightRoot = insightNode;
			
			if ((e.category_ != InsightTree::Void) || (!insightTree))
			{
				if (insightTree)
					insightTree->appendChild(insightNode);
				
				// find identifier
				Ref<Token> child = token->firstChild();
				while (child) {
					if ((child->definition() == definition_) && (child->rule() == e.identifierRule_)) {
						insightNode->identifier_ = source->copy(child, &insightNode->yHint_);
						break;
					}
					child = child->nextSibling();
				}
			}
			else {
				insightNode = insightTree;
			}
			
			if (e.category_ != InsightTree::Function) { // quick logic, HACK (should be user-configurable)
				// cascade
				Ref<Token> child = token->firstChild();
				while (child) {
					parse(child, source, insightNode);
					child = child->nextSibling();
				}
			}
		}
	}
	
	return insightRoot;
}

} // namespace pte
