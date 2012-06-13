#ifndef PTE_HIGHLIGHTER_HPP
#define PTE_HIGHLIGHTER_HPP

#include <QObject>
#include <ftl/Condition.hpp>
#include <ftl/Semaphore.hpp>
#include <ftl/Thread.hpp>
#include "UseFtl.hpp"
#include "Atoms.hpp"

namespace pte
{

class LanguageStack;
class Document;

class Highlighter: public QObject, public Thread
{
	Q_OBJECT
	
public:
	Highlighter(Ref<LanguageStack> languageStack, Ref<Document> document);
	~Highlighter();
	
	void start();
	bool isRunning() const;
	
	Ref<Style> defaultStyle() const; // obsolete architecture HACK
	Ref<LanguageStack> languageStack() const;
	Ref<Document> document() const;
	
	void yield();
	void resume();
	void restart();
	bool sync(Time timeout = 0);
	void shutdown();
	
	bool synchronise();
	
signals:
	void ready();
	
private:
	virtual void run();
	void runWrapped();
	
	Ref<LanguageStack, Owner> languageStack_;
	Ref<Document, SetNull> document_;
	
	bool started_;
	Ref<Semaphore, Owner> keepAlive_;
	
	int yieldCount_;
	Ref<Semaphore, Owner> access_;
	Ref<Semaphore, Owner> resume_;
	Ref<Semaphore, Owner> restart_;
	Ref<Semaphore, Owner> shutdown_;
	Ref<Mutex, Owner> syncMutex_;
	Ref<Condition, Owner> sync_;
	
	int stages_;
};

} // namespace pte

#endif // PTE_HIGHLIGHTER_HPP
