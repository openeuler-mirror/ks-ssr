#ifndef TPKERNELDELEGATE_H
#define TPKERNELDELEGATE_H

#include <QObject>
#include "src/ui/tp/tp-delegate.h"

namespace KS
{
// 防卸载列数
#define PROHIBIT_UNLOADING_COL 4

class TPKernelDelegate : public TPDelegate
{
    Q_OBJECT
public:
    TPKernelDelegate(QObject *parent = nullptr);
    virtual ~TPKernelDelegate(){};

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
};
}  // namespace KS
#endif // TPKERNELDELEGATE_H
