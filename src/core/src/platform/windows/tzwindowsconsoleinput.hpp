#ifndef TZWINDOWSCONSOLEINPUT_HPP
#define TZWINDOWSCONSOLEINPUT_HPP

#include <loom/tzabstractconsoleinput.hpp>

#include <memory>
#include <string>

class TzWindowsConsoleInputPrivate;

class TzWindowsConsoleInput : public TzAbstractConsoleInput
{
public:
    TzWindowsConsoleInput();
    virtual ~TzWindowsConsoleInput() override;

    virtual int fd() const override;
    virtual void start() override;
    virtual void stop() override;
    virtual std::string read() override;

private:
    std::unique_ptr<TzWindowsConsoleInputPrivate> d_ptr;
};

#endif // TZWINDOWSCONSOLEINPUT_HPP
