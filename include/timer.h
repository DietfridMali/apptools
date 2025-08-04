#pragma once

#include "std_defines.h"
#include <windows.h>
#include <stdio.h>

#include "SDL.h"

// =================================================================================================
// Timer functions: Measuring time, delaying program execution, etc.

class Timer {
public:
    int  m_time;
    int  m_lapTime;
    int  m_duration;
    int  m_slack;

    Timer(int duration = 0)
        : m_time(0), m_lapTime(0), m_duration(duration), m_slack(0)
    {
    }


    inline void Setup(int duration) {
        m_duration = duration;
    }


    inline int Start(int offset = 0) {
        return m_time = SDL_GetTicks() + offset;
    }


    inline int GetLapTime(void) {
        return m_lapTime = SDL_GetTicks() - m_time;
    }


    bool HasExpired(int time = 0, bool restart = false) {
        GetLapTime();
        if (time == 0)
            time = m_duration;
        if ((m_time > 0) && (m_lapTime < time))
            return false;
        if (restart)
            Start();
        return true;
    }


    inline int RemainingTime(void) {
        return m_duration - GetLapTime();
    }


    inline float Progress(void) {
        return float(GetLapTime()) / float(m_duration);
    }


    inline bool IsRemaining(int time) {
        return RemainingTime() >= time;
    }


    void Delay(void) {
        int t = m_duration - m_slack - GetLapTime();
        if (t > 0)
            Sleep(DWORD(t));
        m_slack = GetLapTime() - m_duration;
    }
};

// =================================================================================================
