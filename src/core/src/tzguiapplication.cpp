#include <event-loop/tzguiapplication.hpp>
#include <event-loop/tzabstractplatformintegration.hpp>
#include <event-loop/tzabstractwindow.hpp>

TzGuiApplication::TzGuiApplication(int argc, char *argv[])
    : TzCoreApplication(argc, argv)
{
}

TzGuiApplication::~TzGuiApplication()
{
}

TzAbstractWindow *TzGuiApplication::createWindow(int width, int height)
{
    return platformIntegration()->createWindow(width, height);
}
