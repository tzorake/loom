#ifndef TZGUIAPPLICATION_HPP
#define TZGUIAPPLICATION_HPP

#include <event-loop/tzcoreapplication.hpp>

class TzAbstractWindow;

class TzGuiApplication : public TzCoreApplication
{
public:
    TzGuiApplication(int argc, char *argv[]);
    virtual ~TzGuiApplication() override;

    TzAbstractWindow *createWindow(int width, int height);
};

#endif // TZGUIAPPLICATION_HPP
