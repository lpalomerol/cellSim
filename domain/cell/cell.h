//
// Created by luis on 19/10/25.
//

#ifndef CELLSIM_CELL_H
#define CELLSIM_CELL_H


class Cell {
public:
    Cell();

    void live();

    double k; // Genomic unstability
};

#endif //CELLSIM_CELL_H