#ifndef TPDELEGATE_H
#define TPDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>

namespace KS
{
class TPDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TPDelegate(QObject *parent = 0);
    virtual ~TPDelegate();

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
};
}  // namespace KS

#endif // TPDELEGATE_H
