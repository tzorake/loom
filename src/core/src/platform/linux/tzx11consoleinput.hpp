#ifndef TZX11CONSOLEINPUT_HPP
#define TZX11CONSOLEINPUT_HPP

#include <loom/tzabstractconsoleinput.hpp>

#include <memory>
#include <string>

class TzX11ConsoleInputPrivate;

class TzX11ConsoleInput : public TzAbstractConsoleInput
{
public:
    TzX11ConsoleInput();
    ~TzX11ConsoleInput() override;

    int fd() const override;
    void start() override;
    void stop() override;
    std::string read() override;

private:
    std::unique_ptr<TzX11ConsoleInputPrivate> d_ptr;
};

#endif // TZX11CONSOLEINPUT_HPP
