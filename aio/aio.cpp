// aio.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include "aio.h"
#include "Caio.h"
#include "theadsafe_queue.h"
#include <chrono>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//char dev_name[7] = "AIO000";
//Return values of functions 
long	Ret, Ret2;
//ID
short	Id;
//Number of the used channels
short	AiChannels;
//Error code string 
char	ErrorString[256];
TCHAR	szErrorString[256];
//Array uesd for storing the converted data
#define	DATA_MAX	16000
float	AiData[DATA_MAX];
//Count of the occurred event
long	AiEventCount;
//Retrieved total number of samplings
long	AiTotalSamplingTimes;
codepi::ThreadSafeQueue<float> q;
codepi::ThreadSafeQueue<double> q_time;
// 唯一的应用程序对象

CWinApp theApp;

using namespace std;


int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: 在此处为应用程序的行为编写代码。
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}

static long CALLBACK test_cb(short Id, short AiEvent, WPARAM wParam, LPARAM lParam, void* Param)
{
    double stamp;
    switch (AiEvent)
    {
    case AIOM_AIE_DATA_NUM:
        AiEventCount++;
        if (lParam * AiChannels > DATA_MAX) {
            lParam = DATA_MAX / AiChannels;
        }
        //Get the converted data
        Ret = AioGetAiSamplingDataEx(Id, (long*)&lParam, &AiData[0]);
        std::chrono::system_clock::duration d = std::chrono::system_clock::now().time_since_epoch();
        std::chrono::nanoseconds nan = std::chrono::duration_cast<std::chrono::nanoseconds>(d);
        stamp = nan.count() / 1000000000.0;
        if (Ret != 0) {
            Ret2 = AioGetErrorString(Ret, ErrorString);
            wsprintf(szErrorString, _T("%S"), ErrorString);
            printf(ErrorString);
            return 0;
        }
        for (int i = 0; i < lParam; i++)
        {
            q.enqueue(AiData[i]);
            q_time.enqueue(stamp);
        }

        return 1;
        break;
    default:
        return 1;
    }
}
//
void get_available_buffer_size(int *length)
{
    *length = q.size();
}

void get_buffer(int num, float* data, double &stamp)
{
    for (int i = 0; i < num; i++)
    {
        data[i] = q.dequeue();
        stamp = q_time.dequeue();
    }
}
//
int exit_aio()
{
    //Stop converting
    Ret = AioStopAi(Id);
    if (Ret != 0) {
        return Ret;
    }
    //Exit handling of device	
    Ret = AioExit(Id);
    if (Ret != 0) {
        return Ret;
    }
}

long aio_start()
{
    //Reset memory
    Ret = AioResetAiMemory(Id);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return Ret;
    }

    q.clear();

    Ret = AioStartAi(Id);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return Ret;
    }
    return 1;
}

long aio_stop()
{
    Ret = AioStopAi(Id);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return 1;
    }

    q.clear();
    return 1;
}

long aio_init(char *dev_name, float delta, short range, long simple_times)
{
    Ret = AioInit(dev_name, &Id);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return 0;
    }

    //Reset device
    Ret = AioResetDevice(Id);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return 0;
    }

    Ret = AioSetAiRangeAll(Id, range);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return 0;
    }

    AiEventCount = 0;
    AiTotalSamplingTimes = 0;

    //Set the number of channels : 1 channel
    AiChannels = 1;
    Ret = AioSetAiChannels(Id, AiChannels);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return 0;
    }

    //Set the memory type : FIFO
    Ret = AioSetAiMemoryType(Id, 0);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return 0;
    }

    //Set the clock type : Internal
    Ret = AioSetAiClockType(Id, 0);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return 0;
    }

    //Set the sampling clock : simple_clock usec
    Ret = AioSetAiSamplingClock(Id, delta);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return 0;
    }

    //Set the start condition : Software
    Ret = AioSetAiStartTrigger(Id, 0);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return 0;
    }

    //Set the stop condition : Command
    Ret = AioSetAiStopTrigger(Id, 4);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return 0;
    }

    //Set the event factor : Device operation end, store the data of the specified sampling times, 
    //						 Overflow, Clock error, AD converting error
    long	AiEvent;
    AiEvent = AIE_DATA_NUM;
    int temp = 100;
    Ret = AioSetAiCallBackProc(Id, test_cb, AiEvent, (void*)&temp);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return 0;
    }

    //Set the number of samplings of the event that data of the specified sampling times are stored : 1000	
    Ret = AioSetAiEventSamplingTimes(Id, simple_times);
    if (Ret != 0) {
        Ret2 = AioGetErrorString(Ret, ErrorString);
        wsprintf(szErrorString, _T("%S"), ErrorString);
        printf(ErrorString);
        return 0;
    }

    return 1;
}