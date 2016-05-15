#include <Analysis/PECTuples/interface/Candidate.h>
#include <Analysis/PECTuples/interface/CandidateWithID.h>
#include <Analysis/PECTuples/interface/Lepton.h>
#include <Analysis/PECTuples/interface/Muon.h>
#include <Analysis/PECTuples/interface/Electron.h>
#include <Analysis/PECTuples/interface/Jet.h>
#include <Analysis/PECTuples/interface/GenParticle.h>
#include <Analysis/PECTuples/interface/GenJet.h>

#include <Analysis/PECTuples/interface/EventID.h>
#include <Analysis/PECTuples/interface/PileUpInfo.h>
#include <Analysis/PECTuples/interface/GeneratorInfo.h>

#include <vector>


// Instantiate templates
template class std::vector<pec::Candidate>;
template class std::vector<pec::CandidateWithID>;
template class std::vector<pec::Lepton>;
template class std::vector<pec::Muon>;
template class std::vector<pec::Electron>;
template class std::vector<pec::Jet>;
template class std::vector<pec::GenParticle>;
template class std::vector<pec::GenJet>;
