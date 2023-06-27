#include "settings-respond-dialog.h"
#include <kiran-log/qt5-log-i.h>
#include <QCloseEvent>
#include <QPushButton>
#include <QVBoxLayout>

namespace KS
{
SettingsRespondDialog::SettingsRespondDialog(QWidget *parent) : TitlebarWindow(parent),
                                                                m_message(nullptr),
                                                                m_isAccepted(false)
{
    initUI();
}

SettingsRespondDialog::~SettingsRespondDialog()
{
}

void SettingsRespondDialog::setMessage(const QString &text)
{
    m_message->setText(text);
}

void SettingsRespondDialog::closeEvent(QCloseEvent *event)
{
    if (m_isAccepted)
    {
        emit accepted();
    }
    else
    {
        emit rejected();
    }

    m_isAccepted = false;
    event->ignore();
    hide();
}

void SettingsRespondDialog::initUI()
{
    setObjectName("MessageWindow");
    setTitle(tr("Notify"));
    setIcon(QIcon(":/images/logo"));
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);
    setResizeable(false);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setTitleBarHeight(36);
    setFixedSize(259, 219);

    auto vlay = new QVBoxLayout(getWindowContentWidget());
    vlay->setContentsMargins(4, 4, 4, 4);

    auto cusWidget = new QWidget(getWindowContentWidget());
    cusWidget->setObjectName("customWidget");
    auto contentLayout = new QVBoxLayout(cusWidget);
    contentLayout->setContentsMargins(24, 24, 24, 12);
    contentLayout->setSpacing(10);

    m_message = new QLabel(cusWidget);
    m_message->setWordWrap(true);

    auto btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);
    auto confirm = new QPushButton(tr("Confirm"), cusWidget);
    confirm->setObjectName("confirm");
    confirm->setFixedSize(72, 36);
    auto cancel = new QPushButton(tr("Cancel"), cusWidget);
    cancel->setObjectName("cancel");
    cancel->setFixedSize(72, 36);
    btnLayout->addWidget(confirm);
    btnLayout->addWidget(cancel);

    contentLayout->addWidget(m_message);
    contentLayout->addStretch();
    contentLayout->addLayout(btnLayout);

    connect(confirm, &QPushButton::clicked,
            [=]
            {
                m_isAccepted = true;
                close();
            });
    connect(cancel, &QPushButton::clicked,
            [=]
            {
                m_isAccepted = false;
                close();
            });
    vlay->addWidget(cusWidget);
}
}  // namespace KS
