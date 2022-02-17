#ifndef TRANSLATORWINDOW_H
#define TRANSLATORWINDOW_H

#include "lexer.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class TranslatorWindow; }
QT_END_NAMESPACE

class TranslatorWindow : public QMainWindow
{
    Q_OBJECT

public:
    TranslatorWindow(QWidget *parent = nullptr);
    void Fill_BNF_Form();
    ~TranslatorWindow();

private slots:
    void on_Run_Button_clicked();

private:
    Ui::TranslatorWindow *ui;
    Lexer lx;
};
#endif // TRANSLATORWINDOW_H
