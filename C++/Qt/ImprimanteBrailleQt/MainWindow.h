#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QWidget>
#include <QProgressBar>
#include <vector>
#include <string>

class QPushButton;
class QTextEdit;
class CameraViewOverloaded;
class QCamera;
class QLabel;
class QGridLayout;
class QRectF;
class QMediaPlayer;
class StrToBraille;
class ArduinoConnectionManager;

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    void connection();
    bool isProgramClosed() {return _is_closed;}
    bool isCameraAvailable();
private:
    void resizeEvent(QResizeEvent *);
    void wait(unsigned int milliseconds);
    std::vector<qreal> getRectSizeCoef(std::string *str);

    QPushButton              *_button;
    QTextEdit                *_txt_input;
    CameraViewOverloaded     *_viewfinder;
    QCamera                  *_camera;
    QLabel                   *_how_to_use_txt;
    QGridLayout              *_layout;
    QMediaPlayer             *_media_player;
    StrToBraille             *_stb;
    ArduinoConnectionManager *_acm;

    QRect _screen_size;

    const float _camera_height = 200,
                _tray_height   = 200,
                _tray_width    = 200,
                _char_height   = 10,
                _char_width    = 6;

    bool _is_closed;

private slots:
    void textChanged();
    void windowResizedSlot();
    void print();

signals:
    void windowResizedSignal();
};

#endif // MAINWINDOW_H
