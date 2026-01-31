// Option.hpp: Definition of an Option Contract 
#pragma once 
#include "opt/Types.hpp"

namespace opt {
    struct Option {
        double K = 0.0; // Strike Price 
        double T = 0.0; // Time to Maturity in Years         
        OptionType type = OptionType::Call;
        Exercise exercise = Exercise::European;
    };
} // namespace opt