#ifndef LOADINGSPINNER_H
#define LOADINGSPINNER_H

#include <QDialog>

namespace Ui {
class LoadingSpinner;
}

class LoadingSpinner : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingSpinner(QWidget *parent = nullptr);
    ~LoadingSpinner();

    void SetMessage(QString head, QString body);

private:

     Ui::LoadingSpinner *ui;
     QMovie *mv;
};

#endif // LOADINGSPINNER_H
