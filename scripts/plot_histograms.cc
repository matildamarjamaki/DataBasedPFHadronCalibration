
// ---------------- ROOT-kirjastot -------------
// Perus ROOT-headereita kanvaksille, tiedostoille, tyyleille, histoille, graafeille ja fit-funktioille
#include "TROOT.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TColor.h"
#include "TPad.h"
#include "TLegend.h"
#include "TPaveStats.h"
#include "TSystem.h"
#include "TString.h"

#include "TH1.h"
#include "TH1D.h"
#include "TH2.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TProfile.h"
#include "THStack.h"

#include "TGraph.h"
#include "TGraphErrors.h"
#include "TF1.h"
#include "TLatex.h"

#include <cmath>
#include <limits>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

// ---------- ETUDEKLARAATIOT (prototyypit) ----------
// Pääplotit: MC vs Data E/p -overlay, pinottu fraktiovertailu, "kaikki plottaukset" ydinfunktio,
// sivukomposiittien (PDF) kokoaminen ja 2D-heatmapit log-x:llä
void plot_ep_overlay();
void plot_stacked_comparison_raw(bool useHCALCut);
void plot_histograms(const std::string& filename);
void compile_response_pages(const char* inpath,
                            const char* outpdf,
                            const std::vector<std::pair<double,double>>& pBins);
void make_h2_maps_logx_pdf(const char* inpath, const char* outdirTag);


// Yhteiset tiedostopolut (DATA & MC sisäänmenohistot, sekä ulostulopohjakansio)
constexpr const char* kFileMC   = "histograms/SingleNeutrino2024.root";
constexpr const char* kFileData = "histograms/ZeroBias2024_All.root";
constexpr const char* kOutBase  = "plots2";


//  ------------- Pääfunktio -------------
// - Jos argumentti on erikoislippu (esim. --compile-both), ajetaan koosteet/heatmapit
// - Jos argumentti on "dummy", ajetaan muutama nopea vertailuplotti
// - Muuten avataan annettu ROOT-tiedosto ja piirretään kaikki määritellyt kuvat
void plot_histograms(const std::string& filename) {

  // -erikoisajot ilman yksittäisen ROOT-tiedoston avaamista
  // Nämä polut tuottavat PDF-koostesivuja ja/tai logaritmisella x-akselilla
  // varustettuja 2D-kartta-PDF:iä suoraan DATA/MC -tiedostoista
  if (filename == "--compile-both" || filename == "--compile-data" ||
      filename == "--compile-mc"   || filename == "make_h2") {

    // p-ikkunat koostesivuille (järkevät ”matala–keskitaso–korkea” esimerkit)
    std::vector<std::pair<double,double>> pBins = {
      {5.5, 6.0}, {10.0, 12.0}, {20.0, 22.0}
    };

    // varmistetaan ulostulohakemistot
    gSystem->mkdir("plots2", true);
    gSystem->mkdir("plots2/Data_ZeroBias2024", true);
    gSystem->mkdir("plots2/MC_SingleNeutrino2024", true);

    // kooste-PDF:t
    if (filename == "--compile-both" || filename == "--compile-data") {
      compile_response_pages(kFileData, "plots2/Data_ZeroBias2024/response_pages.pdf", pBins);
    }
    if (filename == "--compile-both" || filename == "--compile-mc") {
      compile_response_pages(kFileMC,   "plots2/MC_SingleNeutrino2024/response_pages.pdf", pBins);
    }

    // log-x heatmap PDF:t (2D: E/p vs p, sekä per-tyyppi projektiot 3D:stä)
    if (filename == "--compile-both" || filename == "make_h2") {
      make_h2_maps_logx_pdf(kFileData, "Data_ZeroBias2024");
      make_h2_maps_logx_pdf(kFileMC,   "MC_SingleNeutrino2024");
    }

    return; // tärkeää: ei yritetä avata "filename":a ROOT-tiedostona
  }

  // ---- "dummy" = nopeat vertailuplottaukset ----
  // (Tuottaa yhden MC vs Data overlayn Crystal Ball -fiteillä + kaksi stacked-vertailua)
  if (filename == "dummy") {
    plot_ep_overlay();
    plot_stacked_comparison_raw(false); // no cut
    plot_stacked_comparison_raw(true);  // HCAL cut
    return;
  }

  // ---- Normaali tapaus: avaa annettu ROOT-tiedosto ----
  // Tänne tullaan kun halutaan piirtää ”kaikki yksittäiset” kuvat tietystä tiedostosta.
  TFile* file = TFile::Open(filename.c_str(), "READ");
  if (!file || file->IsZombie()) {
    std::cerr << "Error: cannot open file " << filename << std::endl;
    return;
  }

  // // Valitse outdir automaattisesti syötetiedoston nimen perusteella (Data vs MC)
  // std::string tag = "Unknown";


  // // if (filename.find("ZeroBias") != std::string::npos)             tag = "Data_ZeroBias2024";
  // // else if (filename.find("SingleNeutrino") != std::string::npos)  tag = "MC_SingleNeutrino2024";
  // if (filename.find("ZeroBias") != std::string::npos) {
  //     tag = "Data_" + filename;
  //     // poista .root lopusta:
  //     size_t pos = tag.find(".root");
  //     if (pos != std::string::npos) tag = tag.substr(0, pos);
  // }


  // std::string outdir = "plots2/" + tag + "/";
  // gSystem->mkdir(outdir.c_str(), true);

  // // Asetetaan paletti 2D-ploteille (väriskaala)
  // gStyle->SetPalette(kBird);


    // Valitse outdir automaattisesti syötetiedoston nimen perusteella (Data vs MC)
    std::string tag = "Unknown";

    // Poimi tiedostonimi ilman polkua
    std::string base = gSystem->BaseName(filename.c_str());

    // Poista .root-pääte
    size_t pos = base.find(".root");
    if (pos != std::string::npos) base = base.substr(0, pos);

    // Aseta tag datasetin tyypin mukaan (vain informatiivisesti, ei kansiopolussa)
    if (filename.find("ZeroBias") != std::string::npos) {
        tag = "Data_" + base;
    } 
    else if (filename.find("SingleNeutrino") != std::string::npos) {
        tag = "MC_" + base;
    } 
    else {
        tag = "Unknown_" + base;
    }

    // Peruskansio vuosien mukaan
    std::string baseplots = "plots2";
    if (filename.find("2025") != std::string::npos) baseplots = "plots2025";

    // Lopullinen ulostulokansio: käytetään datasetin nimeä (base) kansiona
    std::string outdir = baseplots + "/" + base + "/";
    gSystem->mkdir(outdir.c_str(), true);

    // Debug-tulostus
    std::cout << "[plot_histograms] Writing plots to: " << outdir << std::endl;



    // Lista nimistä, jotka vastaavat uutta run_histograms.cc:tä
    std::vector<std::string> histNames = {
        // 1D: E/p raakana hadronityypeittäin (nämä ovat olemassa)
        "h_raw_ep_isHadH", "h_raw_ep_isHadE", "h_raw_ep_isHadMIP", "h_raw_ep_isHadEH",

        // 2D: E/p vs p kaikkiin neljään skenaarioon
        "h2_ep_vs_p_S1_raw_cut",
        "h2_ep_vs_p_S2_def_cut",
        "h2_ep_vs_p_S3_raw_all",
        "h2_ep_vs_p_S4_def_all",

        // Profiilit: <E/p> vs p kaikkiin neljään skenaarioon
        "prof_ep_vs_p_S1_raw_cut",
        "prof_ep_vs_p_S2_def_cut",
        "prof_ep_vs_p_S3_raw_all",
        "prof_ep_vs_p_S4_def_all",

        // Hadronifraktio-profiilit (all vs cut)
        "h_frac_H_all","h_frac_E_all","h_frac_MIP_all","h_frac_EH_all",
        "h_frac_H_cut","h_frac_E_cut","h_frac_MIP_cut","h_frac_EH_cut"
    };

    // Piirrä kaikki yllä listatut objektit jos löytyvät
    for (const auto& name : histNames) {
        TObject* obj = file->Get(name.c_str());
        if (!obj) continue;

        TCanvas* c = new TCanvas(("canvas_" + name).c_str(), name.c_str(), 900, 700);

        if (auto* h2 = dynamic_cast<TH2*>(obj)) {
            h2->GetXaxis()->SetTitle("Track momentum p (GeV)");
            h2->GetYaxis()->SetTitle((name.find("raw") != std::string::npos) ? "Raw E/p" : "E/p");
            h2->SetTitle((name.find("raw") != std::string::npos) ? "Raw E/p vs p" : "E/p vs p");
            h2->Draw("COLZ");
        } else if (auto* h1 = dynamic_cast<TH1*>(obj)) {
            h1->GetXaxis()->SetTitle("E/p");
            h1->GetYaxis()->SetTitle("Events");
            h1->SetTitle(name.c_str());
            h1->Draw("HIST");
        } else if (auto* prof = dynamic_cast<TProfile*>(obj)) {
            prof->GetXaxis()->SetTitle("Track momentum p (GeV)");
            if (name.rfind("h_frac_", 0) == 0) {          // profiilit, jotka ovat fraktioita
                prof->GetYaxis()->SetTitle("Fraction");
                prof->SetTitle("Hadron type fraction vs p");
            } else {                                      // <E/p> profiilit
                prof->GetYaxis()->SetTitle("Mean E/p");
                prof->SetTitle("Mean E/p vs p");
            }
            prof->Draw();
        }

        std::string histDir = outdir + "histograms/";
        gSystem->mkdir(histDir.c_str(), true);

        c->SaveAs((histDir + name + ".png").c_str());
        delete c;
    }

    // -------- 3D -> 2D -projektiot (uudet nimet & oikea akselijärjestys) --------
    // h3_*:  X = E/p,  Y = E_HCAL / E_HCAL^raw,  Z = p
    // Tehdään XY-projektio valituissa p-alueissa; HUOM: Y-akseli on korjausfaktori
    std::vector<std::pair<std::string,std::string>> h3_list = {
        {"h3_S1_raw_cut_ep_vs_hcal_p", "S1: Raw (HCAL E/E_{raw} > 0.9)"},
        {"h3_S2_def_cut_ep_vs_hcal_p", "S2: Default (HCAL E/E_{raw} > 0.9)"},
        {"h3_S3_raw_ep_vs_hcal_p",     "S3: Raw (no cut)"},
        {"h3_S4_def_ep_vs_hcal_p",     "S4: Default (no cut)"}
    };

    // Valitse p-ikkunat (voit muokata näitä)
    std::vector<std::pair<double,double>> pRanges = {{5.5,6.0},{10.0,12.0},{20.0,22.0}};
    std::vector<std::string> pTags   = {"low","medium","high"};

    for (const auto& [h3name, titleTag] : h3_list) {
        TH3D* h3 = dynamic_cast<TH3D*>(file->Get(h3name.c_str()));
        if (!h3) continue;

        for (size_t j = 0; j < pRanges.size(); ++j) {
            const double pmin = pRanges[j].first;
            const double pmax = pRanges[j].second;

            // Rajaa Z (p) ja tee XY-projektio: X=E/p, Y=E_HCAL/E_HCAL^raw
            const int zmin = h3->GetZaxis()->FindBin(pmin + 1e-3);
            const int zmax = h3->GetZaxis()->FindBin(pmax - 1e-3);
            h3->GetZaxis()->SetRange(zmin, zmax);

            TH2* proj2D = dynamic_cast<TH2*>(h3->Project3D("xy"));
            if (!proj2D) continue;

            std::string projName = h3name + "_proj2D_" + pTags[j];
            proj2D->SetName(projName.c_str());
            proj2D->SetTitle(Form("%s, p = %.1f-%.1f GeV", titleTag.c_str(), pmin, pmax));
            proj2D->GetXaxis()->SetTitle("E/p");
            proj2D->GetYaxis()->SetTitle("E_{HCAL}/E_{HCAL}^{raw}");
            proj2D->SetMinimum(1e-6);

            TCanvas* c2 = new TCanvas(("canvas_" + projName).c_str(), projName.c_str(), 900, 700);
            proj2D->GetYaxis()->SetRangeUser(0.9, 2.5);   // leikkausalue näkyviin Y-akselille
            proj2D->Draw("COLZ");
            std::string projDir = outdir + "projections/";
            gSystem->mkdir(projDir.c_str(), true);

            c2->SaveAs((projDir + projName + ".png").c_str());
            delete c2;


            // Palaa täyteen Z-rangeen seuraavaa projektiota varten
            h3->GetZaxis()->UnZoom();
        }
    }





// ----------------------------------------------------
// E/p-jakaumat valituilla p-alueilla (CUT), 3-Gauss fitti + komponentit
// ---------------------------------------------------
std::vector<std::pair<double, double>> pBins = {{5.5, 6.0}, {20.0, 22.0}};

// Uudet 2D-histot (CUT-versiot):
//  - S1: Raw E/p vs p (HCAL E/E_raw > 0.9)
//  - S2: Default E/p vs p (HCAL E/E_raw > 0.9)
TH2D* h2_raw_cut = dynamic_cast<TH2D*>(file->Get("h2_ep_vs_p_S1_raw_cut"));
TH2D* h2_def_cut = dynamic_cast<TH2D*>(file->Get("h2_ep_vs_p_S2_def_cut"));

// Apufunktio: projektoi E/p (Y) valitulle p-alueelle ja fittaa 3 Gaussia
auto fit3G = [&](TH2D* h2, const char* tag, const char* ytitle) {
    if (!h2) {
        std::cerr << "[WARN] histogram not found for " << tag << std::endl;
        return;
    }
    for (auto [pmin, pmax] : pBins) {
        int x1 = h2->GetXaxis()->FindBin(pmin + 1e-3);
        int x2 = h2->GetXaxis()->FindBin(pmax - 1e-3);

        std::string hname = Form("proj_%s_%d_%d", tag, int(pmin*10), int(pmax*10));
        TH1D* proj = h2->ProjectionY(hname.c_str(), x1, x2);
        if (!proj || proj->Integral() <= 0) continue;
        proj->SetDirectory(nullptr);
        proj->Scale(1.0 / proj->Integral());

        TF1* fit = new TF1((hname + "_fit").c_str(), "gaus(0)+gaus(3)+gaus(6)", 0.0, 2.5);
        // amp1, mean1, sigma1,  amp2, mean2, sigma2,  amp3, mean3, sigma3 (alut voi säätää)
        fit->SetParameters(0.05, 1.00, 0.10,   0.05, 0.80, 0.10,   0.02, 1.30, 0.20);
        proj->Fit(fit, "RQ", "", 0.7, 1.2);

        TCanvas* c = new TCanvas(("canvas_" + hname).c_str(), "", 800, 600);
        proj->SetFillColorAlpha(kBlue, 0.35);
        proj->SetTitle(Form("%s, %.1f - %.1f GeV (HCAL E/E_{raw} > 0.9)", ytitle, pmin, pmax));
        proj->GetXaxis()->SetTitle(ytitle);
        proj->GetYaxis()->SetTitle("Fraction of particles");
        proj->Draw("hist");

        TF1* fit_full = (TF1*)fit->Clone((hname + "_fit_full").c_str());
        fit_full->SetRange(0.0, 2.5);
        fit_full->SetLineColor(kRed);
        fit_full->SetLineWidth(2);
        fit_full->Draw("same");

        // Yksittäiset gauss-komponentit
        TF1* g1 = new TF1((hname + "_g1").c_str(), "gaus", 0.0, 2.5);
        TF1* g2 = new TF1((hname + "_g2").c_str(), "gaus", 0.0, 2.5);
        TF1* g3 = new TF1((hname + "_g3").c_str(), "gaus", 0.0, 2.5);
        g1->SetParameters(fit->GetParameter(0), fit->GetParameter(1), fit->GetParameter(2));
        g2->SetParameters(fit->GetParameter(3), fit->GetParameter(4), fit->GetParameter(5));
        g3->SetParameters(fit->GetParameter(6), fit->GetParameter(7), fit->GetParameter(8));
        g1->SetLineColor(kBlue + 2);   g1->SetLineStyle(2); g1->SetLineWidth(3); g1->Draw("same");
        g2->SetLineColor(kGreen + 3);  g2->SetLineStyle(3); g2->SetLineWidth(3); g2->Draw("same");
        g3->SetLineColor(kOrange + 1); g3->SetLineStyle(4); g3->SetLineWidth(3); g3->Draw("same");

        std::string fitsDir = outdir + "fits/";
        gSystem->mkdir(fitsDir.c_str(), true);

        c->SaveAs((fitsDir + hname + ".png").c_str());
        c->Write();
    }
};

// Ajetaan S1 ja S2 (poistetaan toinen, jos halutaan käsitellä vain toista casea)
fit3G(h2_raw_cut, "raw_ep_cut", "Raw E/p");
fit3G(h2_def_cut, "def_ep_cut", "E/p");

// (myös versiot ilman leikkausta: S3/S4)
// TH2D* h2_raw_all = dynamic_cast<TH2D*>(file->Get("h2_ep_vs_p_S3_raw_all"));
// TH2D* h2_def_all = dynamic_cast<TH2D*>(file->Get("h2_ep_vs_p_S4_def_all"));
// fit3G(h2_raw_all, "raw_ep_all", "Raw E/p");
// fit3G(h2_def_all, "def_ep_all", "E/p");



// --------------------------------------------------------------
// Fitted mean (ilman HCAL-korjausleikkausta), TGraph-muodossa
// --------------------------------------------------------------

TGraph* g_fit = new TGraph();
g_fit->SetName("g_fit_mean_nocut");
g_fit->SetTitle("Fitted mean E/p vs p (no cut);p (GeV);mean(E/p)");

std::vector<std::pair<double, double>> fit_pBins = {{5.5, 6.0}, {10.0, 12.0}, {20.0, 22.0}};
std::vector<double> pMid = {5.75, 11.0, 21.0};

// käytetään S4: Default E/p, no cut
TH2D* h2_def_all = dynamic_cast<TH2D*>(file->Get("h2_ep_vs_p_S4_def_all"));
if (h2_def_all) {
    for (size_t i = 0; i < fit_pBins.size(); ++i) {
        double pmin = fit_pBins[i].first;
        double pmax = fit_pBins[i].second;

        int bin_min = h2_def_all->GetXaxis()->FindBin(pmin + 1e-3);
        int bin_max = h2_def_all->GetXaxis()->FindBin(pmax - 1e-3);

        std::string projName = Form("proj_fit_ep_nocut_%zu", i);
        TH1D* hproj = h2_def_all->ProjectionY(projName.c_str(), bin_min, bin_max);
        if (!hproj || hproj->Integral() == 0) { delete hproj; continue; }

        hproj->Scale(1.0 / hproj->Integral()); // normalisoi muoto
        TF1* fit = new TF1((projName + "_fit").c_str(), "gaus", 0.7, 1.2);
        hproj->Fit(fit, "RQ");

        g_fit->SetPoint(g_fit->GetN(), pMid[i], fit->GetParameter(1));

        fit->Write();               // talleta halutessasi
        // (hproj talletetaan myöhemmin, jos haluat; muuten vapauta)
        delete hproj;
    }

    // piirrä ja talleta TGraph
    TCanvas* c_mean_nocut = new TCanvas("c_mean_nocut", "", 800, 600);
    g_fit->SetMarkerStyle(20);
    g_fit->SetMarkerSize(1.1);
    g_fit->Draw("AP");
    std::string summaryDir = outdir + "summary/";
    gSystem->mkdir(summaryDir.c_str(), true);

    c_mean_nocut->SaveAs((summaryDir + "mean_ep_vs_p_nocut.png").c_str());
    c_mean_nocut->Write();

    g_fit->Write(); // talleta graafi ROOT-tiedostoon
}

// (Vaihtoehtoisesti nopeampi: käytä profiilia prof_ep_vs_p_S4_def_all 
// ja ota sen bin-meanit suoraan ilman Gauss-fittiä.)

// --------------------------------------
// Fitted-meanit HCAL-korjauksen jälkeen
// --------------------------------------

// p-alueet ja tagit kuvien nimeämiseen
std::vector<std::pair<double, double>> hcalcut_pBins = {{5.5, 6.5}, {10.0, 12.0}, {20.0, 24.0}};
std::vector<std::string> tags = {"low", "medium", "high"};

TGraph* g_fit_hcalcut = new TGraph();
g_fit_hcalcut->SetName("g_fit_mean_hcalcut");
g_fit_hcalcut->SetTitle("Fitted mean E/p vs p (HCAL cut);p (GeV);mean(E/p)");

// käytetään S2: Default E/p, HCAL E/Eraw > 0.9
TH3D* h3 = dynamic_cast<TH3D*>(file->Get("h3_S2_def_cut_ep_vs_hcal_p"));
if (h3) {
    int y_min = h3->GetYaxis()->FindBin(0.9001); // 0.9 < E_HCAL/E_HCAL^raw < 2.5
    int y_max = h3->GetYaxis()->FindBin(2.4999);

    for (size_t i = 0; i < hcalcut_pBins.size(); ++i) {
        double pmin = hcalcut_pBins[i].first, pmax = hcalcut_pBins[i].second;
        double pmid = pMid[i];

        int z_min = h3->GetZaxis()->FindBin(pmin + 1e-3);
        int z_max = h3->GetZaxis()->FindBin(pmax - 1e-3);

        std::string projName = "proj_resp_cut_" + tags[i];
        TH1D* hproj = h3->ProjectionX(projName.c_str(), y_min, y_max, z_min, z_max);
        if (!hproj || hproj->Integral() == 0) { delete hproj; continue; }

        hproj->Scale(1.0 / hproj->Integral());
        TF1* fit = new TF1((projName + "_fit").c_str(), "gaus", 0.7, 1.2);
        hproj->Fit(fit, "RQ");

        g_fit_hcalcut->SetPoint(g_fit_hcalcut->GetN(), pmid, fit->GetParameter(1));

        // piirrä ja talleta jokainen projektiokuva (valinnainen)
        TCanvas* c = new TCanvas(("canvas_" + projName).c_str(), "", 800, 600);
        hproj->SetTitle(Form("E/p for HCALcut, %s (%.1f - %.1f GeV)", tags[i].c_str(), pmin, pmax));
        hproj->GetXaxis()->SetTitle("E/p");
        hproj->GetYaxis()->SetTitle("Fraction of particles");
        hproj->Draw("HIST");
        fit->SetLineColor(kRed);
        fit->Draw("same");
        std::string projDir = outdir + "projections/";
        gSystem->mkdir(projDir.c_str(), true);

        c->SaveAs((projDir + projName + ".png").c_str());
        c->Write();
        fit->Write();


        delete c;
        delete hproj;
    }

    // piirrä ja talleta HCALcut-graafi
    TCanvas* c_mean_cut = new TCanvas("c_mean_cut", "", 800, 600);
    g_fit_hcalcut->SetMarkerStyle(20);
    g_fit_hcalcut->SetMarkerSize(1.1);
    g_fit_hcalcut->Draw("AP");
    std::string summaryDir = outdir + "summary/";
    gSystem->mkdir(summaryDir.c_str(), true);

    c_mean_cut->SaveAs((summaryDir + "mean_ep_vs_p_hcalcut.png").c_str());
    c_mean_cut->Write();
    g_fit_hcalcut->Write();


    h3->GetYaxis()->UnZoom();
    h3->GetZaxis()->UnZoom();
}


// -----------------------------------
// E/p per hadronityyppi kaikissa p-bineissä — KAIKKI CASET ERIKSEEN
// -----------------------------------

// p-binit
const double trkPBins_full[] = {
  3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
  7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
  16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
};
const int nTrkPBins_full = sizeof(trkPBins_full)/sizeof(double) - 1;

// hadronityypit ja labelit
const std::vector<std::pair<std::string,std::string>> hadronTypesWithLabel = {
  {"isHadH",  "HCALonly"},
  {"isHadE",  "ECALonly"},
  {"isHadMIP","MIP"},
  {"isHadEH", "ECALHCAL"}
};

// caset (huom: nimet pitää vastata run_histograms.cc:ssä kirjoitettuihin)
struct CaseDef {
  std::string prefix;   // h3-nimen alku
  std::string suffix;   // h3-nimen loppu
  std::string tag;      // alikansioille ja tiedostonimille
  bool        useHcalCut; // true => 0.9–2.5, false => koko Y-akseli
};

// Jos teit myös RAW+cut -versiot, jätä nämä neljä; muuten poista *_cut raw -case
const std::vector<CaseDef> cases = {
  // default E/p + HCAL cut (tuotettu nimillä h3_resp_corr_p_isHad*)
  {"h3_resp_corr_p_", "",      "def_cut",  true},
  // default E/p, no cut
  {"h3_resp_def_p_",  "_all",  "def_all",  false},
  // raw E/p, no cut
  {"h3_resp_raw_p_",  "_all",  "raw_all",  false},
  // raw E/p + HCAL cut (vain jos lisäsit nämä run_histograms.cc:hen)
  {"h3_resp_raw_p_",  "_cut",  "raw_cut",  true}
};

gStyle->SetOptFit(111);

for (const auto& cs : cases) {
  // tee oma ulostulokansio tälle caselle
  std::string outdir_case = outdir + "perType_" + cs.tag + "/";
  gSystem->mkdir(outdir_case.c_str(), true);

  for (const auto& [type, label] : hadronTypesWithLabel) {
    std::string hname = cs.prefix + type + cs.suffix;
    TH3D* h3 = dynamic_cast<TH3D*>(file->Get(hname.c_str()));
    if (!h3) {
      std::cout << "[WARN] Missing TH3: " << hname << " in " << filename << std::endl;
      continue;
    }

    // Y-akselin (HCAL E/Eraw) alue: leikkauksessa 0.9–2.5, muuten koko alue
    int y_min = h3->GetYaxis()->GetFirst();
    int y_max = h3->GetYaxis()->GetLast();
    if (cs.useHcalCut) {
      y_min = h3->GetYaxis()->FindBin(0.9001);
      y_max = h3->GetYaxis()->FindBin(2.4999);
    }

    for (int i = 0; i < nTrkPBins_full; ++i) {
      double pmin = trkPBins_full[i];
      double pmax = trkPBins_full[i+1];

      int z_min = h3->GetZaxis()->FindBin(pmin + 1e-3);
      int z_max = h3->GetZaxis()->FindBin(pmax - 1e-3);

      // Projektio X (E/p)
      std::string projName = Form("proj_ep_%s_%s_bin_%d", label.c_str(), cs.tag.c_str(), i);
      TH1D* hproj = h3->ProjectionX(projName.c_str(), y_min, y_max, z_min, z_max);
      if (!hproj || hproj->Integral() == 0) { delete hproj; continue; }

      hproj->Scale(1.0 / hproj->Integral());
      hproj->SetTitle(Form("E/p (%s, %s), %.1f - %.1f GeV", label.c_str(), cs.tag.c_str(), pmin, pmax));
      hproj->GetXaxis()->SetTitle("E/p");
      hproj->GetYaxis()->SetTitle("Fraction of particles");
      hproj->SetLineColor(kBlue + 2);

      // 3-Gauss fitti (samat startit kuin aiemmin, p-alueesta riippuvat rajat)
      TF1* fit = new TF1(("fit_" + projName).c_str(), "gaus(0)+gaus(3)+gaus(6)", 0.0, 2.5);
      fit->SetNpx(1000);

      if (pmax < 4.0) {
        fit->SetParameters(0.2, 1.0, 0.2,   0.05, 0.6, 0.1,   0.01, 1.4, 0.3);
        fit->SetParLimits(1, 0.8, 1.2);
        fit->SetParLimits(4, 0.4, 0.8);
      } else if (pmax < 10.0) {
        fit->SetParameters(0.1, 0.8, 0.15,  0.03, 1.0, 0.1,   0.005, 1.4, 0.25);
        fit->SetParLimits(1, 0.6, 1.0);
        fit->SetParLimits(4, 0.8, 1.2);
      } else if (pmax < 25.0) {
        fit->SetParameters(0.2, 0.7, 0.1,   0.1, 1.0, 0.1,   0.01, 1.4, 0.25);
        fit->SetParLimits(1, 0.4, 0.9);
        fit->SetParLimits(4, 0.8, 1.1);
      } else {
        fit->SetParameters(0.3, 0.1, 0.03,  0.1, 0.9, 0.2,   0.01, 1.4, 0.3);
        fit->SetParLimits(1, 0.01, 0.3);
        fit->SetParLimits(2, 0.005, 0.1);
        fit->SetParLimits(4, 0.6, 1.1);
      }
      fit->SetLineColor(kRed);
      fit->SetLineWidth(2);

      // piirto
      TCanvas* c = new TCanvas(("canvas_" + projName).c_str(), "", 800, 600);
      hproj->Fit(fit, "RQ");
      hproj->Draw("HIST");
      fit->Draw("SAME");

      // komponentit näkyviin
      TF1* g1 = new TF1((projName + "_g1").c_str(), "gaus", 0.0, 2.5);
      TF1* g2 = new TF1((projName + "_g2").c_str(), "gaus", 0.0, 2.5);
      TF1* g3 = new TF1((projName + "_g3").c_str(), "gaus", 0.0, 2.5);
      g1->SetParameters(fit->GetParameter(0), fit->GetParameter(1), fit->GetParameter(2));
      g2->SetParameters(fit->GetParameter(3), fit->GetParameter(4), fit->GetParameter(5));
      g3->SetParameters(fit->GetParameter(6), fit->GetParameter(7), fit->GetParameter(8));
      g1->SetLineStyle(2); g1->SetLineColor(kGreen + 2);   g1->Draw("same");
      g2->SetLineStyle(3); g2->SetLineColor(kMagenta + 1); g2->Draw("same");
      g3->SetLineStyle(4); g3->SetLineColor(kOrange + 7);  g3->Draw("same");

      // tallennus
      c->SaveAs((outdir_case + projName + ".png").c_str());
      c->Write();

      // siivous
      delete g1; delete g2; delete g3;
      delete fit;
      delete c;
      delete hproj;
    }

    h3->GetYaxis()->UnZoom();
    h3->GetZaxis()->UnZoom();
  }
}



// ----------------------------
// Yhdistetty E/p per hadronityyppi (kaikki p), kaikki caset
// ----------------------------

gStyle->SetOptFit(111);

for (const auto& cs : cases) {
  // kuvien alikansio per case
  std::string outdir_case = outdir + "perTypeAllPt_" + cs.tag + "/";
  gSystem->mkdir(outdir_case.c_str(), true);

  for (const auto& [type, label] : hadronTypesWithLabel) {
    // hae oikea 3D-histo tälle caselle ja hadronityypille
    std::string hname = cs.prefix + type + cs.suffix;
    TH3D* h3 = dynamic_cast<TH3D*>(file->Get(hname.c_str()));
    if (!h3) {
      std::cout << "[WARN] Missing TH3: " << hname << " in " << filename << std::endl;
      continue;
    }

    // Y-alue (HCAL E/Eraw): cut-caselle 0.9–2.5, muuten koko akseli
    int y_min = h3->GetYaxis()->GetFirst();
    int y_max = h3->GetYaxis()->GetLast();
    if (cs.useHcalCut) {
      y_min = h3->GetYaxis()->FindBin(0.9001);
      y_max = h3->GetYaxis()->FindBin(2.4999);
    }

    // Z-alue: kaikki p
    int z_min = h3->GetZaxis()->GetFirst();
    int z_max = h3->GetZaxis()->GetLast();

    // projektio X:lle (E/p)
    std::string projName = "proj_ep_" + label + "_allpt_" + cs.tag;
    TH1D* hproj = h3->ProjectionX(projName.c_str(), y_min, y_max, z_min, z_max);
    if (!hproj || hproj->Integral() == 0) { delete hproj; continue; }

    hproj->Scale(1.0 / hproj->Integral());

    TCanvas* c = new TCanvas(("canvas_" + projName).c_str(), "", 800, 600);
    hproj->SetTitle(Form("E/p (%s, %s), all p", label.c_str(), cs.tag.c_str()));
    hproj->GetXaxis()->SetTitle("E/p");
    hproj->GetYaxis()->SetTitle("Fraction of particles");
    hproj->SetLineColor(kBlue + 2);
    hproj->SetLineWidth(2);

    // 3-Gauss -fitti (kolmas rajoitettu)
    TF1* fit = new TF1(("fit_" + projName).c_str(), "gaus(0)+gaus(3)+gaus(6)", 0.0, 2.5);
    fit->SetParameters(0.2, 0.8, 0.1,  0.1, 1.0, 0.1,  0.01, 1.4, 0.25);
    fit->SetParLimits(6, 0.0, 0.2);   // amp3
    fit->SetParLimits(7, 1.2, 1.7);   // mean3
    fit->SetParLimits(8, 0.1, 0.5);   // sigma3
    fit->SetLineColor(kRed);
    fit->SetLineWidth(2);

    hproj->Fit(fit, "RQ");
    hproj->Draw("HIST");
    fit->Draw("SAME");

    // komponentit näkyviin
    TF1* g1 = new TF1((projName + "_g1").c_str(), "gaus", 0.0, 2.5);
    TF1* g2 = new TF1((projName + "_g2").c_str(), "gaus", 0.0, 2.5);
    TF1* g3 = new TF1((projName + "_g3").c_str(), "gaus", 0.0, 2.5);
    g1->SetParameters(fit->GetParameter(0), fit->GetParameter(1), fit->GetParameter(2));
    g2->SetParameters(fit->GetParameter(3), fit->GetParameter(4), fit->GetParameter(5));
    g3->SetParameters(fit->GetParameter(6), fit->GetParameter(7), fit->GetParameter(8));
    g1->SetLineStyle(2); g1->SetLineColor(kGreen + 2);   g1->Draw("same");
    g2->SetLineStyle(3); g2->SetLineColor(kMagenta + 1); g2->Draw("same");
    g3->SetLineStyle(4); g3->SetLineColor(kOrange + 7);  g3->Draw("same");

    // tallennus
    c->SaveAs((outdir_case + projName + ".png").c_str());
    c->Write();

    // siivous
    delete g1; delete g2; delete g3;
    delete fit;
    delete c;
    delete hproj;
  }
}

// -------------------------------
// STACKED E/p DISTRIBUTIONS FOR EACH p BIN
// -------------------------------

// p-binit (tämä täsmää run_histograms.cc:ssä käytettyihin)
const double trkPBins[] = {
  3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
  7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
  16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
};
const int nTrkPBins = sizeof(trkPBins) / sizeof(double) - 1;

// Varmistetaan että 3D-histot (default E/p + HCAL cut) on olemassa
TH3D* h3_H   = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadH"));
TH3D* h3_E   = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadE"));
TH3D* h3_MIP = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadMIP"));
TH3D* h3_EH  = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadEH"));

if (h3_H && h3_E && h3_MIP && h3_EH) {
  // Y: 0.9 < E_HCAL/E_HCAL^raw < 2.5
  const int y_min = h3_H->GetYaxis()->FindBin(0.9001);
  const int y_max = h3_H->GetYaxis()->FindBin(2.4999);

  for (int i = 0; i < nTrkPBins; ++i) {
    const double pmin = trkPBins[i];
    const double pmax = trkPBins[i+1];

    const int z_min = h3_H->GetZaxis()->FindBin(pmin + 1e-3);
    const int z_max = h3_H->GetZaxis()->FindBin(pmax - 1e-3);

    // Projektio X:lle (E/p) jokaiselle tyypille
    TH1D* h_H_proj   = h3_H  ? h3_H ->ProjectionX(Form("h_H_%d",   i), y_min, y_max, z_min, z_max) : nullptr;
    TH1D* h_E_proj   = h3_E  ? h3_E ->ProjectionX(Form("h_E_%d",   i), y_min, y_max, z_min, z_max) : nullptr;
    TH1D* h_MIP_proj = h3_MIP? h3_MIP->ProjectionX(Form("h_MIP_%d", i), y_min, y_max, z_min, z_max) : nullptr;
    TH1D* h_EH_proj  = h3_EH ? h3_EH->ProjectionX(Form("h_EH_%d",  i), y_min, y_max, z_min, z_max) : nullptr;

    // Kokonaissumma normalisointia varten
    double total = 0.0;
    if (h_H_proj)   total += h_H_proj->Integral();
    if (h_E_proj)   total += h_E_proj->Integral();
    if (h_MIP_proj) total += h_MIP_proj->Integral();
    if (h_EH_proj)  total += h_EH_proj->Integral();
    if (total <= 0) continue;

    // THStack
    THStack* stack = new THStack(Form("stack_%d", i),
      Form("E/p stack, %.1f - %.1f GeV;E/p;Fraction", pmin, pmax));

    // Lisää komponentit pinoon skaalattuna fraktioiksi
    struct Comp { TH1D* h; Color_t col; const char* lab; };
    std::vector<Comp> comps = {
      {h_MIP_proj, kGreen+2,  "MIP"},
      {h_H_proj,   kRed+1,    "HCAL only"},
      {h_E_proj,   kBlue+1,   "ECAL only"},
      {h_EH_proj,  kMagenta+2,"ECAL+HCAL"}
    };

    TLegend* leg = new TLegend(0.68, 0.62, 0.88, 0.88);
    leg->AddEntry((TObject*)nullptr, Form("p = %.1f-%.1f GeV", pmin, pmax), "");

    for (auto& c : comps) {
      if (!c.h || c.h->Integral() == 0) continue;
      c.h->Scale(1.0 / total);
      c.h->SetFillColor(c.col);
      c.h->SetLineColor(kBlack);
      c.h->SetLineWidth(1);
      stack->Add(c.h);
      leg->AddEntry(c.h, c.lab, "f");
    }

    // Piirto
    TCanvas* c = new TCanvas(Form("canvas_stack_%d", i), "", 900, 650);
    stack->Draw("HIST");
    // Aseta akselit piirtämisen jälkeen (THStack luo akselit vasta Draw:n yhteydessä)
    if (stack->GetXaxis()) stack->GetXaxis()->SetTitle("E/p");
    if (stack->GetYaxis()) { stack->GetYaxis()->SetTitle("Fraction"); stack->SetMinimum(0.0); }
    leg->Draw();

    // Piirrä ja tallenna kuva
    // Luo alakansio "stacked" jos sitä ei ole vielä
    std::string stackedDir = outdir + "stacked/";
    gSystem->mkdir(stackedDir.c_str(), true);

    // Tallenna kuva alakansioon
    c->SaveAs((stackedDir + Form("stack_ep_bin_%d.png", i)).c_str());
    c->Write();


    // Valinnainen: fittaa summamuoto (komponenttien summa)
    TH1D* h_sum = nullptr;
    for (auto& cpt : comps) {
      if (!cpt.h) continue;
      if (!h_sum) h_sum = (TH1D*)cpt.h->Clone(Form("h_sum_%d", i));
      else        h_sum->Add(cpt.h);
    }
    if (h_sum && h_sum->Integral() > 0) {
      TCanvas* c_fit = new TCanvas(Form("c_fit_%d", i), "", 900, 650);
      h_sum->SetTitle(Form("Fit to stacked E/p, %.1f - %.1f GeV;E/p;Fraction", pmin, pmax));
      h_sum->SetLineColor(kBlack);
      h_sum->SetLineWidth(2);
      TF1* f = new TF1(Form("f_sum_%d", i), "gaus(0)", 0.3, 2.0);
      f->SetLineColor(kRed+1);
      f->SetLineWidth(2);
      h_sum->Fit(f, "RQ");
      h_sum->Draw("HIST");
      f->Draw("SAME");
      std::string fitsDir = outdir + "fits/";
      gSystem->mkdir(fitsDir.c_str(), true);

      c_fit->SaveAs((fitsDir + Form("fit_stack_bin_%d.png", i)).c_str());
      c_fit->Write();

      delete f;
      delete c_fit;
    }

    delete leg;
    delete c;
  }

  // Palauta mahdolliset zoomaukset
  h3_H->GetYaxis()->UnZoom(); h3_H->GetZaxis()->UnZoom();
  h3_E->GetYaxis()->UnZoom(); h3_E->GetZaxis()->UnZoom();
  h3_MIP->GetYaxis()->UnZoom(); h3_MIP->GetZaxis()->UnZoom();
  h3_EH->GetYaxis()->UnZoom(); h3_EH->GetZaxis()->UnZoom();
}


// ----------------------------
// HADRON FRACTIONS vs p (no cut ja/tai HCAL cut)
// ----------------------------

auto drawFractions = [&](const char* suffix, const char* tagInName){
  // yritä hakea TProfilet tällä suffiksilla
  TProfile* pH   = dynamic_cast<TProfile*>(file->Get(Form("h_frac_H_%s",   suffix)));
  TProfile* pE   = dynamic_cast<TProfile*>(file->Get(Form("h_frac_E_%s",   suffix)));
  TProfile* pMIP = dynamic_cast<TProfile*>(file->Get(Form("h_frac_MIP_%s", suffix)));
  TProfile* pEH  = dynamic_cast<TProfile*>(file->Get(Form("h_frac_EH_%s",  suffix)));
  if (!(pH && pE && pMIP && pEH)) return false;

  TCanvas* c = new TCanvas(Form("c_frac_%s", suffix), "", 900, 650);

  // piirtojärjestys ja tyyli
  pH->SetLineColor(kRed+1);     pH->SetLineWidth(2);
  pE->SetLineColor(kBlue+1);    pE->SetLineWidth(2);
  pMIP->SetLineColor(kGreen+2); pMIP->SetLineWidth(2);
  pEH->SetLineColor(kMagenta+2);pEH->SetLineWidth(2);

  pH->SetTitle(Form("Hadron type fraction vs p (%s);p (GeV);fraction", tagInName));
  pH->SetMinimum(0.0); pH->SetMaximum(1.05);

  pH->Draw("E1");
  pE->Draw("E1 SAME");
  pMIP->Draw("E1 SAME");
  pEH->Draw("E1 SAME");

  TLegend* leg = new TLegend(0.15, 0.68, 0.55, 0.88);
  leg->SetTextSize(0.035);
  leg->AddEntry(pH,   "HCAL only", "l");
  leg->AddEntry(pE,   "ECAL only", "l");
  leg->AddEntry(pMIP, "MIP",       "l");
  leg->AddEntry(pEH,  "ECAL+HCAL", "l");
  leg->Draw();

  std::string fracDir = outdir + "fractions/";
  gSystem->mkdir(fracDir.c_str(), true);

  c->SaveAs((fracDir + Form("fraction_per_type_vs_pT_%s.png", suffix)).c_str());
  c->Write();


  delete leg;
  delete c;
  return true;
};

// Piirrä HCAL-cut-versio, ja jos ei löydy, piirrä no-cut
bool drew_cut = drawFractions("cut", "HCAL E/E_{raw} > 0.9");
bool drew_all = drawFractions("all", "no cut");
if (!drew_cut && !drew_all) {
  std::cout << "[WARN] No hadron fraction profiles found (neither *_cut nor *_all)." << std::endl;
}


// -------------------------------
// HADRONITYYPPIEN FRAKTIOT: PINOTTU HISTOGRAMMI
// -------------------------------
std::cout << "Drawing stacked hadron fraction plot..." << std::endl;

// yritä ensin HCAL-cut profiileja, muuten no-cut
auto getProf = [&](const char* name)->TProfile* {
  return dynamic_cast<TProfile*>(file->Get(name));
};

const char* suffix = nullptr;
TProfile *pH=nullptr,*pE=nullptr,*pMIP=nullptr,*pEH=nullptr;

// 1) HCAL cut
pH   = getProf("h_frac_H_cut");
pE   = getProf("h_frac_E_cut");
pMIP = getProf("h_frac_MIP_cut");
pEH  = getProf("h_frac_EH_cut");
if (pH && pE && pMIP && pEH) {
  suffix = "cut";
} else {
  // 2) no cut
  pH   = getProf("h_frac_H_all");
  pE   = getProf("h_frac_E_all");
  pMIP = getProf("h_frac_MIP_all");
  pEH  = getProf("h_frac_EH_all");
  if (pH && pE && pMIP && pEH) suffix = "all";
}

if (!suffix) {
  std::cout << "[WARN] Could not find fraction profiles (*_cut nor *_all). Skipping stacked fraction plot.\n";
} else {
  // luodaan TH1D:t samoilla p-bineillä kuin analyysissä
  const double trkPBins[] = {
    3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
    7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
    16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
  };
  const int nTrkPBins = sizeof(trkPBins)/sizeof(double) - 1;

  auto makeH = [&](const char* n)->TH1D*{
    TH1D* h = new TH1D(n, "Hadron fractions;p (GeV);Fraction", nTrkPBins, trkPBins);
    h->SetDirectory(nullptr);
    return h;
  };

  TH1D* h_H_bin   = makeH("h_H_bin");
  TH1D* h_E_bin   = makeH("h_E_bin");
  TH1D* h_MIP_bin = makeH("h_MIP_bin");
  TH1D* h_EH_bin  = makeH("h_EH_bin");

  // kopioi binikohdat TProfileista → TH1D
  for (int i = 1; i <= nTrkPBins; ++i) {
    h_H_bin  ->SetBinContent(i, pH  ->GetBinContent(i));
    h_E_bin  ->SetBinContent(i, pE  ->GetBinContent(i));
    h_MIP_bin->SetBinContent(i, pMIP->GetBinContent(i));
    h_EH_bin ->SetBinContent(i, pEH ->GetBinContent(i));
  }

  // värit linjassa muun koodin kanssa
  h_H_bin  ->SetFillColor(kRed+1);     h_H_bin  ->SetLineColor(kBlack);
  h_E_bin  ->SetFillColor(kBlue+1);    h_E_bin  ->SetLineColor(kBlack);
  h_MIP_bin->SetFillColor(kGreen+2);   h_MIP_bin->SetLineColor(kBlack);
  h_EH_bin ->SetFillColor(kMagenta+2); h_EH_bin ->SetLineColor(kBlack);

  THStack* hs = new THStack("hs",
    Form("Hadron type fraction vs p (%s);Track momentum p (GeV);Fraction",
         (std::string(suffix)=="cut" ? "HCAL E/E_{raw} > 0.9" : "no cut")));

  // pinoamisjärjestys: valitse haluamasi – tämä toimii nätisti
  hs->Add(h_MIP_bin);
  hs->Add(h_EH_bin);
  hs->Add(h_E_bin);
  hs->Add(h_H_bin);

  TCanvas* c_stack = new TCanvas(Form("c_stack_frac_%s", suffix), "", 900, 700);
  c_stack->SetLogx();
  hs->Draw("hist");
  hs->SetMinimum(0.0);
  hs->SetMaximum(1.05);
  if (hs->GetXaxis()) hs->GetXaxis()->SetTitleSize(0.045);
  if (hs->GetYaxis()) hs->GetYaxis()->SetTitleSize(0.045);

  TLegend* leg_stack = new TLegend(0.70, 0.65, 0.88, 0.88);
  leg_stack->AddEntry(h_H_bin,   "HCAL only",  "f");
  leg_stack->AddEntry(h_E_bin,   "ECAL only",  "f");
  leg_stack->AddEntry(h_MIP_bin, "MIP",        "f");
  leg_stack->AddEntry(h_EH_bin,  "ECAL+HCAL",  "f");
  leg_stack->Draw();

  // huom. outdir jo sisältää /-lopun koodissasi; älä lisää toista
  std::string fracDir = outdir + "fractions/";
  gSystem->mkdir(fracDir.c_str(), true);

  c_stack->SaveAs((fracDir + Form("fraction_stacked_%s.png", suffix)).c_str());
  c_stack->Write();


  delete leg_stack;
  delete c_stack;
}

// Suljetaan ROOT-tiedostot (ok)
file->Close();
}

// --------------------------------
// E/p MC vs Data: hae 1D valmiina tai projisoi 2D:stä,
// normalisoi, fittaa Crystal Ball, tallenna kuva
// --------------------------------
void plot_ep_overlay() {
  // ---- Valitse vertailu (vaihda näitä tarvittaessa) ----
  constexpr bool useDefaultEp = true;   // true = Default (S2/S4), false = Raw (S1/S3)
  constexpr bool useHCALCut   = false;  // true = HCAL cut (S1/S2),  false = no cut (S3/S4)

  // Tiedostot (määritelty aiemmin ylhäällä)
  const char* mc_path   = kFileMC;   // "histograms/SingleNeutrino2024.root"
  const char* data_path = kFileData; // "histograms/ZeroBias2024_All.root"

  // 1D-avaimet: default vs raw
  const char* h1_key = useDefaultEp ? "h_ep_iso" : "h_raw_ep_iso";

  // 2D-avaimet: S1–S4 (X=p, Y=E/p)
  const char* h2_key =
    useDefaultEp
      ? (useHCALCut ? "h2_ep_vs_p_S2_def_cut" : "h2_ep_vs_p_S4_def_all")
      : (useHCALCut ? "h2_ep_vs_p_S1_raw_cut" : "h2_ep_vs_p_S3_raw_all");

  // ulostulohakemisto
  std::string outdir = "plots2/";
  gSystem->mkdir(outdir.c_str(), true);

  // ---- Avaa tiedostot ----
  TFile* f_mc   = TFile::Open(mc_path,   "READ");
  TFile* f_data = TFile::Open(data_path, "READ");
  if (!f_mc || !f_data || f_mc->IsZombie() || f_data->IsZombie()) {
    std::cerr << "ERROR: Could not open input ROOT files." << std::endl;
    if (f_mc)   f_mc->Close();
    if (f_data) f_data->Close();
    return;
  }

  auto fetch1D = [&](TFile* f) -> TH1D* {
    if (auto* h1 = dynamic_cast<TH1D*>(f->Get(h1_key))) {
      gROOT->cd(); // irrota tiedostosta
      auto* c = (TH1D*)h1->Clone(Form("%s_clone_%p", h1_key, (void*)f));
      return c;
    }
    return nullptr;
  };

  auto build1Dfrom2D = [&](TFile* f, const char* key2d, const char* name) -> TH1D* {
    auto* h2 = dynamic_cast<TH2D*>(f->Get(key2d));
    if (!h2) return nullptr;
    // Projisoi E/p (Y) koko p-alueen (X) yli
    int xfirst = 1, xlast = h2->GetXaxis()->GetNbins();
    gROOT->cd();
    return h2->ProjectionY(name, xfirst, xlast);
  };

  // ---- Hae/johda 1D-jakaumat ----
  TH1D* h_mc   = fetch1D(f_mc);
  TH1D* h_data = fetch1D(f_data);

  if (!h_mc)   h_mc   = build1Dfrom2D(f_mc,   h2_key, "h_ep_overlay_mc");
  if (!h_data) h_data = build1Dfrom2D(f_data, h2_key, "h_ep_overlay_data");

  if (!h_mc || !h_data) {
    std::cerr << "ERROR: Could not obtain 1D E/p for MC or Data (checked "
              << h1_key << " and " << h2_key << ")." << std::endl;
    if (f_mc)   f_mc->Close();
    if (f_data) f_data->Close();
    return;
  }

  // rajat
  h_mc  ->GetXaxis()->SetRangeUser(0.05, 2.5);
  h_data->GetXaxis()->SetRangeUser(0.05, 2.5);

  // ---- Normalisoi muotoon ----
  if (h_mc->Integral()   > 0) h_mc  ->Scale(1.0 / h_mc->Integral());
  if (h_data->Integral() > 0) h_data->Scale(1.0 / h_data->Integral());

  // tyyli
  h_mc->SetLineColor(kBlack);  h_mc->SetLineWidth(2);  h_mc->SetLineStyle(1);
  h_data->SetLineColor(kBlack);h_data->SetLineWidth(2);h_data->SetLineStyle(2);

  std::string titleTag = useDefaultEp ? "Default E/p" : "Raw E/p";
  titleTag += useHCALCut ? " (HCAL E/E_{raw}>0.9)" : " (no cut)";
  h_mc->SetTitle((titleTag + ";E/p;Normalized entries").c_str());

  // ---- Crystal Ball -fitit ----
  TF1* fit_mc   = new TF1("fit_mc",   "crystalball", 0.05, 2.5);
  TF1* fit_data = new TF1("fit_data", "crystalball", 0.05, 2.5);
  // [0]=norm, [1]=mean, [2]=sigma, [3]=alpha, [4]=n
  fit_mc  ->SetParameters(1.0, 0.95, 0.12, 1.5, 3.0);
  fit_data->SetParameters(1.0, 0.95, 0.12, 1.5, 3.0);
  fit_mc  ->SetParLimits(1, 0.6, 1.3);  fit_mc  ->SetParLimits(2, 0.03, 0.5);
  fit_data->SetParLimits(1, 0.6, 1.3);  fit_data->SetParLimits(2, 0.03, 0.5);

  h_mc  ->Fit(fit_mc,   "RQ0");
  h_data->Fit(fit_data, "RQ0");

  const double peak_mc    = fit_mc  ->GetParameter(1);
  const double sigma_mc   = fit_mc  ->GetParameter(2);
  const double peak_data  = fit_data->GetParameter(1);
  const double sigma_data = fit_data->GetParameter(2);
  const double scale      = (peak_mc != 0.0) ? (peak_data/peak_mc) : std::numeric_limits<double>::quiet_NaN();

  std::cout << "=== E/p peak fit results (Crystal Ball) ===\n"
            << "MC:   peak = " << peak_mc   << ", sigma = " << sigma_mc   << "\n"
            << "Data: peak = " << peak_data << ", sigma = " << sigma_data << "\n"
            << "Scale factor (data/MC): " << scale << std::endl;

  // ---- Piirto ----
  TCanvas* c = new TCanvas("c_ep_overlay", "E/p MC vs Data", 900, 650);
  const double ymax = 1.2 * std::max(h_mc->GetMaximum(), h_data->GetMaximum());
  h_mc->GetYaxis()->SetRangeUser(0.0, ymax);
  h_mc->Draw("HIST");
  h_data->Draw("HIST SAME");

  fit_mc  ->SetLineColor(kRed + 1);   fit_mc  ->SetLineWidth(2); fit_mc  ->SetLineStyle(1);  fit_mc  ->Draw("SAME");
  fit_data->SetLineColor(kGreen + 2); fit_data->SetLineWidth(2); fit_data->SetLineStyle(2);  fit_data->Draw("SAME");

  auto leg = new TLegend(0.55, 0.70, 0.88, 0.88);
  leg->SetBorderSize(0);
  leg->AddEntry(h_mc,   "SingleNeutrino (MC)", "l");
  leg->AddEntry(h_data, "ZeroBias (Data)",     "l");
  leg->AddEntry(fit_mc,   "MC fit (Crystal Ball)",   "l");
  leg->AddEntry(fit_data, "Data fit (Crystal Ball)", "l");
  leg->Draw();

  TLatex latex; latex.SetNDC(); latex.SetTextSize(0.032);
  latex.DrawLatex(0.60, 0.52, Form("MC peak   = %.3f", peak_mc));
  latex.DrawLatex(0.60, 0.47, Form("Data peak = %.3f", peak_data));
  latex.DrawLatex(0.60, 0.42, Form("Scale     = %.3f", scale));

  // ulostulonimi tageista
  std::string tagEp  = useDefaultEp ? "def"  : "raw";
  std::string tagCut = useHCALCut   ? "cut"  : "nocut";
  std::string overlayDir = outdir + "overlay/";
  gSystem->mkdir(overlayDir.c_str(), true);

  std::string png = overlayDir + "ep_overlay_mc_vs_data_" + tagEp + "_" + tagCut + "_fitCB.png";
  c->SaveAs(png.c_str());


  // siivous
  delete leg;
  delete c;
  f_mc->Close();
  f_data->Close();
}

void plot_stacked_comparison_raw(bool useHCALCut) {
  // Valitse “no cut” (all) vai HCAL cut (>0.9)
  
  // Tiedostot (samat kuin muualla)
  TFile* f_mc   = TFile::Open(kFileMC,   "READ");   // "histograms/SingleNeutrino2024.root"
  TFile* f_data = TFile::Open(kFileData, "READ");   // "histograms/ZeroBias2024_All.root"
  if (!f_mc || !f_data || f_mc->IsZombie() || f_data->IsZombie()) {
    std::cerr << "[plot_stacked_comparison_raw] ERROR: cannot open MC/Data files.\n";
    if (f_mc)   f_mc->Close();
    if (f_data) f_data->Close();
    return;
  }

  // Profiilien avaimet
  const char* suf = useHCALCut ? "_cut" : "_all";
  auto getProf = [&](TFile* f, const char* base)->TProfile* {
    return dynamic_cast<TProfile*>(f->Get((std::string(base) + suf).c_str()));
  };

  // MC-profiilit
  TProfile* h_H_mc   = getProf(f_mc,   "h_frac_H");
  TProfile* h_E_mc   = getProf(f_mc,   "h_frac_E");
  TProfile* h_MIP_mc = getProf(f_mc,   "h_frac_MIP");
  TProfile* h_EH_mc  = getProf(f_mc,   "h_frac_EH");

  // Data-profiilit
  TProfile* h_H_dt   = getProf(f_data, "h_frac_H");
  TProfile* h_E_dt   = getProf(f_data, "h_frac_E");
  TProfile* h_MIP_dt = getProf(f_data, "h_frac_MIP");
  TProfile* h_EH_dt  = getProf(f_data, "h_frac_EH");

  if (!h_H_mc || !h_E_mc || !h_MIP_mc || !h_EH_mc ||
      !h_H_dt || !h_E_dt || !h_MIP_dt || !h_EH_dt) {
    std::cerr << "[plot_stacked_comparison_raw] ERROR: missing fraction profiles "
                 "(expected h_frac_*" << suf << "). Did you run run_histograms?\n";
    f_mc->Close(); f_data->Close();
    return;
  }

  // Muunna TProfile -> TH1D MC-pinoutia varten (kopioi binisisällöt ja -virheet)
 auto profToHist = [](TProfile* p, const char* hname)->TH1D* {
  const TArrayD* xb = p->GetXaxis()->GetXbins();
  TH1D* h = nullptr;
  if (xb && xb->GetSize() > 0) {
    h = new TH1D(hname, p->GetTitle(), p->GetNbinsX(), xb->GetArray());
  } else {
    h = new TH1D(hname, p->GetTitle(), p->GetNbinsX(),
                 p->GetXaxis()->GetXmin(), p->GetXaxis()->GetXmax());
  }
  for (int i = 1; i <= p->GetNbinsX(); ++i) {
    h->SetBinContent(i, p->GetBinContent(i));
    h->SetBinError  (i, p->GetBinError(i));
  }
  h->GetXaxis()->SetTitle(p->GetXaxis()->GetTitle());
  h->GetYaxis()->SetTitle("Fraction");
  return h;
};


  TH1D* h_H_bin_mc   = profToHist(h_H_mc,   "h_H_bin_mc");
  TH1D* h_E_bin_mc   = profToHist(h_E_mc,   "h_E_bin_mc");
  TH1D* h_MIP_bin_mc = profToHist(h_MIP_mc, "h_MIP_bin_mc");
  TH1D* h_EH_bin_mc  = profToHist(h_EH_mc,  "h_EH_bin_mc");

  // Värit MC:lle (pidä samat kuin muualla projektissa)
  h_H_bin_mc  ->SetFillColor(kRed+1);
  h_E_bin_mc  ->SetFillColor(kBlue+1);
  h_MIP_bin_mc->SetFillColor(kMagenta+2);
  h_EH_bin_mc ->SetFillColor(kGreen+2);

  // Luo pinottu histogrammi MC:lle
  THStack* stack_mc = new THStack("stack_frac",
    (std::string("Hadron type fraction ") + (useHCALCut ? "(HCAL E/E_{raw}>0.9)" : "(no cut)")
     + ";Track momentum p (GeV);Fraction").c_str());

  // Piirtojärjestys pohjalta ylöspäin
  stack_mc->Add(h_E_bin_mc);
  stack_mc->Add(h_MIP_bin_mc);
  stack_mc->Add(h_EH_bin_mc);
  stack_mc->Add(h_H_bin_mc);

  // Luo data-markerit kumulatiivisina yläreunoina (ja kumulatiiviset virheet)
  const int nb = h_H_dt->GetNbinsX();
  auto makeCumGraph = [&](int markerStyle)->TGraphErrors* {
    auto* g = new TGraphErrors(nb);
    g->SetMarkerStyle(markerStyle);
    g->SetMarkerColor(kBlack);
    g->SetLineColor(kBlack);
    return g;
  };

  TGraphErrors* g_H   = makeCumGraph(20);
  TGraphErrors* g_MIP = makeCumGraph(21);
  TGraphErrors* g_EH  = makeCumGraph(22);
  TGraphErrors* g_E   = makeCumGraph(23);

  for (int i=1;i<=nb;++i) {
    const double x   = h_H_dt->GetBinCenter(i);

    const double yH   = h_H_dt->GetBinContent(i);
    const double yMIP = yH   + h_MIP_dt->GetBinContent(i);
    const double yEH  = yMIP + h_EH_dt->GetBinContent(i);
    const double yE   = yEH  + h_E_dt->GetBinContent(i);

    const double eH   = h_H_dt->GetBinError(i);
    const double eMIP = std::hypot(eH,  h_MIP_dt->GetBinError(i));
    const double eEH  = std::hypot(eMIP,h_EH_dt ->GetBinError(i));
    const double eE   = std::hypot(eEH, h_E_dt  ->GetBinError(i));

    g_H  ->SetPoint(i-1, x, yH);   g_H  ->SetPointError(i-1, 0.0, eH);
    g_MIP->SetPoint(i-1, x, yMIP); g_MIP->SetPointError(i-1, 0.0, eMIP);
    g_EH ->SetPoint(i-1, x, yEH);  g_EH ->SetPointError(i-1, 0.0, eEH);
    g_E  ->SetPoint(i-1, x, yE);   g_E  ->SetPointError(i-1, 0.0, eE);
  }

  // Piirto
  std::string outdir = "plots2/";
  gSystem->mkdir(outdir.c_str(), true);

  TCanvas* c = new TCanvas("c_compare_frac", "Hadron type fraction", 900, 650);
  c->SetLogx();
  stack_mc->Draw("HIST");
  stack_mc->SetMinimum(0.0);
  stack_mc->SetMaximum(1.05);

  // Piirrä data-markkerit
  g_H  ->Draw("P SAME");
  g_MIP->Draw("P SAME");
  g_EH ->Draw("P SAME");
  g_E  ->Draw("P SAME");

  // Legenda
  auto leg = new TLegend(0.52, 0.16, 0.87, 0.44);
  leg->SetBorderSize(0);
  leg->AddEntry((TObject*)nullptr, "MC: stacked", "");
  // HUOM: viittaa suoraan lisättyihin histoihin (järjestys yllä)
  leg->AddEntry(h_H_bin_mc,   "HCAL only",  "f");
  leg->AddEntry(h_E_bin_mc,   "ECAL only",  "f");
  leg->AddEntry(h_MIP_bin_mc, "MIP",        "f");
  leg->AddEntry(h_EH_bin_mc,  "ECAL+HCAL",  "f");
  leg->AddEntry((TObject*)nullptr, "Data: markers", "");
  leg->AddEntry(g_H,   "HCAL only (cum.)", "p");
  leg->AddEntry(g_MIP, "MIP (cum.)",       "p");
  leg->AddEntry(g_EH,  "ECAL+HCAL (cum.)", "p");
  leg->AddEntry(g_E,   "ECAL only (cum.)", "p");
  leg->Draw();

  // Talleta
  std::string tag = useHCALCut ? "cut" : "nocut";
  std::string fracDir = outdir + "fractions/";
  gSystem->mkdir(fracDir.c_str(), true);

  c->SaveAs((fracDir + "stack_frac_comparison_" + tag + ".png").c_str());


  // Siivous
  delete leg;
  delete c;
  f_mc->Close();
  f_data->Close();
}


void compile_response_pages(const char* inpath,
                            const char* outpdf,
                            const std::vector<std::pair<double,double>>& pBins)
{
  gStyle->SetOptStat(0);

  // Avaa input
  std::unique_ptr<TFile> f(TFile::Open(inpath, "READ"));
  if (!f || f->IsZombie()) {
    std::cerr << "[compile_response_pages] Cannot open " << inpath << "\n";
    return;
  }

  // 4 skenaariota (kolumnit)
  struct Scn {
    const char* colTitle;  // otsikko ylös
    const char* h2;        // ALL-histo (2D), X = p, Y = E/p
    const char* h3prefix;  // HUOM: ei "isHad" tässä!
    const char* h3suffix;  // loppuosa per-tyyppi-3D:n nimeen
    bool useHcalCut;       // rajoitetaanko Y 0.9–2.5
  } scn[4] = {
    // S1: Raw E/p, HCAL-cut -> nimet: h3_resp_raw_p_isHad*_cut
    {"Raw/p, HCAL cut",       "h2_ep_vs_p_S1_raw_cut",  "h3_resp_raw_p_",  "_cut", true },

    // S2: Default E/p, HCAL-cut -> nimet: h3_resp_corr_p_isHad*  (EI _cut/_all -päätettä!)
    {"Default/p, HCAL cut",   "h2_ep_vs_p_S2_def_cut",  "h3_resp_corr_p_", "",     true },

    // S3: Raw E/p, no cut -> nimet: h3_resp_raw_p_isHad*_all
    {"Raw/p, no cut",         "h2_ep_vs_p_S3_raw_all",  "h3_resp_raw_p_",  "_all", false},

    // S4: Default E/p, no cut -> nimet: h3_resp_def_p_isHad*_all
    {"Default/p, no cut",     "h2_ep_vs_p_S4_def_all",  "h3_resp_def_p_",  "_all", false}
  };

// Rivit: ALL + 4 kategoriaa (näissä "isHad*" säilyy)
struct TypeRow { const char* key; const char* label; Color_t col; } rows[5] = {
  {"",        "ALL",        kBlack     }, // ALL-rivi käyttää 2D:tä (h2)
  {"isHadH",  "HCAL only",  kRed+1     },
  {"isHadE",  "ECAL only",  kBlue+1    },
  {"isHadMIP","MIP",        kGreen+2   },
  {"isHadEH", "ECAL+HCAL",  kMagenta+2 }
};



  // Luo kansio outpdf:n polusta
  std::string pdfPath = outpdf;
  const char* outDir = gSystem->DirName(pdfPath.c_str());
  gSystem->mkdir(outDir, true);

  // Canvas: 5 riviä x 4 kolumnia
  TCanvas c("c_comp", "Response compilation", 2000, 1400);
  c.Divide(4, 5, 0.001, 0.001);

  // Aloita monisivu-PDF
  c.Print((pdfPath + "[").c_str()); // open

  // 1 sivu / p-alue
  for (size_t ip=0; ip<pBins.size(); ++ip) {
    const double pmin = pBins[ip].first;
    const double pmax = pBins[ip].second;

    for (int r=0; r<5; ++r) {
      for (int j=0; j<4; ++j) {
        c.cd(r*4 + (j+1));

        TH1* hdraw = nullptr;

        if (r == 0) {
          // ALL: 2D → projisoi Y (E/p)
          TH2D* h2 = dynamic_cast<TH2D*>(f->Get(scn[j].h2));
          if (!h2) { TLatex t; t.SetNDC(); t.DrawLatex(0.2,0.5,"[missing 2D]"); continue; }
          const int x1 = h2->GetXaxis()->FindBin(pmin+1e-3);
          const int x2 = h2->GetXaxis()->FindBin(pmax-1e-3);
          TH1D* py = h2->ProjectionY(Form("py_%zu_%d_%d", ip, r, j), x1, x2);
          if (!py || py->Integral()==0) { delete py; TLatex t; t.SetNDC(); t.DrawLatex(0.2,0.5,"[empty]"); continue; }
          py->SetDirectory(nullptr);
          py->SetLineColor(rows[r].col);
          py->GetXaxis()->SetTitle("E/p");
          py->GetYaxis()->SetTitle("Entries");
          py->GetXaxis()->SetRangeUser(0.05, 2.5);
          hdraw = py;
        } else {
          // Per-tyyppi: 3D → projisoi X (E/p)
          std::string h3name = std::string(scn[j].h3prefix) + rows[r].key + scn[j].h3suffix;
          TH3D* h3 = dynamic_cast<TH3D*>(f->Get(h3name.c_str()));
          if (!h3) { TLatex t; t.SetNDC(); t.DrawLatex(0.2,0.5,"[missing 3D]"); continue; }

          int y1 = h3->GetYaxis()->GetFirst();
          int y2 = h3->GetYaxis()->GetLast();
          if (scn[j].useHcalCut) {
            y1 = h3->GetYaxis()->FindBin(0.9001);
            y2 = h3->GetYaxis()->FindBin(2.4999);
          }

          const int z1 = h3->GetZaxis()->FindBin(pmin+1e-3);
          const int z2 = h3->GetZaxis()->FindBin(pmax-1e-3);
          TH1D* px = h3->ProjectionX(Form("px_%zu_%d_%d", ip, r, j), y1, y2, z1, z2);
          if (!px || px->Integral()==0) { delete px; TLatex t; t.SetNDC(); t.DrawLatex(0.2,0.5,"[empty]"); continue; }
          px->SetDirectory(nullptr);
          px->SetLineColor(rows[r].col);
          px->GetXaxis()->SetTitle("E/p");
          px->GetYaxis()->SetTitle("Entries");
          px->GetXaxis()->SetRangeUser(0.05, 2.5);
          hdraw = px;
        }

        // Piirto (raaka jakauma, ei norm/fittiä)
        hdraw->SetLineWidth(2);
        hdraw->Draw("HIST");

        // Riviteksti vasemmalle
        if (j==0) {
          TLatex lab; lab.SetNDC(); lab.SetTextSize(0.040);
          lab.DrawLatex(0.10, 0.88, rows[r].label);
        }

        // Kolumniotsikko + p-alue (piirrä lopuksi)
        if (r==0) {
          TLatex t; t.SetNDC(); t.SetTextAlign(22); t.SetTextSize(0.045);
          t.DrawLatex(0.5, 0.94, scn[j].colTitle);
          TLatex ptxt; ptxt.SetNDC(); ptxt.SetTextSize(0.035);
          ptxt.DrawLatex(0.5, 0.88, Form("p = %.1f - %.1f GeV", pmin, pmax));
        }

        // Entries
        TLatex ent; ent.SetNDC(); ent.SetTextSize(0.030);
        ent.DrawLatex(0.78, 0.80, Form("N = %.0f", hdraw->GetEntries()));
      }
    }

    c.Update();
    c.Print(pdfPath.c_str()); // yksi sivu ulos
  }

  // Sulje PDF
  c.Print((pdfPath + "]").c_str());
}

// --- 2D heatmapit log-x:llä + profiilit + monisivuinen PDF ---
void make_h2_maps_logx_pdf(const char* inpath, const char* outdirTag) {
  gStyle->SetPalette(kBird);

  std::unique_ptr<TFile> f(TFile::Open(inpath, "READ"));
  if (!f || f->IsZombie()) {
    std::cerr << "[make_h2_maps_logx_pdf] cannot open " << inpath << "\n";
    return;
  }

  // Ulostulokansio
  std::string outdir = std::string("plots2/") + outdirTag + "/h2_maps_logx";
  gSystem->mkdir(outdir.c_str(), true);

  // PDF-tiedosto
  std::string pdfPath = outdir + "/h2_maps_logx.pdf";
  bool firstPage = true;

  // Skenaariot
  struct Scn {
    const char* tag;
    const char* h2_all;
    const char* h3pref;
    const char* h3suff;
    bool hcalCut;
  } S[4] = {
    {"S1_raw_cut", "h2_ep_vs_p_S1_raw_cut", "h3_resp_raw_p_",  "_cut", true },
    {"S2_def_cut", "h2_ep_vs_p_S2_def_cut", "h3_resp_corr_p_", "",     true },
    {"S3_raw_all", "h2_ep_vs_p_S3_raw_all", "h3_resp_raw_p_",  "_all", false},
    {"S4_def_all", "h2_ep_vs_p_S4_def_all", "h3_resp_def_p_",  "_all", false}
  };

  struct Row {
    const char* key;
    const char* label;
  } R[5] = {
    {"",       "ALL"},
    {"isHadH", "HCAL only"},
    {"isHadE", "ECAL only"},
    {"isHadMIP","MIP"},
    {"isHadEH","ECAL+HCAL"}
  };

  // 1) ALL-kartat
  for (int j = 0; j < 4; ++j) {
    if (auto* h2 = dynamic_cast<TH2D*>(f->Get(S[j].h2_all))) {
      TCanvas c(Form("c_%s_ALL", S[j].tag), "", 900, 750);
      c.SetRightMargin(0.12);
      gPad->SetLogx();
      h2->SetTitle(Form("%s - ALL;p (GeV);E/p", S[j].tag));
      h2->Draw("COLZ");

      // Profiili
      auto* prof = h2->ProfileX(Form("prof_%s_ALL", S[j].tag));
      if (prof) {
        prof->SetLineWidth(2);
        prof->Draw("SAME");
      }

      std::string pdfCmd = pdfPath + (firstPage ? "(" : "");
      c.SaveAs(pdfCmd.c_str());
      firstPage = false;
    }
  }

  // 2) Per-tyyppi kartat
  for (int r = 1; r < 5; ++r) { // skip R[0] = ALL
    for (int j = 0; j < 4; ++j) {
      std::string h3name = std::string(S[j].h3pref) + R[r].key + S[j].h3suff;
      auto* h3 = dynamic_cast<TH3D*>(f->Get(h3name.c_str()));
      if (!h3) continue;

      int y1 = h3->GetYaxis()->GetFirst();
      int y2 = h3->GetYaxis()->GetLast();
      if (S[j].hcalCut) {
        y1 = h3->GetYaxis()->FindBin(0.9001);
        y2 = h3->GetYaxis()->FindBin(2.4999);
      }
      h3->GetYaxis()->SetRange(y1, y2);

      auto* h2 = dynamic_cast<TH2D*>(h3->Project3D("zx")); // p vs E/p
      if (!h2) {
        h3->GetYaxis()->UnZoom();
        continue;
      }

      h2->SetTitle(Form("%s - %s;p (GeV);E/p", S[j].tag, R[r].label));
      TCanvas c(Form("c_%s_%s", S[j].tag, R[r].key), "", 900, 750);
      c.SetRightMargin(0.12);
      gPad->SetLogx();
      h2->Draw("COLZ");

      auto* prof = h2->ProfileX(Form("prof_%s_%s", S[j].tag, R[r].key));
      if (prof) {
        prof->SetLineWidth(2);
        prof->Draw("SAME");
      }

      c.SaveAs(pdfPath.c_str());
      h3->GetYaxis()->UnZoom();
    }
  }

  // Sulje PDF
  {
    TCanvas ctmp;
    std::string closeCmd = pdfPath + ")";
    ctmp.SaveAs(closeCmd.c_str());
  }
}


// --- 2024 vs 2025 ZeroBias -vuosivertailu ---
void compare_zerobias_years(const char* file2024, const char* file2025) {
  // Avaa ROOT-tiedostot
  std::unique_ptr<TFile> f24(TFile::Open(file2024,"READ"));
  std::unique_ptr<TFile> f25(TFile::Open(file2025,"READ"));
  if (!f24 || f24->IsZombie() || !f25 || f25->IsZombie()) {
    std::cerr << "[compare_zerobias_years] virhe: tiedostojen avaus epäonnistui\n";
    return;
  }

  // Profiilien nimet ja selkeät otsikot
  struct PlotCfg {
    std::string key;
    std::string title;
  };
  std::vector<PlotCfg> configs = {
    {"prof_ep_vs_p_S1_raw_cut", "S1: Raw E/p vs p (HCAL cut)"},
    {"prof_ep_vs_p_S2_def_cut", "S2: Default corrected E/p vs p (HCAL cut)"},
    {"prof_ep_vs_p_S4_def_all", "S4: Default corrected E/p vs p (no cut)"}
  };

  // Tee ulostulokansio
  gSystem->mkdir("plots_compare", true);

  // Loopataan profiilit
  for (auto& cfg : configs) {
    auto* h24 = dynamic_cast<TProfile*>(f24->Get(cfg.key.c_str()));
    auto* h25 = dynamic_cast<TProfile*>(f25->Get(cfg.key.c_str()));

    if (!h24 || !h25) {
      std::cerr << "[compare_zerobias_years] histogrammia " << cfg.key << " ei löytynyt\n";
      continue;
    }

    // Muotoilu
    h24->SetLineColor(kBlue); h24->SetLineWidth(2);
    h25->SetLineColor(kRed);  h25->SetLineWidth(2);

    // Luo canvas selkeällä otsikolla
    TCanvas c(("c_"+cfg.key).c_str(), cfg.title.c_str(), 900, 700);
    c.SetLogx();

    h24->SetTitle((cfg.title + ";p (GeV);E/p").c_str());
    h24->Draw("E1");
    h25->Draw("E1 SAME");

    auto leg = new TLegend(0.6,0.7,0.88,0.88);
    leg->AddEntry(h24,"ZeroBias 2024","l");
    leg->AddEntry(h25,"ZeroBias 2025","l");
    leg->Draw();

    // Tallenna tiedosto
    std::string outname = "plots_compare/compare_" + cfg.key + ".png";
    c.SaveAs(outname.c_str());

    std::cout << "[compare_zerobias_years] Saved " << outname << std::endl;
  }
}


void compile_response_pages(const char* inpath,
                            const char* outpdf,
                            const std::vector<std::pair<double,double>>& pBins)
{
  gStyle->SetOptStat(0);


// ----------------------------------------------------------------------
// Piirretään E/p-jakaumat valituille p-alueille (HCAL only, yksittäinen datasetti)
// ----------------------------------------------------------------------
void plot_response_distributions(const char* filePath,
                                 const char* tag,
                                 const std::vector<std::pair<double,double>>& bins) {
  std::unique_ptr<TFile> f(TFile::Open(filePath, "READ"));
  if (!f || f->IsZombie()) {
    std::cerr << "[plot_response_distributions] cannot open " << filePath << "\n";
    return;
  }

  auto* h3 = dynamic_cast<TH3D*>(f->Get("h3_resp_def_p_isHadH_all"));
  if (!h3) {
    std::cerr << "[plot_response_distributions] histogram 'h3_resp_def_p_isHadH_all' not found\n";
    return;
  }

  std::cout << "[INFO] Opened " << filePath << std::endl;
  std::cout << "[INFO] Histogram range in p: " 
            << h3->GetZaxis()->GetXmin() << " - " << h3->GetZaxis()->GetXmax() << " GeV" << std::endl;

  std::string baseplots = "/eos/user/m/mmarjama/my_pion_analysis/plots2025";
  gSystem->mkdir(baseplots.c_str(), true);

  std::string datasetDir = baseplots + "/" + std::string(tag) + "/resp_dists";
  gSystem->mkdir(datasetDir.c_str(), true);

  int idx = 0;
  for (auto& bin : bins) {
    double pmin = bin.first;
    double pmax = bin.second;

    int z1 = h3->GetZaxis()->FindBin(pmin + 1e-3);
    int z2 = h3->GetZaxis()->FindBin(pmax - 1e-3);
    if (z2 <= z1) z2 = z1 + 1; // varmistetaan että projektion alue ei ole tyhjä

    std::cout << "[DEBUG] Projecting p range " << pmin << "–" << pmax 
              << " GeV  (z1=" << z1 << ", z2=" << z2 << ")" << std::endl;

    auto* h1 = h3->ProjectionX(Form("proj_ep_%s_bin%d", tag, idx), 1, -1, z1, z2);
    if (!h1 || h1->GetEntries() == 0) {
      std::cout << "[WARN] No entries in bin " << pmin << "–" << pmax << " GeV" << std::endl;
      ++idx;
      continue;
    }

    h1->SetLineWidth(2);
    h1->SetLineColor(kBlue + idx);
    h1->SetTitle(Form("E/p distribution (HCAL only) — %.1f < p < %.1f GeV;E/p;Events", pmin, pmax));

    TCanvas c(Form("c_resp_%s_bin%d", tag, idx), "", 800, 600);
    h1->Draw("HIST E");

    std::string outname = Form("%s/respDist_%s_%.0f_%.0f.png", datasetDir.c_str(), tag, pmin, pmax);
    std::cout << "[INFO] Saving " << outname << std::endl;
    c.SaveAs(outname.c_str());
    ++idx;
  }
}

// --- 2D heatmapit log-x:llä + profiilit + monisivuinen PDF ---
void make_h2_maps_logx_pdf(const char* inpath, const char* outdirTag) {
  gStyle->SetPalette(kBird);



// ----------------------------------------------------------------------
// Piirretään overlay: kahden datasetin E/p-jakaumat samoille p-alueille
//  - sisältää nyt:
//     • y-max = 0.6
//     • mahdollisuuden HCAL correction cutiin
//     • vain kaksi hadronityyppiä: HCALonly ja HCALplusECAL
// ----------------------------------------------------------------------
void plot_overlay_responses(const char* file1, const char* tag1,
                            const char* file2, const char* tag2,
                            const char* outdir,
                            const std::vector<std::pair<double,double>>& bins,
                            bool useHCALCut = false) {

  gStyle->SetOptStat(0);  // poistaa entries / mean / RMS -laatikot

  // Open both ROOT files
  std::unique_ptr<TFile> f1(TFile::Open(file1, "READ"));
  std::unique_ptr<TFile> f2(TFile::Open(file2, "READ"));
  if (!f1 || f1->IsZombie() || !f2 || f2->IsZombie()) {
    std::cerr << "[plot_overlay_responses] Cannot open one of the input files.\n";
    return;
  }

  // Define the relevant histogram names for both types (HCALonly and HCALplusECAL)
  std::vector<std::string> hadronCats   = {"isHadH", "isHadEH"};
  std::vector<std::string> hadronLabels = {"HCALonly", "HCALplusECAL"};

  // Optional HCAL correction cut
  std::string cutTag   = (useHCALCut ? "_HCALcut" : "_noCut");
  std::string outputDir = std::string(outdir) + cutTag;
  gSystem->mkdir(outputDir.c_str(), true);

  std::cout << "[INFO] Output directory: " << outputDir << std::endl;
  if (useHCALCut)
    std::cout << "[INFO] Applying HCAL correction cut (E_HCAL / E_HCAL_raw > 0.9)\n";
  else
    std::cout << "[INFO] No HCAL correction cut applied\n";

  // -------------------------------------------------------------
  // Loop over both hadron types
  // -------------------------------------------------------------
  for (size_t h = 0; h < hadronCats.size(); ++h) {

    // Valitaan histogrammin nimi riippuen HCAL-cutin käytöstä
    std::string hname1, hname2;
    if (useHCALCut) {
      // Käytetään HCAL-cutin mukaisia histogrammeja
      hname1 = "h3_resp_corr_p_" + hadronCats[h];
      hname2 = "h3_resp_corr_p_" + hadronCats[h];
    } else {
      // Käytetään ilman HCAL-cuttia tehtyjä histogrammeja
      hname1 = "h3_resp_def_p_" + hadronCats[h] + "_all";
      hname2 = "h3_resp_def_p_" + hadronCats[h] + "_all";
    }

    std::cout << "[DEBUG] Using histograms: " << hname1 << " and " << hname2 << std::endl;

    // Ladataan histogrammit molemmista tiedostoista
    auto* h3_1 = dynamic_cast<TH3D*>(f1->Get(hname1.c_str()));
    auto* h3_2 = dynamic_cast<TH3D*>(f2->Get(hname2.c_str()));

    if (!h3_1 || !h3_2) {
      std::cerr << "[ERROR] Missing histograms for hadron type " << hadronLabels[h] << std::endl;
      continue;
    }

    // -------------------------------------------------------------
    // Loop over pT bins
    // -------------------------------------------------------------
    int idx = 0;
    for (auto& bin : bins) {
      double pmin = bin.first;
      double pmax = bin.second;

      int z1a = h3_1->GetZaxis()->FindBin(pmin + 1e-3);
      int z2a = h3_1->GetZaxis()->FindBin(pmax - 1e-3);
      int z1b = h3_2->GetZaxis()->FindBin(pmin + 1e-3);
      int z2b = h3_2->GetZaxis()->FindBin(pmax - 1e-3);
      if (z2a <= z1a) z2a = z1a + 1;
      if (z2b <= z1b) z2b = z1b + 1;

      std::cout << "[DEBUG] Bin " << idx << ": " << pmin << "–" << pmax
                << " GeV, type=" << hadronLabels[h] << std::endl;

      auto* h1 = h3_1->ProjectionX(Form("h1_%zu_%d", h, idx), 1, -1, z1a, z2a);
      auto* h2 = h3_2->ProjectionX(Form("h2_%zu_%d", h, idx), 1, -1, z1b, z2b);

      if (!h1 || !h2 || h1->GetEntries() == 0 || h2->GetEntries() == 0) {
        std::cout << "[WARN] No entries for " << pmin << "-" << pmax << " GeV" << std::endl;
        ++idx;
        continue;
      }

      // Normalize
      if (h1->Integral() > 0) h1->Scale(1.0 / h1->Integral());
      if (h2->Integral() > 0) h2->Scale(1.0 / h2->Integral());

      h1->SetLineColor(kBlue);
      h2->SetLineColor(kRed);
      h1->SetLineWidth(2);
      h2->SetLineWidth(2);

      // Draw and apply y-max limit
      TCanvas c(Form("c_overlay_%zu_%d", h, idx), "", 800, 600);
      h1->SetTitle(Form("E/p comparison (%s) %.1f < p < %.1f GeV;E/p;Normalized entries",
                        hadronLabels[h].c_str(), pmin, pmax));
      h1->SetMaximum(0.6);
      h1->Draw("HIST");
      h2->Draw("HIST SAME");

      auto* leg = new TLegend(0.62, 0.74, 0.88, 0.88);
      leg->AddEntry(h1, tag1, "l");
      leg->AddEntry(h2, tag2, "l");
      leg->Draw();

      std::string outname = Form("%s/overlay_%s_%.0f_%.0f.png",
                                 outputDir.c_str(), hadronLabels[h].c_str(), pmin, pmax);
      std::cout << "[INFO] Saving " << outname << std::endl;
      c.SaveAs(outname.c_str());

      ++idx;
    }
  }
  // -------------------------------------------------------------
  // Ei categorization: yhdistetään kaikki hadron hadron tyypit yhteen
  // -------------------------------------------------------------
  {
    std::cout << "[DEBUG] Combining all hadron categories (no categorization)" << std::endl;

    // Määritellään neljä histogrammia (def ja corr-versiot)
    std::vector<std::string> catNames = {"isHadH", "isHadEH", "isHadE", "isHadMIP"};

    TH3D *h3_1_sum = nullptr;
    TH3D *h3_2_sum = nullptr;

    for (auto &cat : catNames) {
      std::string hname1, hname2;
      if (useHCALCut) {
        hname1 = "h3_resp_corr_p_" + cat;
        hname2 = "h3_resp_corr_p_" + cat;
      } else {
        hname1 = "h3_resp_def_p_" + cat + "_all";
        hname2 = "h3_resp_def_p_" + cat + "_all";
      }

      auto *tmp1 = dynamic_cast<TH3D*>(f1->Get(hname1.c_str()));
      auto *tmp2 = dynamic_cast<TH3D*>(f2->Get(hname2.c_str()));
      if (!tmp1 || !tmp2) continue;

      if (!h3_1_sum) h3_1_sum = (TH3D*)tmp1->Clone("h3_1_sum");
      else h3_1_sum->Add(tmp1);

      if (!h3_2_sum) h3_2_sum = (TH3D*)tmp2->Clone("h3_2_sum");
      else h3_2_sum->Add(tmp2);
    }

    if (!h3_1_sum || !h3_2_sum) {
      std::cerr << "[WARN] Could not combine histograms for no categorization." << std::endl;
    } else {
      int idx = 0;
      for (auto &bin : bins) {
        double pmin = bin.first;
        double pmax = bin.second;

        int z1a = h3_1_sum->GetZaxis()->FindBin(pmin + 1e-3);
        int z2a = h3_1_sum->GetZaxis()->FindBin(pmax - 1e-3);
        int z1b = h3_2_sum->GetZaxis()->FindBin(pmin + 1e-3);
        int z2b = h3_2_sum->GetZaxis()->FindBin(pmax - 1e-3);
        if (z2a <= z1a) z2a = z1a + 1;
        if (z2b <= z1b) z2b = z1b + 1;

        auto *h1 = h3_1_sum->ProjectionX(Form("h1_noCat_%d", idx), 1, -1, z1a, z2a);
        auto *h2 = h3_2_sum->ProjectionX(Form("h2_noCat_%d", idx), 1, -1, z1b, z2b);

        if (!h1 || !h2 || h1->GetEntries() == 0 || h2->GetEntries() == 0) continue;

        if (h1->Integral() > 0) h1->Scale(1.0 / h1->Integral());
        if (h2->Integral() > 0) h2->Scale(1.0 / h2->Integral());

        h1->SetLineColor(kBlue);
        h2->SetLineColor(kRed);
        h1->SetLineWidth(2);
        h2->SetLineWidth(2);

        TCanvas c(Form("c_noCat_%d", idx), "", 800, 600);
        h1->SetTitle(Form("E/p comparison (no categorization) %.1f < p < %.1f GeV;E/p;Normalized entries",
                          pmin, pmax));
        h1->SetMaximum(0.6);
        h1->Draw("HIST");
        h2->Draw("HIST SAME");

        auto *leg = new TLegend(0.62, 0.74, 0.88, 0.88);
        leg->AddEntry(h1, tag1, "l");
        leg->AddEntry(h2, tag2, "l");
        leg->Draw();

        std::string outname = Form("%s/overlay_noCategory_%.0f_%.0f.png",
                                  outputDir.c_str(), pmin, pmax);
        c.SaveAs(outname.c_str());
        ++idx;
      }
    }
  }
}




// ----------------------------------------------------------------------
// Pääohjelma
// ----------------------------------------------------------------------
int main(int argc, char** argv) {
  gROOT->SetBatch(kTRUE);

  std::vector<std::pair<double,double>> pBins = {
  {3,5},    // very low p
  {8,12},   // low p
  {18,25},  // medium-low p
  {40,60},  // medium p
  {70,85},  // high p
  {90,100}  // very high p (edge of histogram)
};

  if (argc > 1) {
    std::string a1 = argv[1];

    if (a1 == "--resp2025") {
      plot_response_distributions("/eos/user/m/mmarjama/my_pion_analysis/histograms/ZeroBias2025D.root",
                                  "ZeroBias2025D", pBins);
    }
    else if (a1 == "--resp2024") {
      plot_response_distributions("/eos/user/m/mmarjama/my_pion_analysis/histograms/ZeroBias2024_All.root",
                                  "ZeroBias2024", pBins);
    }
    else if (a1 == "--overlay2024vs2025") {
  // Run both versions automatically
      plot_overlay_responses(
        "/eos/user/m/mmarjama/my_pion_analysis/histograms/ZeroBias2025D.root", "Data 2025D",
        "/eos/user/m/mmarjama/my_pion_analysis/histograms/ZeroBias2024_All.root", "Data 2024",
        "/eos/user/m/mmarjama/my_pion_analysis/plots_overlay_2024vs2025",
        pBins, false);  // no HCAL cut

      plot_overlay_responses(
        "/eos/user/m/mmarjama/my_pion_analysis/histograms/ZeroBias2025D.root", "Data 2025D",
        "/eos/user/m/mmarjama/my_pion_analysis/histograms/ZeroBias2024_All.root", "Data 2024",
        "/eos/user/m/mmarjama/my_pion_analysis/plots_overlay_2024vs2025",
        pBins, true);   // with HCAL cut
    }
    else if (a1 == "--overlay2025DataMC") {
      plot_overlay_responses(
        "/eos/user/m/mmarjama/my_pion_analysis/histograms/ZeroBias2025D.root", "Data 2025D",
        "/eos/user/m/mmarjama/my_pion_analysis/histograms/SingleNeutrino2025.root", "MC 2025D",
        "/eos/user/m/mmarjama/my_pion_analysis/plots_overlay_DataMC_2025",
        pBins);
    }
    else if (a1 == "--overlay2024DataMC") {
      plot_overlay_responses(
        "/eos/user/m/mmarjama/my_pion_analysis/histograms/ZeroBias2024_All.root", "Data 2024",
        "/eos/user/m/mmarjama/my_pion_analysis/histograms/SingleNeutrino2024.root", "MC 2024",
        "/eos/user/m/mmarjama/my_pion_analysis/plots_overlay_DataMC_2024",
        pBins);
    }
    else {
      std::cerr << "Unknown option. Use one of:\n"
                << "  --resp2025\n  --resp2024\n"
                << "  --overlay2024vs2025\n"
                << "  --overlay2025DataMC\n"
                << "  --overlay2024DataMC\n";
    }
  } else {
    std::cerr << "Usage: ./plot_histograms [--resp2025 | --resp2024 | --overlay2024vs2025 | --overlay2025DataMC | --overlay2024DataMC]\n";
  }

  return 0;
}
