# SpeedDensityTuner
This tool tunes VE tables using nothing but MAP, RPM, and AFR.


To elaborate, this project seeks to fill a void in the tuning landscape. Tools like HPTuners and EFILive exist to monitor fuel trims and AFR, but not everyone can afford the cost of admission to these. There are alternatives, like Trimalyzer, but there is not support for speed density tuning (without the use of BLM trims or O2 sensors). My approach was to make a tool that accepts any sort of log file containing lines with RPM, MAP, and AFR (in that order), then calculate the AFR for a given VE cell. You can use it as an analysis tool, or even paste raw VE data from TunerPro or HPTuners and apply a correction factor to your own VE table which can then be pasted back.

At the moment, this tool supports AFR natively, with lambda coming soon. DO NOT TUNE POWER ENRICHMENT WITH THIS TOOL. Or at least automatically with paste data. If you must tune WOT, use the observed AFR table to reference against, as the program has no concept of changing AFR during PE.

#TODO
Add lambda support
Add support for PE detection (don't want the program to think 12.7 is rich in a PE cell)
Test other types of VE tables
