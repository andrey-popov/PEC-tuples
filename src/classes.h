#include <UserCode/SingleTop/interface/Candidate.h>
#include <UserCode/SingleTop/interface/CandidateWithID.h>
#include <UserCode/SingleTop/interface/Lepton.h>
#include <UserCode/SingleTop/interface/Muon.h>
#include <UserCode/SingleTop/interface/Electron.h>

#include <vector>


// Instantiate templates
template class std::vector<pec::Candidate>;
template class std::vector<pec::CandidateWithID>;
template class std::vector<pec::Lepton>;
template class std::vector<pec::Muon>;
template class std::vector<pec::Electron>;
