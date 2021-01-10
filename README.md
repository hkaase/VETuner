# VETuner
This tool tunes VE tables using nothing but MAP, RPM, and AFR.


To elaborate, this project seeks to fill a void in the tuning landscape. Tools like HPTuners and EFILive exist to monitor fuel trims and AFR, but not everyone can afford the cost of admission to these. There are alternatives, like Trimalyzer, but there is not support for speed density tuning (without the use of BLM trims or O2 sensors). My approach was to make a tool that accepts any sort of log file containing lines with RPM, MAP, and AFR (in that order), then calculate the AFR for a given VE cell. You can use it as an analysis tool, or even paste raw VE data from TunerPro or HPTuners and apply a correction factor to your own VE table which can then be pasted back.

At the moment, this tool supports AFR and lambda natively, as well as power enrichment.

I have provided a test .csv that is set up for VETuner natively.

If confused on where to start, or what this even is, check the wiki pages for some insight. 

**TO EXECUTE**
Go to releases, download the latest (probably the source code .ZIP if you need the examples), open VE.exe.

**TO BUILD**
This is a difficult project to build, to say the least. The library used for graphing, matplotlibcpp.h, is Python based, and is quite picky about compiling properly. I use Python 3.8, and you must compile with Python's path (if you are on Windows), and also must compile your own Python library by executing the following in your Python root folder: 

cd libs
gendef ..\python38.dll
dlltool --dllname python38.dll --def python38.def --output-lib libpython38.a

Ensure that numpy is installed (pip install numpy), then copy the include/numpy folder to your root Python include folder. Then compile with these command line options.

g++ maincpp -static -o VE.exe -IC:\Python38\include -LC:\Python38\libs -lpython38
