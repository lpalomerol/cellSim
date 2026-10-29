#!/usr/bin/env python3
import argparse
from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

AGES=np.array([30,40,50,60,70,80])

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--homogeneous", default="data/homogeneous_validation_3000.csv")
    ap.add_argument("--search", default="data/homogeneous_search_source.csv")
    ap.add_argument("--clinical", default="data/clinical_penetrance.csv")
    ap.add_argument("--outdir", default="results/sfig5")
    a=ap.parse_args(); out=Path(a.outdir); out.mkdir(parents=True,exist_ok=True)
    hom=pd.read_csv(a.homogeneous); search=pd.read_csv(a.search); c=pd.read_csv(a.clinical).set_index("age")
    obs=c.loc[AGES,"penetrance"].to_numpy(float); lo=c.loc[AGES,"ci_low"].to_numpy(float); hi=c.loc[AGES,"ci_high"].to_numpy(float)
    hp=np.array([(hom.detect_age.between(0,x)).mean()*100 for x in AGES]); sd=(hi-lo)/3.92; hws=float(np.sum(((hp-obs)/sd)**2))

    fig,ax=plt.subplots(figsize=(7,5)); ax.fill_between(AGES,lo,hi,alpha=.15,label="Observed 95% CI"); ax.plot(AGES,obs,"s-",label="Observed"); ax.plot(AGES,hp,"o-",label="Homogeneous tissue-scale model"); ax.set(xlabel="Age (years)",ylabel="Cumulative penetrance (%)",ylim=(0,100)); ax.legend(frameon=False); ax.text(.98,.06,f"weighted SSE = {hws:.2f}",transform=ax.transAxes,ha="right"); fig.tight_layout(); fig.savefig(out/"FigS5a_homogeneous_penetrance.svg"); plt.close(fig)

    fig,ax=plt.subplots(figsize=(6.8,5.2)); sc=ax.scatter(search.sse_early_mid,search.sse_late,c=search.wsse,s=28,alpha=.75); exact_early=float(np.sum((hp[:3]-obs[:3])**2)); exact_late=float(np.sum((hp[3:]-obs[3:])**2)); ax.scatter([exact_early],[exact_late],marker="X",s=95,label="Exact homogeneous validation"); ax.set_xlabel("Early/mid-life discrepancy\n(ages 30–50; unweighted SSE)"); ax.set_ylabel("Late-life discrepancy\n(ages 60–80; unweighted SSE)"); ax.legend(frameon=False); cb=fig.colorbar(sc,ax=ax); cb.set_label("Overall weighted SSE"); fig.tight_layout(); fig.savefig(out/"FigS5b_homogeneous_tradeoff.svg"); plt.close(fig)
    print(f"Wrote Supplementary Fig. S5 panels to {out}")
if __name__=="__main__": main()
