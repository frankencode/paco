// #include <QDebug>
#include <QByteArray>
#include <QMap>
#include <ftl/Mutex.hpp>
#include <ftl/ScopeGuard.hpp>
#include <ftl/LocalStatic.hpp>
#include "CharcoalException.hpp"
#include "ScriptableSyntaxDefinition.hpp"

namespace pte
{

void ScriptableSyntaxDefinition::init(QScriptEngine* engine)
{
	qScriptRegisterMetaType<NODE>(engine, nodeToScriptValue, nodeFromScriptValue);
	qScriptRegisterMetaType<RULE>(engine, ruleToScriptValue, ruleFromScriptValue);
	qScriptRegisterMetaType<const char*>(engine, constCharToScriptValue, constCharFromScriptValue);
	engine->globalObject().setProperty("OPTION", engine->newFunction(OPTION_));
	engine->globalObject().setProperty("CHAR", engine->newFunction(CHAR_));
	engine->globalObject().setProperty("OTHER", engine->newFunction(OTHER_));
	engine->globalObject().setProperty("ANY", engine->newFunction(ANY_));
	engine->globalObject().setProperty("RANGE", engine->newFunction(RANGE_));
	engine->globalObject().setProperty("EXCEPT", engine->newFunction(EXCEPT_));
	engine->globalObject().setProperty("STRING", engine->newFunction(STRING_));
	engine->globalObject().setProperty("REPEAT", engine->newFunction(REPEAT_));
	engine->globalObject().setProperty("BOI", engine->newFunction(BOI_));
	engine->globalObject().setProperty("EOI", engine->newFunction(EOI_));
	engine->globalObject().setProperty("PASS", engine->newFunction(PASS_));
	engine->globalObject().setProperty("FAIL", engine->newFunction(FAIL_));
	engine->globalObject().setProperty("FIND", engine->newFunction(FIND_));
	engine->globalObject().setProperty("AHEAD", engine->newFunction(AHEAD_));
	engine->globalObject().setProperty("NOT", engine->newFunction(NOT_));
	engine->globalObject().setProperty("LENGTH", engine->newFunction(LENGTH_));
	engine->globalObject().setProperty("CHOICE", engine->newFunction(CHOICE_));
	engine->globalObject().setProperty("GLUE", engine->newFunction(GLUE_));
	engine->globalObject().setProperty("KEYWORD", engine->newFunction(KEYWORD_));
	engine->globalObject().setProperty("DEFINE", engine->newFunction(DEFINE_));
	engine->globalObject().setProperty("DEFINE_VOID", engine->newFunction(DEFINE_VOID_));
	engine->globalObject().setProperty("ENTRY", engine->newFunction(ENTRY_));
	engine->globalObject().setProperty("REF", engine->newFunction(REF_));
	engine->globalObject().setProperty("INLINE", engine->newFunction(INLINE_));
	engine->globalObject().setProperty("PREVIOUS", engine->newFunction(PREVIOUS_));
	engine->globalObject().setProperty("LINK", engine->newFunction(LINK_));
	engine->globalObject().setProperty("STATE_FLAG", engine->newFunction(STATE_FLAG_));
	engine->globalObject().setProperty("STATE_CHAR", engine->newFunction(STATE_CHAR_));
	engine->globalObject().setProperty("STATE_STRING", engine->newFunction(STATE_STRING_));
	engine->globalObject().setProperty("SET", engine->newFunction(SET_));
	engine->globalObject().setProperty("IF", engine->newFunction(IF_));
	engine->globalObject().setProperty("GETCHAR", engine->newFunction(GETCHAR_));
	engine->globalObject().setProperty("SETCHAR", engine->newFunction(SETCHAR_));
	engine->globalObject().setProperty("VARCHAR", engine->newFunction(VARCHAR_));
	engine->globalObject().setProperty("VAROTHER", engine->newFunction(VAROTHER_));
	engine->globalObject().setProperty("GETSTRING", engine->newFunction(GETSTRING_));
	engine->globalObject().setProperty("SETSTRING", engine->newFunction(SETSTRING_));
	engine->globalObject().setProperty("VARSTRING", engine->newFunction(VARSTRING_));
	engine->globalObject().setProperty("INVOKE", engine->newFunction(INVOKE_));
}

ScriptableSyntaxDefinition::ScriptableSyntaxDefinition(Ref<Scope> scope, QString name)
	: SyntaxDefinition(scope, constCharFromString(name))
{}

const char* ScriptableSyntaxDefinition::constCharFromString(QString s)
{
	Mutex& mutex = localStatic<Mutex, ScriptableSyntaxDefinition>();
	ScopeGuard<Mutex> guard(&mutex);
	static QMap<QString,QByteArray> constStringPool;
	QByteArray ba = s.toUtf8();
	if (!constStringPool.contains(s))
		constStringPool.insert(s, ba);
	return constStringPool.value(s).constData();
}

QScriptValue ScriptableSyntaxDefinition::nodeToScriptValue(QScriptEngine* engine, NODE const& node)
{
	return engine->newVariant(QVariant::fromValue(node));
}

void ScriptableSyntaxDefinition::nodeFromScriptValue(const QScriptValue& value, NODE& node)
{
	node = value.toVariant().value<NODE>();
}

QScriptValue ScriptableSyntaxDefinition::ruleToScriptValue(QScriptEngine* engine, RULE const& rule)
{
	// return engine->newQObject(new QObjectWrapper<RuleNode, Owner>(rule));
	return engine->newVariant(QVariant::fromValue(rule));
}

void ScriptableSyntaxDefinition::ruleFromScriptValue(const QScriptValue& value, RULE& rule)
{
	// rule = dynamic_cast<QObjectWrapper<RuleNode, Owner>*>(value.toQObject())->instance();
	rule = value.toVariant().value<RULE>();
}

QScriptValue ScriptableSyntaxDefinition::constCharToScriptValue(QScriptEngine* engine, const char* const& cstr)
{
	return QScriptValue(cstr);
}

void ScriptableSyntaxDefinition::constCharFromScriptValue(const QScriptValue& value, const char*& cstr)
{
	cstr = constCharFromString(value.toString());
}

void ScriptableSyntaxDefinition::checkNumberOfArguments(QScriptContext* context, int expected)
{
	if (context->argumentCount() != expected)
		context->throwError(QScriptContext::TypeError, tr("Wrong number of arguments (expected %1).").arg(expected));
}

void ScriptableSyntaxDefinition::checkNumberOfArguments(QScriptContext* context, int min, int max)
{
	if ((context->argumentCount() < min) || (max < context->argumentCount()))
		context->throwError(QScriptContext::TypeError, tr("Wrong number of arguments (expected %1 to %2).").arg(min).arg(max));
}

QScriptValue ScriptableSyntaxDefinition::OPTION_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 2);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	QScriptValue value = context->argument(1);
	if (value.isBool())
		super(context)->OPTION(name, value.toBool());
	/*else if (value.isNumber())
		super(context)->OPTION(name, value.toInt32());
	else if (value.isString())
		super(context)->OPTION(name, value.toString());*/
	return QScriptValue();
}

QScriptValue ScriptableSyntaxDefinition::CHAR_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	QChar ch = context->argument(0).toString().at(0);
	NODE node = super(context)->CHAR(ch);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::OTHER_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	QChar ch = context->argument(0).toString().at(0);
	NODE node = super(context)->OTHER(ch);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::ANY_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 0);
	NODE node = super(context)->ANY();
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::RANGE_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1, 2);
	if (context->argumentCount() == 2) {
		QChar a = context->argument(0).toString().at(0);
		QChar b = context->argument(1).toString().at(0);
		NODE node = super(context)->RANGE(a, b);
		return nodeToScriptValue(engine, node);
	}
	else if (context->argumentCount() == 1) {
		QString s = context->argument(0).toString();
		NODE node = super(context)->RANGE(s.constData());
		return nodeToScriptValue(engine, node);
	}
	return QScriptValue();
}

QScriptValue ScriptableSyntaxDefinition::EXCEPT_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1, 2);
	if (context->argumentCount() == 2) {
		QChar a = context->argument(0).toString().at(0);
		QChar b = context->argument(1).toString().at(0);
		NODE node = super(context)->EXCEPT(a, b);
		return nodeToScriptValue(engine, node);
	}
	else if (context->argumentCount() == 1) {
		QString s = context->argument(0).toString();
		NODE node = super(context)->EXCEPT(s.constData());
		return nodeToScriptValue(engine, node);
	}
	return QScriptValue();
}

QScriptValue ScriptableSyntaxDefinition::STRING_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	QString s = context->argument(0).toString();
	NODE node = super(context)->STRING(s.constData());
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::REPEAT_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1, 3);
	if (context->argumentCount() == 1) {
		NODE entry = context->argument(0).toVariant().value<NODE>();
		NODE node = super(context)->REPEAT(entry);
		return nodeToScriptValue(engine, node);
	}
	else if (context->argumentCount() == 2) {
		int minRepeat = context->argument(0).toUInt32();
		NODE entry = context->argument(1).toVariant().value<NODE>();
		NODE node = super(context)->REPEAT(minRepeat, entry);
		return nodeToScriptValue(engine, node);
	}
	else if (context->argumentCount() == 3) {
		int minRepeat = context->argument(0).toUInt32();
		int maxRepeat = context->argument(1).toUInt32();
		NODE entry = context->argument(2).toVariant().value<NODE>();
		NODE node = super(context)->REPEAT(minRepeat, maxRepeat, entry);
		return nodeToScriptValue(engine, node);
	}
	return QScriptValue();
}

QScriptValue ScriptableSyntaxDefinition::BOI_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 0);
	NODE node = super(context)->BOI();
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::EOI_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 0);
	NODE node = super(context)->EOI();
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::PASS_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 0);
	NODE node = super(context)->PASS();
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::FAIL_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 0);
	NODE node = super(context)->FAIL();
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::FIND_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	NODE entry = context->argument(0).toVariant().value<NODE>();
	NODE node = super(context)->FIND(entry);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::AHEAD_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	NODE entry = context->argument(0).toVariant().value<NODE>();
	NODE node = super(context)->AHEAD(entry);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::NOT_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	NODE entry = context->argument(0).toVariant().value<NODE>();
	NODE node = super(context)->NOT(entry);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::LENGTH_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 2, 3);
	if (context->argumentCount() == 2) {
		int minLength = context->argument(0).toUInt32();
		NODE entry = context->argument(1).toVariant().value<NODE>();
		NODE node = super(context)->LENGTH(minLength, entry);
		return nodeToScriptValue(engine, node);
	}
	else if (context->argumentCount() == 3) {
		int minLength = context->argument(0).toUInt32();
		int maxLength = context->argument(1).toUInt32();
		NODE entry = context->argument(2).toVariant().value<NODE>();
		NODE node = super(context)->LENGTH(minLength, maxLength, entry);
		return nodeToScriptValue(engine, node);
	}
	return QScriptValue();
}

QScriptValue ScriptableSyntaxDefinition::CHOICE_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 2, intMax);
	NODE node = super(context)->CHOICE();
	for (int i = 0, n = context->argumentCount(); i < n; ++i)
		node->appendChild(context->argument(i).toVariant().value<NODE>());
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::GLUE_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 2, intMax);
	NODE node = super(context)->GLUE();
	for (int i = 0, n = context->argumentCount(); i < n; ++i)
		node->appendChild(context->argument(i).toVariant().value<NODE>());
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::KEYWORD_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	QString keys = context->argument(0).toString();
	NODE node = super(context)->KEYWORD(keys.toUtf8());
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::DEFINE_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1, 2);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	NODE entry = 0;
	if (context->argumentCount() == 2)
		entry = context->argument(1).toVariant().value<NODE>();
	return super(context)->DEFINE(name, entry);
}

QScriptValue ScriptableSyntaxDefinition::DEFINE_VOID_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 2);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	NODE entry = context->argument(1).toVariant().value<NODE>();
	super(context)->DEFINE_VOID(name, entry);
	return QScriptValue();
}

QScriptValue ScriptableSyntaxDefinition::ENTRY_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	super(context)->ENTRY(name);
	return QScriptValue();
}

QScriptValue ScriptableSyntaxDefinition::REF_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	NODE node = super(context)->REF(name);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::INLINE_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	NODE node = super(context)->INLINE(name);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::PREVIOUS_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1, 2);
	const char* name = 0;
	const char* keyword = 0;
	constCharFromScriptValue(context->argument(0), name);
	if (context->argumentCount() == 2)
		constCharFromScriptValue(context->argument(1), keyword);
	NODE node = super(context)->PREVIOUS(name, keyword);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::LINK_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 0);
	super(context)->LINK();
	return QScriptValue();
}

QScriptValue ScriptableSyntaxDefinition::STATE_FLAG_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 2);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	bool defaultValue = context->argument(1).toBool();
	super(context)->STATE_FLAG(name, defaultValue);
	return QScriptValue();
}

QScriptValue ScriptableSyntaxDefinition::STATE_CHAR_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 2);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	QChar defaultValue = context->argument(1).toString().at(0);
	super(context)->STATE_CHAR(name, defaultValue);
	return QScriptValue();
}

QScriptValue ScriptableSyntaxDefinition::STATE_STRING_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 2);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	const char* defaultValue = 0;
	constCharFromScriptValue(context->argument(1).toString(), defaultValue);
	super(context)->STATE_STRING(name, defaultValue);
	return QScriptValue();
}

QScriptValue ScriptableSyntaxDefinition::SET_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 2);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	bool value = context->argument(1).toBool();
	NODE node = super(context)->SET(name, value);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::IF_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 3);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	NODE trueBranch = context->argument(1).toVariant().value<NODE>();
	NODE falseBranch = context->argument(2).toVariant().value<NODE>();
	NODE node = super(context)->IF(name, trueBranch, falseBranch);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::GETCHAR_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	NODE node = super(context)->GETCHAR(name);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::SETCHAR_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 2);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	QChar value = context->argument(1).toString().at(0);
	NODE node = super(context)->SETCHAR(name, value);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::VARCHAR_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	NODE node = super(context)->VARCHAR(name);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::VAROTHER_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	NODE node = super(context)->VAROTHER(name);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::ScriptableSyntaxDefinition::GETSTRING_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 2);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	NODE coverage = context->argument(1).toVariant().value<NODE>();
	NODE node = super(context)->GETSTRING(name, coverage);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::ScriptableSyntaxDefinition::SETSTRING_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 2);
	const char* name = 0;
	const char* value = 0;
	constCharFromScriptValue(context->argument(0), name);
	constCharFromScriptValue(context->argument(1), value);
	NODE node = super(context)->SETSTRING(name, value);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::VARSTRING_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	NODE node = super(context)->VARSTRING(name);
	return nodeToScriptValue(engine, node);
}

QScriptValue ScriptableSyntaxDefinition::INVOKE_(QScriptContext* context, QScriptEngine* engine)
{
	checkNumberOfArguments(context, 1, 2);
	const char* name = 0;
	constCharFromScriptValue(context->argument(0), name);
	NODE coverage = context->argument(1).toVariant().value<NODE>();
	NODE node = super(context)->INVOKE(name, coverage);
	return nodeToScriptValue(engine, node);
}

} // namespace pte
