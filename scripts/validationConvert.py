#!/usr/bin/env python

"""Converts PEC files into JSON format for release validation.

Only a subset of properties are considered.
"""

from __future__ import print_function
import argparse
from collections import OrderedDict
import json
import os.path

import ROOT
ROOT.PyConfig.IgnoreCommandLineOptions = True


if __name__ == '__main__':
    
    argParser = argparse.ArgumentParser(
        epilog=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    argParser.add_argument(
        'inputFile', metavar='pec.root',
        help='Input PEC file.'
    )
    argParser.add_argument(
        '-o', '--output', metavar='output.json', default=None,
        help='Name for the output JSON file. If missing, it is constructed from the name of the '
        'input file.'
    )
    args = argParser.parse_args()
    
    if not args.output:
        args.output = os.path.splitext(os.path.split(args.inputFile)[1])[0] + '.json'
    
    
    ROOT.gROOT.SetBatch(True)
    ROOT.gSystem.Load('libAnalysisPECTuples.so')
    
    
    # Read input file.  All information to be stored in the output file
    # is aggregated in list events.
    events = []
    
    inputFile = ROOT.TFile(args.inputFile)
    tree = inputFile.Get('pecEventID/EventID')
    tree.AddFriend('pecMuons/Muons')
    tree.AddFriend('pecElectrons/Electrons')
    tree.AddFriend('pecJetMET/JetMET')
    
    for entry in tree:
        event = OrderedDict()
        
        eventID = entry.eventId
        event['eventID'] = '{}:{}:{}'.format(
            eventID.RunNumber(), eventID.LumiSectionNumber(), eventID.EventNumber()
        )
        
        
        muons = []
        
        for src in entry.muons:
            conv = OrderedDict()
            conv['pt'] = src.Pt()
            conv['eta'] = src.Eta()
            conv['phi'] = src.Phi()
            conv['relIso'] = src.RelIso()
            conv['passTight'] = src.TestBit(2)
            
            muons.append(conv)
        
        event['muons'] = muons
        
        
        electrons = []
        
        for src in entry.electrons:
            conv = OrderedDict()
            conv['pt'] = src.Pt()
            conv['eta'] = src.Eta()
            conv['etaSC'] = src.EtaSC()
            conv['phi'] = src.Phi()
            conv['relIso'] = src.RelIso()
            conv['passVeto'] = src.BooleanID(0)
            conv['passTight'] = src.BooleanID(3)
            
            electrons.append(conv)
        
        event['electrons'] = electrons
        
        
        jets = []
        
        for src in entry.jets:
            conv = OrderedDict()
            conv['rawPt'] = src.Pt()
            conv['corrPt'] = src.Pt() * src.CorrFactor()
            conv['eta'] = src.Eta()
            conv['phi'] = src.Phi()
            conv['uncJEC'] = src.JECUncertainty()
            conv['uncJER'] = src.JERUncertainty()
            conv['passPFLoose'] = src.TestBit(1)
            conv['hasGenMatch'] = src.TestBit(0)
            conv['CSV'] = src.BTag(0)
            
            jets.append(conv)
        
        event['jets'] = jets
        
        
        mets = []
        
        conv = OrderedDict()
        conv['rawPt'] = entry.uncorrMETs[0].Pt()
        conv['rawPhi'] = entry.uncorrMETs[0].Phi()
        conv['corrPt'] = entry.METs[0].Pt()
        conv['corrPhi'] = entry.METs[0].Phi()
        
        mets.append(conv)
        event['mets'] = mets
        
        events.append(event)
    
    inputFile.Close()
    
    
    outFile = open(args.output, 'w')
    json.dump(events, outFile, indent=2)
    outFile.close()
    
    print(len(events), 'events written')
