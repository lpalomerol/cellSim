//
// Created by luis on 31/10/25.
//

#pragma once
#include <string>
#include "../ports/INoiseSource.h"

namespace domain{
    class Gene {
    public:
        enum class State { PlusPlus, PlusMinus, MinusMinus };
        // Tomamos el nombre por valor y lo movemos al miembro para evitar copias innecesarias
        explicit Gene(std::string name, State initial = State::PlusPlus, double mutation_threshold = 0.1, double mutation_instability_k = 0.0);
        [[nodiscard]] const std::string& name() const;
        [[nodiscard]] std::string status() const;
        // Devuelve un string con el detalle: "NAME[status]", por ejemplo "TP53[+/-]"
        [[nodiscard]] std::string details() const;
        void mutate();
        void live();
        [[nodiscard]] double getMutationThreshold() const { return mutation_threshold_; }
        [[nodiscard]] double getMutationInstabilityK() const { return mutation_instability_k_; }
        void setNoiseSource(INoiseSource* noise);

        [[nodiscard]] bool enabled() const;

        // Nota: se ha eliminado la trazabilidad; el gen solo conoce el presente y provee `live()`.

        // Permite forzar el estado del gen (necesario para tests y fábrica)
        void setState(State s);

    private:
        std::string name_;
        State state_;
        double mutation_threshold_;
        double mutation_instability_k_;
        INoiseSource* noise_;
    };
}
