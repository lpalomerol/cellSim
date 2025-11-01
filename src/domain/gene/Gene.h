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
        Gene(std::string name, State initial = State::PlusPlus, double mutation_threshold = 0.1, double mutation_instability_k = 0.0);
        const std::string& name() const;
        std::string status() const;
        void mutate();
        void live();
        double getMutationThreshold() const { return mutation_threshold_; }
        double getMutationInstabilityK() const { return mutation_instability_k_; }
        void setNoiseSource(INoiseSource* noise);

        [[nodiscard]] bool enabled() const;

        // Información de trazado cuando se ejecuta `live` (muestra, umbral y si hubo mutación)
        struct LiveTrace {
            double sample = -1.0;
            double threshold = 0.0;
            bool mutated = false;
            std::string before;
            std::string after;
        };
        // Ejecuta un tick para este gen y devuelve traza detallada
        LiveTrace liveWithTrace();

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
