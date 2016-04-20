QPULSECAPTURE	
=========================================================================================

Hey, if you are interested to integrate pulse measurements from face video in to you projects
see [vpglib](https://github.com/pi-null-mezon/vpglib)

An application for the contactless heart rate and breath (respiration) rate measurements
by means of video processing.

Main features:
- provides real time & contactless heart rate measurement (up to each second update), measurement error depends on webcam quality, stationarity of illumination and CPU speed, but in most cases it is less than 3 bpm;
- provides real time & contactless breath (respiration) rate measurement (up to each second update), measurement error depends on webcam quality, stationarity of illumination and CPU speed, but in most cases it is less than 5 rpm;
- provides real time video (from web cam) & existing video file processing (.mp4, .wmv, .avi);
- provides a record of registered signals and measurements to a hard drive;
- provides mapping of arterial pulse on image;
- compares your heart rate with normal heart rate at rest, comparision depends on the age and sex (statistic courtesy by USA National Health Statistics report of 2011 year).

Minimal system requirements:
- Intel Pentium IV 3.0 GHz;
- 512 Mb of RAM;
- A webcam 0,3 Mpx.

Recommended system requirements:
- AMD or Intel with more than 2 cores and clock rate higher than 2.6 GHz;
- 512 Mb of RAM;
- Logitech B910HD in 720p regime.

Acknowledgements:
- OpenCV (http://opencv.org/);
- FFTW (http://www.fftw.org/);
- ALGLIB (http://www.alglib.net/);
- Qt (http://qt-project.org/);
- And for all engineers and programmers who make the open source products! Cheers!

For the developers:
- all dependencies are provided in Sources.pro;
- an application was properly builded in Qt-creator_5.2.1 with OpenCV_2.4.8 on MSVC2010x32, MSCV2012x64, and MinGW48x32 compilers;
- WVCF_utility is optional, it simply calls DirectShow settings dialog for a webcam, the utility was builded by bcc32 compiler on the base of http://mitov.com/ VisionLab_6.0 library. 

Taranov Alex, 2015.


