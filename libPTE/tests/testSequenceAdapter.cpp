#include <QFile>
#include <QTextStream>
#include <ftl/stdio>
#include <pte/Document.hpp>
#include <pte/SequenceAdapter.hpp>

namespace pte
{

int main(int argc, char** argv)
{
	if (argc == 1) {
		print("usage: testSequenceAdapter [FILE...]\n");
		return 1;
	}
	
	for (int i = 1; i < argc; ++i) {
		QFile file(argv[i]);
		file.open(QIODevice::ReadOnly);
		QTextStream source(&file);
		Ref<Document, Owner> document = new Document;
		document->load(&source);
		print("--- begin: %%\n|", argv[i]);
		Ref<SequenceAdapter, Owner> seq = new SequenceAdapter(document);
		int k = 0;
		while (seq->def(k)) {
			char ch = char(seq->get(k).unicode());
			if (ch < 0x20)
				print("%%(%%)|", ch, int(ch));
			else
				print("%%|", ch);
			++k;
		}
		print("\n--- end: %% (%% chars)\n", argv[i], k);
		
		print("--- begin reverse: %%\n|", argv[i]);
		--k;
		int n = 0;
		while (seq->def(k)) {
			char ch = char(seq->get(k).unicode());
			if (ch < 0x20)
				print("%%(%%)|", ch, int(ch));
			else
				print("%%|", ch);
			++n;
			--k;
		}
		print("\n--- end reverse: %% (%% chars)\n", argv[i], n);
	}
	
	return 0;
}

} // namespace pte

int main(int argc, char** argv)
{
	return pte::main(argc, argv);
}
