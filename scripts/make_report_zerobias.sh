
#!/bin/bash

# Asetetaan työhakemistot
WORKDIR="$(cd "$(dirname "$0")/.." && pwd)"
RESULT_DIR="$WORKDIR/results"
PLOT_DIR="$WORKDIR/plots/Data_ZeroBias2024"

mkdir -p "$RESULT_DIR"

############################
# 1. Luo all_stack_plots.tex
############################
STACK_TEX="$RESULT_DIR/zerobias_stack_plots.tex"

cat <<EOF > "$STACK_TEX"
\documentclass[a4paper,10pt]{article}
\usepackage[margin=1.2cm]{geometry}
\usepackage{graphicx}
\usepackage{float}
\usepackage{fancyhdr}
\usepackage{titlesec}
\usepackage{multicol}
\pagestyle{fancy}
\fancyhf{}
\rhead{E/p Distributions - Stacked}
\lhead{CMS PF Hadron Calibration}
\rfoot{\thepage}
\titleformat{\section}[block]{\bfseries\Large}{\thesection}{1em}{}
\begin{document}
\begin{center}
    \Huge \textbf{E/p Stacked Distributions}\par\vspace{1ex}
    \Large All pT bins on one page\par\vspace{1ex}
    \normalsize Matilda Marjamäki -- CMS Summer Project 2025
\end{center}
\vspace{0.6cm}
\begin{center}
\setlength{\tabcolsep}{2pt}
\renewcommand{\arraystretch}{1.0}
\begin{tabular}{cccccccc}
EOF

for i in $(seq 0 30); do
    echo -n "    \includegraphics[width=0.11\\textwidth]{../Data_ZeroBias2024/plots/stack_ep_bin_$i.png}" >> "$STACK_TEX"
    if (( (i + 1) % 8 == 0 )); then
        echo " \\\\" >> "$STACK_TEX"
    else
        echo " %" >> "$STACK_TEX"
    fi
done

cat <<EOF >> "$STACK_TEX"
\end{tabular}
\end{center}
\newpage
EOF

#####################################
# 2. Lisää E/p-jakaumat hadron-tyypeittäin
#####################################
for type in HCALonly ECALonly MIP ECALHCAL; do
    echo "\\section*{E/p Distributions for $type}" >> "$STACK_TEX"
    echo "\\begin{center}" >> "$STACK_TEX"
    echo "\\setlength{\\tabcolsep}{2pt}" >> "$STACK_TEX"
    echo "\\renewcommand{\\arraystretch}{1.0}" >> "$STACK_TEX"
    echo "\\begin{tabular}{cccccccc}" >> "$STACK_TEX"

    count=0
    for img in "$PLOT_DIR"/proj_ep_${type}_bin_*.png; do
        if [ ! -f "$img" ]; then continue; fi
        imgbase=$(basename "$img")
        echo -n "    \\includegraphics[width=0.11\\textwidth]{../plots/${imgbase}}" >> "$STACK_TEX"
        count=$((count + 1))
        if (( count % 8 == 0 )); then
            echo " \\\\" >> "$STACK_TEX"
        else
            echo " %" >> "$STACK_TEX"
        fi
    done

    echo "\\end{tabular}" >> "$STACK_TEX"
    echo "\\end{center}" >> "$STACK_TEX"
    echo "" >> "$STACK_TEX"
done


##########################################
# 3. Lisää fraktioplotit: hadronifraktiot vs. pT
##########################################
echo "\\section*{Fraction of hadron types vs. p}" >> "$STACK_TEX"
echo "\\begin{center}" >> "$STACK_TEX"
echo "\\includegraphics[width=0.8\\textwidth]{../plots/fraction_per_type_vs_pT.png}" >> "$STACK_TEX"
echo "\\end{center}" >> "$STACK_TEX"
echo "\\newpage" >> "$STACK_TEX"


echo "\\end{document}" >> "$STACK_TEX"

##########################
# 3. Käännä PDF
##########################

cd "$RESULT_DIR"
pdflatex -interaction=nonstopmode zerobias_stack_plots.tex > /dev/null

if [ -f zerobias_stack_plots.pdf ]; then
    echo "[SUCCESS] PDF luotu: results/zerobias_stack_plots.pdf"
else
    echo "[ERROR] PDF-käännös epäonnistui"
    exit 1
fi

# Kopioi nimellä combined
cp zerobias_stack_plots.pdf combined_zerobias_report.pdf