
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
std::string outdir = "plots/Data_ZeroBias2024";

// pääfunktio; lukee ROOT-tiedoston, piirtää histogrammit ja tallentaa kuvat
void plot_histograms(const char* filename) {
    // avataan ROOT-tiedosto ukutilassa
    TFile* file = TFile::Open(filename, "READ"); 
    if (!file || file->IsZombie()) {
        std::cerr << "Error: cannot open file " << filename << std::endl;
        return;
    }

    auto h3_H   = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadH"));
    auto h3_E   = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadE"));
    auto h3_MIP = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadMIP"));
    auto h3_EH  = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadEH"));

    // väriteema 2D-ploteille
    gStyle->SetPalette(kBird);
    // avataan sama tiedosto kirjoitustilassa tulosten tallennusta varten
    TFile* fout = new TFile(filename, "UPDATE");
    fout->cd();

    // Luo output-hakemisto kuville
    std::string outdir = "plots/Data_ZeroBias2024/";
    gSystem->mkdir(outdir.c_str(), true);

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



    // --- E/p distributions per hadron type, all pT bins, no stack, no overlay ---
    const double trkPBins_full[] = {
        3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
        7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
        16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
    };
    const int nTrkPBins_full = sizeof(trkPBins_full) / sizeof(double) - 1;

    std::vector<std::pair<std::string, std::string>> hadronTypesWithLabel = {
        {"isHadH", "HCALonly"},
        {"isHadE", "ECALonly"},
        {"isHadMIP", "MIP"},
        {"isHadEH", "ECALHCAL"}
    };

    gStyle->SetOptFit(111); // näyttää fit-parametrit kuvassa
    for (const auto& [type, label] : hadronTypesWithLabel) {
        TH3D* h3 = dynamic_cast<TH3D*>(file->Get(("h3_resp_corr_p_" + type).c_str()));
        if (!h3) continue;

        // HCAL correction cut: 0.9 < E_HCAL / E_HCAL^raw < 2.5
        int y_min = h3->GetYaxis()->FindBin(0.9001);
        int y_max = h3->GetYaxis()->FindBin(2.4999);

        for (int i = 0; i < nTrkPBins_full; ++i) {
            double pmin = trkPBins_full[i];
            double pmax = trkPBins_full[i + 1];

            int z_min = h3->GetZaxis()->FindBin(pmin + 1e-3);
            int z_max = h3->GetZaxis()->FindBin(pmax - 1e-3);

            std::string projName = Form("proj_ep_%s_bin_%d", label.c_str(), i);
            TH1D* hproj = h3->ProjectionX(projName.c_str(), y_min, y_max, z_min, z_max);
            if (!hproj || hproj->Integral() == 0) continue;

            hproj->Scale(1.0 / hproj->Integral());

            TCanvas* c = new TCanvas(("canvas_" + projName).c_str(), "", 800, 600);
            hproj->SetTitle(Form("E/p (%s), %.1f-%.1f GeV", label.c_str(), pmin, pmax));
            hproj->GetXaxis()->SetTitle("E/p");
            hproj->GetYaxis()->SetTitle("Fraction of particles");
            hproj->SetLineColor(kBlue + 2);

            // Luo uniikki fit-nimi
            std::string fitName = "fit_" + projName;
            TF1* fit = new TF1(fitName.c_str(), "gaus(0) + gaus(3) + gaus(6)", 0.3, 2.0);
            fit->SetParameters(0.2, 0.8, 0.1, 0.1, 1.0, 0.1, 0.05, 1.3, 0.2);
            fit->SetLineColor(kRed);
            fit->SetLineWidth(2);

            // Fit ja piirto
            hproj->Fit(fit, "RQ");
            hproj->Draw("HIST");

            TF1* fit_result = hproj->GetFunction(fitName.c_str());
            if (fit_result) {
                fit_result->SetLineColor(kRed);
                fit_result->SetLineWidth(2);
                fit_result->Draw("SAME");
            }

            c->SaveAs((outdir + projName + ".png").c_str());
            c->Write();


        }

        h3->GetYaxis()->UnZoom();
        h3->GetZaxis()->UnZoom();
    }


















    // --- STACKED E/p DISTRIBUTIONS FOR EACH pT BIN ---
    const double trkPBins[] = {
        3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
        7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
        16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
    };
    const int nTrkPBins = sizeof(trkPBins) / sizeof(double) - 1;

    if (h3_H && h3_E && h3_MIP && h3_EH) {
        int y_min = h3_H->GetYaxis()->FindBin(0.9001);
        int y_max = h3_H->GetYaxis()->FindBin(2.4999);

        // for (int i = 0; i < nTrkPBins; ++i) {
        //     double pmin = trkPBins[i];
        //     double pmax = trkPBins[i + 1];

        //     int z_min = h3_H->GetZaxis()->FindBin(pmin + 1e-3);
        //     int z_max = h3_H->GetZaxis()->FindBin(pmax - 1e-3);

        //     // Projektiot
        //     TH1D* h_H   = h3_H  ? h3_H->ProjectionX(Form("h_H_%d", i), y_min, y_max, z_min, z_max) : nullptr;
        //     TH1D* h_E   = h3_E  ? h3_E->ProjectionX(Form("h_E_%d", i), y_min, y_max, z_min, z_max) : nullptr;
        //     TH1D* h_MIP = h3_MIP? h3_MIP->ProjectionX(Form("h_MIP_%d", i), y_min, y_max, z_min, z_max) : nullptr;
        //     std::cout << "pT bin " << i << ": MIP integral = " << (h_MIP ? h_MIP->Integral() : -1) << std::endl;

        //     TH1D* h_EH  = h3_EH ? h3_EH->ProjectionX(Form("h_EH_%d", i), y_min, y_max, z_min, z_max) : nullptr;

            // Luo THStack
            // THStack* stack = new THStack(Form("stack_%d", i), Form("E/p stack, %.1f-%.1f GeV;E/p;Fraction", pmin, pmax));

            // Laske kokonaismäärä ennen skaalausta
            // double total = 0.0;
            // if (h_H)   total += h_H->Integral();
            // if (h_E)   total += h_E->Integral();
            // if (h_MIP) total += h_MIP->Integral();
            // if (h_EH)  total += h_EH->Integral();

            // if (total == 0) continue; // Vältä jako nollalla

            // // Skaalaa kaikki suhteessa kokonaissummaan
            // if (h_H)   h_H->Scale(1.0 / total);
            // if (h_E)   h_E->Scale(1.0 / total);
            // if (h_MIP) h_MIP->Scale(1.0 / total);
            // if (h_EH)  h_EH->Scale(1.0 / total);

            // // Värit ja lisäys stackiin
            // std::vector<std::pair<TH1D*, Color_t>> hists = {
            //     {h_H, kRed + 1}, {h_E, kBlue + 1}, {h_MIP, kGreen + 2}, {h_EH, kMagenta + 2}
            // };

            // for (auto& [h, color] : hists) {
            //     if (!h) continue;
            //     h->SetFillColor(color);
            //     h->SetLineColor(kBlack);
            //     stack->Add(h);
            // }



        // Laske kokonaismäärä
        for (int i = 0; i < nTrkPBins; ++i) {
            double pmin = trkPBins[i];
            double pmax = trkPBins[i + 1];

            int z_min = h3_H->GetZaxis()->FindBin(pmin + 1e-3);
            int z_max = h3_H->GetZaxis()->FindBin(pmax - 1e-3);
            
            // ... tästä eteenpäin pysyy samana
            TH1D* h_H   = h3_H  ? h3_H->ProjectionX(Form("h_H_%d", i), y_min, y_max, z_min, z_max) : nullptr;
            TH1D* h_E   = h3_E  ? h3_E->ProjectionX(Form("h_E_%d", i), y_min, y_max, z_min, z_max) : nullptr;
            TH1D* h_MIP = h3_MIP? h3_MIP->ProjectionX(Form("h_MIP_%d", i), y_min, y_max, z_min, z_max) : nullptr;
            TH1D* h_EH  = h3_EH ? h3_EH->ProjectionX(Form("h_EH_%d", i), y_min, y_max, z_min, z_max) : nullptr;
            
            // Laske kokonaissumma
            double total = 0.0;
                if (h_H)   total += h_H->Integral();
                if (h_E)   total += h_E->Integral();
                if (h_MIP) total += h_MIP->Integral();
                if (h_EH)  total += h_EH->Integral();
                if (total == 0) continue;

                // Luo stack tässä
                THStack* stack = new THStack(Form("stack_%d", i), Form("E/p stack, %.1f-%.1f GeV;E/p;Fraction", pmin, pmax));

                // Värit + skaalaus
                std::vector<std::pair<TH1D*, Color_t>> hists = {
                   {h_MIP, kGreen + 2}, {h_H, kRed + 1}, {h_E, kBlue + 1}, {h_EH, kMagenta + 2}
                };

                for (auto& [h, color] : hists) {
                    if (!h || h->Integral() == 0) continue;
                    h->Scale(1.0 / total);
                    h->SetFillColor(color);
                    h->SetLineColor(kBlack);
                    stack->Add(h);
                    std::cout << "pT bin " << i << ": Adding " << h->GetName() << ", scaled integral = " << h->Integral() << std::endl;
                }

                // Piirto ja tallennus
                TCanvas* c = new TCanvas(Form("canvas_stack_%d", i), "", 800, 600);
                stack->Draw("HIST");
                TLegend* leg = new TLegend(0.7, 0.65, 0.88, 0.88);
                leg->AddEntry((TObject*)0, Form("p_{T} = %.1f-%.1f GeV", pmin, pmax), "");
                leg->AddEntry(h_H, "HCAL only", "f");
                leg->AddEntry(h_E, "ECAL only", "f");
                leg->AddEntry(h_MIP, "MIP", "f");
                leg->AddEntry(h_EH, "ECAL+HCAL", "f");
                leg->Draw();
                c->SaveAs((outdir + Form("/stack_ep_bin_%d.png", i)).c_str());
                c->Write();



                // fitataan
                if (h_H && h_E && h_MIP && h_EH) {
                TH1D* h_sum = (TH1D*)h_H->Clone(Form("h_sum_%d", i));
                h_sum->Add(h_E);
                h_sum->Add(h_MIP);
                h_sum->Add(h_EH);
                h_sum->Scale(1.0 / h_sum->Integral());

                TF1* fit = new TF1(Form("fit_%d", i), "gaus(0)", 0.3, 2.0); // tai sopivampi funktio!
                fit->SetLineColor(kBlack);
                fit->SetLineWidth(2);
                h_sum->Fit(fit, "RQ");

                TCanvas* c_fit = new TCanvas(Form("c_fit_%d", i), "", 800, 600);
                h_sum->SetTitle(Form("Fit to stacked E/p, %.1f-%.1f GeV", pmin, pmax));
                h_sum->GetXaxis()->SetTitle("E/p");
                h_sum->GetYaxis()->SetTitle("Fraction");
                h_sum->Draw("HIST");
                fit->Draw("SAME");
                c_fit->SaveAs((outdir + Form("/fit_stack_bin_%d.png", i)).c_str());
                c_fit->Write();
                }






            }


        h3_H->GetYaxis()->UnZoom();
        h3_H->GetZaxis()->UnZoom();
    }





    // UUSI !!!!!!!!!!!!!!!!!!!!


    // --- Fraktiopiirto: eri hadronityyppien osuus pT:n funktiona ---
    TCanvas* c_frac = new TCanvas("c_frac", "Hadron type fraction vs p", 800, 600);
    auto h_frac_H   = dynamic_cast<TH1D*>(file->Get("h_frac_H"));
    auto h_frac_E   = dynamic_cast<TH1D*>(file->Get("h_frac_E"));
    auto h_frac_MIP = dynamic_cast<TH1D*>(file->Get("h_frac_MIP"));
    auto h_frac_EH  = dynamic_cast<TH1D*>(file->Get("h_frac_EH"));

    if (h_frac_H && h_frac_E && h_frac_MIP && h_frac_EH) {
        // Aseta värit ja tyyli
        h_frac_H->SetLineColor(kRed + 1);
        h_frac_E->SetLineColor(kBlue + 1);
        h_frac_MIP->SetLineColor(kGreen + 2);
        h_frac_EH->SetLineColor(kMagenta + 2);

        h_frac_H->SetLineWidth(2);
        h_frac_E->SetLineWidth(2);
        h_frac_MIP->SetLineWidth(2);
        h_frac_EH->SetLineWidth(2);

        h_frac_H->SetTitle("Hadron type fraction vs p");
        h_frac_H->GetXaxis()->SetTitle("Track momentum p (GeV)");
        h_frac_H->GetYaxis()->SetTitle("Fraction");

        h_frac_H->SetMinimum(0.0);
        h_frac_H->SetMaximum(1.05);

        h_frac_H->Draw("HIST");
        h_frac_E->Draw("HIST SAME");
        h_frac_MIP->Draw("HIST SAME");
        h_frac_EH->Draw("HIST SAME");

        // Legenda
        TLegend* leg = new TLegend(0.15, 0.68, 0.55, 0.88);
        leg->SetTextSize(0.035);
        leg->AddEntry(h_frac_H,   "HCAL only", "l");
        leg->AddEntry(h_frac_E,   "ECAL only", "l");
        leg->AddEntry(h_frac_MIP, "MIP",       "l");
        leg->AddEntry(h_frac_EH,  "ECAL+HCAL", "l");
        leg->Draw();

        // Tallennus
        c_frac->SaveAs((outdir + "fraction_per_type_vs_pT.png").c_str());
        c_frac->Write();
    }















    std::cout << "Drawing stacked hadron fraction plot..." << std::endl;

    // Luodaan uudet TH1D:t (tyhjät) samalla binityksellä
    TH1D* h_H_bin   = new TH1D("h_H_bin", "Hadron fractions", nTrkPBins, trkPBins);
    TH1D* h_E_bin   = new TH1D("h_E_bin", "Hadron fractions", nTrkPBins, trkPBins);
    TH1D* h_MIP_bin = new TH1D("h_MIP_bin", "Hadron fractions", nTrkPBins, trkPBins);
    TH1D* h_EH_bin  = new TH1D("h_EH_bin", "Hadron fractions", nTrkPBins, trkPBins);

    // Täytetään binien arvot profiileista
    for (int i = 1; i <= h_H_bin->GetNbinsX(); ++i) {
        h_H_bin->SetBinContent(i, h_frac_H->GetBinContent(i));
        h_E_bin->SetBinContent(i, h_frac_E->GetBinContent(i));
        h_MIP_bin->SetBinContent(i, h_frac_MIP->GetBinContent(i));
        h_EH_bin->SetBinContent(i, h_frac_EH->GetBinContent(i));
    }

    // Asetetaan värit
    h_H_bin->SetFillColor(kRed);
    h_E_bin->SetFillColor(kBlue);
    h_MIP_bin->SetFillColor(kGreen+2);
    h_EH_bin->SetFillColor(kMagenta);

    // Luodaan stack
    THStack* hs = new THStack("hs", "Hadron type fraction vs p;Track momentum p (GeV);Fraction");
    hs->Add(h_E_bin);
    hs->Add(h_MIP_bin);
    hs->Add(h_EH_bin);
    hs->Add(h_H_bin);

    // Piirretään
    TCanvas* c_stack = new TCanvas("c_stack_frac", "", 900, 700);
    hs->Draw("hist");
    hs->SetMaximum(1.05);
    hs->GetXaxis()->SetTitleSize(0.045);
    hs->GetYaxis()->SetTitleSize(0.045);

    TLegend* leg_stack = new TLegend(0.70, 0.65, 0.88, 0.88);
    leg_stack->AddEntry(h_H_bin, "HCAL only", "f");
    leg_stack->AddEntry(h_E_bin, "ECAL only", "f");
    leg_stack->AddEntry(h_MIP_bin, "MIP", "f");
    leg_stack->AddEntry(h_EH_bin, "ECAL+HCAL", "f");
    leg_stack->Draw();

    c_stack->SaveAs((outdir + "/fraction_stacked.png").c_str());
    c_stack->Write();














        
        

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