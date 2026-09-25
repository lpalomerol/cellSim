#!/usr/bin/env python3
"""
export_pdf.py — Generates a publication-grade HTML and PDF report for
docs/paper20260828_1_validation_secondary/working_paper_secondary_validation.md
using MathJax, elegant typography, responsive tables, and headless Chrome.
"""

from __future__ import annotations

import os
import re
import subprocess
import sys
import time
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
PAPER_DIR = SCRIPT_DIR.parent
MD_PATH = PAPER_DIR / "working_paper_secondary_validation.md"
HTML_PATH = PAPER_DIR / "working_paper_secondary_validation.html"
PDF_PATH = PAPER_DIR / "working_paper_secondary_validation.pdf"


def build_html_report() -> Path:
    import markdown

    with open(MD_PATH, encoding="utf-8") as f:
        md_text = f.read()

    html_body = markdown.markdown(
        md_text,
        extensions=["extra", "toc", "md_in_html", "tables", "fenced_code"]
    )

    full_html = f"""<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>cellSim: Inferencia Bayesiana Canónica y Validación de Parámetros Secundarios</title>
    
    <!-- Google Fonts -->
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&family=Merriweather:ital,wght@0,300;0,400;0,700;1,300&family=JetBrains+Mono:wght@400;500&display=swap" rel="stylesheet">

    <!-- MathJax for rendering LaTeX formulas -->
    <script>
    MathJax = {{
      tex: {{
        inlineMath: [['$', '$'], ['\\\\(', '\\\\)']],
        displayMath: [['$$', '$$'], ['\\\\[', '\\\\]']]
      }},
      svg: {{
        fontCache: 'global'
      }}
    }};
    </script>
    <script id="MathJax-script" async src="https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-mml-chtml.js"></script>

    <style>
        @page {{
            size: A4;
            margin: 20mm 15mm 20mm 15mm;
            @bottom-right {{
                content: counter(page);
            }}
        }}
        * {{ margin: 0; padding: 0; box-sizing: border-box; }}
        body {{
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            line-height: 1.65;
            color: #1e293b;
            background: #ffffff;
            padding: 40px 60px;
            max-width: 1000px;
            margin: 0 auto;
            font-size: 14px;
        }}

        @media print {{
            body {{
                padding: 0;
                max-width: 100%;
                font-size: 12px;
                color: #0f172a;
            }}
            .no-print {{ display: none !important; }}
            h1, h2, h3, h4 {{ page-break-after: avoid; }}
            table, figure, .figure-container, pre {{ page-break-inside: avoid; }}
        }}

        h1 {{
            font-family: 'Merriweather', Georgia, serif;
            font-size: 26px;
            color: #0f172a;
            margin-bottom: 8px;
            font-weight: 700;
            line-height: 1.3;
            border-bottom: 2px solid #0284c7;
            padding-bottom: 12px;
        }}
        h2 {{
            font-family: 'Inter', sans-serif;
            font-size: 19px;
            color: #0369a1;
            margin-top: 36px;
            margin-bottom: 14px;
            font-weight: 700;
            border-bottom: 1px solid #e2e8f0;
            padding-bottom: 6px;
        }}
        h3 {{
            font-size: 15px;
            color: #1e293b;
            margin-top: 24px;
            margin-bottom: 10px;
            font-weight: 600;
        }}
        h4 {{
            font-size: 14px;
            color: #334155;
            margin-top: 18px;
            margin-bottom: 8px;
            font-weight: 600;
        }}
        p {{
            margin-bottom: 14px;
            text-align: justify;
        }}
        ul, ol {{
            margin-left: 24px;
            margin-bottom: 14px;
        }}
        li {{
            margin-bottom: 6px;
        }}
        hr {{
            border: 0;
            height: 1px;
            background: #e2e8f0;
            margin: 30px 0;
        }}
        table {{
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
            font-size: 12.5px;
            background: #ffffff;
            box-shadow: 0 1px 3px rgba(0,0,0,0.05);
            border-radius: 6px;
            overflow: hidden;
        }}
        th, td {{
            padding: 9px 12px;
            text-align: left;
            border: 1px solid #cbd5e1;
        }}
        th {{
            background: #f1f5f9;
            color: #0f172a;
            font-weight: 600;
            border-bottom: 2px solid #94a3b8;
        }}
        tr:nth-child(even) {{
            background: #f8fafc;
        }}
        blockquote {{
            border-left: 4px solid #0284c7;
            padding: 12px 18px;
            margin: 18px 0;
            background: #f0f9ff;
            border-radius: 0 6px 6px 0;
            color: #0369a1;
            font-size: 13.5px;
        }}
        code {{
            font-family: 'JetBrains Mono', monospace;
            background: #f1f5f9;
            padding: 2px 5px;
            border-radius: 4px;
            font-size: 12px;
            color: #0f172a;
        }}
        pre {{
            font-family: 'JetBrains Mono', monospace;
            background: #0f172a;
            color: #f8fafc;
            padding: 14px 18px;
            border-radius: 6px;
            overflow-x: auto;
            margin: 18px 0;
            font-size: 11.5px;
            line-height: 1.5;
        }}
        pre code {{
            background: transparent;
            color: inherit;
            padding: 0;
        }}
        img {{
            max-width: 100%;
            height: auto;
            border-radius: 6px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.08);
            margin: 12px 0 6px 0;
            border: 1px solid #e2e8f0;
        }}
        em {{
            color: #475569;
            font-size: 12.5px;
        }}
        a {{
            color: #0284c7;
            text-decoration: none;
        }}
        a:hover {{
            text-decoration: underline;
        }}
    </style>
</head>
<body>

{html_body}

</body>
</html>
"""

    with open(HTML_PATH, "w", encoding="utf-8") as f:
        f.write(full_html)
    print(f"Generated HTML report -> {HTML_PATH}")
    return HTML_PATH


def export_pdf_via_chrome(html_path: Path, pdf_path: Path) -> None:
    cmd = [
        "google-chrome",
        "--headless=new",
        "--disable-gpu",
        "--no-pdf-header-footer",
        "--run-all-compositor-stages-before-draw",
        "--virtual-time-budget=6000",
        f"--print-to-pdf={pdf_path}",
        str(html_path.resolve()),
    ]
    print("Exporting PDF via Headless Chrome...")
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode == 0 and pdf_path.exists():
        print(f"Successfully generated PDF -> {pdf_path} ({pdf_path.stat().st_size / 1024 / 1024:.2f} MB)")
    else:
        print(f"Chrome export output:\n{res.stdout}\n{res.stderr}")


def main():
    html_path = build_html_report()
    export_pdf_via_chrome(html_path, PDF_PATH)


if __name__ == "__main__":
    main()
