#include "main_window.h"

#include <QFontDatabase>
#include <QScrollBar>
#include <QPainter>

#include <QTextStream>



MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{

    QIcon sendButtonIcon(":/images/resources/img/sendButton.png");
    QIcon addAttachmentIcon(":/images/resources/img/addAttachmentIcon.png");
    QIcon settingsIcon(QPixmap(":/images/resources/img/settings.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QIcon logOutIcon(QPixmap(":/images/resources/img/logOut.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QIcon toggleThemeIcon(QPixmap(":/images/resources/img/darkMode.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QIcon chatSearchIcon(QPixmap(":/images/resources/img/search.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QIcon muteIcon(QPixmap(":/images/resources/img/mute.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QIcon chatInfoIcon(QPixmap(":/images/resources/img/info.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QFontDatabase::addApplicationFont(":/font/resources/Nunito/Nunito-Regular.ttf");
    QFont nunitoFont("Nunito");
    setFont(nunitoFont);

    lastMsgId = db->check();

    connect(addAttachment, &QPushButton::clicked, this, &MainWindow::openPhoto);
    connect(sendMessageButton, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(inputField, &InputField::ret, this, &MainWindow::sendMessage);
    connect(maw, &ScrollAreaWidget::upd, this, [=] {messageArea->ensureWidgetVisible(lastMsg);});

    connect(loginWidget->loginButton, &QPushButton::clicked, this, &MainWindow::login);
    connect(loginWidget, &Login::login_field_correct, loginWidget, &Login::redrawLoginField);
    connect(loginWidget, &Login::pass_field_correct, loginWidget, &Login::redrawPasswordField);
    connect(loginWidget, &Login::login_failed, loginWidget, &Login::handle_login_failed);

    connect(c, &Connection::loginResponseReceived, this, &MainWindow::handleLoginResponse);
    connect(c, &Connection::loginTimedOut, this, &MainWindow::handleLoginFailed);
    connect(c, &Connection::messageReceived, this, &MainWindow::handleIncomingMessage);




    setObjectName("mainWindow");


    topPanel->setObjectName("topPanel");
    topPanel->setFixedHeight(65);
    topPanel->setLayout(topPanelLayout);
    topPanelLayout->setContentsMargins(0, 0, 0, 0);
    topPanelLayout->setSpacing(0);
    topPanelLayout->addWidget(logo);
    topPanelLayout->addWidget(chatTitle);
    topPanelLayout->addWidget(options);

    logoLayout->addWidget(logoIcon);
    logoLayout->addWidget(logoName);
    logoLayout->setAlignment(Qt::AlignCenter);

    logo->setLayout(logoLayout);
    logo->setFixedSize(295, 65);


    logoIcon->setFixedSize(32,32);
    logoIcon->setScaledContents(true);
    logoIcon->setText("C");
    logoIcon->setAlignment(Qt::AlignCenter);
    logoIcon->setObjectName("logoIcon");


    logoName->setText("COMMUTIFY");
    logoName->setObjectName("logoName");


    chatTitle->setLayout(new QHBoxLayout());
    QLabel *chatName = new QLabel();
    chatName->setText(mainChat->chatTitle->text());
    chatName->setObjectName("activeDialogName");
    chatName->setAlignment(Qt::AlignVCenter);

    chatTitle->layout()->addWidget(chatName);
    chatTitle->setFixedSize(675, 65);


    options->setLayout(new QHBoxLayout());
    options->setObjectName("globalOptions");
    QPushButton *toggleTheme = new QPushButton(), *settings = new QPushButton(), *logOut = new QPushButton();
    toggleTheme->setIcon(toggleThemeIcon);
    toggleTheme->setIconSize(QSize(24, 24));
    toggleTheme->setFixedSize(24, 24);
    toggleTheme->setCursor(Qt::PointingHandCursor);
    settings->setIcon(settingsIcon);
    settings->setIconSize(QSize(24, 24));
    settings->setFixedSize(24, 24);
    settings->setCursor(Qt::PointingHandCursor);
    logOut->setIcon(logOutIcon);
    logOut->setIconSize(QSize(24, 24));
    logOut->setFixedSize(24, 24);
    logOut->setCursor(Qt::PointingHandCursor);

    connect(logOut, &QPushButton::clicked, this, &MainWindow::logOut);

    options->layout()->addItem(new QSpacerItem(10,10));
    options->layout()->addWidget(toggleTheme);
    options->layout()->addWidget(settings);
    options->layout()->addWidget(logOut);

    options->setFixedSize(260, 65);


    leftPanel->setLayout(leftPanelLayout);
    leftPanel->setFixedSize(285, 535);

    leftPanelLayout->addWidget(userInfoFrame);
    leftPanelLayout->addWidget(searchField);
    leftPanelLayout->addWidget(chatList);
    leftPanelLayout->setContentsMargins(5, 5, 5, 5);
    leftPanelLayout->setSpacing(5);

    userInfoFrame->setLayout(userInfoLayout);
    userInfoFrame->setFixedSize(275, 155);
    userInfoFrame->setObjectName("userInfo");

    userInfoLayout->addWidget(userPhoto, 0, Qt::AlignHCenter);
    userInfoLayout->addWidget(userName);
    userInfoLayout->addWidget(userFaculty);
    userInfoLayout->setSpacing(0);

    userPhoto->setFixedSize(75, 75);
    QPixmap p = QPixmap(":/images/resources/img/e.png").scaled(75, 75, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    userPhoto->setPixmap(p);
    userName->setAlignment(Qt::AlignCenter);
    userName->setObjectName("userName");
    userFaculty->setAlignment(Qt::AlignCenter);
    userFaculty->setObjectName("userFaculty");
    userFaculty->setTextInteractionFlags(Qt::TextSelectableByMouse);

    searchField->setPlaceholderText("Search...");
    searchField->setFixedSize(275, 35);
    searchField->setObjectName("searchField");

    chatList->setFixedSize(275, 325);
    chatList->setLayout(chatListLayout);

    chatListLayout->setContentsMargins(0, 0, 0, 0);
    chatListLayout->setSpacing(0);


    chatListLayout->addWidget(mainChat, 0, Qt::AlignTop);


    centralPanel->setLayout(centralPanelLayout);
    centralPanel->setFixedSize(665, 535);

    centralPanelLayout->addWidget(messageArea);
    centralPanelLayout->addWidget(inputFrame, 0, Qt::AlignBottom);
    centralPanelLayout->setContentsMargins(5, 5, 5, 5);
    centralPanelLayout->setSpacing(5);

    maw->setLayout(messageAreaLayout);

    messageArea->setObjectName("messageArea");
    maw->setObjectName("maWidget");
    messageArea->setWidget(maw);
    messageArea->setWidgetResizable(true);
    messageArea->setFixedWidth(655);
    messageArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    messageArea->setFrameShape(QFrame::NoFrame);
    messageArea->verticalScrollBar()->hide();


    messageAreaLayout->addStretch();


    inputFrame->setLayout(inputLayout);
    inputLayout->addWidget(addAttachment, 0, Qt::AlignBottom);
    inputLayout->addWidget(inputField, 0, Qt::AlignBottom);
    inputLayout->addWidget(sendMessageButton, 0, Qt::AlignBottom);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(5);

    addAttachment->setObjectName("attachmentButton");
    addAttachment->setFixedSize(41, 41);
    addAttachment->setCursor(Qt::PointingHandCursor);
    addAttachment->setIcon(addAttachmentIcon);

    inputField->setPlaceholderText("Write a message...");
    inputField->setFixedWidth(563);
    inputField->resize(563, 41);
    inputField->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);

    sendMessageButton->setFixedSize(41, 41);
    sendMessageButton->setObjectName("sendMessage");
    sendMessageButton->setCursor(Qt::PointingHandCursor);
    sendMessageButton->setIcon(sendButtonIcon);


    rightPanel->setLayout(rightPanelLayout);
    rightPanel->setFixedSize(250, 535);

    rightPanelLayout->addWidget(chatOptions);
    chatOptions->setObjectName("chatOptions");

    chatOptions->setLayout(new QHBoxLayout());
    QPushButton *srch = new QPushButton(), *mute = new QPushButton(), *info = new QPushButton();
    srch->setIcon(chatSearchIcon);
    srch->setIconSize(QSize(24, 24));
    srch->setFixedSize(24, 24);
    srch->setCursor(Qt::PointingHandCursor);
    mute->setIcon(muteIcon);
    mute->setIconSize(QSize(24, 24));
    mute->setFixedSize(24, 24);
    mute->setCursor(Qt::PointingHandCursor);
    info->setIcon(chatInfoIcon);
    info->setIconSize(QSize(24, 24));
    info->setFixedSize(24, 24);
    info->setCursor(Qt::PointingHandCursor);

    chatOptions->layout()->addWidget(srch);
    chatOptions->layout()->addWidget(mute);
    chatOptions->layout()->addWidget(info);
    chatOptions->layout()->setContentsMargins(0, 0, 0, 0);
    chatOptions->setFixedHeight(32);

    rightPanelLayout->addWidget(memberList);


    QVBoxLayout *memLstLay = new QVBoxLayout();
    memberList->setLayout(memLstLay);
    memberList->setObjectName("memberList");
    memLstLay->setAlignment(Qt::AlignTop);
    memLstLay->setContentsMargins(0, 0, 0, 0);
    memLstLay->setSpacing(0);

    MemberWidget *w1 = new MemberWidget(":/images/resources/img/r.png", "Rostyslav Buriak", "online 21:55"),
            *w2 = new MemberWidget(":/images/resources/img/y.png", "Yurii Brovko", "online"),
            *w3 = new MemberWidget(":/images/resources/img/e.png", "Evhen Martsinkovskyi", "online");
    memLstLay->addWidget(w3);
    w1->setProperty("online", false);
    w1->setObjectName("memberWidget");
    memLstLay->addWidget(w2);
    w2->setProperty("online", true);
    w2->setObjectName("memberWidget");
    memLstLay->addWidget(w1);
    w3->setProperty("online", true);
    w3->setObjectName("memberWidget");


    rightPanelLayout->addWidget(attachments);

    rightPanelLayout->setContentsMargins(5, 5, 5, 5);
    rightPanelLayout->setSpacing(5);


    mainLayout->addWidget(topPanel, 0, 0, 0, -1, Qt::AlignTop);
    mainLayout->addWidget(leftPanel, 1, 0, Qt::AlignLeft);
    mainLayout->addWidget(centralPanel, 1, 1, Qt::AlignBottom);
    mainLayout->addWidget(rightPanel, 1, 2, Qt::AlignRight);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);


    if (lastMsgId > 0) {
        for (int i = 1; i < lastMsgId + 1; i++) {
            QStringList s = db->select(i);
            QString name = s[1], text = s[2];
            QDateTime date = QDateTime::fromString(s[3], Qt::RFC2822Date);
            drawMessage(s[0].toInt(), name, text, date);
        }
    }

    mainWidget->setLayout(mainLayout);

    window->addWidget(loginWidget);
    window->addWidget(mainWidget);

    setLayout(window);

    setFixedSize(1200, 600);
}


void MainWindow::sendMessage() {
    QString text = inputField->toPlainText().trimmed();
    if (!text.isEmpty()) {
        QString name = me.name;
        QDateTime date = QDateTime::currentDateTime();
        drawMessage(++lastMsgId, name, text, date);
        db->insert(name, text, date);

        text.prepend("msg ");
        text.prepend(QString().setNum(text.length()) + " ");
        QVariant msgText(text);
        int sent = c->sendMessage(QByteArray(msgText.toByteArray()));
        if (sent == -1) {
            QTextStream(stderr) << "error sending message\n";
        }
    }
    inputField->clear();
    inputField->setFocus();
}


void MainWindow::openPhoto() {
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.exec();
}


extern void MainWindow::drawMessage(int mId, QString n, QString t, QDateTime d) {
    MessageBox *m = new MessageBox(mId, n, t, d.time());
    messageAreaLayout->addWidget(m);
    lastMsg = m;
    mainChat->updateMsgText(n + ": " + t, d.time().toString("hh:mm"));
    lastMsgId = db->check();
}



void MainWindow::login() {
    QTextStream(stdout) << "started login check\n";

    if (QLabel *lf = loginWidget->loginForm->findChild<QLabel*>("loginFail")) {
        loginWidget->loginFormLayout->removeWidget(lf);
        lf->deleteLater();
    }

    if (loginWidget->loginField->text().isEmpty() && loginWidget->passwordField->text().isEmpty()) {
        emit loginWidget->login_field_correct(false);
        emit loginWidget->pass_field_correct(false);
        return;
    } else if (loginWidget->loginField->text().isEmpty()) {
        emit loginWidget->login_field_correct(false);
        emit loginWidget->pass_field_correct(true);
        return;
    } else if (loginWidget->passwordField->text().isEmpty()) {
        emit loginWidget->login_field_correct(true);
        emit loginWidget->pass_field_correct(false);
        return;
    }

    if (loginWidget->loginField->text() == "demo" && loginWidget->passwordField->text() == "demo") {
        QString res = "SERVER OK Evhen Martsinkovskyi%Information Technology, first-cycle, full-time%Faculty of Applied Information Technology%6%Internet and Mobile Technologies";
        emit c->loginResponseReceived(res);
    }

    QString loginCredentials_s = "connect " + loginWidget->loginField->text() + " " + loginWidget->passwordField->text() + " " + QString().setNum(lastMsgId);
    loginCredentials_s.prepend(QString().setNum(loginCredentials_s.length()) + " ");
    QVariant loginCredentials(loginCredentials_s);
    QByteArray loginData = loginCredentials.toByteArray();


    int sent = c->sendMessage(loginData);
    if (sent == -1) {
        QTextStream(stderr) << "error sending login credentials\n";
    }

}


void MainWindow::handleLoginResponse(QString &res) {
    QTextStream(stdout) << "\nhandling login resp\n";
    if (res.startsWith("SERVER OK")) {
        QTextStream(stdout) << "\nstarted\n";
        QStringList resL = res.split('%');

        me.name = resL[0].section(' ', 2);
        me.faculty = resL[1].section(", ", 0, 0);
        me.department = resL[2];
        me.semester = resL[3].toInt();
        me.specialization = resL[4];
        userName->setText(me.name);
        userFaculty->setText(QString("%1, sem. %2").arg(me.faculty).arg(me.semester));
        loginWidget->loginField->setText("");
        loginWidget->passwordField->setText("");
        window->setCurrentWidget(mainWidget);
        inputField->setFocus();
        QTextStream(stdout) << "Login successful\n";
    }
    QTextStream(stdout) << "finished login check\n\n";
}


void MainWindow::handleLoginFailed()
{
    emit loginWidget->login_failed();
    QTextStream(stderr) << "login failed\n";
    return;
}


void MainWindow::handleIncomingMessage(QString &msg) {
    if (msg.startsWith("SERVERSYNC")) {
        QString timeStr = msg.section(' ', 2, 3);
        int nameLength = msg.section(' ', 5, 5).toInt();
        QString name = msg.section(' ', 6).left(nameLength);
        QString text = msg.right(msg.section(name, 1, 1).trimmed().section(' ', 0, 0).toInt());
        drawMessage(++lastMsgId, name, text.trimmed(), QDateTime::fromString(timeStr, "yyyy-MM-dd HH:mm:ss.zzz"));
    } else if (msg.startsWith("msg")) {
        QString timeStr = msg.section(' ', 1, 2);
        int nameLength = msg.section(' ', 4, 4).toInt();
        QString name = msg.section(' ', 5).left(nameLength);
        QString text = msg.section(name, 1, 1);
        drawMessage(++lastMsgId, name, text.trimmed(), QDateTime::fromString(timeStr, "yyyy-MM-dd HH:mm:ss.zzz"));
    } else {
        QTextStream(stdout) << "Unknown message: " << msg << '\n';
    }
}


void MainWindow::logOut() {
    c->disconnectFromServer();
    window->setCurrentWidget(loginWidget);
}


MainWindow::~MainWindow()
{
}



MemberWidget::MemberWidget(QString qrc, QString name, QString lastSeen, QWidget *parent) : QWidget(parent) {
    *memberPhotoPixmap = QPixmap(qrc).scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    memberPhoto->setPixmap(*memberPhotoPixmap);
    memberPhoto->setFixedSize(50, 50);

    memberName->setText(name);
    memberName->setStyleSheet("font-weight: 700; font-size: 13px;");
    memberName->setFixedHeight(memberName->fontMetrics().height());

    memberlastSeen->setText(lastSeen);
    memberlastSeen->setObjectName("memberLastSeen");
    memberlastSeen->setFixedHeight(memberlastSeen->fontMetrics().height());

    QVBoxLayout *memberTextLayout = new QVBoxLayout();
    memberText->setLayout(memberTextLayout);

    memberTextLayout->addWidget(memberName);
    memberTextLayout->addWidget(memberlastSeen);


    memberText->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);


    QIcon memberContextOptionsIcon(QPixmap(":/images/resources/img/userContextOptions.png"));
    memberContextOptions->setIcon(memberContextOptionsIcon);
    memberContextOptions->setFixedSize(10, 20);
    memberContextOptions->setCursor(Qt::PointingHandCursor);


    QHBoxLayout *memberLayout = new QHBoxLayout();
    memberLayout->addWidget(memberPhoto, 0, Qt::AlignLeft);
    memberLayout->addWidget(memberText, 1, Qt::AlignLeft);
    memberLayout->addWidget(memberContextOptions, 0, Qt::AlignRight);

    setLayout(memberLayout);
    setFixedSize(240, memberPhoto->height() + 10);
};

MemberWidget::~MemberWidget() {};
