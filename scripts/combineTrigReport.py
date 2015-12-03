#! /usr/bin/env python

"""
    The script loops over all files CMSSW_*.stdout in the current directory and combines trigger
    reports in the files into a single report
"""

import os
import sys
import re


class Module:
    """ The class aggregates event counters for a single module
    """
    
    def __init__(self, name):
        # Name of the module
        self.name = name
        
        # Number of events visited this module
        self.visited = 0
        
        # Number of events passed this module
        self.passed = 0
    
    def Update(self, visited, passed):
        self.visited += visited
        self.passed += passed


class Path:
    """ An ordered collection of instances of ModuleCounters
    """
    
    def __init__(self, name):
        # Name of the path
        self.name = name
        
        # Ordered list of modules
        self.modules = []
    
    def UpdateModule(self, index, visited, passed):
        self.modules[index].Update(visited, passed)
    
    def AddModule(self, moduleName, visited, passed):
        self.modules.append(Module(moduleName))
        self.UpdateModule(len(self.modules) - 1, visited, passed)


if __name__ == '__main__':
    # Define regular expressions to parse log files
    pathHeaderRegex = re.compile(r'^TrigReport.*Modules\W+in\W+Path:\W+(\w+)')
    moduleTableRegex = re.compile(r'^TrigReport\W+\d+\W+\d+\W+(\d+)\W+(\d+)\W+\d+\W+\d+\W+(\w+)')
    
    # Collection of paths
    paths = {}
    
    # Loop over files in the current directory
    noLogFilesFound = True
    firstLogFile = True
    
    for f in os.listdir('./'):
        if not os.path.isfile(f):
            continue
        
        if not f.startswith('CMSSW_') or not f.endswith('.stdout'):
            continue
        
        noLogFilesFound = False
        
        
        # Open the log file
        logFile = open(f, 'r')
        reportMissing = False
        
        # Scroll it to the path summary in the beginning of the trigger report
        line = logFile.readline()
        
        while not (line.startswith('TrigReport') and line.find('Path') and line.find('Summary')):
            line = logFile.readline()
            
            if line == '':
                reportMissing = True
                break
        
        if reportMissing:
            print 'No trigger report found in file "' + f + '". It will be skipped.'
            continue
        
        
        # Loop over path summaries
        nPathsFound = 0
        noMorePaths = False
        
        while True:
            line = logFile.readline()
            matchRes = pathHeaderRegex.match(line)
            
            while matchRes is None:
                line = logFile.readline()
                
                if line == '':
                    if nPathsFound == 0:
                        # File is over while no paths have been found so far. Something is wrong
                        print 'File "' + f + '" seems corrupted. Abort.'
                        sys.exit(1)
                    else:
                        # There are no more paths in the log
                        noMorePaths = True
                        break
                
                matchRes = pathHeaderRegex.match(line)
            
            if noMorePaths:
                break
            
            nPathsFound += 1
            
            
            # If it's the first file, need to create the path
            if firstLogFile:
                paths[matchRes.group(1)] = Path(matchRes.group(1))
            
            curPath = paths[matchRes.group(1)]
            
            
            # Skip the table header
            line = logFile.readline()
            
            
            # Read modules
            moduleIndex = 0
            line = logFile.readline()
            matchRes = moduleTableRegex.match(line)
            
            while matchRes is not None:
                moduleName = matchRes.group(3)
                nVisited = int(matchRes.group(1))
                nPassed = int(matchRes.group(2))
                
                if firstLogFile:
                    curPath.AddModule(moduleName, nVisited, nPassed)
                else:
                    curPath.UpdateModule(moduleIndex, nVisited, nPassed)
                
                moduleIndex += 1
                line = logFile.readline()
                matchRes = moduleTableRegex.match(line)
            
            
        
        logFile.close()
        firstLogFile = False
    
    
    # Make sure at least one file with trigger report was found
    if noLogFilesFound:
        print 'Did not find any CMSSW output files.'
        sys.exit(1)
    
    
    # Print results
    for path in paths.values():
        print 'Path "' + path.name + '":\n'
        print 'Visited   Passed     Module'
        
        for module in path.modules:
            print '{0:7d}   {1:7d}    {2}'.format(module.visited, module.passed, module.name)
        
        print '\n\n'
