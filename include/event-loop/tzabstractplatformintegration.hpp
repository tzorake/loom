#ifndef TZABSTRACTPLATFORMINTEGRATION_HPP
#define TZABSTRACTPLATFORMINTEGRATION_HPP

#include <memory>
#include <string>

class TzAbstractEventDispatcher;
class TzAbstractConsoleInput;

class TzAbstractPlatformIntegration
{
public:
    virtual ~TzAbstractPlatformIntegration();
    
    virtual TzAbstractEventDispatcher *createEventDispatcher() = 0;
    virtual TzAbstractConsoleInput *createConsoleInput() = 0;
    virtual std::string name() const = 0;
};

TzAbstractPlatformIntegration *createPlatformIntegration();

#endif // TZABSTRACTPLATFORMINTEGRATION_HPP
