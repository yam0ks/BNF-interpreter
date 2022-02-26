#include "translatorwindow.h"
#include "ui_translatorwindow.h"

TranslatorWindow::TranslatorWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TranslatorWindow)
{
    ui->setupUi(this);
    Fill_BNF_Form();
}

void TranslatorWindow::Fill_BNF_Form()
{
  ui->BNF_TextBrowser->setText("Язык = \"Start\" Звено \",\" ... Звено Опер \";\" ... Опер \"Stop\"\n"
                               "Звено = \"Первое\" Вещ ! \"Второе\" Вещ ... Вещ ! \"Third\" Цел ! \"Fourth\" Цел \",\" ... Цел\n"
                               "Опер = </ Метка ... Метка\":\" /> Перем \"=\" Пр. ч.\n"
                               "Пр. ч. = </\"-\"/> Блок зн1 ... Блок\n"
                               "зн1 = \"+\" ! \"-\"\n"
                               "Блок = Часть зн2 ... Часть\n"
                               "зн2 = \"*\" ! \"/\"\n"
                               "Часть = Кусок зн3 ... Кусок\n"
                               "зн3 = \"∨\" ! \"∧\"\n"
                               "Кусок = </ \"¬\" /> Кусочек\n"
                               "Кусочек = </ Фун ... Фун /> Частица\n"
                               "Фун = \"cos\" ! \"sin\" ! \"tg\" ! \"ctg\"\n"
                               "Частица = Перем ! Вещ\n"
                               "Метка = Цел\n"
                               "Вещ = Цел \".\" Цел\n"
                               "Цел = Цифр ... Цифр\n"
                               "Перем = Бук </ [Бук ! Цифр] ... [Бук ! Цифр] />\n"
                               "Бук = \"А\" ! \"Б\" ! ... ! \"Я\" ! \"A\" ! \"B\" ! ... ! \"Z\"\n"
                               "Цифр = \"Ø\" ! \"1\" ! ... ! \"7\"");
}

TranslatorWindow::~TranslatorWindow()
{
    delete ui;
}

void TranslatorWindow::on_Run_Button_clicked()
{
    ui->Output_TextBrowser->clear();

    auto [first, second, error] = in.InterpretCode(ui->Programm_TextEdit->toPlainText());

    if(error != QString()){
        emit ui->Programm_TextEdit->errorHappens(first, second);
        ui->Output_TextBrowser->setTextColor(Qt::red);
        ui->Output_TextBrowser->setText(error);
    }
    else{
        ui->Output_TextBrowser->setTextColor(Qt::black);
        ui->Output_TextBrowser->setText(in.OutputValues());
    }
}
