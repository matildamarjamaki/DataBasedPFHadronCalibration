
#!/bin/bash

# asetetaan työhakemistot
WORKDIR="$(cd "$(dirname "$0")/.." && pwd)"
PLOT_DIR="$WORKDIR/plots"
RESULT_DIR="$WORKDIR/results"
ROOTFILE="$WORKDIR/histograms/SingleNeutrino_Summer2024.root" 

# ajetaan ROOT-plottiskripti
echo "[INFO] Running plot_histograms..."
root -l -b -q "$WORKDIR/scripts/plot_histograms.cc(\"$ROOTFILE\")"

# luodaan LaTeX-tiedosto
TEX_FILE="$RESULT_DIR/all_plots.tex"
PDF_FILE="$RESULT_DIR/all_plots.pdf"

echo "[INFO] Generating LaTeX report..."

cat <<EOF > "$TEX_FILE"
\documentclass[a4paper,12pt]{article}
\usepackage[margin=2cm]{geometry}
\usepackage{graphicx}
\usepackage{caption}
\usepackage{float}
\usepackage{fancyhdr}
\usepackage{titlesec}
\usepackage{pgffor}

\pagestyle{fancy}
\fancyhf{}
\rhead{E/p Distributions - HCAL only}
\lhead{CMS PF Hadron Calibration}
\rfoot{\thepage}

\titleformat{\section}[block]{\bfseries\Large}{\thesection}{1em}{}

\begin{document}

\begin{center}
    \Huge \textbf{E/p Distributions for HCAL Only}\\[1ex]
    \Large Full pT bin scan\\[2ex]
    \normalsize Matilda Marjamäki - CMS Summer Project 2025
\end{center}

\vspace{1cm}

\foreach \i in {0,...,30} {
    \begin{figure}[H]
        \centering
        \includegraphics[width=0.85\textwidth]{../plots/proj_resp_cut_bin_\i.png}
        \caption{E/p distribution for HCAL only, bin \i}
    \end{figure}
}

\end{document}
EOF

# käännetään PDF-raportti
echo "[INFO] Compiling LaTeX to PDF..."
cd "$RESULT_DIR"
pdflatex -interaction=nonstopmode all_plots.tex > /dev/null

if [ -f "$PDF_FILE" ]; then
    echo "[SUCCESS] PDF created at: $PDF_FILE"
else
    echo "[ERROR] PDF compilation failed."
fi