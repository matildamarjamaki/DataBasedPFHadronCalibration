
#include "TFile.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TH1D.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TF1.h"
#include "TProfile.h"
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

// kansiopolku, johon png-kuvat tallennetaan
std::string outdir = "plots/";

// pääfunktio histogrammien plottaukseen
void plot_histograms(const char* filename) {
    // avataan ROOT-tiedosto ukutilassa
    TFile* file = TFile::Open(filename, "READ");
    if (!file || file->IsZombie()) {
        std::cerr << "Error: cannot open file " << filename << std::endl;
        return;
    }

    // väriteema 2D-ploteille
    gStyle->SetPalette(kBird);
    // avataan sama tiedosto kirjoitustilassa tulosten tallennusta varten
    TFile* fout = new TFile(filename, "UPDATE");

    // lista histogrammeista, jotka piirretään suoraan
    std::vector<std::string> histNames = {
        "h2_ep_vs_p_iso", "h2_raw_ep_vs_p_iso",
        "prof_ep_vs_p_iso", "prof_raw_ep_vs_p_iso",
        "prof_ep_vs_p_iso_custom", "prof_raw_ep_vs_p_iso_custom",
        "h_pt_iso", "h_eta_iso", "h_ep_iso", "h_raw_ep_iso",
        "h_ep_isHadH", "h_ep_isHadE", "h_ep_isHadMIP", "h_ep_isHadEH",
        "h_raw_ep_isHadH", "h_raw_ep_isHadE", "h_raw_ep_isHadMIP", "h_raw_ep_isHadEH"
    };

    // käydään histogrammit läpi
    for (const auto& name : histNames) {
        TObject* obj = file->Get(name.c_str());
        if (!obj) continue;

        // luodaan canvas jokaiselle histogrammille
        TCanvas* c = new TCanvas(("canvas_" + name).c_str(), name.c_str(), 800, 600);

        // 2D-histogrammit (esim. E/p vs p)
        if (auto* h2 = dynamic_cast<TH2*>(obj)) {
            h2->GetXaxis()->SetTitle("Track momentum p (GeV)");
            h2->GetYaxis()->SetTitle((name.find("raw") != std::string::npos) ? "Raw E/p" : "E/p");
            h2->SetTitle((name.find("raw") != std::string::npos) ? "Raw E/p vs p" : "E/p vs p");
            h2->Draw("COLZ");

        // 1D-histogrammit
        } else if (auto* h1 = dynamic_cast<TH1*>(obj)) {
            h1->GetXaxis()->SetTitle("E/p");
            h1->GetYaxis()->SetTitle("Events");
            h1->SetTitle(name.c_str());
            h1->Draw("HIST");

        // profiilit (keskiarvo E/p vs p)
        } else if (auto* prof = dynamic_cast<TProfile*>(obj)) {
            prof->GetXaxis()->SetTitle("Track momentum p (GeV)");
            prof->GetYaxis()->SetTitle("Mean E/p");
            prof->SetTitle("Mean E/p vs p");
            prof->Draw();
        }

        // tallennetaan kuva png:nä ja canvas ROOT-tiedostoon
        c->SaveAs((outdir + name + ".png").c_str());
        c->Write();
    }

    // hadronityypit ja niihin liittyvät tunnisteet sekä nimeämiset
    std::vector<std::string> hadronTypes = {"isHadH", "isHadE", "isHadMIP", "isHadEH"};
    std::vector<std::string> labels = {"HCAL only", "ECAL only", "MIP", "ECAL+HCAL"};
    std::vector<std::pair<double, double>> ptRanges = {{5.5, 6.0}, {10.0, 12.0}, {20.0, 22.0}};
    std::vector<std::string> ptTags = {"low", "medium", "high"};

    // käydään läpi jokianen hadronityyppi ja pT-alue
    for (size_t i = 0; i < hadronTypes.size(); ++i) {
        std::string name3d = "h3_resp_corr_p_" + hadronTypes[i];
        TH3D* h3 = dynamic_cast<TH3D*>(file->Get(name3d.c_str()));
        if (!h3) continue;

        for (size_t j = 0; j < ptRanges.size(); ++j) {
            double pmin = ptRanges[j].first;
            double pmax = ptRanges[j].second;

            h3->GetZaxis()->SetRange(h3->GetZaxis()->FindBin(pmin + 1e-3),
                                     h3->GetZaxis()->FindBin(pmax - 1e-3));
            auto proj2D = dynamic_cast<TH2D*>(h3->Project3D("xy"));
            std::string proj2D_name = name3d + "_proj2D_" + ptTags[j];

            proj2D->SetName(proj2D_name.c_str());
            proj2D->SetMinimum(1e-6);
            proj2D->SetTitle(Form("E/p vs E_{HCAL}/E_{HCAL}^{raw}, %s (%.1f–%.1f GeV)", labels[i].c_str(), pmin, pmax));
            proj2D->GetXaxis()->SetTitle("E_{HCAL}/E_{HCAL}^{raw}");
            proj2D->GetYaxis()->SetTitle("E/p");

            TCanvas* c2D = new TCanvas(("canvas_" + proj2D_name).c_str(), proj2D_name.c_str(), 800, 600);
            proj2D->Draw("COLZ");
            c2D->SaveAs((outdir + proj2D_name + ".png").c_str());
            proj2D->Write();
            c2D->Write();
        }
    }

    // E/p-jakaumat tietyillä p-alueilla, fitataan Gaussin käyrä
    std::vector<std::pair<double, double>> pBins = {{5.5, 6.0}, {20.0, 22.0}};
    TH2D* h2_ep = dynamic_cast<TH2D*>(file->Get("h2_ep_vs_p_iso"));
    TH2D* h2_raw = dynamic_cast<TH2D*>(file->Get("h2_raw_ep_vs_p_iso"));

    if (h2_ep && h2_raw) {
        for (const auto& [pmin, pmax] : pBins) {
            int bin_min = h2_ep->GetXaxis()->FindBin(pmin + 1e-3);
            int bin_max = h2_ep->GetXaxis()->FindBin(pmax - 1e-3);

            std::stringstream sn, sr;
            sn << "proj_ep_" << int(pmin * 10) << "_" << int(pmax * 10);
            sr << "proj_raw_ep_" << int(pmin * 10) << "_" << int(pmax * 10);

            auto proj_n = h2_ep->ProjectionY(sn.str().c_str(), bin_min, bin_max);
            auto proj_r = h2_raw->ProjectionY(sr.str().c_str(), bin_min, bin_max);

            if (proj_n->Integral() > 0) {
                proj_n->Scale(1.0 / proj_n->Integral());
                TF1* fit_n = new TF1((sn.str() + "_fit").c_str(), "gaus", 0.0, 2.5);
                proj_n->Fit(fit_n, "RQ", "", 0.7, 1.2);

                proj_n->SetTitle(Form("E/p in %.1f <= p < %.1f GeV", pmin, pmax));
                proj_n->GetXaxis()->SetTitle("E/p");
                proj_n->GetYaxis()->SetTitle("Fraction of particles");

                TCanvas* c_n = new TCanvas(("canvas_" + sn.str()).c_str(), sn.str().c_str(), 800, 600);
                proj_n->SetFillColorAlpha(kBlue, 0.35);
                proj_n->Draw("hist");
                fit_n->SetLineColor(kRed);
                fit_n->Draw("same");
                c_n->SaveAs((outdir + sn.str() + ".png").c_str());
                c_n->Write();
            }

            if (proj_r->Integral() > 0) {
                proj_r->Scale(1.0 / proj_r->Integral());
                TF1* fit_r = new TF1((sr.str() + "_fit").c_str(), "gaus", 0.0, 2.5);
                proj_r->Fit(fit_r, "RQ", "", 0.7, 1.2);

                proj_r->SetTitle(Form("Raw E/p in %.1f <= p < %.1f GeV", pmin, pmax));
                proj_r->GetXaxis()->SetTitle("Raw E/p");
                proj_r->GetYaxis()->SetTitle("Fraction of particles");

                TCanvas* c_r = new TCanvas(("canvas_" + sr.str()).c_str(), sr.str().c_str(), 800, 600);
                proj_r->SetFillColorAlpha(kBlue, 0.35);
                proj_r->Draw("hist");
                fit_r->SetLineColor(kRed);
                fit_r->Draw("same");
                c_r->SaveAs((outdir + sr.str() + ".png").c_str());
                c_r->Write();
            }
        }
    }

    // suljetaan tiedostot
    fout->Close();
    file->Close();
}

// pääohjelma, joka ottaa ROOT-tiedoston nimen argumenttina
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <histogram_file.root>" << std::endl;
        return 1;
    }
    plot_histograms(argv[1]);
    return 0;
}