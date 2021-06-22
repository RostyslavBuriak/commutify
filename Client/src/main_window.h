#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QScrollArea>
#include <QPushButton>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QFileDialog>
#include <QStackedLayout>
#include <QDebug>

#include "connection.h"
#include "database.h"
#include "dialog_box.h"
#include "input_field.h"
#include "message_box.h"
#include "scrollarea_widget.h"
#include "login.h"

class MainWindow : public QWidget
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    struct {
        QString name;
        QString faculty;
        QString department;
        QString specialization;
        int semester;
    } me;

    Login *loginWidget = new Login();
    InputField *inputField = new InputField();
    DataBase *db = new DataBase();
    ScrollAreaWidget *maw = new ScrollAreaWidget();
    MessageBox *lastMsg;
    Connection *c = new Connection("127.0.0.1", 9008);

    QPushButton *sendMessageButton = new QPushButton();
    QPushButton *addAttachment = new QPushButton();
    QStackedLayout *window = new QStackedLayout();
    QWidget *mainWidget = new QWidget();
    QGridLayout *mainLayout = new QGridLayout();
    QLineEdit *searchField = new QLineEdit();
    QFrame *userInfoFrame = new QFrame(), *topPanel = new QFrame(), *leftPanel = new QFrame(),
            *centralPanel = new QFrame(), *rightPanel = new QFrame(), *chatOptions = new QFrame(),
            *inputFrame = new QFrame(), *logo = new QFrame(), *chatTitle = new QFrame(), *options = new QFrame();
    QHBoxLayout *topPanelLayout = new QHBoxLayout(), *chatOptionsLayout = new QHBoxLayout(),
            *inputLayout = new QHBoxLayout(), *logoLayout = new QHBoxLayout();
    QVBoxLayout *leftPanelLayout = new QVBoxLayout(), *centralPanelLayout = new QVBoxLayout(),
            *rightPanelLayout = new QVBoxLayout(), *userInfoLayout = new QVBoxLayout(),
            *chatListLayout = new QVBoxLayout(), *messageAreaLayout = new QVBoxLayout();
    QLabel *logoIcon = new QLabel(), *logoName = new QLabel(), *userPhoto = new QLabel(), *userName = new QLabel(), *userFaculty = new QLabel();
    QWidget *chatList = new QWidget(), *memberList = new QWidget();
    QTabWidget *attachments = new QTabWidget();
    QScrollArea *messageArea = new QScrollArea();
    DialogBox *mainChat = new DialogBox(1, "Information Technology");


private slots:
    void openPhoto();
    void sendMessage();
    void login();
    void handleLoginResponse(QString &);
    void handleLoginFailed();
    void handleIncomingMessage(QString &);
    void logOut();

private:
    int lastMsgId = 0;
    void drawMessage(int, QString, QString, QDateTime);
};

class MemberWidget : public QWidget {
    Q_OBJECT

    QLabel *memberPhoto = new QLabel();
    QPixmap *memberPhotoPixmap = new QPixmap();
    QLabel *memberName = new QLabel();
    QLabel *memberlastSeen = new QLabel();
    QWidget *memberText = new QWidget();
    QPushButton *memberContextOptions = new QPushButton();
public:
    MemberWidget(QString qrc_path_to_photo, QString name, QString lastSeen, QWidget *parent = nullptr);
    ~MemberWidget();
};
