#include "TH2F.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TROOT.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cmath>

void make_vetomap(const char* input = "veto_regions.txt",
                  const char* output = "JetVetoMap.root",
                  const char* pngout = "/eos/user/m/mmarjama/my_pion_analysis/scripts/JetVetoMap.png") {
    // Luo veto map nu–phi -tasoon (72 phi-biniä = 5 asteen välein)
    TH2F* hmap = new TH2F("h_jetVetoMap", "Jet veto map;#eta;#phi",
                          100, -5, 5,
                          72, -M_PI, M_PI);

    std::ifstream f(input);
    if (!f.is_open()) {
        std::cerr << "ERROR: Cannot open veto list file: " << input << std::endl;
        return;
    }

    double eta1, eta2, phi1, phi2;
    std::string reason;
    int nRegions = 0;

    // Lue alueet tiedostosta
    while (true) {
        std::string line;
        if (!std::getline(f, line)) break;
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        if (!(iss >> eta1 >> eta2 >> phi1 >> phi2)) continue;
        iss >> reason; // optional

        ++nRegions;
        std::cout << "Adding region " << nRegions
                  << ": eta(" << eta1 << "," << eta2
                  << ") phi(" << phi1 << "," << phi2
                  << ") " << (reason.empty() ? "" : reason) << std::endl;

        for (int i = 1; i <= hmap->GetNbinsX(); ++i) {
            double eta = hmap->GetXaxis()->GetBinCenter(i);
            for (int j = 1; j <= hmap->GetNbinsY(); ++j) {
                double phi = hmap->GetYaxis()->GetBinCenter(j);
                if (eta >= eta1 && eta <= eta2 && phi >= phi1 && phi <= phi2)
                    hmap->SetBinContent(i, j, 1.0);
            }
        }
    }

    std::cout << "Filled " << nRegions << " veto regions." << std::endl;

    // Tallenna ROOT-tiedosto
    TFile out(output, "RECREATE");
    hmap->Write();
    out.Close();
    std::cout << "Wrote veto map to " << output << std::endl;

    // --- Piirrä veto map PNG-kuvaksi CERNBoxiin ---
    gROOT->SetBatch(kTRUE);
    gStyle->SetOptStat(0);
    gStyle->SetPalette(kBird);

    TCanvas* c = new TCanvas("c_veto", "Jet veto map", 800, 600);
    hmap->Draw("COLZ");
    hmap->SetMinimum(0.0);
    hmap->SetMaximum(1.0);

    // Tallenna kuva CERNBoxiin näkyvään EOS-hakemistoon
    c->SaveAs(pngout);
    std::cout << "Saved image to " << pngout << std::endl;

    delete c;
}