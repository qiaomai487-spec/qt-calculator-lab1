/*
 * 计算器行为测试（不需要 QtTest 模块，直接发送鼠标点击/键盘事件并检查显示结果）。
 *
 * 覆盖：四则运算、小数、连续小数点、连续运算符、退格、清除、除零、
 *       计算完成后继续输入、连续运算，以及“鼠标与键盘结果一致”。
 */

#include "../calculator.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QTextStream>

static int g_passed = 0;
static int g_failed = 0;
static QTextStream out(stdout);

static QLineEdit *screen(Calculator *c)
{
    return c->findChild<QLineEdit *>(QStringLiteral("display"));
}

static QLabel *expression(Calculator *c)
{
    return c->findChild<QLabel *>(QStringLiteral("expressionLabel"));
}

static void check(Calculator *c, const QString &name, const QString &expected)
{
    const QString actual = screen(c)->text();
    if (actual == expected) {
        ++g_passed;
        out << "  [通过] " << name << "  ->  " << actual << "\n";
    } else {
        ++g_failed;
        out << "  [失败] " << name << "  期望 " << expected << "，实际 " << actual << "\n";
    }
}

// ---- 键盘事件 ----
static void key(Calculator *c, int k, const QString &text = QString())
{
    QKeyEvent press(QEvent::KeyPress, k, Qt::NoModifier, text);
    QApplication::sendEvent(c, &press);
}

static void digit(Calculator *c, int d)
{
    key(c, Qt::Key_0 + d, QString::number(d));
}

static void dot(Calculator *c)      { key(c, Qt::Key_Period, QStringLiteral(".")); }
static void plus(Calculator *c)     { key(c, Qt::Key_Plus, QStringLiteral("+")); }
static void minus(Calculator *c)    { key(c, Qt::Key_Minus, QStringLiteral("-")); }
static void times(Calculator *c)    { key(c, Qt::Key_Asterisk, QStringLiteral("*")); }
static void divide(Calculator *c)   { key(c, Qt::Key_Slash, QStringLiteral("/")); }
static void equals(Calculator *c)   { key(c, Qt::Key_Return, QString()); }
static void backspace(Calculator *c){ key(c, Qt::Key_Backspace, QString()); }
static void clearKey(Calculator *c) { key(c, Qt::Key_Escape, QString()); }

// ---- 鼠标事件 ----
static void click(Calculator *c, const QString &name)
{
    if (QPushButton *b = c->findChild<QPushButton *>(name))
        b->click();
    else
        out << "  [警告] 找不到按钮 " << name << "\n";
}

static void typeDigits(Calculator *c, const QString &digits)
{
    for (const QChar ch : digits)
        digit(c, ch.digitValue());
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    {
        out << "用例 1：基本四则运算（键盘输入）\n";
        Calculator c;
        typeDigits(&c, QStringLiteral("12")); plus(&c); typeDigits(&c, QStringLiteral("34")); equals(&c);
        check(&c, QStringLiteral("12 + 34 = 46"), QStringLiteral("46"));

        clearKey(&c);
        typeDigits(&c, QStringLiteral("7")); minus(&c); typeDigits(&c, QStringLiteral("10")); equals(&c);
        check(&c, QStringLiteral("7 − 10 = -3"), QStringLiteral("-3"));

        clearKey(&c);
        typeDigits(&c, QStringLiteral("6")); times(&c); typeDigits(&c, QStringLiteral("7")); equals(&c);
        check(&c, QStringLiteral("6 × 7 = 42"), QStringLiteral("42"));

        clearKey(&c);
        typeDigits(&c, QStringLiteral("8")); divide(&c); typeDigits(&c, QStringLiteral("2")); equals(&c);
        check(&c, QStringLiteral("8 ÷ 2 = 4"), QStringLiteral("4"));
    }

    {
        out << "用例 2：小数输入与浮点误差处理\n";
        Calculator c;
        typeDigits(&c, QStringLiteral("1")); dot(&c); digit(&c, 5);
        plus(&c);
        typeDigits(&c, QStringLiteral("2")); dot(&c); typeDigits(&c, QStringLiteral("25"));
        equals(&c);
        check(&c, QStringLiteral("1.5 + 2.25 = 3.75"), QStringLiteral("3.75"));

        clearKey(&c);
        digit(&c, 0); dot(&c); digit(&c, 1);
        plus(&c);
        digit(&c, 0); dot(&c); digit(&c, 2);
        equals(&c);
        check(&c, QStringLiteral("0.1 + 0.2 = 0.3（避免 0.30000000000000004）"), QStringLiteral("0.3"));
    }

    {
        out << "用例 3：连续小数点异常输入\n";
        Calculator c;
        digit(&c, 1); dot(&c); dot(&c); dot(&c); digit(&c, 2);
        check(&c, QStringLiteral("输入 1...2 只保留一个小数点"), QStringLiteral("1.2"));

        clearKey(&c);
        dot(&c); digit(&c, 5);
        check(&c, QStringLiteral("直接按小数点自动补 0"), QStringLiteral("0.5"));
    }

    {
        out << "用例 4：连续运算符异常输入（只替换运算符）\n";
        Calculator c;
        digit(&c, 5); plus(&c); minus(&c); times(&c); digit(&c, 3); equals(&c);
        check(&c, QStringLiteral("5 + − × 3 =（最后一个有效运算符生效）"), QStringLiteral("15"));
    }

    {
        out << "用例 5：输入过程中退格\n";
        Calculator c;
        typeDigits(&c, QStringLiteral("123"));
        backspace(&c);
        check(&c, QStringLiteral("123 退格一次 -> 12"), QStringLiteral("12"));
        backspace(&c); backspace(&c);
        check(&c, QStringLiteral("继续退格到空 -> 0"), QStringLiteral("0"));
        typeDigits(&c, QStringLiteral("9")); plus(&c); digit(&c, 1); equals(&c);
        check(&c, QStringLiteral("退格清空后重新计算 9 + 1 = 10"), QStringLiteral("10"));
    }

    {
        out << "用例 6：连续运算\n";
        Calculator c;
        digit(&c, 1); plus(&c); digit(&c, 2); plus(&c); digit(&c, 3); plus(&c);
        digit(&c, 4); equals(&c);
        check(&c, QStringLiteral("1 + 2 + 3 + 4 = 10"), QStringLiteral("10"));
    }

    {
        out << "用例 7：除数为 0\n";
        Calculator c;
        digit(&c, 5); divide(&c); digit(&c, 0); equals(&c);
        check(&c, QStringLiteral("5 ÷ 0 显示错误提示"), QStringLiteral("错误"));
        digit(&c, 7);
        check(&c, QStringLiteral("除零后输入 7 自动开始新计算"), QStringLiteral("7"));
        plus(&c); digit(&c, 3); equals(&c);
        check(&c, QStringLiteral("错误恢复后 7 + 3 = 10"), QStringLiteral("10"));
    }

    {
        out << "用例 8：计算完成后继续输入\n";
        Calculator c;
        digit(&c, 2); plus(&c); digit(&c, 3); equals(&c);
        check(&c, QStringLiteral("2 + 3 = 5"), QStringLiteral("5"));
        typeDigits(&c, QStringLiteral("40"));
        check(&c, QStringLiteral("按 = 后输入 4 0 -> 开始新的数字 40"), QStringLiteral("40"));
        plus(&c); digit(&c, 2); equals(&c);
        check(&c, QStringLiteral("40 + 2 = 42"), QStringLiteral("42"));
    }

    {
        out << "用例 9：等于后接着按运算符继续累计\n";
        Calculator c;
        digit(&c, 2); plus(&c); digit(&c, 3); equals(&c);   // 5
        times(&c); digit(&c, 4); equals(&c);                // 20
        check(&c, QStringLiteral("2 + 3 = 后 × 4 = 20"), QStringLiteral("20"));
    }

    {
        out << "用例 10：鼠标与键盘混合输入，结果一致\n";
        Calculator mouseCalc;
        click(&mouseCalc, QStringLiteral("button1"));
        click(&mouseCalc, QStringLiteral("button2"));
        click(&mouseCalc, QStringLiteral("buttonDot"));
        click(&mouseCalc, QStringLiteral("button5"));
        click(&mouseCalc, QStringLiteral("buttonMul"));
        click(&mouseCalc, QStringLiteral("button4"));
        click(&mouseCalc, QStringLiteral("buttonEquals"));
        check(&mouseCalc, QStringLiteral("鼠标：12.5 × 4 = 50"), QStringLiteral("50"));

        click(&mouseCalc, QStringLiteral("buttonClear"));
        click(&mouseCalc, QStringLiteral("button9"));      // 鼠标
        plus(&mouseCalc);                                  // 键盘
        click(&mouseCalc, QStringLiteral("button3"));      // 鼠标
        equals(&mouseCalc);                                // 键盘
        check(&mouseCalc, QStringLiteral("混合：鼠标 9 + 键盘 + + 鼠标 3 + 键盘 Enter = 12"),
              QStringLiteral("12"));
    }

    {
        out << "用例 11：退格后再输入小数与运算\n";
        Calculator c;
        typeDigits(&c, QStringLiteral("12"));
        dot(&c);
        typeDigits(&c, QStringLiteral("34"));
        backspace(&c);                    // 12.3
        plus(&c);
        digit(&c, 1); equals(&c);
        check(&c, QStringLiteral("12.34 退格成 12.3，再 +1 = 13.3"), QStringLiteral("13.3"));
    }

    out << "\n==================== 测试结果 ====================\n";
    out << "通过：" << g_passed << "  失败：" << g_failed << "\n";
    out.flush();

    return g_failed == 0 ? 0 : 1;
}
