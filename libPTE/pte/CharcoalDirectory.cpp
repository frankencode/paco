#include <QtScript>
#include <QResizeEvent>
#include <QApplication>
#include <ftl/streams>
#include <ftl/process>
#include <ftl/utils>
#include "View.hpp"
#include "ViewColors.hpp"
#include "CharcoalException.hpp"
#include "LanguageLayer.hpp"
#include "LanguageStack.hpp"
#include "InsightParser.hpp"
#include "LanguageManager.hpp"
#include "ThemeManager.hpp"
#include "PaletteManager.hpp"
#include "CharcoalDirectory.hpp"

namespace pte
{

String CharcoalDirectory::findCharcoalDir()
{
	Path path = Process::execPath();
	path = path.reduce().reduce();
	#ifdef Q_WS_MAC
	return path.expand("Resources").expand("charcoal");
	#else
	return path.expand("shared").expand("charcoal");
	#endif
}

CharcoalDirectory::CharcoalDirectory()
	: engine_(new QScriptEngine(this)),
	  path_(findCharcoalDir()),
	  languageManager_(new LanguageManager),
	  themeManager_(new ThemeManager(languageManager_)),
	  paletteManager_(new PaletteManager)
{
	ScriptableSyntaxDefinition::init(engine_);

	QScriptValue charcoal = engine_->newObject();
	charcoal.setProperty("syntax", engine_->newObject());
	charcoal.setProperty("themes", engine_->newObject());
	charcoal.setProperty("background", engine_->newObject());
	charcoal.setProperty("stacks", engine_->newObject());
	charcoal.setProperty("colors", engine_->newObject());
	engine_->globalObject().setProperty("charcoal", charcoal);

	const char* dirNames[] = { "syntax", "stacks", "themes", "colors" };
	const int numDirs = sizeof(dirNames) / sizeof(dirNames[0]);

	for (int i = 0; i < numDirs; ++i) {
		Dir dir(path_.expand(dirNames[i]));
		if (!dir.exists())
			throw CharcoalException(tr("Could not open directory '%1'").arg(dir.path().utf8()));

		Ref<DirEntry, Owner> entry = new DirEntry;
		while (dir.read(entry)) {
			if ((entry->name() == ".") || (entry->name() == "..") || (!entry->name().contains(".js"))) continue;
			readScript(entry->path());
		}
	}

	try {
		evalSyntax();
		languageManager_->link();
		evalStacks();
		evalThemes();
		evalColors();
	}
	catch (DebugException& ex) {
		throw CharcoalException(ex.what());
	}
}

Ref<LanguageManager> CharcoalDirectory::languageManager() const
{
	return languageManager_;
}

Ref<ThemeManager> CharcoalDirectory::themeManager() const
{
	return themeManager_;
}

Ref<PaletteManager> CharcoalDirectory::paletteManager() const
{
	return paletteManager_;
}

QString CharcoalDirectory::readText(String path)
{
	File file(path);
	int fileSize = file.size();
	Array<char> buf(fileSize + 1);
	file.open(File::Read);
	file.read(buf, fileSize);
	file.close();
	buf.set(buf.size() - 1, 0);
	return QString::fromUtf8(buf);
}

void CharcoalDirectory::checkError(String path)
{
	if (engine_->hasUncaughtException())
	{
		QString dirName = QString::fromUtf8(Path(path).reduce().fileName());
		QString fileName = QString::fromUtf8(Path(path).fileName());

		throw CharcoalException(
			QString::fromUtf8(path),
			engine_->uncaughtExceptionLineNumber(),
			tr("Script Exception: ") + engine_->uncaughtException().toString()
		);

		engine_->clearExceptions();
	}
}

void CharcoalDirectory::readScript(String path)
{
	QString text = readText(path);
	{
		QScriptSyntaxCheckResult result = QScriptEngine::checkSyntax(text);
		if (result.state() == QScriptSyntaxCheckResult::Error) {
			throw CharcoalException(
				QString::fromUtf8(path),
				result.errorLineNumber(),
				result.errorColumnNumber(),
				tr("Syntax Error: ") + result.errorMessage()
			);
		}
	}
	engine_->evaluate(text);
	checkError(path);
}

void CharcoalDirectory::evalSyntax()
{
	QScriptValue charcoal = engine_->globalObject().property("charcoal");
	QScriptValue syntax = charcoal.property("syntax");
	QScriptValueIterator syntaxIterator(syntax);
	while (syntaxIterator.hasNext()) {
		syntaxIterator.next();
		// print("Evaluating syntax %%\n", syntaxIterator.name().toUtf8().constData());
		Ref<ScriptableSyntaxDefinition, Owner> definition = new ScriptableSyntaxDefinition(languageManager_, syntaxIterator.name());
		QScriptValue thisObject = engine_->newQObject(definition);
		syntaxIterator.value().call(thisObject);
		checkError(path_.expand("syntax").expand(syntaxIterator.name().toUtf8().constData()));
		#ifdef PTE_DEBUG_SYNTAX
		print("-------------------------------------------\n");
		print("DEFINITION: \"%%\"\n", syntaxIterator.name().toUtf8().constData());
		print("-------------------------------------------\n");
		print("\n");
		definition->debugger()->printDefinition();
		print("\n");
		#endif
	}
}

void CharcoalDirectory::readStack(QString stackName, QScriptValue stackValue)
{
	if (!stackValue.property("title").isValid())
		throw CharcoalException(
			tr("Missing 'title' property in stack '%1'").arg(stackName)
		);
	if (!stackValue.property("syntax").isValid())
		throw CharcoalException(
			tr("Missing 'syntax' property in stack '%1'").arg(stackName)
		);
	if (!stackValue.property("filename").isValid())
		throw CharcoalException(
			tr("Missing 'filename' property in stack '%1'").arg(stackName)
		);

	QString title = stackValue.property("title").toString();
	QStringList syntax;
	QStringList filename;

	if (stackValue.property("syntax").isString()) {
		syntax << stackValue.property("syntax").toString();
	}
	else if (stackValue.property("syntax").isArray()) {
		QScriptValueIterator elementIterator(stackValue.property("syntax"));
		while (elementIterator.hasNext()) {
			elementIterator.next();
			if (elementIterator.flags() & QScriptValue::SkipInEnumeration)
				continue;
			syntax << elementIterator.value().toString();
		}
	}

	// qDebug() << "syntax =" << syntax;

	if (stackValue.property("filename").isString()) {
		filename << stackValue.property("filename").toString();
	}
	else if (stackValue.property("filename").isArray()) {
		QScriptValueIterator elementIterator(stackValue.property("filename"));
		while (elementIterator.hasNext()) {
			elementIterator.next();
			if (elementIterator.flags() & QScriptValue::SkipInEnumeration)
				continue;
			filename << elementIterator.value().toString();
		}
	}

	Ref<LanguageStack, Owner> stack = new LanguageStack(languageManager_, stackName, title, filename);
	for (int i = 0, n = syntax.length(); i < n; ++i) {
		QString definitionName = syntax.at(i);
		Ref<LanguageLayer> layer;
		try {
			layer = languageManager_->layerByDefinitionName(definitionName);
		}
		catch (...) {
		}
		if (!layer) {

			//throw CharcoalException(
			#ifndef NDEBUG
			qDebug() <<
				tr("In stack '%1': Can't find syntax definition '%2'").arg(stackName).arg(definitionName);
			#endif
			//);
		}
		else
			stack->addLayer(layer);
	}

	QScriptValue insight = stackValue.property("insight");
	if (insight.isValid()) {
		QScriptValue syntax = insight.property("syntax");
		if (!syntax.isValid())
			throw CharcoalException(
				tr("In stack '%1': Missing property 'insight.syntax'").arg(stackName)
			);
		if (!syntax.isString())
			throw CharcoalException(
				tr("In stack '%1': Can't interpret property 'insight.syntax'").arg(stackName)
			);
		QString definitionName = syntax.toString();
		Ref<LanguageLayer> layer = languageManager_->layerByDefinitionName(definitionName);
		if (!layer)
			throw CharcoalException(
				tr("In stack '%1': Can't find syntax definition 'insight.syntax' ('%2')").arg(stackName).arg(definitionName)
			);

		int definition = layer->syntax()->id();
		Ref<InsightParser, Owner> insightParser = new InsightParser(definition);
		stack->setInsightParser(insightParser);

		QScriptValue entities = insight.property("entities");
		if (!entities.isValid())
			throw CharcoalException(
				tr("In stack '%1': Missing property 'insight.entities'").arg(stackName)
			);
		if (!entities.isObject())
			throw CharcoalException(
				tr("In stack '%1': Can't interpret property 'insight.entities'").arg(stackName)
			);
		QScriptValueIterator entityIterator(entities);
		while (entityIterator.hasNext()) {
			entityIterator.next();
			QScriptValue entity = entityIterator.value();
			QString ruleName = entityIterator.name();
			QString identifierRuleName = entity.property("identifier").toString();
			int category = InsightTree::Undefined;
			int rule = layer->syntax()->ruleByName(ruleName.toUtf8().constData())->id();
			int identifierRule =  entity.property("identifier").isValid() ? layer->syntax()->ruleByName(identifierRuleName.toUtf8().constData())->id() : -1;
			if (entity.property("category").isValid()) {
				QString categoryString = entity.property("category").toString();
				if (categoryString == "Void")
					category = InsightTree::Void;
				else if (categoryString == "Function")
					category = InsightTree::Function;
				else if (categoryString == "Variable")
					category = InsightTree::Variable;
				else if (categoryString == "Class")
					category = InsightTree::Class;
				else if (categoryString == "Module")
					category = InsightTree::Module;
				else
					throw CharcoalException(
						tr("In stack '%1': Insight category '%2' undefined").arg(stackName).arg(categoryString)
					);
			}
			insightParser->addEntity(category, rule, identifierRule);
			// qDebug() << "category, rule, identifierRule =" << category << "," << rule << "," << identifierRule;
		}
	}
}

void CharcoalDirectory::evalStacks()
{
	QScriptValue charcoal = engine_->globalObject().property("charcoal");
	QScriptValue stacks = charcoal.property("stacks");

	QScriptValue plainStackValue;

	QScriptValueIterator stackIterator(stacks);
	while (stackIterator.hasNext())
	{
		stackIterator.next();
		// qDebug() << "Evaluating stack" << stackIterator.name();
		QString stackName = stackIterator.name();
		QScriptValue stackValue = stackIterator.value();

		if (stackName.toLower() == "plain")
			plainStackValue = stackValue;
		else
			readStack(stackName, stackValue);
	}

	if (plainStackValue.isValid())
		readStack("plain", plainStackValue);
}

void CharcoalDirectory::evalThemes()
{
	QScriptValue charcoal = engine_->globalObject().property("charcoal");
	QScriptValue themes = charcoal.property("themes");

	QScriptValueIterator themeIterator(themes);
	while (themeIterator.hasNext())
	{
		themeIterator.next();
		// qDebug() << "Evaluating theme" << themeIterator.name();

		QString name = themeIterator.name();
		QString displayName = themeIterator.value().property("displayName").toString();
		bool hasSuperTheme = themeIterator.value().property("superTheme").isValid();
		QString superThemeName = themeIterator.value().property("superTheme").toString();

		if (themeManager_->themeByName(name))
			throw CharcoalException(
				tr("Theme name '%1' used twice for two different themes").arg(name)
			);

		Ref<HighlightingTheme> superTheme;
		if (hasSuperTheme) superTheme = themeManager_->themeByName(superThemeName);

		Ref<HighlightingTheme> theme = new HighlightingTheme(themeManager_, name, displayName, superTheme);

		QScriptValueIterator layerIterator(themeIterator.value().property("layers"));
		while (layerIterator.hasNext())
		{
			layerIterator.next();

			Ref<LanguageLayer> layer = languageManager_->layerByDefinitionName(layerIterator.name());
			// qDebug() << "  Reading layer" << layerIterator.name();
			QScriptValueIterator styleIterator(layerIterator.value());
			while (styleIterator.hasNext())
			{
				styleIterator.next();
				QScriptValue style = styleIterator.value();
				QColor color; bool bold = false;

				if (style.isString()) {
					QString s = style.toString();
					if (s.length() > 0) {
						if (s.at(0) == '=') {

						}
						else {
							color.setNamedColor(s);
						}
					}
				}
				else if (style.isObject()) {
					QScriptValueIterator attributeIterator(style);
					while (attributeIterator.hasNext()) {
						attributeIterator.next();
						if (attributeIterator.name() == "color")
							color.setNamedColor(attributeIterator.value().toString());
						else if (attributeIterator.name() == "bold")
							bold = attributeIterator.value().toBool();
					}
				}

				// qDebug() << "    Style" << styleIterator.name() << ":" << color << bold;
				new SyntaxStyle(theme, layer, styleIterator.name(), color, bold);
			}
		}

		if (themeIterator.value().property("editor").isValid())
		{
			Ref<ViewColors, Owner> colors = new ViewColors;
			colors->selectionColor_ = qApp->palette().color(QPalette::Highlight);
			QScriptValueIterator pi(themeIterator.value().property("editor"));
			bool hasLineNumbersBackgroundBright = false;
			bool hasCurrentLineColor = false;
			while (pi.hasNext()) {
				pi.next();
				QColor color = QColor(pi.value().toString());
				if (pi.name() == "foreground")
					colors->foregroundColor_ = color;
				else if (pi.name() == "background")
					colors->backgroundColor_ = color;
				else if (pi.name() == "cursor")
					colors->cursorColor_ = color;
				else if (pi.name() == "lineNumbers")
					colors->lineNumbersFgColor_ = color;
				else if (pi.name() == "lineNumbersBright")
					colors->lineNumbersFgColorBright_ = color;
				else if (pi.name() == "lineNumbersBackground")
					colors->lineNumbersBgColor_ = color;
				else if (pi.name() == "lineNumbersBackgroundBright") {
					colors->lineNumbersBgColorBright_ = color;
					hasLineNumbersBackgroundBright = true;
				}
				else if (pi.name() == "selection")
					colors->selectionColor_ = color;
				else if (pi.name() == "match")
					colors->matchColor_ = color;
				else if (pi.name() == "bracketMatch")
					colors->bracketMatchColor_ = color;
				else if (pi.name() == "currentLine") {
					colors->currentLineColor_ = color;
					hasCurrentLineColor = true;
				}
			}
			/*
			if ((!hasLineNumbersBackgroundBright) && hasCurrentLineColor) {
				qreal hue, sat, ign;
				colors->currentLineColor_.getHslF(&hue, &sat, &ign);
				{
					qreal h, s, l;
					colors->lineNumbersBgColorBright_.getHslF(&ign, &s, &l);
					sat /= 2;
					colors->lineNumbersBgColorBright_.setHslF(hue, sat, l);
				}
			}*/

			theme->setViewColors(colors);
		}
	}

	themeManager_->sortThemesByDisplayName();

	int defaultIndex = themeManager_->defaultThemeIndex();
	if (defaultIndex == -1)
		throw CharcoalException(
			tr("No 'Default' theme defined")
		);
}

void CharcoalDirectory::evalColors()
{
	QScriptValue charcoal = engine_->globalObject().property("charcoal");
	QScriptValue palette = charcoal.property("colors");
	QScriptValueIterator paletteIterator(palette);
	while (paletteIterator.hasNext()) {
		paletteIterator.next();
		QString name = paletteIterator.name();
		QScriptValue value = paletteIterator.value();
		QString displayName = value.property("displayName").toString();
		QScriptValue colors = value.property("colors");
		int numColors = 0; {
			QScriptValueIterator colorIterator(colors);
			while (colorIterator.hasNext()) {
				colorIterator.next();
				++numColors;
			}
		}
		Ref<Palette, Owner> palette = new Palette(
			paletteManager_,
			name,
			displayName,
			numColors
		);
		QScriptValueIterator colorIterator(colors);
		while (colorIterator.hasNext()) {
			colorIterator.next();
			QString name = colorIterator.name();
			QString value = colorIterator.value().toString();
			int alpha = 0xFF;
			if (value.length() == 9) {
				if (value.at(0) == '#') {
					bool ok = true;
					alpha = value.mid(7, 2).toInt(&ok, 16);
					if (ok)
						value = value.mid(0, 7);
				}
			}
			QColor color = QColor(value);
			if (alpha != 0xFF)
				color.setAlpha(alpha);
			palette->colorByIndex_->set(palette->colorByName_->size(), color);
			palette->colorByName_->insert(name, color);
		}
	}

	paletteManager_->sortPalettesByDisplayName();
}

QPixmap CharcoalDirectory::themePreview(Ref<HighlightingTheme> theme) const
{
	Dir themes(path_.expand("themes"));
	Ref<DirEntry, Owner> entryFound = new DirEntry;
	Ref<DirEntry, Owner> entry = new DirEntry;
	while (themes.read(entry)) {
		if (entry->name().contains(".sample")) {
			if (entry->name().contains(theme->name().toUtf8().constData())) {
				entryFound = entry;
				break;
			}
			if (entry->name().contains("Default"))
				*entryFound = *entry;
		}
	}

	if (!entryFound) return QPixmap();

	View* view = new View;
	Ref<Document, Owner> document = new Document;

	QString samplePath = QString::fromUtf8(entryFound->path());

	{
		QFile file(samplePath);
		if (!file.open(QIODevice::ReadOnly))
			return QPixmap();
		{
			QTextStream source(&file);
			source.setCodec(QTextCodec::codecForName("UTF-8"));
			document->load(&source);
		}
	}

	Ref<HighlightingTheme> themeSaved = themeManager_->activeTheme();
	themeManager_->activateTheme(theme);

	view->setDocument(document);
	view->setColors(theme->viewColors());
	{
		samplePath.replace(".sample", "");
		Ref<LanguageStack> stack = languageManager_->stackByFileName(QFileInfo(samplePath).fileName());
		if (stack)
			view->setHighlighter(new Highlighter(stack, document));
	}
	view->highlighterYield();
	view->highlighterSync();
	view->highlighterResume();

	QPixmap preview(256, 256);
	#ifdef QT_MAC_USE_COCOA
	preview.fill(Qt::transparent);
	#endif
	view->resize(preview.width(), preview.height());
	{	// paranoid HACK
		QResizeEvent event(preview.size(), preview.size());
		QApplication::sendEvent(view, &event);
	}
	view->render(&preview);

	themeManager_->activateTheme(themeSaved);

	return preview.scaled(preview.size()/2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

} // namespace pte
