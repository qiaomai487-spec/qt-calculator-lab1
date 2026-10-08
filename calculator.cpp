#include "calculator.h"
#include "ui_calculator.h"

#include <QFile>
#include <QKeyEvent>
#include <QLineEdit>
#include <QList>
#include <QPushButton>

namespace {

/// 把 double 结果格式化成适合显示的字符串（最多 15 位有效数字）
QString formatNumber(double value)
{
    QString text = QString::number(value, 'g', 15);
    if (text == QLatin1String("-0"))
        text = QStringLiteral("0");
    return text;
}

} // namespace

Calculator::Calculator(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Calculator)
{
    ui->setupUi(this);

    // 从资源文件加载样式表，统一美化界面
    QFile styleFile(QStringLiteral(":/style.qss"));
    if (styleFile.open(QFile::ReadOnly | QFile::Text))
        setStyleSheet(QString::fromUtf8(styleFile.readAll()));

    // 显示屏只用于展示数据，不接收焦点，保证键盘事件交给主窗口处理
    ui->display->setReadOnly(true);
    ui->display->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->display->setFocusPolicy(Qt::NoFocus);

    // 让所有按钮都不抢占焦点，这样点击按钮后键盘事件仍然由主窗口接收
    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (QPushButton *button : buttons)
        button->setFocusPolicy(Qt::NoFocus);

    setFocusPolicy(Qt::StrongFocus);

    initConnections();
    clearAll();
}

Calculator::~Calculator()
{
    delete ui;
}

void Calculator::initConnections()
{
    // 数字键 0-9
    for (int i = 0; i <= 9; ++i) {
        const QString name = QStringLiteral("button%1").arg(i);
        if (QPushButton *button = findChild<QPushButton *>(name)) {
            connect(button, &QPushButton::clicked, this,
                    [this, i] { enterDigit(QString::number(i)); });
        }
    }

    // 其余功能键：鼠标点击后同样调用内部处理函数
    auto bind = [this](const QString &name, auto slot) {
        if (QPushButton *button = findChild<QPushButton *>(name))
            connect(button, &QPushButton::clicked, this, slot);
    };

    bind(QStringLiteral("buttonDot"),       [this] { enterDot(); });
    bind(QStringLiteral("buttonAdd"),       [this] { applyOperator(Op::Add); });
    bind(QStringLiteral("buttonSub"),       [this] { applyOperator(Op::Subtract); });
    bind(QStringLiteral("buttonMul"),       [this] { applyOperator(Op::Multiply); });
    bind(QStringLiteral("buttonDiv"),       [this] { applyOperator(Op::Divide); });
    bind(QStringLiteral("buttonEquals"),    [this] { evaluate(); });
    bind(QStringLiteral("buttonClear"),     [this] { clearAll(); });
    bind(QStringLiteral("buttonBackspace"), [this] { backspace(); });
    bind(QStringLiteral("buttonSign"),      [this] { toggleSign(); });
    bind(QStringLiteral("buttonPercent"),   [this] { applyPercent(); });
}

void Calculator::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();
    const QString text = event->text();

    // 1) 数字 0-9（主键盘区和小键盘区都会被映射到 Qt::Key_0..Key_9）
    if (text.size() == 1 && text.at(0).isDigit()) {
        enterDigit(text);
        event->accept();
        return;
    }

    // 2) 其余按键：统一转成与鼠标相同的处理函数
    switch (key) {
    case Qt::Key_Period:      // 小数点（主键盘和小键盘）
    case Qt::Key_Comma:       // 部分键盘布局的小数点
        enterDot();
        break;
    case Qt::Key_Plus:
        applyOperator(Op::Add);
        break;
    case Qt::Key_Minus:
        applyOperator(Op::Subtract);
        break;
    case Qt::Key_Asterisk:    // '*' 乘号
    case Qt::Key_X:           // 'x' 也作为乘号
        applyOperator(Op::Multiply);
        break;
    case Qt::Key_Slash:       // '/' 除号
        applyOperator(Op::Divide);
        break;
    case Qt::Key_Equal:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        evaluate();
        break;
    case Qt::Key_Backspace:
        backspace();
        break;
    case Qt::Key_Escape:
    case Qt::Key_Delete:
    case Qt::Key_C:
        clearAll();
        break;
    case Qt::Key_Percent:
        applyPercent();
        break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }

    event->accept();
}

void Calculator::enterDigit(const QString &digit)
{
    if (m_freshOperand) {
        if (m_pendingOp == Op::None)
            m_accumulator = 0.0;      // 上一步的结果不再参与新的计算
        setDisplayText(digit);        // 用新数字替换显示屏内容
        m_freshOperand = false;
        return;
    }

    QString text = ui->display->text();
    if (text == QLatin1String("0"))
        text = digit;                 // 避免出现 "0123" 这类前导 0
    else
        text += digit;

    if (text.length() > kMaxInputLength)
        return;

    setDisplayText(text);
}

void Calculator::enterDot()
{
    if (m_freshOperand) {
        if (m_pendingOp == Op::None)
            m_accumulator = 0.0;
        setDisplayText(QStringLiteral("0."));
        m_freshOperand = false;
        return;
    }

    setDisplayText(ui->display->text() + QLatin1Char('.'));
}

void Calculator::applyOperator(Op op)
{
    const double value = currentValue();

    if (m_pendingOp == Op::None) {
        m_accumulator = value;        // 记下第一个操作数
    } else {
        const double result = compute(m_accumulator, value, m_pendingOp);
        m_accumulator = result;       // 连续运算：把中间结果作为新的第一个操作数
        setDisplayText(formatNumber(result));
    }

    m_pendingOp = op;
    m_freshOperand = true;            // 接下来等待输入第二个操作数
}

void Calculator::evaluate()
{
    const double value = currentValue();

    if (m_pendingOp == Op::None) {
        // 没有待处理的运算符：直接把当前显示作为结果
        setDisplayText(formatNumber(value));
        m_freshOperand = true;
        return;
    }

    const double result = compute(m_accumulator, value, m_pendingOp);
    setDisplayText(formatNumber(result));

    m_accumulator = result;
    m_pendingOp = Op::None;
    m_freshOperand = true;            // 计算完成后继续输入数字将开始新的计算
}

void Calculator::backspace()
{
    QString text = ui->display->text();
    text.chop(1);
    if (text.isEmpty())
        text = QStringLiteral("0");
    setDisplayText(text);
}

void Calculator::clearAll()
{
    m_accumulator = 0.0;
    m_pendingOp = Op::None;
    m_freshOperand = true;
    setDisplayText(QStringLiteral("0"));
}

void Calculator::toggleSign()
{
    QString text = ui->display->text();
    if (text == QLatin1String("0"))
        return;

    if (text.startsWith(QLatin1Char('-')))
        text.remove(0, 1);
    else
        text.prepend(QLatin1Char('-'));

    setDisplayText(text);
    m_freshOperand = false;
}

void Calculator::applyPercent()
{
    setDisplayText(formatNumber(currentValue() / 100.0));
    m_freshOperand = false;
}

void Calculator::setDisplayText(const QString &text)
{
    ui->display->setText(text);
    ui->display->setCursorPosition(ui->display->text().size());
}

double Calculator::currentValue() const
{
    bool ok = false;
    const double value = ui->display->text().toDouble(&ok);
    return ok ? value : 0.0;
}

double Calculator::compute(double lhs, double rhs, Op op)
{
    switch (op) {
    case Op::Add:      return lhs + rhs;
    case Op::Subtract: return lhs - rhs;
    case Op::Multiply: return lhs * rhs;
    case Op::Divide:   return lhs / rhs;
    case Op::None:     break;
    }
    return rhs;
}
