#!/usr/bin/env python

"""Compares validation JSON files produced by validationConvert.py."""

from __future__ import print_function
import argparse
from collections import OrderedDict
import copy
import json
import math


def compare_properties(dict1, dict2):
    """Compare properties of two objects.
    
    Only consider properties that exist for both objects.
    """
    
    mismatchKeys = set()
    
    for key in dict1.keys():
        if key not in dict2.keys():
            continue
        
        value1 = dict1[key]
        value2 = dict2[key]
        
        if isinstance(value1, float) and isinstance(value2, float):
            # Perform the comparison using a precision a bit worse than
            # in float32.  Implement here math.isclose from newer
            # versions of Python.
            relTol = 1e-7
            absTol = 1e-7
            
            if abs(value1 - value2) > max(relTol * max(abs(value1), abs(value2)), absTol):
                mismatchKeys.add(key)
        else:
            # Perform exact comparison for types other than
            # floating-point numbers
            if value1 != value2:
                mismatchKeys.add(key)
    
    return mismatchKeys


def print_mismatch(dict1, dict2, mismatchKeys, indent=4):
    """Print out two objects highlighing differences."""
    
    # First loop over keys in the first dict
    for key in dict1.keys():
        value1 = dict1[key]
        
        if key in dict2.keys():
            value2 = dict2[key]
        else:
            value2 = '--'
        
        text = '{}{}:  {}  {}'.format(' ' * indent, key, value1, value2)
        
        if key in mismatchKeys:
            print('\033[1;31m{}\033[0m'.format(text))
        else:
            print(text)
    
    # Then print properties from the second dict that are not found in
    # the first one
    for key in dict2.keys():
        if key in dict1.keys():
            continue
        
        print('{}{}:  --  {}'.format(' ' * indent, key, dict2[key]))


if __name__ == '__main__':
    
    argParser = argparse.ArgumentParser(
        epilog=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    argParser.add_argument(
        'file1', metavar='file1.json',
        help='First input file.'
    )
    argParser.add_argument(
        'file2', metavar='file2.json',
        help='Second input file.'
    )
    args = argParser.parse_args()
    
    
    # Read input files converting data from lists into maps with
    # event IDs as keys
    events1 = {}
    events2 = {}
    
    for fileName, events in [(args.file1, events1), (args.file2, events2)]:
        with open(fileName) as f:
            data = json.load(f, object_pairs_hook=OrderedDict)
        
        for entry in data:
            eventID = tuple(int(token) for token in entry['eventID'].split(':'))
            
            entryStripped = copy.copy(entry)
            del entryStripped['eventID']
            
            events[eventID] = entryStripped
        
        del data
    
    
    # Compare selected events
    eventIDs1 = set(events1.keys())
    eventIDs2 = set(events2.keys())
    
    if eventIDs1 == eventIDs2:
        eventIDs = eventIDs1
        print('\033[1;32mEvent lists match\033[0m')
    else:
        print('\033[1;31mEvent lists do not match\033[0m')
        
        diff = eventIDs1 - eventIDs2
        print('  Events in the first file but not in second:', end='')
        
        if diff:
            print(['{}:{}:{}'.format(*d) for d in diff])
        else:
            print('(none)')
        
        diff = eventIDs2 - eventIDs1
        print('  Events in the second file but not in first:', end='')
        
        if diff:
            print(['{}:{}:{}'.format(*d) for d in diff])
        else:
            print('(none)')
        
        eventIDs = eventIDs1 & eventIDs2
        print('  Will only consider the overlap of the two lists, which contains {} events'.format(
            len(eventIDs)
        ))
        print()
    
    
    # Loop over top-level groups of properties
    topGroups = list(events1.itervalues().next().keys())
    
    for topGroup in topGroups:
        
        # Perform a comparison for the current group in all events
        mismatchIntroPrinted = False
        
        for eventID in eventIDs:
            group1 = events1[eventID][topGroup]
            group2 = events2[eventID][topGroup]
            
            # Compare the two groups of properties.  The algorithm
            # depends on whether they are lists or dictionaries.
            if not isinstance(group1, list) and not isinstance(group2, list):
                
                mismatchKeys = compare_properties(group1, group2)
                
                if mismatchKeys:
                    if not mismatchIntroPrinted:
                        print(
                            '\033[1;31mDisagreement found in group "{}"\033[0m'.format(topGroup)
                        )
                        mismatchIntroPrinted = True
                    
                    print('  \033[1;1mEvent {}:{}:{}\033[0m'.format(*eventID))
                    print_mismatch(group1, group2, mismatchKeys, indent=4)
                    print()
            
            elif isinstance(group1, list) and isinstance(group2, list):
                
                # Build matching between elements of the two lists
                indexMatch = OrderedDict()
                mismatchedElements1, mismatchedElements2 = set(), set()
                
                if len(group1) > 0 and len(group2) > 0 and \
                    'eta' in group1[0].keys() and 'phi' in group2[0].keys() and \
                    'eta' in group2[0].keys() and 'phi' in group2[0].keys():
                    
                    # Can build a matching based on (eta, phi)
                    matchedElements2 = set()
                    
                    for i1 in range(len(group1)):
                        matchFound = False
                        
                        for i2 in range(len(group2)):
                            if i2 in matchedElements2:
                                continue
                            
                            dEta = abs(group1[i1]['eta'] - group2[i2]['eta'])
                            dPhi = abs(group1[i1]['phi'] - group2[i2]['phi'])
                            
                            if dPhi > math.pi:
                                dPhi = 2 * math.pi - dPhi
                            
                            if dEta**2 + dPhi**2 < 1e-4:  # dR < 0.01
                                matchFound = True
                                indexMatch[i1] = i2
                                matchedElements2.add(i2)
                                break
                        
                        if not matchFound:
                            mismatchedElements1.add(i1)
                    
                    # Looped over all elements of the first list, but
                    # there might be some unmatched elements in the
                    # second list.  Identify them.
                    mismatchedElements2 = set(range(len(group2))) - matchedElements2
                
                else:
                    # Objects stored in the lists do not have properties
                    # 'eta' and 'phi'.  Simply match objects with the
                    # same indices.
                    commonLen = min(len(group1), len(group2))
                    
                    for i in range(commonLen):
                        indexMatch[i] = i
                    
                    for i1 in range(commonLen, len(group1)):
                        mismatchedElements1.add(i1)
                    
                    for i2 in range(commonLen, len(group2)):
                        mismatchedElements2.add(i2)
                
                # Report mismatched elements
                if mismatchedElements1 or mismatchedElements2:
                    if not mismatchIntroPrinted:
                        print(
                            '\033[1;31mDisagreement found in group "{}"\033[0m'.format(topGroup)
                        )
                        mismatchIntroPrinted = True
                    
                    print('  \033[1;1mEvent {}:{}:{}\033[0m'.format(*eventID))
                    
                    for mismatchedElements, fileLabel in [
                        (mismatchedElements1, 'first'), (mismatchedElements2, 'second')
                    ]:
                        if mismatchedElements:
                            print(
                                '    Failed to match elements with the following indices in the '
                                '{} file:'.format(fileLabel),
                                list(mismatchedElements)
                            )
                    
                    print()
                
                
                # Compare matched elements
                eventIDPrinted = False
                
                for i1, i2 in indexMatch.items():
                    mismatchKeys = compare_properties(group1[i1], group2[i2])
                    
                    if mismatchKeys:
                        if not mismatchIntroPrinted:
                            print(
                                '\033[1;31mDisagreement found in group "{}"\033[0m'.format(topGroup)
                            )
                            mismatchIntroPrinted = True
                        
                        if not eventIDPrinted:
                            print('  \033[1;1mEvent {}:{}:{}\033[0m'.format(*eventID))
                            eventIDPrinted = True
                        
                        print('  \033[1;1mElements {}:{}\033[0m'.format(i1, i2))
                        print_mismatch(group1[i1], group2[i2], mismatchKeys, indent=4)
                        print()
            
            else:
                # Wrong format
                raise RuntimeError('Different formats for group "{}" in event {}:{}:{}'.format(
                    topGroup, *eventID
                ))
        
        if not mismatchIntroPrinted:
            # No disagreements found for the current group
            print('\033[1;32mData in group "{}" agree\033[0m'.format(topGroup))
