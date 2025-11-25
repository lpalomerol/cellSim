// filepath: src/application/ExperimentalTracking.h
#ifndef CELLSIM_EXPERIMENTALTRACKING_H
#define CELLSIM_EXPERIMENTALTRACKING_H

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>

#include "../domain/ports/ICell.h"
#include "../domain/cell/AgenticCell.h"

namespace app {

struct ExperimentalTracking {
    // Contadores por categorías genéticas
    int brca_het_tp53_hom_plus = 0; // BRCA1 "+/-" y TP53 "+/+"
    int brca_het_tp53_het = 0;      // BRCA1 "+/-" y TP53 "+/-"
    int brca_het_tp53_hom_minus = 0;// BRCA1 "+/-" y TP53 "-/-"
    int brca_hom_minus = 0;         // BRCA1 "-/-" (cualquier TP53)

    // Neoplasias por categoría (solo para BRCA1 +/- categories solicitadas)
    int neo_brca_het_tp53_hom_plus = 0;
    int neo_brca_het_tp53_het = 0;
    int neo_brca_het_tp53_hom_minus = 0;

    // Min/max de genomic instability para las categorías con TP53 +/- y -/-
    double min_inst_tp53_het = std::numeric_limits<double>::infinity();
    double max_inst_tp53_het = -std::numeric_limits<double>::infinity();
    double min_inst_tp53_hom_minus = std::numeric_limits<double>::infinity();
    double max_inst_tp53_hom_minus = -std::numeric_limits<double>::infinity();

    // Observa una célula AgenticCell y actualiza contadores/min-max.
    void observeCell(const domain::AgenticCell* ac) {
        if (!ac) return;
        const std::string brca = ac->getBRCA1();
        const std::string tp53 = ac->getTP53();
        const double instability = ac->getGenomicInstability();
        const bool isNeo = ac->isNeoplastic();

        if (brca == "+/-") {
            if (tp53 == "+/+") {
                ++brca_het_tp53_hom_plus;
                if (isNeo) ++neo_brca_het_tp53_hom_plus;
            } else if (tp53 == "+/-") {
                ++brca_het_tp53_het;
                if (isNeo) ++neo_brca_het_tp53_het;
                if (instability < min_inst_tp53_het) min_inst_tp53_het = instability;
                if (instability > max_inst_tp53_het) max_inst_tp53_het = instability;
            } else if (tp53 == "-/-") {
                ++brca_het_tp53_hom_minus;
                if (isNeo) ++neo_brca_het_tp53_hom_minus;
                if (instability < min_inst_tp53_hom_minus) min_inst_tp53_hom_minus = instability;
                if (instability > max_inst_tp53_hom_minus) max_inst_tp53_hom_minus = instability;
            }
        } else if (brca == "-/-") {
            ++brca_hom_minus;
        }
    }

    // Imprime el resumen genético y, si verbose==true, detalles por célula.
    void printSummary(int current_year, int max_t, int neoplastic_count,
                      const std::vector<std::unique_ptr<domain::ICell>> &cells,
                      bool verbose) const {
        // Convert unique_ptr vector to raw pointers and delegate to the pointer overload
        std::vector<domain::ICell*> ptrs;
        ptrs.reserve(cells.size());
        for (const auto &u : cells) ptrs.push_back(u.get());
        printSummary(current_year, max_t, neoplastic_count, ptrs, verbose);
    }

    // Overload: accept a lightweight view of cells as raw pointers (no ownership transfer).
    void printSummary(int current_year, int max_t, int neoplastic_count,
                      const std::vector<domain::ICell*> &cells,
                      bool verbose) const {
         // Resumen compacto de año y conteos neoplásicos
        std::cout << "Año " << current_year << " / " << max_t
                  << " | Neoplásicas: " << neoplastic_count << " / " << cells.size() << "\n";

         // Imprimir resumen por categorías pedido por el usuario
         const int boxWidth = 9; // ancho interior para incluir cuenta y (neos)
         auto makeBox = [&](int count, int neos){
             std::ostringstream ss;
             ss << count;
             if (neos >= 0) ss << "(" << neos << ")"; // mostrar (neoplasias)
             return ss.str();
         };
         std::string b1 = makeBox(brca_het_tp53_hom_plus, neo_brca_het_tp53_hom_plus);
         std::string b2 = makeBox(brca_het_tp53_het, neo_brca_het_tp53_het);
         std::string b3 = makeBox(brca_het_tp53_hom_minus, neo_brca_het_tp53_hom_minus);
         std::string b4 = makeBox(brca_hom_minus, -1);

        std::cout << "  Resumen genético: |"
                  << std::setw(boxWidth) << b1 << " |"
                  << std::setw(boxWidth) << b2 << " |"
                  << std::setw(boxWidth) << b3 << " |"
                  << std::setw(boxWidth) << b4 << " |\n";

         // Leyenda compacta impresa solo si verbose para no ensuciar la salida
         if (verbose) {
             std::cout << "    [BRCA+/- TP53+/+] [BRCA+/- TP53+/-] [BRCA+/- TP53-/-] [BRCA-/-]\n";
         }

         // Añadir min/max de genomic instability para TP53 +/- y TP53 -/- (2 decimales)
         std::cout << std::fixed << std::setprecision(2);
         if (brca_het_tp53_het > 0) {
             std::cout << "    TP53(+/-) instability: min=" << min_inst_tp53_het << " max=" << max_inst_tp53_het << "\n";
         } else {
             std::cout << "    TP53(+/-) instability: min=N/A max=N/A\n";
         }
         if (brca_het_tp53_hom_minus > 0) {
             std::cout << "    TP53(-/-) instability: min=" << min_inst_tp53_hom_minus << " max=" << max_inst_tp53_hom_minus << "\n";
         } else {
             std::cout << "    TP53(-/-) instability: min=N/A max=N/A\n";
         }
         std::cout << std::defaultfloat;

         // (Opcional) mantener la salida detallada por célula si verbose
        if (verbose) {
            for (size_t i = 0; i < cells.size(); ++i) {
                domain::ICell* ic = cells[i];
                auto* ac = dynamic_cast<domain::AgenticCell*>(ic);
                if (ac) {
                    std::cout << "  Cell[" << i << "] seed=" << ac->getSeed()
                              << " alive=" << (ac->alive() ? "yes" : "no")
                              << " neoplastic=" << (ac->isNeoplastic() ? "yes" : "no")
                              << " TP53=" << ac->getTP53()
                              << " BRCA1=" << ac->getBRCA1()
                              << " age=" << ac->getAge()
                              << " instability=" << ac->getGenomicInstability()
                              << "\n";
                } else if (ic) {
                    std::cout << "  Cell[" << i << "] <no-agentic-info> neoplastic=" << (ic->isNeoplastic() ? "yes" : "no") << "\n";
                }
            }
        }
         // Mensaje de detección de neoplasias (como antes)
         if (neoplastic_count > 0) {
             std::cout << "! Hay neoplasia(s) detectadas en la población en el año " << current_year << "\n";
         }
     }
 };

 } // namespace app

 #endif // CELLSIM_EXPERIMENTALTRACKING_H
