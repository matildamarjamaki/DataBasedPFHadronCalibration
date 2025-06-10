
// #include "TFile.h"
// #include "TH1D.h"
// #include "TH2D.h"
// #include "TProfile.h"
// #include "TF1.h"
// #include "TCanvas.h"
// #include "TLegend.h"
// #include "TStyle.h"
// #include <iostream>

// void plot_histograms() {
//     TFile *file = TFile::Open("histograms/pion_histos_iso.root");
//     if (!file || file->IsZombie()) {
//         std::cerr << "ERROR: Cannot open histogram file!" << std::endl;
//         return;
//     }

//     // Lue histogrammit ja profiilit isoloiduille pioneille
//     TH1D     *h_pt_iso             = (TH1D*)file->Get("h_pt_iso");
//     TH1D     *h_eta_iso            = (TH1D*)file->Get("h_eta_iso");
//     TH1D     *h_ep_iso             = (TH1D*)file->Get("h_ep_iso");
//     TH1D     *h_raw_ep_iso         = (TH1D*)file->Get("h_raw_ep_iso");
//     TProfile *prof_ep_vs_p_iso     = (TProfile*)file->Get("prof_ep_vs_p_iso");
//     TProfile *prof_raw_ep_vs_p_iso = (TProfile*)file->Get("prof_raw_ep_vs_p_iso");
//     TH2D     *h2_ep_vs_p_iso       = (TH2D*)file->Get("h2_ep_vs_p_iso");
//     TH2D     *h2_raw_ep_vs_p_iso   = (TH2D*)file->Get("h2_raw_ep_vs_p_iso");

//     if (!h_pt_iso || !h_eta_iso || !h_ep_iso || !h_raw_ep_iso || !prof_ep_vs_p_iso || !prof_raw_ep_vs_p_iso || !h2_ep_vs_p_iso || !h2_raw_ep_vs_p_iso) {
//         std::cerr << "ERROR: One or more histograms or profiles not found in file!" << std::endl;
//         return;
//     }

//     // Perushistogrammit
//     TCanvas *c1 = new TCanvas("c1_iso", "Basic isolated pion histograms", 1200, 900);
//     c1->Divide(1, 3);

//     c1->cd(1);
//     gPad->SetLogy();
//     h_pt_iso->Draw();

//     c1->cd(2);
//     h_eta_iso->Draw();

//     c1->cd(3);
//     h_ep_iso->Draw();

//     c1->SaveAs("plots/isolated_pions.png");
//     delete c1;

//     // Profiiliplotti: ⟨E/p⟩ vs p
//     TCanvas *c2 = new TCanvas("c2_iso", "<E/p> vs p (isolated)", 800, 600);
//     prof_ep_vs_p_iso->SetLineColor(kBlue);
//     prof_ep_vs_p_iso->SetLineWidth(2);
//     prof_ep_vs_p_iso->Draw("E1");
//     c2->SaveAs("plots/iso_ep_vs_p.png");
//     delete c2;

//     // 2D-histogrammi: E/p vs p
//     TCanvas *c3 = new TCanvas("c3_iso", "E/p vs p (2D isolated)", 800, 600);
//     gStyle->SetOptStat(0);
//     h2_ep_vs_p_iso->GetXaxis()->SetNdivisions(10);
//     h2_ep_vs_p_iso->GetYaxis()->SetRangeUser(0.01, 2.25);
//     h2_ep_vs_p_iso->Draw("COLZ");
//     c3->SaveAs("plots/iso_ep_vs_p_2D.png");
//     delete c3;

//     // Raaka E/p histogrammi
//     TCanvas *c4 = new TCanvas("c4_iso", "Raw E/p histogram (isolated)", 800, 600);
//     h_raw_ep_iso->SetLineColor(kMagenta + 1);
//     h_raw_ep_iso->Draw();
//     c4->SaveAs("plots/iso_raw_ep_hist.png");
//     delete c4;

//     // Raaka profiili: ⟨Raw E/p⟩ vs p
//     TCanvas *c5 = new TCanvas("c5_iso", "<Raw E/p> vs p (isolated)", 800, 600);
//     prof_raw_ep_vs_p_iso->SetLineColor(kGreen + 2);
//     prof_raw_ep_vs_p_iso->SetLineWidth(2);
//     prof_raw_ep_vs_p_iso->Draw("E1");
//     c5->SaveAs("plots/iso_raw_ep_vs_p.png");
//     delete c5;

//     // 2D-histogrammi: Raw E/p vs p
//     TCanvas *c6 = new TCanvas("c6_iso", "Raw E/p vs p (2D isolated)", 800, 600);
//     h2_raw_ep_vs_p_iso->GetXaxis()->SetNdivisions(10);
//     h2_raw_ep_vs_p_iso->GetYaxis()->SetRangeUser(0.01, 2.25);
//     h2_raw_ep_vs_p_iso->Draw("COLZ");
//     c6->SaveAs("plots/iso_raw_ep_vs_p_2D.png");
//     delete c6;

//     file->Close();
// }

// #include "TFile.h"
// #include "TH1D.h"
// #include "TH2D.h"
// #include "TProfile.h"
// #include "TF1.h"
// #include "TCanvas.h"
// #include "TLegend.h"
// #include "TStyle.h"
// #include <iostream>

// void plot_histograms() {
//     TFile *file = TFile::Open("histograms/pion_histos_iso.root");
//     if (!file || file->IsZombie()) {
//         std::cerr << "ERROR: Cannot open histogram file!" << std::endl;
//         return;
//     }

//     // Histogrammit ja profiilit
//     TH1D     *h_pt_iso     = (TH1D*)file->Get("h_pt_iso");
//     TH1D     *h_eta_iso    = (TH1D*)file->Get("h_eta_iso");
//     TH1D     *h_ep_iso     = (TH1D*)file->Get("h_ep_iso");
//     TH1D     *h_raw_ep_iso = (TH1D*)file->Get("h_raw_ep_iso");

//     TProfile *prof_ep_vs_p_iso            = (TProfile*)file->Get("prof_ep_vs_p_iso");
//     TProfile *prof_ep_vs_p_iso_custom     = (TProfile*)file->Get("prof_ep_vs_p_iso_custom");
//     TProfile *prof_raw_ep_vs_p_iso        = (TProfile*)file->Get("prof_raw_ep_vs_p_iso");
//     TProfile *prof_raw_ep_vs_p_iso_custom = (TProfile*)file->Get("prof_raw_ep_vs_p_iso_custom");

//     TH2D *h2_ep_vs_p_iso     = (TH2D*)file->Get("h2_ep_vs_p_iso");
//     TH2D *h2_raw_ep_vs_p_iso = (TH2D*)file->Get("h2_raw_ep_vs_p_iso");

//     if (!h_pt_iso || !h_eta_iso || !h_ep_iso || !h_raw_ep_iso ||
//         !prof_ep_vs_p_iso || !prof_ep_vs_p_iso_custom ||
//         !prof_raw_ep_vs_p_iso || !prof_raw_ep_vs_p_iso_custom ||
//         !h2_ep_vs_p_iso || !h2_raw_ep_vs_p_iso) {
//         std::cerr << "ERROR: One or more histograms or profiles not found in file!" << std::endl;
//         return;
//     }

//     // Perushistogrammit
//     h_pt_iso->GetXaxis()->SetTitle("p_{T} [GeV]");
//     h_pt_iso->GetYaxis()->SetTitle("Entries");
//     h_eta_iso->GetXaxis()->SetTitle("#eta");
//     h_eta_iso->GetYaxis()->SetTitle("Entries");
//     h_ep_iso->GetXaxis()->SetTitle("(E_{ECAL}+E_{HCAL}) / p");
//     h_ep_iso->GetYaxis()->SetTitle("Entries");

//     TCanvas *c1 = new TCanvas("c1_iso", "Basic isolated pion histograms", 1200, 900);
//     c1->Divide(1, 3);
//     c1->cd(1); gPad->SetLogy(); h_pt_iso->Draw();
//     c1->cd(2); h_eta_iso->Draw();
//     c1->cd(3); h_ep_iso->Draw();
//     c1->SaveAs("plots/isolated_pions.png");
//     delete c1;

//     // Profiilit
//     prof_ep_vs_p_iso->GetXaxis()->SetTitle("p [GeV]");
//     prof_ep_vs_p_iso->GetYaxis()->SetTitle("<E/p>");
//     TCanvas *c2 = new TCanvas("c2_iso", "<E/p> vs p (isolated)", 800, 600);
//     prof_ep_vs_p_iso->SetLineColor(kBlue); prof_ep_vs_p_iso->SetLineWidth(2);
//     prof_ep_vs_p_iso->Draw("E1");
//     c2->SaveAs("plots/iso_ep_vs_p.png");
//     delete c2;

//     prof_ep_vs_p_iso_custom->GetXaxis()->SetTitle("p [GeV]");
//     prof_ep_vs_p_iso_custom->GetYaxis()->SetTitle("<E/p>");
//     TCanvas *c2b = new TCanvas("c2b_iso", "<E/p> vs p (isolated, custom)", 800, 600);
//     prof_ep_vs_p_iso_custom->SetLineColor(kOrange + 1); prof_ep_vs_p_iso_custom->SetLineWidth(2);
//     prof_ep_vs_p_iso_custom->Draw("E1");
//     c2b->SaveAs("plots/iso_ep_vs_p_custom.png");
//     delete c2b;

//     // 2D E/p vs p
//     h2_ep_vs_p_iso->GetXaxis()->SetTitle("p [GeV]");
//     h2_ep_vs_p_iso->GetYaxis()->SetTitle("E/p");
//     h2_ep_vs_p_iso->GetYaxis()->SetRangeUser(0.01, 2.25);
//     // h2_ep_vs_p_iso->GetYaxis()->SetNdivisions(45);  // ≈0.05 välein
//     TCanvas *c3 = new TCanvas("c3_iso", "E/p vs p (2D isolated)", 800, 600);
//     gStyle->SetOptStat(0);
//     h2_ep_vs_p_iso->Draw("COLZ");
//     c3->SaveAs("plots/iso_ep_vs_p_2D.png");
//     delete c3;

//     // Raaka E/p histogrammi
//     h_raw_ep_iso->GetXaxis()->SetTitle("Raw (E_{ECAL}+E_{HCAL}) / p");
//     h_raw_ep_iso->GetYaxis()->SetTitle("Entries");
//     TCanvas *c4 = new TCanvas("c4_iso", "Raw E/p histogram (isolated)", 800, 600);
//     h_raw_ep_iso->SetLineColor(kMagenta + 1);
//     h_raw_ep_iso->Draw();
//     c4->SaveAs("plots/iso_raw_ep_hist.png");
//     delete c4;

//     // Raaka profiilit
//     prof_raw_ep_vs_p_iso->GetXaxis()->SetTitle("p [GeV]");
//     prof_raw_ep_vs_p_iso->GetYaxis()->SetTitle("<Raw E/p>");
//     prof_raw_ep_vs_p_iso->GetYaxis()->SetRangeUser(0.0, 2.25);
//     TCanvas *c5 = new TCanvas("c5_iso", "<Raw E/p> vs p (isolated)", 800, 600);
//     prof_raw_ep_vs_p_iso->SetLineColor(kGreen + 2); prof_raw_ep_vs_p_iso->SetLineWidth(2);
//     prof_raw_ep_vs_p_iso->Draw("E1");
//     c5->SaveAs("plots/iso_raw_ep_vs_p.png");
//     delete c5;

//     prof_raw_ep_vs_p_iso_custom->GetXaxis()->SetTitle("p [GeV]");
//     prof_raw_ep_vs_p_iso_custom->GetYaxis()->SetTitle("<Raw E/p>");
//     prof_raw_ep_vs_p_iso_custom->GetYaxis()->SetRangeUser(0.0, 2.25);
//     TCanvas *c5b = new TCanvas("c5b_iso", "<Raw E/p> vs p (isolated, custom)", 800, 600);
//     prof_raw_ep_vs_p_iso_custom->SetLineColor(kCyan + 2); prof_raw_ep_vs_p_iso_custom->SetLineWidth(2);
//     prof_raw_ep_vs_p_iso_custom->Draw("E1");
//     c5b->SaveAs("plots/iso_raw_ep_vs_p_custom.png");
//     delete c5b;

//     // 2D Raw E/p
//     h2_raw_ep_vs_p_iso->GetXaxis()->SetTitle("p [GeV]");
//     h2_raw_ep_vs_p_iso->GetYaxis()->SetTitle("Raw E/p");
//     h2_raw_ep_vs_p_iso->GetYaxis()->SetRangeUser(0.0, 2.25);
//     // h2_raw_ep_vs_p_iso->GetYaxis()->SetNdivisions(45);  // ≈0.05 välein
//     TCanvas *c6 = new TCanvas("c6_iso", "Raw E/p vs p (2D isolated)", 800, 600);
//     h2_raw_ep_vs_p_iso->Draw("COLZ");
//     c6->SaveAs("plots/iso_raw_ep_vs_p_2D.png");
//     delete c6;

//     file->Close();
// }

#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"
#include <iostream>

void plot_histograms() {
    TFile *file = TFile::Open("histograms/pion_histos_iso.root");
    if (!file || file->IsZombie()) {
        std::cerr << "ERROR: Cannot open histogram file!" << std::endl;
        return;
    }

    // Histogrammit ja profiilit
    TH1D     *h_pt_iso     = (TH1D*)file->Get("h_pt_iso");
    TH1D     *h_eta_iso    = (TH1D*)file->Get("h_eta_iso");
    TH1D     *h_ep_iso     = (TH1D*)file->Get("h_ep_iso");
    TH1D     *h_raw_ep_iso = (TH1D*)file->Get("h_raw_ep_iso");

    TProfile *prof_ep_vs_p_iso            = (TProfile*)file->Get("prof_ep_vs_p_iso");
    TProfile *prof_ep_vs_p_iso_custom     = (TProfile*)file->Get("prof_ep_vs_p_iso_custom");
    TProfile *prof_raw_ep_vs_p_iso        = (TProfile*)file->Get("prof_raw_ep_vs_p_iso");
    TProfile *prof_raw_ep_vs_p_iso_custom = (TProfile*)file->Get("prof_raw_ep_vs_p_iso_custom");

    TH2D *h2_ep_vs_p_iso     = (TH2D*)file->Get("h2_ep_vs_p_iso");
    TH2D *h2_raw_ep_vs_p_iso = (TH2D*)file->Get("h2_raw_ep_vs_p_iso");

    if (!h_pt_iso || !h_eta_iso || !h_ep_iso || !h_raw_ep_iso ||
        !prof_ep_vs_p_iso || !prof_ep_vs_p_iso_custom ||
        !prof_raw_ep_vs_p_iso || !prof_raw_ep_vs_p_iso_custom ||
        !h2_ep_vs_p_iso || !h2_raw_ep_vs_p_iso) {
        std::cerr << "ERROR: One or more histograms or profiles not found in file!" << std::endl;
        return;
    }

    // Perushistogrammit
    h_pt_iso->GetXaxis()->SetTitle("p_{T} [GeV]");
    h_pt_iso->GetYaxis()->SetTitle("Entries");
    h_eta_iso->GetXaxis()->SetTitle("#eta");
    h_eta_iso->GetYaxis()->SetTitle("Entries");
    h_ep_iso->GetXaxis()->SetTitle("(E_{ECAL}+E_{HCAL}) / p");
    h_ep_iso->GetYaxis()->SetTitle("Entries");

    TCanvas *c1 = new TCanvas("c1_iso", "Basic isolated pion histograms", 1200, 900);
    c1->Divide(1, 3);
    c1->cd(1); gPad->SetLogy(); h_pt_iso->Draw();
    c1->cd(2); h_eta_iso->Draw();
    c1->cd(3); h_ep_iso->Draw();
    c1->SaveAs("plots/isolated_pions.png");
    delete c1;

    // Profiilit
    prof_ep_vs_p_iso->GetXaxis()->SetTitle("p [GeV]");
    prof_ep_vs_p_iso->GetYaxis()->SetTitle("<E/p>");
    TCanvas *c2 = new TCanvas("c2_iso", "<E/p> vs p (isolated)", 800, 600);
    prof_ep_vs_p_iso->SetLineColor(kBlue); prof_ep_vs_p_iso->SetLineWidth(2);
    prof_ep_vs_p_iso->Draw("E1");
    c2->SaveAs("plots/iso_ep_vs_p.png");
    delete c2;

    prof_ep_vs_p_iso_custom->GetXaxis()->SetTitle("p [GeV]");
    prof_ep_vs_p_iso_custom->GetYaxis()->SetTitle("<E/p>");
    TCanvas *c2b = new TCanvas("c2b_iso", "<E/p> vs p (isolated, custom)", 800, 600);
    prof_ep_vs_p_iso_custom->SetLineColor(kOrange + 1); prof_ep_vs_p_iso_custom->SetLineWidth(2);
    c2b->SetLogx();  // <-- x-akseli logaritmiseksi
    prof_ep_vs_p_iso_custom->Draw("E1");
    c2b->SaveAs("plots/iso_ep_vs_p_custom.png");
    delete c2b;

    // 2D E/p vs p
    h2_ep_vs_p_iso->GetXaxis()->SetTitle("p [GeV]");
    h2_ep_vs_p_iso->GetYaxis()->SetTitle("E/p");
    h2_ep_vs_p_iso->GetYaxis()->SetRangeUser(0.01, 2.25);
    TCanvas *c3 = new TCanvas("c3_iso", "E/p vs p (2D isolated)", 800, 600);
    gStyle->SetOptStat(0);
    gPad->SetLogx();  // Logaritminen pT-akseli
    gStyle->SetNumberContours(50); // pehmeämpi väritoteutus
    h2_ep_vs_p_iso->Draw("COLZ");
    c3->SaveAs("plots/iso_ep_vs_p_2D.png");
    delete c3;

// ===================== E/p PROJEKTIOT ============================
std::vector<std::pair<double, double>> pBins = {
    {5.5, 6.0},
    {6.0, 7.0},
    {7.0, 8.0},
    {8.0, 9.0},
    {20.0, 22.0}
};

for (const auto& [pmin, pmax] : pBins) {
    int bin_min = h2_ep_vs_p_iso->GetXaxis()->FindBin(pmin + 1e-3);
    int bin_max = h2_ep_vs_p_iso->GetXaxis()->FindBin(pmax - 1e-3);
    std::stringstream name;
    name << "proj_ep_" << int(pmin * 10) << "_" << int(pmax * 10);

    TH1D* proj = h2_ep_vs_p_iso->ProjectionY(name.str().c_str(), bin_min, bin_max);
    if (proj->Integral() == 0) continue;
    proj->Scale(1.0 / proj->Integral());

    proj->SetLineColor(kBlack);
    proj->SetLineWidth(2);
    proj->SetFillColorAlpha(kAzure + 1, 0.5);  // Sininen täyttö

    proj->SetTitle("");
    proj->GetXaxis()->SetTitle("E/p");
    proj->GetYaxis()->SetTitle("Fraction of particles");

    TCanvas* c = new TCanvas((name.str() + "_canvas").c_str(), name.str().c_str(), 800, 600);
    proj->Draw("HIST");

    // Gaussian-sovitus
    TF1* fit = new TF1("fit", "gaus", 0.0, 2.3); // voit säätää rangea
    proj->Fit(fit, "R");  // R = fit only in range
    fit->SetLineColor(kRed);
    fit->SetLineWidth(2);
    fit->Draw("same");

    // Latex-otsikko
    TLatex latex;
    latex.SetNDC();
    latex.SetTextSize(0.04);
    latex.DrawLatex(0.15, 0.92, Form("E/p in %.1f <= p < %.1f GeV", pmin, pmax));

    c->SaveAs(("plots/" + name.str() + ".png").c_str());
    delete c;
}


    // Raw E/p histogrammi
    h_raw_ep_iso->GetXaxis()->SetTitle("Raw (E_{ECAL}+E_{HCAL}) / p");
    h_raw_ep_iso->GetYaxis()->SetTitle("Entries");
    TCanvas *c4 = new TCanvas("c4_iso", "Raw E/p histogram (isolated)", 800, 600);
    h_raw_ep_iso->SetLineColor(kMagenta + 1);
    h_raw_ep_iso->Draw();
    c4->SaveAs("plots/iso_raw_ep_hist.png");
    delete c4;

    // Raaka profiilit
    prof_raw_ep_vs_p_iso->GetXaxis()->SetTitle("p [GeV]");
    prof_raw_ep_vs_p_iso->GetYaxis()->SetTitle("<Raw E/p>");
    // prof_raw_ep_vs_p_iso->GetYaxis()->SetRangeUser(0.0, 2.25);
    TCanvas *c5 = new TCanvas("c5_iso", "<Raw E/p> vs p (isolated)", 800, 600);
    prof_raw_ep_vs_p_iso->SetLineColor(kGreen + 2); prof_raw_ep_vs_p_iso->SetLineWidth(2);
    prof_raw_ep_vs_p_iso->Draw("E1");
    c5->SaveAs("plots/iso_raw_ep_vs_p.png");
    delete c5;

    prof_raw_ep_vs_p_iso_custom->GetXaxis()->SetTitle("p [GeV]");
    prof_raw_ep_vs_p_iso_custom->GetYaxis()->SetTitle("<Raw E/p>");
    // prof_raw_ep_vs_p_iso_custom->GetYaxis()->SetRangeUser(0.0, 2.25);
    TCanvas *c5b = new TCanvas("c5b_iso", "<Raw E/p> vs p (isolated, custom)", 800, 600);
    prof_raw_ep_vs_p_iso_custom->SetLineColor(kCyan + 2); prof_raw_ep_vs_p_iso_custom->SetLineWidth(2);
    c5b->SetLogx();  // <-- x-akseli logaritmiseksi
    prof_raw_ep_vs_p_iso_custom->Draw("E1");
    c5b->SaveAs("plots/iso_raw_ep_vs_p_custom.png");
    delete c5b;

    // 2D Raw E/p
    h2_raw_ep_vs_p_iso->GetXaxis()->SetTitle("p [GeV]");
    h2_raw_ep_vs_p_iso->GetYaxis()->SetTitle("Raw E/p");
    h2_raw_ep_vs_p_iso->GetYaxis()->SetRangeUser(0.01, 2.25);
    TCanvas *c6 = new TCanvas("c6_iso", "Raw E/p vs p (2D isolated)", 800, 600);
    gPad->SetLogx();  // Logaritminen pT-akseli
    h2_raw_ep_vs_p_iso->Draw("COLZ");
    c6->SaveAs("plots/iso_raw_ep_vs_p_2D.png");
    delete c6;

    // 2D Raw E/p (zoomattu y-akseli)
    // h2_raw_ep_vs_p_iso->GetXaxis()->SetTitle("p_{T} [GeV]");
    // h2_raw_ep_vs_p_iso->GetYaxis()->SetTitle("Raw E/p");
    // h2_raw_ep_vs_p_iso->GetYaxis()->SetRangeUser(0.00, 0.5);  // <-- TÄMÄ MUUTTUU
    // TCanvas *c6 = new TCanvas("c6_iso", "Raw E/p vs p (2D isolated)", 800, 600);
    // gPad->SetLogx();  // Logaritminen pT-akseli
    // gStyle->SetNumberContours(999); // pehmeämpi väritoteutus
    // h2_raw_ep_vs_p_iso->Draw("COLZ");
    // c6->SaveAs("plots/iso_raw_ep_vs_p_2D.png");
    // delete c6;


    file->Close();
}