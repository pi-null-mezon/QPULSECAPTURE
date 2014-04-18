/*------------------------------------------------------------------------------
 This program was created as utility, that tries to open settings dialog for
 plugged video capture device. It is free for noncommercial use, and was created
 on the base of VIdeoLab library, thus all rights belong to Bojan Mitov,
 check it out www.mitov.com
 Taranov Alex, 2014
------------------------------------------------------------------------------*/
#include <vcl.h>
#include <iostream>

#pragma hdrstop
#pragma argsused

#include "VLDSCapture.h"
#include <tchar.h>

#ifdef _UNICODE
#define _tcout std::wcout
#else
#define _tcout std::cout
#endif

int _tmain(int argc, _TCHAR* argv[])
{
	_tcout << "Tries to construct video capture filter setting dialog:" << std::endl;
	_tcout << "Tries to construct TVLDSCapture instance..." << std::endl;
	TVLDSCapture *ptDevice = new TVLDSCapture(NULL);

	_TCHAR temp;

	while( (--argc > 0) && ((*++argv)[0] == '-')) // a loop till argv[] ends
		while (temp = *++argv[0]) // a loop till string's end
			switch (temp)
			{
				case 'l':
					_tcout << "Launched with device select option, tries to open device select dialog..." << std::endl;
					if (!ptDevice->VideoCaptureDevice->ShowDeviceSelctDialog())
					{
						_tcout << "Fails or dialog canceled..." << std::endl << "Exit with code -1";
						delete ptDevice;
						return -1;
					}
					break;
			} 

	_tcout << "Tries to open video capture device...." << std::endl;
	if(ptDevice->Open())
	{
		_tcout << "Success..." << std::endl;
		_tcout << "Tries to open Video Capture Filter embedded dialog..." << std::endl;
		if (ptDevice->ShowVideoDialog( cdVideoCapture ))
		{
			_tcout << "Success..." << std::endl << "Exit with success code 0";
			ptDevice->Close(true);
			delete ptDevice;
			return 0;
		}
	}
	_tcout << "Fails..." << std::endl << "Exit with error code -2";
	delete ptDevice;
	return -2;
}
