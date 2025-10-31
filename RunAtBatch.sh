#!/bin/bash
export X509_USER_PROXY=/afs/cern.ch/user/m/mmarjama/myProxy
export EOS_MGM_URL=root://eosuser.cern.ch
source /cvmfs/cms.cern.ch/cmsset_default.sh

cd ${JOBWORKDIR}
source ./SetupLCGStack.sh
export PATH=$PWD/scripts:$PATH

SAMPLE=$1
INDIR="./input_lists"
INFILE="${INDIR}/${SAMPLE}.txt"
OUTDIR="./histograms/${SAMPLE}"
NCORES=$2

mkdir -p "${OUTDIR}"
echo ">>> Running sample: ${SAMPLE}"
echo ">>> Input list: ${INFILE}"
echo ">>> Output directory: ${OUTDIR}"

root -l -b -q "./scripts/run_histograms.cc(\"${INFILE}\",\"${SAMPLE}\",\"${OUTDIR}\",${NCORES})"
#root -l -b -q "./scripts/run_histograms_cc.so(\"${INFILE}\",\"${SAMPLE}\",\"${OUTDIR}\")"
eos cp -r $OUTDIR /eos/user/m/mmarjama/my_pion_analysis/histograms/test

