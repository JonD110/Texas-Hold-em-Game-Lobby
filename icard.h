#ifndef COMMON_ICARD_H
#define COMMON_ICARD_H
#include <memory>
#include <string>

namespace algorithm {

class ICard
{
public:
	virtual ~ICard() {}
	virtual int index() const = 0;
	virtual int type() const = 0;
	virtual int face() const = 0;
	virtual int color() const = 0;
	virtual bool empty() const = 0;
	virtual std::string name() const = 0;
};

using ICardPtr = std::shared_ptr<ICard>;

} // namespace algorithm
#endif // COMMON_ICARD_H

