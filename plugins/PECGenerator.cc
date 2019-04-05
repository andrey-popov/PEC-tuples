#include "PECGenerator.h"

#include <FWCore/Framework/interface/EventSetup.h>
#include <FWCore/Framework/interface/MakerMacros.h>
#include <FWCore/Utilities/interface/Exception.h>
#include <FWCore/Utilities/interface/InputTag.h>


using namespace edm;
using namespace std;


PECGenerator::PECGenerator(ParameterSet const &cfg):
    saveAltLHEWeights(cfg.getParameter<bool>("saveAltLHEWeights"))
{
    generatorToken = consumes<GenEventInfoProduct>(cfg.getParameter<InputTag>("generator"));
    
    // LHEEventProduct must be read whenever an LHE-based sample is processed, not just when
    //alternative LHE-level weights are requested. This is because process ID is normally read from
    //the LHE event record. When processing samples without LHE (e.g. pure Pythia), an empty tag
    //should be given to indicate that this information is not available.
    InputTag lheEventInfoTag(cfg.getParameter<InputTag>("lheEventProduct"));
    
    if (lheEventInfoTag.label() != "")
    {
        readLHEEventRecord = true;
        lheEventInfoToken = consumes<LHEEventProduct>(lheEventInfoTag);
    }
    else
    {
        readLHEEventRecord = false;
        
        if (saveAltLHEWeights)
        {
            cms::Exception excp("Configuration");
            excp << "A valid value for lheEventProduct must be provided in order to access " <<
              "alternative LHE-level weights.";
            excp.raise();
        }
    }

    auto const psWeightSetLabel = cfg.getParameter<string>("savePSWeights");

    if (psWeightSetLabel == "all")
        psWeightSet = PSWeightSet::All;
    else if (psWeightSetLabel == "main")
        psWeightSet = PSWeightSet::Main;
    else if (psWeightSetLabel == "none")
        psWeightSet = PSWeightSet::None;
    else
    {
        cms::Exception excp("Configuration");
        excp << "Value \"" << psWeightSetLabel << "\" is no supported for configuration " <<
          "parameter \"savePSWeights\".";
        excp.raise();
    }
}


void PECGenerator::fillDescriptions(ConfigurationDescriptions &descriptions)
{
    ParameterSetDescription desc;
    desc.add<InputTag>("generator", InputTag("generator"))->
      setComment("Tag to access GenEventInfoProduct.");
    desc.add<InputTag>("lheEventProduct", InputTag("externalLHEProducer"))->
      setComment("Tag to access LHEEventProduct. An empty value (\"\") is allowed.");
    desc.add<bool>("saveAltLHEWeights", true)->
      setComment("Requests saving alternative LHE-level weights.");
    desc.add<string>("savePSWeights", "none")->
      setComment("Requests saving of PS weights. Supported options are \"all\", \"main\", and "
                 "\"none\".");
    
    descriptions.add("generator", desc);
}


void PECGenerator::beginJob()
{
    outTree = fileService->make<TTree>("Generator", "Global generator-level properties");
    
    generatorInfoPointer = &generatorInfo;
    outTree->Branch("generator", &generatorInfoPointer);
}


void PECGenerator::analyze(Event const &event, EventSetup const &)
{
    // Reset the buffer from the previous event
    generatorInfo.Reset();
    
    
    // Read generator information for the current event and set process ID
    Handle<GenEventInfoProduct> generator;
    event.getByToken(generatorToken, generator);
    
    Handle<LHEEventProduct> lheEventInfo;
    
    if (readLHEEventRecord)
    {
        event.getByToken(lheEventInfoToken, lheEventInfo);
        generatorInfo.SetProcessId(lheEventInfo->hepeup().IDPRUP);
    }
    else
    {
        // Cannot read process ID as given in the LHE event record. Instead of a default, read one
        //from GenEventInfoProduct
        generatorInfo.SetProcessId(generator->signalProcessID());
    }

    
    
    // Event weights
    generatorInfo.SetNominalWeight(generator->weight());
    
    if (readLHEEventRecord and saveAltLHEWeights)
    {
        // Alternative LHE weights will be rescaled by the ratio between the nominal weight above
        //and the nominal LHE weight, as instructed here [1]
        //[1] https://twiki.cern.ch/twiki/bin/viewauth/CMS/LHEReaderCMSSW?rev=7#How_to_use_weights
        double const factor = generator->weight() / lheEventInfo->originalXWGTUP();
        
        
        // Save the alternative weights
        vector<gen::WeightsInfo> const &altWeights = lheEventInfo->weights();
        
        for (auto const &altWeight: altWeights)
            generatorInfo.AddAltLheWeight(altWeight.wgt * factor);
    }

    vector<double> const &genWeights = generator->weights();

    if (psWeightSet != PSWeightSet::None and genWeights.size() > 1)
    {
        if (psWeightSet == PSWeightSet::Main)
        {
            // The four weights that correspond to independent factor 2 variations in ISR and FSR,
            //as done for NanoAOD [1]
            //[1] https://github.com/cms-sw/cmssw/blob/e580f628505e08ac1577040d47fa2f041125e250/PhysicsTools/NanoAOD/plugins/GenWeightsTableProducer.cc#L240-L248
            for (unsigned index = 6; index < 10; ++index)
                generatorInfo.AddAltPsWeight(genWeights.at(index));
        }
        else
        {
            for (auto const &weight: genWeights)
                generatorInfo.AddAltPsWeight(weight);
        }
    }
        
    
    // PDF information
    GenEventInfoProduct::PDF const *pdf = generator->pdf();
    
    if (pdf)
    {
        generatorInfo.SetPdfXs(pdf->x.first, pdf->x.second);
        generatorInfo.SetPdfIds(pdf->id.first, pdf->id.second);
        generatorInfo.SetPdfQScale(pdf->scalePDF);
    }
    
    
    // Fill the output tree
    outTree->Fill();
}


DEFINE_FWK_MODULE(PECGenerator);
