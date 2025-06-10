
#!/bin/bash

#!/bin/bash

for input in scripts/input_files_*.txt; do
    tag=$(basename "$input" .txt | sed 's/input_files_//')
    echo "=== Processing dataset: $tag ==="

    root -l -q "scripts/run_histograms.cc(\"$input\")"

    if [[ -f histograms/pion_histos_iso.root ]]; then
        mv histograms/pion_histos_iso.root histograms/pion_histos_iso_${tag}.root
        echo "Finished: $tag"
    else
        echo "Missing pion_histos_iso.root for $tag - skipping move"
    fi
done
