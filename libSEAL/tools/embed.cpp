#include <ftl/streams>
#include <ftl/binary>
#include <seal/private.hpp>

namespace seal
{

int main(int argc, char** argv)
{
	bool embed = (argc == 2);
	bool strip = (argc == 3) && (String(argv[1]) == "--strip");
	
	if (!(embed || strip)) {
		print(
			"Usage: seal [--embed] <FILE>\n"
			"\n"
			"Embeds a digital seal into the executable file <FILE>.\n"
		);
		return 0;
	}
	
	String execPath = argv[argc - 1];
	
	if (verify(execPath)) {
		if (strip) {
			Ref<File, Owner> file = new File(execPath);
			file->truncate(file->size() - 4);
		}
		else
			print("Input file is already secured by a seal.\n");
		return 0;
	}
	else {
		if (strip)
			return 0;
	}
	
	File::syncAll();
	
	Ref<File, Owner> file = new File(execPath);
	file->open(File::Read|File::Write);
	
	const int bufSize = 10000;
	char buf[bufSize];
	uint64_t fileSize = file->size();
	uint32_t sum;
	
	Crc32 crc;
	while (true) {
		int bufFill = file->readAvail(buf, bufSize);
		if (bufFill == 0) break;
		crc.feed(buf, bufFill);
	}
	
	fileSize = sealEndianGate(fileSize);
	crc.feed(&fileSize, sizeof(fileSize));
	
	sum = crc.sum();
	debug("sum = 0x%hex%\n", sum);
	
	sum = sealEndianGate(sum);
	file->write(&sum, sizeof(sum));
	
	file->sync();
	
	return 0;
}

} // namespace seal

int main(int argc, char** argv)
{
	return seal::main(argc, argv);
}
