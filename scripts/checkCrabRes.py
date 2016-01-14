#! /usr/bin/env python

"""Script to verify results of a CRAB job.

This script is for CRAB 2 and therefore is outdated.  It needs to be
updated significantly to work with CRAB 3, and it is not even clear if
there is a strong need for such verification with the new version of
CRAB.

The code also needs to be updated to comply with Python coding
standards.
"""

##
# This scripts checks the CRAB results for consistency. It assertains that all the necessary files
# are present, checks the exit codes, looks for excess ROOT output files (left from previous
# submissions). It also calculates the total number of read events and events that passed the
# whole selection. The user should provide path to the res/ directory of the task as the only
# parameter to the script (the path defaults to current directory).
# 
# Always check that the total number of events read is the same as reported by `crab -report`. In
# case of MC presence of a difference is not critical (just normalise to the total number of events
# reported by this utility), but in case of real data in might lead to serious problems. CRAB
# calculates the number of events read and prepares luminosity JSON files based on FJR reports for
# those jobs only which are considered successful. It might happen that CRAB marks a job as failed
# (claiming it also in `crab -status`) whereas its log files are perfectly OK. Files for such a job
# must be deleted manually because it is not accounted for in the luminosity JSON files.
# 
# Keep in mind that hadd utility works unreliably and sometimes misses huge fraction of events
# silently. For this nasty behaviour always verify that the total number of events in a merged ROOT
# file is the same as reported by this utility.

import sys
import os
import re
import optparse


class JobInfo:
    
    
    def __init__(self, index):
        self.jobIndex = index
        self.rootOutputName = ""
        self.isStdoutPresent = False
        self.isStderrPresent = False
        self.isCrabFjrPresent = False
        self.isStderrEmpty = True
        self.rootFiles = []
        self.nRead = 0
        self.nPassed = 0
        self.exeExitCode = -1
        self.jobExitCode = -1
        self.fjrExitCode = -1
        self.isStdoutCorrupted = True
        self.isCrabFjrCorrupted = True
    
    def __str__(self):
        description = ""
        description += "job #" + str(self.jobIndex) + "\n"
        
        if self.isStdoutPresent:
            description += "CMSSW stdout: present"
            if self.isStdoutCorrupted:
                description += " (CORRUPTED)\n"
            else:
                description += " (structure Ok)\n"
                description += str(self.nRead) + " events read, " + str(self.nPassed) + " passed\n"
        else:
            description += "CMSSW stdout: MISSING\n"
        
        if self.isStderrPresent:
            description += "CMSSW stderr: present"
            if self.isStderrEmpty:
                description += ", empty\n"
            else:
                description += ", NOT EMPTY"
        else:
            description += "CMSSW stderr: MISSING\n"
        
        if self.isCrabFjrPresent:
            description += "CRAB FJR: present"
            if self.isCrabFjrCorrupted:
                description += ", (CORRUPTED)\n"
            else:
                description += " (structure Ok)\n"
        else:
            description += "CRAB FJR: MISSING\n"
        
        description += "Exit codes: " + str(self.exeExitCode) + " (exe), " + \
         str(self.jobExitCode) + " (job), " + str(self.fjrExitCode) + " (fjr)\n"
        
        description += "Output file: " + self.rootOutputName + "\n"
        description += "All output files for the job: " + self.rootFiles.__str__()
        
        return description
        


class FileCorrupted(Exception):
    pass


parser = optparse.OptionParser(
    description = "The script checks the output files for presence and the exit codes, and marks "\
                  "the missing jobs or jobs with errors")
parser.add_option("-r", "--no-root", help = "do not check for ROOT files", action = "store_true",
                  dest = "noRootFiles", default = False)
parser.add_option('-e', '--ignore-stderr', help = 'do not check *.stderr files',
 action = 'store_true', dest = 'ignoreStdErr', default = True)
(options, args) = parser.parse_args()

if len(args) > 0:
    d = args[0]  # directory to read
else:
    d = "."

if d[-1] != '/':
    d += '/'


# Read the lists of CMSSW stdout and stderr files and CRAB FJR (only jobs' indices are kept instead
#of the whole file names). Note they starts from 1
stdoutIndices = []
stderrIndices = []
crabFjrIndices = []

crabStatusRegex = re.compile(r"<ExitCode Value=\"(\d+)\"/>")

for f in os.listdir(d):
    if not os.path.isfile(f):
        continue
    
    if f.startswith("CMSSW_"):
        if f.endswith(".stdout"):
            stdoutIndices.append(int(f[6:-7]))
        elif f.endswith(".stderr"):
            stderrIndices.append(int(f[6:-7]))
    
    if f.startswith("crab_fjr_") and f.endswith(".xml"):
        crabFjrIndices.append(int(f[9:-4]))

# Make sure the lists are not empty
if len(stdoutIndices) == 0:
    print 'No *.stdout file has been found.'
    sys.exit(0)

if len(stderrIndices) == 0:
    print 'No *.stderr file has been found.'
    sys.exit(0)

if len(crabFjrIndices) == 0:
    print 'No CRAB FJR file has been found.'
    sys.exit(0)

stdoutIndices.sort()
stderrIndices.sort()
crabFjrIndices.sort()


# List of jobs info
jobs = [JobInfo(i + 1) for i in range(max(stdoutIndices[-1], stderrIndices[-1], crabFjrIndices[-1]))]

curStdoutIndex = 0
curStderrIndex = 0
curCrabFjrIndex = 0
rootFilePattern = "" # common starting of all the ROOT files

for i in range(len(jobs)):
    if stdoutIndices[curStdoutIndex] == jobs[i].jobIndex:
    # no missing indices since the previous iteration
        curStdoutIndex += 1
        jobs[i].isStdoutPresent = True

        
        try:
            fp = open(d + "CMSSW_" + str(jobs[i].jobIndex) + ".stdout", "r")
            line = fp.readline()
            
            while line != "TrigReport ---------- Event  Summary ------------\n":
                line = fp.readline()
                if line == "":
                    raise FileCorrupted()
            
            words = fp.readline().split()
            jobs[i].nRead = int(words[4])
            jobs[i].nPassed = int(words[7])
            
            line = fp.readline()
            
            while not (line.startswith("tarring file ") or line.startswith("will tar file ")):
                line = fp.readline()
                if line == "":
                    print 'File is corrupted'
                    raise FileCorrupted()
            
            if line.startswith("tarring file "):  # old style of the output
                jobs[i].rootOutputName = line.split()[2]
            else:
                jobs[i].rootOutputName = line.split()[3]
            
            if rootFilePattern == "":
                pos = jobs[i].rootOutputName.rfind("_")
                pos = jobs[i].rootOutputName.rfind("_", 0, pos)
                pos = jobs[i].rootOutputName.rfind("_", 0, pos)
                rootFilePattern = jobs[i].rootOutputName[0:pos + 1]
            
            while not line.startswith("ExeExitCode="):
                line = fp.readline()
                if line == "":
                    raise FileCorrupted()
            
            jobs[i].exeExitCode = int(line[12:])
            
            while not (line.startswith("JOB_EXIT_STATUS = ") or line.startswith('JobExitCode=')):
                line = fp.readline()
                if line == "":
                    raise FileCorrupted()
            
            if line.startswith('JOB_EXIT_STATUS = '):  # old style
                jobs[i].jobExitCode = int(line[len('JOB_EXIT_STATUS = '):])
            else:
                jobs[i].jobExitCode = int(line[len('JobExitCode='):])
            
            fp.close()
            
        except FileCorrupted:
            jobs[i].isStdoutCorrupted = True
        else:
            jobs[i].isStdoutCorrupted = False
        
    else:
    # No CMSSW stdout for a given index
        jobs[i].isStdoutPresent = False
    
    
    if stderrIndices[curStderrIndex] == jobs[i].jobIndex:
        curStderrIndex += 1
        jobs[i].isStderrPresent = True
        
        if os.path.getsize(d + "CMSSW_" + str(jobs[i].jobIndex) + ".stderr") > 0:
            jobs[i].isStderrEmpty = False
        else:
            jobs[i].isStderrEmpty = True
        
    else:
        jobs[i].isStderrPresent = False
    
    
    if crabFjrIndices[curCrabFjrIndex] == jobs[i].jobIndex:
        curCrabFjrIndex += 1
        jobs[i].isCrabFjrPresent = True
        
        try:
            fp = open(d + "crab_fjr_" + str(jobs[i].jobIndex) + ".xml")
            line = fp.readline()
            match = None
            
            while match is None:
                line = fp.readline()
                match = crabStatusRegex.search(line)
                if line == "":
                    raise FileCorrupted()
            
            jobs[i].fjrExitCode = int(match.group(1))
            fp.close()
        
        except FileCorrupted:
            jobs[i].isCrabFjrCorrupted = True
        else:
            jobs[i].isCrabFjrCorrupted = False
        
    else:
        jobs[i].isCrabFjrPresent = False


if rootFilePattern == "" and not options.noRootFiles:
    print "No ROOT output file names found. Probably the task was run with copy_data = 1. Use "\
          "option -r (--no-root) to suppress check for ROOT files"
    exit()

# Now the job info objects are almost ready. Look for the ROOT files in the directory
if not options.noRootFiles:
    for f in os.listdir(d):
        if f.startswith(rootFilePattern):
            pos = f.rfind("_")
            pos = f.rfind("_", 0, pos)
            
            if pos <= len(rootFilePattern):
                print 'Warining: ROOT file "' + f + '" has unexpected name pattern and is skipped.'
            else:
                jobIndex = int(f[len(rootFilePattern):pos])
                jobs[jobIndex - 1].rootFiles.append(f)



# Analyze the job info and print the errors (if any)
jobsWithError = 0
jobsWithWarning = 0
jobsOk = 0
nReadTotal = 0
nPassedTotal = 0
rootFilesToDelete = []
jobsWithErrorList = []

for j in jobs:
    errorMessage = ""
    warningMessage = ""
    
    if not j.isStdoutPresent:
        errorMessage += "CMSSW stdout is missing, "
    if not j.isStderrPresent:
        errorMessage += "CMSSW stderr is missing, "
    if not j.isCrabFjrPresent:
        errorMessage += "CRAB FJR is missing, "
    if j.isStdoutPresent and j.isStdoutCorrupted:
        errorMessage += "CMSSW stdout is corrupted, "
    if j.isCrabFjrPresent and j.isCrabFjrCorrupted:
        errorMessage += "CRAB FJR is corrupted, "
    if j.isStdoutPresent and not j.isStdoutCorrupted and j.exeExitCode != 0:
        errorMessage += "exeExitCode " + str(j.exeExitCode) + ", "
    if j.isStdoutPresent and not j.isStdoutCorrupted and j.jobExitCode != 0:
        errorMessage += "jobExitCode " + str(j.jobExitCode) + ", "
    if j.isCrabFjrPresent and not j.isCrabFjrCorrupted and j.fjrExitCode != 0 and \
       j.fjrExitCode != 50117:
        if j.fjrExitCode == 50117:
            #warningMessage += "fjrExitCode 50117, "
            pass  # for some reason all the jobs always have 50117
        else:
            errorMessage += "fjrExitCode " + str(j.fjrExitCode) + ", "
    if j.isStderrPresent and not options.ignoreStdErr and not j.isStderrEmpty:
        errorMessage += "CMSSW stderr is not empty, "
    
    if not options.noRootFiles and j.isStdoutPresent and not j.isStdoutCorrupted:
        excessFiles = []
        rootOutputIsPresent = False
        
        for r in j.rootFiles:
            if r == j.rootOutputName:
                rootOutputIsPresent = True
            else:
                excessFiles.append(r)
        
        if not rootOutputIsPresent:
            errorMessage += "output ROOT file is missing, "
        
        if len(excessFiles) > 0:
            warningMessage += "excess ROOT files are found (see below), "
            rootFilesToDelete.extend(excessFiles)
    
    
    if errorMessage != "":
        jobsWithError += 1
        jobsWithErrorList.append(j.jobIndex)
        print "Error(s) in job " + str(j.jobIndex) + ": " + errorMessage[:-2]
    else:
        nReadTotal += j.nRead
        nPassedTotal += j.nPassed
    
    if warningMessage != "":
        if errorMessage == "":
            jobsWithWarning += 1
        print "Warning(s) in job " + str(j.jobIndex) + ": " + warningMessage[:-2]
    
    if errorMessage == "" and warningMessage == "":
        jobsOk += 1

# Compactify list of the jobs with errors (replace the consecutive jobs with ranges)
jobsWithErrorList.sort()
jobsWithErrorText = ""
veto = False

for i in range(len(jobsWithErrorList) - 1):
    if not veto:
        jobsWithErrorText += str(jobsWithErrorList[i])
        if jobsWithErrorList[i+1] - jobsWithErrorList[i] == 1:
            jobsWithErrorText += '-'
            veto = True
        else:
            jobsWithErrorText += ','
    else:
        if jobsWithErrorList[i+1] - jobsWithErrorList[i] != 1:
            jobsWithErrorText += str(jobsWithErrorList[i]) + ','
            veto = False
if len(jobsWithErrorList) > 0:
    jobsWithErrorText += str(jobsWithErrorList[-1])


# Print the result summary
if jobsWithError != 0 or jobsWithWarning != 0:
    print "--------------------------------------------------"

print str(jobsWithError) + " job(s) with errors, " + str(jobsWithWarning) + \
 " job(s) with warnings, " + str(jobsOk) + " job(s) Ok (" + str(jobsWithError + jobsWithWarning +\
 jobsOk) + " in total)"

text = str(nReadTotal) + " events read, " + str(nPassedTotal) + " events passed"
if jobsWithError != 0:
    text += " (jobs with errors are not accounted for)"
print text

if options.noRootFiles:
    print "WARNING! The output ROOT files were not checked"

if len(rootFilesToDelete) > 0:
    print "Excess ROOT files are found:",
    for r in rootFilesToDelete:
        print r,
    print "\nYou can remove them with the following command:"
    print "rm ",
    for r in rootFilesToDelete:
        print d + r,
    print ''

if len(jobsWithErrorList) > 0:
    print "Compact list of jobs with errors: ", jobsWithErrorText
    print "WARNING! This list also includes the jobs that have not finished or have not been "\
          "retrieved yet. Resubmit these jobs after having checked the list only"
