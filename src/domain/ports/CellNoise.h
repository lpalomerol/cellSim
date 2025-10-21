#pragma once

namespace domain {
    struct CellNoise {
        double homeostasis_u01; // Uniform [0,1)
        double mutation_brca_u01; // Uniform [0,1)
        double mutation_tp53_u01; // Uniform [0,1)
    };
};


