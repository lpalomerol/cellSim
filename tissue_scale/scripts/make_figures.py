#!/usr/bin/env python3
import argparse
from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.stats import chi2_contingency

AGES=np.array([30,40,50,60,70,80])
LABELS=["<30","30–39","40–49","50–59","60–69","70–79","80"]

def pen(df): return np.array([(df.detect_age.between(0,a)).mean()*100 for a in AGES])

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--final", default="data/final_multiobjective_validation_10000.csv")
    ap.add_argument("--homogeneous", default="data/homogeneous_validation_3000.csv")
    ap.add_argument("--clinical", default="data/clinical_penetrance.csv")
    ap.add_argument("--outdir", default="results/figures")
    a=ap.parse_args(); out=Path(a.outdir); out.mkdir(parents=True,exist_ok=True)
    final=pd.read_csv(a.final); hom=pd.read_csv(a.homogeneous); c=pd.read_csv(a.clinical).set_index("age")
    obs=c.loc[AGES,"penetrance"].to_numpy(float); lo=c.loc[AGES,"ci_low"].to_numpy(float); hi=c.loc[AGES,"ci_high"].to_numpy(float)
    fp=pen(final); hp=pen(hom); sd=(hi-lo)/3.92
    fws=np.sum(((fp-obs)/sd)**2); hws=np.sum(((hp-obs)/sd)**2)

    fig,ax=plt.subplots(figsize=(6.8,5.1)); ax.fill_between(AGES,lo,hi,alpha=.15,label="Observed 95% CI"); ax.plot(AGES,obs,"s-",label="Observed"); ax.plot(AGES,hp,"o-",label="Homogeneous restraint"); ax.plot(AGES,fp,"o-",label="Variable D2 restraint"); ax.set(xlabel="Age (years)",ylabel="Cumulative penetrance (%)",ylim=(0,100)); ax.legend(frameon=False); ax.text(.98,.12,f"Homogeneous wSSE = {hws:.2f}",transform=ax.transAxes,ha="right"); ax.text(.98,.06,f"Variable restraint wSSE = {fws:.2f}",transform=ax.transAxes,ha="right"); fig.tight_layout(); fig.savefig(out/"Fig6a_penetrance.svg"); plt.close(fig)

    pos=final[final.detect_age>=0].copy(); nf=pos.brca1_null_fraction
    vals=[100*(nf<=.5).mean(),100*((nf>.5)&(nf<=.75)).mean(),100*(nf>.75).mean()]
    fig,ax=plt.subplots(figsize=(7,3.3)); left=0
    for lab,v in zip(["BRCA1+/- predominant","Mixed","BRCA1-/- predominant"],vals): ax.barh([""],[v],left=left,label=lab); ax.text(left+v/2,0,f"{v:.0f}%",ha="center",va="center"); left+=v
    ax.set(xlim=(0,100),xlabel="Tumor-positive tissues (%)",title="BRCA1 status in emerging tumors"); ax.legend(frameon=False,ncol=3,bbox_to_anchor=(.5,-.22),loc="upper center"); fig.tight_layout(); fig.savefig(out/"Fig6b_BRCA1.svg"); plt.close(fig)

    pos["age_bin"]=pd.cut(pos.detect_age,[-np.inf,29,39,49,59,69,79,80],labels=LABELS); pos["fc"]=pd.cut(pos.major_founders_5pct,[0,1,2,3,np.inf],labels=["1","2","3","≥4"]); tab=pd.crosstab(pos.age_bin,pos.fc).reindex(index=LABELS,columns=["1","2","3","≥4"],fill_value=0); pct=tab.div(tab.sum(axis=1),axis=0)*100; chi,pv,_,_=chi2_contingency(tab.values)
    fig,ax=plt.subplots(figsize=(7.2,5)); x=np.arange(len(LABELS)); bottom=np.zeros(len(LABELS))
    for cat in ["1","2","3","≥4"]: v=pct[cat].to_numpy(float); ax.bar(x,v,bottom=bottom,label=cat); bottom+=v
    ax.set_xticks(x,LABELS,rotation=35,ha="right"); ax.set(xlabel="Age at first tumor-positive detection (years)",ylabel="Tumor-positive tissues (%)",ylim=(0,100)); ax.legend(title="Tumoral founder lineages (n)",frameon=False,ncol=4); ax.text(.98,.04,f"chi-square P = {pv:.2g}",transform=ax.transAxes,ha="right"); fig.tight_layout(); fig.savefig(out/"Fig6c_founders.svg"); plt.close(fig)
    print(f"Wrote figures to {out}")
if __name__=="__main__": main()
