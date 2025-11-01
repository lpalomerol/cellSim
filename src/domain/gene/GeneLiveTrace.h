#pragma once

#include <string>
#include <sstream>
#include <iomanip>

namespace domain {

// Clase que representa la traza de un gen tras ejecutar `live()`.
// Se separa del header de `Gene` para reducir la verbosidad y permitir helpers.
class GeneLiveTrace {
public:
    double sample = -1.0;
    double threshold = 0.0;
    bool mutated = false;
    std::string before;
    std::string after;

    GeneLiveTrace() = default;

    // Devuelve una representación compacta usada en CSV: "sample:threshold:mutated"
    std::string compactString() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6) << sample << ':' << threshold << ':' << (mutated ? '1' : '0');
        return oss.str();
    }
};

} // namespace domain
