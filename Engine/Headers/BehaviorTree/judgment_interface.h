#pragma	once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <type_traits>

class judgment_interface
{
public:
	judgment_interface() {}
	virtual ~judgment_interface() {}
	virtual bool judgment() = 0;
};
