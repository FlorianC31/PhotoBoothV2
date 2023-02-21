#include "printer.h"
#include <QDebug>

Printer::Printer()
{

}

void Printer::print(unsigned int nbPrint){
    qDebug() << "nbPrint:" << nbPrint;
}
