#ifndef TZMACOSCONSOLEINPUT_HPP
#define TZMACOSCONSOLEINPUT_HPP

#include <loom/tzabstractconsoleinput.hpp>

#include <memory>
#include <string>

class TzMacosConsoleInputPrivate;

class TzMacosConsoleInput : public TzAbstractConsoleInput
{
public:
    TzMacosConsoleInput();
    virtual ~TzMacosConsoleInput() override;

    virtual int fd() const override;
    virtual void start() override;
    virtual void stop() override;
    virtual std::string read() override;

private:
    std::unique_ptr<TzMacosConsoleInputPrivate> d_ptr;
};

#endif // TZMACOSCONSOLEINPUT_HPP
