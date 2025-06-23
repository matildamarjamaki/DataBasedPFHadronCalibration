
#include "ROOT/RDataFrame.hxx"
#include "TFile.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TProfile.h"
#include "TCanvas.h"
#include "TStyle.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <tuple>
#include <chrono>

// track-p binitys, logiikka perustuu epälineaariseen binitykseen pienillä momenteilla
const double trkPBins[] = {
    3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
    7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
};
const int nTrkPBins = sizeof(trkPBins) / sizeof(double) - 1;

void run_histograms(const char* listfile, const char* tag) {
    auto start = std::chrono::high_resolution_clock::now();
    ROOT::EnableImplicitMT();

    // luetaan ROOT-tiedostolistauksen rivit ja muodostetaan polut EOS-palvelimelle
    std::ifstream infile(listfile);
    std::string line;
    std::vector<std::string> files;
    while (std::getline(infile, line)) {
        if (!line.empty())
            files.push_back("root://hip-cms-se.csc.fi/" + line);
    }

    std::cout << "Processing " << files.size() << " files..." << std::endl;

    // luodaan RDataFrame ROOT-puu nimeltä "Events"
    ROOT::RDataFrame df("Events", files);
    ROOT::RDF::Experimental::AddProgressBar(df);

    // määritellään tarvittavat maskit ja suureet (vain isoloidut, varatut)
    auto df_iso = df
        .Define("isoMask", [](const ROOT::VecOps::RVec<int>& pdgId,
                               const ROOT::VecOps::RVec<bool>& isoFlag) {
            ROOT::VecOps::RVec<bool> mask(pdgId.size());
            for (size_t i = 0; i < pdgId.size(); ++i) {
                mask[i] = (std::abs(pdgId[i]) == 211) && isoFlag[i];
            }
            return mask;
        }, {"PFCand_pdgId", "PFCand_isIsolatedChargedHadron"})
        .Filter("Sum(isoMask) > 0", "Has isolated charged pions")
        // otetaan pT ja energia isoloiduille
        .Define("ptIso", "PFCand_pt[isoMask]")
        .Define("etaIso", "PFCand_eta[isoMask]")
        .Define("pIso", "ptIso * cosh(etaIso)")
        .Define("ecalIso", "PFCand_ecalEnergy[isoMask]")
        .Define("hcalIso", "PFCand_hcalEnergy[isoMask]")
        .Define("rawEcalIso", "PFCand_rawEcalEnergy[isoMask]")
        .Define("rawHcalIso", "PFCand_rawHcalEnergy[isoMask]")
        // määritellään E/p ja muut johdetut suureet
        .Define("detIsoE", "ecalIso + hcalIso")
        .Define("rawDetIsoE", "rawEcalIso + rawHcalIso")
        .Define("epIso", "detIsoE / pIso")
        .Define("rawEpIso", "rawDetIsoE / pIso")
        .Define("fracEcal", "ecalIso / pIso")
        .Define("fracHcal", "hcalIso / pIso")
        // hadronityypin maskit (HCAL only, ECAL only, MIP, ECAL+HCAL)
        .Define("isHadH", "ROOT::VecOps::RVec<bool> v; for (size_t i = 0; i < fracEcal.size(); ++i) v.push_back((fracEcal[i] <= 0) && (fracHcal[i] > 0)); return v;")
        .Define("isHadE", "ROOT::VecOps::RVec<bool> v; for (size_t i = 0; i < fracEcal.size(); ++i) v.push_back((fracEcal[i] > 0) && (fracHcal[i] <= 0)); return v;")
        .Define("isHadMIP", "ROOT::VecOps::RVec<bool> v; for (size_t i = 0; i < fracEcal.size(); ++i) v.push_back((fracEcal[i] > 0) && (fracEcal[i] * pIso[i] < 1) && (fracHcal[i] > 0)); return v;")
        .Define("isHadEH", "ROOT::VecOps::RVec<bool> v; for (size_t i = 0; i < fracEcal.size(); ++i) v.push_back((fracEcal[i] > 0) && (fracEcal[i] * pIso[i] >= 1) && (fracHcal[i] > 0)); return v;")
        // korjauskerroin HCAL:lle ja osittain valmiiksi suodatetut E/p:t
        .Define("hcalCorrFactor", "hcalIso / rawHcalIso")
        .Define("response", "detIsoE / pIso")
        .Define("rawEpIso_H", "ROOT::VecOps::Where(isHadH, rawEpIso, -1.0)")
        .Define("rawEpIso_E", "ROOT::VecOps::Where(isHadE, rawEpIso, -1.0)")
        .Define("rawEpIso_MIP", "ROOT::VecOps::Where(isHadMIP, rawEpIso, -1.0)")
        .Define("rawEpIso_EH", "ROOT::VecOps::Where(isHadEH, rawEpIso, -1.0)");

    // histogrammivektorit eri tyypeille
    std::vector<ROOT::RDF::RResultPtr<TH1D>> h1_histos;
    std::vector<ROOT::RDF::RResultPtr<TH2D>> h2_histos;
    std::vector<ROOT::RDF::RResultPtr<TProfile>> profiles;
    std::vector<ROOT::RDF::RResultPtr<TH3D>> h3_histos;

    // 2D-histogrammit E/p vs p
    h2_histos.push_back(df_iso.Histo2D({"h2_ep_vs_p_iso", "E/p vs p; p (GeV); E/p", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "epIso"));
    h2_histos.push_back(df_iso.Histo2D({"h2_raw_ep_vs_p_iso", "Raw E/p vs p; p (GeV); Raw E/p", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "rawEpIso"));

    // profiilit (keskiarvo E/p vs p)
    profiles.push_back(df_iso.Profile1D({"prof_ep_vs_p_iso", "<E/p> vs p; p (GeV); E/p", 50, 0, 10}, "pIso", "epIso"));
    profiles.push_back(df_iso.Profile1D({"prof_raw_ep_vs_p_iso", "<Raw E/p> vs p; p (GeV); Raw E/p", 50, 0, 10}, "pIso", "rawEpIso"));
    profiles.push_back(df_iso.Profile1D({"prof_ep_vs_p_iso_custom", "<E/p> vs p (custom); p (GeV); E/p", nTrkPBins, trkPBins}, "pIso", "epIso"));
    profiles.push_back(df_iso.Profile1D({"prof_raw_ep_vs_p_iso_custom", "<Raw E/p> vs p (custom); p (GeV); Raw E/p", nTrkPBins, trkPBins}, "pIso", "rawEpIso"));

    // 1D-histogrammit pT, eta, E/p, jne
    h1_histos.push_back(df_iso.Histo1D({"h_pt_iso", "p_{T} distribution; p_{T} (GeV); Events", 50, 0, 100}, "ptIso"));
    h1_histos.push_back(df_iso.Histo1D({"h_eta_iso", "#eta distribution; #eta; Events", 50, -1.3, 1.3}, "etaIso"));
    h1_histos.push_back(df_iso.Histo1D({"h_ep_iso", "E/p distribution; E/p; Events", 50, 0, 5}, "epIso"));
    h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_iso", "Raw E/p distribution; E/p; Events", 50, 0, 5}, "rawEpIso"));

    // hadronityyppien mukaiset leikatut histogrammit
    h1_histos.push_back(df_iso.Filter("Sum(isHadH) > 0").Histo1D({"h_ep_isHadH", "E/p: HCAL only; E/p; Events", 50, 0, 2.5}, "epIso"));
    h1_histos.push_back(df_iso.Filter("Sum(isHadE) > 0").Histo1D({"h_ep_isHadE", "E/p: ECAL only; E/p; Events", 50, 0, 2.5}, "epIso"));
    h1_histos.push_back(df_iso.Filter("Sum(isHadMIP) > 0").Histo1D({"h_ep_isHadMIP", "E/p: MIP; E/p; Events", 50, 0, 2.5}, "epIso"));
    h1_histos.push_back(df_iso.Filter("Sum(isHadEH) > 0").Histo1D({"h_ep_isHadEH", "E/p: ECAL+HCAL; E/p; Events", 50, 0, 2.5}, "epIso"));

    // raaka E/p tyypeittäin
    h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadH", "Raw E/p: HCAL only; E/p; Events", 40, 0, 2.5}, "rawEpIso_H"));
    h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadE", "Raw E/p: ECAL only; E/p; Events", 40, 0, 2.5}, "rawEpIso_E"));
    h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadMIP", "Raw E/p: MIP; E/p; Events", 40, 0, 2.5}, "rawEpIso_MIP"));
    h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadEH", "Raw E/p: ECAL+HCAL; E/p; Events", 40, 0, 2.5}, "rawEpIso_EH"));

    // 3D-histogrammit: E/p vs E_HCAL/E_HCAL^raw vs p
    std::vector<std::tuple<std::string, std::string>> hadronTypes = {
        {"isHadH", "HCAL only"},
        {"isHadE", "ECAL only"},
        {"isHadMIP", "MIP"},
        {"isHadEH", "ECAL+HCAL"}
    };

    for (const auto& [mask, label] : hadronTypes) {
        std::string name3d = "h3_resp_corr_p_" + mask;
        std::string title3d = "E/p vs E_{HCAL}/E_{HCAL}^{raw} (" + label + ");E/p;E_{HCAL}/E_{HCAL}^{raw};p (GeV)";
        h3_histos.push_back(df_iso
            .Filter("Sum(" + mask + ") > 0 && Sum(rawHcalIso) > 0")
            .Histo3D({name3d.c_str(), title3d.c_str(),
                     50, 0.0, 2.5,
                     50, 0.0, 2.5,
                     30, 0.0, 100.0},
                     "response", "hcalCorrFactor", "pIso"));
    }

    // kirjoitetaan kaikki histogrammit ROOT-tiedostoon
    std::string filename = "histograms/" + std::string(tag) + ".root";
    TFile out(filename.c_str(), "RECREATE");

    for (auto& h : h1_histos) h->Write();
    for (auto& h : h2_histos) h->Write();
    for (auto& h : h3_histos) h->Write();
    for (auto& p : profiles)  p->Write();
    out.Close();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time: " << std::chrono::duration<double>(end - start).count() << " s\n";
}