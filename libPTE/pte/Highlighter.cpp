#include <ftl/streams> // DEBUG
#include <ftl/Syntax.hpp>
#include <ftl/LocalStatic.hpp>
#include <ftl/Guard.hpp>
#include <ftl/ThreadFactory.hpp>
#include "Document.hpp"
#include "SyntaxDefinition.hpp"
#include "DocumentColorizer.hpp"
#include "LanguageLayer.hpp"
#include "LanguageStack.hpp"
#include "Highlighter.hpp"

#define PTE_DETECT_DEADLOCK 1
#define PTE_DEBUG_HIGHLIGHTING 0
#define PTE_PROFILE_HIGHLIGHTING 0

namespace pte
{

Highlighter::Highlighter(Ref<LanguageStack> languageStack, Ref<Document> document)
	: languageStack_(languageStack),
	  document_(document),
	  started_(false),
	  keepAlive_(new Semaphore(0)),
	  yieldCount_(0),
	  access_(new Semaphore(1)),
	  resume_(new Semaphore(0)),
	  restart_(new Semaphore(1)),
	  shutdown_(new Semaphore(0)),
	  syncMutex_(new Mutex),
	  sync_(new Condition)
{}

Highlighter::~Highlighter() // prevent inlining of destructor
{}

void Highlighter::start()
{
	if (refCount() > 0) {
		keepAlive_->release();
		incRefCount(); // prevent self destruction while running
	}
	ThreadFactory factory;
	factory.setStackSize(1*1024*1024); // HACK, limit should be user-configurable
	factory.start(this);
	started_ = true;
}

bool Highlighter::isRunning() const
{
	if (!started_) return false;
	return stillAlive();
}

Ref<Style> Highlighter::defaultStyle() const { return document_->defaultStyle(); }
Ref<LanguageStack> Highlighter::languageStack() const { return languageStack_; }
Ref<Document> Highlighter::document() const { return document_; }

bool Highlighter::synchronise()
{
	if (access_->releaseOnDemand()) {
		resume_->acquireAll();
		access_->acquire();
	}
	if (restart_->tryAcquire()) {
		restart_->release();
		return false;
	}
	return true;
}

void Highlighter::run()
{
	runWrapped();
	if (keepAlive_->tryAcquire())
		decRefCount(); // allow self destruction after termination
}

void Highlighter::runWrapped()
{
	#if PTE_DEBUG_HIGHLIGHTING
	print("Highlighter::run().0\n");
	#endif
	
	Ref<Token, Owner> rootToken0 = 0;
	
	while (true)
	{
		#if PTE_PROFILE_HIGHLIGHTING
		stages_ = -2;
		#endif
		
		restart_->acquireAll();
		if (shutdown_->tryAcquire()) break;
		
		if (rootToken0) {
			#if PTE_PROFILE_HIGHLIGHTING
			stages_ = -1;
			#endif
			
			// force tree destruction at this point
			// (reducing heap utilization a little bit and making tree destruction a little bit more
			//  deterministic (although leafs aren't destroyed at this point;))
			rootToken0 = 0;
			
			if (restart_->tryAcquire()) {
				restart_->release();
				continue;
			}
		}
		
		#if PTE_PROFILE_HIGHLIGHTING
		stages_ = 0;
		#endif
		#if PTE_DEBUG_HIGHLIGHTING
		print("Highlighter::run().1\n");
		#endif
		
		bool restartRequested = false;
		
		for (int depth = 0; depth < languageStack_->numLayers(); ++depth)
		{
			Ref<LanguageLayer> languageLayer = languageStack_->layer(depth);
			
			#if PTE_DEBUG_HIGHLIGHTING
			print("Highlighter::run(): depth = %%\n", depth);
			#endif
			
			access_->acquire();
			SequenceAdapter source(document_);
			Ref<Token, Owner> rootToken1 = languageLayer->syntax()->match(&source);
			if (rootToken1) {
				// extent match range if incomplete match
				if (source.has(rootToken1->i1())) {
					// print("Highlighter::highlight(): Incomplete match: rootToken1->i1() = %%\n", rootToken1->i1());
					int i1 = rootToken1->i1();
					while (source.has(i1)) ++i1;
					rootToken1->setRange(rootToken1->i0(), i1);
				}
				else {
					/*print("Highlighter::highlight(): rootToken1->countChildren() = %%\n", rootToken1->countChildren());
					print("Highlighter::highlight(): rootToken1->rule() = %%\n", rootToken1->rule());*/
				}
			}
			access_->release();
			
			if (restart_->tryAcquire()) {
				restart_->release();
				restartRequested = true;
				break;
			}
			
			if (depth > 0)
				Token::meld(rootToken1, rootToken0);
			rootToken0 = rootToken1;
		}
		if (restartRequested)
			continue;
		
		#if PTE_DEBUG_HIGHLIGHTING
		print("Highlighter::run().3: document_->filePath() = %%\n", document_->filePath().toUtf8().data());
		#endif
		#if PTE_PROFILE_HIGHLIGHTING
		stages_ = 3;
		#endif
		
		if (rootToken0) {
			bool needUpdate = false;
			
			// print("Highlighter::run().3.1: rootToken0->i0(), rootToken0->i1() = %%, %%\n", rootToken0->i0(), rootToken0->i1());
			Mutex& mutex = localStatic<Mutex, Highlighter>();
			Guard<Mutex> guard(&mutex); // workaround hack, protects against unresolved race conditions
			access_->acquire();
			// print("Highlighter::run().3.2\n");;
			DocumentColorizer colorizer(languageStack_, document_);
			rootToken0->glow(&colorizer);
			needUpdate = colorizer.needUpdate();
			// print("Highlighter::run().3.3\n");
			access_->release();
			
			if (restart_->tryAcquire()) {
				restart_->release();
				continue;
			}
			
			#if PTE_DEBUG_HIGHLIGHTING
			print("Highlighter::run().4\n");
			#endif
			#if PTE_PROFILE_HIGHLIGHTING
			stages_ = 4;
			#endif
			
			if (needUpdate)
				emit ready();
			
			#if PTE_DEBUG_HIGHLIGHTING
			print("Highlighter::run().5\n");
			#endif
			#if PTE_PROFILE_HIGHLIGHTING
			stages_ = 5;
			#endif
			
			syncMutex_->acquire();
			sync_->broadcast();
			syncMutex_->release();
			
			#if PTE_DEBUG_HIGHLIGHTING
			print("Highlighter::run().s\n");
			#endif
			#if PTE_PROFILE_HIGHLIGHTING
			stages_ = 6;
			#endif
		}
	}
	
	#if PTE_DEBUG_HIGHLIGHTING
	print("Highlighter::run().e: %%\n", (void*)this);
	#endif
}

void Highlighter::yield()
{
	if (yieldCount_ == 0) {
		#if PTE_PROFILE_HIGHLIGHTING
		Time t0 = now();
		#endif
		#if PTE_DETECT_DEADLOCK
		if (!access_->acquireBefore(Time::now() + 3)) {
			print("Highlighter::yield(): possible deadlock detected, check your REPEAT statements and possible recursion.\n");
			access_->acquire();
		}
		#else
		access_->acquire();
		#endif
		#if PTE_PROFILE_HIGHLIGHTING
		print("Highlighter::yield(): dt = %% us (stages_ = %%)\n", (now() - t0).us(), stages_);
		#endif
	}
	++yieldCount_;
}

void Highlighter::resume()
{
	--yieldCount_;
	if (yieldCount_ == 0) {
		if (access_->tryAcquire())
			FTL_THROW(Exception, "Highlighter::resume(): double access release!\n");
		access_->release();
		resume_->release();
	}
}

void Highlighter::restart()
{
	// print("Highlighter::restart()\n");
	restart_->release();
}

bool Highlighter::sync(Time timeout)
{
	// print("Highlighter::sync()\n");
	restart_->release();
	syncMutex_->acquire();
	access_->release();
	resume_->release();
	bool inSync = true;
	if (timeout == 0)
		sync_->wait(syncMutex_);
	else
		inSync = sync_->waitUntil(syncMutex_, timeout);
	syncMutex_->release();
	access_->acquire();
	return inSync;
}

void Highlighter::shutdown()
{
	// print("Highlighter::shutdown()\n");
	#ifndef NDEBUG
	if (yieldCount_ > 0)
		print("Highlighter::shutdown(): shutting down yielded highlighter!\n");
	#endif
	shutdown_->release();
	restart_->release();
}

} // namespace pte
