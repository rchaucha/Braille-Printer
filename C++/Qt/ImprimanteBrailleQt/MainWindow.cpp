#define _USE_MATH_DEFINES

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QCameraInfo>
#include <QCamera>
#include <QLabel>
#include <QPainter>
#include <QRect>
#include <QRectF>
#include <QtGui>
#include <QApplication>
#include <QDesktopWidget>
#include <QSizePolicy>
#include <QStyle>
#include <QTextCodec>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QInputDialog>
#include <QProgressBar>
#include <QSplashScreen>
#include <QProgressDialog>
#include <QDesktopServices>
#include <QDebug>
#include <bitset>
#include <math.h>
#include <vector>
#include <chrono>
#include <thread>
#include <wchar.h>
#include "MainWindow.h"
#include "CameraViewOverloaded.h"
#include "CameraViewOverloaded.cpp"
#include "StrToBraille.h"
#include "ArduinoConnectionManager.h"
#include "tserial.h"

using namespace std;

MainWindow::MainWindow() : QWidget(),
  _is_closed(false)
{
    _screen_size = QApplication::desktop()->screenGeometry();

    this->setWindowTitle("Imprimante Braille");
    this->resize(_screen_size.width()*0.96, _screen_size.height()*0.9); //90% de la taille de l'écran
    this->setMinimumSize(950, 516);
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, this->size(), _screen_size)); //centrer la fenêtre

    qreal button_font_size = this->width() * this->height() * 23./1440000.; //on calcule la taille de police

    if(button_font_size < 16)
        button_font_size = 16;

    QFont button_font("Arial");
    button_font.setPointSizeF(button_font_size);

    _button = new QPushButton("Imprimer", this);
    _button->setFont(button_font);
    _button->setCursor(Qt::PointingHandCursor);
    _button->move(60, 50);
    _button->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));

    _txt_input = new QTextEdit();
    _txt_input->setPlaceholderText("Entrez votre texte ici");
    _txt_input->setAcceptRichText(false);

    unsigned int tuto_font_size = floor(this->width() * this->height() * 1./207000.); //on calcule la taille de police
    if(tuto_font_size < 6)
        tuto_font_size = 6;

    _how_to_use_txt = new QLabel;
    _how_to_use_txt->setFont(QFont("Arial", tuto_font_size));
    _how_to_use_txt->setText("<font size=\"9\"><strong>Comment utiliser ce logiciel:</strong></font><br><br><br>"
        "<font size=\"6\"><b>1.</b>Entrez le texte à imprimer dans le champs ci-dessus<br><br>"
        "<b>2.</b>En tapant votre texte, un cadre apparaîtra sur le retour caméra ci-contre, et "
        "vous indiquera la zone où votre texte sera imprimé. Vérifiez bien que ce cadre "
        " ne dépasse pas de votre support, et qu'il est placé où vous le souhaitez.<br><br>"
        "<b>3.</b>Une fois votre texte prêt, appuyez sur le bouton \"Imprimer\" ci-dessus.</font>");
    _how_to_use_txt->setWordWrap(true);

    _viewfinder = new CameraViewOverloaded;

    if (isCameraAvailable())
    {
        /*QList<QCameraInfo> cameras = QCameraInfo::availableCameras();         //A utiliser pour choisir la bonne webcam nommée "braillewebcam" s'il y en a plusieurs
        foreach (const QCameraInfo &cameraInfo, cameras)
        {
            if (cameraInfo.deviceName() == "braillewebcam")
                camera = new QCamera(cameraInfo);
        }*/

        _camera = new QCamera;
        _camera->setViewfinder(_viewfinder);

        _camera->start();

        _viewfinder->setAspectRatioMode(Qt::IgnoreAspectRatio);
    }

    _layout = new QGridLayout;
    _layout->addWidget(_txt_input, 0, 0, 1, 3);
    _layout->addWidget(_button, 1, 0, 1, 1);
    _layout->addWidget(_how_to_use_txt, 2, 0, 110, 1);
    _layout->addWidget(_viewfinder, 1, 1, 120, 2);

    _layout->setColumnStretch(1, 65);   //_viewfinder width
    _layout->setRowStretch(119, 75);    //_viewfinder height
    _layout->setRowStretch(1, 25./3.);  //_button
    _layout->setRowStretch(0, 50);      //_txt_input
    _layout->setColumnStretch(0, 35);   //_how_to_use_txt

    this->setLayout(_layout);

    _media_player = new QMediaPlayer;
    _media_player->setVolume(50);

    QObject::connect(_txt_input, SIGNAL(textChanged()), this, SLOT(textChanged()));
    QObject::connect(_button, SIGNAL(clicked()), this, SLOT(print()));
    QObject::connect(this, SIGNAL(windowResizedSignal()), this, SLOT(windowResizedSlot()));
}

MainWindow::~MainWindow()
{
    delete _button;
    _button = nullptr;

    delete _txt_input;
    _txt_input = nullptr;

    delete _how_to_use_txt;
    _how_to_use_txt = nullptr;

    delete _camera;
    _camera = nullptr;

    delete _viewfinder;
    _viewfinder = nullptr;

    delete _layout;
    _layout = nullptr;

    delete _media_player;
    _media_player = nullptr;

    delete _stb;
    _stb = nullptr;

    _acm->disconnect();
    delete _acm;
    _acm = nullptr;
}

void MainWindow::connection()
{
    _stb = new StrToBraille;
    _acm = new ArduinoConnectionManager ;

    bool is_error = true;
    while(is_error)
    {
        is_error = false;

        bool is_ok_pressed = false;
        const unsigned char com_port = QInputDialog::getInt(this, "Port COM", "Entrez le numéro du port COM utilisé par la carte Arduino (ex : \"2\" pour COM2):",
                                            1, 1, 255, 1, &is_ok_pressed, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

        if(is_ok_pressed)
        {
            //////TEMPORAIRE//////

            if(com_port == 99)
                break;

            //////////////////////

            if(!_acm->connect(com_port)) //si la connection a échoué
            {
                is_error = true;
                int answer = QMessageBox::critical(this, "Erreur", "La liaison avec l'Arduino n'a pas pu être effectuée, veuillez vérifier que vous avez branché l'imprimante à l'ordinateur et que le port COM saisi est le bon", QMessageBox::Retry | QMessageBox::Help, QMessageBox::Retry);
                if(answer == QMessageBox::Help)
                    QDesktopServices::openUrl(QUrl("https://youtu.be/HJRAcZFdg4Y", QUrl::TolerantMode));
            }
            else
                _acm->sendBitSet(bitset<12>("111111111111")); //on débute la connection
        }
        else //Annuler / Croix pour fermer
        {
            int answer = QMessageBox::warning(this, "Attention", "Êtes-vous bien sûr de vouloir quitter ?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if(answer == QMessageBox::Yes)
                _is_closed = true;
            else                    //si l'utilisateur ne veut pas fermer
                is_error = true;
        }
    }
}

bool MainWindow::isCameraAvailable()
{
    if (QCameraInfo::availableCameras().count() > 0)
        return true;
    else
        return false;
}

void MainWindow::windowResizedSlot()
{
    qreal button_font_size = this->width() * this->height() * 23./1440000.; //on calcule la taille de police
    if(button_font_size < 16)
        button_font_size = 16;

    QFont button_font("Arial");
    button_font.setPointSizeF(button_font_size);

    _button->setFont(button_font);

    qreal tuto_font_size = this->width() * this->height() * 1./207000.; //on calcule la taille de police du tuto
    if(tuto_font_size < 6)
        tuto_font_size = 6;

    QFont tuto_font("Arial");
    tuto_font.setPointSizeF(tuto_font_size);

    _how_to_use_txt->setFont(tuto_font);

    textChanged(); //redimensionne le rectangle en recalculant sa taille p/r à la nouvelle fenêtre
}

void MainWindow::textChanged()
{
    QRectF *rect = new QRectF;

    string str = _txt_input->toPlainText().toStdString();

    if (!str.empty())
    {
        vector<qreal> rect_size_coef = getRectSizeCoef(&str);

        qreal width_coef = rect_size_coef[0],
                height_coef = rect_size_coef[1];

        if(height_coef >= 1)     //si le message dépasse du plateau, on le fixe à la taille du plateau
            height_coef = 0.99;

        if(width_coef >= 1)
            width_coef = 0.99;

        if((height_coef >= 0.99) || (width_coef >= 0.99))   //si le texte dépasse du plateau, on empêche l'impression
        {
            if(_button->isEnabled())
            {
                qreal font_size = _button->font().pointSizeF() * 0.85;

                if(font_size > 20)
                    font_size = 20;

                QFont button_font("Arial");
                button_font.setPointSizeF(font_size);

                _button->setEnabled(false);
                _button->setIcon(QIcon("../../files/alert_ico.png"));
                _button->setFont(button_font);
                _button->setStyleSheet("color: red;");
                _button->setText("Votre texte dépasse de la zone imprimable!");

                _media_player->setMedia(QUrl::fromLocalFile("../../files/alert.mp3"));
                _media_player->play();
            }
        }
        else if(!_button->isEnabled())      //sinon, si on avait empêché l'impression avant on la réactive
        {
            qreal font_size = this->width() * this->height() * 23./1440000.;

            QFont button_font("Arial");
            button_font.setPointSizeF(font_size);

            _button->setEnabled(true);
            _button->setIcon(QIcon(""));
            _button->setFont(button_font);
            _button->setStyleSheet("color: black;");
            _button->setText("Imprimer");
        }

        rect->setHeight(_viewfinder->height() * height_coef);
        rect->setWidth(_viewfinder->width() * width_coef);
    }
    else //si le texte est vide : ""
        rect->setSize(QSizeF(0, 0));

    _viewfinder->updateRectSize(rect);
}

void MainWindow::print()
{
    int answer = QMessageBox::warning(this, "Attention", "Êtes-vous bien sûr de vouloir imprimer le texte que vous avez saisi ?\n"
                                                         "Il sera impossible d'arrêter l'impression ensuite.", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if(answer == QMessageBox::Yes) //si l'utilisateur a validé
    {
        if(_acm->sendBitSet(bitset<12>("000010000010"))) //si la connection est toujours valable (on envoie 2 fois le code StartOfText pour vérifier)
        {
            setEnabled(false); //grey MainWindows out

            string str = _txt_input->toPlainText().toStdString();
            vector<bitset<12>> bitset_vector = _stb->strToBitsetVector(str);

            int vector_size = static_cast<int>(bitset_vector.size());

            QProgressDialog progress("Veuillez patienter le temps que votre texte soit traité...", 0, 0, vector_size - 1);
            progress.setWindowModality(Qt::WindowModal);
            progress.setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
            progress.setAutoReset(true);
            progress.setMinimumDuration(200);

            for (const bitset<12> &bitset_12 : bitset_vector)
            {
                _acm->sendBitSet(bitset_12);
                progress.setValue(progress.value() + 1);

                wait(20);
            }

            vector<char> msg_size = _stb->getNumberOfBrailleCharNeeded(str); //on calcule le nombre de caractère en braille que le texte va prendre

            char longest_line = msg_size[0],
                 lines_nbr    = msg_size[1];

            _acm->sendBitSet(bitset<12>("000100000100")); //EndOfTransmission x2

            wait(50);

            _acm->sendChar(longest_line);
            wait(10);
            _acm->sendChar(lines_nbr);

            setEnabled(true);

            QMessageBox::information(this, "Terminé", "L'impression devrait commencer dans quelques instants, l'application va se fermer.\n"
                                                      "Si vous souhaitez imprimer à nouveau, veuillez attendre la fin de l'impression, et redémarrer l'application.");
            this->close();
        }
       else
        {
            QMessageBox::critical(this, "Erreur", "La liaison avec l'Arduino n'a pas pu être effectuée, veuillez vérifier que vous n'avez pas débranché l'imprimante de l'ordinateur, puis reconnectez-vous.");
            this->connection();
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    emit(windowResizedSignal());
}

vector<qreal> MainWindow::getRectSizeCoef(string *str)
{
    vector<qreal> sizeCoef(2);

    vector<char> msg_size = _stb->getNumberOfBrailleCharNeeded(*str); //on calcule le nombre de caractère en braille que le texte va prendre

    float nbr_of_chars_on_one_line = msg_size[0],
        lines_nbr = msg_size[1];

    sizeCoef[0] = nbr_of_chars_on_one_line * 6 / _tray_width; //en réalité c'est un peu moins, mais on garde une marge de sécurité
    sizeCoef[1] = lines_nbr * 10 / _tray_height;

    return sizeCoef;
}

void MainWindow::wait(unsigned int milliseconds)
{
    this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
