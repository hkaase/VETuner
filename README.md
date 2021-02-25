# VETuner
This tool tunes VE tables using nothing but MAP, RPM, and AFR.


To elaborate, this project seeks to fill a void in the tuning landscape. Tools like HPTuners and EFILive exist to monitor fuel trims and AFR, but not everyone can afford the cost of admission to these. There are alternatives, like Trimalyzer, but there is not support for speed density tuning (without the use of BLM trims or O2 sensors). My approach was to make a tool that accepts any sort of log file containing lines with RPM, MAP, and AFR (in that order), then calculate the AFR for a given VE cell. You can use it as an analysis tool, or even paste raw VE data from TunerPro or HPTuners and apply a correction factor to your own VE table which can then be pasted back.

At the moment, this tool supports AFR and lambda natively, as well as power enrichment.

I have provided a test .csv that is set up for VETuner natively.

If confused on where to start, or what this even is, check the wiki pages for some insight. 

**TO EXECUTE**
Go to releases, download the latest (probably the source code .ZIP if you need the examples), open VE.exe.

**TO BUILD**
This used to be hard to build, but it's not anymore. 

g++ main.cpp -o VE.exe

Then run VE.exe
