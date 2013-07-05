/**
 * @author Andrey.Popov@cern.ch
 *
 * The plugin saves the information about b- and c-quarks in the event. It finds the genetically
 * connected consecutive chains of the same flavour, classifies them as parton shower, entering or
 * leaving the matrix element, finds the sister chains. It also stores some additional generator
 * information such as the input partons flavours and the signal process ID number.
 */

#ifndef SINGLE_TOP_FLAVOUR_ANALYZER
#define SINGLE_TOP_FLAVOUR_ANALYZER

#include <FWCore/Framework/interface/EDAnalyzer.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>


class FlavourAnalyzer: public edm::EDAnalyzer
{
    public:
        FlavourAnalyzer(const edm::ParameterSet &);
        ~FlavourAnalyzer();
    
    private:
        void beginJob();
        void endJob();
        void analyze(const edm::Event &, const edm::EventSetup &);
        void performClassification();
        
        edm::InputTag const genParticlesSrc;
        edm::InputTag const generatorSrc;
        edm::InputTag const genJetsSrc;
        double ptCut;  // cut to filter the generator jets
        
        // Flags to control the workflow
        bool const minimalisticChains;  // save chains' PDG ID and flavour source only
        bool const savePDFInfo;
        bool const saveLightPartons;
        bool const saveGenJets;
        bool const classify;  // perform event classificaion and store the result
        
        edm::Service<TFileService> fs;
        TTree *tree;
        
        int const static maxSize = 100, maxSize2D = 1000;
        
        // Classification results
        Int_t classDecision, simpleClassDecision;
        
        // Information about the chains
        Int_t nChains_;
        Int_t pdgId_[maxSize];
        Int_t flavourSource_[maxSize];
        Float_t pt_[maxSize];
        Float_t eta_[maxSize];
        Float_t phi_[maxSize];
        Float_t energy_[maxSize];
        Int_t length2_[maxSize];
        Int_t length3_[maxSize];
        // Number of parents for each chain, it sums up to sizeParentsGlobal_
        Int_t nParents_[maxSize];
        Int_t nParentsGlobal_;  // needed to emulated dynamic 2D arrays in ROOT tree
        Int_t parentsPdgId_[maxSize2D];
        // Number of sisters for each chain, it sums up to sizeSistersGlobal_
        Int_t nSisters_[maxSize];
        Int_t nSistersGlobal_;  // needed to emulated dynamic 2D arrays in ROOT tree
        Int_t sisters_[maxSize2D];
        
        // Generator information
        UInt_t processId_;
        Int_t pdfIdFirst_, pdfIdSecond_;
        Float_t xFirst_, xSecond_;
        Float_t ptFirst_, etaFirst_, phiFirst_, energyFirst_;
        Float_t ptSecond_, etaSecond_, phiSecond_, energySecond_;
        
        // Information about light partons leaving ME
        Int_t nLight_;
        Int_t pdgIdLight_[maxSize];
        Float_t ptLight_[maxSize];
        Float_t etaLight_[maxSize];
        Float_t phiLight_[maxSize];
        Float_t energyLight_[maxSize];
        
        // Generator jets
        Int_t nJets;
        Float_t ptJet_[maxSize];
        Float_t etaJet_[maxSize];
        Float_t phiJet_[maxSize];
        Float_t energyJet_[maxSize];
};

#endif
