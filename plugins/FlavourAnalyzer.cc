/**
 * @author Andrey.Popov@cern.ch
 *
 * Description: see the header file.
 */

#include <UserCode/SingleTop/interface/FlavourAnalyzer.h>
#include <UserCode/SingleTop/interface/HFClass.h>

#include <DataFormats/HepMCCandidate/interface/GenParticle.h>
#include <SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h>
#include <FWCore/Utilities/interface/EDMException.h>
#include <FWCore/Framework/interface/MakerMacros.h>

#include <cmath>
#include <vector>
#include <list>
#include <algorithm>


using edm::View;
using edm::Handle;
using reco::GenParticle;
using std::vector;
using std::list;
using std::find;


/* The scructure to store the information about the genetically connected consecutive partons
 * of the same flavour. Applied to b- or c-quarks only. */
struct FlavourChain
{
    enum FlavourSource
    {
        undefined,   // nothing of the below
        FSR,         // the chain contains status 2 partons only
        ME,          // the last status 3 parton in the chain has status 2 daughters only
        PDF,         // the chain contains status 3 partons only (these are entering ME partons)
        UE,          // partons have status 2 and are daughters of the initial proton
        ISR          // status 2 partons that are daughters of the initial section (except for UE)
    };
    
    int pdgId;
    FlavourSource flavourSource;
    int length2;  // number of status 2 partons in the chain
    int length3;  // number of status 3 patrons in the chain
    vector<GenParticle const *> content;
    vector<GenParticle const *> parents;  // parents of the first (ancestor) parton in the chain
    vector<int> parentsPdgId;
    vector<int> sisterChains;
};


FlavourAnalyzer::FlavourAnalyzer(const edm::ParameterSet &cfg):
    genParticlesSrc(cfg.getParameter<edm::InputTag>("genParticles")),
    generatorSrc(cfg.getParameter<edm::InputTag>("generator")),
    genJetsSrc(cfg.getParameter<edm::InputTag>("genJets")),
    ptCut(cfg.exists("jetPtCut") ? cfg.getParameter<double>("jetPtCut") : 20.),
    minimalisticChains(cfg.exists("saveMinimalisticChains") ?
     cfg.getParameter<bool>("saveMinimalisticChains") : false),
    savePDFInfo(cfg.exists("savePDFInfo") ? cfg.getParameter<bool>("savePDFInfo") : false),
    saveLightPartons(cfg.exists("saveLightPartons") ?
     cfg.getParameter<bool>("saveLightPartons") : false),
    saveGenJets(cfg.exists("saveGenJets") ? cfg.getParameter<bool>("saveGenJets") : false),
    classify(cfg.exists("classify") ? cfg.getParameter<bool>("classify") : true)
{}


FlavourAnalyzer::~FlavourAnalyzer()
{}


void FlavourAnalyzer::beginJob()
{
    tree = fs->make<TTree>("FlavourTree", "");
    
    if (classify)
    {
        tree->Branch("class", &classDecision);
        tree->Branch("simpleClass", &simpleClassDecision);
    }
    
    tree->Branch("nChains", &nChains_);
    tree->Branch("pdgId", pdgId_, "pdgId[nChains]/I");
    tree->Branch("flavourSource", flavourSource_, "flavourSource[nChains]/I");
    
    if (!minimalisticChains)
    {
        tree->Branch("pt", pt_, "pt[nChains]/F");
        tree->Branch("eta", eta_, "eta[nChains]/F");
        tree->Branch("phi", phi_, "phi[nChains]/F");
        tree->Branch("energy", energy_, "energy[nChains]/F");
        tree->Branch("length2", length2_, "length2[nChains]/I");
        tree->Branch("length3", length3_, "length3[nChains]/I");
        tree->Branch("nParents", nParents_, "nParents[nChains]/I");
        tree->Branch("nParentsGlobal", &nParentsGlobal_);
        tree->Branch("parentsPdgId", parentsPdgId_, "parentsPdgId[nParentsGlobal]/I");
        tree->Branch("nSisters", nSisters_, "nSisters[nChains]/I");
        tree->Branch("nSistersGlobal", &nSistersGlobal_);
        tree->Branch("sisters", sisters_, "sisters[nSistersGlobal]/I");
    }
    
    
    if (savePDFInfo)
    {
        tree->Branch("processId", &processId_);
        
        tree->Branch("pdfIdFirst", &pdfIdFirst_);
        tree->Branch("xFirst", &xFirst_);
        tree->Branch("ptFirst", &ptFirst_);
        tree->Branch("etaFirst", &etaFirst_);
        tree->Branch("phiFirst", &phiFirst_);
        tree->Branch("energyFirst", &energyFirst_);
        
        tree->Branch("pdfIdSecond", &pdfIdSecond_);
        tree->Branch("xSecond", &xSecond_);
        tree->Branch("ptSecond", &ptSecond_);
        tree->Branch("etaSecond", &etaSecond_);
        tree->Branch("phiSecond", &phiSecond_);
        tree->Branch("energySecond", &energySecond_);
    }
    
    if (saveLightPartons)
    {
        tree->Branch("nLight", &nLight_);
        tree->Branch("pdgIdLight", pdgIdLight_, "pdgIdLight[nLight]/I");
        tree->Branch("ptLight", ptLight_, "ptLight[nLight]/F");
        tree->Branch("etaLight", etaLight_, "etaLight[nLight]/F");
        tree->Branch("phiLight", phiLight_, "phiLight[nLight]/F");
        tree->Branch("energyLight", energyLight_, "energyLight[nLight]/F");
    }
    
    if (saveGenJets)
    {
        tree->Branch("nJets", &nJets);
        tree->Branch("ptJet", ptJet_, "ptJet[nJets]/F");
        tree->Branch("etaJet", etaJet_, "etaJet[nJets]/F");
        tree->Branch("phiJet", phiJet_, "phiJet[nJets]/F");
        tree->Branch("energyJet", energyJet_, "energyJet[nJets]/F");
    }
}


void FlavourAnalyzer::endJob()
{}


void FlavourAnalyzer::analyze(const edm::Event &event, const edm::EventSetup &eventSetup)
{
    // Get the generator particles collection from the event
    Handle<View<GenParticle> > genParticles;
    event.getByLabel(genParticlesSrc, genParticles);
    
    // It is more convenient to work with a list of pointers, not the original vector of objects
    list<GenParticle const *> particles;
    
    for (View<GenParticle>::const_iterator p = genParticles->begin(); p != genParticles->end(); ++p)
        particles.push_back(&*p);
    
    
    // Find the chains (consecutive partons of the same flavour) in the particles collection
    vector<FlavourChain> flavourChains;
    
    for (list<GenParticle const *>::iterator partIt = particles.begin(); partIt != particles.end();)
    {
        int pdgId = (*partIt)->pdgId();
        
        if (abs(pdgId) != 4  &&  abs(pdgId) != 5)  // we are interested in b- or c-quarks only
        {
            ++partIt;
            continue;
        }
        
        
        // A b- or c-quark is found. Now we look for its chain, put it into the flavourChains vector
        //and remove all the found quarks from the particles collection. Note that the list does
        //not contain any b- or c-partons before the current position of the iterator, we really
        //can remove the partons from the list not turning it into a mess
        FlavourChain chain;
        chain.pdgId = pdgId;
        
        // First we store temporarily the path to the final ancestor of the found parton. This is
        //needed because if we just rewind to the final ancestor and then go back taking the first
        //daughter each time, we might not return to the particle partIt (in case of multiple
        //daughters with the given pdgId). However for the quarks it should not be the case, but
        //still it is nice to preserve the generality of the algorithm. Also the found parton will
        //probably never have mothers of the same flavour, but it is not guaranteed.
        vector<GenParticle const *> ancestors;
        GenParticle const *currentParticle = *partIt;
        
        do
        {
            // Iterate the mothers
            const int nMothers = currentParticle->numberOfMothers();
            int i = 0;
            
            for (; i < nMothers; ++i)
                if (currentParticle->mother(i)->pdgId() == chain.pdgId)
                // The first mother with the same pdgId is chosen
                {
                    // Do not consider mothers in the initial section, because if we have, e.g. a
                    //charm entering ME and a charm leaving it, we do not want to consider them as
                    //one chain
                    if (currentParticle->mother(i)->status() == 3  &&
                      currentParticle->mother(i)->mother(0)->mother(0)->pdgId() == 2212)  //proton
                        continue;
                    
                    // Add the mother if only it is found within the particles list. There can be
                    //showering c -> c cbar c X, in which case the initial c is the mother of the
                    //two resulting c, and one of them might have already taken the mother
                    if (find(particles.begin(), particles.end(), currentParticle->mother(i)) ==
                      particles.end())
                        continue;
                    else
                    {
                        currentParticle =
                          dynamic_cast<GenParticle const *>(currentParticle->mother(i));
                        ancestors.push_back(currentParticle);
                        break;
                    }
                }
            
            if (i == nMothers)  // i.e. the mother of the same flavour is not found
                currentParticle = NULL;
        }
        while (currentParticle);
        
        // Now push the ancestors to the chain
        for (vector<GenParticle const *>::const_reverse_iterator a = ancestors.rbegin();
          a != ancestors.rend(); ++a)
            chain.content.push_back(*a);
        
        
        // Find the descendants and push them to the chain
        currentParticle = *partIt;
        chain.content.push_back(currentParticle);
        
        // If the heavy flavour is in the initial section do not look for daughters
        if (currentParticle->status() == 3  &&
          currentParticle->mother(0)->mother(0)->pdgId() == 2212)  // proton
            currentParticle = NULL;
        
        while (currentParticle)
        {
            // Iterate the daughters
            const int nDaughters = currentParticle->numberOfDaughters();
            int i = 0;
            
            for (; i < nDaughters; ++i)
                if (currentParticle->daughter(i)->pdgId() == chain.pdgId)
                // The first daughter of the same pdgId is saved only
                {
                    // This is mostly to preserve generality. Do not think that, e.g., charm can
                    //have more than one mother of the same flavour
                    if (find(particles.begin(), particles.end(), currentParticle) ==
                      particles.end())
                        continue;
                    else
                    {
                        currentParticle =
                          dynamic_cast<GenParticle const *>(currentParticle->daughter(i));
                        chain.content.push_back(currentParticle);
                        break;
                    }
                }
            
            if (i == nDaughters)  // i.e. the daughter of the same flavour is not found
                currentParticle = NULL;
        }
        
        
        // The chain's content is completely filled. Push it to the vector of chains
        flavourChains.push_back(chain);
        
        
        // As the chain content is filled, we remove the found partons from the particles list (and
        //the std::list is good for it ;) ). Since the particle pointed to by iterator partIt will
        //be removed for sure, and the iterator will be invalidated, we step one position back. Note
        //that b- or c-parton is never the first in the list (the first two particles are the
        //initial protons)
        --partIt;  // now it points to the so far latest non b- or c-parton in the list
        
        for (vector<GenParticle const *>::const_iterator chainContentIt = chain.content.begin();
          chainContentIt != chain.content.end(); ++chainContentIt)
        {
            list<GenParticle const *>::iterator found = find(particles.begin(), particles.end(),
              *chainContentIt);
            
            if (found == particles.end())  // well, it is supposed not to happen ever
            {
                edm::Exception excp(edm::errors::LogicError);
                excp << "Double counting is found when constructing chains. Some partons are " <<
                  "included into several chains. It is a bug.\n";
                excp.raise();
            }
            
            particles.erase(found);
        }
        
        // Now step to the first particle that has not been viewed yet
        ++partIt;
    }  // end of loop over the particles list
    
    // Nothing to store in the tree if no HF is found in the event
    //if (flavourChains.size() == 0)
    //    return;
    
    
    // Fill the missing data members for the chains, except for the info about the sisters
    for (vector<FlavourChain>::iterator chainIt = flavourChains.begin();
      chainIt != flavourChains.end(); ++chainIt)
    {
        // Fill the parents of the final ancestor
        const int nParents = chainIt->content.front()->numberOfMothers();
        bool hasProtonAsParent = false;       // needed to determine the flavour source
        bool hasProtonAsGrandParent = false;  //
        
        for (int i = 0; i < nParents; ++i)
        {
            chainIt->parents.push_back(
              dynamic_cast<GenParticle const *>(chainIt->content.front()->mother(i)));
            chainIt->parentsPdgId.push_back(chainIt->parents.back()->pdgId());
            
            if (chainIt->parentsPdgId.back() == 2212)
                hasProtonAsParent = true;
            
            const int nGrandParents = chainIt->parents.back()->numberOfMothers();
            
            for (int j = 0; j < nGrandParents; ++j)
                if (chainIt->parents.back()->mother(j)->pdgId() == 2212)
                    hasProtonAsGrandParent = true;
        }
        
        
        // Fill the lengths and parent. Also collect info for flavour source derivation
        chainIt->length2 = chainIt->length3 = 0;
        // Indices of the first status 2, last status 3 particles, and the last status 3 particle
        //having no status 3 daughters
        int firstStatus2Particle = -1;
        int lastStatus3Particle = -1;
        int lastNoStatus3DaughtersParticle = -1;
        const vector<GenParticle const *>::const_iterator contentBegin = chainIt->content.begin(),
          contentEnd = chainIt->content.end();
        
        for (vector<GenParticle const *>::const_iterator p = contentBegin, pNext; p != contentEnd; )
        {
            pNext = p + 1;
            
            if ((*p)->status() == 2)
            {
                ++(chainIt->length2);
                
                if (firstStatus2Particle == -1)
                    firstStatus2Particle = p - contentBegin;
            }
            else if ((*p)->status() == 3)
            {
                ++(chainIt->length3);
                lastStatus3Particle = p - contentBegin;
                
                if (pNext != contentEnd  &&  (*pNext)->status() != 3)
                // We are looking for a parton with no status 3 daughters
                {
                    // Check the daughters
                    const int nDaughters = (*p)->numberOfDaughters();
                    int i = 0;
                    
                    for (; i < nDaughters; ++i)
                        if ((*p)->daughter(i)->status() == 3)
                            break;
                    
                    if (i == nDaughters)  // i.e. no status 3 daughters found
                        lastNoStatus3DaughtersParticle = p - contentBegin;
                }
            }
            
            p = pNext;
        }  // end of loop over the content of the current chain
        
        
        // Make decision about the flavour source of the current chain
        if (chainIt->length3 == 0  &&  chainIt->length2 != 0  &&  !hasProtonAsParent  &&
          !hasProtonAsGrandParent)
            chainIt->flavourSource = FlavourChain::FSR;
        else if (chainIt->length3 == 0  &&  chainIt->length2 != 0  &&  hasProtonAsParent)
            chainIt->flavourSource = FlavourChain::UE;
        else if (chainIt->length3 == 0  &&  chainIt->length2 != 0  &&  hasProtonAsGrandParent)
            chainIt->flavourSource = FlavourChain::ISR;
        else if (chainIt->length3 != 0  &&  chainIt->length2 == 0)
            chainIt->flavourSource = FlavourChain::PDF;
        else if (chainIt->length3 != 0  &&  chainIt->length2 != 0  &&
                 lastStatus3Particle == lastNoStatus3DaughtersParticle  &&
                 firstStatus2Particle - lastStatus3Particle == 1)
            chainIt->flavourSource = FlavourChain::ME;
        else
            chainIt->flavourSource = FlavourChain::undefined;
    }  // end of loop over the flavour chains
    
    
    // Now the flavour chains are almost filled. The only data member left to set is sisters
    const int nChains = flavourChains.size();
    
    for (int iChain = 0; iChain < nChains; ++iChain)
        for (int jChain = iChain + 1; jChain < nChains; ++jChain)
        {
            // Check if this two chains have common parents
            bool commonParent = false;
            const vector<GenParticle const *>::const_iterator
              parentsJBegin = flavourChains[jChain].parents.begin(),
              parentsJEnd = flavourChains[jChain].parents.end();
            
            for (vector<GenParticle const *>::const_iterator parentIIt =
              flavourChains[iChain].parents.begin(); parentIIt != flavourChains[iChain].parents.end();
              ++parentIIt)
            {
                vector<GenParticle const *>::const_iterator found =
                  find(parentsJBegin, parentsJEnd, *parentIIt);
                
                if (found != parentsJEnd)
                {
                    commonParent = true;
                    break;
                }
            }
            
            
            // Save the information about the common parents
            if (commonParent)
            {
                flavourChains[iChain].sisterChains.push_back(jChain);
                flavourChains[jChain].sisterChains.push_back(iChain);
            }
        }  // end of internal loop over the chains
    // end of loop over the chains
    
    
    // Now we are ready to out the information into the ROOT tree
    nChains_ = nChains;
    int iParentGlobal = 0, iSisterGlobal = 0;
    
    for (int iChain = 0; iChain < nChains  &&  iChain < maxSize; ++iChain)
    {
        pdgId_[iChain] = flavourChains[iChain].pdgId;
        flavourSource_[iChain] = flavourChains[iChain].flavourSource;
        pt_[iChain] = flavourChains[iChain].content[0]->pt();
        eta_[iChain] = flavourChains[iChain].content[0]->eta();
        phi_[iChain] = flavourChains[iChain].content[0]->phi();
        energy_[iChain] = flavourChains[iChain].content[0]->energy();
        length2_[iChain] = flavourChains[iChain].length2;
        length3_[iChain] = flavourChains[iChain].length3;
        nParents_[iChain] = flavourChains[iChain].parentsPdgId.size();
        nSisters_[iChain] = flavourChains[iChain].sisterChains.size();
        
        for (int iParent = 0; iParent < nParents_[iChain]  &&  iParentGlobal < maxSize2D;
          ++iParent)
            parentsPdgId_[iParentGlobal++] = flavourChains[iChain].parentsPdgId[iParent];
        
        for (int iSister = 0; iSister < nSisters_[iChain]  &&  iSisterGlobal < maxSize2D;
          ++iSister)
            sisters_[iSisterGlobal++] = flavourChains[iChain].sisterChains[iSister];
    }
    
    nParentsGlobal_ = iParentGlobal;
    nSistersGlobal_ = iSisterGlobal;
    
    
    // Classify the event
    if (classify)
    {
        HFClass bHF(5, nChains_, pdgId_, flavourSource_, nParents_, parentsPdgId_,
         pdfIdFirst_, pdfIdSecond_);
        bHF.Classify();
        HFClass cHF(4, nChains_, pdgId_, flavourSource_, nParents_, parentsPdgId_,
         pdfIdFirst_, pdfIdSecond_);
        cHF.Classify();
        
        HFClass::EventClass bCl = bHF.GetClass(), cCl = cHF.GetClass();
        int HFToSelect = -1;  // 0 -- light, 1 -- c, 2 -- b
        
        if (bCl != HFClass::Light  &&  cCl != HFClass::Light)  // collision
        {
            if (bHF.GetPriority() <= cHF.GetPriority())  // within the same priority b is favoured
                HFToSelect = 2;
            else
                HFToSelect = 1;
        }
        else
        {
            if (bCl != HFClass::Light)
                HFToSelect = 2;
            else if (cCl != HFClass::Light)
                HFToSelect = 1;
            else
                HFToSelect = 0;
        }
        
        if (HFToSelect == 2)
        {
            classDecision = bCl;
            simpleClassDecision = bHF.GetSimpleClass();
        }
        else if (HFToSelect == 1)
        {
            classDecision = cCl;
            simpleClassDecision = cHF.GetSimpleClass();
        }
        else
        {
            classDecision = HFClass::Light;
            simpleClassDecision = HFClass::SLight;
        }
    }
    
    
    // Add some generator info to store
    if (savePDFInfo)
    {
        Handle<GenEventInfoProduct> generator;
        event.getByLabel(generatorSrc, generator);
        
        processId_ = generator->signalProcessID();
        const GenParticle &pdfFirst = genParticles->at(4);
        const GenParticle &pdfSecond = genParticles->at(5);
        
        ptFirst_ = pdfFirst.pt();
        etaFirst_ = pdfFirst.eta();
        phiFirst_ = pdfFirst.phi();
        energyFirst_ = pdfFirst.energy();
        
        ptSecond_ = pdfSecond.pt();
        etaSecond_ = pdfSecond.eta();
        phiSecond_ = pdfSecond.phi();
        energySecond_ = pdfSecond.energy();
       
        if (generator->hasPDF())
        {
            const GenEventInfoProduct::PDF *pdf = generator->pdf();
            pdfIdFirst_ = pdf->id.first;
            pdfIdSecond_ = pdf->id.second;
            
            // Here 0 is used to denote gluon; let's change it to the standard PDG value
            if (pdfIdFirst_ == 0)
                pdfIdFirst_ = 21;
            
            if (pdfIdSecond_ == 0)
                pdfIdSecond_ = 21;
            
            xFirst_ = pdf->x.first;
            xSecond_ = pdf->x.second;
        }
        else
        {
            pdfIdFirst_ = pdfFirst.pdgId();
            pdfIdSecond_ = pdfSecond.pdgId();
            
            xFirst_ = 999.;
            xSecond_ = 999.;
        }
    }
    
    
    // Add the information about the partons leaving ME (except for HF already accounted for)
    if (saveLightPartons)
    {
        nLight_ = 0;
        
        for (list<GenParticle const *>::const_iterator partIt = particles.begin();
          partIt != particles.end(); ++partIt)
        {
            int pdgId = (*partIt)->pdgId();
            
            // We are interested in u, d, s, g only
            if (abs(pdgId) > 3  &&  pdgId != 21)
                continue;
            
            // A particle from ME has status 3...
            if ((*partIt)->status() != 3)
                continue;
            
            // ...and do not have daughter with status 3
            const int nDaughters = (*partIt)->numberOfDaughters();
            int i = 0;
            
            for (; i < nDaughters; ++i)
                if ((*partIt)->daughter(i)->status() == 3)
                    break;
            
            if (i != nDaughters)  // i.e. a status 3 daughter was found and the loop was broken
                continue;
            
            
            // Now we have what we need
            pdgIdLight_[nLight_] = pdgId;
            ptLight_[nLight_] = (*partIt)->pt();
            etaLight_[nLight_] = (*partIt)->eta();
            phiLight_[nLight_] = (*partIt)->phi();
            energyLight_[nLight_] = (*partIt)->energy();
            
            ++nLight_;
        }
    }
    
    
    // Fill the generator jets
    if (saveGenJets)
    {
        Handle<View<reco::Candidate> > genJets;
        event.getByLabel(genJetsSrc, genJets);
        
        nJets = 0;
        
        for (View<reco::Candidate>::const_iterator j = genJets->begin(); j != genJets->end(); ++j)
        {
            if (j->pt() < ptCut)
                break;
            
            ptJet_[nJets] = j->pt();
            etaJet_[nJets] = j->eta();
            phiJet_[nJets] = j->phi();
            energyJet_[nJets] = j->energy();
            
            ++nJets;
        }
    }
    
    
    // Now everything is calculated
    tree->Fill();
}


DEFINE_FWK_MODULE(FlavourAnalyzer);
