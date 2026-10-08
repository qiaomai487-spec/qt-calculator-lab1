/*
 * 生成实验报告用的运行截图：在离屏（offscreen）平台下运行真实界面，
 * 通过发送键盘/鼠标事件把程序驱动到指定状态，再用 QWidget::grab() 保存 PNG。
 */

#include "../calculator.h"

#include <QApplication>
#include <QDir>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QTextStream>

static QTextStream out(stdout);

static void key(Calculator *c, int k, const QString &text = QString())
{
    QKeyEvent press(QEvent::KeyPress, k, Qt::NoModifier, text);
    QApplication::sendEvent(c, &press);
}

static void digit(Calculator *c, int d) { key(c, Qt::Key_0 + d, QString::number(d)); }
static void dot(Calculator *c)      { key(c, Qt::Key_Period, QStringLiteral(".")); }
static void plus(Calculator *c)     { key(c, Qt::Key_Plus, QStringLiteral("+")); }
static void times(Calculator *c)    { key(c, Qt::Key_Asterisk, QStringLiteral("*")); }
static void divide(Calculator *c)   { key(c, Qt::Key_Slash, QStringLiteral("/")); }
static void equals(Calculator *c)   { key(c, Qt::Key_Return, QString()); }
static void backspace(Calculator *c){ key(c, Qt::Key_Backspace, QString()); }

static void click(Calculator *c, const QString &name)
{
    if (QPushButton *b = c->findChild<QPushButton *>(name))
        b->click();
}

static void typeDigits(Calculator *c, const QString &digits)
{
    for (const QChar ch : digits)
        digit(c, ch.digitValue());
}

static void shot(Calculator *c, const QString &dir, const QString &file)
{
    QApplication::processEvents();
    const QPixmap pm = c->grab();
    // 放大 2 倍并平滑缩放，方便在实验报告中查看
    const QPixmap scaled = pm.scaled(pm.size() * 2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    const QString path = dir + QLatin1Char('/') + file;
    if (scaled.save(path))
        out << "已保存 " << path << "\n";
    else
        out << "保存失败 " << path << "\n";
}

static Calculator *makeCalculator()
{
    auto *c = new Calculator;
    // 让窗口完成布局与绘制，但不真正显示到屏幕上；
    // 这样既能拿到真实的系统字体渲染，又不会弹出窗口。
    c->setAttribute(Qt::WA_DontShowOnScreen, true);
    c->resize(340, 520);
    c->show();
    QApplication::processEvents();
    return c;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const QString dir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral("docs/screenshots");
    QDir().mkpath(dir);

    // 1. 初始界面：鼠标操作与键盘操作共用同一套逻辑，界面完整
    {
        Calculator *c = makeCalculator();
        shot(c, dir, QStringLiteral("01_初始界面.png"));
        delete c;
    }

    // 2. 小数输入：显示屏实时显示输入的数据
    {
        Calculator *c = makeCalculator();
        typeDigits(c, QStringLiteral("12"));
        dot(c);
        digit(c, 5);
        shot(c, dir, QStringLiteral("02_小数输入.png"));
        delete c;
    }

    // 3. 按下乘号：上方显示运算过程“12.5 ×”
    {
        Calculator *c = makeCalculator();
        typeDigits(c, QStringLiteral("12"));
        dot(c);
        digit(c, 5);
        times(c);
        shot(c, dir, QStringLiteral("03_运算过程.png"));
        delete c;
    }

    // 4. 得出结果：12.5 × 4 = 50
    {
        Calculator *c = makeCalculator();
        typeDigits(c, QStringLiteral("12"));
        dot(c);
        digit(c, 5);
        times(c);
        digit(c, 4);
        equals(c);
        shot(c, dir, QStringLiteral("04_计算结果.png"));
        delete c;
    }

    // 5. 输入过程中退格：12.34 -> 12.3
    {
        Calculator *c = makeCalculator();
        typeDigits(c, QStringLiteral("12"));
        dot(c);
        typeDigits(c, QStringLiteral("34"));
        backspace(c);
        shot(c, dir, QStringLiteral("05_退格.png"));
        delete c;
    }

    // 6. 除数为 0：给出错误提示
    {
        Calculator *c = makeCalculator();
        digit(c, 5);
        divide(c);
        digit(c, 0);
        equals(c);
        shot(c, dir, QStringLiteral("06_除零错误.png"));
        delete c;
    }

    // 7. 连续运算：1 + 2 + 3 + 4 = 10
    {
        Calculator *c = makeCalculator();
        digit(c, 1); plus(c);
        digit(c, 2); plus(c);
        digit(c, 3); plus(c);
        digit(c, 4); equals(c);
        shot(c, dir, QStringLiteral("07_连续运算.png"));
        delete c;
    }

    // 8. 纯键盘输入：6 * 7 = 42
    {
        Calculator *c = makeCalculator();
        digit(c, 6); times(c); digit(c, 7); equals(c);
        shot(c, dir, QStringLiteral("08_键盘输入.png"));
        delete c;
    }

    // 9. 鼠标点击输入：与键盘结果一致
    {
        Calculator *c = makeCalculator();
        click(c, QStringLiteral("button6"));
        click(c, QStringLiteral("buttonMul"));
        click(c, QStringLiteral("button7"));
        click(c, QStringLiteral("buttonEquals"));
        shot(c, dir, QStringLiteral("09_鼠标输入.png"));
        delete c;
    }

    out.flush();
    return 0;
}
