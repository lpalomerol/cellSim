// Paper-specific matched intervention executable for BRCA1-ONSET.
// Reads a design CSV and writes one outcome row per virtual tissue/condition.
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef BOOTSTRAPPING_USE_OMP
#include <omp.h>
#endif
#include <cxxopts.hpp>

#include "../src/application/simulation/Simulation.h"
#include "../src/application/bootstrapping/BootstrappingMetrics.h"
#include "../src/domain/adapters/NullLogger.h"
#include "../src/domain/adapters/RandomNoise.h"
#include "../src/domain/cell/CellFactory.h"
#include "../src/domain/gene/GenomeFactory.h"

namespace {
using application::bootstrapping::makeCellSeed;

class PaperSimulation final : public application::Simulation {
public:
    explicit PaperSimulation(int max_t): application::Simulation(max_t) {}
    std::vector<domain::ICell*> liveCells() { return tissue_.getLiveCells(); }
    bool saturated() const { return tissue_.isSaturated(); }
};

std::vector<std::string> split(const std::string&s) {
    std::vector<std::string> out; std::stringstream ss(s); std::string x;
    while(std::getline(ss,x,',')) out.push_back(x);
    return out;
}

struct Design {
    long long tissue_id=0;
    int seed=0;
    double brca1_rate=0,low_delta=0,high_delta=0,neoplastic_div_rate=0;
    double tp53_rate=.003,d1_threshold=2,d2_threshold=5,division_rate=.001,tumor_threshold=.05;
    double kappa_brca1=2,age_drift=1e-5;
    int intervention_age=30;
    double r_d1=0,r_d2=0;
    std::string condition;
    double dose=0;
};

struct Outcome {
    Design d;
    int eligible=0;
    int onset_year=-1;
    int tumor_by70=0;
    int tumor_by80=0;
};

double fieldD(const std::vector<std::string>&v,const std::unordered_map<std::string,int>&h,const std::string&k,double def=0){
    auto it=h.find(k); return it==h.end()?def:std::stod(v.at(it->second));
}
long long fieldLL(const std::vector<std::string>&v,const std::unordered_map<std::string,int>&h,const std::string&k,long long def=0){
    auto it=h.find(k); return it==h.end()?def:std::stoll(v.at(it->second));
}
std::string fieldS(const std::vector<std::string>&v,const std::unordered_map<std::string,int>&h,const std::string&k,const std::string&def=""){
    auto it=h.find(k); return it==h.end()?def:v.at(it->second);
}

std::vector<Design> readDesign(const std::string&path){
    std::ifstream f(path); if(!f) throw std::runtime_error("Cannot open design CSV: "+path);
    std::string line; std::getline(f,line); auto hdr=split(line);
    std::unordered_map<std::string,int> h; for(int i=0;i<(int)hdr.size();++i) h[hdr[i]]=i;
    std::vector<Design> out;
    while(std::getline(f,line)){
        if(line.empty()) continue; auto v=split(line); Design d;
        d.tissue_id=fieldLL(v,h,"tissue_id"); d.seed=(int)fieldLL(v,h,"seed");
        d.brca1_rate=fieldD(v,h,"brca1_rate"); d.low_delta=fieldD(v,h,"low_delta");
        d.high_delta=fieldD(v,h,"high_delta"); d.neoplastic_div_rate=fieldD(v,h,"neoplastic_div_rate");
        d.tp53_rate=fieldD(v,h,"tp53_rate",.003); d.d1_threshold=fieldD(v,h,"d1_threshold",2);
        d.d2_threshold=fieldD(v,h,"d2_threshold",5); d.division_rate=fieldD(v,h,"division_rate",.001);
        d.tumor_threshold=fieldD(v,h,"tumor_threshold",.05); d.kappa_brca1=fieldD(v,h,"kappa_brca1",2);
        d.age_drift=fieldD(v,h,"age_drift",1e-5); d.intervention_age=(int)fieldLL(v,h,"intervention_age",30);
        d.r_d1=fieldD(v,h,"r_d1",0); d.r_d2=fieldD(v,h,"r_d2",0);
        d.condition=fieldS(v,h,"condition"); d.dose=fieldD(v,h,"dose",0);
        out.push_back(d);
    }
    return out;
}

bool tumorPositive(PaperSimulation&sim,double threshold){
    auto cells=sim.liveCells(); if(cells.empty()) return false;
    int n=0; for(auto*c:cells) if(c->isNeoplastic()) ++n;
    return static_cast<double>(n)/cells.size()>=threshold;
}

Outcome runOne(const Design&d,int max_t=80,int n_cells=500){
    auto logger=std::make_shared<domain::adapters::NullLogger>();
    PaperSimulation sim(max_t);
    for(int i=0;i<n_cells;++i){
        auto genome=domain::genome_factory::makeDefaultGenome(
          {{"BRCA1",d.brca1_rate},{"TP53",d.tp53_rate}},
          {{"BRCA1",0.0},{"TP53",0.0}},logger);
        auto noise=std::make_unique<domain::adapters::RandomNoise>(makeCellSeed(d.seed,i));
        auto cell=domain::CellFactory::createCustomCell(
          std::move(noise),genome,d.low_delta,d.high_delta,d.division_rate,
          d.neoplastic_div_rate,true,d.d1_threshold,d.d2_threshold,logger,
          0.0,d.kappa_brca1,d.age_drift);
        sim.addCell(std::move(cell));
    }

    Outcome o; o.d=d;
    for(int year=1;year<=d.intervention_age;++year){
        sim.executeCellCycle();
        if(tumorPositive(sim,d.tumor_threshold)){ o.onset_year=year; o.eligible=0; return o; }
        if(sim.saturated()){ o.eligible=0; return o; }
    }
    o.eligible=1;
    for(auto*c:sim.liveCells()) c->setAttenuation(d.r_d1,d.r_d2);

    for(int year=d.intervention_age+1;year<=max_t;++year){
        sim.executeCellCycle();
        if(tumorPositive(sim,d.tumor_threshold)){ o.onset_year=year; break; }
        if(sim.saturated()) break;
    }
    o.tumor_by70=(o.onset_year>=0 && o.onset_year<=70)?1:0;
    o.tumor_by80=(o.onset_year>=0 && o.onset_year<=80)?1:0;
    return o;
}
}

int main(int argc,char**argv){
    cxxopts::Options o("run_paper_interventions","Matched BRCA1-ONSET interception simulations");
    o.add_options()
      ("design","Design CSV",cxxopts::value<std::string>())
      ("output","Output CSV",cxxopts::value<std::string>()->default_value("paper_intervention_results.csv"))
      ("h,help","Help");
    auto a=o.parse(argc,argv);
    if(a.count("help")||!a.count("design")){ std::cout<<o.help(); return a.count("design")?0:2; }
    auto design=readDesign(a["design"].as<std::string>());
    std::vector<Outcome> out(design.size());
#ifdef BOOTSTRAPPING_USE_OMP
#pragma omp parallel for schedule(dynamic,4)
#endif
    for(long long i=0;i<(long long)design.size();++i) out[i]=runOne(design[i]);

    std::ofstream f(a["output"].as<std::string>());
    f<<"tissue_id,seed,condition,dose,intervention_age,r_d1,r_d2,eligible,onset_year,tumor_by70,tumor_by80\n";
    for(const auto&o:out) f<<o.d.tissue_id<<","<<o.d.seed<<","<<o.d.condition<<","<<o.d.dose<<","
       <<o.d.intervention_age<<","<<o.d.r_d1<<","<<o.d.r_d2<<","<<o.eligible<<","
       <<o.onset_year<<","<<o.tumor_by70<<","<<o.tumor_by80<<"\n";
    return 0;
}
