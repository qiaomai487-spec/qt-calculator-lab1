/*
 * 演示视频的原始帧生成器。
 *
 * 在离屏（offscreen）平台下运行真实的计算器窗口，用键盘/鼠标事件把程序
 * 依次驱动到各个演示状态，每个状态抓取一张截图，并把“说明文字 + 停留时间”
 * 写入 steps.txt，最后由 tools/build_video.py 合成带字幕的 MP4。
 */

#include "../calculator.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QPushButton>
#include <QTextStream>

static QTextStream out(stdout);
static QTextStream manifest;

static Calculator *g_calc = nullptr;
static QString g_dir;
static int g_index = 0;

static void key(int k, const QString &text = QString())
{
    QKeyEvent press(QEvent::KeyPress, k, Qt::NoModifier, text);
    QApplication::sendEvent(g_calc, &press);
}

static void digit(int d)   { key(Qt::Key_0 + d, QString::number(d)); }
static void dot()          { key(Qt::Key_Period, QStringLiteral(".")); }
static void plus()         { key(Qt::Key_Plus, QStringLiteral("+")); }
static void minus()        { key(Qt::Key_Minus, QStringLiteral("-")); }
static void times()        { key(Qt::Key_Asterisk, QStringLiteral("*")); }
static void divide()       { key(Qt::Key_Slash, QStringLiteral("/")); }
static void equals()       { key(Qt::Key_Return); }
static void backspace()    { key(Qt::Key_Backspace); }
static void clearAll()     { key(Qt::Key_Escape); }

static void click(const QString &name)
{
    if (QPushButton *b = g_calc->findChild<QPushButton *>(name))
        b->click();
}

/// 记录当前界面状态，并附上说明文字与停留时间（毫秒）
static void step(const QString &caption, int holdMs)
{
    QApplication::processEvents();
    ++g_index;
    const QString file = QStringLiteral("f%1.png").arg(g_index, 4, 10, QLatin1Char('0'));
    if (!g_calc->grab().save(g_dir + QLatin1Char('/') + file))
        out << "保存失败 " << file << "\n";
    manifest << file << QLatin1Char('\t') << holdMs << QLatin1Char('\t') << caption << QLatin1Char('\n');
    manifest.flush();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    g_dir = argc > 1 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral("docs/video_frames");
    QDir().mkpath(g_dir);

    QFile manifestFile(g_dir + QStringLiteral("/steps.txt"));
    if (!manifestFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        out << "无法写入 steps.txt\n";
        return 1;
    }
    manifest.setDevice(&manifestFile);

    Calculator calc;
    // 完成布局与绘制但不真正显示窗口，保证用真实系统字体渲染
    calc.setAttribute(Qt::WA_DontShowOnScreen, true);
    calc.resize(340, 520);
    calc.show();
    QApplication::processEvents();
    g_calc = &calc;

    // ---------- 开场 ----------
    step(QStringLiteral("实验一　带键盘事件的计算器"), 4000);
    step(QStringLiteral("界面用 Qt 设计器布局：显示屏 + 4×5 按键；鼠标与键盘共用同一套处理逻辑"), 4500);

    // ---------- 一、鼠标完成带小数的运算：12.5 × 4 = 50 ----------
    click(QStringLiteral("button1"));
    step(QStringLiteral("【鼠标】点击数字 1"), 900);
    click(QStringLiteral("button2"));
    step(QStringLiteral("【鼠标】点击数字 2"), 900);
    click(QStringLiteral("buttonDot"));
    step(QStringLiteral("【鼠标】点击小数点 ."), 900);
    click(QStringLiteral("button5"));
    step(QStringLiteral("【鼠标】点击数字 5，显示屏显示 12.5"), 1500);
    click(QStringLiteral("buttonMul"));
    step(QStringLiteral("【鼠标】点击乘号 ×，上方显示运算过程“12.5 ×”"), 1600);
    click(QStringLiteral("button4"));
    step(QStringLiteral("【鼠标】点击数字 4"), 1200);
    click(QStringLiteral("buttonEquals"));
    step(QStringLiteral("【鼠标】点击 =，得到结果 12.5 × 4 = 50"), 3000);
    click(QStringLiteral("buttonClear"));
    step(QStringLiteral("【鼠标】点击 C 清除，显示屏归零，可以开始新的计算"), 2200);

    // ---------- 二、键盘完成带小数的运算：1.5 + 2.25 = 3.75 ----------
    digit(1);
    step(QStringLiteral("【键盘】输入 1"), 900);
    dot();
    step(QStringLiteral("【键盘】输入小数点 ."), 800);
    digit(5);
    step(QStringLiteral("【键盘】输入 5，显示屏显示 1.5"), 1400);
    plus();
    step(QStringLiteral("【键盘】按 +（加号），上方显示“1.5 +”"), 1500);
    digit(2);
    step(QStringLiteral("【键盘】输入 2"), 800);
    dot();
    step(QStringLiteral("【键盘】输入小数点 ."), 800);
    digit(2);
    digit(5);
    step(QStringLiteral("【键盘】输入 2.25"), 1500);
    equals();
    step(QStringLiteral("【键盘】按 Enter（等于），得到 1.5 + 2.25 = 3.75"), 3000);

    // ---------- 三、退格 ----------
    clearAll();
    step(QStringLiteral("按 C 清除，准备演示退格"), 1600);
    digit(1); digit(2); digit(3); digit(4);
    step(QStringLiteral("输入 1234"), 1400);
    backspace();
    step(QStringLiteral("【键盘】按 Backspace 退格：1234 → 123"), 2000);
    click(QStringLiteral("buttonBackspace"));
    step(QStringLiteral("【鼠标】点击 ⌫ 按钮退格：123 → 12（与键盘效果一致）"), 2400);
    click(QStringLiteral("buttonBackspace"));
    step(QStringLiteral("【鼠标】再点一次 ⌫：12 → 1"), 2000);
    clearAll();
    step(QStringLiteral("【键盘】按 C 清除，显示屏回到 0，可以重新计算"), 2000);

    // ---------- 四、除数为 0 ----------
    digit(5);
    step(QStringLiteral("【键盘】输入 5"), 900);
    divide();
    step(QStringLiteral("【键盘】按 /（除号）"), 1200);
    digit(0);
    step(QStringLiteral("【键盘】输入 0"), 900);
    equals();
    step(QStringLiteral("【键盘】按 Enter：5 ÷ 0，显示屏显示“错误”，并提示“除数不能为 0”"), 3500);
    digit(7);
    step(QStringLiteral("【键盘】出错后直接输入 7，自动开始新的计算"), 2500);

    // ---------- 五、连续运算 ----------
    clearAll();
    step(QStringLiteral("【键盘】按 C 清除，演示连续运算"), 1400);
    digit(1); plus();
    step(QStringLiteral("【键盘】1 +"), 1100);
    digit(2); plus();
    step(QStringLiteral("【键盘】1 + 2 +　（中间结果 3 已显示）"), 1600);
    digit(3); plus();
    step(QStringLiteral("【键盘】1 + 2 + 3 +　（中间结果 6）"), 1600);
    digit(4); equals();
    step(QStringLiteral("【键盘】按 Enter：连续运算 1 + 2 + 3 + 4 = 10"), 3000);

    // ---------- 六、异常输入一：连续小数点 ----------
    clearAll();
    step(QStringLiteral("【键盘】按 C 清除，演示异常输入：连续小数点"), 1500);
    digit(1);
    step(QStringLiteral("【键盘】输入 1"), 800);
    dot();
    step(QStringLiteral("【键盘】按小数点 ."), 800);
    dot();
    step(QStringLiteral("【键盘】再按小数点：被忽略，同一操作数只保留一个小数点"), 2200);
    dot();
    step(QStringLiteral("【键盘】第三次按小数点：仍被忽略"), 1600);
    digit(2);
    step(QStringLiteral("【键盘】输入 2：1...2 最终得到 1.2（而不是报错或变成 0）"), 2800);

    // ---------- 七、异常输入二：连续运算符 ----------
    clearAll();
    digit(5);
    step(QStringLiteral("【键盘】按 C 清除后输入 5"), 1200);
    plus();
    step(QStringLiteral("【键盘】按 +（加号）"), 1100);
    minus();
    step(QStringLiteral("【键盘】紧接着按 −（减号）：只替换运算符，不会出错"), 2200);
    times();
    step(QStringLiteral("【键盘】紧接着按 ×（乘号）：继续替换运算符"), 2200);
    digit(3);
    step(QStringLiteral("【键盘】输入 3"), 900);
    equals();
    step(QStringLiteral("【键盘】按 Enter：5 + − × 3，最终按最后一个运算符计算，得到 15"), 3200);

    // ---------- 结束 ----------
    clearAll();
    step(QStringLiteral("演示结束：鼠标与键盘结果一致，连续小数点、连续运算符、除零均已正确处理"), 4000);

    out << "共生成 " << g_index << " 帧\n";
    out.flush();
    return 0;
}
