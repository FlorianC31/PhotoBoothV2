#include "PhotoBooth.h"
#include "ui_PhotoBooth.h"

PhotoBooth::PhotoBooth(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PhotoBooth)
{
    ui->setupUi(this);
}

PhotoBooth::~PhotoBooth()
{
    delete ui;
}


