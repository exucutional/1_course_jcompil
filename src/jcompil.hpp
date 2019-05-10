#ifndef JCOMPIL_HPP_
#define JCOMPIL_HPP_

class Jcompil
{
	uint8_t* memory;
	uint8_t* memdel;
	size_t mcapacity;
public:
	Jcompil(size_t mcap);
	int assembl(FILE* fin) const;
	int translate() const;
	int load();
	int run() const;
	~Jcompil();
};
#endif //JCOMPIL_HPP_