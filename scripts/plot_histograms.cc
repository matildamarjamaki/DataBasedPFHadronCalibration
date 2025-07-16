
// =================== ROOT-kirjastot ===================
#include "TFile.h" // ROOT-tiedostojen lukemiseen ja kirjoittamiseen
#include "TCanvas.h" // Piirtoalustan luomiseen
#include "TStyle.h" // Tyylimääritykset (esim. väriteemat)
#include "TH1D.h" // 1D histogrammit
#include "TH2D.h" // 2D histogrammit
#include "TH3D.h" // 3D histogrammit
#include "TProfile.h" // Profiilihistogrammit
#include "TLegend.h" // Selitteiden/legendojen luomiseen
#include "TGraph.h" // Pistejoukkojen piirtämiseen
#include "TF1.h" // Sovitusfunktioiden luomiseen
#include "THStack.h" // Histogrammien pinottamiseen
#include "TSystem.h" // Järjestelmätoiminnot, kuten hakemistojen luomiseen

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// =================== Funktioprototyypit ===================
void plot_stacked_comparison(); // Piirtää stacked E/p-distribuutiot (valmiista histogrammeista)
void plot_ep_overlay(); // Piirtää E/p overlayn datalle ja simulaatiolle
//void plot_ep_scale_vs_pt(); // Piirtää fitted-mean/profiili-mean vs. pT
void plot_stacked_comparison_raw(); // Piirtää stacked E/p-distribuutiot raakadatasta (valmiista histogrammeista)

// =================== Pääfunktio ===================
void plot_histograms(const std::string& filename) {
    // Jos filename on dummy, suoritetaan vain valmiiksi tallennettujen histogrammien plottaukset
    if (filename == "dummy") {
        plot_stacked_comparison(); // kutsutaan stacked-plotti tässä
        plot_ep_overlay();
        //plot_ep_scale_vs_pt();
        plot_stacked_comparison_raw();
        return;
    }

    // Avataan ROOT-tiedosto lukutilassa
    TFile* file = TFile::Open(filename.c_str(), "READ"); 
    if (!file || file->IsZombie()) {
        std::cerr << "Error: cannot open file " << filename << std::endl;
        return;
    }

    // Haetaan neljä 3D-histogrammia, jotka sisältävät E/p-arvot eri hadronityypeille
    auto h3_H   = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadH")); // HCAL only
    auto h3_E   = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadE")); // ECAL only
    auto h3_MIP = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadMIP")); // MIP
    auto h3_EH  = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadEH")); // ECAL + HCAL

    // Asetetaan ROOTin globaali väripaletti 2D-plotteihin (kBird = sininen-violetti)
    gStyle->SetPalette(kBird);

    // Avataan sama tiedosto kirjoitustilassa, jotta voidaan tallentaa uusia objekteja (esim. fitit, kanvaasit)
    TFile* fout = new TFile(filename.c_str(), "UPDATE");
    fout->cd(); // asetetaan kirjoitusosoite nykyiseksi hakemistoksi

    // Luo output-hakemisto kuville
    std::string outdir = "plots/Data_ZeroBias2024_without_cut/";
    gSystem->mkdir(outdir.c_str(), true); // luo hakemiston tarvittaessa

    // Lista histogrammien nimistä, jotka piirretään sellaisenaan (1D, 2D tai TProfile)
    std::vector<std::string> histNames = {
        "h2_ep_vs_p_iso", "h2_raw_ep_vs_p_iso",
        "prof_ep_vs_p_iso", "prof_raw_ep_vs_p_iso",
        "prof_ep_vs_p_iso_custom", "prof_raw_ep_vs_p_iso_custom",
        "h_pt_iso", "h_eta_iso", "h_ep_iso", "h_raw_ep_iso",
        "h_ep_isHadH", "h_ep_isHadE", "h_ep_isHadMIP", "h_ep_isHadEH",
        "h_raw_ep_isHadH", "h_raw_ep_isHadE", "h_raw_ep_isHadMIP", "h_raw_ep_isHadEH"
    };

    // Käydään läpi kaikki histogrammit ja piirretään ne omalle TCanvas-ikkunalle
    for (const auto& name : histNames) {
        TObject* obj = file->Get(name.c_str());
        if (!obj) continue;

        // Luodaan uusi piirtoikkuna (canvas)
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

        // Tallennetaan kuva PNG-tiedostona ja ROOT-tiedostoon
        c->SaveAs((outdir + name + ".png").c_str());
        c->Write();
    }

    // Hadronityypit ja niihin liittyvät otsikot sekä halutut pT-alueet
    std::vector<std::string> hadronTypes = {"isHadH", "isHadE", "isHadMIP", "isHadEH"};
    std::vector<std::string> labels = {"HCAL only", "ECAL only", "MIP", "ECAL+HCAL"};
    std::vector<std::pair<double, double>> ptRanges = {{5.5, 6.0}, {10.0, 12.0}, {20.0, 22.0}};
    std::vector<std::string> ptTags = {"low", "medium", "high"};

    // Käydään läpi jokainen hadronityyppi ja jokainen valittu p-alue
    for (size_t i = 0; i < hadronTypes.size(); ++i) {
        std::string name3d = "h3_resp_corr_p_" + hadronTypes[i];
        TH3D* h3 = dynamic_cast<TH3D*>(file->Get(name3d.c_str()));
        if (!h3) continue;

        for (size_t j = 0; j < ptRanges.size(); ++j) {
            double pmin = ptRanges[j].first;
            double pmax = ptRanges[j].second;

            // Rajataan Z-akseli (eli p-momentum) kyseiseen alueeseen
            h3->GetZaxis()->SetRange(h3->GetZaxis()->FindBin(pmin + 1e-3),
                                     h3->GetZaxis()->FindBin(pmax - 1e-3));
            // Projektoidaan jäljelle jäävä 3D-histogrammi 2D:ksi (X vs Y)
            auto proj2D = dynamic_cast<TH2D*>(h3->Project3D("xy"));
            std::string proj2D_name = name3d + "_proj2D_" + ptTags[j];

            // Asetetaan akselit ja otsikot
            proj2D->SetName(proj2D_name.c_str());
            proj2D->SetMinimum(1e-6); // log-plotin alkuarvo
            proj2D->SetTitle(Form("E/p vs E_{HCAL}/E_{HCAL}^{raw}, %s (%.1f - %.1f GeV)", labels[i].c_str(), pmin, pmax));
            proj2D->GetXaxis()->SetTitle("E_{HCAL}/E_{HCAL}^{raw}");
            proj2D->GetYaxis()->SetTitle("E/p");

            // Piirretään ja tallennetaan kuva
            TCanvas* c2D = new TCanvas(("canvas_" + proj2D_name).c_str(), proj2D_name.c_str(), 800, 600);
            proj2D->GetXaxis()->SetRangeUser(0.9, 2.5); // rajoitetaan x-akseli
            proj2D->Draw("COLZ");
            c2D->SaveAs((outdir + proj2D_name + ".png").c_str());
            proj2D->Write();
            c2D->Write();
        }
    }

        // ==========================================
        // E/p-jakaumat tietyillä p-alueilla, fitataan Gaussin käyrä
        // ==========================================

        // Valitaan pT-alueet, joilla projektiot tehdään
        std::vector<std::pair<double, double>> pBins = {{5.5, 6.0}, {20.0, 22.0}};
        
        // Haetaan 2D-histogrammit: E/p vs p
        TH2D* h2_ep = dynamic_cast<TH2D*>(file->Get("h2_ep_vs_p_iso"));  // korjattu E/p
        TH2D* h2_raw = dynamic_cast<TH2D*>(file->Get("h2_raw_ep_vs_p_iso")); // raakadata

        // Tarkistetaan, että histogrammit löytyvät
        if (h2_ep && h2_raw) {
            for (const auto& [pmin, pmax] : pBins) {

                // Määritetään projektioalue X-akselilla (p)
                int bin_min = h2_ep->GetXaxis()->FindBin(pmin + 1e-3);
                int bin_max = h2_ep->GetXaxis()->FindBin(pmax - 1e-3);

                // Luodaan histogrammien nimet merkkijonona
                std::stringstream sn, sr;
                sn << "proj_ep_" << int(pmin * 10) << "_" << int(pmax * 10); // esim. proj_ep_55_60
                sr << "proj_raw_ep_" << int(pmin * 10) << "_" << int(pmax * 10); // esim. proj_raw_ep_55_60

                // Projektoidaan 2D-histogrammista E/p y-akselilla p-alueelle
                TH1D* proj_n = h2_ep->ProjectionY(sn.str().c_str(), bin_min, bin_max);
                TH1D* proj_r = h2_raw->ProjectionY(sr.str().c_str(), bin_min, bin_max);

                // Jos korjattu histogrammi löytyy ja ei ole tyhjä fitataan rajoitetulla alueella
                if (proj_n && proj_n->Integral() > 0) {
                    proj_n->Scale(1.0 / proj_n->Integral()); // normalisoi histogrammi
                    TF1* fit_n = new TF1((sn.str() + "_fit").c_str(), "gaus", 0.0, 2.5);
                    proj_n->Fit(fit_n, "RQ", "", 0.7, 1.2);

                    // Asetetaan otsikot ja akselit
                    proj_n->SetTitle(Form("E/p in %.1f - %.1f GeV", pmin, pmax));
                    proj_n->GetXaxis()->SetTitle("E/p");
                    proj_n->GetYaxis()->SetTitle("Fraction of particles");

                    // Piirretään ja tallennetaan kuva
                    TCanvas* c_n = new TCanvas(("canvas_" + sn.str()).c_str(), sn.str().c_str(), 800, 600);
                    proj_n->SetFillColorAlpha(kBlue, 0.35);
                    proj_n->Draw("hist");
                    fit_n->SetLineColor(kRed);
                    fit_n->Draw("same");
                    c_n->SaveAs((outdir + sn.str() + ".png").c_str());
                    c_n->Write();
                }

                // Sama raakadatalle (ilman korjausta)
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



    // ==========================================
    // Fitted mean (ilman HCAL-korjausleikkausta), TGraph muodossa
    // ==========================================

    // Luodaan TGraph fitted-mean-arvoille
    TGraph* g_fit = new TGraph();

    // Määritellään pT-alueet ja niiden keskikohdat
    std::vector<std::pair<double, double>> fit_pBins = {{5.5, 6.0}, {10.0, 12.0}, {20.0, 22.0}};
    std::vector<double> pMid = {5.75, 11.0, 21.0};

    // Varmistetaan, että h2_ep on olemassa
    if (h2_ep) {
        for (size_t i = 0; i < fit_pBins.size(); ++i) {
            double pmin = fit_pBins[i].first;
            double pmax = fit_pBins[i].second;

            // Binirajat p-alueelle
            int bin_min = h2_ep->GetXaxis()->FindBin(pmin + 1e-3);
            int bin_max = h2_ep->GetXaxis()->FindBin(pmax - 1e-3);

            // Projektion nimi
            std::string projName = Form("proj_fit_ep_nocut_%zu", i);
            TH1D* hproj = h2_ep->ProjectionY(projName.c_str(), bin_min, bin_max);

            // Jos histogrammi on tyhjä, ohitetaan
            if (!hproj || hproj->Integral() == 0) continue;

            hproj->Scale(1.0 / hproj->Integral()); // normalisointi

            // Fitataan Gaussin käyrä E/p-jakaumaan
            TF1* fit = new TF1((projName + "_fit").c_str(), "gaus", 0.7, 1.2);
            hproj->Fit(fit, "RQ");

            // Lisätään fitted-mean piste graafiin
            g_fit->SetPoint(g_fit->GetN(), pMid[i], fit->GetParameter(1));

            // Tallennetaan sovitusfunktio ROOT-tiedostoon (ei pakollista)
            fit->Write();  // voi poistaa tämän jos ei tarvita
        }
    }

    // ===============================
    // Fitted-meanit HCAL-korjauksen jälkeen
    // ===============================

    // Määritellään pT-välit ja niiden tagit (low, medium, high)
    std::vector<std::pair<double, double>> hcalcut_pBins = {{5.5, 6.5}, {10.0, 12.0}, {20.0, 24.0}};
    std::vector<std::string> tags = {"low", "medium", "high"};

    // TGraph fitted-mean pisteiden piirtämistä varten (HCAL-korjauksella)
    TGraph* g_fit_hcalcut = new TGraph();

    // Ladataan histogrammi HCAL only -hadronityypille (esim. pionit, jotka tallettavat energiansa pääosin HCALiin)
    TH3D* h3 = dynamic_cast<TH3D*>(file->Get("h3_resp_corr_p_isHadH"));
    if (h3) {
        for (size_t i = 0; i < hcalcut_pBins.size(); ++i) {
            double pmin = hcalcut_pBins[i].first, pmax = hcalcut_pBins[i].second;
            double pmid = pMid[i]; // Vastaa keskimääräistä pT-arvoa TGraph-pistettä varten

            // Määritetään HCAL-korjauksen leikkausalue (0.9 < E_HCAL / E_HCAL^raw < 2.5)
            int y_min = h3->GetYaxis()->FindBin(0.9001); // pieni epsilon estää reunabinit
            int y_max = h3->GetYaxis()->FindBin(2.4999);

            // Määritetään p-alue Z-akselilla
            int z_min = h3->GetZaxis()->FindBin(pmin + 1e-3);
            int z_max = h3->GetZaxis()->FindBin(pmax - 1e-3);

            std::string projName = "proj_resp_cut_" + tags[i];
            // Projektio X-akselille (E/p), leikkaamalla sopivilla Y- ja Z-akselin rajoilla
            TH1D* hproj = h3->ProjectionX(projName.c_str(), y_min, y_max, z_min, z_max);

            if (!hproj || hproj->Integral() == 0) {
                std::cout << "[INFO] Skip empty projection: " << projName << std::endl;
                continue;
            }

            // Normalisointi
            hproj->Scale(1.0 / hproj->Integral());

            // Gauss-fit
            TF1* fit = new TF1((projName + "_fit").c_str(), "gaus", 0.0, 2.5);
            hproj->Fit(fit, "RQ");

            // Lisätään fitted-mean (muuttuja: keskikohta, eli fit->GetParameter(1)) TGraphiin)
            g_fit_hcalcut->SetPoint(g_fit_hcalcut->GetN(), pmid, fit->GetParameter(1));

            // Piirretään histogrammi ja fit
            TCanvas* c = new TCanvas(("canvas_" + projName).c_str(), "", 800, 600);
            hproj->SetTitle(Form("E/p for HCALcut, %s (%.1f - %.1f GeV)", tags[i].c_str(), pmin, pmax));
            hproj->GetXaxis()->SetTitle("E/p");
            hproj->GetYaxis()->SetTitle("Fraction of particles");
            hproj->Draw("HIST");
            fit->SetLineColor(kRed);
            fit->Draw("same");

            // Tallennus
            c->SaveAs((outdir + projName + ".png").c_str());
            c->Write();
            fit->Write();
        }

        // Resetoi mahdolliset axis-ranget seuraavaa käyttöä varten
        h3->GetYaxis()->UnZoom();
        h3->GetZaxis()->UnZoom();
    }

    // ===============================
    // Vertailuplotti: profiili vs. fitted-mean vs. HCAL-cut fitted-mean
    // ===============================

    // Luo uusi canvas
    TCanvas* c = new TCanvas("canvas_compare_fit_vs_profile", "", 800, 600);
    c->SetGrid();  // taustalle ruudukko

    // Piilotetaan statistiikkalaatikko
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



    // ===============================
    // E/p-jakaumat per hadronityyppi kaikissa pT-bineissä (ei stack, ei overlay)
    // ===============================

    // Määritellään koko pT-binnitys
    const double trkPBins_full[] = {
        3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
        7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
        16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
    };
    const int nTrkPBins_full = sizeof(trkPBins_full) / sizeof(double) - 1;

    // Hadronityypit ja labelit (käytetään tiedostonimissä ja otsikoissa)
    std::vector<std::pair<std::string, std::string>> hadronTypesWithLabel = {
        {"isHadH", "HCALonly"},
        {"isHadE", "ECALonly"},
        {"isHadMIP", "MIP"},
        {"isHadEH", "ECALHCAL"}
    };

    // Näytetään fit-parametrit kuvan yhteydessä
    gStyle->SetOptFit(111);

    for (const auto& [type, label] : hadronTypesWithLabel) {
        // Ladataan oikea 3D-histogrammi
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

            // Fitataan useammalla Gaussilla (kolmen komponentin malli)
            std::string fitName = "fit_" + projName;
            TF1* fit = new TF1(fitName.c_str(), "gaus(0) + gaus(3) + gaus(6)", 0.3, 2.0);
            fit->SetParameters(0.2, 0.8, 0.1, 0.1, 1.0, 0.1, 0.05, 1.3, 0.2);
            fit->SetLineColor(kRed);
            fit->SetLineWidth(2);

            hproj->Fit(fit, "RQ");
            hproj->Draw("HIST");

            // Jos fitti onnistui, piirretään se
            TF1* fit_result = hproj->GetFunction(fitName.c_str());
            if (fit_result) {
                fit_result->SetLineColor(kRed);
                fit_result->SetLineWidth(2);
                fit_result->Draw("SAME");
            }

            // Tallennus
            c->SaveAs((outdir + projName + ".png").c_str());
            c->Write();
        }

        // Resetoi zoomaukset
        h3->GetYaxis()->UnZoom();
        h3->GetZaxis()->UnZoom();
    }

    // ===============================
    // STACKED E/p DISTRIBUTIONS FOR EACH pT BIN
    // ===============================

    // Määritellään pT-binnit (täsmää histogrammin Z-akseliin)
    const double trkPBins[] = {
        3.0, 3.3, 3.6, 3.9, 4.2, 4.6, 5.0, 5.5, 6.0, 6.6,
        7.2, 7.9, 8.6, 9.3, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0,
        16.5, 18.0, 20.0, 22.0, 25.0, 30.0, 35.0, 40.0, 45.0, 55.0, 70.0, 130.0
    };
    const int nTrkPBins = sizeof(trkPBins) / sizeof(double) - 1;

    if (h3_H && h3_E && h3_MIP && h3_EH) {
        // HCAL correction cut Y-akselilla
        int y_min = h3_H->GetYaxis()->FindBin(0.9001);
        int y_max = h3_H->GetYaxis()->FindBin(2.4999);

        // Laske kokonaismäärä
        for (int i = 0; i < nTrkPBins; ++i) {
            double pmin = trkPBins[i];
            double pmax = trkPBins[i + 1];

            int z_min = h3_H->GetZaxis()->FindBin(pmin + 1e-3);
            int z_max = h3_H->GetZaxis()->FindBin(pmax - 1e-3);
            
            // Projektio E/p-akselille (X) jokaiselle hadronityypille
            TH1D* h_H   = h3_H  ? h3_H->ProjectionX(Form("h_H_%d", i), y_min, y_max, z_min, z_max) : nullptr;
            TH1D* h_E   = h3_E  ? h3_E->ProjectionX(Form("h_E_%d", i), y_min, y_max, z_min, z_max) : nullptr;
            TH1D* h_MIP = h3_MIP? h3_MIP->ProjectionX(Form("h_MIP_%d", i), y_min, y_max, z_min, z_max) : nullptr;
            TH1D* h_EH  = h3_EH ? h3_EH->ProjectionX(Form("h_EH_%d", i), y_min, y_max, z_min, z_max) : nullptr;
            
            // Kokonaissumman lasku normalisointia varten
            double total = 0.0;
                if (h_H)   total += h_H->Integral();
                if (h_E)   total += h_E->Integral();
                if (h_MIP) total += h_MIP->Integral();
                if (h_EH)  total += h_EH->Integral();
                if (total == 0) continue;

                // Luo stacked histogrammi (THStack)
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

                // Piirretään stack ja legenda
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

                // Fitataan kokonaissumman E/p-jakauma (summa kaikista tyypeistä)
                if (h_H && h_E && h_MIP && h_EH) {
                TH1D* h_sum = (TH1D*)h_H->Clone(Form("h_sum_%d", i));
                h_sum->Add(h_E);
                h_sum->Add(h_MIP);
                h_sum->Add(h_EH);
                h_sum->Scale(1.0 / h_sum->Integral());

                // Fittaus esim. Gaussilla
                TF1* fit = new TF1(Form("fit_%d", i), "gaus(0)", 0.3, 2.0); // tai sopivampi funktio!
                fit->SetLineColor(kBlack);
                fit->SetLineWidth(2);
                h_sum->Fit(fit, "RQ");

                // Piirrä fitti
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

        // Palautetaan zoomaukset
        h3_H->GetYaxis()->UnZoom();
        h3_H->GetZaxis()->UnZoom();
    }

    // ===============================
// FRAKTIOPLOTTI: HADRONITYYPIT p:n FUNKTIONA
// ===============================

    // Ladataan hadronifraktioiden 1D-histogrammit
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

        // Piirto
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

    // ===============================
    // HADRONITYYPPIEN FRAKTIOT: PINOTTU HISTOGRAMMI
    // ===============================

    std::cout << "Drawing stacked hadron fraction plot..." << std::endl;

    // Luodaan uudet TH1D:t samoilla pT-bineillä
    TH1D* h_H_bin   = new TH1D("h_H_bin", "Hadron fractions", nTrkPBins, trkPBins);
    TH1D* h_E_bin   = new TH1D("h_E_bin", "Hadron fractions", nTrkPBins, trkPBins);
    TH1D* h_MIP_bin = new TH1D("h_MIP_bin", "Hadron fractions", nTrkPBins, trkPBins);
    TH1D* h_EH_bin  = new TH1D("h_EH_bin", "Hadron fractions", nTrkPBins, trkPBins);

    // Täytetään fraktiotiedot aiemmin tehdyistä 1D-histogrammeista
    for (int i = 1; i <= h_H_bin->GetNbinsX(); ++i) {
        h_H_bin->SetBinContent(i, h_frac_H->GetBinContent(i));
        h_E_bin->SetBinContent(i, h_frac_E->GetBinContent(i));
        h_MIP_bin->SetBinContent(i, h_frac_MIP->GetBinContent(i));
        h_EH_bin->SetBinContent(i, h_frac_EH->GetBinContent(i));
    }

    // Asetetaan värit (sama kuin aiemmissa stack-ploteissa)
    h_H_bin->SetFillColor(kRed);
    h_E_bin->SetFillColor(kBlue);
    h_MIP_bin->SetFillColor(kMagenta);
    h_EH_bin->SetFillColor(kGreen+2);

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

    //Legenda
    TLegend* leg_stack = new TLegend(0.70, 0.65, 0.88, 0.88);
    leg_stack->AddEntry(h_H_bin, "HCAL only", "f");
    leg_stack->AddEntry(h_E_bin, "ECAL only", "f");
    leg_stack->AddEntry(h_MIP_bin, "MIP", "f");
    leg_stack->AddEntry(h_EH_bin, "ECAL+HCAL", "f");
    leg_stack->Draw();

    // Tallennus
    c_stack->SaveAs((outdir + "/fraction_stacked.png").c_str());
    c_stack->Write();

    // Suljetaan ROOT -tiedostot
    fout->Close();
    file->Close();
}


// stacked vertailuplotti hadronifraktioista
// Piirtää rinnakkain stacked histogrammit hadronikomponenttien fraktioista MC- ja data-aineistosta.
// Vasemmalle piirtyy MC:n stacked histogrammi ja datapisteet päälle,
// oikealle piirtyy datan stacked histogrammi.
// Fraktiot: HCAL only, MIP, ECAL+HCAL, ECAL only

void plot_stacked_comparison() {
    // Avaa ROOT-tiedostot
    TFile* f1 = new TFile("histograms/SingleNeutrino2024_no_cut.root");   // MC
    TFile* f2 = new TFile("histograms/ZeroBias2024_without_cut.root");     // Data

    // MC-profiilit
    auto h_H   = (TProfile*)f1->Get("h_frac_H"); // HCAL only
    auto h_E   = (TProfile*)f1->Get("h_frac_E"); // ECAL only
    auto h_MIP = (TProfile*)f1->Get("h_frac_MIP"); // MIP
    auto h_EH  = (TProfile*)f1->Get("h_frac_EH"); // ECAL+HCAL

    // Data-profiilit
    auto d_H   = (TProfile*)f2->Get("h_frac_H");
    auto d_E   = (TProfile*)f2->Get("h_frac_E");
    auto d_MIP = (TProfile*)f2->Get("h_frac_MIP");
    auto d_EH  = (TProfile*)f2->Get("h_frac_EH");

    // Luo pinotut histogrammit MC:lle ja datalle
    auto stack1 = new THStack("stack1", "SingleNeutrino2024_no_cut (corrected);Track momentum p (GeV);Fraction");
    auto stack2 = new THStack("stack2", "ZeroBias2024_without_cut (corrected);Track momentum p (GeV);Fraction");

    // Apu-funktio: luo binitetyt histogrammit profiileista ja lisää ne stackiin
    auto add_to_stack = [](THStack* stack, TProfile* hH, TProfile* hMIP, TProfile* hEH, TProfile* hE) {
        auto hH_bin   = hH->ProjectionX(Form("%s_bin", hH->GetName()));
        auto hMIP_bin = hMIP->ProjectionX(Form("%s_bin", hMIP->GetName()));
        auto hEH_bin  = hEH->ProjectionX(Form("%s_bin", hEH->GetName()));
        auto hE_bin   = hE->ProjectionX(Form("%s_bin", hE->GetName()));

        // Aseta värit selkeästi (HCAL=pun, MIP=violetti, ECAL+HCAL=vihreä, ECAL=sininen)
        hH_bin->SetFillColor(kRed);
        hMIP_bin->SetFillColor(kMagenta);
        hEH_bin->SetFillColor(kGreen+2);
        hE_bin->SetFillColor(kBlue);

        // Lisää oikeassa järjestyksessä: pohjalle HCAL, päälle ECAL
        stack->Add(hH_bin);
        stack->Add(hMIP_bin);
        stack->Add(hEH_bin);
        stack->Add(hE_bin);
    };

    // Lisää stackit
    add_to_stack(stack1, h_H, h_MIP, h_EH, h_E); // MC
    add_to_stack(stack2, d_H, d_MIP, d_EH, d_E); // Data

    // Luo canvas ja jaa kahteen ruutuun
    TCanvas* c = new TCanvas("c_compare", "Hadron fractions comparison", 1200, 600);
    c->Divide(2, 1);

    // === 1. Vasen ruutu: MC stacked + datapisteet ===
    c->cd(1);
    stack1->Draw("HIST");
    stack1->SetMinimum(0);
    stack1->SetMaximum(1.0); // fraktiot [0,1]


    int nbins = d_H->GetNbinsX(); // oletetaan sama binitys

     // Luo TGraphErrors objektit kullekin komponentille (datan päällepiirtoon)
    TGraphErrors* g_H   = new TGraphErrors(nbins);
    TGraphErrors* g_MIP = new TGraphErrors(nbins);
    TGraphErrors* g_EH  = new TGraphErrors(nbins);
    TGraphErrors* g_E   = new TGraphErrors(nbins);

    // Lasketaan stacked datapisteet kumulatiivisesti joka komponentille
    for (int i = 1; i <= nbins; ++i) {
        double x = d_H->GetBinCenter(i);
        double y_H   = d_H->GetBinContent(i);
        double y_MIP = y_H   + d_MIP->GetBinContent(i);
        double y_EH  = y_MIP + d_EH->GetBinContent(i);
        double y_E   = y_EH  + d_E->GetBinContent(i);

        double err_H   = d_H->GetBinError(i);
        double err_MIP = d_MIP->GetBinError(i);
        double err_EH  = d_EH->GetBinError(i);
        double err_E   = d_E->GetBinError(i);

        g_H  ->SetPoint(i-1, x, y_H);   g_H  ->SetPointError(i-1, 0, err_H);
        g_MIP->SetPoint(i-1, x, y_MIP); g_MIP->SetPointError(i-1, 0, err_MIP);
        g_EH ->SetPoint(i-1, x, y_EH);  g_EH ->SetPointError(i-1, 0, err_EH);
        g_E  ->SetPoint(i-1, x, y_E);   g_E  ->SetPointError(i-1, 0, err_E);
    }

    // Mustat markerit
    g_H  ->SetMarkerStyle(20); g_H  ->SetMarkerColor(kBlack); g_H  ->Draw("P SAME");
    g_MIP->SetMarkerStyle(21); g_MIP->SetMarkerColor(kBlack); g_MIP->Draw("P SAME");
    g_EH ->SetMarkerStyle(22); g_EH ->SetMarkerColor(kBlack); g_EH ->Draw("P SAME");
    g_E  ->SetMarkerStyle(23); g_E  ->SetMarkerColor(kBlack); g_E  ->Draw("P SAME");

    // Legenda vasempaan alakulmaan
    auto leg1 = new TLegend(0.5, 0.15, 0.85, 0.43);
    leg1->AddEntry((TObject*)0, "MC: stacked", "");
    leg1->AddEntry(stack1->GetHists()->At(0), "HCAL only", "f");
    leg1->AddEntry(stack1->GetHists()->At(1), "MIP", "f");
    leg1->AddEntry(stack1->GetHists()->At(2), "ECAL+HCAL", "f");
    leg1->AddEntry(stack1->GetHists()->At(3), "ECAL only", "f");
    leg1->AddEntry((TObject*)0, "Data: markers", "");
    leg1->AddEntry(g_H, "HCAL only", "p");
    leg1->AddEntry(g_MIP, "MIP", "p");
    leg1->AddEntry(g_EH, "ECAL+HCAL", "p");
    leg1->AddEntry(g_E, "ECAL only", "p");
    leg1->Draw();

    // === 2. Oikea ruutu: datan stacked histogrammi ===
    c->cd(2);
    stack2->Draw("HIST");
    stack2->SetMinimum(0);
    stack2->SetMaximum(1.0);

    // Legenda oikeaan alakulmaan
    auto leg2 = new TLegend(0.7, 0.15, 0.9, 0.43);
    leg2->AddEntry((TObject*)0, "Data: stacked", "");
    leg2->AddEntry(stack2->GetHists()->At(0), "HCAL only", "f");
    leg2->AddEntry(stack2->GetHists()->At(1), "MIP", "f");
    leg2->AddEntry(stack2->GetHists()->At(2), "ECAL+HCAL", "f");
    leg2->AddEntry(stack2->GetHists()->At(3), "ECAL only", "f");
    leg2->Draw();

    // Tallenna kuva
    c->SaveAs("plots/stack_frac_comparison.png");

    // Sulje tiedostot
    f1->Close();
    f2->Close();
}







// Vertailee E/p-jakaumia datan ja simulaation välillä sekä fittaa huipun Crystal Ball -funktiolla

// ===============================================
// Piirrä E/p-jakaumat datalle ja MC:lle sekä fittaa ne Crystal Ball -funktiolla
// - Normalisoi jakaumat yksikköön (vertailukelpoisuus muodon osalta)
// - Fittaa molemmat käyrät (MC ja Data) käyttäen Crystal Ball -funktiota
// - Tulostaa fitted-parametrit: peak (mean), sigma ja skaalakerroin data/MC
// - Visualisoi tulokset yhdessä kuvassa (histogrammit + fitit + annotaatiot)
//
// Syötteenä ROOT-tiedostot, joissa histogrammi "h_ep_iso"
// ===============================================

void plot_ep_overlay() {
    // Avaa ROOT-tiedostot
    TFile* f_mc = new TFile("histograms/SingleNeutrino2024_no_cut.root"); // MC
    TFile* f_data = new TFile("histograms/ZeroBias2024_without_cut.root"); // Data

    // Tarkistaa, että tiedostot ja histogrammit avattiin oikein/ovat kunnossa
    if (!f_mc || !f_data || f_mc->IsZombie() || f_data->IsZombie()) {
        std::cerr << "ERROR: Could not open input ROOT files." << std::endl;
        return;
    }

    // Hakee histogrammit nimellä "h_ep_iso" (oletetaan, että ne ovat olemassa molemmissa tiedostoissa)
    // Jos histogrammeja ei löydy tai ne ovat tyhiä, tulostaa virheilmoituksen ja lopettaa funktion suorittamisen
    auto h_mc = (TH1D*)f_mc->Get("h_ep_iso");
    auto h_data = (TH1D*)f_data->Get("h_ep_iso");

    if (!h_mc || !h_data) {
        std::cerr << "ERROR: Could not find h_ep_iso in input files." << std::endl;
        return;
    }

    // Normalisointi (alue = 1)
    h_mc->Scale(1.0 / h_mc->Integral());
    h_data->Scale(1.0 / h_data->Integral());

    // Asetetaan histogrammien ulkoasu
    h_mc->SetLineColor(kBlack);
    h_mc->SetLineWidth(2);
    h_mc->SetLineStyle(1);
    h_mc->SetTitle("E/p comparison between MC and Data;E/p;Normalized entries");

    h_data->SetLineColor(kBlack);
    h_data->SetLineWidth(2);
    h_data->SetLineStyle(2); // katkoviiva

    // Crystal Ball -fitit koko x-alueelle
    TF1* fit_mc = new TF1("fit_mc", "crystalball", 0.0, 3.0);
    TF1* fit_data = new TF1("fit_data", "crystalball", 0.0, 3.0);

     // Alkuarvot: [0]=norm, [1]=mean, [2]=sigma, [3]=alpha, [4]=n
    fit_mc->SetParameters(1.0, 1.0, 0.1, 1.5, 2.0);
    fit_data->SetParameters(1.0, 1.0, 0.1, 1.5, 2.0);

    // Fitataan histogrammit
    // "RQ0" tarkoittaa, että fitataan ilman alussa olevaa nollaa ja ilman alussa olevaa nollapistettä
    // Tämä on hyödyllistä, jos histogrammi ei ole nolla alussa
    h_mc->Fit(fit_mc, "RQ0");
    h_data->Fit(fit_data, "RQ0");

    // Tulostaa fitted-parametrit ja skaalakertoinet
    double peak_mc = fit_mc->GetParameter(1);
    double sigma_mc = fit_mc->GetParameter(2);
    double peak_data = fit_data->GetParameter(1);
    double sigma_data = fit_data->GetParameter(2);
    double scale = peak_data / peak_mc;

    // Tulostaa fitted-parametrit konsoliin
    std::cout << "=== E/p peak fit results (Crystal Ball) ===" << std::endl;
    std::cout << "MC:    peak = " << peak_mc << ", sigma = " << sigma_mc << std::endl;
    std::cout << "Data:  peak = " << peak_data << ", sigma = " << sigma_data << std::endl;
    std::cout << "Scale factor (data/MC): " << scale << std::endl;

    // Kuvan piirto
    TCanvas* c = new TCanvas("c_ep_overlay", "E/p MC vs Data", 800, 600);
    h_mc->SetStats(0);
    h_mc->GetXaxis()->SetRangeUser(0.0, 3.0);
    h_mc->GetYaxis()->SetRangeUser(0.0, 0.3);

    h_mc->Draw("HIST"); // piirretään MC histogrammi
    h_data->Draw("HIST SAME"); // piirretään Data histogrammi päälle

    // Piirtää fitit
    fit_mc->SetLineColor(kRed + 1);
    fit_mc->SetLineWidth(2);
    fit_mc->SetLineStyle(1);
    fit_mc->Draw("SAME");

    fit_data->SetLineColor(kGreen + 2);
    fit_data->SetLineWidth(2);
    fit_data->SetLineStyle(2);
    fit_data->Draw("SAME");

    // Legenda
    auto leg = new TLegend(0.55, 0.72, 0.88, 0.88);
    leg->SetBorderSize(0);
    leg->AddEntry(h_mc, "SingleNeutrino (MC)", "l");
    leg->AddEntry(h_data, "ZeroBias (Data)", "l");
    leg->AddEntry(fit_mc, "MC fit (Crystal Ball)", "l");
    leg->AddEntry(fit_data, "Data fit (Crystal Ball)", "l");
    leg->Draw();

    // Tekstituloste fitted-arvoista
    TLatex latex;
    latex.SetNDC(); // Normalized Device Coordinates
    latex.SetTextSize(0.032);
    latex.DrawLatex(0.60, 0.48, Form("MC peak = %.3f", peak_mc));
    latex.DrawLatex(0.60, 0.43, Form("Data peak = %.3f", peak_data));
    latex.DrawLatex(0.60, 0.38, Form("Scale factor = %.3f", scale));

    // Tallennetaan kuva
    c->SaveAs("plots/ep_overlay_mc_vs_data_fit_CB_annotated.png");

    // Suljetaan tiedostot
    f_mc->Close();
    f_data->Close();
}



// void plot_ep_scale_vs_pt() {
//     const int nbins = 6;
//     const double pt_bin_centers[nbins] = {4.5, 6.5, 8.5, 11.0, 15.0, 25.0}; // Säädä tarvittaessa

//     std::vector<double> v_pt, v_scale, v_err;

//     TFile* f_mc = TFile::Open("histograms/SingleNeutrino2024.root");
//     TFile* f_data = TFile::Open("histograms/ZeroBias2024all2.root");
//     if (!f_mc || !f_data || f_mc->IsZombie() || f_data->IsZombie()) {
//         std::cerr << "ERROR: Could not open input ROOT files." << std::endl;
//         return;
//     }

//     for (int i = 0; i < nbins; ++i) {
//         TString name = Form("h_ep_bin%d", i);
//         auto h_mc = dynamic_cast<TH1D*>(f_mc->Get(name));
//         auto h_data = dynamic_cast<TH1D*>(f_data->Get(name));

//         if (!h_mc || !h_data) {
//             std::cerr << "WARNING: Histogram " << name << " not found. Skipping.\n";
//             continue;
//         }

//         if (h_mc->Integral() == 0 || h_data->Integral() == 0) {
//             std::cerr << "WARNING: Histogram " << name << " has zero integral. Skipping.\n";
//             continue;
//         }

//         h_mc->Scale(1.0 / h_mc->Integral());
//         h_data->Scale(1.0 / h_data->Integral());

//         TF1 fit_mc("fit_mc", "crystalball", 0.0, 3.0);
//         TF1 fit_data("fit_data", "crystalball", 0.0, 3.0);
//         fit_mc.SetParameters(1.0, 1.0, 0.1, 1.5, 2.0);
//         fit_data.SetParameters(1.0, 1.0, 0.1, 1.5, 2.0);

//         h_mc->Fit(&fit_mc, "RQ0");
//         h_data->Fit(&fit_data, "RQ0");

//         double peak_mc = fit_mc.GetParameter(1);
//         double peak_data = fit_data.GetParameter(1);
//         double err_mc = fit_mc.GetParError(1);
//         double err_data = fit_data.GetParError(1);

//         double scale = peak_data / peak_mc;
//         double scale_err = scale * std::sqrt(
//             std::pow(err_data / peak_data, 2) + std::pow(err_mc / peak_mc, 2));

//         v_pt.push_back(pt_bin_centers[i]);
//         v_scale.push_back(scale);
//         v_err.push_back(scale_err);
//     }

//     // Plotting
//     TCanvas* c = new TCanvas("c_scale_vs_pt", "Scale factor vs. p_{T}", 800, 600);
//     TGraphErrors* gr = new TGraphErrors(v_pt.size(), v_pt.data(), v_scale.data(), nullptr, v_err.data());
//     gr->SetTitle("Data/MC scale factor vs. p_{T};p_{T} [GeV];Scale factor (Data / MC)");
//     gr->SetMarkerStyle(20);
//     gr->SetMarkerSize(1.2);
//     gr->SetMarkerColor(kBlue + 1);
//     gr->SetLineColor(kBlue + 1);
//     gr->Draw("AP");

//     c->SaveAs("plots/scale_factor_vs_pt.png");

//     // Optional: save graph to file
//     TFile* fout = TFile::Open("plots/scale_factors.root", "RECREATE");
//     gr->Write("g_scale_vs_pt");
//     fout->Close();

//     f_mc->Close();
//     f_data->Close();
// }


void plot_stacked_comparison_raw() {
    // Avaa ROOT-tiedostot
    TFile* f1 = new TFile("histograms/SingleNeutrino2024_no_cut.root");   // MC
    TFile* f2 = new TFile("histograms/ZeroBias2024_without_cut.root");     // Data

    // MC-profiilit (raw)
    auto h_H   = (TProfile*)f1->Get("h_raw_frac_H");
    auto h_E   = (TProfile*)f1->Get("h_raw_frac_E");
    auto h_MIP = (TProfile*)f1->Get("h_raw_frac_MIP");
    auto h_EH  = (TProfile*)f1->Get("h_raw_frac_EH");

    // Data-profiilit (raw)
    auto d_H   = (TProfile*)f2->Get("h_raw_frac_H");
    auto d_E   = (TProfile*)f2->Get("h_raw_frac_E");
    auto d_MIP = (TProfile*)f2->Get("h_raw_frac_MIP");
    auto d_EH  = (TProfile*)f2->Get("h_raw_frac_EH");

    // Luo pinotut histogrammit MC:lle ja datalle
    auto stack1 = new THStack("stack1_raw", "SingleNeutrino2024_no_cut (raw);Track momentum p (GeV);Fraction");
    auto stack2 = new THStack("stack2_raw", "ZeroBias2024_without_cut (raw);Track momentum p (GeV);Fraction");

    // Apufunktio profiilien konvertointiin ja värien asettamiseen
    auto add_to_stack = [](THStack* stack, TProfile* hH, TProfile* hMIP, TProfile* hEH, TProfile* hE) {
        auto hH_bin   = hH->ProjectionX(Form("%s_bin", hH->GetName()));
        auto hMIP_bin = hMIP->ProjectionX(Form("%s_bin", hMIP->GetName()));
        auto hEH_bin  = hEH->ProjectionX(Form("%s_bin", hEH->GetName()));
        auto hE_bin   = hE->ProjectionX(Form("%s_bin", hE->GetName()));

        hH_bin->SetFillColor(kRed);
        hMIP_bin->SetFillColor(kMagenta);
        hEH_bin->SetFillColor(kGreen + 2);
        hE_bin->SetFillColor(kBlue);

        stack->Add(hH_bin);
        stack->Add(hMIP_bin);
        stack->Add(hEH_bin);
        stack->Add(hE_bin);
    };

    // Lisää data stackeihin
    add_to_stack(stack1, h_H, h_MIP, h_EH, h_E); // MC
    add_to_stack(stack2, d_H, d_MIP, d_EH, d_E); // Data

    // Luo canvas ja jaa kahteen ruutuun
    TCanvas* c = new TCanvas("c_compare_raw", "Hadron fractions comparison (raw)", 1200, 600);
    c->Divide(2, 1);

    // === 1. Vasen ruutu: MC stacked + datan markerit ===
    c->cd(1);
    stack1->Draw("HIST");
    stack1->SetMinimum(0);
    stack1->SetMaximum(1.0);

    int nbins = d_H->GetNbinsX(); // oletetaan sama binitys

    // Luo TGraphErrors datan markeroinneille
    TGraphErrors* g_H   = new TGraphErrors(nbins);
    TGraphErrors* g_MIP = new TGraphErrors(nbins);
    TGraphErrors* g_EH  = new TGraphErrors(nbins);
    TGraphErrors* g_E   = new TGraphErrors(nbins);

    for (int i = 1; i <= nbins; ++i) {
        double x = d_H->GetBinCenter(i);
        double y_H   = d_H->GetBinContent(i);
        double y_MIP = y_H   + d_MIP->GetBinContent(i);
        double y_EH  = y_MIP + d_EH->GetBinContent(i);
        double y_E   = y_EH  + d_E->GetBinContent(i);

        double err_H   = d_H->GetBinError(i);
        double err_MIP = d_MIP->GetBinError(i);
        double err_EH  = d_EH->GetBinError(i);
        double err_E   = d_E->GetBinError(i);

        g_H  ->SetPoint(i - 1, x, y_H);   g_H  ->SetPointError(i - 1, 0, err_H);
        g_MIP->SetPoint(i - 1, x, y_MIP); g_MIP->SetPointError(i - 1, 0, err_MIP);
        g_EH ->SetPoint(i - 1, x, y_EH);  g_EH ->SetPointError(i - 1, 0, err_EH);
        g_E  ->SetPoint(i - 1, x, y_E);   g_E  ->SetPointError(i - 1, 0, err_E);
    }

    g_H  ->SetMarkerStyle(20); g_H  ->SetMarkerColor(kBlack); g_H  ->Draw("P SAME");
    g_MIP->SetMarkerStyle(21); g_MIP->SetMarkerColor(kBlack); g_MIP->Draw("P SAME");
    g_EH ->SetMarkerStyle(22); g_EH ->SetMarkerColor(kBlack); g_EH ->Draw("P SAME");
    g_E  ->SetMarkerStyle(23); g_E  ->SetMarkerColor(kBlack); g_E  ->Draw("P SAME");

    // Legenda
    auto leg1 = new TLegend(0.5, 0.15, 0.85, 0.43);
    leg1->AddEntry((TObject*)0, "MC: stacked", "");
    leg1->AddEntry(stack1->GetHists()->At(0), "HCAL only", "f");
    leg1->AddEntry(stack1->GetHists()->At(1), "MIP", "f");
    leg1->AddEntry(stack1->GetHists()->At(2), "ECAL+HCAL", "f");
    leg1->AddEntry(stack1->GetHists()->At(3), "ECAL only", "f");
    leg1->AddEntry((TObject*)0, "Data: markers", "");
    leg1->AddEntry(g_H, "HCAL only", "p");
    leg1->AddEntry(g_MIP, "MIP", "p");
    leg1->AddEntry(g_EH, "ECAL+HCAL", "p");
    leg1->AddEntry(g_E, "ECAL only", "p");
    leg1->Draw();

    // === 2. Oikea ruutu: Data stacked ===
    c->cd(2);
    stack2->Draw("HIST");
    stack2->SetMinimum(0);
    stack2->SetMaximum(1.0);

    auto leg2 = new TLegend(0.7, 0.15, 0.9, 0.43);
    leg2->AddEntry((TObject*)0, "Data: stacked", "");
    leg2->AddEntry(stack2->GetHists()->At(0), "HCAL only", "f");
    leg2->AddEntry(stack2->GetHists()->At(1), "MIP", "f");
    leg2->AddEntry(stack2->GetHists()->At(2), "ECAL+HCAL", "f");
    leg2->AddEntry(stack2->GetHists()->At(3), "ECAL only", "f");
    leg2->Draw();

    // Tallenna kuva
    c->SaveAs("plots/stack_frac_comparison_raw.png");

    // Sulje tiedostot
    f1->Close();
    f2->Close();
}


int main() {
    plot_stacked_comparison();
    plot_ep_overlay();
    //plot_ep_scale_vs_pt();
    plot_stacked_comparison_raw();
    return 0;
}