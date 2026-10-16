#!/bin/bash

INPUT_DOT=$1
OUTPUT_PDF="output.pdf"

# Crée un dossier temporaire
mkdir -p tmp_graphs

# Étape 1 : découper le fichier en blocs de graphe
csplit -z -f tmp_graphs/part_ -b "%03d.dot" "$INPUT_DOT" '/^graph /' '{*}'

# Étape 2 : générer les fichiers PDF
pdf_list=""
i=0
for f in tmp_graphs/part_*.dot; do
    pdf="tmp_graphs/graph_$i.pdf"
    echo $f
    neato -Tpdf "$f" -o "$pdf"
    pdf_list="$pdf_list $pdf"
    ((i++))
done

# Étape 3 : combiner tous les PDFs
if command -v pdfunite &> /dev/null; then
    pdfunite $pdf_list "$OUTPUT_PDF"
elif command -v gs &> /dev/null; then
    gs -dBATCH -dNOPAUSE -q -sDEVICE=pdfwrite -sOutputFile="$OUTPUT_PDF" $pdf_list
else
    echo "❌ Erreur : ni 'pdfunite' ni 'gs' (Ghostscript) ne sont installés."
    exit 1
fi

echo "✅ PDF final généré : $OUTPUT_PDF"

# Nettoyage (optionnel)
rm -r tmp_graphs
