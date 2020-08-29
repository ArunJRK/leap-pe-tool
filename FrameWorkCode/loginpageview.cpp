#include "loginpageview.h"
#include "ui_loginpageview.h"
#include "mainwindow.h"
<<<<<<< HEAD
=======

>>>>>>> ed347a18f3df975cd1075d9bc56eb722ecbefcc1
LoginPageView::LoginPageView(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginPageView)
{
    ui->setupUi(this);
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
}

LoginPageView::~LoginPageView()
{
    delete ui;
}


void LoginPageView::on_loginButton_clicked()
{
<<<<<<< HEAD
    QString usernameString= ui->usernameEdit->text();
    QString passwordString= ui->passwordEdit->text();
    //ui->usernameEdit->clear();
    ui->passwordEdit->clear();

    //QString projectId = ""; //should be added in project.xml file and fetched from there
    QString userId, userRole;  //fetch on sign in   //add location
    QString userJsonFilePath = QDir::currentPath() + "/UserJson.json";  //add location


    //User JSON
    QFile userJsonFile(userJsonFilePath);
    userJsonFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray userData = userJsonFile.readAll();

    QJsonDocument userDocument = QJsonDocument::fromJson(userData);
    QJsonObject userObj = userDocument.object();
    if(!userObj.value(usernameString).isUndefined()) {
        QJsonObject user = userObj.value(usernameString).toObject();
//      QString username = user["github userid"].toString();
        QString password =user["github password"].toString();
        if(password==passwordString){
            userId=usernameString;
            userRole = user["Role"].toString();
            userJsonFile.close();
            QMessageBox::information(this,"Login","Login Succesful!");
            hide();
            mwindow =new MainWindow(this);
            mwindow->setRole(userRole);
            mwindow->show();
        } else {
            userJsonFile.close();
            QMessageBox::information(this,"Login","Login Un-succesful!\nIncorrent Password");
        }
    } else {
        userJsonFile.close();
        QMessageBox::information(this,"Login","Login Un-succesful!\nUsername not found");
    }
=======
            QString usernameString= ui->usernameEdit->text();
            QString passwordString= ui->passwordEdit->text();
            ui->usernameEdit->clear();
            ui->passwordEdit->clear();

            //QString projectId = ""; //should be added in project.xml file and fetched from there
            QString userId, userRole;  //fetch on sign in   //add location
            QString userJsonFilePath = "UserJson.json";  //add location


            //User JSON
            QFile userJsonFile(userJsonFilePath);
            userJsonFile.open(QIODevice::ReadOnly | QIODevice::Text);
            QByteArray userData = userJsonFile.readAll();

            QJsonDocument userDocument = QJsonDocument::fromJson(userData);
            QJsonObject userObj = userDocument.object();

            QJsonObject user = userObj.value(usernameString).toObject();
            QString username = user["github userid"].toString();
            QString password =user["github password"].toString();


            if(username==usernameString && password==passwordString){
                userId=username;
                userRole = user["Role"].toString();
                userJsonFile.close();
                if(userRole=="Verifier"){
                QMessageBox::information(this,"Login","Login Succesful!");
                hide();
                mwindow =new MainWindow(this);
                mwindow->show();
                }
            }
            else{
                userJsonFile.close();
                QMessageBox::information(this,"Login","Login Un-succesful!\nPlease Check Login Credentials!");
            }
>>>>>>> ed347a18f3df975cd1075d9bc56eb722ecbefcc1
}
