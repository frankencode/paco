#ifndef PTE_LANGUAGESTACK_HPP
#define PTE_LANGUAGESTACK_HPP

#include <ftl/List.hpp>
#include "UseFtl.hpp"
#include "Atoms.hpp"
// #include "CustomList.hpp"
#include "LanguageManager.hpp"

namespace pte
{

class LanguageLayer;
class InsightParser;

class LanguageStack: public Instance
{
public:
	LanguageStack(Ref<LanguageManager> languageManager, QString name, QString displayName, QStringList fileNamePattern);
	
	inline Ref<LanguageManager> languageManager() const { return languageManager_; }
	
	QString name() const;
	QString displayName() const;
	QStringList fileNamePattern() const;
	void setFileNamePattern(QStringList pattern);
	
	void addLayer(Ref<LanguageLayer> layer);
	Ref<LanguageLayer> layer(int i) const;
	int numLayers() const;
	
	Ref<InsightParser> insightParser() const;
	void setInsightParser(Ref<InsightParser> parser);
	
private:
	Ref<LanguageManager> languageManager_;
	
	QString name_;
	QString displayName_;
	QStringList fileNamePattern_;
	
	typedef List< Ref<LanguageLayer, SetNull> > Layers;
	Ref<Layers> layers_;
	
	Ref<InsightParser, Owner> insightParser_;
};

} // namespace pte

#endif // PTE_LANGUAGESTACK_HPP
