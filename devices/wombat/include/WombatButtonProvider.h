#ifndef _WALLABYBUTTONPROVIDER_H_
#define _WALLABYBUTTONPROVIDER_H_

#include "ButtonProvider.h"

namespace kipr::button {
class AbstractTextButton;
}

namespace Wombat {

class ButtonProvider : public ::ButtonProvider {
    Q_OBJECT
  public:
    ButtonProvider(QObject *parent = 0);
    ~ButtonProvider();

    virtual bool isExtraShown() const;
    virtual QString text(const ButtonProvider::ButtonId &id) const;
    virtual bool setPressed(const ButtonProvider::ButtonId &id,
                            bool pressed) const;

  public slots:
    virtual void reset();
    virtual void refresh();

  private:
    kipr::button::AbstractTextButton *
    lookup(const ButtonProvider::ButtonId &id) const;

    bool m_shown;
};
} // namespace Wombat

#endif
