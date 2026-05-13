#ifndef TZWIN32CONSOLEINPUT_HPP
#define TZWIN32CONSOLEINPUT_HPP

#include <loom/tzabstractconsoleinput.hpp>

#include <memory>
#include <string>

class TzWin32ConsoleInputPrivate;

class TzWin32ConsoleInput : public TzAbstractConsoleInput
{
public:
    TzWin32ConsoleInput();
    virtual ~TzWin32ConsoleInput() override;

    virtual int fd() const override;
    virtual void start() override;
    virtual void stop() override;
    virtual std::string read() override;

private:
    std::unique_ptr<TzWin32ConsoleInputPrivate> d_ptr;
};

#endif // TZWIN32CONSOLEINPUT_HPP
