#include "utils.h"

/**
 * @brief transformColor transform #HEX color code in decimal RGB
 * @param colorCode #HEX color code
 * @param colorRGB reference to QMap with decimal value for each chanel (char 'R', 'G' and 'B')
 * @return true is no error
 */
bool transformColor(QString colorCode, QMap<char, uint>& colorRGB)
{
    QString errorMsg("The color code in settings file is incorrect");

    if (colorCode.size() != 7)
    {
        qDebug() << errorMsg;
        return false;
    }

    if (colorCode[0] != QChar('#'))
    {
        qDebug() << errorMsg;
        return false;
    }

    bool okR, okG, okB;
    colorRGB['R'] = QStringView(colorCode).mid(1,2).toUInt(&okR, 16);
    colorRGB['G'] = QStringView(colorCode).mid(3,2).toUInt(&okG, 16);
    colorRGB['B'] = QStringView(colorCode).mid(5,2).toUInt(&okB, 16);


    if (!okR || !okG ||! okB)
    {
        qDebug() << errorMsg;
        return false;
    }

    return true;

}
