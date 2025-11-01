#pragma once

#include <string>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "../gene/Gene.h"

namespace domain {

// Clase ligera que representa la traza por tick de una AgenticCell.
// Se puso en un header separado para reducir la verbosidad del header de AgenticCell.
class AgenticTickTrace {
public:
    int tick = 0;
    bool alive_before = true;
    bool alive_after = true;
    bool is_neoplastic_before = false;
    bool is_neoplastic_after = false;
    double neoplasm_sample = -1.0;
    double neoplasm_threshold = 0.0;
    bool neoplasm_happened = false;
    std::unordered_map<std::string, domain::Gene::LiveTrace> gene_traces;

    AgenticTickTrace() = default;

    // Inicializa los campos básicos antes del tick
    void init(int t, bool aliveBefore, bool neoplasticBefore) {
        tick = t;
        alive_before = aliveBefore;
        is_neoplastic_before = neoplasticBefore;
        // valores por defecto
        alive_after = aliveBefore;
        is_neoplastic_after = neoplasticBefore;
        neoplasm_sample = -1.0;
        neoplasm_threshold = 0.0;
        neoplasm_happened = false;
        gene_traces.clear();
    }

    // Marcar que la célula estaba muerta: no se debe avanzar nada ni consumir ruido
    void markDeadNoChange() {
        gene_traces.clear();
        alive_after = false;
        neoplasm_happened = false;
        neoplasm_threshold = 0.0;
        neoplasm_sample = -1.0;
        is_neoplastic_after = is_neoplastic_before; // sin cambio
    }

    // Asignar trazas de genes y registrar estado alive_after
    void setGeneTracesAndAliveAfter(std::unordered_map<std::string, domain::Gene::LiveTrace>&& traces, bool aliveAfter) {
        gene_traces = std::move(traces);
        alive_after = aliveAfter;
    }

    // Caso: si la célula ya era neoplásica antes del muestreo, no samplear
    void markAlreadyNeoplastic() {
        neoplasm_happened = false;
        neoplasm_threshold = 0.0;
        neoplasm_sample = -1.0;
        is_neoplastic_after = is_neoplastic_before;
    }

    // Ejecuta la lógica de decisión de neoplasia: actualiza neoplasm fields y puede cambiar el flag `isNeoplastic` externo.
    // Devuelve true si se realizó muestreo (p>0), false si no se muestreó.
    bool decideNeoplasm(double p, double sample, bool &isNeoplasticFlag) {
        neoplasm_threshold = p;
        if (p <= 0.0) {
            neoplasm_sample = -1.0;
            neoplasm_happened = false;
            is_neoplastic_after = isNeoplasticFlag;
            return false;
        }
        neoplasm_sample = sample;
        if (sample < p) {
            isNeoplasticFlag = true;
            neoplasm_happened = true;
        } else {
            neoplasm_happened = false;
        }
        is_neoplastic_after = isNeoplasticFlag;
        return true;
    }

    // --- Serialización ---
    // Cabecera CSV estándar (incluye un campo compacto con las trazas de genes)
    static std::string csvHeader() {
        return "tick,alive_before,alive_after,is_neoplastic_before,is_neoplastic_after,neoplasm_sample,neoplasm_threshold,neoplasm_happened,gene_traces";
    }

    // Escape simple para CSV: si el campo contiene comas o comillas, encerrar entre comillas y duplicar comillas internas
    static std::string escapeCsvField(const std::string &s) {
        bool needQuote = s.find(',') != std::string::npos || s.find('"') != std::string::npos || s.find('\n') != std::string::npos;
        if (!needQuote) return s;
        std::string out = "\"";
        for (char c : s) {
            if (c == '"') out += "\"\"";
            else out += c;
        }
        out += '\"';
        return out;
    }

    // Convierte la traza a una línea CSV compacta. El campo `gene_traces` contiene una representación semicolon-separated.
    std::string toCsv() const {
        std::ostringstream oss;
        oss << tick << ','
            << (alive_before ? "1" : "0") << ','
            << (alive_after ? "1" : "0") << ','
            << (is_neoplastic_before ? "1" : "0") << ','
            << (is_neoplastic_after ? "1" : "0") << ',';
        // números con precisión fija
        oss << std::fixed << std::setprecision(6) << neoplasm_sample << ',' << neoplasm_threshold << ',' << (neoplasm_happened ? "1" : "0") << ',';

        // construir campo gene_traces compacto
        std::ostringstream goss;
        bool first = true;
        for (const auto &kv : gene_traces) {
            if (!first) goss << ';';
            first = false;
            const auto &name = kv.first;
            const auto &gt = kv.second;
            // formato: NAME:before->after|sample:threshold:mutated
            goss << name << ':' << gt.before << "->" << gt.after << "|";
            goss << std::fixed << std::setprecision(6) << gt.sample << ':' << gt.threshold << ':' << (gt.mutated ? '1' : '0');
        }
        std::string geneField = goss.str();
        oss << escapeCsvField(geneField);
        return oss.str();
    }

    // Escapar cadenas para JSON (muy básico)
    static std::string escapeJson(const std::string &s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '\\': out += "\\\\"; break;
                case '"': out += "\\\""; break;
                case '\b': out += "\\b"; break;
                case '\f': out += "\\f"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        // control chars as \u00XY
                        char buf[7];
                        std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                        out += buf;
                    } else out += c;
            }
        }
        return out;
    }

    // Serializar a JSON con la estructura completa, incluyendo trazas por gen
    std::string toJson() const {
        std::ostringstream oss;
        oss << '{';
        oss << "\"tick\":" << tick << ',';
        oss << "\"alive_before\":" << (alive_before ? "true" : "false") << ',';
        oss << "\"alive_after\":" << (alive_after ? "true" : "false") << ',';
        oss << "\"is_neoplastic_before\":" << (is_neoplastic_before ? "true" : "false") << ',';
        oss << "\"is_neoplastic_after\":" << (is_neoplastic_after ? "true" : "false") << ',';
        oss << "\"neoplasm_sample\":" << std::fixed << std::setprecision(6) << neoplasm_sample << ',';
        oss << "\"neoplasm_threshold\":" << neoplasm_threshold << ',';
        oss << "\"neoplasm_happened\":" << (neoplasm_happened ? "true" : "false") << ',';

        oss << "\"gene_traces\":{";
        bool first = true;
        for (const auto &kv : gene_traces) {
            if (!first) oss << ',';
            first = false;
            const auto &name = kv.first;
            const auto &gt = kv.second;
            // imprimir "name":{
            oss << '"' << escapeJson(name) << "\":{";
            oss << R"("before":")" << escapeJson(gt.before) << '"' << ',';
            oss << R"("after":")" << escapeJson(gt.after) << '"' << ',';
            oss << "\"sample\":" << std::fixed << std::setprecision(6) << gt.sample << ',';
            oss << "\"threshold\":" << gt.threshold << ',';
            oss << "\"mutated\":" << (gt.mutated ? "true" : "false");
            oss << '}';
        }
        oss << '}';

        oss << '}';
        return oss.str();
    }

};

} // namespace domain
