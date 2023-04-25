#pragma once

#include <QWidget>
#include "src/ui/trusted/tp-execute-table.h"

namespace Ui {
class TPExecute;
}

namespace KS
{
class TPExecute : public QWidget
{
    Q_OBJECT
public:
    TPExecute(QWidget *parent = nullptr);
    ~TPExecute();

private:
    void updateInfo();

private Q_SLOTS:
    void searchTextChanged(const QString &text);
    void addClicked(bool checked);
    void updateClicked(bool checked);
    void unprotectClicked(bool checked);

private:
    Ui::TPExecute *m_ui;

    TrustedProxy *m_trustedProtectedProxy;
};
}  // namespace KS

