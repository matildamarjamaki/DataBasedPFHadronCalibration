
// tuodaan ROOT-kirjastot histogrammien, canvasien ja funktioiden käsittelyyn
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

// polku, johon tallennetaan png-kuvat jokaisesta plottauksesta
std::string outdir = "plots/";

// pääfunktio; lukee ROOT-tiedoston, piirtää histogrammit ja tallentaa kuvat
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

    // lista histogramminimistä, jotka piirretään suoraan
    std::vector<std::string> histNames = {
        "h2_ep_vs_p_iso", "h2_raw_ep_vs_p_iso",
        "prof_ep_vs_p_iso", "prof_raw_ep_vs_p_iso",
        "prof_ep_vs_p_iso_custom", "prof_raw_ep_vs_p_iso_custom",
        "h_pt_iso", "h_eta_iso", "h_ep_iso", "h_raw_ep_iso",
        "h_ep_isHadH", "h_ep_isHadE", "h_ep_isHadMIP", "h_ep_isHadEH",
        "h_raw_ep_isHadH", "h_raw_ep_isHadE", "h_raw_ep_isHadMIP", "h_raw_ep_isHadEH"
    };

    // käydään lista läpi ja piirretään jokainen histogrammi
    for (const auto& name : histNames) {
        TObject* obj = file->Get(name.c_str());
        if (!obj) continue;

        // luodaan canvas jokaiselle histogrammille
        TCanvas* c = new TCanvas(("canvas_" + name).c_str(), name.c_str(), 800, 600);

        // jos histogrammi on 2D (TH2), piirretään lämpökarttana (COLZ)
        if (auto* h2 = dynamic_cast<TH2*>(obj)) {
            h2->GetXaxis()->SetTitle("Track momentum p (GeV)");
            h2->GetYaxis()->SetTitle((name.find("raw") != std::string::npos) ? "Raw E/p" : "E/p");
            h2->SetTitle((name.find("raw") != std::string::npos) ? "Raw E/p vs p" : "E/p vs p");
            h2->Draw("COLZ");

        // jos histogrammi on 1D (TH1), piirretään tavallisena jakaumana
        } else if (auto* h1 = dynamic_cast<TH1*>(obj)) {
            h1->GetXaxis()->SetTitle("E/p");
            h1->GetYaxis()->SetTitle("Events");
            h1->SetTitle(name.c_str());
            h1->Draw("HIST");

        // jos histogrammi on profiili (TProfile), piirretään pistejoukkona
        } else if (auto* prof = dynamic_cast<TProfile*>(obj)) {
            prof->GetXaxis()->SetTitle("Track momentum p (GeV)");
            prof->GetYaxis()->SetTitle("Mean E/p");
            prof->SetTitle("Mean E/p vs p");
            prof->Draw();
        }

        // tallennetaan kuvat png:nä ja canvas tallennetaan ROOT-tiedostoon
        c->SaveAs((outdir + name + ".png").c_str());
        c->Write();
    }

    // hadronityypit, niiden labelit ja valitut p-alueet projisointeja varten
    std::vector<std::string> hadronTypes = {"isHadH", "isHadE", "isHadMIP", "isHadEH"};
    std::vector<std::string> labels = {"HCAL only", "ECAL only", "MIP", "ECAL+HCAL"};
    std::vector<std::pair<double, double>> ptRanges = {{5.5, 6.0}, {10.0, 12.0}, {20.0, 22.0}};
    std::vector<std::string> ptTags = {"low", "medium", "high"};

    // käydään läpi kukin hadronityyppi ja jokainen p-alue
    for (size_t i = 0; i < hadronTypes.size(); ++i) {
        std::string name3d = "h3_resp_corr_p_" + hadronTypes[i];
        TH3D* h3 = dynamic_cast<TH3D*>(file->Get(name3d.c_str()));
        if (!h3) continue;

        for (size_t j = 0; j < ptRanges.size(); ++j) {
            double pmin = ptRanges[j].first;
            double pmax = ptRanges[j].second;

            // rajataan 3D-histogrammi tiettyyn p-alueeseen
            h3->GetZaxis()->SetRange(h3->GetZaxis()->FindBin(pmin + 1e-3),
                                     h3->GetZaxis()->FindBin(pmax - 1e-3));
            // projektio (x-y) tälle alueelle
            auto proj2D = dynamic_cast<TH2D*>(h3->Project3D("xy"));
            std::string proj2D_name = name3d + "_proj2D_" + ptTags[j];

            // asetetaan akselit ja otsikot
            proj2D->SetName(proj2D_name.c_str());
            proj2D->SetMinimum(1e-6); // log-plotin alkuarvo
            proj2D->SetTitle(Form("E/p vs E_{HCAL}/E_{HCAL}^{raw}, %s (%.1f - %.1f GeV)", labels[i].c_str(), pmin, pmax));
            proj2D->GetXaxis()->SetTitle("E_{HCAL}/E_{HCAL}^{raw}");
            proj2D->GetYaxis()->SetTitle("E/p");

            TCanvas* c2D = new TCanvas(("canvas_" + proj2D_name).c_str(), proj2D_name.c_str(), 800, 600);
            proj2D->GetXaxis()->SetRangeUser(0.9, 2.5); // rajoitetaan x-akseli
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

                TH1D* proj_n = h2_ep->ProjectionY(sn.str().c_str(), bin_min, bin_max);
                TH1D* proj_r = h2_raw->ProjectionY(sr.str().c_str(), bin_min, bin_max);

                if (proj_n && proj_n->Integral() > 0) {
                    proj_n->Scale(1.0 / proj_n->Integral());
                    TF1* fit_n = new TF1((sn.str() + "_fit").c_str(), "gaus", 0.0, 2.5);
                    proj_n->Fit(fit_n, "RQ", "", 0.7, 1.2);

                    proj_n->SetTitle(Form("E/p in %.1f - %.1f GeV", pmin, pmax));
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

                if (proj_r && proj_r->Integral() > 0) {
                    proj_r->Scale(1.0 / proj_r->Integral());
                    TF1* fit_r = new TF1((sr.str() + "_fit").c_str(), "gaus", 0.0, 2.5);
                    proj_r->Fit(fit_r, "RQ", "", 0.7, 1.2);

                    proj_r->SetTitle(Form("Raw E/p in %.1f - %.1f GeV", pmin, pmax));
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




    // fitted mean (no cut), suoraan h2_ep_vs_p_iso:sta tietyillä p-väleillä
    TGraph* g_fit = new TGraph();

    std::vector<std::pair<double, double>> fit_pBins = {{5.5, 6.0}, {10.0, 12.0}, {20.0, 22.0}};
    std::vector<double> pMid = {5.75, 11.0, 21.0};

    if (h2_ep) {
        for (size_t i = 0; i < fit_pBins.size(); ++i) {
            double pmin = fit_pBins[i].first;
            double pmax = fit_pBins[i].second;
            int bin_min = h2_ep->GetXaxis()->FindBin(pmin + 1e-3);
            int bin_max = h2_ep->GetXaxis()->FindBin(pmax - 1e-3);

            std::string projName = Form("proj_fit_ep_nocut_%zu", i);
            TH1D* hproj = h2_ep->ProjectionY(projName.c_str(), bin_min, bin_max);

            if (!hproj || hproj->Integral() == 0) continue;
            hproj->Scale(1.0 / hproj->Integral());

            TF1* fit = new TF1((projName + "_fit").c_str(), "gaus", 0.7, 1.2);
            hproj->Fit(fit, "RQ");

            g_fit->SetPoint(g_fit->GetN(), pMid[i], fit->GetParameter(1));
            fit->Write();  // voit poistaa tämän jos ei tarvita
        }
    }


    // Määritellään pT-välit ja niiden tagit
    std::vector<std::pair<double, double>> hcalcut_pBins = {{5.5, 6.5}, {10.0, 12.0}, {20.0, 24.0}};
    std::vector<std::string> tags = {"low", "medium", "high"};
    //std::vector<double> pMid = {6.0, 11.0, 22.0};

    TGraph* g_fit_hcalcut = new TGraph();

    TH3D* h3 = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadH"));
    if (h3) {
        for (size_t i = 0; i < hcalcut_pBins.size(); ++i) {
            double pmin = hcalcut_pBins[i].first, pmax = hcalcut_pBins[i].second;
            double pmid = pMid[i];

            // HCAL correction cut range: 0.9 < E_HCAL / E_HCAL^raw < 2.5
            int y_min = h3->GetYaxis()->FindBin(0.9001); // pieni epsilon estää reunabinit
            int y_max = h3->GetYaxis()->FindBin(2.4999);
            int z_min = h3->GetZaxis()->FindBin(pmin + 1e-3);
            int z_max = h3->GetZaxis()->FindBin(pmax - 1e-3);

            std::string projName = "proj_resp_cut_" + tags[i];
            TH1D* hproj = h3->ProjectionX(projName.c_str(), y_min, y_max, z_min, z_max);

            if (!hproj || hproj->Integral() == 0) {
                std::cout << "[INFO] Skip empty projection: " << projName << std::endl;
                continue;
            }

            // Normalisointi (valinnainen)
            hproj->Scale(1.0 / hproj->Integral());

            // Gauss-fit
            TF1* fit = new TF1((projName + "_fit").c_str(), "gaus", 0.0, 2.5);
            hproj->Fit(fit, "RQ");

            // Lisää fitted mean piste TGraphiin
            g_fit_hcalcut->SetPoint(g_fit_hcalcut->GetN(), pmid, fit->GetParameter(1));

            // Piirto ja tallennus
            TCanvas* c = new TCanvas(("canvas_" + projName).c_str(), "", 800, 600);
            hproj->SetTitle(Form("E/p for HCALcut, %s (%.1f - %.1f GeV)", tags[i].c_str(), pmin, pmax));
            hproj->GetXaxis()->SetTitle("E/p");
            hproj->GetYaxis()->SetTitle("Fraction of particles");
            hproj->Draw("HIST");
            fit->SetLineColor(kRed);
            fit->Draw("same");
            c->SaveAs((outdir + projName + ".png").c_str());
            c->Write();
            fit->Write();
        }

        // Resetoi mahdolliset axis-ranget seuraavaa käyttöä varten
        h3->GetYaxis()->UnZoom();
        h3->GetZaxis()->UnZoom();
    }









    // vertailuplotti: profiili vs. fitted-mean vs. HCAL-cut fitted-mean
    TCanvas* c = new TCanvas("canvas_compare_fit_vs_profile", "", 800, 600);
    c->SetGrid();  // Lisää ruudukko taustalle

    // Poistetaan stats-laatikko
    gStyle->SetOptStat(0);

    // Profiilikeskiarvo (ilman leikkausta)
    TProfile* prof = dynamic_cast<TProfile*>(file->Get("prof_ep_vs_p_iso_custom"));
    if (prof) {
        prof->SetLineColor(kBlue + 2);
        prof->SetLineWidth(2);
        prof->SetTitle("E/p response vs p (with and without HCAL correction cut)");
        prof->GetXaxis()->SetTitle("Track momentum p (GeV)");
        prof->GetYaxis()->SetTitle("E/p");

        // Rajoitetaan y-akselin näkyvä alue
        prof->GetYaxis()->SetRangeUser(0.75, 1.1);
        prof->Draw("HIST");
    }

    // Fitted-mean ilman HCAL-leikkausta
    g_fit->SetMarkerStyle(20);
    g_fit->SetMarkerColor(kRed + 1);
    g_fit->SetLineColor(kRed + 1);
    g_fit->SetLineWidth(2);
    g_fit->Draw("PL SAME");

    // Fitted-mean HCAL-korjausleikkauksella
    g_fit_hcalcut->SetMarkerStyle(21);
    g_fit_hcalcut->SetMarkerColor(kGreen + 2);
    g_fit_hcalcut->SetLineColor(kGreen + 2);
    g_fit_hcalcut->SetLineWidth(2);
    g_fit_hcalcut->Draw("PL SAME");

    // Legenda
    TLegend* leg = new TLegend(0.15, 0.70, 0.65, 0.88);
    leg->SetTextSize(0.035);
    leg->AddEntry(prof, "Profile mean (no cut)", "l");
    leg->AddEntry(g_fit, "Fitted mean (no cut)", "lp");
    leg->AddEntry(g_fit_hcalcut, "Fitted mean (HCAL cut: E_{HCAL}/E_{HCAL}^{raw} > 0.9)", "lp");
    leg->Draw();

    // Tallennus
    c->SaveAs((outdir + "compare_fit_vs_profile_improved.png").c_str());
    c->Write();













    // automaattinen kaikkien pT-biinien plottaus (HCAL only)
    const double trkPBins[] = {
        3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
        7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
        16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
    };
    const int nTrkPBins = sizeof(trkPBins) / sizeof(double) - 1;

    TH3D* h3_H = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadH"));
    if (h3_H) {
        int y_min = h3_H->GetYaxis()->FindBin(0.9001);
        int y_max = h3_H->GetYaxis()->FindBin(2.4999);

        for (int i = 0; i < nTrkPBins; ++i) {
            double pmin = trkPBins[i];
            double pmax = trkPBins[i + 1];
            double pmid = 0.5 * (pmin + pmax);

            int z_min = h3_H->GetZaxis()->FindBin(pmin + 1e-3);
            int z_max = h3_H->GetZaxis()->FindBin(pmax - 1e-3);

            std::string projName = Form("proj_resp_cut_bin_%d", i);
            TH1D* hproj = h3_H->ProjectionX(projName.c_str(), y_min, y_max, z_min, z_max);

            if (!hproj || hproj->Integral() == 0) {
                std::cout << "[INFO] Skipping empty bin: " << pmin << "-" << pmax << " GeV" << std::endl;
                continue;
            }

            hproj->Scale(1.0 / hproj->Integral());
            TF1* fit = new TF1((projName + "_fit").c_str(), "gaus", 0.0, 2.5);
            hproj->Fit(fit, "RQ");

            // Piirto ja tallennus
            TCanvas* c = new TCanvas(("canvas_" + projName).c_str(), "", 800, 600);
            hproj->SetTitle(Form("E/p (HCAL only), %.1f–%.1f GeV", pmin, pmax));
            hproj->GetXaxis()->SetTitle("E/p");
            hproj->GetYaxis()->SetTitle("Fraction of particles");
            hproj->Draw("HIST");
            fit->SetLineColor(kRed);
            fit->Draw("same");

            // Tallennus tiedostoon
            c->SaveAs((outdir + projName + ".png").c_str());
            c->Write();
            hproj->Write();
            fit->Write();
        }

        // Reset zoom
        h3_H->GetYaxis()->UnZoom();
        h3_H->GetZaxis()->UnZoom();
    }















        
        

    // suljetaan ROOT -tiedostot
    fout->Close();
    file->Close();
}

// pääohjelma, joka saa ROOT-tiedoston nimen komentoriviargumenttina ja käynnistää plottauksen
int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <histogram_file.root>" << std::endl;
        return 1;
    }
    plot_histograms(argv[1]);
    return 0;
}