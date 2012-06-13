#ifndef PTE_SEQUENCEADAPTER_HPP
#define PTE_SEQUENCEADAPTER_HPP

#include <ftl/TokenFactory.hpp>
#include "UseFtl.hpp"
#include "Atoms.hpp"
#include "Highlighter.hpp"

namespace pte
{

class Document;

class SequenceAdapter: public TokenFactory
{
public:
	typedef int Index;
	typedef QChar Item;
	
	SequenceAdapter(Ref<Document> document, bool synchronize = true);
	SequenceAdapter(SequenceAdapter* parent, int k1);
	
	static inline int ill() { return -1; }
	bool has(int i);
	QChar get(int i);
	
	QString copy(Ref<Token> token, int* yHint = 0);
	
	virtual Token* newToken();
	
private:
	QString copy(int i0, int i1);
	
	class YToken: public Token {
	private:
		friend class SequenceAdapter;
		YToken(int yHint)
			: yHint_(yHint)
		{}
		int yHint_;
	};
	
	Ref<Highlighter> highlighter_;
	Ref<Cache> cache_;
	Ref<ChunkList> chunkList_;
	Ref<Chunk> chunk_;
	int k1_; // scope length of child adapter
	int y_, ci_; // line number and chunk index
	int i0_, i1_; // linear chunk offsets
	bool nl_; // introduce newline character
};

} // namespace pte

#endif // PTE_SEQUENCEADAPTER_HPP
