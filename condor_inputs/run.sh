#!/bin/bash
# $1 = tiedostolista (polku purettuun tiedostoon)
# $2 = tag (esim. SN_aa)

INPUT=$1
TAG=$2

echo "[$(date)] Starting job on host $(hostname)"
echo "Input list: $INPUT"
echo "Tag: $TAG"

# Pura filelist-tarballi (sisältää hakemiston filelists/)
if [[ -f filelists_SN.tar.gz ]]; then
  echo "Extracting filelists_SN.tar.gz ..."
  tar -xzf filelists_SN.tar.gz
fi
if [[ -f filelists_ZB.tar.gz ]]; then
  echo "Extracting filelists_ZB.tar.gz ..."
  tar -xzf filelists_ZB.tar.gz
fi

# Luo väliaikainen tiedostolista jossa redirectori korvattu (/store/ → root://xrootd-cms.infn.it//store/)
TMP_LIST=$(mktemp)
sed 's|^/store/|root://xrootd-cms.infn.it//store/|' $INPUT > $TMP_LIST
echo "First 3 input files after redirect fix:"
head -n 3 $TMP_LIST

# EOS-hakemisto (uusi)
EOS_DIR=/eos/user/m/mmarjama/my_pion_analysis/root_files2025
EOS_URL=root://eosuser.cern.ch/${EOS_DIR}

# Luo EOS-hakemisto jos puuttuu
xrdfs root://eosuser.cern.ch mkdir -p ${EOS_DIR}

export XRD_REQUESTTIMEOUT=300
export XRD_CONNECTIONRETRY=10
export XRD_STREAMTIMEOUT=300
export XRD_TIMEOUTRESOLUTION=5

# Aja analyysi -> kirjoita output paikallisesti
echo "[$(date)] Running run_histograms..."
./run_histograms $TMP_LIST $TAG ${TAG}.root
EXIT_CODE=$?

rm -f $TMP_LIST

if [[ $EXIT_CODE -ne 0 ]]; then
  echo "[$(date)] ERROR: run_histograms failed with exit code $EXIT_CODE"
  exit $EXIT_CODE
fi

# Kopioi EOS:iin
echo "[$(date)] Copying output to EOS: ${EOS_URL}/${TAG}.root"
xrdcp -f ${TAG}.root ${EOS_URL}/${TAG}.root
XRDEXIT=$?

# Poista paikallinen tiedosto jos kopio onnistui
if [[ $XRDEXIT -eq 0 ]]; then
  rm -f ${TAG}.root
  echo "[$(date)] Finished job successfully, file stored in EOS"
else
  echo "[$(date)] ERROR: xrdcp failed with exit code $XRDEXIT"
  exit $XRDEXIT
fi
