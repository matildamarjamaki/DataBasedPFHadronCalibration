
// #include "ROOT/RDataFrame.hxx"
// #include "TFile.h"
// #include "TH1F.h"
// #include "TH2D.h"
// #include "TProfile.h"
// #include "TF1.h"
// #include <fstream>
// #include <iostream>
// #include <vector>
// #include <string>
// #include <chrono>

// // Määrittele epälineaariset momentum-binit
// double trkPBins[] = {
//     3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
//     7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
//     16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0
// };
// int nTrkPBins = sizeof(trkPBins)/sizeof(double) - 1;

// void run_histograms(const char* listfile = "scripts/input_files.txt") {
//     auto start = std::chrono::high_resolution_clock::now();
//     ROOT::EnableImplicitMT();

//     std::ifstream infile(listfile);
//     std::string line;
//     std::vector<std::string> files;
//     while (std::getline(infile, line)) {
//         if (!line.empty()) {
//             files.push_back("root://hip-cms-se.csc.fi/" + line);
//         }
//     }

//     std::cout << "Processing " << files.size() << " files..." << std::endl;
//     ROOT::RDataFrame df("Events", files);

//     // Kaikki ladatut pionit (korjattu vektorimaskilla)
//     auto df_all = df
//         .Filter("Sum(abs(PFCand_pdgId) == 211) > 0", "Has charged pions")
//         .Define("pionMask", "abs(PFCand_pdgId) == 211")
//         .Define("pt", "PFCand_pt[pionMask]")
//         .Define("eta", "PFCand_eta[pionMask]")
//         .Define("ecal", "PFCand_ecalEnergy[pionMask]")
//         .Define("hcal", "PFCand_hcalEnergy[pionMask]")
//         .Define("rawEcal", "PFCand_rawEcalEnergy[pionMask]")
//         .Define("rawHcal", "PFCand_rawHcalEnergy[pionMask]")
//         .Define("p", "pt * cosh(eta)")
//         .Define("detE", "ecal + hcal")
//         .Define("rawDetE", "rawEcal + rawHcal")
//         .Define("ep", "detE / p")
//         .Define("rawEp", "rawDetE / p");

//     // Isoloidut ladatut pionit (korjattu vektorimaskilla)
//     auto df_iso = df
//         .Filter("Sum(abs(PFCand_pdgId) == 211 && PFCand_isIsolatedChargedHadron) > 0", "Has isolated charged pions")
//         .Define("isoMask", "abs(PFCand_pdgId) == 211 && PFCand_isIsolatedChargedHadron")
//         .Define("ptIso", "PFCand_pt[isoMask]")
//         .Define("etaIso", "PFCand_eta[isoMask]")
//         .Define("ecalIso", "PFCand_ecalEnergy[isoMask]")
//         .Define("hcalIso", "PFCand_hcalEnergy[isoMask]")
//         .Define("rawEcalIso", "PFCand_rawEcalEnergy[isoMask]")
//         .Define("rawHcalIso", "PFCand_rawHcalEnergy[isoMask]")
//         .Define("pIso", "ptIso * cosh(etaIso)")
//         .Define("detIsoE", "ecalIso + hcalIso")
//         .Define("rawDetIsoE", "rawEcalIso + rawHcalIso")
//         .Define("epIso", "detIsoE / pIso")
//         .Define("rawEpIso", "rawDetIsoE / pIso");

//     // Histogrammit
//     auto h_pt = df_all.Histo1D({"h_pt", "p_{T} of all charged pions", 50, 0, 100}, "pt");
//     auto h_eta = df_all.Histo1D({"h_eta", "#eta of all charged pions", 50, -1.3, 1.3}, "eta");
//     auto h_ep = df_all.Histo1D({"h_ep", "E/p (all); E/p; Entries", 50, 0, 5}, "ep");
//     auto h_raw_ep = df_all.Histo1D({"h_raw_ep", "Raw E/p (all); Raw E/p; Entries", 50, 0, 5}, "rawEp");
//     auto prof_ep = df_all.Profile1D({"prof_ep_vs_p", "<E/p> vs p", 50, 0, 10}, "p", "ep");
//     auto prof_raw_ep = df_all.Profile1D({"prof_raw_ep_vs_p", "<Raw E/p> vs p", 50, 0, 10}, "p", "rawEp");
//     auto h2_ep = df_all.Histo2D({"h2_ep_vs_p", "E/p vs p (all); p [GeV]; E/p", 10, 0, 10, 40, 0.01, 2.25}, "p", "ep");
//     auto h2_raw_ep = df_all.Histo2D({"h2_raw_ep_vs_p", "Raw E/p vs p (all); p [GeV]; Raw E/p", 10, 0, 10, 40, 0.01, 2.25}, "p", "rawEp");

//     auto h_pt_iso = df_iso.Histo1D({"h_pt_iso", "p_{T} of isolated pions", 50, 0, 100}, "ptIso");
//     auto h_eta_iso = df_iso.Histo1D({"h_eta_iso", "#eta of isolated pions", 50, -1.3, 1.3}, "etaIso");
//     auto h_ep_iso = df_iso.Histo1D({"h_ep_iso", "E/p (isolated); E/p; Entries", 50, 0, 5}, "epIso");
//     auto h_raw_ep_iso = df_iso.Histo1D({"h_raw_ep_iso", "Raw E/p (isolated); Raw E/p; Entries", 50, 0, 5}, "rawEpIso");
//     auto prof_ep_iso = df_iso.Profile1D({"prof_ep_vs_p_iso", "<E/p> vs p (isolated)", 50, 0, 10}, "pIso", "epIso");
//     auto prof_raw_ep_iso = df_iso.Profile1D({"prof_raw_ep_vs_p_iso", "<Raw E/p> vs p (isolated)", 50, 0, 10}, "pIso", "rawEpIso");
//     auto h2_ep_iso = df_iso.Histo2D({"h2_ep_vs_p_iso", "E/p vs p (isolated); p [GeV]; E/p", 10, 0, 10, 40, 0.01, 2.25}, "pIso", "epIso");
//     auto h2_raw_ep_iso = df_iso.Histo2D({"h2_raw_ep_vs_p_iso", "Raw E/p vs p (isolated); p [GeV]; Raw E/p", 10, 0, 10, 40, 0.01, 2.25}, "pIso", "rawEpIso");

//     // Uudet profiilit epälineaarisilla binityksillä
//     auto prof_ep_custom = df_all.Profile1D({"prof_ep_vs_p_custom", "<E/p> vs p (custom bins)", nTrkPBins, trkPBins}, "p", "ep");
//     auto prof_ep_iso_custom = df_iso.Profile1D({"prof_ep_vs_p_iso_custom", "<E/p> vs p (iso, custom bins)", nTrkPBins, trkPBins}, "pIso", "epIso");

//     // Tallennus
//     TFile out_all("histograms/pion_histos_all.root", "RECREATE");
//     h_pt->Write(); h_eta->Write(); h_ep->Write(); h_raw_ep->Write();
//     prof_ep->Write(); prof_raw_ep->Write(); h2_ep->Write(); h2_raw_ep->Write();
//     TF1 *fit = new TF1("fit", "pol1", 2.5, 9.0);
//     prof_ep->Fit(fit, "RQ");
//     fit->Write();
//     out_all.Close();

//     TFile out_iso("histograms/pion_histos_iso.root", "RECREATE");
//     h_pt_iso->Write(); h_eta_iso->Write(); h_ep_iso->Write(); h_raw_ep_iso->Write();
//     prof_ep_iso->Write(); prof_raw_ep_iso->Write(); h2_ep_iso->Write(); h2_raw_ep_iso->Write();
//     out_iso.Close();

//     auto end = std::chrono::high_resolution_clock::now();
//     std::cout << "Elapsed time: "
//               << std::chrono::duration<double>(end - start).count()
//               << " s" << std::endl;
// }


// #include "ROOT/RDataFrame.hxx"
// #include "TFile.h"
// #include "TH1F.h"
// #include "TH2D.h"
// #include "TProfile.h"
// #include "TF1.h"
// #include <fstream>
// #include <iostream>
// #include <vector>
// #include <string>
// #include <chrono>

// // Määrittele epälineaariset momentum-binit
// double trkPBins[] = {
//     3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
//     7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
//     16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0,45.,55.,70.,130.
// };

// int nTrkPBins = sizeof(trkPBins) / sizeof(double) - 1;

// // void run_histograms(const char* listfile = "scripts/input_files.txt") {
// void run_histograms(const char* listfile) {
//     auto start = std::chrono::high_resolution_clock::now();
//     ROOT::EnableImplicitMT();

//     std::ifstream infile(listfile);
//     std::string line;
//     std::vector<std::string> files;
//     while (std::getline(infile, line)) {
//         if (!line.empty()) {
//             files.push_back("root://hip-cms-se.csc.fi/" + line);
//         }
//     }

//     std::cout << "Processing " << files.size() << " files..." << std::endl;
//     ROOT::RDataFrame df("Events", files);

//     // Kaikki ladatut pionit
//     auto df_all = df
//         .Filter("Sum(abs(PFCand_pdgId) == 211) > 0", "Has charged pions")
//         .Define("pionMask", "abs(PFCand_pdgId) == 211")
//         .Define("pt", "PFCand_pt[pionMask]")
//         .Define("eta", "PFCand_eta[pionMask]")
//         .Define("ecal", "PFCand_ecalEnergy[pionMask]")
//         .Define("hcal", "PFCand_hcalEnergy[pionMask]")
//         .Define("rawEcal", "PFCand_rawEcalEnergy[pionMask]")
//         .Define("rawHcal", "PFCand_rawHcalEnergy[pionMask]")
//         .Define("p", "pt * cosh(eta)")
//         .Define("detE", "ecal + hcal")
//         .Define("rawDetE", "rawEcal + rawHcal")
//         .Define("ep", "detE / p")
//         .Define("rawEp", "rawDetE / p");

//     // Isoloidut ladatut pionit
//     auto df_iso = df
//         .Filter("Sum(abs(PFCand_pdgId) == 211 && PFCand_isIsolatedChargedHadron) > 0", "Has isolated charged pions")
//         .Define("isoMask", "abs(PFCand_pdgId) == 211 && PFCand_isIsolatedChargedHadron")
//         .Define("ptIso", "PFCand_pt[isoMask]")
//         .Define("etaIso", "PFCand_eta[isoMask]")
//         .Define("ecalIso", "PFCand_ecalEnergy[isoMask]")
//         .Define("hcalIso", "PFCand_hcalEnergy[isoMask]")
//         .Define("rawEcalIso", "PFCand_rawEcalEnergy[isoMask]")
//         .Define("rawHcalIso", "PFCand_rawHcalEnergy[isoMask]")
//         .Define("pIso", "ptIso * cosh(etaIso)")
//         .Define("detIsoE", "ecalIso + hcalIso")
//         .Define("rawDetIsoE", "rawEcalIso + rawHcalIso")
//         .Define("epIso", "detIsoE / pIso")
//         .Define("rawEpIso", "rawDetIsoE / pIso");

//     // Histogrammit ja profiilit
//     auto h_pt              = df_all.Histo1D({"h_pt", "p_{T} of all charged pions", 50, 0, 100}, "pt");
//     auto h_eta             = df_all.Histo1D({"h_eta", "#eta of all charged pions", 50, -1.3, 1.3}, "eta");
//     auto h_ep              = df_all.Histo1D({"h_ep", "E/p (all); E/p; Entries", 50, 0, 5}, "ep");
//     auto h_raw_ep          = df_all.Histo1D({"h_raw_ep", "Raw E/p (all); Raw E/p; Entries", 50, 0, 5}, "rawEp");
//     auto prof_ep           = df_all.Profile1D({"prof_ep_vs_p", "<E/p> vs p", 50, 0, 10}, "p", "ep");
//     auto prof_raw_ep       = df_all.Profile1D({"prof_raw_ep_vs_p", "<Raw E/p> vs p", 50, 0, 10}, "p", "rawEp");
//     auto h2_ep             = df_all.Histo2D({"h2_ep_vs_p", "E/p vs p (all)", 10, 0, 10, 40, 0.01, 2.25}, "p", "ep");
//     auto h2_raw_ep         = df_all.Histo2D({"h2_raw_ep_vs_p", "Raw E/p vs p (all)", 10, 0, 10, 40, 0.01, 2.25}, "p", "rawEp");

//     auto h_pt_iso          = df_iso.Histo1D({"h_pt_iso", "p_{T} of isolated pions", 50, 0, 100}, "ptIso");
//     auto h_eta_iso         = df_iso.Histo1D({"h_eta_iso", "#eta of isolated pions", 50, -1.3, 1.3}, "etaIso");
//     auto h_ep_iso          = df_iso.Histo1D({"h_ep_iso", "E/p (isolated)", 50, 0, 5}, "epIso");
//     auto h_raw_ep_iso      = df_iso.Histo1D({"h_raw_ep_iso", "Raw E/p (isolated)", 50, 0, 5}, "rawEpIso");
//     auto prof_ep_iso       = df_iso.Profile1D({"prof_ep_vs_p_iso", "<E/p> vs p (isolated)", 50, 0, 10}, "pIso", "epIso");
//     auto prof_raw_ep_iso   = df_iso.Profile1D({"prof_raw_ep_vs_p_iso", "<Raw E/p> vs p (isolated)", 50, 0, 10}, "pIso", "rawEpIso");
//     // auto h2_ep_iso         = df_iso.Histo2D({"h2_ep_vs_p_iso", "E/p vs p (isolated)", 5, 0, 10, 10, 0.05, 2.25}, "pIso", "epIso");
//     auto h2_ep_iso = df_iso.Histo2D({"h2_ep_vs_p_iso", "E/p vs p (isolated)", nTrkPBins, trkPBins, 40, 0.05, 2.25},"pIso", "epIso");
//     // auto h2_raw_ep_iso     = df_iso.Histo2D({"h2_raw_ep_vs_p_iso", "Raw E/p vs p (isolated)", 10, 0, 10, 40, 0.05, 2.25}, "pIso", "rawEpIso");
//     auto h2_raw_ep_iso = df_iso.Histo2D({"h2_raw_ep_vs_p_iso", "E/p vs p (isolated)", nTrkPBins, trkPBins, 40, 0.05, 2.25},"pIso", "rawEpIso");

//     // Custom-binitetyt profiilit
//     auto prof_ep_custom         = df_all.Profile1D({"prof_ep_vs_p_custom", "<E/p> vs p (custom)", nTrkPBins, trkPBins}, "p", "ep");
//     auto prof_raw_ep_custom     = df_all.Profile1D({"prof_raw_ep_vs_p_custom", "<Raw E/p> vs p (custom)", nTrkPBins, trkPBins}, "p", "rawEp");
//     auto prof_ep_iso_custom     = df_iso.Profile1D({"prof_ep_vs_p_iso_custom", "<E/p> vs p (iso, custom)", nTrkPBins, trkPBins}, "pIso", "epIso");
//     auto prof_raw_ep_iso_custom = df_iso.Profile1D({"prof_raw_ep_vs_p_iso_custom", "<Raw E/p> vs p (iso, custom)", nTrkPBins, trkPBins}, "pIso", "rawEpIso");

//     // Tallennus
//     TFile out_all("histograms/pion_histos_all.root", "RECREATE");
//     h_pt->Write(); h_eta->Write(); h_ep->Write(); h_raw_ep->Write();
//     prof_ep->Write(); prof_raw_ep->Write(); h2_ep->Write(); h2_raw_ep->Write();
//     prof_ep_custom->Write(); prof_raw_ep_custom->Write();
//     TF1 *fit = new TF1("fit", "pol1", 2.5, 9.0);
//     prof_ep->Fit(fit, "RQ");
//     fit->Write();
//     out_all.Close();

//     TFile out_iso("histograms/pion_histos_iso.root", "RECREATE");
//     h_pt_iso->Write(); h_eta_iso->Write(); h_ep_iso->Write(); h_raw_ep_iso->Write();
//     prof_ep_iso->Write(); prof_raw_ep_iso->Write(); h2_ep_iso->Write(); h2_raw_ep_iso->Write();
//     prof_ep_iso_custom->Write(); prof_raw_ep_iso_custom->Write();
//     out_iso.Close();

//     auto end = std::chrono::high_resolution_clock::now();
//     std::cout << "Elapsed time: "
//               << std::chrono::duration<double>(end - start).count()
//               << " s" << std::endl;
// }




// #include "ROOT/RDataFrame.hxx"
// #include "TFile.h"
// #include "TH1F.h"
// #include "TH2D.h"
// #include "TProfile.h"
// #include <fstream>
// #include <iostream>
// #include <vector>
// #include <string>
// #include <chrono>

// // Määrittele epälineaariset momentum-binit
// double trkPBins[] = {
//     3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
//     7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
//     16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
// };

// int nTrkPBins = sizeof(trkPBins) / sizeof(double) - 1;

// void run_histograms(const char* listfile, const char* tag) {
//     auto start = std::chrono::high_resolution_clock::now();
//     ROOT::EnableImplicitMT();

//     std::ifstream infile(listfile);
//     std::string line;
//     std::vector<std::string> files;
//     while (std::getline(infile, line)) {
//         if (!line.empty()) {
//             files.push_back("root://hip-cms-se.csc.fi/" + line);
//         }
//     }

//     std::cout << "Processing " << files.size() << " files..." << std::endl;
//     ROOT::RDataFrame df("Events", files);

//     // Isoloidut ladatut pionit
//     auto df_iso = df
//         .Filter("Sum(abs(PFCand_pdgId) == 211 && PFCand_isIsolatedChargedHadron) > 0", "Has isolated charged pions")
//         .Define("isoMask", "abs(PFCand_pdgId) == 211 && PFCand_isIsolatedChargedHadron")
//         .Define("ptIso", "PFCand_pt[isoMask]")
//         .Define("etaIso", "PFCand_eta[isoMask]")
//         .Define("ecalIso", "PFCand_ecalEnergy[isoMask]")
//         .Define("hcalIso", "PFCand_hcalEnergy[isoMask]")
//         .Define("rawEcalIso", "PFCand_rawEcalEnergy[isoMask]")
//         .Define("rawHcalIso", "PFCand_rawHcalEnergy[isoMask]")
//         .Define("pIso", "ptIso * cosh(etaIso)")
//         .Define("detIsoE", "ecalIso + hcalIso")
//         .Define("rawDetIsoE", "rawEcalIso + rawHcalIso")
//         .Define("epIso", "detIsoE / pIso")
//         .Define("rawEpIso", "rawDetIsoE / pIso");

//     // Histogrammit ja profiilit
//     auto h_pt_iso          = df_iso.Histo1D({"h_pt_iso", "p_{T} of isolated pions", 50, 0, 100}, "ptIso");
//     auto h_eta_iso         = df_iso.Histo1D({"h_eta_iso", "#eta of isolated pions", 50, -1.3, 1.3}, "etaIso");
//     auto h_ep_iso          = df_iso.Histo1D({"h_ep_iso", "E/p (isolated)", 50, 0, 5}, "epIso");
//     auto h_raw_ep_iso      = df_iso.Histo1D({"h_raw_ep_iso", "Raw E/p (isolated)", 50, 0, 5}, "rawEpIso");
//     auto prof_ep_iso       = df_iso.Profile1D({"prof_ep_vs_p_iso", "<E/p> vs p (isolated)", 50, 0, 10}, "pIso", "epIso");
//     auto prof_raw_ep_iso   = df_iso.Profile1D({"prof_raw_ep_vs_p_iso", "<Raw E/p> vs p (isolated)", 50, 0, 10}, "pIso", "rawEpIso");
//     auto h2_ep_iso         = df_iso.Histo2D({"h2_ep_vs_p_iso", "E/p vs p (isolated)", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "epIso");
//     auto h2_raw_ep_iso     = df_iso.Histo2D({"h2_raw_ep_vs_p_iso", "Raw E/p vs p (isolated)", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "rawEpIso");
//     auto prof_ep_iso_custom     = df_iso.Profile1D({"prof_ep_vs_p_iso_custom", "<E/p> vs p (iso, custom)", nTrkPBins, trkPBins}, "pIso", "epIso");
//     auto prof_raw_ep_iso_custom = df_iso.Profile1D({"prof_raw_ep_vs_p_iso_custom", "<Raw E/p> vs p (iso, custom)", nTrkPBins, trkPBins}, "pIso", "rawEpIso");

//     // Tallennus
//     std::string iso_filename = "histograms/pion_histos_iso_" + std::string(tag) + ".root";
//     TFile out_iso(iso_filename.c_str(), "RECREATE");
//     h_pt_iso->Write(); h_eta_iso->Write(); h_ep_iso->Write(); h_raw_ep_iso->Write();
//     prof_ep_iso->Write(); prof_raw_ep_iso->Write(); h2_ep_iso->Write(); h2_raw_ep_iso->Write();
//     prof_ep_iso_custom->Write(); prof_raw_ep_iso_custom->Write();
//     out_iso.Close();

//     auto end = std::chrono::high_resolution_clock::now();
//     std::cout << "Elapsed time: "
//               << std::chrono::duration<double>(end - start).count()
//               << " s" << std::endl;
// }











// #include "ROOT/RDataFrame.hxx"
// #include "TFile.h"
// #include "TH1F.h"
// #include "TH2D.h"
// #include "TProfile.h"
// #include <fstream>
// #include <iostream>
// #include <vector>
// #include <string>
// #include <chrono>

// // Määrittele epälineaariset momentum-binit
// double trkPBins[] = {
//     3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
//     7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
//     16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
// };

// int nTrkPBins = sizeof(trkPBins) / sizeof(double) - 1;

// void run_histograms(const char* listfile, const char* tag) {
//     auto start = std::chrono::high_resolution_clock::now();
//     ROOT::EnableImplicitMT();

//     std::ifstream infile(listfile);
//     std::string line;
//     std::vector<std::string> files;
//     while (std::getline(infile, line)) {
//         if (!line.empty()) {
//             files.push_back("root://hip-cms-se.csc.fi/" + line);
//         }
//     }

//     std::cout << "Processing " << files.size() << " files..." << std::endl;
//     ROOT::RDataFrame df("Events", files);


// // Isoloidut varatut pionit
// auto df_iso = df
//     .Define("isoMaskBool", [](const ROOT::VecOps::RVec<int>& pdgId,
//                                const ROOT::VecOps::RVec<bool>& isoFlag) {
//         ROOT::VecOps::RVec<bool> mask(pdgId.size());
//         for (size_t i = 0; i < pdgId.size(); ++i) {
//             mask[i] = (std::abs(pdgId[i]) == 211) && isoFlag[i];
//         }
//         return mask;
//     }, {"PFCand_pdgId", "PFCand_isIsolatedChargedHadron"})
//     .Filter([](const ROOT::VecOps::RVec<bool>& mask) { return Sum(mask) > 0; }, {"isoMaskBool"}, "Has isolated charged pions")
//     .Define("ptIso", "PFCand_pt[isoMaskBool]")
//     .Define("etaIso", "PFCand_eta[isoMaskBool]")
//     .Define("ecalIso", "PFCand_ecalEnergy[isoMaskBool]")
//     .Define("hcalIso", "PFCand_hcalEnergy[isoMaskBool]")
//     .Define("rawEcalIso", "PFCand_rawEcalEnergy[isoMaskBool]")
//     .Define("rawHcalIso", "PFCand_rawHcalEnergy[isoMaskBool]")
//     .Define("pIso", "ptIso * cosh(etaIso)")
//     .Define("detIsoE", "ecalIso + hcalIso")
//     .Define("rawDetIsoE", "rawEcalIso + rawHcalIso")
//     .Define("epIso", "detIsoE / pIso")
//     .Define("rawEpIso", "rawDetIsoE / pIso")
//     .Define("fracEcal", "ecalIso / pIso")
//     .Define("fracHcal", "hcalIso / pIso")
//     .Define("isHadH", [](const ROOT::VecOps::RVec<float>& fracEcal,
//                          const ROOT::VecOps::RVec<float>& fracHcal) {
//         ROOT::VecOps::RVec<bool> result(fracEcal.size());
//         for (size_t i = 0; i < fracEcal.size(); ++i)
//             result[i] = (fracEcal[i] <= 0) && (fracHcal[i] > 0);
//         return result;
//     }, {"fracEcal", "fracHcal"})
//     .Define("isHadE", [](const ROOT::VecOps::RVec<float>& fracEcal,
//                          const ROOT::VecOps::RVec<float>& fracHcal) {
//         ROOT::VecOps::RVec<bool> result(fracEcal.size());
//         for (size_t i = 0; i < fracEcal.size(); ++i)
//             result[i] = (fracEcal[i] > 0) && (fracHcal[i] <= 0);
//         return result;
//     }, {"fracEcal", "fracHcal"})
//     .Define("isHadMIP", [](const ROOT::VecOps::RVec<float>& fracEcal,
//                            const ROOT::VecOps::RVec<float>& fracHcal,
//                            const ROOT::VecOps::RVec<float>& pIso) {
//         ROOT::VecOps::RVec<bool> result(fracEcal.size());
//         for (size_t i = 0; i < fracEcal.size(); ++i)
//             result[i] = (fracEcal[i] > 0) && (fracEcal[i] * pIso[i] < 1) && (fracHcal[i] > 0);
//         return result;
//     }, {"fracEcal", "fracHcal", "pIso"})
//     .Define("isHadEH", [](const ROOT::VecOps::RVec<float>& fracEcal,
//                           const ROOT::VecOps::RVec<float>& fracHcal,
//                           const ROOT::VecOps::RVec<float>& pIso) {
//         ROOT::VecOps::RVec<bool> result(fracEcal.size());
//         for (size_t i = 0; i < fracEcal.size(); ++i)
//             result[i] = (fracEcal[i] > 0) && (fracEcal[i] * pIso[i] >= 1) && (fracHcal[i] > 0);
//         return result;
//     }, {"fracEcal", "fracHcal", "pIso"});

// // Histogrammit ja profiilit
// auto h_pt_iso  = df_iso.Histo1D({"h_pt_iso", "p_{T} of isolated pions", 50, 0, 100}, "ptIso");
// auto h_eta_iso = df_iso.Histo1D({"h_eta_iso", "#eta of isolated pions", 50, -1.3, 1.3}, "etaIso");
// auto h_ep_iso  = df_iso.Histo1D({"h_ep_iso", "E/p (isolated)", 50, 0, 5}, "epIso");
// auto h_raw_ep_iso = df_iso.Histo1D({"h_raw_ep_iso", "Raw E/p (isolated)", 50, 0, 5}, "rawEpIso");

// auto prof_ep_iso     = df_iso.Profile1D({"prof_ep_vs_p_iso", "<E/p> vs p (isolated)", 50, 0, 10}, "pIso", "epIso");
// auto prof_raw_ep_iso = df_iso.Profile1D({"prof_raw_ep_vs_p_iso", "<Raw E/p> vs p (isolated)", 50, 0, 10}, "pIso", "rawEpIso");

// auto h2_ep_iso = df_iso.Histo2D({"h2_ep_vs_p_iso", "E/p vs p (isolated)", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "epIso");
// auto h2_raw_ep_iso = df_iso.Histo2D({"h2_raw_ep_vs_p_iso", "Raw E/p vs p (isolated)", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "rawEpIso");

// auto prof_ep_iso_custom = df_iso.Profile1D({"prof_ep_vs_p_iso_custom", "<E/p> vs p (iso, custom)", nTrkPBins, trkPBins}, "pIso", "epIso");
// auto prof_raw_ep_iso_custom = df_iso.Profile1D({"prof_raw_ep_vs_p_iso_custom", "<Raw E/p> vs p (iso, custom)", nTrkPBins, trkPBins}, "pIso", "rawEpIso");

// // isHad*-maskien histogrammit
// auto h_ep_isHadH = df_iso.Filter([](const ROOT::VecOps::RVec<bool>& mask) { return Sum(mask) > 0; }, {"isHadH"})
//     .Histo1D({"h_ep_isHadH", "E/p: HCAL-only", 50, 0, 2.5}, "epIso");

// auto h_ep_isHadE = df_iso.Filter([](const ROOT::VecOps::RVec<bool>& mask) { return Sum(mask) > 0; }, {"isHadE"})
//     .Histo1D({"h_ep_isHadE", "E/p: ECAL-only", 50, 0, 2.5}, "epIso");

// auto h_ep_isHadMIP = df_iso.Filter([](const ROOT::VecOps::RVec<bool>& mask) { return Sum(mask) > 0; }, {"isHadMIP"})
//     .Histo1D({"h_ep_isHadMIP", "E/p: MIP", 50, 0, 2.5}, "epIso");

// auto h_ep_isHadEH = df_iso.Filter([](const ROOT::VecOps::RVec<bool>& mask) { return Sum(mask) > 0; }, {"isHadEH"})
//     .Histo1D({"h_ep_isHadEH", "E/p: ECAL+HCAL", 50, 0, 2.5}, "epIso");

// // Tiedoston nimi ja tallennus
// std::string iso_filename = "histograms/pion_histos_iso_" + std::string(tag) + ".root";
// TFile out_iso(iso_filename.c_str(), "RECREATE");

// h_pt_iso->Write(); h_eta_iso->Write(); h_ep_iso->Write(); h_raw_ep_iso->Write();
// prof_ep_iso->Write(); prof_raw_ep_iso->Write();
// h2_ep_iso->Write(); h2_raw_ep_iso->Write();
// prof_ep_iso_custom->Write(); prof_raw_ep_iso_custom->Write();
// h_ep_isHadH->Write(); h_ep_isHadE->Write(); h_ep_isHadMIP->Write(); h_ep_isHadEH->Write();

// out_iso.Close();

// auto end = std::chrono::high_resolution_clock::now();
// std::cout << "Elapsed time: " << std::chrono::duration<double>(end - start).count() << " s" << std::endl;

// }












// #include "ROOT/RDataFrame.hxx"
// #include "TFile.h"
// #include "TH1F.h"
// #include "TH2D.h"
// #include "TProfile.h"
// #include "TF1.h"
// #include "TCanvas.h"
// #include "TLatex.h"
// #include <fstream>
// #include <iostream>
// #include <vector>
// #include <string>
// #include <sstream>
// #include <chrono>

// // Momentum-binien määrittely
// const double trkPBins[] = {
//     3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
//     7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
//     16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
// };
// const int nTrkPBins = sizeof(trkPBins) / sizeof(double) - 1;

// void run_histograms(const char* listfile, const char* tag) {
//     auto start = std::chrono::high_resolution_clock::now();
//     ROOT::EnableImplicitMT();

//     std::ifstream infile(listfile);
//     std::string line;
//     std::vector<std::string> files;
//     while (std::getline(infile, line)) {
//         if (!line.empty())
//             files.push_back("root://hip-cms-se.csc.fi/" + line);
//     }

//     std::cout << "Processing " << files.size() << " files..." << std::endl;
//     ROOT::RDataFrame df("Events", files);

//     auto df_iso = df
//         .Define("isoMask", [](const ROOT::VecOps::RVec<int>& pdgId,
//                             const ROOT::VecOps::RVec<bool>& isoFlag) {
//             ROOT::VecOps::RVec<bool> mask(pdgId.size());
//             for (size_t i = 0; i < pdgId.size(); ++i) {
//                 mask[i] = (std::abs(pdgId[i]) == 211) && isoFlag[i];
//             }
//             return mask;
//         }, {"PFCand_pdgId", "PFCand_isIsolatedChargedHadron"})

//         .Filter("Sum(isoMask) > 0", "Has isolated charged pions")
//         .Define("ptIso", "PFCand_pt[isoMask]")
//         .Define("etaIso", "PFCand_eta[isoMask]")
//         .Define("pIso", "ptIso * cosh(etaIso)")
//         .Define("ecalIso", "PFCand_ecalEnergy[isoMask]")
//         .Define("hcalIso", "PFCand_hcalEnergy[isoMask]")
//         .Define("rawEcalIso", "PFCand_rawEcalEnergy[isoMask]")
//         .Define("rawHcalIso", "PFCand_rawHcalEnergy[isoMask]")
//         .Define("detIsoE", "ecalIso + hcalIso")
//         .Define("rawDetIsoE", "rawEcalIso + rawHcalIso")
//         .Define("epIso", "detIsoE / pIso")
//         .Define("rawEpIso", "rawDetIsoE / pIso")
//         .Define("fracEcal", "ecalIso / pIso")
//         .Define("fracHcal", "hcalIso / pIso")
//         .Define("isHadH", "(fracEcal <= 0) && (fracHcal > 0)")
//         .Define("isHadE", "(fracEcal > 0) && (fracHcal <= 0)")
//         .Define("isHadMIP", "(fracEcal > 0) && (fracEcal * pIso < 1) && (fracHcal > 0)")
//         .Define("isHadEH", "(fracEcal > 0) && (fracEcal * pIso >= 1) && (fracHcal > 0)");

//     // Asetetaan root filen sijoituspaikka ja nimi
//     std::string filename = "histograms/" + std::string(tag) + ".root";
//     TFile out(filename.c_str(), "RECREATE");

//     auto h2_ep = df_iso.Histo2D({"h2_ep_vs_p_iso", "E/p vs p", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "epIso");
//     auto h2_raw = df_iso.Histo2D({"h2_raw_ep_vs_p_iso", "Raw E/p vs p", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "rawEpIso");

//     auto prof_ep = df_iso.Profile1D({"prof_ep_vs_p_iso", "<E/p> vs p", 50, 0, 10}, "pIso", "epIso");
//     auto prof_raw_ep = df_iso.Profile1D({"prof_raw_ep_vs_p_iso", "<Raw E/p> vs p", 50, 0, 10}, "pIso", "rawEpIso");
//     auto prof_ep_custom = df_iso.Profile1D({"prof_ep_vs_p_iso_custom", "<E/p> vs p (custom)", nTrkPBins, trkPBins}, "pIso", "epIso");
//     auto prof_raw_ep_custom = df_iso.Profile1D({"prof_raw_ep_vs_p_iso_custom", "<Raw E/p> vs p (custom)", nTrkPBins, trkPBins}, "pIso", "rawEpIso");

//     auto h_pt = df_iso.Histo1D({"h_pt_iso", "pT", 50, 0, 100}, "ptIso");
//     auto h_eta = df_iso.Histo1D({"h_eta_iso", "eta", 50, -1.3, 1.3}, "etaIso");
//     auto h_ep = df_iso.Histo1D({"h_ep_iso", "E/p", 50, 0, 5}, "epIso");
//     auto h_raw_ep = df_iso.Histo1D({"h_raw_ep_iso", "Raw E/p", 50, 0, 5}, "rawEpIso");

//     auto h_ep_H   = df_iso.Filter("Sum(isHadH) > 0").Histo1D({"h_ep_isHadH", "E/p: HCAL only", 50, 0, 2.5}, "epIso");
//     auto h_ep_E   = df_iso.Filter("Sum(isHadE) > 0").Histo1D({"h_ep_isHadE", "E/p: ECAL only", 50, 0, 2.5}, "epIso");
//     auto h_ep_MIP = df_iso.Filter("Sum(isHadMIP) > 0").Histo1D({"h_ep_isHadMIP", "E/p: MIP", 50, 0, 2.5}, "epIso");
//     auto h_ep_EH  = df_iso.Filter("Sum(isHadEH) > 0").Histo1D({"h_ep_isHadEH", "E/p: ECAL+HCAL", 50, 0, 2.5}, "epIso");

//     auto h_raw_H   = df_iso.Filter("Sum(isHadH) > 0").Histo1D({"h_raw_ep_isHadH", "Raw E/p: HCAL only", 50, 0, 2.5}, "rawEpIso");
//     auto h_raw_E   = df_iso.Filter("Sum(isHadE) > 0").Histo1D({"h_raw_ep_isHadE", "Raw E/p: ECAL only", 50, 0, 2.5}, "rawEpIso");
//     auto h_raw_MIP = df_iso.Filter("Sum(isHadMIP) > 0").Histo1D({"h_raw_ep_isHadMIP", "Raw E/p: MIP", 50, 0, 2.5}, "rawEpIso");
//     auto h_raw_EH  = df_iso.Filter("Sum(isHadEH) > 0").Histo1D({"h_raw_ep_isHadEH", "Raw E/p: ECAL+HCAL", 50, 0, 2.5}, "rawEpIso");

    

//     std::vector<std::pair<double, double>> pBins = {{5.5, 6.0}, {20.0, 22.0}};
//     for (const auto& [pmin, pmax] : pBins) {
//         int bin_min = h2_ep->GetXaxis()->FindBin(pmin + 1e-3);
//         int bin_max = h2_ep->GetXaxis()->FindBin(pmax - 1e-3);

//         std::stringstream sn, sr;
//         sn << "proj_ep_" << int(pmin * 10) << "_" << int(pmax * 10);
//         sr << "proj_raw_ep_" << int(pmin * 10) << "_" << int(pmax * 10);

//         auto proj_n = h2_ep->ProjectionY(sn.str().c_str(), bin_min, bin_max);
//         auto proj_r = h2_raw->ProjectionY(sr.str().c_str(), bin_min, bin_max);

//         proj_n->Write(); proj_r->Write();

//         if (proj_n->Integral() > 0) {
//             proj_n->Scale(1.0 / proj_n->Integral());
//             std::string fitn_name = sn.str() + "_fit";
//             TF1* fit_n = new TF1(fitn_name.c_str(), "gaus", 0.0, 2.5);
//             proj_n->Fit(fit_n, "RQ", "", 0.7, 1.2);
//             fit_n->Write();

//             TCanvas* c_n = new TCanvas(("canvas_" + sn.str()).c_str(), "Gaussian fit (E/p)", 800, 600);
//             proj_n->Draw();
//             fit_n->SetLineColor(kRed);
//             fit_n->Draw("same");
//             c_n->Write();
//         }

//         if (proj_r->Integral() > 0) {
//             proj_r->Scale(1.0 / proj_r->Integral());
//             std::string fitr_name = sr.str() + "_fit";
//             TF1* fit_r = new TF1(fitr_name.c_str(), "gaus", 0.0, 2.5);
//             proj_r->Fit(fit_r, "RQ", "", 0.7, 1.2);
//             fit_r->Write();

//             TCanvas* c_r = new TCanvas(("canvas_" + sr.str()).c_str(), "Gaussian fit (Raw E/p)", 800, 600);
//             proj_r->Draw();
//             fit_r->SetLineColor(kRed);
//             fit_r->Draw("same");
//             c_r->Write();
//         }
//     }


//     // Kirjoitetaan plottaukset
//     h2_ep->Write(); h2_raw->Write();
//     prof_ep->Write(); prof_raw_ep->Write();
//     prof_ep_custom->Write(); prof_raw_ep_custom->Write();
//     h_pt->Write(); h_eta->Write(); h_ep->Write(); h_raw_ep->Write();
//     h_ep_H->Write(); h_ep_E->Write(); h_ep_MIP->Write(); h_ep_EH->Write();
//     h_raw_H->Write(); h_raw_E->Write(); h_raw_MIP->Write(); h_raw_EH->Write();

//     out.Close();

//     // Lasketaan vielä datan läpikäymiseen kulunut aika
//     auto end = std::chrono::high_resolution_clock::now();
//     std::cout << "Elapsed time: " << std::chrono::duration<double>(end - start).count() << " s\n";
// }









// #include "ROOT/RDataFrame.hxx"
// #include "TFile.h"
// #include "TH1F.h"
// #include "TH2D.h"
// #include "TProfile.h"
// #include "TF1.h"
// #include "TCanvas.h"
// #include "TLatex.h"
// #include <fstream>
// #include <iostream>
// #include <vector>
// #include <string>
// #include <sstream>
// #include <chrono>

// // Momentum-binien määrittely
// const double trkPBins[] = {
//     3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
//     7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
//     16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
// };
// const int nTrkPBins = sizeof(trkPBins) / sizeof(double) - 1;

// void run_histograms(const char* listfile, const char* tag) {
//     auto start = std::chrono::high_resolution_clock::now();
//     ROOT::EnableImplicitMT();

//     std::ifstream infile(listfile);
//     std::string line;
//     std::vector<std::string> files;
//     while (std::getline(infile, line)) {
//         if (!line.empty())
//             files.push_back("root://hip-cms-se.csc.fi/" + line);
//     }

//     std::cout << "Processing " << files.size() << " files..." << std::endl;
//     ROOT::RDataFrame df("Events", files);

//     auto df_iso = df
//         .Define("isoMask", [](const ROOT::VecOps::RVec<int>& pdgId,
//                              const ROOT::VecOps::RVec<bool>& isoFlag) {
//             ROOT::VecOps::RVec<bool> mask(pdgId.size());
//             for (size_t i = 0; i < pdgId.size(); ++i) {
//                 mask[i] = (std::abs(pdgId[i]) == 211) && isoFlag[i];
//             }
//             return mask;
//         }, {"PFCand_pdgId", "PFCand_isIsolatedChargedHadron"})

//         .Filter("Sum(isoMask) > 0", "Has isolated charged pions")
//         .Define("ptIso", "PFCand_pt[isoMask]")
//         .Define("etaIso", "PFCand_eta[isoMask]")
//         .Define("pIso", "ptIso * cosh(etaIso)")
//         .Define("ecalIso", "PFCand_ecalEnergy[isoMask]")
//         .Define("hcalIso", "PFCand_hcalEnergy[isoMask]")
//         .Define("rawEcalIso", "PFCand_rawEcalEnergy[isoMask]")
//         .Define("rawHcalIso", "PFCand_rawHcalEnergy[isoMask]")
//         .Define("detIsoE", "ecalIso + hcalIso")
//         .Define("rawDetIsoE", "rawEcalIso + rawHcalIso")
//         .Define("epIso", "detIsoE / pIso")
//         .Define("rawEpIso", "rawDetIsoE / pIso")
//         .Define("fracEcal", "ecalIso / pIso")
//         .Define("fracHcal", "hcalIso / pIso")
//         .Define("isHadH", "(fracEcal <= 0) && (fracHcal > 0)")
//         .Define("isHadE", "(fracEcal > 0) && (fracHcal <= 0)")
//         .Define("isHadMIP", "(fracEcal > 0) && (fracEcal * pIso < 1) && (fracHcal > 0)")
//         .Define("isHadEH", "(fracEcal > 0) && (fracEcal * pIso >= 1) && (fracHcal > 0)");

//     std::string filename = "histograms/" + std::string(tag) + ".root";
//     TFile out(filename.c_str(), "RECREATE");

//     auto h2_ep = df_iso.Histo2D({"h2_ep_vs_p_iso", "E/p vs p; p (GeV); E/p", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "epIso");
//     auto h2_raw = df_iso.Histo2D({"h2_raw_ep_vs_p_iso", "Raw E/p vs p; p (GeV); Raw E/p", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "rawEpIso");

//     auto prof_ep = df_iso.Profile1D({"prof_ep_vs_p_iso", "<E/p> vs p; p (GeV); E/p", 50, 0, 10}, "pIso", "epIso");
//     auto prof_raw_ep = df_iso.Profile1D({"prof_raw_ep_vs_p_iso", "<Raw E/p> vs p; p (GeV); Raw E/p", 50, 0, 10}, "pIso", "rawEpIso");
//     auto prof_ep_custom = df_iso.Profile1D({"prof_ep_vs_p_iso_custom", "<E/p> vs p (custom); p (GeV); E/p", nTrkPBins, trkPBins}, "pIso", "epIso");
//     auto prof_raw_ep_custom = df_iso.Profile1D({"prof_raw_ep_vs_p_iso_custom", "<Raw E/p> vs p (custom); p (GeV); Raw E/p", nTrkPBins, trkPBins}, "pIso", "rawEpIso");

//     auto h_pt = df_iso.Histo1D({"h_pt_iso", "p_{T} distribution; p_{T} (GeV); Events", 50, 0, 100}, "ptIso");
//     auto h_eta = df_iso.Histo1D({"h_eta_iso", "#eta distribution; #eta; Events", 50, -1.3, 1.3}, "etaIso");
//     auto h_ep = df_iso.Histo1D({"h_ep_iso", "E/p distribution; E/p; Events", 50, 0, 5}, "epIso");
//     auto h_raw_ep = df_iso.Histo1D({"h_raw_ep_iso", "Raw E/p distribution; E/p; Events", 50, 0, 5}, "rawEpIso");

//     auto h_ep_H = df_iso.Filter("Sum(isHadH) > 0").Histo1D({"h_ep_isHadH", "E/p: HCAL only; E/p; Events", 50, 0, 2.5}, "epIso");
//     auto h_ep_E = df_iso.Filter("Sum(isHadE) > 0").Histo1D({"h_ep_isHadE", "E/p: ECAL only; E/p; Events", 50, 0, 2.5}, "epIso");
//     auto h_ep_MIP = df_iso.Filter("Sum(isHadMIP) > 0").Histo1D({"h_ep_isHadMIP", "E/p: MIP; E/p; Events", 50, 0, 2.5}, "epIso");
//     auto h_ep_EH = df_iso.Filter("Sum(isHadEH) > 0").Histo1D({"h_ep_isHadEH", "E/p: ECAL+HCAL; E/p; Events", 50, 0, 2.5}, "epIso");

//     auto h_raw_H = df_iso.Filter("Sum(isHadH) > 0").Histo1D({"h_raw_ep_isHadH", "Raw E/p: HCAL only; E/p; Events", 50, 0, 2.5}, "rawEpIso");
//     auto h_raw_E = df_iso.Filter("Sum(isHadE) > 0").Histo1D({"h_raw_ep_isHadE", "Raw E/p: ECAL only; E/p; Events", 50, 0, 2.5}, "rawEpIso");
//     auto h_raw_MIP = df_iso.Filter("Sum(isHadMIP) > 0").Histo1D({"h_raw_ep_isHadMIP", "Raw E/p: MIP; E/p; Events", 50, 0, 2.5}, "rawEpIso");
//     auto h_raw_EH = df_iso.Filter("Sum(isHadEH) > 0").Histo1D({"h_raw_ep_isHadEH", "Raw E/p: ECAL+HCAL; E/p; Events", 50, 0, 2.5}, "rawEpIso");

//     std::vector<std::pair<double, double>> pBins = {{5.5, 6.0}, {20.0, 22.0}};
//     for (const auto& [pmin, pmax] : pBins) {
//         int bin_min = h2_ep->GetXaxis()->FindBin(pmin + 1e-3);
//         int bin_max = h2_ep->GetXaxis()->FindBin(pmax - 1e-3);

//         std::stringstream sn, sr;
//         sn << "proj_ep_" << int(pmin * 10) << "_" << int(pmax * 10);
//         sr << "proj_raw_ep_" << int(pmin * 10) << "_" << int(pmax * 10);

//         auto proj_n = h2_ep->ProjectionY(sn.str().c_str(), bin_min, bin_max);
//         auto proj_r = h2_raw->ProjectionY(sr.str().c_str(), bin_min, bin_max);

//         proj_n->Write(); proj_r->Write();

//         if (proj_n->Integral() > 0) {
//             proj_n->Scale(1.0 / proj_n->Integral());
//             proj_n->SetFillColorAlpha(kBlue, 0.35);
//             proj_n->SetFillStyle(1001);
//             proj_n->SetLineColor(kBlue + 2);
//             proj_n->GetYaxis()->SetTitle("Fraction of particles");
//             proj_n->GetXaxis()->SetTitle("E/p");

//             std::string fitn_name = sn.str() + "_fit";
//             TF1* fit_n = new TF1(fitn_name.c_str(), "gaus", 0.0, 2.5);
//             proj_n->Fit(fit_n, "RQ", "", 0.7, 1.2);
//             fit_n->Write();

//             TCanvas* c_n = new TCanvas(("canvas_" + sn.str()).c_str(), "Gaussian fit (E/p)", 800, 600);
//             proj_n->Draw("hist");
//             fit_n->SetLineColor(kRed);
//             fit_n->Draw("same");
//             c_n->Write();
//         }

//         if (proj_r->Integral() > 0) {
//             proj_r->Scale(1.0 / proj_r->Integral());
//             proj_r->SetFillColorAlpha(kBlue, 0.35);
//             proj_r->SetFillStyle(1001);
//             proj_r->SetLineColor(kBlue + 2);
//             proj_r->GetYaxis()->SetTitle("Fraction of particles");
//             proj_r->GetXaxis()->SetTitle("E/p");

//             std::string fitr_name = sr.str() + "_fit";
//             TF1* fit_r = new TF1(fitr_name.c_str(), "gaus", 0.0, 2.5);
//             proj_r->Fit(fit_r, "RQ", "", 0.7, 1.2);
//             fit_r->Write();

//             TCanvas* c_r = new TCanvas(("canvas_" + sr.str()).c_str(), "Gaussian fit (Raw E/p)", 800, 600);
//             proj_r->Draw("hist");
//             fit_r->SetLineColor(kRed);
//             fit_r->Draw("same");
//             c_r->Write();
//         }
//     }

//     h2_ep->Write(); h2_raw->Write();
//     prof_ep->Write(); prof_raw_ep->Write();
//     prof_ep_custom->Write(); prof_raw_ep_custom->Write();
//     h_pt->Write(); h_eta->Write(); h_ep->Write(); h_raw_ep->Write();
//     h_ep_H->Write(); h_ep_E->Write(); h_ep_MIP->Write(); h_ep_EH->Write();
//     h_raw_H->Write(); h_raw_E->Write(); h_raw_MIP->Write(); h_raw_EH->Write();

//     out.Close();

//     auto end = std::chrono::high_resolution_clock::now();
//     std::cout << "Elapsed time: " << std::chrono::duration<double>(end - start).count() << " s\n";
// }











#include "ROOT/RDataFrame.hxx"
#include "TFile.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TStyle.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>

// Momentum-binien määrittely
const double trkPBins[] = {
    3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
    7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
};
const int nTrkPBins = sizeof(trkPBins) / sizeof(double) - 1;

void run_histograms(const char* listfile, const char* tag) {
    auto start = std::chrono::high_resolution_clock::now();
    ROOT::EnableImplicitMT();

    std::ifstream infile(listfile);
    std::string line;
    std::vector<std::string> files;
    while (std::getline(infile, line)) {
        if (!line.empty())
            files.push_back("root://hip-cms-se.csc.fi/" + line);
    }

    std::cout << "Processing " << files.size() << " files..." << std::endl;
    ROOT::RDataFrame df("Events", files);

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
        .Define("ptIso", "PFCand_pt[isoMask]")
        .Define("etaIso", "PFCand_eta[isoMask]")
        .Define("pIso", "ptIso * cosh(etaIso)")
        .Define("ecalIso", "PFCand_ecalEnergy[isoMask]")
        .Define("hcalIso", "PFCand_hcalEnergy[isoMask]")
        .Define("rawEcalIso", "PFCand_rawEcalEnergy[isoMask]")
        .Define("rawHcalIso", "PFCand_rawHcalEnergy[isoMask]")
        .Define("detIsoE", "ecalIso + hcalIso")
        .Define("rawDetIsoE", "rawEcalIso + rawHcalIso")
        .Define("epIso", "detIsoE / pIso")
        .Define("rawEpIso", "rawDetIsoE / pIso")
        .Define("fracEcal", "ecalIso / pIso")
        .Define("fracHcal", "hcalIso / pIso")
        .Define("isHadH", "(fracEcal <= 0) && (fracHcal > 0)")
        .Define("isHadE", "(fracEcal > 0) && (fracHcal <= 0)")
        .Define("isHadMIP", "(fracEcal > 0) && (fracEcal * pIso < 1) && (fracHcal > 0)")
        .Define("isHadEH", "(fracEcal > 0) && (fracEcal * pIso >= 1) && (fracHcal > 0)")
        .Define("hcalCorrFactor", "hcalIso / rawHcalIso")
        .Define("response", "detIsoE / pIso");

    std::string filename = "histograms/" + std::string(tag) + ".root";
    TFile out(filename.c_str(), "RECREATE");

    auto h2_resp_vs_hcalCorr = df_iso
        .Filter("Sum(rawHcalIso) > 0")
        .Histo2D({"h2_resp_vs_hcalCorr", "Response vs E_{HCAL}/E_{HCAL}^{raw}; E_{HCAL}/E_{HCAL}^{raw}; Response (E/p)", 50, 0.4, 1.6, 50, 0.3, 1.8}, "hcalCorrFactor", "response");

    // Canvas and style for white background on zero-content bins
    TCanvas* c_corr = new TCanvas("c_corr", "Response vs HCAL Correction", 800, 600);
    gStyle->SetPalette(kBird);
    h2_resp_vs_hcalCorr->Draw("COLZ");
    gPad->Update();
    auto palette = (TPaletteAxis*)h2_resp_vs_hcalCorr->GetListOfFunctions()->FindObject("palette");
    h2_resp_vs_hcalCorr->SetMinimum(1e-6);
    gPad->Modified();
    gPad->Update();
    c_corr->Write();

    h2_resp_vs_hcalCorr->Write();

    auto h2_ep = df_iso.Histo2D({"h2_ep_vs_p_iso", "E/p vs p; p (GeV); E/p", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "epIso");
    auto h2_raw = df_iso.Histo2D({"h2_raw_ep_vs_p_iso", "Raw E/p vs p; p (GeV); Raw E/p", nTrkPBins, trkPBins, 40, 0.05, 2.25}, "pIso", "rawEpIso");

    auto prof_ep = df_iso.Profile1D({"prof_ep_vs_p_iso", "<E/p> vs p; p (GeV); E/p", 50, 0, 10}, "pIso", "epIso");
    auto prof_raw_ep = df_iso.Profile1D({"prof_raw_ep_vs_p_iso", "<Raw E/p> vs p; p (GeV); Raw E/p", 50, 0, 10}, "pIso", "rawEpIso");
    auto prof_ep_custom = df_iso.Profile1D({"prof_ep_vs_p_iso_custom", "<E/p> vs p (custom); p (GeV); E/p", nTrkPBins, trkPBins}, "pIso", "epIso");
    auto prof_raw_ep_custom = df_iso.Profile1D({"prof_raw_ep_vs_p_iso_custom", "<Raw E/p> vs p (custom); p (GeV); Raw E/p", nTrkPBins, trkPBins}, "pIso", "rawEpIso");

    auto h_pt = df_iso.Histo1D({"h_pt_iso", "p_{T} distribution; p_{T} (GeV); Events", 50, 0, 100}, "ptIso");
    auto h_eta = df_iso.Histo1D({"h_eta_iso", "#eta distribution; #eta; Events", 50, -1.3, 1.3}, "etaIso");
    auto h_ep = df_iso.Histo1D({"h_ep_iso", "E/p distribution; E/p; Events", 50, 0, 5}, "epIso");
    auto h_raw_ep = df_iso.Histo1D({"h_raw_ep_iso", "Raw E/p distribution; E/p; Events", 50, 0, 5}, "rawEpIso");

    auto h_ep_H = df_iso.Filter("Sum(isHadH) > 0").Histo1D({"h_ep_isHadH", "E/p: HCAL only; E/p; Events", 50, 0, 2.5}, "epIso");
    auto h_ep_E = df_iso.Filter("Sum(isHadE) > 0").Histo1D({"h_ep_isHadE", "E/p: ECAL only; E/p; Events", 50, 0, 2.5}, "epIso");
    auto h_ep_MIP = df_iso.Filter("Sum(isHadMIP) > 0").Histo1D({"h_ep_isHadMIP", "E/p: MIP; E/p; Events", 50, 0, 2.5}, "epIso");
    auto h_ep_EH = df_iso.Filter("Sum(isHadEH) > 0").Histo1D({"h_ep_isHadEH", "E/p: ECAL+HCAL; E/p; Events", 50, 0, 2.5}, "epIso");

    auto h_raw_H = df_iso.Filter("Sum(isHadH) > 0").Histo1D({"h_raw_ep_isHadH", "Raw E/p: HCAL only; E/p; Events", 50, 0, 2.5}, "rawEpIso");
    auto h_raw_E = df_iso.Filter("Sum(isHadE) > 0").Histo1D({"h_raw_ep_isHadE", "Raw E/p: ECAL only; E/p; Events", 50, 0, 2.5}, "rawEpIso");
    auto h_raw_MIP = df_iso.Filter("Sum(isHadMIP) > 0").Histo1D({"h_raw_ep_isHadMIP", "Raw E/p: MIP; E/p; Events", 50, 0, 2.5}, "rawEpIso");
    auto h_raw_EH = df_iso.Filter("Sum(isHadEH) > 0").Histo1D({"h_raw_ep_isHadEH", "Raw E/p: ECAL+HCAL; E/p; Events", 50, 0, 2.5}, "rawEpIso");

    std::vector<std::pair<double, double>> pBins = {{5.5, 6.0}, {20.0, 22.0}};
    for (const auto& [pmin, pmax] : pBins) {
        int bin_min = h2_ep->GetXaxis()->FindBin(pmin + 1e-3);
        int bin_max = h2_ep->GetXaxis()->FindBin(pmax - 1e-3);

        std::stringstream sn, sr;
        sn << "proj_ep_" << int(pmin * 10) << "_" << int(pmax * 10);
        sr << "proj_raw_ep_" << int(pmin * 10) << "_" << int(pmax * 10);

        auto proj_n = h2_ep->ProjectionY(sn.str().c_str(), bin_min, bin_max);
        auto proj_r = h2_raw->ProjectionY(sr.str().c_str(), bin_min, bin_max);

        proj_n->Write(); proj_r->Write();

        if (proj_n->Integral() > 0) {
            proj_n->Scale(1.0 / proj_n->Integral());
            proj_n->SetFillColorAlpha(kBlue, 0.35);
            proj_n->SetFillStyle(1001);
            proj_n->SetLineColor(kBlue + 2);
            proj_n->GetYaxis()->SetTitle("Fraction of particles");
            proj_n->GetXaxis()->SetTitle("E/p");

            std::string fitn_name = sn.str() + "_fit";
            TF1* fit_n = new TF1(fitn_name.c_str(), "gaus", 0.0, 2.5);
            proj_n->Fit(fit_n, "RQ", "", 0.7, 1.2);
            fit_n->Write();

            TCanvas* c_n = new TCanvas(("canvas_" + sn.str()).c_str(), "Gaussian fit (E/p)", 800, 600);
            proj_n->Draw("hist");
            fit_n->SetLineColor(kRed);
            fit_n->Draw("same");
            c_n->Write();
        }

        if (proj_r->Integral() > 0) {
            proj_r->Scale(1.0 / proj_r->Integral());
            proj_r->SetFillColorAlpha(kBlue, 0.35);
            proj_r->SetFillStyle(1001);
            proj_r->SetLineColor(kBlue + 2);
            proj_r->GetYaxis()->SetTitle("Fraction of particles");
            proj_r->GetXaxis()->SetTitle("E/p");

            std::string fitr_name = sr.str() + "_fit";
            TF1* fit_r = new TF1(fitr_name.c_str(), "gaus", 0.0, 2.5);
            proj_r->Fit(fit_r, "RQ", "", 0.7, 1.2);
            fit_r->Write();

            TCanvas* c_r = new TCanvas(("canvas_" + sr.str()).c_str(), "Gaussian fit (Raw E/p)", 800, 600);
            proj_r->Draw("hist");
            fit_r->SetLineColor(kRed);
            fit_r->Draw("same");
            c_r->Write();
        }
    }

    h2_ep->Write(); h2_raw->Write();
    prof_ep->Write(); prof_raw_ep->Write();
    prof_ep_custom->Write(); prof_raw_ep_custom->Write();
    h_pt->Write(); h_eta->Write(); h_ep->Write(); h_raw_ep->Write();
    h_ep_H->Write(); h_ep_E->Write(); h_ep_MIP->Write(); h_ep_EH->Write();
    h_raw_H->Write(); h_raw_E->Write(); h_raw_MIP->Write(); h_raw_EH->Write();

    out.Close();

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time: " << std::chrono::duration<double>(end - start).count() << " s\n";
}