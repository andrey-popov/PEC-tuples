/**
 * @author Andrey.Popov@cern.ch
 * 
 * The class performs the classification of the event containing heavy flavour quarks.
 */


class HFClass
{
    public:
        enum EventClass
        {
            // The classification is top-to-bottom
            MEPair,  // <0b|ME|bB>
            MESpectator,  // <b|ME|b>
            MECreated,  // <0b|ME|b>
            MEMissFinal,  // <X|ME|>=1b>
            MEDoubleInitial,  //<bb|ME|X>
            MEKilled,  // <1b|ME|0b>
            FSRg,  // single FSR from a gluon
            FSRq,  // single FSR from a quark
            MultipleFSR,  // multiple FSR
            ISR,  // single ISR
            MultipleISR,  // multiple ISR
            UE,  // underlying event
            Unknown, // nothing of the above
            Light  // the given HF not found
        };
        
        enum SimpleEventClass
        {
            SPair,
            SSingle,
            SUE,
            SLight,
            SUnknown
        };
        
        HFClass(int flavour_, Int_t nChains_, Int_t *pdgId_, Int_t *flavourSource_,
         Int_t *nParents_, Int_t *parentsPdgId_, Int_t pdfIdFirst_, Int_t pdfIdSecond_);
        
        void SetPtEtaCut(Float_t *pt_, Float_t *eta_, float ptCut_, float absEtaCut_);
        void Classify();
        EventClass GetClass() const;
        SimpleEventClass GetSimpleClass() const;
        unsigned GetPriority() const;
    
    private:
        int const flavour;
        Int_t const nChains;
        Int_t * const pdgId;
        Int_t * const flavourSource;
        Int_t * const nParents;
        Int_t * const parentsPdgId;
        Int_t pdfIdFirst, pdfIdSecond;
        Float_t *pt, *eta;
        float ptCut, absEtaCut;
        
        EventClass classDecision;
        unsigned priority;  // 0 for ME-related, 1 for PS-related, 2 for UE
};


HFClass::HFClass(int flavour_, Int_t nChains_, Int_t *pdgId_, Int_t *flavourSource_,
 Int_t *nParents_, Int_t *parentsPdgId_, Int_t pdfIdFirst_, Int_t pdfIdSecond_):
    flavour(abs(flavour_)),
    nChains(nChains_),
    pdgId(pdgId_),
    flavourSource(flavourSource_),
    nParents(nParents_),
    parentsPdgId(parentsPdgId_),
    pdfIdFirst(pdfIdFirst_), pdfIdSecond(pdfIdSecond_),
    pt(NULL), eta(NULL),
    ptCut(0.), absEtaCut(100.)
{}


void HFClass::SetPtEtaCut(Float_t *pt_, Float_t *eta_, float ptCut_, float absEtaCut_)
{
    pt = pt_;
    eta = eta_;
    ptCut = ptCut_;
    absEtaCut = absEtaCut_;
}


void HFClass::Classify()
{
    // The auxiliary counters
    int nQLeavingME = 0;
    int nQbarLeavingME = 0;
    int nQEnteringME = 0;
    int nQbarEnteringME = 0;
    int nQFSR = 0;
    int nQbarFSR = 0;
    int nQISR = 0;
    int nQbarISR = 0;
    int nQUE = 0;
    
    int chFSR = -1;  // index of a FSR chain (any one)
    int pdgIdParentFSR = -1;  // pdgId of the first parent of the chain
    
    bool passKinematic = false;
    bool HFFound = false;
    
    
    for (int ch = 0, nParentsGlobal = 0; ch < nChains; ++ch)
    {
        if (abs(pdgId[ch]) == flavour)
        {
            HFFound = true;
            
            if (pt  &&  pt[ch] > ptCut  &&  fabs(eta[ch]) < absEtaCut)
                passKinematic = true;
            
            if (flavourSource[ch] == 2)  // ME final state
            {
                if (pdgId[ch] > 0)
                    ++nQLeavingME;
                else
                    ++nQbarLeavingME;
            }
            
            if (flavourSource[ch] == 1)  // FSR
            {
                chFSR = ch;
                pdgIdParentFSR = parentsPdgId[nParentsGlobal];
                
                if (pdgId[ch] > 0)
                    ++nQFSR;
                else
                    ++nQbarFSR;
            }
            
            if (flavourSource[ch] == 5)  // ISR
            {
                if (pdgId[ch] > 0)
                    ++nQISR;
                else
                    ++nQbarISR;
            }
            
            if (flavourSource[ch] == 4)  // UE
                ++nQUE;
        }
        
        nParentsGlobal += nParents[ch];
    }
    
    if (pdfIdFirst == flavour)
        ++nQEnteringME;
    if (pdfIdSecond == flavour)
        ++nQEnteringME;
    if (pdfIdFirst == -flavour)
        ++nQbarEnteringME;
    if (pdfIdSecond == -flavour)
        ++nQbarEnteringME;
    
    
    if (!HFFound  ||  (pt  &&  !passKinematic))
    {
        classDecision = Light;
        priority = 4;
        return;
    }
    
    
    // The actual classification
    if (nQLeavingME == 1  &&  nQbarLeavingME == 1  &&  nQEnteringME + nQbarEnteringME == 0)
        classDecision = MEPair;
    else if ((nQLeavingME == 1  &&  nQbarLeavingME == 0  &&  nQEnteringME == 1  &&
     nQbarEnteringME == 0)  ||  (nQbarLeavingME == 1  &&  nQLeavingME == 0  &&
     nQbarEnteringME == 1  &&  nQEnteringME == 0))
        classDecision = MESpectator;
    else if (nQLeavingME + nQbarLeavingME == 1  &&  nQEnteringME + nQbarEnteringME == 0)
        classDecision = MECreated;
    else if (nQLeavingME + nQbarLeavingME > 0)
        classDecision = MEMissFinal;
    else if (nQEnteringME + nQbarEnteringME == 2)
        classDecision = MEDoubleInitial;
    else if (nQEnteringME + nQbarEnteringME == 1)
        classDecision = MEKilled;
    else if (nQFSR + nQbarFSR == 2)
    {
        if (nParents[chFSR] == 1)
        {
            if (pdgIdParentFSR == 21)  // gluon
                classDecision = FSRg;
            else
                classDecision = FSRq;
        }
        else
            classDecision = Unknown;
    }
    else if (nQFSR + nQbarFSR > 2)
        classDecision = MultipleFSR;
    else if (nQISR + nQbarISR == 2)
        classDecision = ISR;
    else if (nQISR + nQbarISR > 2)
        classDecision = MultipleISR;
    else if (nQUE > 0)
        classDecision = UE;
    else
        classDecision = Unknown;
    
    
    // Set up the priority
    if (classDecision <= MEKilled)
        priority = 0;
    else if (classDecision <= MultipleISR)
        priority = 1;
    else if (classDecision == UE)
        priority = 2;
    else
        priority = 3;
}


HFClass::EventClass HFClass::GetClass() const
{
    return classDecision;
}


HFClass::SimpleEventClass HFClass::GetSimpleClass() const
{
    if (classDecision == MEPair  ||  classDecision == MESpectator  ||  classDecision == FSRg  ||
     classDecision == FSRq  ||  classDecision == MultipleFSR  ||  classDecision == ISR  ||
     classDecision == MultipleISR)
        return SPair;
    
    if (classDecision == MECreated  ||  classDecision == MEKilled)
        return SSingle;
    
    if (classDecision == UE)
        return SUE;
    
    if (classDecision == Light)
        return SLight;
    
    return SUnknown;
}


unsigned HFClass::GetPriority() const
{
    return priority;
}