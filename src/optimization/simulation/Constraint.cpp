#include "Constraint.h"

double Constraint::k_stiff = 1.0;

std::vector<std::string> Constraint::CONSTRAINT_NAME = std::vector<std::string>{
        "K_SPRING_STRETCH",
        "K_ATTACHMENT",
        "K_STRETCH",
        "K_BEND"
};