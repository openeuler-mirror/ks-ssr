#ifndef TPKERNEL_H
#define TPKERNEL_H

#include <QWidget>
#include "src/ui/trusted/tp-kernel-table.h"

namespace Ui {
class TPKernel;
}

namespace KS
{
class TPKernel : public QWidget
{
    Q_OBJECT
public:
    TPKernel(QWidget *parent = nullptr);
    ~TPKernel();

private:
    void updateInfo();

private Q_SLOTS:
    void searchTextChanged(const QString &text);
    void addClicked(bool checked);
    void updateClicked(bool checked);
    void unprotectClicked(bool checked);

private:
    Ui::TPKernel *m_ui;

    TrustedProxy *m_trustedProtectedProxy;
};
}  // namespace KS

#endif // TPKERNEL_H
