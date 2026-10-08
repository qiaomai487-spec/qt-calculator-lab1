#include "calculator.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Calculator"));
    QApplication::setApplicationDisplayName(QStringLiteral("计算器"));

    Calculator window;
    window.show();

    return app.exec();
}
