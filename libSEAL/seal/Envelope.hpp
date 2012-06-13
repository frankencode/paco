#ifndef SEAL_ENVELOPE_HPP
#define SEAL_ENVELOPE_HPP

namespace seal
{

class Envelope
{
public:
	Envelope();
	
private:
	static int count_;
};

namespace { Envelope envelope_; }

} // namespace seal

#endif // SEAL_ENVELOPE_HPP
