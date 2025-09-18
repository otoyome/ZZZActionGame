#pragma once

#define	NOMINMAX
#include <windows.h>
#include <tchar.h>
#include <sstream>
#include <algorithm>

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem>

#include "misc.h"
#include "high_resolution_timer.h"

#include "enum_class_attribute.h"
#include "singleton.h"

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>
#include <cassert>

#include "system_units.h"
#include "system_utility.h"
#include "finite_state_machine.h"

#include "nameof.hpp"
#include "magic_enum.hpp"
#include "render_data.h"
#include "shader.h"
#include "graphics_utility.h"

#include "material.h"
#include "math_utility.h"
#include "animations/common_animation.h"

#include "input/mouse.h"
#include "input/keyboard.h"

#if	defined( USE_IMGUI )
#define	IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#endif	//	defined( USE_IMGUI )

#include "Graphics/debug_primitive_collector.h"

#include "object/object_manager.h"
