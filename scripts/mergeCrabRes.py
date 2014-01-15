#! /bin/env python

"""
@author Andrey Popov

The script merges output ROOT files produced by a CRAB task.

The current directory is seached for *.root files (user can provide a mask to tighten selection of
the files). They are merged into a small number of larger files (called "parts") that match
approximately provided file size. The script exploits the hadd utility from ROOT distribution.
Because the utility does not cope well with a large number of input files (~500 or more), each part
is not merged in one go but splitted into several blocks, each of a sufficiently small number of
source files. Files in blocks are merged first, and then the blocks are merged into parts. The
merging is performed in several threads; their number can be adjusted by user. Produced final files
(as well as temporary ones) are placed in a newly created temporary directory; user can specify a
parent directory in which the temporary one is created.

After source files are merged, the script calculates the total number of events in them. It is done
by counting entries in a specified tree.

The script runs with python 2.7 or newer 2.X and requires pyROOT to be configured properly. In a
clean session of lxplus the installation can be performed with the following example commands:
  export PATH=/afs/cern.ch/sw/lcg/external/Python/2.7.3/x86_64-slc5-gcc47-opt/bin/:$PATH
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/afs/cern.ch/sw/lcg/external/Python/2.7.3/x86_64-slc5-gcc47-opt/lib/
  source /afs/cern.ch/sw/lcg/app/releases/ROOT/5.34.11/x86_64-slc5-gcc46-opt/root/bin/thisroot.sh
"""

import sys
import os
import argparse
import re
import math
import tempfile
from multiprocessing import Pool
from subprocess import call
import shutil
import ROOT


class SourceFileName:
    """
    A class to store the name of a source file. It extracts the job number from the name and stores
    it as a number for an easy access.
    """
    
    # A static regular expression to parse the name of a source file
    nameRegex = re.compile(r'.*_(\d+)_\d+_[a-zA-Z0-9]{3}\.root')
    
    def __init__(self, fileName):
        self.name = fileName
        
        res = SourceFileName.nameRegex.match(fileName)
        if res is None:
            raise RuntimeError('File name "' + fileName + '" does not seem as an output ROOT file '\
                'delivered by CRAB.')
        
        self.jobIndex = int(res.group(1))


class FileNameBlock:
    """
    The class lists all files in a single block within a part. It is an auxiliary data type needed
    to merge results of a CRAB task. In a general case results of individual jobs are merged into
    a small number of files called parts instead of a single large file. Jobs within a single part
    are splitted among several blocks. Jobs in each block are merged in one go; then produced files
    for all blocks within a part are merged.
    """
    
    def __init__(self, partNumber_, blockNumber_):
        """
        Initialised with indices of a part and a block within the part. Both indices start from
        zero.
        """
        self.partNumber = partNumber_
        self.blockNumber = blockNumber_
        self.fileNames = []
    
    def AddFile(self, fileName):
        """
        Adds a new file name to the list
        """
        self.fileNames.append(fileName)


def mergeFiles(outputFileName, sourceFiles):
    """
    Merges provided ROOT files. The function calls hadd program from ROOT distribution.
    """
    # Make sure there is more than one source file
    if len(sourceFiles) > 1:
        devnull = open('/dev/null', 'w')
        res = call(['hadd', '-v0', outputFileName] + sourceFiles,
            stdout = devnull, stderr = devnull)
        
        if res != 0:
            raise RuntimeError('Call to hadd terminated with an error.')
    else:
        shutil.copyfile(sourceFiles[0], outputFileName)


if __name__ == '__main__':
    # Define supported arguments and options
    optionParser = argparse.ArgumentParser(description = __doc__)
    optionParser.add_argument('-m', '--mask',
        help = 'POSIX regular expression to specify input files', default = '.*\.root')
    optionParser.add_argument('-s', '--max-size', help = 'Maximal size of an output file (GiB). '\
        'The output is splitted into several files if needed', type = float, default = 2.,
        dest = 'max_size')
    optionParser.add_argument('-o', '--out-dir', help = 'Directory to store output. If it does '\
        'not exist, it is created. All output is placed in a temporary directory created within '\
        'the specified one', default = '/tmp/', dest = 'out_dir')
    optionParser.add_argument('-n', '--num-threads', help = 'Number of threads to be used',
        default = 6, type = int, dest = 'num_threads')
    optionParser.add_argument('-t', '--tree-name', help = 'Name of a tree to count events',
        default = 'eventContent/BasicInfo', dest = 'tree_name')
    optionParser.add_argument('-d', '--keep-tmp-files', help = 'Do not delete temporary files',
        action = 'store_true', dest = 'keep_tmp_files')

    # Parse the arguments and options
    args = optionParser.parse_args()

    # The current directory only will be searched for the input files. For this reason, the mask
    # must not contain a slash
    if args.mask.find('/') != -1:
        print 'Error. The mask to choose input files must not contain a slash.'
        sys.exit(1)


    # Identify source files in the current directory. Save their names and calculate their total
    # size
    maskRegex = re.compile(args.mask)
    sourceFiles = []
    totalSize = 0  # in bytes

    for fileName in os.listdir('./'):
        if maskRegex.match(fileName) is None:
            continue
        
        sourceFiles.append(SourceFileName(fileName))
        totalSize += os.stat(fileName).st_size

    # Sort the list of source files' names
    sourceFiles.sort(key = lambda fileName: fileName.jobIndex)


    # Find out the number of parts into which the output should be splitted
    nParts = int(totalSize / float(args.max_size * 1024**3))

    if nParts == 0:
        nParts = 1

    nFilesPerPart = int(math.ceil(len(sourceFiles) / float(nParts)))


    # Specify explicitly what files are assigned to what part. Files for one part will not be merged
    # all in one go; instead, they will be splitted into blocks of size maxFilesToMerge (defined
    # below), and each block will be merged independently. Files are separated by part and by block
    maxFilesToMerge = 256
    blocks = []

    iPart = -1
    iBlock = -1

    for iJob in range(len(sourceFiles)):
        # Check if a new part is started
        if iJob % nFilesPerPart == 0:
            iPart += 1
            iBlock = -1
        
        # Check if a new block is started
        if (iJob % nFilesPerPart) % maxFilesToMerge == 0:
        #^ If the part number has just increased, this condition is also true
            iBlock += 1
            
            blocks.append(FileNameBlock(iPart, iBlock))
        
        # Add file name to the current block
        blocks[-1].AddFile(sourceFiles[iJob].name)
    
    
    # Prepare to merge files within the blocks. First create a temporary output directory
    if not os.path.exists(args.out_dir):
        os.makedirs(args.out_dir)
    outputDir = tempfile.mkdtemp(dir = args.out_dir)
    outputDir += '/'
    
    print 'Starting merging the files. Results will be placed in directory', outputDir + '.'
    

    # Deduce the base name of output ROOT files: everything before the job number. It is used to name
    # output files
    res = re.match(r'(.*)_\d+_\d+_[a-zA-Z0-9]{3}\.root', sourceFiles[0].name)
    basename = res.group(1)
    
    
    # Create a thread pool to merge files within the blocks and submit jobs to it
    pool = Pool(processes = args.num_threads)
    
    for block in blocks:
        outputName = outputDir + basename + '_p' + str(block.partNumber + 1) + \
            '_' + str(block.blockNumber + 1) + '.root'
        pool.apply_async(mergeFiles, (outputName, block.fileNames))
    
    # Wait until the jobs are done
    pool.close()
    pool.join()
    
    
    # Now merge the blocks to produce final files ("parts"). In order to do it, it is convenient to
    # know which block are available for each part
    partToBlock = dict()
    
    for block in blocks:
        if block.partNumber not in partToBlock:
            partToBlock[block.partNumber] = []
        
        partToBlock[block.partNumber].append(block.blockNumber)
    
    for partNumber in partToBlock.iterkeys():
        partToBlock[partNumber].sort()
    
    
    # Perform the actual merging
    pool = Pool(processes = args.num_threads)
    
    for partNumber in partToBlock.iterkeys():
        nameFragment = outputDir + basename
        if nParts > 1:
            nameFragment += '_p' + str(partNumber + 1)
        sourceFiles = []
        
        for blockNumber in partToBlock[partNumber]:
            sourceFiles.append(nameFragment + '_' + str(blockNumber + 1) + '.root')
        
        pool.apply_async(mergeFiles, (nameFragment + '.root', sourceFiles))
    
    # Wait for the jobs to finish
    pool.close()
    pool.join()
    
    
    # Delete temporary files and make a list of final files
    finalFileNames = []
    
    for partNumber in partToBlock.iterkeys():
        nameFragment = outputDir + basename
        if nParts > 1:
            nameFragment += '_p' + str(partNumber + 1)
        
        finalFileNames.append(nameFragment + '.root')
        
        if not args.keep_tmp_files:
            for blockNumber in partToBlock[partNumber]:
                os.remove(nameFragment + '_' + str(blockNumber + 1) + '.root')
    
    
    # Print out names of the final files
    print 'Results of all jobs have been merged into the following files:'
    
    for fileName in finalFileNames:
        print '', fileName
    
    
    # Count events in the produced files
    nTotalEvents = 0
    
    for fileName in finalFileNames:
        finalFile = ROOT.TFile(fileName)
        tree = finalFile.Get(args.tree_name)
        nTotalEvents += tree.GetEntries()
        finalFile.Close()
    
    print 'Total number of events in these files:', nTotalEvents
















