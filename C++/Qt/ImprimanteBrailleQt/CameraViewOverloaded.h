#ifndef CAMERAVIEWOVERLOADED_H
#define CAMERAVIEWOVERLOADED_H

#include <QCameraViewfinder>

class QRectF;
class QPainter;

class CameraViewOverloaded : public QCameraViewfinder
{
    Q_OBJECT

public:
    CameraViewOverloaded();
    ~CameraViewOverloaded();
    void updateRectSize(QRectF *new_rect);
    void paintEvent(QPaintEvent *event);

private:
    QRectF *_rect;

public slots:
};

#endif // CAMERAVIEWOVERLOADED_H
