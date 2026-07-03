#ifndef TRUSTEDUSERPIN_H
#define TRUSTEDUSERPIN_H

#include <QWidget>
#include "src/ui/common/titlebar-window.h"

namespace Ui {
class TrustedUserPin;
}

namespace KS
{
class TrustedUserPin : public TitlebarWindow
{
    Q_OBJECT

public:
    TrustedUserPin(QWidget *parent = nullptr);
    ~TrustedUserPin();

    QString getUserPin();

private:
    void init();

signals:
    void accepted();
    void rejected();

private:
    Ui::TrustedUserPin *m_ui;
};
}  // namespace KS

#endif // TRUSTEDUSERPIN_H
