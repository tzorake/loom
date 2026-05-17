#ifndef TZABSTRACTCONSOLEINPUT_HPP
#define TZABSTRACTCONSOLEINPUT_HPP

#include <string>

class TzAbstractConsoleInput
{
public:
    virtual ~TzAbstractConsoleInput();

    virtual int fd() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual std::string read() = 0;
};

#endif // TZABSTRACTCONSOLEINPUT_HPP
