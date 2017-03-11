#ifndef PROCESSREADER_H
#define PROCESSREADER_H

#include <windows.h>
#include <tchar.h>
#include <vector>

class ProcessReader
{
public:
    ProcessReader();
    void update();
private:
    void getBaseAddress();
    DWORD_PTR baseAddress;
    DWORD pid;
    HANDLE windowHandle;
};

#endif // PROCESSREADER_H
