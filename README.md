# PEC-tuples

Code in this repository is intended for physics analysis of data collected by the [CMS experiment](https://cms.cern) (CERN). You must be a CMS user to be able to use it.

The repository defines a package for the [CMSSW](https://github.com/cms-sw/cmssw). It allows to run over the [MiniAOD](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD) data tier and save relevant reconstructed and generator-level physics objects in a simple [ROOT](https://root.cern.ch) format for subsequent analysis. No dependence on CMSSW is required to read produced files. An analysis that starts from these files can be implemented with the [mensura framework](https://github.com/andrey-popov/mensura/).

Usage instructions are provided in the [wiki](https://github.com/andrey-popov/PEC-tuples/wiki) of the repository. Detailed documentation is included in the source code.

As of 2019, this package is not being maintained actively. Consider switching to [NanoAOD](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookNanoAOD) instead, which is conceptually similar to PEC tuples.
