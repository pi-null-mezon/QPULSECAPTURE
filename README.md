QPULSECAPTURE	

An application for the contactless heart rate measurement.

The "QPULSECAPTURE" project, a descendant of "PULSECAPTURE" project, firstly was designed
for contactless heart rate measurements by means of an ordinary webcam. But later We have
found that such technology can even provide a measurements of photoplethysmogram signal of
face skin reflection. As mentioned before, this project had been created as next step of
"PULSE CAPTURE", but this time We have tried to use libraries with GNU GPL and BSD licenses only. 

Minimal system requirements:
- Intel Pentium IV 3.0 GHz;
- 512 Mb of RAM;
- A webcam 0,3 Mpx (Logitech B910 HD is the best choice).

Acknowledgements:
- OpenCV (http://opencv.org/);
- FFTW (http://www.fftw.org/);
- ALGLIB (http://www.alglib.net/);
- Qt (http://qt-project.org/);
- And for all engineers and programmers, who make the open source products.

For the developers:
- all dependencies are provided in Sources.pro;
- an application was properly builded in Qt-creator_5.2.1 with OpenCV_2.4.8 on MSVC2010x32, MSCV2012x64, and MinGW48x32 compilers;
- WVCFutility is optional, it simply calls DirectShow settings dialog for a webcam, the utility was builded by bcc32 compiler on the base of http://mitov.com/ VisionLab_6.0 library. 

Taranov Alex, 2014.


