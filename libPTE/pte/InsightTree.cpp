#include "InsightTree.hpp"

namespace pte
{

Ref<InsightTree::Children> InsightTree::children()
{
	if (!children_) {
		if (!firstChild()) return 0;
		int n = 0;
		Ref<InsightTree> child = firstChild();
		while (child) {
			++n;
			child = child->nextSibling();
		}
		children_ = new Children(n);
		child = firstChild();
		int i = 0;
		while (child) {
			children_->set(i, child);
			child->index_ = i;
			++i;
			child = child->nextSibling();
		}
	}
	return children_;
}

} // namespace pte
