#ifndef TZEVENT_HPP
#define TZEVENT_HPP

class TzEvent
{
public:
    enum Type {
        None = 0,
        User = 1000,  // custom event types start here
    };

    explicit TzEvent(int type);
    virtual ~TzEvent();

    int type() const;

private:
    int m_type;
};

#endif // TZEVENT_HPP
