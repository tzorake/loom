#ifndef TZEVENT_HPP
#define TZEVENT_HPP

class TzEvent
{
public:
    enum Type {
        None          = 0,
        DeferredDelete = 1,
        User          = 1000,  // custom event types start here
    };

    explicit TzEvent(int type);
    virtual ~TzEvent();

    int  type() const;

    void setAccepted(bool accepted);
    bool isAccepted() const;
    void accept();
    void ignore();

    virtual TzEvent *clone() const;

private:
    int  m_type;
    bool m_accepted{ true };
};

#endif // TZEVENT_HPP
