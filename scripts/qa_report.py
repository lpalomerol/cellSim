#!/usr/bin/env python3
"""
qa_report.py — Generate HTML QA report from validation JSON

Usage:
    python3 scripts/qa_report.py docs/paper3/validation_report.json docs/paper3/qa_report.html
"""

import json
import sys
from pathlib import Path


def generate_html(json_path: str, html_path: str):
    """Generate HTML report from validation JSON"""

    if not Path(json_path).exists():
        print(f"❌ Error: {json_path} not found")
        print("   Run 'make qa-full' or 'python3 scripts/generate_figures.py' first")
        sys.exit(1)

    with open(json_path) as f:
        data = json.load(f)

    # Status color
    status_colors = {
        "PASS": "#22c55e",
        "WARN": "#f59e0b",
        "FAIL": "#ef4444"
    }
    status_color = status_colors.get(data["status"], "#6b7280")
    status_icon = {"PASS": "✅", "WARN": "⚠️", "FAIL": "❌"}.get(data["status"], "ℹ️")

    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>CellSim Bootstrap Calibration — QA Report</title>
    <style>
        * {{ margin: 0; padding: 0; box-sizing: border-box; }}
        body {{
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
            line-height: 1.6;
            color: #1f2937;
            background: #f9fafb;
            padding: 20px;
        }}
        .container {{
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            box-shadow: 0 1px 3px rgba(0,0,0,0.1);
            border-radius: 8px;
            overflow: hidden;
        }}
        .header {{
            background: linear-gradient(135deg, #1e3a8a 0%, #3b82f6 100%);
            color: white;
            padding: 40px;
        }}
        .header h1 {{
            font-size: 2em;
            margin-bottom: 10px;
            font-weight: 700;
        }}
        .header .subtitle {{
            font-size: 1.1em;
            opacity: 0.9;
        }}
        .header .timestamp {{
            margin-top: 15px;
            font-size: 0.9em;
            opacity: 0.8;
        }}
        .content {{
            padding: 40px;
        }}
        .status-badge {{
            display: inline-block;
            padding: 12px 24px;
            border-radius: 6px;
            font-size: 1.5em;
            font-weight: bold;
            color: white;
            background: {status_color};
            margin-bottom: 30px;
        }}
        .summary {{
            background: #f3f4f6;
            padding: 25px;
            border-radius: 8px;
            margin-bottom: 30px;
        }}
        .summary h2 {{
            margin-bottom: 15px;
            color: #1f2937;
            font-size: 1.3em;
        }}
        .summary-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-top: 15px;
        }}
        .summary-item {{
            background: white;
            padding: 15px;
            border-radius: 6px;
            border-left: 4px solid #3b82f6;
        }}
        .summary-item .label {{
            font-size: 0.85em;
            color: #6b7280;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            margin-bottom: 5px;
        }}
        .summary-item .value {{
            font-size: 1.8em;
            font-weight: bold;
            color: #1f2937;
        }}
        h2 {{
            color: #1f2937;
            font-size: 1.5em;
            margin: 30px 0 20px 0;
            padding-bottom: 10px;
            border-bottom: 2px solid #e5e7eb;
        }}
        table {{
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
            background: white;
            box-shadow: 0 1px 3px rgba(0,0,0,0.05);
            border-radius: 8px;
            overflow: hidden;
        }}
        th, td {{
            padding: 14px;
            text-align: left;
            border-bottom: 1px solid #e5e7eb;
        }}
        th {{
            background: #f9fafb;
            font-weight: 600;
            color: #374151;
            text-transform: uppercase;
            font-size: 0.85em;
            letter-spacing: 0.5px;
        }}
        tr:hover {{
            background: #f9fafb;
        }}
        tr:last-child td {{
            border-bottom: none;
        }}
        .pass {{
            color: #22c55e;
            font-weight: 600;
        }}
        .fail {{
            color: #ef4444;
            font-weight: 600;
        }}
        .warn {{
            color: #f59e0b;
            font-weight: 600;
        }}
        .badge {{
            display: inline-block;
            padding: 4px 10px;
            border-radius: 12px;
            font-size: 0.8em;
            font-weight: 600;
        }}
        .badge-pass {{
            background: #d1fae5;
            color: #065f46;
        }}
        .badge-fail {{
            background: #fee2e2;
            color: #991b1b;
        }}
        .metric {{
            font-family: "SF Mono", Monaco, "Cascadia Code", "Roboto Mono", Consolas, "Courier New", monospace;
            font-size: 0.95em;
        }}
        .footer {{
            background: #f9fafb;
            padding: 20px 40px;
            text-align: center;
            color: #6b7280;
            font-size: 0.9em;
            border-top: 1px solid #e5e7eb;
        }}
        .interpretation {{
            background: #eff6ff;
            border-left: 4px solid #3b82f6;
            padding: 20px;
            margin: 20px 0;
            border-radius: 6px;
        }}
        .interpretation h3 {{
            color: #1e40af;
            margin-bottom: 10px;
            font-size: 1.1em;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🧬 CellSim Bootstrap Calibration</h1>
            <div class="subtitle">QA Validation Report — Kuchenbaecker et al. 2017 (BRCA1)</div>
            <div class="timestamp">Generated: {data['timestamp']}</div>
        </div>
        
        <div class="content">
            <div class="status-badge">
                {status_icon} Status: {data['status']}
            </div>
            
            <div class="summary">
                <h2>Summary</h2>
                <div class="summary-grid">
                    <div class="summary-item">
                        <div class="label">CSV File</div>
                        <div class="value" style="font-size: 1em;">{data['csv_file']}</div>
                    </div>
                    <div class="summary-item">
                        <div class="label">Bootstrap Runs</div>
                        <div class="value">{data['n_runs']}</div>
                    </div>
                    <div class="summary-item">
                        <div class="label">SSE</div>
                        <div class="value">{data['sse']:.1f}</div>
                    </div>
                    <div class="summary-item">
                        <div class="label">Non-penetrance</div>
                        <div class="value">{data['non_penetrance_pct']:.1f}%</div>
                    </div>
                    <div class="summary-item">
                        <div class="label">CI Coverage</div>
                        <div class="value">{data['ci_pass_count']}/6</div>
                    </div>
                </div>
            </div>
            
            <h2>Cumulative Risk Validation</h2>
            <table>
                <thead>
                    <tr>
                        <th>Age (years)</th>
                        <th>Simulated (%)</th>
                        <th>Target (%)</th>
                        <th>Difference (pp)</th>
                        <th>95% CI</th>
                        <th>Status</th>
                    </tr>
                </thead>
                <tbody>
"""

    for age in sorted([int(a) for a in data['risks'].keys()]):
        risk_data = data['risks'][str(age)]
        sim = risk_data['simulated']
        target = risk_data['target']
        diff = risk_data['diff']
        ci = risk_data['ci']
        in_ci = risk_data['in_ci']

        status_class = "pass" if in_ci else "fail"
        badge_class = "badge-pass" if in_ci else "badge-fail"
        status_text = "✅ IN CI" if in_ci else "❌ OUT OF CI"

        html += f"""
                    <tr>
                        <td><strong>{age}</strong></td>
                        <td class="metric">{sim:.1f}%</td>
                        <td class="metric">{target:.1f}%</td>
                        <td class="metric {status_class}">{diff:+.1f}pp</td>
                        <td class="metric">[{ci[0]:.1f}–{ci[1]:.1f}]%</td>
                        <td><span class="badge {badge_class}">{status_text}</span></td>
                    </tr>
"""

    html += """
                </tbody>
            </table>
            
            <div class="interpretation">
                <h3>📊 Interpretation</h3>
                <ul style="margin-left: 20px; margin-top: 10px;">
"""

    if data['sse'] < 100:
        html += "                    <li><strong>Excellent fit:</strong> SSE < 100 indicates high agreement with clinical data</li>\n"
    elif data['sse'] < 300:
        html += "                    <li><strong>Good fit:</strong> SSE < 300 indicates acceptable agreement</li>\n"
    else:
        html += "                    <li><strong>Poor fit:</strong> SSE > 300 suggests significant deviation from clinical data</li>\n"

    if 20 <= data['non_penetrance_pct'] <= 40:
        html += f"                    <li><strong>Non-penetrance ({data['non_penetrance_pct']:.1f}%):</strong> Within expected range (25–35%)</li>\n"
    else:
        html += f"                    <li><strong>Non-penetrance ({data['non_penetrance_pct']:.1f}%):</strong> Outside expected range (25–35%)</li>\n"

    if data['ci_pass_count'] >= 5:
        html += f"                    <li><strong>CI coverage:</strong> {data['ci_pass_count']}/6 ages within 95% CI (excellent)</li>\n"
    elif data['ci_pass_count'] >= 4:
        html += f"                    <li><strong>CI coverage:</strong> {data['ci_pass_count']}/6 ages within 95% CI (acceptable)</li>\n"
    else:
        html += f"                    <li><strong>CI coverage:</strong> {data['ci_pass_count']}/6 ages within 95% CI (poor)</li>\n"

    html += """
                </ul>
            </div>
            
            <h2>Recommendations</h2>
"""

    if data['status'] == "PASS":
        html += """
            <p style="color: #22c55e; font-weight: 600;">
                ✅ Calibration meets publication quality standards. Proceed with manuscript preparation.
            </p>
"""
    elif data['status'] == "WARN":
        html += """
            <p style="color: #f59e0b; font-weight: 600;">
                ⚠️ Calibration is acceptable but has some deviations. Consider reviewing outlier ages.
            </p>
"""
    else:
        html += """
            <p style="color: #ef4444; font-weight: 600;">
                ❌ Calibration does not meet standards. Re-run parameter search or review simulation logic.
            </p>
"""

    html += """
        </div>
        
        <div class="footer">
            Generated by CellSim QA Pipeline • 
            <a href="https://github.com/yourusername/cellSim" style="color: #3b82f6; text-decoration: none;">GitHub</a>
        </div>
    </div>
</body>
</html>
"""

    with open(html_path, "w") as f:
        f.write(html)

    print(f"✅ HTML report generated: {html_path}")
    print(f"   Open in browser: file://{Path(html_path).resolve()}")


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: qa_report.py <validation.json> <output.html>")
        print("Example: python3 scripts/qa_report.py docs/paper3/validation_report.json docs/paper3/qa_report.html")
        sys.exit(1)

    generate_html(sys.argv[1], sys.argv[2])

