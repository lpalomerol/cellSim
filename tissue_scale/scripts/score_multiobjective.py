#!/usr/bin/env python3
"""Score an exact-validation CSV using the manuscript multi-objective criterion.

The biological targets are soft anchors, not exact likelihood observations:
  L = percent of tumor-positive tissues with >50% BRCA1-null Tumoral cells; target 90, scale 5.
  H = percent of L tumors retaining >=25% BRCA1+/- Tumoral cells; target 24, scale 8.
D_joint = penetrance weighted SSE + ((L-90)/5)^2 + ((H-24)/8)^2.
"""
import argparse
import numpy as np
import pandas as pd

AGES = [30,40,50,60,70,80]

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--input", required=True)
    ap.add_argument("--clinical", default="data/clinical_penetrance.csv")
    args=ap.parse_args()
    df=pd.read_csv(args.input)
    c=pd.read_csv(args.clinical).set_index("age")
    p=np.array([(df.detect_age.between(0,a)).mean()*100 for a in AGES])
    o=c.loc[AGES,"penetrance"].to_numpy(float)
    sd=(c.loc[AGES,"ci_high"].to_numpy(float)-c.loc[AGES,"ci_low"].to_numpy(float))/3.92
    dpen=float(np.sum(((p-o)/sd)**2))
    pos=df[df.detect_age>=0]
    loss=pos.brca1_null_fraction>0.5
    L=100*loss.mean()
    H=100*((loss)&(pos.brca1_wt_fraction>=0.25)).sum()/loss.sum()
    dbio=((L-90)/5)**2+((H-24)/8)**2
    print(f"D_penetrance={dpen:.6f}")
    print(f"L={L:.6f}")
    print(f"H={H:.6f}")
    print(f"D_bio={dbio:.6f}")
    print(f"D_joint={dpen+dbio:.6f}")

if __name__=="__main__": main()
