#include "Action.h"
#include "Utils.h"

#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#define _WIN32_WINNT 0x050

bool isRecording = false;
enum State { RECORDING, PLAYING, STANDBY };
State state = STANDBY;
auto seq = ActionsSequence();
long long last_ts;

long long update_clock()
{
    long long new_ts = Utils::now();
    long long diff = new_ts - last_ts;
    last_ts = new_ts;
    return diff;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    BOOL fEatKeystroke = FALSE;

    if (nCode == HC_ACTION)
    {
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
        if (fEatKeystroke = (p->vkCode == 0x7B)) //F12
        {
            if (wParam == WM_KEYUP)
            {
                if (state == RECORDING)
                {
                    state = STANDBY;
                    
                }
                else if (state == STANDBY)
                {
                    seq.clear();
                    state = RECORDING;
                }
                
                update_clock();
                std::cout << "state: " << state << std::endl;

            }
        }
        else if (fEatKeystroke = (p->vkCode == 0x7A)) //F11
        {
            if (wParam == WM_KEYUP)
            {
                state = PLAYING;
                std::cout << "playing... " << std::endl;
            }
        }
        else if (fEatKeystroke = (p->vkCode == 0x79)) //F10
        {
            if (wParam == WM_KEYUP)
            {
                state = STANDBY;
                std::cout << "stopping... " << std::endl;
            }
        }
        else
        {
            if (state == RECORDING)
            {
                long long delay_ms = update_clock();
                std::unique_ptr<Action> delay = std::make_unique<Delay>(delay_ms);
                seq.push_back(std::move(delay));
                std::unique_ptr<Action> kp = std::make_unique<KeyPress>(p->vkCode, wParam);
                seq.push_back(std::move(kp));
                
                seq.print();
            }
        }
    }
    return(fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
}

int main()
{
    // Install the low-level keyboard & mouse hooks
    HHOOK hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);

    // Keep this app running until we're told to stop
    MSG msg;
    while (!GetMessage(&msg, NULL, NULL, NULL)) {    //this while loop keeps the hook
        if (state == PLAYING)
        {
            seq.exec();
        }
        else
        {
            seq.reset();
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hhkLowLevelKybd);

    return(0);
}