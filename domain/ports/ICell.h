//
// Created by luis on 20/10/25.
//

#pragma once

namespace domain {
    struct ICell {
        virtual ~ICell() = default;
        virtual void live() = 0;
        virtual bool alive() = 0;
    };
}

