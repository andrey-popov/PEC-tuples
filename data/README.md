# Data files

This directory contains the following data files:

 * `Fall15_25nsV2_MC_Uncertainty_AK4PFchs.txt` <br />
   JEC total uncertainties taken from a tarball provided [here](https://twiki.cern.ch/twiki/bin/viewauth/CMS/JECDataMC?rev=111#Recommended_for_MC). The file is needed by the PAT MET tool to evaluate systematic uncertainties in MET corresponding to variations in jet energy scale.
 * `ecalscn1043093_Dec01.txt` <br />
   A copy of [this file](https://twiki.cern.ch/twiki/pub/CMS/MissingETOptionalFiltersRun2/ecalscn1043093_Dec01.txt.gz) (referenced from [this page](https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2?rev=70)). List of IDs of additional events flagged as “bad” because of a noisy supercluster in EE.

Additional files are not stored under the version control system but are downloaded by the `download.sh` script:

 * `csc2015_Dec01.root` <br />
   List of IDs of events flagged by the beam halo filter. The list is taken from [this file](https://twiki.cern.ch/twiki/pub/CMS/MissingETOptionalFiltersRun2/csc2015_Dec01.txt.gz) (referenced from [this page](https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2?rev=70)) and converted to a ROOT tree in order to make the file more compact.
