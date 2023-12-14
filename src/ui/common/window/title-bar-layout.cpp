/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd. 
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

#include "title-bar-layout.h"
#include <QDebug>

namespace KS
{
TitlebarLayout::TitlebarLayout(QWidget *parent)
    : QLayout(parent)
{
    setSpacing(0);
    setMargin(0);
}

TitlebarLayout::~TitlebarLayout()
{
    delete m_iconLabelItem;
    delete m_titleLabelItem;
    delete m_customWidgetItem;
    delete m_rightWidgetItem;
}

QLayoutItem *TitlebarLayout::titleBarIcon()
{
    return m_iconLabelItem;
}

void TitlebarLayout::setTitleBarIconLabel(QLabel *icon)
{
    delete m_iconLabelItem;
    m_iconLabelItem = new QWidgetItem(icon);
}

QLayoutItem *TitlebarLayout::titleBarTitle()
{
    return m_titleLabelItem;
}

void TitlebarLayout::setTitleBarTitleLabel(QLabel *title)
{
    delete m_titleLabelItem;
    m_titleLabelItem = new QWidgetItem(title);
}

QLayoutItem *TitlebarLayout::titleBarCustomWidget()
{
    return m_customWidgetItem;
}

void TitlebarLayout::setTitleBarCustomWidget(QWidget *customWidget)
{
    delete m_customWidgetItem;
    m_customWidgetItem = new QWidgetItem(customWidget);
}

QLayoutItem *TitlebarLayout::titleBarRightWidget()
{
    return m_rightWidgetItem;
}

void TitlebarLayout::setTitleBarRightWidget(QWidget *rightWidget)
{
    delete m_rightWidgetItem;
    m_rightWidgetItem = new QWidgetItem(rightWidget);
}

void TitlebarLayout::addItem(QLayoutItem *item)
{
}

QSize TitlebarLayout::sizeHint() const
{
    return QSize();
}

QLayoutItem *TitlebarLayout::itemAt(int index) const
{
    return nullptr;
}

void TitlebarLayout::setGeometry(const QRect &rect)
{
    ///如果标题栏自定义控件区域需要居中
    ///１.算出左侧未省略文本和图标的一般宽度，算出右侧按钮的宽度，两者取最大值，都设置为最大值
    ///2.如果在两边宽度相同的情况下，看是否能放下合适大小的自定义控件区域,如果不能则需要省略左侧文本，同时再计算左右宽度最大值同步
    if (rect == geometry())
    {
        return;
    }

    QRect contentRect = rect;
    int leftMargin, topMargin, rightMargin, bottomMargin;
    getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
    contentRect.adjust(leftMargin, topMargin, -rightMargin, -bottomMargin);

    QSize iconLabelSize = m_iconLabelItem->sizeHint();

    //获取文本显示完全应该占用的大小
#if 0
    QFontMetrics fontMetrics = m_titleLabelItem->widget()->fontMetrics();
    QSize titleLabelSize;
    titleLabelSize.setHeight(fontMetrics.height());
    titleLabelSize.setWidth(fontMetrics.width(m_completeTitle));
#endif
    /// fixme:由于QFontMetrics::width(QString,int)和QFontMetrics::boundingRect(QString)获取的字体宽度有误差(过小)，导致标题栏空间足够但被压缩
    /// note:现在先设置lable文本为未省略的文本，再获取应该呈现的大小，再进行计算省略后的文本，会更加耗资源
    QLabel *labelTitle = qobject_cast<QLabel *>(m_titleLabelItem->widget());
    labelTitle->setText(m_completeTitle);
    QSize titleLabelSize = labelTitle->sizeHint();

    QSize customWidgetSize = m_customWidgetItem->sizeHint();
    QSize rightWidgetSize = m_rightWidgetItem->sizeHint();

    /*　居中左右两侧宽度需要同步，计算剩余宽度是否够　*/
    if (m_customWidgetCenter)
    {
        //左侧需要占据的宽度
        int leftSideWidth = m_iconMargins.left() + iconLabelSize.width() + m_iconMargins.right() +
                            m_titleMargins.left() + titleLabelSize.width() + m_titleMargins.right();
        //右侧需要占据的宽度
        int rightSideWidth = m_rightWidgetMargins.left() + rightWidgetSize.width() + m_rightWidgetMargins.right();
        //如果不调整两侧宽度，判断是否能居中装下自定义组件
        if ((contentRect.width() - (2 * qMax(leftSideWidth, rightSideWidth))) <
            (m_customWidgetMargins.left() + customWidgetSize.width() + m_customWidgetMargins.right()))
        {
            /* 剩下的空间不够的情况，需要省略文本,最多把左侧宽度压缩到右侧一样，不能改变右侧的宽度！ */
            //计算出合适的两侧宽度
            int fitSideWidth = (contentRect.width() - (m_customWidgetMargins.left() + customWidgetSize.width() +
                                                       m_customWidgetMargins.right())) /
                               2;
            if (fitSideWidth < rightSideWidth)
            {
                qWarning("侧边宽度不能压缩小于右侧宽度");
                return;
            }
            //通过侧边宽度计算出左侧标题label的宽度
            int reducedTitleWidth = fitSideWidth - m_iconMargins.left() - iconLabelSize.width() - m_iconMargins.right() - m_titleMargins.left() - m_titleMargins.right();
            titleLabelSize.setWidth(reducedTitleWidth);
            //设置右侧的宽度
            int rightWidgetWidth = fitSideWidth - m_rightWidgetMargins.left() - m_rightWidgetMargins.right();
            rightWidgetSize.setWidth(rightWidgetWidth);
        }
        else
        {
            int fitSideWidth = qMax(leftSideWidth, rightSideWidth);
            int fitRightWidgetWidth = fitSideWidth - m_rightWidgetMargins.left() - m_rightWidgetMargins.right();
            int fitTitleLabelWidth = fitSideWidth - m_iconMargins.left() - iconLabelSize.width() - m_iconMargins.right() - m_titleMargins.left() - m_titleMargins.right();
            titleLabelSize.setWidth(fitTitleLabelWidth);
            rightWidgetSize.setWidth(fitRightWidgetWidth);
        }
    }
    else
    {
        int leftSideWidth = m_iconMargins.left() + iconLabelSize.width() + m_iconMargins.right() +
                            m_titleMargins.left() + titleLabelSize.width() + m_titleMargins.right();
        int rightSideWidth = m_rightWidgetMargins.left() + rightWidgetSize.width() + m_rightWidgetMargins.right();
        int customWidgetWidth = m_customWidgetMargins.left() + customWidgetSize.width() + m_customWidgetMargins.right();
        //如果该大小放置不下 CustomWidget则调整标题栏文本长度
        if ((contentRect.width() - leftSideWidth - rightSideWidth) <
            (m_customWidgetMargins.left() + customWidgetSize.width() + m_customWidgetMargins.right()))
        {
            /*正常情况放置不下,需要压缩左侧标题Label宽度*/
            /*计算压缩后的Label宽度*/
            int reducedTitleWidth = contentRect.width() - m_iconMargins.left() - iconLabelSize.width() - m_iconMargins.right() -
                                    m_titleMargins.left() - m_titleMargins.right() - customWidgetWidth - rightSideWidth;
            titleLabelSize.setWidth(reducedTitleWidth);
        }
    }

    //定位左侧的图标、标题栏
    QRect iconLabelRect(contentRect.left() + m_iconMargins.left(),
                        contentRect.top() + (contentRect.height() - iconLabelSize.height()) / 2,
                        iconLabelSize.width(),
                        iconLabelSize.height());
    m_iconLabelItem->setGeometry(iconLabelRect);
    contentRect.setLeft(iconLabelRect.right() + m_iconMargins.right());

    QRect titleLabelRect(contentRect.left() + m_titleMargins.left(),
                         contentRect.top() + (contentRect.height() - titleLabelSize.height()) / 2,
                         titleLabelSize.width(),
                         titleLabelSize.height());
    QFontMetrics elideFontMetrics(m_titleLabelItem->widget()->font());
    QLabel *label_title = qobject_cast<QLabel *>(m_titleLabelItem->widget());
    label_title->setText(elideFontMetrics.elidedText(m_completeTitle, Qt::ElideRight, titleLabelRect.width()));
    m_titleLabelItem->setGeometry(titleLabelRect);
    contentRect.setLeft(titleLabelRect.right() + m_titleMargins.right());

    //定位右侧的组件
    QRect rightWidgetRect(contentRect.right() - m_rightWidgetMargins.right() - rightWidgetSize.width(),
                          contentRect.top() + (contentRect.height() - rightWidgetSize.height()) / 2,
                          rightWidgetSize.width(),
                          rightWidgetSize.height());
    m_rightWidgetItem->setGeometry(rightWidgetRect);
    contentRect.setRight(rightWidgetRect.left() + m_rightWidgetMargins.left());

    //自定义控件使用剩下的内容区域
    QRect customWidgetRect(contentRect.left() + m_customWidgetMargins.left(),
                           contentRect.top() + (contentRect.height() - customWidgetSize.height()) / 2,
                           contentRect.width() - m_customWidgetMargins.right(),
                           customWidgetSize.height());
    m_customWidgetItem->setGeometry(customWidgetRect);
    QLayout::setGeometry(rect);
}

QSize TitlebarLayout::minimumSize() const
{
    //,如果不居中则可按照标题Label宽度为0来进行计算
    QSize size = QLayout::minimumSize();
    int minWidth = 0;
    int leftMargin, topMargin, rightMargin, bottomMargin;

    getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
    minWidth += leftMargin + rightMargin;

    if (m_customWidgetCenter)
    {
        //NOTE:如果是居中自定义控件的话，标题栏最小宽度应该是右侧侧边栏宽度x2+自定义控件宽度
        int rightSideWidth = m_rightWidgetItem->sizeHint().width() + m_rightWidgetMargins.left() + m_rightWidgetMargins.right();
        int customWidgetWidth = m_customWidgetMargins.left() + m_customWidgetItem->sizeHint().width() + m_customWidgetMargins.right();
        minWidth += 2 * rightSideWidth;
        minWidth += customWidgetWidth;
    }
    else
    {
        //NOTE:如果不需要居中自定义控件的话,标题Label宽度可以计算为0
        int rightSideWidth = m_rightWidgetItem->sizeHint().width() + m_rightWidgetMargins.left() + m_rightWidgetMargins.right();
        int customWidgetWidth = m_customWidgetMargins.left() + m_customWidgetItem->sizeHint().width() + m_customWidgetMargins.right();
        int leftSideWidth = m_iconMargins.left() + m_iconLabelItem->sizeHint().width() + m_iconMargins.right();
        minWidth += leftSideWidth;
        minWidth += customWidgetWidth;
        minWidth += rightSideWidth;
    }

    size.setWidth(minWidth);
    return size;
}

QLayoutItem *TitlebarLayout::takeAt(int index)
{
    return nullptr;
}

int TitlebarLayout::count() const
{
    int count = 0;
    if (m_iconLabelItem != nullptr)
        count++;
    if (m_titleLabelItem != nullptr)
        count++;
    if (m_customWidgetItem != nullptr)
        count++;
    if (m_rightWidgetItem != nullptr)
        count++;
    return count;
}

QMargins TitlebarLayout::titleBarIconMargin()
{
    return m_iconMargins;
}

void TitlebarLayout::setTitleBarIconMargin(QMargins margins)
{
    if (m_iconMargins != margins)
    {
        m_iconMargins = margins;
        invalidate();
    }
}

QMargins TitlebarLayout::titleBarLabelMargin()
{
    return m_titleMargins;
}

void TitlebarLayout::setTitleBarTLabelMargin(QMargins margins)
{
    if (m_titleMargins != margins)
    {
        m_titleMargins = margins;
        invalidate();
    }
}

QMargins TitlebarLayout::titleBarCustomMargin()
{
    return m_customWidgetMargins;
}

void TitlebarLayout::setTitleBarCustomMargin(QMargins margins)
{
    m_customWidgetMargins = margins;
}

QMargins TitlebarLayout::titleBarRightMargin()
{
    return m_rightWidgetMargins;
}

void TitlebarLayout::setTitleBarRightWidgetMargin(QMargins margins)
{
    if (m_rightWidgetMargins != margins)
    {
        m_rightWidgetMargins = margins;
        invalidate();
    }
}

void TitlebarLayout::setCustomWidgetCenter(bool center)
{
    if (m_customWidgetCenter != center)
    {
        m_customWidgetCenter = center;
        invalidate();
    }
}

void TitlebarLayout::setTitleBarCompleteTitle(const QString &title)
{
    if (m_completeTitle != title)
    {
        m_completeTitle = title;
        invalidate();
    }
}

bool TitlebarLayout::customWidgetCenter()
{
    return m_customWidgetCenter;
}

}  // namespace  KS
