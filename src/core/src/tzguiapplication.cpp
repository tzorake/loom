#include <loom/tzabstractplatformintegration.hpp>
#include <loom/tzabstractwindow.hpp>
#include <loom/tzguiapplication.hpp>

TzGuiApplication::TzGuiApplication(int argc, char *argv[])
    : TzCoreApplication(argc, argv)
{}

TzGuiApplication::~TzGuiApplication() {}

TzAbstractWindow *TzGuiApplication::createWindow(int width, int height)
{
    return platformIntegration()->createWindow(width, height);
}
