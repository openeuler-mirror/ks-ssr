#pragma once

#include <QWidget>

class QLabel;

namespace KS
{
class RoundProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit RoundProgressBar(const QString &name,
                              int total,
                              int conform,
                              int inconform,
                              QWidget *parent = nullptr);
    virtual ~RoundProgressBar(){};
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void initUI();

private:
    // 比率label
    QLabel *m_percentLabel;
    // 分类名label
    QLabel *m_nameLabel;
    // 详情label 符合与不符合项
    QLabel *m_noteLabel;
    // 比率
    float m_percent = 0.00;
    QString m_name;
    int m_total;
    int m_conform;
    int m_inconform;
};
}  // namespace KS
