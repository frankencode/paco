#include "LanguageLayer.hpp"
#include "InsightParser.hpp"
#include "LanguageStack.hpp"

namespace pte
{

LanguageStack::LanguageStack(Ref<LanguageManager> languageManager, QString name, QString displayName, QStringList fileNamePattern)
	: languageManager_(languageManager),
	  name_(name),
	  displayName_(displayName),
	  fileNamePattern_(fileNamePattern),
	  layers_(new Layers)
{
	languageManager_->addStack(this);
}

QString LanguageStack::name() const { return name_; }
QString LanguageStack::displayName() const { return displayName_; }
QStringList LanguageStack::fileNamePattern() const { return fileNamePattern_; }
void LanguageStack::setFileNamePattern(QStringList pattern) { fileNamePattern_ = pattern; }

void LanguageStack::addLayer(Ref<LanguageLayer> layer) { layers_->append(layer); }
Ref<LanguageLayer> LanguageStack::layer(int i) const { return layers_->at(i); }
int LanguageStack::numLayers() const { return layers_->length(); }

Ref<InsightParser> LanguageStack::insightParser() const { return insightParser_; }
void LanguageStack::setInsightParser(Ref<InsightParser> parser) { insightParser_ = parser; }

} // namespace pte
