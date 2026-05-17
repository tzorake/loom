#ifndef TZWIDGET_HPP
#define TZWIDGET_HPP

#include <loom/tzobject.hpp>
#include <loom/tzclasshelpermacros.hpp>

class TzAnchors;
class TzPainter;
class TzScene;
struct TzRect;
struct TzPoint;
struct TzSize;
class TzWidgetPrivate;

class TzWidget : public TzObject
{
    TZ_DECLARE_PRIVATE_D(d_ptr, TzWidget)
public:
    explicit TzWidget(TzWidget *parent = nullptr);
    virtual ~TzWidget() override;

    void setX(double x);
    double x() const;

    void setY(double y);
    double y() const;
    
    void setWidth(double width);
    double width() const;

    void setHeight(double height);
    double height() const;

    bool setGeometry(double x, double y, double width, double height);
    bool setGeometry(const TzRect &rect);
    TzRect geometry() const;

    TzPoint position() const;
    TzSize size() const;

    void move(const TzPoint &pos);
    void resize(const TzSize &size);

    void setImplicitWidth(double w);
    double implicitWidth() const;

    void setImplicitHeight(double h);
    double implicitHeight() const;

    void setImplicitSize(double w, double h);
    TzSize implicitSize() const;

    double effectiveWidth()  const;
    double effectiveHeight() const;

    TzAnchors *anchors();

    void setVisible(bool visible);
    bool isVisible() const;

    void setFocus();
    bool hasFocus() const;

    void update();
    TzScene *scene() const;

    TzWidget *parentWidget() const;

    TzPoint mapToGlobal(const TzPoint &local) const;
    TzPoint mapFromGlobal(const TzPoint &global) const;

    virtual void paint(TzPainter *painter);

    bool event(TzEvent *event) override;

protected:
    explicit TzWidget(TzWidgetPrivate &d, TzWidget *parent = nullptr);

    virtual void geometryChanged(const TzRect &newGeom, const TzRect &oldGeom);

private:
    friend class TzScene;
    friend class TzScenePrivate;
};

#endif // TZWIDGET_HPP
