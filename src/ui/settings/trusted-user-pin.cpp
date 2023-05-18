#include "trusted-user-pin.h"
#include "ui_trusted-user-pin.h"

namespace KS
{
TrustedUserPin::TrustedUserPin(QWidget *parent) :
    TitlebarWindow(parent),
    m_ui(new Ui::TrustedUserPin)
{
    m_ui->setupUi(getWindowContentWidget());

    init();
}

TrustedUserPin::~TrustedUserPin()
{
    delete m_ui;
}

QString TrustedUserPin::getUserPin()
{
    return m_ui->m_userPin->text();
}

void TrustedUserPin::init()
{
    // 页面关闭时销毁
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::ApplicationModal);
    setIcon(QIcon(":/images/logo"));
    setTitle(tr("Input pin code"));
    setResizeable(false);
    setTitleBarHeight(36);
    setFixedSize(300, 240);
    setButtonHints(TitlebarWindow::TitlebarCloseButtonHint);

    connect(m_ui->m_cancel, &QPushButton::clicked, this, [this]
            {
                close();
                emit rejected();
            });

    connect(m_ui->m_ok, &QPushButton::clicked, this, [this]
            {
                close();
                emit accepted();
                m_ui->m_userPin->setText("");
            });
}
}  // namespace KS
