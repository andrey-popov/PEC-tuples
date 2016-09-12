#!/usr/bin/env python

"""Script to merge output ROOT files produced by a CRAB task.

This script searches the current directory for files whose names match a
user-defined mask (".*\.root" by default), and the files are then merged
into several larger files.  The grouping is done such that size of each
produced file is close to a value specified by the user (2 GiB by
default).

Merging of ROOT files does not always work well for a large number of
input files (hundreds), and for this reason the script does it in a
recursive manner, limiting the total number of files that are merged in
a single act.

The result of merging is validated by counting events in a given tree in
the output files and comparing it to the total number of events in input
files.
"""

import argparse
import math
from multiprocessing import Pool
from operator import attrgetter
import os
import re
import shutil
from subprocess import call
import sys
import tempfile

import ROOT


class InputFile(object):
    """Stores name of a source file together with size and job index.
    
    The job index is extracted from the file name.
    """
    
    # A static regular expression to parse the name of a source file
    nameRegex = re.compile(r'^.*_(\d+)\.root$')
    
    def __init__(self, fileName, fileSize):
        self.name = fileName
        
        res = InputFile.nameRegex.match(fileName)
        if res is None:
            raise RuntimeError(
                'Failed to extract job index from CRAB output file name "{}".'.format(fileName)
            )
        
        self.jobIndex = int(res.group(1))
        self.size = fileSize


class Partitioner(object):
    """Class to split a list of files into parts by size and length.
    
    Input files are grouped into "parts" such that the total size of
    each part is close to the given target value.  Files assigned to
    each part are further split into blocks such that number of files
    in each block does not exceed the given limit.  Ordering of input
    files is preserved.
    """
    
    def __init__(self, targetPartSize, maxBlockLength):
        """Create new object giving target part size and block length.
        
        The part size is given in bytes.
        """
        
        self.targetPartSize = targetPartSize
        self.maxBlockLength = maxBlockLength
        
        self.parts = [[[]]]
        self._curPart = self.parts[0]
        self._curPartSize = 0
        self._curBlock = self._curPart[0]
        self._curBlockLength = 0
    
    
    def _add_cur_part(self, inputFile):
        """Add a file to the current part.
        
        A new block is created in the current part if needed.
        """
        
        if self._curBlockLength == self.maxBlockLength:
            # Create a new block within the current part
            self._curPart.append([])
            self._curBlock = self._curPart[-1]
            self._curBlockLength = 0
        
        self._curBlock.append(inputFile)
        self._curBlockLength += 1
        self._curPartSize += inputFile.size
    
    
    def _close_part(self):
        """Close current part and add a new one."""
        
        self.parts.append([[]])
        self._curPart = self.parts[-1]
        self._curPartSize = 0
        self._curBlock = self._curPart[0]
        self._curBlockLength = 0
    
    
    def add(self, inputFile):
        """Add new input file.
        
        New blocks and parts are created as necessary.
        """
        
        newPartSize = self._curPartSize + inputFile.size
        if newPartSize > self.targetPartSize:
            # Adding this file would overflow the current part, so a new
            # part will be created.  Depending on what would give the
            # total size closer to the target, add the file to current
            # part or the new one
            if newPartSize - self.targetPartSize > self.targetPartSize - self._curPartSize:
                self._close_part()
                self._add_cur_part(inputFile)
            else:
                self._add_cur_part(inputFile)
                self._close_part()
        
        else:
            # No need to start a new part
            self._add_cur_part(inputFile)
    
    
    def get_partitioning(self):
        """Return created partitioning."""
        
        # It is possible to have an empty part at the end.  Remove it
        # if present.
        if len(self.parts[-1]) == 1 and len(self.parts[-1][0]) == 0:
            del(self.parts[-1])
            self._curPart = None
            self._curBlock = None
        
        return self.parts


def count_events(inputFiles, treeName):
    """Count events in the given tree in all input files."""
    
    counter = 0
    
    for inputFile in inputFiles:
        f = ROOT.TFile(inputFile)
        tree = f.Get(treeName)
        
        if not tree:
            raise RuntimeError(
                'File "{}" does not contain requested tree "{}".'.format(inputFile, treeName)
            )
        
        counter += tree.GetEntries()
        f.Close()
    
    return counter
        

def critical_error(formatString, *args, **kwargs):
    """Report a critical error.
    
    Print an error message and exit the script with code 1.  The error
    message is formed from the given format string and subsequent
    arguments using method str.format.
    """
    
    print 'Error:', formatString.format(*args, **kwargs)
    sys.exit(1)


def merge_files(outputFileName, inputFiles):
    """Merge provided ROOT files.
    
    Perform the task by calling the hadd program from ROOT distribution.
    Alternatively, could have used TFileMerger class, but it is not
    clear if it can run in a multithread environment.
    """
    
    # Make sure there is more than one source file
    if len(inputFiles) > 1:
        devnull = open('/dev/null', 'w')
        res = call(['hadd', '-v0', outputFileName] + inputFiles,
            stdout=devnull, stderr=devnull)
        devnull.close()
        
        if res != 0:
            raise RuntimeError('Call to hadd terminated with an error.')
    else:
        shutil.copyfile(inputFiles[0], outputFileName)



if __name__ == '__main__':
    
    # Define supported arguments and options
    argParser = argparse.ArgumentParser(
        epilog=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    argParser.add_argument(
        '-m', '--mask',
        help='POSIX regular expression to specify input files', default='.*\.root'
    )
    argParser.add_argument(
        '-s', '--size',
        help='Target size of an output file (GiB). The output is splitted into several files '
            'if needed',
        type=float, default=2., dest='target_size'
    )
    argParser.add_argument(
        '-o', '--out-dir',
        help='Directory to store output. If it does not exist, it is created',
        default='./', dest='out_dir'
    )
    argParser.add_argument(
        '-n', '--num-threads', help='Number of threads to be used',
        type=int, default=5, dest='num_threads'
    )
    argParser.add_argument(
        '--max-files-to-merge', help='Maximal number of files to be merged in a single act',
        type=int, default=256, dest='max_files_to_merge'
    )
    argParser.add_argument(
        '-t', '--tree-name', help='Name of a tree to count events',
        default='pecEventID/EventID', dest='tree_name'
    )
    argParser.add_argument(
        '-k', '--keep-tmp-files', help='Do not delete temporary files',
        action='store_true', dest='keep_tmp_files'
    )
    
    # Parse the arguments and options
    args = argParser.parse_args()
    
    # The current directory only will be searched for the input files.
    # For this reason, the mask must not contain a slash.
    if args.mask.find('/') != -1:
        critical_error(
            'Mask to choose input files ("{}") can only refer to the current directory.',
            args.mask
        )
    
    
    # Identify input files in the current directory.  Order them by job
    # index.
    maskRegex = re.compile(args.mask)
    inputFiles = []
    
    for fileName in os.listdir('./'):
        if maskRegex.match(fileName) is None:
            continue
        
        fileSize = os.stat(fileName).st_size  # in bytes
        inputFiles.append(InputFile(fileName, fileSize))
    
    inputFiles.sort(key=attrgetter('jobIndex'))
    
    
    # Split the list of input files into parts such that the total size
    # of each part is close to the given target.  Files assigned to each
    # part are further split into blocks with the given maximal size (in
    # terms of the number of files), which will be merged independently.
    partitioner = Partitioner(args.target_size * 1024**3, args.max_files_to_merge)
    for inputFile in inputFiles:
        partitioner.add(inputFile)
    
    parts = partitioner.get_partitioning()
    
    
    # Prepare to merge files in the blocks.  First create a temporary
    # output directory.
    outputDir = args.out_dir
    if not os.path.exists(outputDir):
        os.makedirs(outputDir)
    tmpDir = tempfile.mkdtemp(dir=outputDir)
    tmpDir += '/'
        
    # Deduce the base name of output ROOT files: everything before the
    # job number.  It is used to name output files.
    res = re.match(r'(.*)_\d+\.root', inputFiles[0].name)
    baseOutputName = res.group(1)
    
    
    # Merge files in the blocks
    pool = Pool(processes=args.num_threads)
    
    for iPart, part in enumerate(parts):
        for iBlock, block in enumerate(part):
            outputName = tmpDir + '{baseName}_part{partNumber:d}_{blockNumber:d}.root'.format(
                baseName=baseOutputName, partNumber=iPart + 1, blockNumber=iBlock + 1
            )
            pool.apply_async(merge_files, (outputName, [inputFile.name for inputFile in block]))
    
    pool.close()
    pool.join()
    
    
    # Now merge the blocks to produce final files
    partFileShortNames = []
    pool = Pool(processes=args.num_threads)
    
    for iPart, part in enumerate(parts):
        if len(parts) > 1:
            outputShortName = '{baseName}.part{partNumber:d}.root'.format(
                baseName=baseOutputName, partNumber=iPart + 1
            )
        else:
            outputShortName = baseOutputName + '.root'
        
        partFileShortNames.append(outputShortName)
        outputName = tmpDir + outputShortName
        
        blockFiles = []
        for iBlock in range(len(part)):
            blockFiles.append(
                tmpDir + '{baseName}_part{partNumber:d}_{blockNumber:d}.root'.format(
                    baseName=baseOutputName, partNumber=iPart + 1, blockNumber=iBlock + 1
                )
            )
        
        pool.apply_async(merge_files, (outputName, blockFiles))
    
    pool.close()
    pool.join()
    
    
    # Move merged files to the output directory and delete the temporary
    # directory with all temporary files
    for shortName in partFileShortNames:
        if os.path.exists(outputDir + shortName):
            critical_error(
                'Cannot create output file "{}" because a file with such name already exists.',
                outputDir + shortName
            )
        shutil.move(tmpDir + shortName, outputDir + shortName)
    
    if not args.keep_tmp_files:
        shutil.rmtree(tmpDir)
    
    
    # Print out names of the final files
    print 'Results of {} jobs have been merged into the following files:'.format(len(inputFiles))
    outputFiles = [outputDir + shortName for shortName in partFileShortNames]
    for fileName in outputFiles:
        print '', fileName
    
    
    # Count events in input and processed files
    nEventInput = count_events([inputFile.name for inputFile in inputFiles], args.tree_name)
    nEventMerged = count_events(outputFiles, args.tree_name)
    

    if nEventInput != nEventMerged:
        critical_error(
            'Total number of events in input and merged files do not agree ({} vs {}).',
            nEventInput, nEventMerged
        )
    else:
        print 'Total number of events in these files:', nEventMerged
