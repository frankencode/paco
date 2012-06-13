#ifndef PTE_LANGUAGEMANAGER_HPP
#define PTE_LANGUAGEMANAGER_HPP

#include <QString>
#include <ftl/Array.hpp>
#include <ftl/Map.hpp>
#include "UseFtl.hpp"
#include "SyntaxDefinition.hpp"

namespace pte
{

class LanguageLayer;
class LanguageStack;

class LanguageManager: public SyntaxDefinition::Scope, public virtual BackRefList
{
public:
	LanguageManager();
	
	Ref<LanguageLayer> layerByDefinitionId(int definitionId) const;
	Ref<LanguageLayer> layerByDefinitionName(QString definitionName) const;
	Ref<LanguageLayer> layerByIndex(int index) const;
	int numLayers() const;
	
	void addStack(Ref<LanguageStack> stack);
	Ref<LanguageStack> stackByIndex(int index) const;
	Ref<LanguageStack> stackByName(const QString& name) const;
	Ref<LanguageStack> stackByFileName(const QString& fileName) const;
	int numStacks() const;
	
private:
	friend class LanguageLayer;
	
	void addLayer(int definitionId, Ref<LanguageLayer> layer);
	virtual void commit(Ref<SyntaxDefinitionSuper> definition);
	
	typedef Array< Ref<LanguageLayer, Owner> > Layers;
	Ref<Layers, Owner> layers_;
	
	typedef Map<int, Ref<LanguageStack, Owner> > StackByIndex;
	typedef Map<QString, Ref<LanguageStack> > StackByName;
	Ref<StackByIndex, Owner> stackByIndex_;
	Ref<StackByName, Owner> stackByName_;
};

} // namespace pte

#endif // PTE_LANGUAGEMANAGER_HPP
