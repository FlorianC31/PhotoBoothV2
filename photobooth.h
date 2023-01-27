#ifndef PhotoBooth_H
#define PhotoBooth_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class PhotoBooth; }
QT_END_NAMESPACE

class PhotoBooth : public QWidget
{
    Q_OBJECT

public:
    PhotoBooth(QWidget *parent = nullptr);
    ~PhotoBooth();

private:
    Ui::PhotoBooth *ui;
};

#endif // PhotoBooth_H
