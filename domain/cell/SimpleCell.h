//
// Created by luis on 20/10/25.
//

#pragma once
#include "../ports/ICell.h"

namespace domain {
    class SimpleCell final : public ICell {
        public :
            SimpleCell() = default;
            void live() override;
            bool alive() override;

    };
}
