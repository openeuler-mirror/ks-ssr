#pragma once

#include <QWidget>

namespace Ui
{
class Table;
}
namespace KS
{
class Table : public QWidget
{
    Q_OBJECT

public:
    explicit Table(QWidget *parent);
    virtual ~Table();

    void addSpacer();
    void showTailBar();
    void addLine(const QString &name,
                 const QString &scanResult,
                 const QString &reinforceResult,
                 const QString &remarks,
                 const QColor &scanColor,
                 const QColor &reinforceColor,
                 const QString &backgroundColor);
    void addScanLine(const QString &filesName,
                     const QString &scanType,
                     const QString &remarks,
                     const QString &backgroundColor);

private:
    Ui::Table *m_ui;

    int m_rowHeight;
};
}  // namespace KS
