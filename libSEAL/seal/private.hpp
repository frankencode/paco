#include <ftl/Crc32.hpp>
#include "endian.hpp"

namespace seal
{

using namespace ftl;

bool checkCrc()
{
	Crc32 crc;
	for (int i = 0; i < 10; ++i) {
		uint32_t oldSum = crc.sum();
		uint16_t lol = i * 100 - i * 10;
		crc.feed(&lol, sizeof(lol));
		if (crc.sum() == oldSum) return false;
	}
	return true;
}

bool verify(String execPath)
{
	Ref<File, Owner> file  = new File(execPath);
	file->open(File::Read);
	
	const int bufSize = 10000;
	char buf[bufSize];
	uint64_t fileSize = file->size();
	uint64_t feedCount = 0;
	uint32_t fileSum, sum;
	
	Crc32 crc;
	while (true) {
		int bufFill = file->readAvail(buf, bufSize);
		if (bufFill == 0) break;
		if (feedCount + bufFill == fileSize) bufFill -= sizeof(fileSum);
		crc.feed(buf, bufFill);
		feedCount += bufFill;
	}
	fileSize = sealEndianGate(fileSize - sizeof(fileSum));
	crc.feed(&fileSize, sizeof(fileSize));
	
	sum = crc.sum();
	
	file->seekMove(-int(sizeof(fileSum)));
	file->read((void*)&fileSum, sizeof(fileSum));
	fileSum = sealEndianGate(fileSum);
	
	debug("(seal) fileSum, sum = 0x%hex%, 0x%hex%\n", fileSum, sum);
	
	return fileSum == sum;
}

} // namespace seal
