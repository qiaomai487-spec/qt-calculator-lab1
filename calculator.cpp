#include "calculator.h"
#include "ui_calculator.h"

#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPushButton>
#include <QtGlobal>

namespace {

/**
 * 把 double 结果格式化成适合显示屏的字符串。
 * 使用 15 位有效数字既能自动去掉多余的小数 0，又能掩盖二进制浮点误差
 * （例如 0.1 + 0.2 会显示为 0.3，而不是 0.30000000000000004）。
 */
QString formatNumber(double value)
{
    if (qIsNaN(value) || qIsInf(value))
        return QStringLiteral("错误");

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
    if (m_error)
        clearAll();               // 出错后重新输入数字会自动开始新的计算

    if (m_freshOperand) {
        if (m_pendingOp == Op::None) {
            m_accumulator = 0.0;  // 上一步的结果不再参与新的计算
            setExpressionText(QString());
        }
        setDisplayText(digit);    // 用新数字替换显示屏内容
        m_freshOperand = false;
        return;
    }

    QString text = ui->display->text();
    if (text == QLatin1String("0"))
        text = digit;                 // 避免出现 "0123" 这类前导 0
    else if (text == QLatin1String("-0"))
        text = QLatin1Char('-') + digit;
    else
        text += digit;

    if (text.length() > kMaxInputLength)
        return;

    setDisplayText(text);
}

void Calculator::enterDot()
{
    if (m_error)
        clearAll();

    if (m_freshOperand) {
        if (m_pendingOp == Op::None) {
            m_accumulator = 0.0;
            setExpressionText(QString());
        }
        setDisplayText(QStringLiteral("0."));
        m_freshOperand = false;
        return;
    }

    QString text = ui->display->text();
    if (text.contains(QLatin1Char('.')))
        return;                       // 同一操作数只允许出现一个小数点

    if (text.isEmpty())
        text = QStringLiteral("0");
    if (text.length() >= kMaxInputLength)
        return;

    setDisplayText(text + QLatin1Char('.'));
}

void Calculator::applyOperator(Op op)
{
    if (m_error)
        return;

    const double value = currentValue();

    // 连续按下运算符（此时还没有输入第二个操作数）：只替换运算符
    if (m_freshOperand && m_pendingOp != Op::None) {
        m_pendingOp = op;
        setExpressionText(formatNumber(m_accumulator) + QLatin1Char(' ') + opSymbol(op));
        return;
    }

    if (m_pendingOp == Op::None) {
        m_accumulator = value;        // 记下第一个操作数
    } else {
        double result = 0.0;
        if (!compute(m_accumulator, value, m_pendingOp, result)) {
            showError(QStringLiteral("除数不能为 0"));
            return;
        }
        m_accumulator = result;       // 连续运算：把中间结果作为新的第一个操作数
        setDisplayText(formatNumber(result));
    }

    m_pendingOp = op;
    m_freshOperand = true;            // 接下来等待输入第二个操作数
    setExpressionText(formatNumber(m_accumulator) + QLatin1Char(' ') + opSymbol(op));
}

void Calculator::evaluate()
{
    if (m_error)
        return;

    const double value = currentValue();

    if (m_pendingOp == Op::None) {
        // 没有待处理的运算符：直接把当前显示作为结果
        setDisplayText(formatNumber(value));
        m_freshOperand = true;
        return;
    }

    double result = 0.0;
    if (!compute(m_accumulator, value, m_pendingOp, result)) {
        showError(QStringLiteral("除数不能为 0"));
        return;
    }

    setExpressionText(formatNumber(m_accumulator) + QLatin1Char(' ')
                      + opSymbol(m_pendingOp) + QLatin1Char(' ')
                      + formatNumber(value) + QStringLiteral(" ="));
    setDisplayText(formatNumber(result));

    m_accumulator = result;
    m_pendingOp = Op::None;
    m_freshOperand = true;            // 计算完成后继续输入数字将开始新的计算
}

void Calculator::backspace()
{
    if (m_error) {
        clearAll();
        return;
    }

    if (m_freshOperand)               // 刚按下运算符/等号，当前没有正在输入的操作数
        return;

    QString text = ui->display->text();
    text.chop(1);
    if (text.isEmpty() || text == QLatin1String("-")) {
        text = QStringLiteral("0");
        m_freshOperand = true;        // 删除干净后回到“等待新操作数”的状态
    }
    setDisplayText(text);
}

void Calculator::clearAll()
{
    m_accumulator = 0.0;
    m_pendingOp = Op::None;
    m_freshOperand = true;
    m_error = false;
    setDisplayText(QStringLiteral("0"));
    setExpressionText(QString());
}

void Calculator::toggleSign()
{
    if (m_error)
        return;

    // 刚按下运算符、还没有输入第二个操作数时，没有可翻转的对象
    if (m_freshOperand && m_pendingOp != Op::None)
        return;

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
    if (m_error)
        return;

    if (m_freshOperand && m_pendingOp != Op::None)
        return;

    setDisplayText(formatNumber(currentValue() / 100.0));
    m_freshOperand = false;
}

void Calculator::setDisplayText(const QString &text)
{
    ui->display->setText(text);
    ui->display->setCursorPosition(ui->display->text().length());
}

void Calculator::setExpressionText(const QString &text)
{
    ui->expressionLabel->setText(text);
}

double Calculator::currentValue() const
{
    bool ok = false;
    const double value = ui->display->text().toDouble(&ok);
    return ok ? value : 0.0;
}

bool Calculator::compute(double lhs, double rhs, Op op, double &result)
{
    switch (op) {
    case Op::Add:
        result = lhs + rhs;
        break;
    case Op::Subtract:
        result = lhs - rhs;
        break;
    case Op::Multiply:
        result = lhs * rhs;
        break;
    case Op::Divide:
        if (qFuzzyIsNull(rhs))
            return false;             // 除数不能为 0
        result = lhs / rhs;
        break;
    case Op::None:
    default:
        result = rhs;
        break;
    }

    if (qIsNaN(result) || qIsInf(result))
        return false;

    return true;
}

QChar Calculator::opSymbol(Op op)
{
    switch (op) {
    case Op::Add:      return QLatin1Char('+');
    case Op::Subtract: return QChar(0x2212); // −
    case Op::Multiply: return QChar(0x00D7); // ×
    case Op::Divide:   return QChar(0x00F7); // ÷
    case Op::None:     break;
    }
    return QChar();
}

void Calculator::showError(const QString &message)
{
    m_error = true;
    setDisplayText(QStringLiteral("错误"));
    setExpressionText(message);
}
