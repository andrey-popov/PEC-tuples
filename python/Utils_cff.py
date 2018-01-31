"""Utility classes and functions used in the main configuration."""

from __future__ import print_function
import re

import FWCore.ParameterSet.Config as cms


class PathManager:
    """A class to work with multiple CMS paths simultaneuosly."""
    
    def __init__(self, *paths):
        """Construct from an arbitrary number of cms.Path."""
        
        self.paths = list(paths)
    
    
    def associate(self, *tasks):
        """Associate each path with given tasks."""
        
        for p in self.paths:
            p.associate(*tasks)
    
    
    def append(self, *modules):
        """Append one or more modules to each path."""
        
        for p in self.paths:
            for m in modules:
                p += m


def add_jet_selection(selection, process, paths, runOnData, src='analysisPatJets', verbose=True):
    """Implement jet selection.
    
    Add modules implementing a loose event selection based on properties
    of jets to the process.  The selection is described with a text
    string of the following form:
      <n>j<minPt> <m><bTagAlgo><bTagCut>
    This will select events with at least n jets with pt > minPt, out of
    which at least m jets have the b-tagging discriminator of type
    bTagAlgo larger than bTagCut.  The following b-tagging algorithms
    are supported:
      CSV, cMVA
    Either kinematic selection or the b-tagging requirement can be
    omitted.
    
    Arguments:
        selection: String defining the event selection following the
            format described above.
        process: Process to which producers and filters are added.
        paths: Paths to which filters are added.
        runOnData: Flag to disctinguish processing of data and
            simulation.
        src: Name of the input collection of jets.
        verbose: Flag that controls print-out when the configuration is
            executed.
    
    Return value:
        None.
    """
    
    if not selection:
        return
    
    
    # Parse the selection string considering several options
    minNumJets = 0
    minPt = 0.
    minBTags = 0
    bTagAlgo = ''
    minBDiscr = -float('inf')
    
    patternKin = r'(\d+)\s*j\s*(\d+(\.\d+)?)'
    patternBTag = r'(\d+)\s*([A-Za-z]+)\s*([+-]?\d+(\.\d+)?)'
    
    match = re.match(r'{}\s+{}'.format(patternKin, patternBTag), selection)
    
    if match:
        minNumJets = int(match.group(1))
        minPt = float(match.group(2))
        minBTags = int(match.group(4))
        bTagAlgo = match.group(5)
        minBDiscr = float(match.group(6))
    
    else:
        match = re.match(patternKin, selection)
        
        if match:
            minNumJets = int(match.group(1))
            minPt = float(match.group(2))
        
        else:
            match = re.match(patternBTag, selection)
            
            if match:
                minBTags = int(match.group(1))
                bTagAlgo = match.group(2)
                minBDiscr = float(match.group(3))
            else:
                raise RuntimeError('Failed to parse jet selection string "{}".'.format(selection))
    
    
    if verbose and (minNumJets > 0 or minBTags > 0):
        print('Will select events with ', end='')
        
        if minNumJets > 0:
            print('at least {} jets with pt > {} GeV'.format(minNumJets, minPt), end='')
            
            if minBTags > 0:
                print(' and ', end='')
            else:
                print()
        
        if minBTags > 0:
            print('at least {} jets with {} > {}'.format(minBTags, bTagAlgo, minBDiscr))
    
    
    # Kinematic selection
    if minNumJets > 0:
        process.jetsForEventSelection = cms.EDFilter('JERCJetSelector',
            src = cms.InputTag(src),
            jetTypeLabel = cms.string('AK4PFchs'),
            minPt = cms.double(minPt),
            includeJERCVariations = cms.bool(not runOnData),
            genJets = cms.InputTag('slimmedGenJets'),
            rho = cms.InputTag('fixedGridRhoFastjetAll'),
            minNum = cms.uint32(minNumJets)
        )
        paths.append(process.jetsForEventSelection)
    
    
    # Selection based on b-tags
    if minBTags > 0:
        
        bTagAlgoMap = {
            'CSV': 'pfCombinedInclusiveSecondaryVertexV2BJetTags',
            'CMVA': 'pfCombinedMVAV2BJetTags'
        }
        
        try:
            bTagAlgoExpanded = bTagAlgoMap[bTagAlgo.upper()]
        except KeyError:
            raise RuntimeError(
                'Uknown label "{}" provided for b-tagging algorithm'.format(bTagAlgo)
            )
        
        process.bTaggedJetsForEventSelection = cms.EDFilter('PATJetSelector',
            src = cms.InputTag('jetsForEventSelection' if minNumJets > 0 else src),
            cut = cms.string('bDiscriminator("{}") > {}'.format(bTagAlgoExpanded, minBDiscr))
        )
        
        process.countBTaggedJets = cms.EDFilter('PATCandViewCountFilter',
            src = cms.InputTag('bTaggedJetsForEventSelection'),
            minNumber = cms.uint32(minBTags), maxNumber = cms.uint32(9999)
        )
        
        paths.append(process.countBTaggedJets)
        
        producers = cms.Task(process.bTaggedJetsForEventSelection)
        paths.associate(producers)


def get_task(process, taskName):
    """Find and return a task with the given name.
    
    If the task does not exist, create it.
    """
    
    if hasattr(process, taskName):
        task = getattr(process, taskName)
        
        if not isinstance(task, cms.Task):
            raise RuntimeError('Attribute "{}" of process "{}" is not a Task.'.format(
                taskName, process.name_()
            ))
    
    else:
        setattr(process, taskName, cms.Task())
        task = getattr(process, taskName)
    
    return task
