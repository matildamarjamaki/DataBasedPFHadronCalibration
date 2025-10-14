#include "TFile.h"
#include "TTree.h"
#include <fstream>
#include <iostream>
#include <string>
#include <cctype>

static std::string trim(std::string s){
  while(!s.empty() && std::isspace((unsigned char)s.front())) s.erase(s.begin());
  while(!s.empty() && std::isspace((unsigned char)s.back()))  s.pop_back();
  return s;
}

void validate_filelist(const char* inpath="input_lists/SingleNeutrino25_files.txt",
                       const char* outpath="input_lists/SingleNeutrino25_good.txt") {
  std::ifstream fin(inpath);
  if(!fin.is_open()){ std::cerr << "[validate] Cannot open " << inpath << "\n"; return; }
  std::ofstream fout(outpath);
  if(!fout.is_open()){ std::cerr << "[validate] Cannot create " << outpath << "\n"; return; }

  std::string line;
  long total=0, good=0, bad=0;
  while(std::getline(fin, line)){
    line = trim(line);
    if(line.empty() || line[0]=='#') continue;
    ++total;

    TFile *f = TFile::Open(line.c_str(), "READ");
    bool ok = (f && !f->IsZombie());
    if (ok) {
      // varmistetaan että "Events" puu on olemassa
      ok = (f->Get("Events") != nullptr);
    }
    if (ok) { fout << line << "\n"; ++good; }
    else    { ++bad; }

    if (f) f->Close();

    if (total % 50 == 0)
      std::cout << "[validate] checked " << total << " files, good=" << good << " bad=" << bad << std::endl;
  }
  std::cout << "[validate] DONE: total=" << total
            << " good=" << good << " bad=" << bad
            << " -> wrote " << outpath << std::endl;
}
