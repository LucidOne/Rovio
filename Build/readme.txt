How to create an executable binary for Rovio...

Notes:
The easiest way to compile the Rovio source files for Rovio's platform is using the ARM developer suite called ADS 1.2.

Steps 1-3 can also be completed by running IPCam_Builder.hta unchecking everything but server components however ADS 1.2 must be installed first.

Instructions:
1. Compile the libraries at ..\Host
2. Compile the IPCam application at
     ..\Host\LibCamera\Samples\CameraTest\Src\CameraTest.mcp
   The built result can be found at:
     ..\Host\LibCamera\Samples\CameraTest\Bin\CameraTest.bin
3. Make a self-decompressed .bin executable
   Compress CameraTest.bin and then attach wpackage.bin at the beginning.
     zip -9 IPCam.zip CameraTest.bin
     cat wpackage.bin CameraTest.zip > IPCam.bin
4. Use ..\Tools\FirmwareMaker.exe to add the web and resource files.
5. Flash the firmware onto Rovio using Rovio Firmware Updater (..\Tools\RovioFirmwareUpdaterInstaller_v2.1.exe).