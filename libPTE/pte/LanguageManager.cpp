#include "LanguageLayer.hpp"
#include "LanguageStack.hpp"
#include "SyntaxDefinition.hpp"
#include "LanguageManager.hpp"

namespace pte
{

LanguageManager::LanguageManager()
	: stackByIndex_(new StackByIndex),
	  stackByName_(new StackByName)
{}

Ref<LanguageLayer> LanguageManager::layerByDefinitionId(int definitionId) const
{
	return layers_->get(definitionId);
}

Ref<LanguageLayer> LanguageManager::layerByDefinitionName(QString definitionName) const
{
	return layers_->get(definitionByName(definitionName.toUtf8())->id());
}

Ref<LanguageLayer> LanguageManager::layerByIndex(int index) const
{
	return layers_->get(index);
}

int LanguageManager::numLayers() const
{
	return layers_->size();
}

void LanguageManager::addStack(Ref<LanguageStack> stack)
{
	stackByIndex_->insert(stackByIndex_->size(), stack);
	stackByName_->insert(stack->name(), stack);
}

Ref<LanguageStack> LanguageManager::stackByIndex(int index) const
{
	return stackByIndex_->value(index);
}

Ref<LanguageStack> LanguageManager::stackByName(const QString& name) const
{
	return stackByName_->value(name);
}

Ref<LanguageStack> LanguageManager::stackByFileName(const QString& fileName) const
{
	Ref<LanguageStack> approved;
	int approvalLength = 0;
	
	for (int i = 0; i < stackByIndex_->size(); ++i) {
		Ref<LanguageStack> candidate = stackByIndex_->value(i);
		QStringList pattern = candidate->fileNamePattern();
		
		for (int i = 0, n = pattern.size(); i < n; ++i) {
			if (
				QRegExp(pattern.at(i), Qt::CaseInsensitive, QRegExp::Wildcard).exactMatch(fileName) &&
				(pattern.at(i).length() > approvalLength)
			) {
				approved = candidate;
				approvalLength = pattern.at(i).length();
				break;
			}
		}
	}
	
	return approved;
}

int LanguageManager::numStacks() const
{
	return stackByIndex_->size();
}

void LanguageManager::addLayer(int definitionId, Ref<LanguageLayer> layer)
{
	layers_->set(definitionId, layer);
}

void LanguageManager::commit(Ref<SyntaxDefinitionSuper> definition)
{
	if (!layers_)
		layers_ = new Layers(numDefinitions());
	new LanguageLayer(this, definition);
}

} // namespace pte
