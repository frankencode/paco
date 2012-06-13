/*
 * WordPredictor.hpp -- word indexer and auto-completer
 *
 * Copyright (c) 2010, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef CODETIPS_WORDPREDICTOR_HPP
#define CODETIPS_WORDPREDICTOR_HPP

#include <ftl/PrefixTree.hpp>
#include <ftl/Singleton.hpp>
#include "codetips.hpp"
#include "Assistant.hpp"

namespace codetips
{

class WordPredictor: public Assistant, public Singleton<WordPredictor>
{
public:
	String language() const;
	String name() const;
	String displayName() const;
	String description() const;
	
	Ref<Tip, Owner> assist(Ref<Context> context, int modifiers, uchar_t key);
	
	void extractWords(String path, String text);
	void disbandWords(String path);
	
private:
	friend class Singleton<WordPredictor>;
	
	WordPredictor();
	
	typedef PrefixTree<char, String> Words;
	typedef PrefixTree<char, Ref<Words, Owner> > WordsByPath;
	Ref<WordsByPath, Owner> wordsByPath_;
};

} // namespace codetips

#endif // CODETIPS_WORDPREDICTOR_HPP
