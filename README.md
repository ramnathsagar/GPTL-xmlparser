GPTL-xmlparser
==============

XML Parser library for GPTL profiling options

This repo provides a capability to hook XML parser into the existing GPTL instrumentation Library.


You can grab the latest version from git repo:


The current limiation with GPTL is that the 'GPTL Profiling Options' are hardcoded into the source. Hence everytime, a profiling option has to be added/removed , one has to recompile the package.
An alternate approach would be to use an xml based file where one could provide profiling options, thereby providing the xml file during profling. This prevents user from recompiling the package and hence improve the development time.

In order to do so, one needs to install LIBXML2 and its devel package.

The installation process is :

* Clone the GPTL from : git@github.com:jmrosinski/GPTL.git
* Clone this xmlParser from: git@github.com:ramnathsagar/GPTL-xmlparser.git
* Apply the provided patch GPTL-XMLparser.patch
* Continue with the installation as described in GPTL

This GIT repo also has two folders

script/
A sample script to initialize GPTL XML environment varialbles and run an application (linked with -lgptl)

config/
A sample gptl.xml file where one can specify profiling options.


If you have any suggestions on profiling options that could be added to XML parser / any general suggestions / any help,  drop by a line @ ramnath.sagar@gmail.com

