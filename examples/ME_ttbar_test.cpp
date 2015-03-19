#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "Riostream.h"

#include "external/ExRootAnalysis/ExRootTreeReader.h"
#include "external/ExRootAnalysis/ExRootTreeWriter.h"
#include "external/ExRootAnalysis/ExRootTreeBranch.h"
#include "external/ExRootAnalysis/ExRootResult.h"
#include "classes/DelphesClasses.h"

#include "src/HelAmps_sm.h"
#include "SubProcesses/P0_Sigma_sm_gg_epvebmumvmxbx/CPPProcess.h"
#include "src/rambo.h"
#include "src/Parameters_sm.h"

#include "TStopwatch.h"

#include "TString.h"
#include "TApplication.h"
#include "TChain.h"
#include "TFile.h"
#include "TObject.h"

#include "TClonesArray.h"
#include "TRandom.h"
#include "TF1.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TSystem.h"
#include "TLorentzVector.h"
#include "TMath.h"

#include "TFoam.h"
#include "TFoamIntegrand.h"
#include "TRandom3.h"

#include "LHAPDF/LHAPDF.h"
#include "LHAPDF/PDFSet.h"

#define SSTR( x ) dynamic_cast< std::ostringstream & > \
        ( std::ostringstream() << std::dec << x ).str()

using namespace LHAPDF;

int mycount = 0, count_wgt = 0, count_perm=1;

class TFDISTR: public TFoamIntegrand {
private:
  CPPProcess process;
  TLorentzVector pep, pmum, pb, pbbar;
  TLorentzVector Met;

  const PDF *pdf;

public:
  TFDISTR(std::string paramCardPath, TLorentzVector ep, TLorentzVector mum, TLorentzVector b, TLorentzVector bbar, TLorentzVector met){
  process.initProc(paramCardPath);
  //process.setInitial(2, -2);
  pep = ep;
  pmum = mum;
  pb = b;
  pbbar = bbar;
  Met = met;
  pdf=LHAPDF::mkPDF("cteq6l1", 0);
  }
/*
  static void SetPDF(TString name, Int_t imem){
    *pdf = mkPDF("cteq6l1", 0);
    }
*/
  Double_t Density(int nDim, Double_t *Xarg){
  // Integrand for mFOAM

  //Double_t Px1=-500+1000*Xarg[0];
  //Double_t Py1=-500+1000*Xarg[1];
  //Double_t Pz1=-500+1000*Xarg[2];
  //Double_t Px2=Met.Px()-Px1;
  //Double_t Py2=Met.Py()-Py1;
  //Double_t Pz2=-500+1000*Xarg[3];

  Double_t Px1=TMath::Tan(-TMath::Pi()/2. + TMath::Pi()*Xarg[0]);
  Double_t Py1=TMath::Tan(-TMath::Pi()/2. + TMath::Pi()*Xarg[1]);
  Double_t Pz1=TMath::Tan(-TMath::Pi()/2. + TMath::Pi()*Xarg[2]);
  Double_t Px2=Met.Px()-Px1;
  Double_t Py2=Met.Py()-Py1;
  Double_t Pz2=TMath::Tan(-TMath::Pi()/2. + TMath::Pi()*Xarg[3]);


  /*cout << "x0=" << Xarg[0] << ", Px1=" << Px1 << ", x1=" << Xarg[1];
  cout << ", Py1=" << Py1 << ", x2=" << Xarg[2] << ", Pz1=" << Pz1;
  cout << ", x3=" << Xarg[3] << ", Pz2=" << Pz2;*/

  Double_t E1= TMath::Sqrt(pow(Px1,2)+pow(Py1,2)+pow(Pz1,2));
  Double_t E2= TMath::Sqrt(pow(Px2,2)+pow(Py2,2)+pow(Pz2,2));

  TLorentzVector nu1,nu2;
  nu1.SetPxPyPzE(Px1,Py1,Pz1,E1);
  nu2.SetPxPyPzE(Px2,Py2,Pz2,E2);

  /*cout << "Electron" << endl;
  cout << pep.E() << "," << pep.Px() << "," << pep.Py() << "," << pep.Pz() << endl;
  cout << "Electron neutrino" << endl;
  cout << nu1.E() << "," << nu1.Px() << "," << nu1.Py() << "," << nu1.Pz() << endl;
  cout << "b quark" << endl;
  cout << pb.E() << "," << pb.Px() << "," << pb.Py() << "," << pb.Pz() << endl;
  cout << "Muon" << endl;
  cout << pmum.E() << "," << pmum.Px() << "," << pmum.Py() << "," << pmum.Pz() << endl;
  cout << "Muon neutrino" << endl;
  cout << nu2.E() << "," << nu2.Px() << "," << nu2.Py() << "," << nu2.Pz() << endl;
  cout << "Anti b quark" << endl;
  cout << pbbar.E() << "," << pbbar.Px() << "," << pbbar.Py() << "," << pbbar.Pz() << endl;*/

  TLorentzVector tot = nu1 + nu2 + pep + pmum + pb + pbbar;

 // Double_t gEcm=(EEl+ENu)/2.0;

  Double_t Eext=tot.E();
  Double_t Pzext=tot.Pz();

  Double_t q1Pz=(Pzext+Eext)/2;
  Double_t q2Pz=(Pzext-Eext)/2;

  //cout << " ===> Eext=" << Eext << ", Pzext=" << Pzext << ", q1Pz=" << q1Pz << ", q2Pz=" << q2Pz << endl;
  //cout << "q1Pz=" << q1Pz << ", q2Pz=" << q2Pz << endl;

  if(q1Pz > 6500. || q2Pz < -6500. || q1Pz < 0. || q2Pz > 0.){
	//cout << "Fail!" << endl;
	mycount++;
	return 0.;
  }


  // momentum vector definition
  vector<double*> p(1, new double[4]);
  p[0][0] = q1Pz; p[0][1] = 0.0; p[0][2] = 0.0; p[0][3] = q1Pz;
  p.push_back(new double[4]);
  p[1][0] = TMath::Abs(q2Pz); p[1][1] = 0.0; p[1][2] = 0.0; p[1][3] = q2Pz;
  p.push_back(new double[4]);
  p[2][0] = pep.E(); p[2][1] = pep.Px(); p[2][2] = pep.Py(); p[2][3] = pep.Pz();
  p.push_back(new double[4]);
  p[3][0] = nu1.E(); p[3][1] = nu1.Px(); p[3][2] = nu1.Py(); p[3][3] = nu1.Pz();
  p.push_back(new double[4]);
  p[4][0] = pb.E(); p[4][1] = pb.Px(); p[4][2] = pb.Py(); p[4][3] = pb.Pz();
  p.push_back(new double[4]);
  p[5][0] = pmum.E(); p[5][1] = pmum.Px(); p[5][2] = pmum.Py(); p[5][3] = pmum.Pz();
  p.push_back(new double[4]);
  p[6][0] = nu2.E(); p[6][1] = nu2.Px(); p[6][2] = nu2.Py(); p[6][3] = nu2.Pz();
  p.push_back(new double[4]);
  p[7][0] = pbbar.E(); p[7][1] = pbbar.Px(); p[7][2] = pbbar.Py(); p[7][3] = pbbar.Pz();

  // Compute the Pdfs
  double pdf1_1 = ComputePdf(21,TMath::Abs(q1Pz/6500.0), pow(tot.M(),2)) / TMath::Abs(q1Pz/6500.0);
  double pdf1_2 = ComputePdf(21,TMath::Abs(q2Pz/6500.0), pow(tot.M(),2)) / TMath::Abs(q2Pz/6500.0);

  // Compute de Phase Space
  double PhaseSpaceIn = 1.0 / ( TMath::Abs(q1Pz/6500.0) *  TMath::Abs(q2Pz/6500.0) * pow(13000.0,2)); // ? factor 2?

  double dphipep = pep.Pt()/(2.0*pow(2.0*TMath::Pi(),3));
  double dphipmum = pmum.Pt()/(2.0*pow(2.0*TMath::Pi(),3));
  double dphipb = pow(pb.P(),2.)*TMath::Sin(pb.Theta())/(2.0*pb.E()*pow(2.0*TMath::Pi(),3)); // massive b's!
  double dphipbbar = pow(pbbar.P(),2.)*TMath::Sin(pbbar.Theta())/(2.0*pbbar.E()*pow(2.0*TMath::Pi(),3));
  double dphinu1 = 1./(pow(2*TMath::Pi(),3)*2*nu1.E()); // nu1.Pt()/(2.0*pow(2.0*TMath::Pi(),3));
  double dphinu2 = 1./(pow(2*TMath::Pi(),3)*2*nu2.E()); // nu2.Pt()/(2.0*pow(2.0*TMath::Pi(),3));

  double PhaseSpaceOut = pow(2.0*TMath::Pi(),4) * 4./pow(13000.0,2) * dphipep * dphipmum * dphipb * dphipbbar * dphinu1 * dphinu2;

  // Additional factor due to the integration range:
  //double jac = pow(1000,4);
  double jac = pow(TMath::Pi(),4.)/( pow(TMath::Cos(-TMath::Pi()/2. + TMath::Pi()*Xarg[0]), 2.) * pow(TMath::Cos(-TMath::Pi()/2. + TMath::Pi()*Xarg[1]), 2.)
	* pow(TMath::Cos(-TMath::Pi()/2. + TMath::Pi()*Xarg[2]), 2.) * pow(TMath::Cos(-TMath::Pi()/2. + TMath::Pi()*Xarg[3]), 2.) );

  //cout << "phase space=" << jac * PhaseSpaceIn * PhaseSpaceOut << ", pdfprod=" << pdf1_1*pdf1_2 << "\n\n";

  // Set momenta for this event
  process.setMomenta(p);

  // Evaluate matrix element
  process.sigmaKin();
  const Double_t* matrix_elements1 = process.getMatrixElements();
  
  // final integrand
  double matrix_element = jac * (matrix_elements1[0] * pdf1_1 * pdf1_2) * PhaseSpaceIn * PhaseSpaceOut;

  //cout << "|M|^2 = " << matrix_element << "     W :" << (pep+nu1).M() << " " << (pmum+nu2).M() << endl;
  return matrix_element;
  }


  Double_t ComputePdf(int pid, double x, double q2){
    // return the xf(pid,x,q2), be careful: this value must be divided by x to obtain f
    const double xf = pdf->xfxQ2(pid, x, q2);
    return xf;
  }
};


Double_t ME(TLorentzVector ep, TLorentzVector mum, TLorentzVector b, TLorentzVector bbar, TLorentzVector Met){

  TH1D *hst_Wm = new TH1D("test_mu", "test_1D", 150,0,150);
  TH1D *hst_We = new TH1D("test_ep", "test_1D", 150,0,150);
  TH1D *hst_t = new TH1D("test_t", "test_1D", 100,150,250);
  TH1D *hst_tbar = new TH1D("test_tbar", "test_1D", 100,150,250);


  TStopwatch chrono;
  std::cout << "initialising : " ;
  // TFoam Implementation
  Double_t *MCvect = new Double_t[4];
  TRandom3  *PseRan   = new TRandom3();
  PseRan->SetSeed(2245);  
  TFoam   *FoamX    = new TFoam("FoamX");
  FoamX->SetkDim(4);
  FoamX->SetnCells(20000);      // No. of cells, can be omitted, default=2000
  FoamX->SetnSampl(10);

  TString pdfname = "cteq6l1";
  //Int_t imem = 0;

  TFoamIntegrand *rho= new TFDISTR("/home/fynu/swertz/scratch/Madgraph/madgraph5/cpp_ttbar_epmum/Cards/param_card.dat", ep, mum, b, bbar, Met );

  FoamX->SetRho(rho);
  FoamX->SetPseRan(PseRan);   // Set random number generator

  std::cout << "Starting initialisation..." << std::endl;
  FoamX->Initialize(); 
  
  std::cout << "CPU time : " << chrono.CpuTime() << "  Real-time : " << chrono.RealTime() << std::endl;
  chrono.Reset();
  chrono.Start();
  std::cout << "Looping over phase-space points" << std::endl;

//  chrono->Reset();
//  chrono->Start();

  for(Long_t loop=0; loop<50000; loop++){
    int count_old = mycount;
    FoamX->MakeEvent();          // generate MC event
    FoamX->GetMCvect( MCvect );   // get generated vector (x,y)
    
    /*Double_t px1=-500+1000*MCvect[0];
    Double_t py1=-500+1000*MCvect[1];
    Double_t pz1=-500+1000*MCvect[2];
    Double_t pz2=-500+1000*MCvect[3];*/
	Double_t px1=TMath::Tan(-TMath::Pi()/2. + TMath::Pi()*MCvect[0]);
  	Double_t py1=TMath::Tan(-TMath::Pi()/2. + TMath::Pi()*MCvect[1]);
  	Double_t pz1=TMath::Tan(-TMath::Pi()/2. + TMath::Pi()*MCvect[2]);
  	Double_t pz2=TMath::Tan(-TMath::Pi()/2. + TMath::Pi()*MCvect[3]);
    //if(loop<10) cout<<"(x,y) =  ( "<< x <<", "<< y <<" )"<<endl;
    TLorentzVector nu1,nu2;
    nu1.SetPxPyPzE(px1,py1,pz1,TMath::Sqrt(pow(px1,2)+pow(py1,2)+pow(pz1,2)));
    nu2.SetPxPyPzE(Met.Px()-px1,Met.Py()-py1,pz2,TMath::Sqrt(pow(Met.Px()-px1,2)+pow(Met.Py()-py1,2)+pow(pz2,2)));
    //cout << "W mass : " << (nu1+ep).M() << endl;  
	if(count_old == mycount){
		hst_We->Fill((nu1+ep).M());           // fill scattergram
		hst_Wm->Fill((nu2+mum).M());           // fill scattergram
		hst_t->Fill((nu1+ep+b).M());
		hst_tbar->Fill((nu2+mum+bbar).M());
	}
    
   }// loop



  Double_t mcResult, mcError;
  FoamX->GetIntegMC( mcResult, mcError);  // get MC integral, should be one
  
  std::cout << "CPU time : " << chrono.CpuTime() << "  Real-time : " << chrono.RealTime() << std::endl;
  cout << "nr. fails: " << mycount  << endl;

  cout << " mcResult= " << mcResult << " +- " << mcError <<endl;
  // now hst_xy will be plotted visualizing generated distribution
  TCanvas *c = new TCanvas("c","Canvas for plotting",600,600);
   c->cd();
   hst_We->Draw();
   c->Print(TString("plots/")+SSTR(count_wgt)+"_"+SSTR(count_perm)+"_Enu.png");
   delete hst_We;
   delete c;
  
   c = new TCanvas("c","Canvas for plotting",600,600);
   c->cd();
   hst_Wm->Draw();
   c->Print(TString("plots/")+SSTR(count_wgt)+"_"+SSTR(count_perm)+"_Munu.png");
   delete hst_Wm;
   delete c;


   c = new TCanvas("c","Canvas for plotting",600,600);
   c->cd();
   hst_t->Draw();
   c->Print(TString("plots/")+SSTR(count_wgt)+"_"+SSTR(count_perm)+"_t.png");
   delete hst_t;
   delete c;
  
   c = new TCanvas("c","Canvas for plotting",600,600);
   c->cd();
   hst_tbar->Draw();
   c->Print(TString("plots/")+SSTR(count_wgt)+"_"+SSTR(count_perm)+"_tbar.png");
   delete hst_tbar;
   delete c;

  delete FoamX;
  delete PseRan;
  delete MCvect;

  return mcResult;
}


int main(int argc, char *argv[])
//const char *inputFile,const char *outputFile
{

  TString inputFile(argv[1]);
  TString outputFile(argv[2]);

  gSystem->Load("libDelphes");

  // Create chain of root trees
  TChain chain("Delphes");
  chain.Add(inputFile);

  // Create object of class ExRootTreeReader
  ExRootTreeReader *treeReader = new ExRootTreeReader(&chain);
  //Long64_t numberOfEntries = treeReader->GetEntries();

  // Get pointers to branches used in this analysis
  TClonesArray *branchGen = treeReader->UseBranch("Particle");
  // Loop over all events

/*  TString pdfname = "cteq6l1";
  Int_t imem = 0;

  TFDISTR::SetPDF(pdfname, imem);
*/

  cout << "Entries:" << chain.GetEntries() << endl;

  double weight1 = 0, weight2 = 0;

  ofstream fout(outputFile);

  double madweight1[10] = {1.58495292058e-21, 2.09681384879e-21, 4.34399623629e-22, 1.68163897955e-22, 3.20350498956e-22, 5.22232034307e-22, 6.04738375743e-21, 9.55643564854e-22, 8.12425265344e-22, 5.81210532053e-23};
  double madweight2[10] = {1.02514966131e-21, 1.45375719248e-21, 1.65080839221e-22, 1.55653414654e-24, 5.60531044001e-25, 1.,  9.70526105314e-24, 3.89103636371e-22, 6.38206925825e-23, 9.37189585544e-26};

  for(Int_t entry = 0; entry < 10; ++entry)//numberOfEntries; ++entry)
  {
    // Load selected branches with data from specified event
    treeReader->ReadEntry(entry);

    TLorentzVector gen_ep, gen_mum, gen_b, gen_bbar, gen_Met;

    GenParticle *gen;

	//int count_ep=0, count_mum=0;

    for (Int_t i = 0; i < branchGen->GetEntries(); i++)
    {
      gen = (GenParticle *) branchGen->At(i);
	  //cout << "Status=" << gen->Status << ", PID=" << gen->PID << ", E=" << gen->P4().E() << endl;
      if (gen->Status == 1)
      {
        if (gen->PID == -11){
			gen_ep = gen->P4();
			//count_ep++;
		}
        else if (gen->PID == 13){
			gen_mum = gen->P4();
			//count_mum++;
		}
		else if (gen->PID == 12) gen_Met += gen->P4();
		else if (gen->PID == -14) gen_Met += gen->P4();
		else if (gen->PID == 5) gen_b = gen->P4();
		else if (gen->PID == -5) gen_bbar = gen->P4();
      }
    }

	//if(count_ep != 1 || count_mum != 1)
	//	continue;
	//gen_Met.SetPz(0.);
  
  cout << "From MadGraph:" << endl;
  cout << "Electron" << endl;
  cout << gen_ep.E() << "," << gen_ep.Px() << "," << gen_ep.Py() << "," << gen_ep.Pz() << endl;
  cout << "b quark" << endl;
  cout << gen_b.E() << "," << gen_b.Px() << "," << gen_b.Py() << "," << gen_b.Pz() << endl;
  cout << "Muon" << endl;
  cout << gen_mum.E() << "," << gen_mum.Px() << "," << gen_mum.Py() << "," << gen_mum.Pz() << endl;
  cout << "Anti b quark" << endl;
  cout << gen_bbar.E() << "," << gen_bbar.Px() << "," << gen_bbar.Py() << "," << gen_bbar.Pz() << endl;
  cout << "MET" << endl;
  cout << gen_Met.E() << "," << gen_Met.Px() << "," << gen_Met.Py() << "," << gen_Met.Pz() << endl;

  count_perm = 1;
  weight1 = ME(gen_ep, gen_mum, gen_b, gen_bbar, gen_Met);
  count_perm = 2;
  weight2 = ME(gen_ep, gen_mum, gen_bbar, gen_b, gen_Met);

  double weight3;
  if(weight1 < weight2){
	weight3 = weight2;
	weight2 = weight1;
	weight1 = weight3;
  }

  fout << entry << " " << weight1 << "   " << weight1 / (double) madweight1[entry] << ", " << weight2 << "    " << weight2/madweight2[entry] <<  endl;

  count_wgt++;

  }
 //delete treeReader; 
}
