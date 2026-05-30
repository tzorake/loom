#pragma once

#include <loom/tzabstractconsoleinput.hpp>

// Stub console input for the web platform.
//
// The browser has no TTY / stdin.  Keyboard input arrives as DOM events and
// is delivered into the framework through the exported loom_key_event()
// function in tzwebwindow.cpp.  This class exists only to satisfy the
// TzAbstractPlatformIntegration interface.
class TzWebConsoleInput : public TzAbstractConsoleInput
{
public:
    int         fd()    const override { return -1; }
    void        start()       override {}
    void        stop()        override {}
    std::string read()        override { return {}; }
};
