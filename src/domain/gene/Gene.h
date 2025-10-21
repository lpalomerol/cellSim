#pragma once

namespace domain {
    class Gene {
    public:
        enum State { Active, PartiallyDisabled, Disabled };

        Gene();

        Gene(const State initial_state);

        void disable();

        State getState() const;

    private:
        State state;
    };
}
