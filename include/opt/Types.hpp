// Types.hpp: Definitions of option types and exercise styles
#pragma once 

namespace opt {
    enum class OptionType { Call, Put };
    enum class Exercise { European, American };
} // namespace opt