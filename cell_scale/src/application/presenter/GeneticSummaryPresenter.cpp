#include "GeneticSummaryPresenter.h"
#include <sstream>
#include <iomanip>

namespace application {

std::string GeneticSummaryPresenter::formatGeneticSummary(const domain::GeneticTrackingData& tracking) {
    // Formato boxes: total(neo/activo)
    std::string b1 = formatBox(tracking.getBrcaHetTp53HomPlus(),
                                tracking.getNeoBrcaHetTp53HomPlus(),
                                tracking.getActiveNeoBrcaHetTp53HomPlus());
    std::string b2 = formatBox(tracking.getBrcaHetTp53Het(),
                                tracking.getNeoBrcaHetTp53Het(),
                                tracking.getActiveNeoBrcaHetTp53Het());
    std::string b3 = formatBox(tracking.getBrcaHetTp53HomMinus(),
                                tracking.getNeoBrcaHetTp53HomMinus(),
                                tracking.getActiveNeoBrcaHetTp53HomMinus());
    std::string b4 = formatBox(tracking.getBrcaHomMinus(), -1, -1);

    // Armar resumen
    std::ostringstream summary;
    summary << "  Resumen genético [total(neo/activo)]: |"
            << std::setw(BOX_WIDTH) << b1 << " |"
            << std::setw(BOX_WIDTH) << b2 << " |"
            << std::setw(BOX_WIDTH) << b3 << " |"
            << std::setw(BOX_WIDTH) << b4 << " |\n"
            << "    [BRCA+/- TP53+/+] [BRCA+/- TP53+/-] [BRCA+/- TP53-/-] [BRCA-/-]";

    return summary.str();
}

std::string GeneticSummaryPresenter::formatBox(int count, int neos, int active_neos) {
    std::ostringstream ss;
    ss << count;
    if (neos > 0 || active_neos > 0) {
        ss << "(" << neos;
        if (active_neos > 0) ss << "/" << active_neos;
        ss << ")";
    }
    return ss.str();
}

} // namespace application

