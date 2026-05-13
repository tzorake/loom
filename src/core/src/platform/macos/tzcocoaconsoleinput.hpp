#ifndef TZCOCOACONSOLEINPUT_HPP
#define TZCOCOACONSOLEINPUT_HPP

#include <loom/tzabstractconsoleinput.hpp>

#include <memory>
#include <string>

class TzCocoaConsoleInputPrivate;

class TzCocoaConsoleInput : public TzAbstractConsoleInput
{
public:
    TzCocoaConsoleInput();
    virtual ~TzCocoaConsoleInput() override;

    virtual int fd() const override;
    virtual void start() override;
    virtual void stop() override;
    virtual std::string read() override;

private:
    std::unique_ptr<TzCocoaConsoleInputPrivate> d_ptr;
};

#endif // TZCOCOACONSOLEINPUT_HPP
