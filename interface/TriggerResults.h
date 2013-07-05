/**
 * @author Andrey.Popov@cern.ch
 * 
 * The plugin saves trigger results and performs filtering on them, depending on the set flags.
 * The triggers to store are selected with the help of two sets of POSIX regular expressions which
 * select and veto triggers (white and black list resp.). Please note that the backslashes must be
 * double escaped, i.e. an example of the correct string to define a regular expression is as
 * follows: '^HTL_Jet\\\\d+_v\\\\d+$' or r'^HLT_Jet\\d+_v\\d+$' (using python raw string). The
 * regular expressions are checked for occurence, not for exact match (i.e. search, not match). The
 * way this lists are combined is govern with 'orMode' flag. In filtering mode it defines the
 * triggers that had to fire for the event to pass the filtering.
 * 
 * Input parameters:
 *   triggerProcessName (string): name of the trigger process, defaults to 'HTL'
 *   whiteList (vstring): list of POSIX regular expressions, by default matches any trigger
 *   blackList (vstring): same
 *   orMode (bool): if true (false) a trigger is selected if its name matches a pattern in the white
 *                  list OR (AND) doesn't match any pattern in the black list
 *   filter (bool): determines whether the event filtering is enabled, defaults to true
 *   dumper (bool): determines whether the trigger information is saved to a ROOT file, defaults to
 *                  false
 * 
 * When the dumper mode is active, the plugin saves the list of trigger names that passed the
 * selection and also the bit indicating whether the trigger was accepted and its prescale factor.
 * 
 * Usage example:
 *   process.triggerInfo = cms.EDFilter('TriggerResults',
 *     whiteList = cms.vstring(
 *       r'^HLT_Jet\\d+_v\\d+$', r'^HLT_(Iso)?Mu\\d+_', r'^HLT_Ele\\d+_', r'^HLT_DoubleMu\\d+_',
 *       r'^HLT_DoubleEle\\d+_'),
 *     blackList = cms.vstring(
 *       r'_Jpsi_', r'_PFMT\\d+', r'_R\\d+_', r'Photon\\d+_', r'PFTau\\d+_', r'_HT\\d+_',
 *       r'_PFMHT\\d+_', r'Acoplanarity'),
 *     filter = cms.bool(False),
 *     dumper = cms.bool(True),
 *     orMode = cms.bool(False))
 */

#ifndef SINGLE_TOP_TRIGGER_RESULTS
#define SINGLE_TOP_TRIGGER_RESULTS

#include <FWCore/Framework/interface/EDFilter.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <HLTrigger/HLTcore/interface/HLTConfigProvider.h>
#include <FWCore/ServiceRegistry/interface/Service.h>
#include <CommonTools/UtilAlgos/interface/TFileService.h>

#include <TTree.h>
#include <TClonesArray.h>

#include <boost/regex.hpp>
#include <vector>
#include <string>


using std::vector;
using std::string;


#define MAX_LEN 512  // maximum size used to allocate the arrays


class TriggerResults: public edm::EDFilter
{
    public:
        TriggerResults(const edm::ParameterSet &);
        ~TriggerResults();
    
    private:
        void beginJob();
        void endJob();
        bool beginRun(edm::Run &, const edm::EventSetup &);
        bool endRun(edm::Run &, const edm::EventSetup &);
        bool filter(edm::Event &, const edm::EventSetup &);
        
        // The method chechs if the given trigger name can match any of the regular expressions
        bool matchTriggerName(const string &, const vector<boost::regex> &);
        
        const string triggerProcessName;  // the name of the trigger process (usually, "HLT")
        const vector<string> whiteList;  // the regular expressions defining the triggers to keep
        const vector<string> blackList;  //
        const bool orMode;  // defines the mode of combination of inWhiteList and !inBlackList
        const bool filterOn;  // should the plugin filter events?
        const bool dumperOn;  // should the plugin save the trigger information in a ROOT file?
        
        vector<boost::regex> whiteListRegex;  // compiled regular expressions
        vector<boost::regex> blackListRegex;  //
        vector<string> selectedTriggers;  // the triggers' names that satisfy the selection
        
        HLTConfigProvider hltConfigProvider;  // the object providing some trigger information
        
        edm::Service<TFileService> fs;
        TTree *triggerInfoTree;  // the tree to store the trigger information
        
        Int_t size;  // number of selected trigger names in the current entry
        TClonesArray names;  // array of the selected trigger names
        TClonesArray *namesPointer;  // this pointer is needed to be fed into TTree::Branch
        //Bool_t wasRun[MAX_LEN];  // it's always true -- useless
        Bool_t hasFired[MAX_LEN];  // whether the trigger was accepted
        Int_t prescale[MAX_LEN];  // the prescale factor
};

#endif
