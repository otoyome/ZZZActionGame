#pragma	once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <type_traits>

class action_interface
{
public:
	enum class state
	{
		run,		// Às’†
		failed,		// Às¸”s
		complete,	// Às¬Œ÷
	};

public:
	action_interface() {}
	virtual ~action_interface() {}
	virtual state run(float elapsedTime) = 0;
};
