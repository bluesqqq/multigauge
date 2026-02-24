#pragma once

class GraphicsContext;
class FileSystem;
class Time;
class Logger;

/*
    Global platform services for multigauge-core

    You MUST call setPlatform() exactly once during startup,
    before calling platform(), GFX(), FS(), TIME(), LOG(), etc.
    Calling platform() before setPlatform() is a fata error (library will abort).
*/
struct Platform {
    GraphicsContext& gfx;
    FileSystem& fs;
    Time& time;
    Logger* logger = nullptr;
};

Platform& platform();
void setPlatform(Platform&);
bool initPlatform();

inline GraphicsContext& GFX() { return platform().gfx; }
inline FileSystem& FS() { return platform().fs; }
inline Time& TIME() { return platform().time; }
inline Logger* LOG() { return platform().logger; }