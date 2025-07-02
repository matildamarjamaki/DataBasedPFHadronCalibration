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

// määritellään epälineaarinen binitys pionin momentille (p)
// tarkka binitys pienillä p-arvioilla auttaa havaitsemaan responsen muutoksia
const double trkPBins[] = {
    3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
    7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
};
const int nTrkPBins = sizeof(trkPBins) / sizeof(double) - 1;

// pääfunktio, joka tekee histogrammit ja kirjoittaa ne tiedostoon
void run_histograms(const char* listfile, const char* tag) {
    // aloitetaan ajanotto ajon kestolle
    auto start = std::chrono::high_resolution_clock::now();

    // otetaan ROOT-kirjaston monisäikeisyys (threads) käyttöön
    // eli ROOT käsittelee useita useita tapahtumia rinnakkain usealla
    // CPU-ytimellä nopeuttaakseen analyysia
    ROOT::EnableImplicitMT();

    // luetaan ROOT-tiedostojen polut listasta
    // ja muodostetaan kokonaiset XRootD-osoitteet
    std::ifstream infile(listfile);
    std::string line;
    std::vector<std::string> files;
    while (std::getline(infile, line)) {
        if (!line.empty())
            files.push_back("root://hip-cms-se.csc.fi/" + line);
    }

    std::cout << "Processing " << files.size() << " files..." << std::endl;

    // luodaan RDataFrame käyttäen "Events"-puuta useista tiedostoista
    ROOT::RDataFrame df("Events", files);
    ROOT::RDF::Experimental::AddProgressBar(df); // näyttää etenemispalkin

    // valitaan kiinnostavat hiukkaset --> ladatut, isoloidut hadronit (pdgId == +/- 211)
    auto df_iso = df
        .Define("isoMask", [](const ROOT::VecOps::RVec<int>& pdgId,
                               const ROOT::VecOps::RVec<bool>& isoFlag) {
            ROOT::VecOps::RVec<bool> mask(pdgId.size());
            for (size_t i = 0; i < pdgId.size(); ++i) {
                mask[i] = (std::abs(pdgId[i]) == 211) && isoFlag[i];
            }
            return mask;
        }, {"PFCand_pdgId", "PFCand_isIsolatedChargedHadron"})

        // suodatetaan eventit, joissa on ainakin yksi isoloitu ladattu pionikandidaatti
        .Filter("Sum(isoMask) > 0", "Has isolated charged pions")

        // poimitaan vain isoloiduille pT, eta, p ja energiatiedot
        .Define("ptIso", "PFCand_pt[isoMask]")
        .Define("etaIso", "PFCand_eta[isoMask]")
        .Define("pIso", "ptIso * cosh(etaIso)")
        .Define("ecalIso", "PFCand_ecalEnergy[isoMask]")
        .Define("hcalIso", "PFCand_hcalEnergy[isoMask]")
        .Define("rawEcalIso", "PFCand_rawEcalEnergy[isoMask]")
        .Define("rawHcalIso", "PFCand_rawHcalEnergy[isoMask]")

        // johdetut suureet --> detektorielementtien kokonaisenergia, E/p ja fraktio-osuudet
        .Define("detIsoE", "ecalIso + hcalIso")
        .Define("rawDetIsoE", "rawEcalIso + rawHcalIso")
        .Define("epIso", "detIsoE / pIso")
        .Define("rawEpIso", "rawDetIsoE / pIso")
        .Define("fracEcal", "ecalIso / pIso")
        .Define("fracHcal", "hcalIso / pIso")

        // luokitellaan hiukkanen hadronityypin mukaan ECAL/HCAL-energian perusteella
        .Define("isHadH", "ROOT::VecOps::RVec<bool> v; for (size_t i = 0; i < fracEcal.size(); ++i) v.push_back((fracEcal[i] <= 0) && (fracHcal[i] > 0)); return v;")
        .Define("isHadE", "ROOT::VecOps::RVec<bool> v; for (size_t i = 0; i < fracEcal.size(); ++i) v.push_back((fracEcal[i] > 0) && (fracHcal[i] <= 0)); return v;")
        .Define("isHadMIP", "ROOT::VecOps::RVec<bool> v; for (size_t i = 0; i < fracEcal.size(); ++i) v.push_back((fracEcal[i] > 0) && (fracEcal[i] * pIso[i] < 1) && (fracHcal[i] > 0)); return v;")
        .Define("isHadEH", "ROOT::VecOps::RVec<bool> v; for (size_t i = 0; i < fracEcal.size(); ++i) v.push_back((fracEcal[i] > 0) && (fracEcal[i] * pIso[i] >= 1) && (fracHcal[i] > 0)); return v;")

        // määritellään HCAL-korjauskerroin ja valmiiksi maskatut E/p-arvot eri tyypeille
        .Define("hcalCorrFactor", "hcalIso / rawHcalIso")
        .Define("response", "detIsoE / pIso")
        .Define("rawEpIso_H", "ROOT::VecOps::Where(isHadH, rawEpIso, -1.0)")
        .Define("rawEpIso_E", "ROOT::VecOps::Where(isHadE, rawEpIso, -1.0)")
        .Define("rawEpIso_MIP", "ROOT::VecOps::Where(isHadMIP, rawEpIso, -1.0)")
        .Define("rawEpIso_EH", "ROOT::VecOps::Where(isHadEH, rawEpIso, -1.0)");

    auto df_cut = df_iso.Filter("All(hcalCorrFactor > 0.9)", "Cut on E_HCAL/rawE_HCAL > 0.9");

    // leikatut datasetit hadronityypeittäin
    auto df_cut_H   = df_cut.Filter("Sum(isHadH) > 0");
    auto df_cut_E   = df_cut.Filter("Sum(isHadE) > 0");
    auto df_cut_MIP = df_cut.Filter("Sum(isHadMIP) > 0");
    auto df_cut_EH  = df_cut.Filter("Sum(isHadEH) > 0");

    // histogrammivektorit eri dimensioille
    std::vector<ROOT::RDF::RResultPtr<TH1D>> h1_histos;
    std::vector<ROOT::RDF::RResultPtr<TH2D>> h2_histos;
    std::vector<ROOT::RDF::RResultPtr<TProfile>> profiles;
    std::vector<ROOT::RDF::RResultPtr<TH3D>> h3_histos;

    // 2D-histogrammit: E/p vs p ja raw E/p vs p
    h2_histos.push_back(df_iso.Histo2D({"h2_ep_vs_p_iso", "E/p vs p; p (GeV); E/p", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "epIso"));
    h2_histos.push_back(df_iso.Histo2D({"h2_raw_ep_vs_p_iso", "Raw E/p vs p; p (GeV); Raw E/p", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "rawEpIso"));

    // uusi 2D-histo filteröidyllä datalla
    h2_histos.push_back(df_cut.Histo2D({"h2_ep_vs_p_cut", "E/p vs p (cut); p (GeV); E/p", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "epIso"));

    // profiilit: <E/p> vs. p lineaarisella ja epälineaarisella binityksellä
    profiles.push_back(df_iso.Profile1D({"prof_ep_vs_p_iso", "<E/p> vs p; p (GeV); E/p", 50, 0, 10}, "pIso", "epIso"));
    profiles.push_back(df_iso.Profile1D({"prof_raw_ep_vs_p_iso", "<Raw E/p> vs p; p (GeV); Raw E/p", 50, 0, 10}, "pIso", "rawEpIso"));
    profiles.push_back(df_iso.Profile1D({"prof_ep_vs_p_iso_custom", "<E/p> vs p (custom); p (GeV); E/p", nTrkPBins, trkPBins}, "pIso", "epIso"));
    profiles.push_back(df_iso.Profile1D({"prof_raw_ep_vs_p_iso_custom", "<Raw E/p> vs p (custom); p (GeV); Raw E/p", nTrkPBins, trkPBins}, "pIso", "rawEpIso"));

    // uusi profiili filteröidyllä datalla
    profiles.push_back(df_cut.Profile1D({"prof_ep_vs_p_cut", "<E/p> vs p (cut); p (GeV); E/p", nTrkPBins, trkPBins}, "pIso", "epIso"));

    // 1D-histogrammit: pT, eta, E/p jne
    h1_histos.push_back(df_iso.Histo1D({"h_pt_iso", "p_{T} distribution; p_{T} (GeV); Events", 50, 0, 100}, "ptIso"));
    h1_histos.push_back(df_iso.Histo1D({"h_eta_iso", "#eta distribution; #eta; Events", 50, -1.3, 1.3}, "etaIso"));
    h1_histos.push_back(df_iso.Histo1D({"h_ep_iso", "E/p distribution; E/p; Events", 50, 0, 5}, "epIso"));
    h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_iso", "Raw E/p distribution; E/p; Events", 50, 0, 5}, "rawEpIso"));

    // 1D-histogrammit jaoteltuna hadronityypin mukaan
    h1_histos.push_back(df_iso.Filter("Sum(isHadH) > 0").Histo1D({"h_ep_isHadH", "E/p: HCAL only; E/p; Events", 50, 0, 2.5}, "epIso"));
    h1_histos.push_back(df_iso.Filter("Sum(isHadE) > 0").Histo1D({"h_ep_isHadE", "E/p: ECAL only; E/p; Events", 50, 0, 2.5}, "epIso"));
    h1_histos.push_back(df_iso.Filter("Sum(isHadMIP) > 0").Histo1D({"h_ep_isHadMIP", "E/p: MIP; E/p; Events", 50, 0, 2.5}, "epIso"));
    h1_histos.push_back(df_iso.Filter("Sum(isHadEH) > 0").Histo1D({"h_ep_isHadEH", "E/p: ECAL+HCAL; E/p; Events", 50, 0, 2.5}, "epIso"));

    // Low p: 5.5–6.0 GeV
    auto df_cut_H_low = df_cut_H.Filter("All(pIso > 5.5 && pIso < 6.0)");
    auto h1_H_low = df_cut_H_low.Histo1D({"h_ep_cut_H_low", "E/p: HCAL only (low p); E/p; Events", 50, 0, 2.5}, "epIso");

    // Medium p: 10–12 GeV
    auto df_cut_H_medium = df_cut_H.Filter("All(pIso > 10.0 && pIso < 12.0)");
    auto h1_H_medium = df_cut_H_medium.Histo1D({"h_ep_cut_H_medium", "E/p: HCAL only (medium p); E/p; Events", 50, 0, 2.5}, "epIso");

    // High p: 20–22 GeV
    auto df_cut_H_high = df_cut_H.Filter("All(pIso > 20.0 && pIso < 22.0)");
    auto h1_H_high = df_cut_H_high.Histo1D({"h_ep_cut_H_high", "E/p: HCAL only (high p); E/p; Events", 50, 0, 2.5}, "epIso");

    h1_histos.push_back(h1_H_low);
    h1_histos.push_back(h1_H_medium);
    h1_histos.push_back(h1_H_high);

    // raaka E/p -jakaumat eri tyypeille
    h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadH", "Raw E/p: HCAL only; E/p; Events", 40, 0, 2.5}, "rawEpIso_H"));
    h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadE", "Raw E/p: ECAL only; E/p; Events", 40, 0, 2.5}, "rawEpIso_E"));
    h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadMIP", "Raw E/p: MIP; E/p; Events", 40, 0, 2.5}, "rawEpIso_MIP"));
    h1_histos.push_back(df_iso.Histo1D({"h_raw_ep_isHadEH", "Raw E/p: ECAL+HCAL; E/p; Events", 40, 0, 2.5}, "rawEpIso_EH"));

    // 3D-histogrammit: E/p vs E_HCAL/E_HCAL^raw vs. p, tyyppikohtaisesti
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

    // tulostetaan kokonaisajankesto
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time: " << std::chrono::duration<double>(end - start).count() << " s\n";
}