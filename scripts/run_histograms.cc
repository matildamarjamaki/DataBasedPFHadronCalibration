
// ROOT RDataFrame -kirjasto nopeaan, lazy datan käsittelyyn
#include "ROOT/RDataFrame.hxx"
// ROOTin histogrammi- ja tiedostokirjastot
#include "TFile.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TStyle.h"
// C++ standardikirjastoja tiedostonkäsittelyyn, tulostukseen, datarakenteisiin ja ajastukseen
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <tuple>
#include <chrono>
#include <cmath>        // std::abs
#include "TH1D.h"       // käytät TH1D:tä
#include "TSystem.h"    // gSystem->Exec
#include "TString.h"    // Form(...)
#include "ROOT/RVec.hxx"

// MÄÄRITELLÄÄN BINITYS pionien momentille (p = pT * cosh(η))
// tarkka binitys pienillä p-arvioilla auttaa havaitsemaan responsen muutoksia
const double trkPBins[] = {
    3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
    7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
};
const int nTrkPBins = sizeof(trkPBins) / sizeof(double) - 1;


// --- helper: normalisoi lista-rivi käyttökelpoiseksi poluksi/URL:ksi ---
std::string NormalizeInputPath(const std::string &s) {
    if (s.rfind("/store/user/", 0) == 0) {
        // Poimi käyttäjätunnus
        size_t p = std::string("/store/user/").size();
        size_t slash = s.find('/', p);
        std::string user = s.substr(p, slash - p);
        std::string tail = s.substr(slash);
        char first = user.empty() ? 'x' : user[0];

        std::ostringstream oss;
        oss << "root://eosuser.cern.ch//eos/user/"
            << first << "/" << user << tail;
        return oss.str();
    }
    // Muut store-polut globaaliin XRootD:hen
    if (s.rfind("/store/", 0) == 0) {
        return "root://cms-xrd-global.cern.ch" + s;
    }
    return s;
}

// PÄÄFUNKTIO: tekee histogrammit ja kirjoittaa ne tiedostoon
void run_histograms(const char* listfile, const char* tag, const char* outpath) {

// // --- helper: normalisoi lista-rivi käyttökelpoiseksi poluksi/URL:ksi ---
// std::string NormalizeInputPath(const std::string &s) {
//     if (s.rfind("/store/user/", 0) == 0) {
//         // Poimi käyttäjätunnus
//         size_t p = std::string("/store/user/").size();
//         size_t slash = s.find('/', p);
//         std::string user = s.substr(p, slash - p);
//         std::string tail = s.substr(slash);
//         char first = user.empty() ? 'x' : user[0];

//         std::ostringstream oss;
//         oss << "root://eosuser.cern.ch//eos/user/"
//             << first << "/" << user << tail;
//         return oss.str();
//     }
//     // Muut store-polut globaaliin XRootD:hen
//     if (s.rfind("/store/", 0) == 0) {
//         //return "root://cms-xrd-global.cern.ch" + s;
//         return "root://hip-cms-se.csc.fi" + s;
//     }
//     return s;
// }



// helper: käännetään /store/... XRootD-URL:ksi (HIP SE). Jätä valmiit root://-URLit sellaisenaan
std::string NormalizeInputPath(const std::string &s) {
  if (s.rfind("root://", 0) == 0) return s;                // jo valmis
  if (s.rfind("/store/", 0) == 0) return "root://hip-cms-se.csc.fi//" + s;  // HUOM tupla-slash
  return s;
}





// PÄÄFUNKTIO: tekee histogrammit ja kirjoittaa ne tiedostoon
// void run_histograms(const char* listfile, const char* tag, const char* outpath) {
void run_histograms(const char* listfile, const char* tag, const char* outpath = "") {


    // aloitetaan ajanotto ajon kestolle
    auto start = std::chrono::high_resolution_clock::now();

    // monisäikeisyys
    ROOT::EnableImplicitMT();

    // LUETAAN ROOT-tiedostojen polut listasta (robusti)
    std::ifstream infile(listfile);
    if (!infile.is_open()) {
      std::cerr << "[run_histograms] ERROR: cannot open list file: " << listfile << "\n";
      return;
    }
    std::string line;
    std::vector<std::string> files;
    files.reserve(1000);
    while (std::getline(infile, line)) {
        auto norm = NormalizeInputPath(line);
        if (!norm.empty())
            files.push_back(norm);
    }

    std::cout << "Processing " << files.size() << " files..." << std::endl;


    // LUODAAN RDataFrame ...
    ROOT::RDataFrame df("Events", files);

   
// --- SUODATUS JA MUODOSTETTAVAT SUUREET ---
// 1) Perusketju: isoloidut ±211-kandidaatit + E/p:t ja HCAL-korjaus
auto df_base = df
  .Define("isoMask",
          [](const ROOT::VecOps::RVec<int>& pdgId,
             const ROOT::VecOps::RVec<bool>& isoFlag) {
            ROOT::VecOps::RVec<bool> m(pdgId.size());
            for (size_t i = 0; i < pdgId.size(); ++i)
              m[i] = (std::abs(pdgId[i]) == 211) && isoFlag[i];
            return m;
          },
          {"PFCand_pdgId","PFCand_isIsolatedChargedHadron"})
  .Filter("Sum(isoMask) > 0", "Has isolated charged pions")

  // Isoloitujen kenttien poiminta
  .Define("ptIso",      "PFCand_pt[isoMask]")
  .Define("etaIso",     "PFCand_eta[isoMask]")
  .Define("pIso",       "ptIso * cosh(etaIso)")
  .Define("hcalIso",    "PFCand_hcalEnergy[isoMask]")
  .Define("ecalIso",    "PFCand_ecalEnergy[isoMask]")
  .Define("rawEcalIso", "PFCand_rawEcalEnergy[isoMask]")
  .Define("rawHcalIso", "PFCand_rawHcalEnergy[isoMask]")

  // Summat ja E/p (vartioi nollajaot)
  .Define("detIsoE",     "ecalIso + hcalIso")
  .Define("rawDetIsoE",  "rawEcalIso + rawHcalIso")
  .Define("ep_def_all",  "ROOT::VecOps::Where(pIso > 0, (ecalIso + hcalIso)/pIso, -1.0)")
  .Define("ep_raw_all",  "ROOT::VecOps::Where(pIso > 0, (rawEcalIso + rawHcalIso)/pIso, -1.0)")

  // HCAL E/E_raw ja leikkausmaski
  .Define("hcalCorrFactor", "ROOT::VecOps::Where(rawHcalIso > 0, hcalIso/rawHcalIso, -1.0)")
  .Define("passHCAL",       "hcalCorrFactor > 0.9")

  // Leikatut E/p:t S1/S2-skenaarioille (maskatut arvot jätetään -1.0)
  .Define("ep_def_cut", "ROOT::VecOps::Where(passHCAL, ep_def_all, -1.0)") // S2
  .Define("ep_raw_cut", "ROOT::VecOps::Where(passHCAL, ep_raw_all, -1.0)"); // S1

// 2) Hadronimaskit + niistä johdetut vektorit (yhtenä ketjuna)
auto df_iso = df_base
  .Define("fracEcal", "ecalIso / pIso")
  .Define("fracHcal", "hcalIso / pIso")

  // Hadronityypit (bool-vektorit)
  .Define("isHadH",   "ROOT::VecOps::RVec<bool> v; "
                      "v.reserve(fracEcal.size()); "
                      "for (size_t i=0;i<fracEcal.size();++i) "
                      "  v.push_back((fracEcal[i] <= 0) && (fracHcal[i] > 0)); "
                      "return v;")
  .Define("isHadE",   "ROOT::VecOps::RVec<bool> v; "
                      "v.reserve(fracEcal.size()); "
                      "for (size_t i=0;i<fracEcal.size();++i) "
                      "  v.push_back((fracEcal[i] > 0) && (fracHcal[i] <= 0)); "
                      "return v;")
  .Define("isHadMIP", "ROOT::VecOps::RVec<bool> v; "
                      "v.reserve(fracEcal.size()); "
                      "for (size_t i=0;i<fracEcal.size();++i) "
                      "  v.push_back((fracEcal[i] > 0) && (fracEcal[i]*pIso[i] < 1) && (fracHcal[i] > 0)); "
                      "return v;")
  .Define("isHadEH",  "ROOT::VecOps::RVec<bool> v; "
                      "v.reserve(fracEcal.size()); "
                      "for (size_t i=0;i<fracEcal.size();++i) "
                      "  v.push_back((fracEcal[i] > 0) && (fracEcal[i]*pIso[i] >= 1) && (fracHcal[i] > 0)); "
                      "return v;")

  // bool -> float (fraktioprofiileja varten)
  .Define("isHadH_float",    "ROOT::VecOps::RVec<float> v; v.reserve(isHadH.size()); "
                             "for (auto x : isHadH)    v.push_back(x ? 1.f : 0.f); return v;")
  .Define("isHadE_float",    "ROOT::VecOps::RVec<float> v; v.reserve(isHadE.size()); "
                             "for (auto x : isHadE)    v.push_back(x ? 1.f : 0.f); return v;")
  .Define("isHadMIP_float",  "ROOT::VecOps::RVec<float> v; v.reserve(isHadMIP.size()); "
                             "for (auto x : isHadMIP)  v.push_back(x ? 1.f : 0.f); return v;")
  .Define("isHadEH_float",   "ROOT::VecOps::RVec<float> v; v.reserve(isHadEH.size()); "
                             "for (auto x : isHadEH)   v.push_back(x ? 1.f : 0.f); return v;")

  // Leikkausversiot (suodata sekä x että y passHCAL:lla)
  .Define("pIso_cut",           "pIso[passHCAL]")
  .Define("isHadH_float_cut",   "isHadH_float[passHCAL]")
  .Define("isHadE_float_cut",   "isHadE_float[passHCAL]")
  .Define("isHadMIP_float_cut", "isHadMIP_float[passHCAL]")
  .Define("isHadEH_float_cut",  "isHadEH_float[passHCAL]")

  // Raw E/p maskattuna hadronityypeittäin (1D-jakaumia varten)
  .Define("rawEpIso_H",   "ROOT::VecOps::Where(isHadH,   ep_raw_all, -1.0)")
  .Define("rawEpIso_E",   "ROOT::VecOps::Where(isHadE,   ep_raw_all, -1.0)")
  .Define("rawEpIso_MIP", "ROOT::VecOps::Where(isHadMIP, ep_raw_all, -1.0)")
  .Define("rawEpIso_EH",  "ROOT::VecOps::Where(isHadEH,  ep_raw_all, -1.0)");

// 2b) Per-tyyppiset maskit HCAL-cutilla + viipaleet (X=E/p default, Y=HCALcorr, Z=p)
auto df_iso2 = df_iso
  // yhdistelmämaskit: hadronityyppi AND passHCAL
  .Define("mask_H_cut", R"(
    ROOT::VecOps::RVec<char> m(isHadH.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadH[i] && passHCAL[i];
    return m;
  )")
  .Define("mask_E_cut", R"(
    ROOT::VecOps::RVec<char> m(isHadE.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadE[i] && passHCAL[i];
    return m;
  )")
  .Define("mask_MIP_cut", R"(
    ROOT::VecOps::RVec<char> m(isHadMIP.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadMIP[i] && passHCAL[i];
    return m;
  )")
  .Define("mask_EH_cut", R"(
    ROOT::VecOps::RVec<char> m(isHadEH.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadEH[i] && passHCAL[i];
    return m;
  )")

  // viipaleet: X=E/p (default), Y=HCALcorr, Z=p  (HCAL cut)
  .Define("ep_def_H_cut",   "ep_def_all[mask_H_cut]")
  .Define("ep_def_E_cut",   "ep_def_all[mask_E_cut]")
  .Define("ep_def_MIP_cut", "ep_def_all[mask_MIP_cut]")
  .Define("ep_def_EH_cut",  "ep_def_all[mask_EH_cut]")

  .Define("hcal_H_cut",     "hcalCorrFactor[mask_H_cut]")
  .Define("hcal_E_cut",     "hcalCorrFactor[mask_E_cut]")
  .Define("hcal_MIP_cut",   "hcalCorrFactor[mask_MIP_cut]")
  .Define("hcal_EH_cut",    "hcalCorrFactor[mask_EH_cut]")

  .Define("p_H_cut",        "pIso[mask_H_cut]")
  .Define("p_E_cut",        "pIso[mask_E_cut]")
  .Define("p_MIP_cut",      "pIso[mask_MIP_cut]")
  .Define("p_EH_cut",       "pIso[mask_EH_cut]");

// 2c) Per-tyyppiset maskit ILMAN HCAL-cuttia + viipaleet (X=E/p default, Y=HCALcorr, Z=p)
auto df_iso_nocut = df_iso
  // yhdistelmämaskit: vain hadronityyppi (ei passHCAL)
  .Define("mask_H_all", R"(
    ROOT::VecOps::RVec<char> m(isHadH.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadH[i];
    return m;
  )")
  .Define("mask_E_all", R"(
    ROOT::VecOps::RVec<char> m(isHadE.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadE[i];
    return m;
  )")
  .Define("mask_MIP_all", R"(
    ROOT::VecOps::RVec<char> m(isHadMIP.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadMIP[i];
    return m;
  )")
  .Define("mask_EH_all", R"(
    ROOT::VecOps::RVec<char> m(isHadEH.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadEH[i];
    return m;
  )")

  // viipaleet: X=E/p (default), Y=HCALcorr, Z=p  (ilman leikkausta)
  .Define("ep_def_H_all",   "ep_def_all[mask_H_all]")
  .Define("ep_def_E_all",   "ep_def_all[mask_E_all]")
  .Define("ep_def_MIP_all", "ep_def_all[mask_MIP_all]")
  .Define("ep_def_EH_all",  "ep_def_all[mask_EH_all]")

  .Define("hcal_H_all",     "hcalCorrFactor[mask_H_all]")
  .Define("hcal_E_all",     "hcalCorrFactor[mask_E_all]")
  .Define("hcal_MIP_all",   "hcalCorrFactor[mask_MIP_all]")
  .Define("hcal_EH_all",    "hcalCorrFactor[mask_EH_all]")

  .Define("p_H_all",        "pIso[mask_H_all]")
  .Define("p_E_all",        "pIso[mask_E_all]")
  .Define("p_MIP_all",      "pIso[mask_MIP_all]")
  .Define("p_EH_all",       "pIso[mask_EH_all]");

// RAW-versiot (no cut): vaihdetaan X-akseleiksi ep_raw_all[..]
auto df_iso_raw_all = df_iso_nocut
  .Define("ep_raw_H_all",   "ep_raw_all[mask_H_all]")
  .Define("ep_raw_E_all",   "ep_raw_all[mask_E_all]")
  .Define("ep_raw_MIP_all", "ep_raw_all[mask_MIP_all]")
  .Define("ep_raw_EH_all",  "ep_raw_all[mask_EH_all]");

// RAW + HCAL-cut versiot: ketjuta df_iso2:sta (mask_*_cut on määritelty siellä!)
auto df_iso_raw_cut = df_iso2
  .Define("ep_raw_H_cut",   "ep_raw_all[mask_H_cut]")
  .Define("ep_raw_E_cut",   "ep_raw_all[mask_E_cut]")
  .Define("ep_raw_MIP_cut", "ep_raw_all[mask_MIP_cut]")
  .Define("ep_raw_EH_cut",  "ep_raw_all[mask_EH_cut]");


// --- keräysvektorit ---
std::vector<ROOT::RDF::RResultPtr<TH1D>>     h1_histos;
std::vector<ROOT::RDF::RResultPtr<TH2D>>     h2_histos;
std::vector<ROOT::RDF::RResultPtr<TProfile>> profiles;
std::vector<ROOT::RDF::RResultPtr<TH3D>>     h3_histos;

// --- 1D: perusjakaumat, jotta plot_histograms.cc:n "histNames" löytyy ---
h1_histos.push_back(df_iso.Histo1D({"h_pt_iso",       "Isolated track p_{T};p_{T} [GeV];Events",  60, 0.0, 120.0}, "ptIso"));
h1_histos.push_back(df_iso.Histo1D({"h_eta_iso",      "Isolated track #eta;#eta;Events",          60, -3.0, 3.0},  "etaIso"));
h1_histos.push_back(df_iso.Histo1D({"h_ep_iso",       "Default E/p;E/p;Events",                    40, 0.05, 2.5},  "ep_def_all"));
h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_iso",   "Raw E/p;E/p;Events",                        40, 0.05, 2.5},  "ep_raw_all"));

// --- 1D: E/p per hadronityyppi (sekä default että raw) ---
df_iso = df_iso
  .Define("defEpIso_H",   "ROOT::VecOps::Where(isHadH,   ep_def_all, -1.0)")
  .Define("defEpIso_E",   "ROOT::VecOps::Where(isHadE,   ep_def_all, -1.0)")
  .Define("defEpIso_MIP", "ROOT::VecOps::Where(isHadMIP, ep_def_all, -1.0)")
  .Define("defEpIso_EH",  "ROOT::VecOps::Where(isHadEH,  ep_def_all, -1.0)");

h1_histos.push_back(df_iso.Histo1D({"h_ep_isHadH",   "Default E/p: HCAL only;E/p;Events", 40, 0.05, 2.5}, "defEpIso_H"));
h1_histos.push_back(df_iso.Histo1D({"h_ep_isHadE",   "Default E/p: ECAL only;E/p;Events", 40, 0.05, 2.5}, "defEpIso_E"));
h1_histos.push_back(df_iso.Histo1D({"h_ep_isHadMIP", "Default E/p: MIP;E/p;Events",       40, 0.05, 2.5}, "defEpIso_MIP"));
h1_histos.push_back(df_iso.Histo1D({"h_ep_isHadEH",  "Default E/p: ECAL+HCAL;E/p;Events", 40, 0.05, 2.5}, "defEpIso_EH"));

h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadH",   "Raw E/p: HCAL only;E/p;Events", 40, 0.05, 2.5}, "rawEpIso_H"));
h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadE",   "Raw E/p: ECAL only;E/p;Events", 40, 0.05, 2.5}, "rawEpIso_E"));
h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadMIP", "Raw E/p: MIP;E/p;Events",       40, 0.05, 2.5}, "rawEpIso_MIP"));
h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadEH",  "Raw E/p: ECAL+HCAL;E/p;Events", 40, 0.05, 2.5}, "rawEpIso_EH"));

// --- Fraktioprofiilit: no cut ---
profiles.push_back(df_iso.Profile1D({"h_frac_H_all",   "Fraction (no cut): HCAL only;p (GeV);fraction",    nTrkPBins, trkPBins}, "pIso", "isHadH_float"));
profiles.push_back(df_iso.Profile1D({"h_frac_E_all",   "Fraction (no cut): ECAL only;p (GeV);fraction",    nTrkPBins, trkPBins}, "pIso", "isHadE_float"));
profiles.push_back(df_iso.Profile1D({"h_frac_MIP_all", "Fraction (no cut): MIP;p (GeV);fraction",          nTrkPBins, trkPBins}, "pIso", "isHadMIP_float"));
profiles.push_back(df_iso.Profile1D({"h_frac_EH_all",  "Fraction (no cut): ECAL+HCAL;p (GeV);fraction",    nTrkPBins, trkPBins}, "pIso", "isHadEH_float"));

// --- Fraktioprofiilit: HCAL E/E_raw > 0.9 cut ---
profiles.push_back(df_iso.Profile1D({"h_frac_H_cut",   "Fraction (HCAL E/E_{raw}>0.9): HCAL only;p (GeV);fraction",    nTrkPBins, trkPBins}, "pIso_cut", "isHadH_float_cut"));
profiles.push_back(df_iso.Profile1D({"h_frac_E_cut",   "Fraction (HCAL E/E_{raw}>0.9): ECAL only;p (GeV);fraction",    nTrkPBins, trkPBins}, "pIso_cut", "isHadE_float_cut"));
profiles.push_back(df_iso.Profile1D({"h_frac_MIP_cut", "Fraction (HCAL E/E_{raw}>0.9): MIP;p (GeV);fraction",          nTrkPBins, trkPBins}, "pIso_cut", "isHadMIP_float_cut"));
profiles.push_back(df_iso.Profile1D({"h_frac_EH_cut",  "Fraction (HCAL E/E_{raw}>0.9): ECAL+HCAL;p (GeV);fraction",    nTrkPBins, trkPBins}, "pIso_cut", "isHadEH_float_cut"));

// --- 2D & profiilit: S1–S4 (nimet yhtenevät myöhempien plottien kanssa) ---
// S1: Raw/p, HCAL cut
h2_histos.push_back(df_iso.Histo2D({"h2_ep_vs_p_S1_raw_cut", "S1: Raw E/p vs p (HCAL E/E_{raw}>0.9);p (GeV);E/p",
                                    nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "ep_raw_cut"));
profiles .push_back(df_iso.Profile1D({"prof_ep_vs_p_S1_raw_cut", "S1: <Raw E/p> vs p (HCAL E/E_{raw}>0.9);p (GeV);E/p",
                                      nTrkPBins, trkPBins}, "pIso", "ep_raw_cut"));
h3_histos.push_back(df_iso.Histo3D({"h3_S1_raw_cut_ep_vs_hcal_p",
                                    "S1: Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (cut);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
                                    50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_raw_cut", "hcalCorrFactor", "pIso"));

// S2: Default/p, HCAL cut
h2_histos.push_back(df_iso.Histo2D({"h2_ep_vs_p_S2_def_cut", "S2: Default E/p vs p (HCAL E/E_{raw}>0.9);p (GeV);E/p",
                                    nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "ep_def_cut"));
profiles .push_back(df_iso.Profile1D({"prof_ep_vs_p_S2_def_cut", "S2: <Default E/p> vs p (HCAL E/E_{raw}>0.9);p (GeV);E/p",
                                      nTrkPBins, trkPBins}, "pIso", "ep_def_cut"));
h3_histos.push_back(df_iso.Histo3D({"h3_S2_def_cut_ep_vs_hcal_p",
                                    "S2: Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (cut);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
                                    50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_def_cut", "hcalCorrFactor", "pIso"));

// S3: Raw/p, no cut
h2_histos.push_back(df_iso.Histo2D({"h2_ep_vs_p_S3_raw_all", "S3: Raw E/p vs p (no cut);p (GeV);E/p",
                                    nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "ep_raw_all"));
profiles .push_back(df_iso.Profile1D({"prof_ep_vs_p_S3_raw_all", "S3: <Raw E/p> vs p (no cut);p (GeV);E/p",
                                      nTrkPBins, trkPBins}, "pIso", "ep_raw_all"));
h3_histos.push_back(df_iso.Histo3D({"h3_S3_raw_ep_vs_hcal_p",
                                    "S3: Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p;E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
                                    50, 0.0, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_raw_all", "hcalCorrFactor", "pIso"));

// S4: Default/p, no cut
h2_histos.push_back(df_iso.Histo2D({"h2_ep_vs_p_S4_def_all", "S4: Default E/p vs p (no cut);p (GeV);E/p",
                                    nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "ep_def_all"));
profiles .push_back(df_iso.Profile1D({"prof_ep_vs_p_S4_def_all", "S4: <Default E/p> vs p (no cut);p (GeV);E/p",
                                      nTrkPBins, trkPBins}, "pIso", "ep_def_all"));
h3_histos.push_back(df_iso.Histo3D({"h3_S4_def_ep_vs_hcal_p",
                                    "S4: Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p;E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
                                    50, 0.0, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_def_all", "hcalCorrFactor", "pIso"));

// --- Per-tyyppiset 3D:t: Default E/p + HCAL cut (plot_histograms.cc käyttää näitä) ---
h3_histos.push_back(df_iso2.Histo3D({"h3_resp_corr_p_isHadH",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, HCAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_def_H_cut", "hcal_H_cut", "p_H_cut"));
h3_histos.push_back(df_iso2.Histo3D({"h3_resp_corr_p_isHadE",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, ECAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_def_E_cut", "hcal_E_cut", "p_E_cut"));
h3_histos.push_back(df_iso2.Histo3D({"h3_resp_corr_p_isHadMIP",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, MIP);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_def_MIP_cut", "hcal_MIP_cut", "p_MIP_cut"));
h3_histos.push_back(df_iso2.Histo3D({"h3_resp_corr_p_isHadEH",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, ECAL+HCAL);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_def_EH_cut", "hcal_EH_cut", "p_EH_cut"));

// --- Per-tyyppiset 3D:t: Default E/p, no cut (Y 0.0–2.5) ---
h3_histos.push_back(df_iso_nocut.Histo3D({"h3_resp_def_p_isHadH_all",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, HCAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_def_H_all", "hcal_H_all", "p_H_all"));
h3_histos.push_back(df_iso_nocut.Histo3D({"h3_resp_def_p_isHadE_all",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, ECAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_def_E_all", "hcal_E_all", "p_E_all"));
h3_histos.push_back(df_iso_nocut.Histo3D({"h3_resp_def_p_isHadMIP_all",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, MIP);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_def_MIP_all", "hcal_MIP_all", "p_MIP_all"));
h3_histos.push_back(df_iso_nocut.Histo3D({"h3_resp_def_p_isHadEH_all",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, ECAL+HCAL);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_def_EH_all", "hcal_EH_all", "p_EH_all"));

// --- Per-tyyppiset 3D:t: Raw E/p, no cut (Y 0.0–2.5) ---
h3_histos.push_back(df_iso_raw_all.Histo3D({"h3_resp_raw_p_isHadH_all",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, HCAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_raw_H_all", "hcal_H_all", "p_H_all"));
h3_histos.push_back(df_iso_raw_all.Histo3D({"h3_resp_raw_p_isHadE_all",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, ECAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_raw_E_all", "hcal_E_all", "p_E_all"));
h3_histos.push_back(df_iso_raw_all.Histo3D({"h3_resp_raw_p_isHadMIP_all",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, MIP);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_raw_MIP_all", "hcal_MIP_all", "p_MIP_all"));
h3_histos.push_back(df_iso_raw_all.Histo3D({"h3_resp_raw_p_isHadEH_all",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, ECAL+HCAL);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_raw_EH_all", "hcal_EH_all", "p_EH_all"));

// --- Per-tyyppiset 3D:t: Raw E/p + HCAL cut (pyysit nämä nimillä "h3_resp_raw_p_isHad*_cut") ---
h3_histos.push_back(df_iso_raw_cut.Histo3D({"h3_resp_raw_p_isHadH_cut",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, HCAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_raw_H_cut", "hcal_H_cut", "p_H_cut"));
h3_histos.push_back(df_iso_raw_cut.Histo3D({"h3_resp_raw_p_isHadE_cut",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, ECAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_raw_E_cut", "hcal_E_cut", "p_E_cut"));
h3_histos.push_back(df_iso_raw_cut.Histo3D({"h3_resp_raw_p_isHadMIP_cut",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, MIP);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_raw_MIP_cut", "hcal_MIP_cut", "p_MIP_cut"));
h3_histos.push_back(df_iso_raw_cut.Histo3D({"h3_resp_raw_p_isHadEH_cut",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, ECAL+HCAL);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_raw_EH_cut", "hcal_EH_cut", "p_EH_cut"));


// --- AVAA ROOT-tiedosto ---
std::string finalPath;
if (outpath && std::string(outpath).size()>0) {
  finalPath = outpath;
  // tee hakemisto, jos ei olemassa
  std::string dir = gSystem->DirName(finalPath.c_str());
  if (dir != "" && dir != "." && dir != "/") {
    gSystem->mkdir(dir.c_str(), true);
  }
} else {
  std::string outdir = "histograms";
  gSystem->mkdir(outdir.c_str(), true);
  finalPath = outdir + "/" + std::string(tag) + ".root";
}


    // luodaan RDataFrame ...
    ROOT::RDataFrame df("Events", files);

    // Lisätään progress-tulostus
    auto df_mon = df.Define("dummyCounter", [](ULong64_t entry){
        if (entry % 100000 == 0) {  // tulosta joka 100k eventti
            std::cout << "[Progress] Processed " << entry << " events" << std::endl;
        }
        return 0;
    }, {"rdfentry_"});

    // SUODATUS JA MUODOSTETTAVAT SUUREET
    // Käytetään df_mon eikä df
    auto df_base = df_mon
      .Define("isoMask",
              [](const ROOT::VecOps::RVec<int>& pdgId,
                const ROOT::VecOps::RVec<bool>& isoFlag) {
                ROOT::VecOps::RVec<bool> m(pdgId.size());
                for (size_t i = 0; i < pdgId.size(); ++i)
                  m[i] = (std::abs(pdgId[i]) == 211) && isoFlag[i];
                return m;
              },
              {"PFCand_pdgId","PFCand_isIsolatedChargedHadron"})
      .Filter("Sum(isoMask) > 0", "Has isolated charged pions")


  // Isoloitujen kenttien poiminta
  .Define("ptIso",      "PFCand_pt[isoMask]")
  .Define("etaIso",     "PFCand_eta[isoMask]")
  .Define("pIso",       "ptIso * cosh(etaIso)")
  .Define("hcalIso",    "PFCand_hcalEnergy[isoMask]")
  .Define("ecalIso",    "PFCand_ecalEnergy[isoMask]")
  .Define("rawEcalIso", "PFCand_rawEcalEnergy[isoMask]")
  .Define("rawHcalIso", "PFCand_rawHcalEnergy[isoMask]")

  // Summat ja E/p (vartioi nollajaot)
  .Define("detIsoE",     "ecalIso + hcalIso")
  .Define("rawDetIsoE",  "rawEcalIso + rawHcalIso")
  .Define("ep_def_all",  "ROOT::VecOps::Where(pIso > 0, (ecalIso + hcalIso)/pIso, -1.0)")
  .Define("ep_raw_all",  "ROOT::VecOps::Where(pIso > 0, (rawEcalIso + rawHcalIso)/pIso, -1.0)")

  // HCAL E/E_raw ja leikkausmaski
  .Define("hcalCorrFactor", "ROOT::VecOps::Where(rawHcalIso > 0, hcalIso/rawHcalIso, -1.0)")
  .Define("passHCAL",       "hcalCorrFactor > 0.9")

  // Leikatut E/p:t S1/S2-skenaarioille (maskatut arvot jätetään -1.0)
  .Define("ep_def_cut", "ROOT::VecOps::Where(passHCAL, ep_def_all, -1.0)") // S2
  .Define("ep_raw_cut", "ROOT::VecOps::Where(passHCAL, ep_raw_all, -1.0)"); // S1

// 2) Hadronimaskit + niistä johdetut vektorit (yhtenä ketjuna)
auto df_iso = df_base
  .Define("fracEcal", "ecalIso / pIso")
  .Define("fracHcal", "hcalIso / pIso")

  // Hadronityypit (bool-vektorit)
  .Define("isHadH",   "ROOT::VecOps::RVec<bool> v; "
                      "v.reserve(fracEcal.size()); "
                      "for (size_t i=0;i<fracEcal.size();++i) "
                      "  v.push_back((fracEcal[i] <= 0) && (fracHcal[i] > 0)); "
                      "return v;")
  .Define("isHadE",   "ROOT::VecOps::RVec<bool> v; "
                      "v.reserve(fracEcal.size()); "
                      "for (size_t i=0;i<fracEcal.size();++i) "
                      "  v.push_back((fracEcal[i] > 0) && (fracHcal[i] <= 0)); "
                      "return v;")
  .Define("isHadMIP", "ROOT::VecOps::RVec<bool> v; "
                      "v.reserve(fracEcal.size()); "
                      "for (size_t i=0;i<fracEcal.size();++i) "
                      "  v.push_back((fracEcal[i] > 0) && (fracEcal[i]*pIso[i] < 1) && (fracHcal[i] > 0)); "
                      "return v;")
  .Define("isHadEH",  "ROOT::VecOps::RVec<bool> v; "
                      "v.reserve(fracEcal.size()); "
                      "for (size_t i=0;i<fracEcal.size();++i) "
                      "  v.push_back((fracEcal[i] > 0) && (fracEcal[i]*pIso[i] >= 1) && (fracHcal[i] > 0)); "
                      "return v;")

  // bool -> float (fraktioprofiileja varten)
  .Define("isHadH_float",    "ROOT::VecOps::RVec<float> v; v.reserve(isHadH.size()); "
                             "for (auto x : isHadH)    v.push_back(x ? 1.f : 0.f); return v;")
  .Define("isHadE_float",    "ROOT::VecOps::RVec<float> v; v.reserve(isHadE.size()); "
                             "for (auto x : isHadE)    v.push_back(x ? 1.f : 0.f); return v;")
  .Define("isHadMIP_float",  "ROOT::VecOps::RVec<float> v; v.reserve(isHadMIP.size()); "
                             "for (auto x : isHadMIP)  v.push_back(x ? 1.f : 0.f); return v;")
  .Define("isHadEH_float",   "ROOT::VecOps::RVec<float> v; v.reserve(isHadEH.size()); "
                             "for (auto x : isHadEH)   v.push_back(x ? 1.f : 0.f); return v;")

  // Leikkausversiot (suodata sekä x että y passHCAL:lla)
  .Define("pIso_cut",           "pIso[passHCAL]")
  .Define("isHadH_float_cut",   "isHadH_float[passHCAL]")
  .Define("isHadE_float_cut",   "isHadE_float[passHCAL]")
  .Define("isHadMIP_float_cut", "isHadMIP_float[passHCAL]")
  .Define("isHadEH_float_cut",  "isHadEH_float[passHCAL]")

  // Raw E/p maskattuna hadronityypeittäin (1D-jakaumia varten)
  .Define("rawEpIso_H",   "ROOT::VecOps::Where(isHadH,   ep_raw_all, -1.0)")
  .Define("rawEpIso_E",   "ROOT::VecOps::Where(isHadE,   ep_raw_all, -1.0)")
  .Define("rawEpIso_MIP", "ROOT::VecOps::Where(isHadMIP, ep_raw_all, -1.0)")
  .Define("rawEpIso_EH",  "ROOT::VecOps::Where(isHadEH,  ep_raw_all, -1.0)");

// 2b) Per-tyyppiset maskit HCAL-cutilla + viipaleet (X=E/p default, Y=HCALcorr, Z=p)
auto df_iso2 = df_iso
  // yhdistelmämaskit: hadronityyppi AND passHCAL
  .Define("mask_H_cut", R"(
    ROOT::VecOps::RVec<char> m(isHadH.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadH[i] && passHCAL[i];
    return m;
  )")
  .Define("mask_E_cut", R"(
    ROOT::VecOps::RVec<char> m(isHadE.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadE[i] && passHCAL[i];
    return m;
  )")
  .Define("mask_MIP_cut", R"(
    ROOT::VecOps::RVec<char> m(isHadMIP.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadMIP[i] && passHCAL[i];
    return m;
  )")
  .Define("mask_EH_cut", R"(
    ROOT::VecOps::RVec<char> m(isHadEH.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadEH[i] && passHCAL[i];
    return m;
  )")

  // viipaleet: X=E/p (default), Y=HCALcorr, Z=p  (HCAL cut)
  .Define("ep_def_H_cut",   "ep_def_all[mask_H_cut]")
  .Define("ep_def_E_cut",   "ep_def_all[mask_E_cut]")
  .Define("ep_def_MIP_cut", "ep_def_all[mask_MIP_cut]")
  .Define("ep_def_EH_cut",  "ep_def_all[mask_EH_cut]")

  .Define("hcal_H_cut",     "hcalCorrFactor[mask_H_cut]")
  .Define("hcal_E_cut",     "hcalCorrFactor[mask_E_cut]")
  .Define("hcal_MIP_cut",   "hcalCorrFactor[mask_MIP_cut]")
  .Define("hcal_EH_cut",    "hcalCorrFactor[mask_EH_cut]")

  .Define("p_H_cut",        "pIso[mask_H_cut]")
  .Define("p_E_cut",        "pIso[mask_E_cut]")
  .Define("p_MIP_cut",      "pIso[mask_MIP_cut]")
  .Define("p_EH_cut",       "pIso[mask_EH_cut]");

// Per-tyyppiset maskit ILMAN HCAL-cuttia + viipaleet (X=E/p default, Y=HCALcorr, Z=p)
auto df_iso_nocut = df_iso
  // yhdistelmämaskit: vain hadronityyppi (ei passHCAL)
  .Define("mask_H_all", R"(
    ROOT::VecOps::RVec<char> m(isHadH.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadH[i];
    return m;
  )")
  .Define("mask_E_all", R"(
    ROOT::VecOps::RVec<char> m(isHadE.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadE[i];
    return m;
  )")
  .Define("mask_MIP_all", R"(
    ROOT::VecOps::RVec<char> m(isHadMIP.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadMIP[i];
    return m;
  )")
  .Define("mask_EH_all", R"(
    ROOT::VecOps::RVec<char> m(isHadEH.size());
    for (size_t i = 0; i < m.size(); ++i) m[i] = isHadEH[i];
    return m;
  )")

  // viipaleet: X=E/p (default), Y=HCALcorr, Z=p  (ilman leikkausta)
  .Define("ep_def_H_all",   "ep_def_all[mask_H_all]")
  .Define("ep_def_E_all",   "ep_def_all[mask_E_all]")
  .Define("ep_def_MIP_all", "ep_def_all[mask_MIP_all]")
  .Define("ep_def_EH_all",  "ep_def_all[mask_EH_all]")

  .Define("hcal_H_all",     "hcalCorrFactor[mask_H_all]")
  .Define("hcal_E_all",     "hcalCorrFactor[mask_E_all]")
  .Define("hcal_MIP_all",   "hcalCorrFactor[mask_MIP_all]")
  .Define("hcal_EH_all",    "hcalCorrFactor[mask_EH_all]")

  .Define("p_H_all",        "pIso[mask_H_all]")
  .Define("p_E_all",        "pIso[mask_E_all]")
  .Define("p_MIP_all",      "pIso[mask_MIP_all]")
  .Define("p_EH_all",       "pIso[mask_EH_all]");

// RAW-versiot (no cut): vaihdetaan X-akseleiksi ep_raw_all[..]
auto df_iso_raw_all = df_iso_nocut
  .Define("ep_raw_H_all",   "ep_raw_all[mask_H_all]")
  .Define("ep_raw_E_all",   "ep_raw_all[mask_E_all]")
  .Define("ep_raw_MIP_all", "ep_raw_all[mask_MIP_all]")
  .Define("ep_raw_EH_all",  "ep_raw_all[mask_EH_all]");

// RAW + HCAL-cut versiot: ketjuta df_iso2:sta (mask_*_cut on määritelty siellä)
auto df_iso_raw_cut = df_iso2
  .Define("ep_raw_H_cut",   "ep_raw_all[mask_H_cut]")
  .Define("ep_raw_E_cut",   "ep_raw_all[mask_E_cut]")
  .Define("ep_raw_MIP_cut", "ep_raw_all[mask_MIP_cut]")
  .Define("ep_raw_EH_cut",  "ep_raw_all[mask_EH_cut]");


// keräysvektorit
std::vector<ROOT::RDF::RResultPtr<TH1D>>     h1_histos;
std::vector<ROOT::RDF::RResultPtr<TH2D>>     h2_histos;
std::vector<ROOT::RDF::RResultPtr<TProfile>> profiles;
std::vector<ROOT::RDF::RResultPtr<TH3D>>     h3_histos;

// 1D: perusjakaumat, jotta plot_histograms.cc:n "histNames" löytyy
h1_histos.push_back(df_iso.Histo1D({"h_pt_iso",       "Isolated track p_{T};p_{T} [GeV];Events",  60, 0.0, 120.0}, "ptIso"));
h1_histos.push_back(df_iso.Histo1D({"h_eta_iso",      "Isolated track #eta;#eta;Events",          60, -3.0, 3.0},  "etaIso"));
h1_histos.push_back(df_iso.Histo1D({"h_ep_iso",       "Default E/p;E/p;Events",                    40, 0.05, 2.5},  "ep_def_all"));
h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_iso",   "Raw E/p;E/p;Events",                        40, 0.05, 2.5},  "ep_raw_all"));

// 1D: E/p per hadronityyppi (sekä default että raw)
df_iso = df_iso
  .Define("defEpIso_H",   "ROOT::VecOps::Where(isHadH,   ep_def_all, -1.0)")
  .Define("defEpIso_E",   "ROOT::VecOps::Where(isHadE,   ep_def_all, -1.0)")
  .Define("defEpIso_MIP", "ROOT::VecOps::Where(isHadMIP, ep_def_all, -1.0)")
  .Define("defEpIso_EH",  "ROOT::VecOps::Where(isHadEH,  ep_def_all, -1.0)");

h1_histos.push_back(df_iso.Histo1D({"h_ep_isHadH",   "Default E/p: HCAL only;E/p;Events", 40, 0.05, 2.5}, "defEpIso_H"));
h1_histos.push_back(df_iso.Histo1D({"h_ep_isHadE",   "Default E/p: ECAL only;E/p;Events", 40, 0.05, 2.5}, "defEpIso_E"));
h1_histos.push_back(df_iso.Histo1D({"h_ep_isHadMIP", "Default E/p: MIP;E/p;Events",       40, 0.05, 2.5}, "defEpIso_MIP"));
h1_histos.push_back(df_iso.Histo1D({"h_ep_isHadEH",  "Default E/p: ECAL+HCAL;E/p;Events", 40, 0.05, 2.5}, "defEpIso_EH"));

h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadH",   "Raw E/p: HCAL only;E/p;Events", 40, 0.05, 2.5}, "rawEpIso_H"));
h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadE",   "Raw E/p: ECAL only;E/p;Events", 40, 0.05, 2.5}, "rawEpIso_E"));
h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadMIP", "Raw E/p: MIP;E/p;Events",       40, 0.05, 2.5}, "rawEpIso_MIP"));
h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadEH",  "Raw E/p: ECAL+HCAL;E/p;Events", 40, 0.05, 2.5}, "rawEpIso_EH"));

// Fraktioprofiilit: no cut
profiles.push_back(df_iso.Profile1D({"h_frac_H_all",   "Fraction (no cut): HCAL only;p (GeV);fraction",    nTrkPBins, trkPBins}, "pIso", "isHadH_float"));
profiles.push_back(df_iso.Profile1D({"h_frac_E_all",   "Fraction (no cut): ECAL only;p (GeV);fraction",    nTrkPBins, trkPBins}, "pIso", "isHadE_float"));
profiles.push_back(df_iso.Profile1D({"h_frac_MIP_all", "Fraction (no cut): MIP;p (GeV);fraction",          nTrkPBins, trkPBins}, "pIso", "isHadMIP_float"));
profiles.push_back(df_iso.Profile1D({"h_frac_EH_all",  "Fraction (no cut): ECAL+HCAL;p (GeV);fraction",    nTrkPBins, trkPBins}, "pIso", "isHadEH_float"));

// Fraktioprofiilit: HCAL E/E_raw > 0.9 cut
profiles.push_back(df_iso.Profile1D({"h_frac_H_cut",   "Fraction (HCAL E/E_{raw}>0.9): HCAL only;p (GeV);fraction",    nTrkPBins, trkPBins}, "pIso_cut", "isHadH_float_cut"));
profiles.push_back(df_iso.Profile1D({"h_frac_E_cut",   "Fraction (HCAL E/E_{raw}>0.9): ECAL only;p (GeV);fraction",    nTrkPBins, trkPBins}, "pIso_cut", "isHadE_float_cut"));
profiles.push_back(df_iso.Profile1D({"h_frac_MIP_cut", "Fraction (HCAL E/E_{raw}>0.9): MIP;p (GeV);fraction",          nTrkPBins, trkPBins}, "pIso_cut", "isHadMIP_float_cut"));
profiles.push_back(df_iso.Profile1D({"h_frac_EH_cut",  "Fraction (HCAL E/E_{raw}>0.9): ECAL+HCAL;p (GeV);fraction",    nTrkPBins, trkPBins}, "pIso_cut", "isHadEH_float_cut"));

// 2D & profiilit: S1–S4 (nimet yhtenevät myöhempien plottien kanssa)
// S1: Raw/p, HCAL cut
h2_histos.push_back(df_iso.Histo2D({"h2_ep_vs_p_S1_raw_cut", "S1: Raw E/p vs p (HCAL E/E_{raw}>0.9);p (GeV);E/p",
                                    nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "ep_raw_cut"));
profiles .push_back(df_iso.Profile1D({"prof_ep_vs_p_S1_raw_cut", "S1: <Raw E/p> vs p (HCAL E/E_{raw}>0.9);p (GeV);E/p",
                                      nTrkPBins, trkPBins}, "pIso", "ep_raw_cut"));
h3_histos.push_back(df_iso.Histo3D({"h3_S1_raw_cut_ep_vs_hcal_p",
                                    "S1: Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (cut);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
                                    50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_raw_cut", "hcalCorrFactor", "pIso"));

// S2: Default/p, HCAL cut
h2_histos.push_back(df_iso.Histo2D({"h2_ep_vs_p_S2_def_cut", "S2: Default E/p vs p (HCAL E/E_{raw}>0.9);p (GeV);E/p",
                                    nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "ep_def_cut"));
profiles .push_back(df_iso.Profile1D({"prof_ep_vs_p_S2_def_cut", "S2: <Default E/p> vs p (HCAL E/E_{raw}>0.9);p (GeV);E/p",
                                      nTrkPBins, trkPBins}, "pIso", "ep_def_cut"));
h3_histos.push_back(df_iso.Histo3D({"h3_S2_def_cut_ep_vs_hcal_p",
                                    "S2: Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (cut);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
                                    50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_def_cut", "hcalCorrFactor", "pIso"));

// S3: Raw/p, no cut
h2_histos.push_back(df_iso.Histo2D({"h2_ep_vs_p_S3_raw_all", "S3: Raw E/p vs p (no cut);p (GeV);E/p",
                                    nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "ep_raw_all"));
profiles .push_back(df_iso.Profile1D({"prof_ep_vs_p_S3_raw_all", "S3: <Raw E/p> vs p (no cut);p (GeV);E/p",
                                      nTrkPBins, trkPBins}, "pIso", "ep_raw_all"));
h3_histos.push_back(df_iso.Histo3D({"h3_S3_raw_ep_vs_hcal_p",
                                    "S3: Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p;E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
                                    50, 0.0, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_raw_all", "hcalCorrFactor", "pIso"));

// S4: Default/p, no cut
h2_histos.push_back(df_iso.Histo2D({"h2_ep_vs_p_S4_def_all", "S4: Default E/p vs p (no cut);p (GeV);E/p",
                                    nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "ep_def_all"));
profiles .push_back(df_iso.Profile1D({"prof_ep_vs_p_S4_def_all", "S4: <Default E/p> vs p (no cut);p (GeV);E/p",
                                      nTrkPBins, trkPBins}, "pIso", "ep_def_all"));
h3_histos.push_back(df_iso.Histo3D({"h3_S4_def_ep_vs_hcal_p",
                                    "S4: Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p;E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
                                    50, 0.0, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_def_all", "hcalCorrFactor", "pIso"));

// Per-tyyppiset 3D:t: Default E/p + HCAL cut (plot_histograms.cc käyttää näitä)
h3_histos.push_back(df_iso2.Histo3D({"h3_resp_corr_p_isHadH",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, HCAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_def_H_cut", "hcal_H_cut", "p_H_cut"));
h3_histos.push_back(df_iso2.Histo3D({"h3_resp_corr_p_isHadE",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, ECAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_def_E_cut", "hcal_E_cut", "p_E_cut"));
h3_histos.push_back(df_iso2.Histo3D({"h3_resp_corr_p_isHadMIP",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, MIP);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_def_MIP_cut", "hcal_MIP_cut", "p_MIP_cut"));
h3_histos.push_back(df_iso2.Histo3D({"h3_resp_corr_p_isHadEH",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, ECAL+HCAL);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_def_EH_cut", "hcal_EH_cut", "p_EH_cut"));

// Per-tyyppiset 3D:t: Default E/p, no cut (Y 0.0–2.5)
h3_histos.push_back(df_iso_nocut.Histo3D({"h3_resp_def_p_isHadH_all",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, HCAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_def_H_all", "hcal_H_all", "p_H_all"));
h3_histos.push_back(df_iso_nocut.Histo3D({"h3_resp_def_p_isHadE_all",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, ECAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_def_E_all", "hcal_E_all", "p_E_all"));
h3_histos.push_back(df_iso_nocut.Histo3D({"h3_resp_def_p_isHadMIP_all",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, MIP);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_def_MIP_all", "hcal_MIP_all", "p_MIP_all"));
h3_histos.push_back(df_iso_nocut.Histo3D({"h3_resp_def_p_isHadEH_all",
  "Default E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, ECAL+HCAL);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_def_EH_all", "hcal_EH_all", "p_EH_all"));

// Per-tyyppiset 3D:t: Raw E/p, no cut (Y 0.0–2.5)
h3_histos.push_back(df_iso_raw_all.Histo3D({"h3_resp_raw_p_isHadH_all",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, HCAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_raw_H_all", "hcal_H_all", "p_H_all"));
h3_histos.push_back(df_iso_raw_all.Histo3D({"h3_resp_raw_p_isHadE_all",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, ECAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_raw_E_all", "hcal_E_all", "p_E_all"));
h3_histos.push_back(df_iso_raw_all.Histo3D({"h3_resp_raw_p_isHadMIP_all",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, MIP);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_raw_MIP_all", "hcal_MIP_all", "p_MIP_all"));
h3_histos.push_back(df_iso_raw_all.Histo3D({"h3_resp_raw_p_isHadEH_all",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (no cut, ECAL+HCAL);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.0, 2.5,  30, 0.0, 100.0}, "ep_raw_EH_all", "hcal_EH_all", "p_EH_all"));

// Per-tyyppiset 3D:t: Raw E/p + HCAL cut (pyysit nämä nimillä "h3_resp_raw_p_isHad*_cut")
h3_histos.push_back(df_iso_raw_cut.Histo3D({"h3_resp_raw_p_isHadH_cut",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, HCAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_raw_H_cut", "hcal_H_cut", "p_H_cut"));
h3_histos.push_back(df_iso_raw_cut.Histo3D({"h3_resp_raw_p_isHadE_cut",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, ECAL only);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_raw_E_cut", "hcal_E_cut", "p_E_cut"));
h3_histos.push_back(df_iso_raw_cut.Histo3D({"h3_resp_raw_p_isHadMIP_cut",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, MIP);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_raw_MIP_cut", "hcal_MIP_cut", "p_MIP_cut"));
h3_histos.push_back(df_iso_raw_cut.Histo3D({"h3_resp_raw_p_isHadEH_cut",
  "Raw E/p vs E_{HCAL}/E_{HCAL}^{raw} vs p (HCAL cut, ECAL+HCAL);E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)",
  50, 0.05, 2.5,  50, 0.9, 2.5,  30, 0.0, 100.0}, "ep_raw_EH_cut", "hcal_EH_cut", "p_EH_cut"));


//  KIRJOITA TIEDOSTOON (kirjoitus laukaisee myös evaluoinnin)
// root tiedoston avaus
std::string finalPath;
if (outpath && std::string(outpath).size()>0) {
  finalPath = outpath;
  // tee hakemisto, jos ei olemassa
  std::string dir = gSystem->DirName(finalPath.c_str());
  if (dir != "" && dir != "." && dir != "/") {
    gSystem->mkdir(dir.c_str(), true);
  }
} else {
  std::string outdir = "histograms";
  gSystem->mkdir(outdir.c_str(), true);
  finalPath = outdir + "/" + std::string(tag) + ".root";
}

TFile out(finalPath.c_str(), "RECREATE");

// kirjoitetaan root tiedostoon (kirjoitus aloittaa myös evaluoinnin) ---
for (auto& h : h1_histos) h->Write();
for (auto& h : h2_histos) h->Write();
for (auto& h : h3_histos) h->Write();
for (auto& p : profiles)  p->Write();

out.Close();

// tulostetaan kokonaisajankesto
auto end = std::chrono::high_resolution_clock::now();
std::cout << "Wrote " << finalPath << " | Elapsed time: "
          << std::chrono::duration<double>(end - start).count() << " s\n";
}