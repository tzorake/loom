#ifndef TZGUIAPPLICATION_HPP
#define TZGUIAPPLICATION_HPP

#include <loom/tzcoreapplication.hpp>

#define tzGuiApp (static_cast<TzGuiApplication *>(TzCoreApplication::instance()))

class TzGuiApplication : public TzCoreApplication
{
public:
    TzGuiApplication(int argc, char *argv[]);
    virtual ~TzGuiApplication() override;
};

#endif // TZGUIAPPLICATION_HPP
