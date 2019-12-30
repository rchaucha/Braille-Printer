#include "CameraViewOverloaded.h"
#include <QtGui>
#include <QRectF>
#include <QPainter>

CameraViewOverloaded::CameraViewOverloaded()
{
    _rect = new QRectF(0, 0, 0, 0);
}

CameraViewOverloaded::~CameraViewOverloaded()
{
    delete _rect;
    _rect = nullptr;
}

void CameraViewOverloaded::updateRectSize(QRectF *new_rect)
{
    new_rect->moveTo((this->width() - new_rect->width()) / 2, (this->height() - new_rect->height()) / 2);

    _rect = new_rect;

    update();

    new_rect = nullptr;
}

void CameraViewOverloaded::paintEvent(QPaintEvent *event)
{
    QCameraViewfinder::paintEvent(event);

    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 3));
    painter.setRenderHint(QPainter::Antialiasing);

    painter.drawRect(*_rect);
}
