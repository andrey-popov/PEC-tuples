#!/usr/bin/env python

"""Submits multiple CRAB 3 tasks configured with a JSON file.

This script facilitates configuration and submission of multiple tasks
for CRAB 3 [1].  It starts from a template CRAB configuration, which is
loaded from file 'crabConfig.py' in the current directory by default.
Template is then adjusted for each task according to parameters provided
in a JSON file.

The JSON file must contain a list of parameter sets, one set per task to
be submitted.  A parameter set can add or overwrite any parameter in the
template configuration, referring to it using section name and name of a
parameter within the section, e.g. "JobType.psetName".

In addition, several shortcuts are provided, which only affect those
parameters that have not been set using the fully qualified names.
  "name": Sets General.requestName (if not (re)set explicitly) and name
      of the output file.
  "dataset": Used as alias for Data.inputDataset.
  "params": Updates JobType.pyCfgParams.  In contrast to an explicit
    specification, the list of parameters provided in the template
    configuration is not overwritten but instead extended with the given
    parameters.
Regardless of the way it was constructed, the list JobType.pyCfgParams
in the customized CRAB configuration is checked for presence of cmsRun
argument named "outputFile".  If it is not found, it is added to the
list, giving as the name of the file the value of the shortcut "name" or
General.requestName if the shortcut does not exist.

Input JSON file may contain single-line C++ style comments (//).
Multiline comments (/**/) are not supported.

[1] https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideCrab
"""

import argparse
from copy import deepcopy
from httplib import HTTPException
import imp
import json
import re
import sys

from CRABAPI.RawCommand import crabCommand
from CRABClient.ClientExceptions import ClientException
import WMCore.Configuration


class ConfigInvariantError(Exception):
    """Exception to indicate problems in configuration of a task."""
    pass


def critical_error(formatString, *args, **kwargs):
    """Report a critical error.
    
    Print an error message and exit the script with code 1.  The error
    message is formed from the given format string and subsequent
    arguments using method str.format.
    """
    
    print 'Error:', formatString.format(*args, **kwargs)
    sys.exit(1)


def extract_sample_name(sample):
    """Extract name of a sample.
    
    Check values of fields 'name' and 'General.requestName'.  If they
    are not found or the values are empty, an exception is raised.
    """
    
    if 'name' in sample.keys():
        name = sample['name']
        if name:
            return name
    elif 'General.requestName' in sample.keys():
        name = sample['General.requestName']
        if name:
            return name
    
    raise ConfigInvariantError('No valid name')


def json_byteify(data, ignoreDicts=False):
    """A JSON hook to convert strings from Unicode.
    
    Taken from here [1].
    [1] http://stackoverflow.com/a/33571117/966461
    """
    
    if isinstance(data, unicode):
        return data.encode('utf-8')
    
    if isinstance(data, list):
        return [json_byteify(item, ignoreDicts=True) for item in data]
    
    if isinstance(data, dict) and not ignoreDicts:
        return {
            json_byteify(key, ignoreDicts=True): json_byteify(value, ignoreDicts=True)
            for key, value in data.iteritems()
        }
    
    return data


def strip_comments(fileHandle):
    """Read the file and strip one-line C/C++ comments (//).
    
    Ignore comments inside strings marked with double quotes, taking
    into account possible escaping.  Return a string with the full
    content of the input file without the comments.
    """
    
    cleanLines = []
    
    for line in fileHandle:
        inString = False
        commentFound = False
        
        for i in range(len(line) - 2):
            if line[i] == '"':
                # Handle opening and closing of strings
                if not inString:
                    inString = True
                else:
                    # This is probably the end of a string.  But the
                    # quote might also be escaped.
                    escapeCounts = 0
                    
                    for j in range(i - 1, -1, -1):
                        if line[j] == '\\':
                            escapeCounts += 1
                        else:
                            break
                    
                    if escapeCounts % 2 == 0:
                        inString = False
            
            elif not inString and line.startswith('//', i):
                commentFound = True
                break
        
        if commentFound:
            cleanLines.append(line[:i] + '\n')
        else:
            cleanLines.append(line)
    
    return ''.join(cleanLines)



if __name__ == '__main__':
    
    # Parse arguments
    argParser = argparse.ArgumentParser(
        epilog=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    argParser.add_argument(
        'samples', metavar='samples.json',
        help='JSON file with information about datasets'
    )
    argParser.add_argument(
        '-t', '--template', metavar='crabConfg.py', default='crabConfig.py',
        help='Path to template CRAB configuration'
    )
    args = argParser.parse_args()
    
    
    # Read JSON file with parameters for samples
    try:
        f = open(args.samples)
        content = strip_comments(f)
        parameterSets = json.loads(content, object_hook=json_byteify)
        del(content)
        f.close()
    except EnvironmentError:
        critical_error('Failed to open file "{}".', args.samples)
    except ValueError:
        critical_error('Failed to parse file "{}" as JSON.', args.samples)
    
    if not isinstance(parameterSets, list):
        critical_error('Top-level object in file "{}" is not a list.', args.samples)
    

    # Load template configuration
    try:
        templateModule = imp.load_source('config', args.template)
        templateConfig = templateModule.config
    except EnvironmentError:
        critical_error('Failed to open file "{}".', args.template)
    except (ImportError, AttributeError):
        critical_error(
            'Failed to load template configuration "config" from file "{}".', args.template
        )
    
    if not isinstance(templateConfig, WMCore.Configuration.Configuration):
        critical_error(
            'Atttribute "config" loaded from file "{}" is of a wrong type.', args.template
        )
    
    
    # Validate configurations for all parameter sets
    for index, parameterSet in enumerate(parameterSets):
        
        # Extract name of the current sample to make sure it is valid
        try:
            name = extract_sample_name(parameterSet)
        except ConfigInvariantError:
            critical_error('Task #{} does not have a valid name.', index + 1)
        
        
        # Validate names of fields
        for key in parameterSet.keys():
            
            if '.' in key:
                # This is a fully qualified field for CRAB configuration
                tokens = key.split('.')
                
                if len(tokens) != 2 or not tokens[0] or not tokens[1]:
                    critical_error(
                        'Field "{key}" in configuration of task "{name}" ' \
                        'has illegal format.',
                        key=key, name=name
                    )
                
                if tokens[0] not in ['General', 'JobType', 'Data', 'Site', 'User', 'Debug']:
                    critical_error(
                        'Field "{key}" in configuration of task "{name}" ' \
                        'refers to an unknown section.',
                        key=key, name=name
                    )
            
            else:
                # This is a custom shortcut
                if key not in ['name', 'dataset', 'params']:
                    critical_error('Field "{key}" in configuration of task "{name}" ' \
                        'is unknown.',
                        key=key, name=name
                    )
        
        if 'dataset' in parameterSet.keys() and 'Data.inputDataset' in parameterSet.keys():
            if parameterSets['dataset'] != parameterSet['Data.inputDataset']:
                critical_error(
                    'In task "{name}" both fields "dataset" and "Data.inputDataset" '\
                    'are provided.',
                    name=name
                )
        
        if 'params' in parameterSet.keys() and 'JobType.pyCfgParams' in parameterSet.keys():
            critical_error(
                'In task "{name}" both fields "params" and "JobType.pyCfgParams" are provided.',
                name=name
            )
    
    
    for parameterSet in parameterSets:
        
        # Build a full CRAB configuration for the current sample
        config = deepcopy(templateConfig)
        
        
        # Set all fully qualified parameters.  Syntax of their names has
        # already been validated
        for key in parameterSet.keys():
            if '.' not in key:
                continue
            
            tokens = key.split('.')
            setattr(getattr(config, tokens[0]), tokens[1], parameterSet[key])
        
        
        # Apply customization
        name = extract_sample_name(parameterSet)
        
        if 'General.requestName' not in parameterSet.keys():
            config.General.requestName = name
        
        if 'dataset' in parameterSet.keys():
            config.Data.inputDataset = parameterSet['dataset']
        
        if 'JobType.pyCfgParams' not in parameterSet.keys():
            if hasattr(config.JobType, 'pyCfgParams'):
                params = config.JobType.pyCfgParams
            else:
                params = []
            
            if 'params' in parameterSet.keys():
                params += parameterSet['params']
            
            outputFileGiven = False
            outputFileRegex = re.compile(r'^outputFile\s*=.+$')
            
            for param in params:
                if outputFileRegex.match(param):
                    outputFileGiven = True
                    break
            
            params.append('outputFile=' + name)
        
        if hasattr(config.Data, 'outLFNDirBase'):
            if config.Data.outLFNDirBase.endswith('/'):
                config.Data.outLFNDirBase += name + '/'
            else:
                config.Data.outLFNDirBase += '/' + name + '/'
        
        
        # Submit the task
        try:
            print 'Submitting task "{}"...'.format(name)
            crabCommand('submit', config=config)
        except HTTPException as e:
            print 'Failed to submit because of an HTTP exception:'
            print ' ', str(e.headers)
            print 'Skip this task'
        except ClientException as e:
            print 'Failed to submit because of a client exception:'
            print ' ' + str(e)
            print 'Skip this task'
        
        # A blank line for better formatting
        print ''
