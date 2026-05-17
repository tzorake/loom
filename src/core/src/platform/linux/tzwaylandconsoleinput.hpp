#ifndef TZWAYLANDCONSOLEINPUT_HPP
#define TZWAYLANDCONSOLEINPUT_HPP

#include <loom/tzabstractconsoleinput.hpp>

#include <memory>
#include <string>

class TzWaylandConsoleInputPrivate;

class TzWaylandConsoleInput : public TzAbstractConsoleInput
{
public:
    TzWaylandConsoleInput();
    virtual ~TzWaylandConsoleInput() override;

    virtual int fd() const override;
    virtual void start() override;
    virtual void stop() override;
    virtual std::string read() override;

private:
    std::unique_ptr<TzWaylandConsoleInputPrivate> d_ptr;
};

#endif // TZWAYLANDCONSOLEINPUT_HPP
