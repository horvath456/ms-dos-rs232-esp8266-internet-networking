cls
set path=Z:\;C:\WATCOM\BINW;C:\DRIVER\INCLUDE
SET INCLUDE=C:\WATCOM\H;C:\DRIVER\INCLUDE
SET WATCOM=C:\WATCOM
SET EDPATH=C:\WATCOM\EDDAT
SET WIPFC=C:\WATCOM\WIPFC
rescan
del esp8266.exe
mkdir build
del build\*.*
cd build

wpp -4 -fp5 -ml -xs -obehi+l+mrt -zq ..\src\driver.cpp
wpp -4 -fp5 -ml -xs -obehi+l+mrt -zq ..\src\serial.cpp
wlib driver +driver +serial

mkdir dist
del dist\driver.lib
copy driver.lib dist\driver.lib
mkdir dist\include
del dist\include\*.h
copy ..\include\*.h dist\include

wpp -4 -fp5 -ml -xs -obehi+l+mrt -zq ..\src\main.cpp
wlink @..\proj.lnk
cd ..
copy build\esp8266.exe esp8266.exe