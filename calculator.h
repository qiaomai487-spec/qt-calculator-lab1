#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QChar>
#include <QString>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Calculator; }
QT_END_NAMESPACE

class QKeyEvent;

/**
 * @brief 带键盘事件的计算器主窗口。
 *
 * 鼠标点击按钮和键盘按键最终都会调用同一组内部处理函数
 * （enterDigit / enterDot / applyOperator / evaluate / backspace / clearAll），
 * 从而保证两种输入方式的行为完全一致，也避免了重复代码。
 */
class Calculator : public QWidget
{
    Q_OBJECT

public:
    explicit Calculator(QWidget *parent = nullptr);
    ~Calculator() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    /// 待处理的四则运算符
    enum class Op { None, Add, Subtract, Multiply, Divide };

    // ---- 输入处理：鼠标与键盘共用 ----
    void enterDigit(const QString &digit);
    void enterDot();
    void applyOperator(Op op);
    void evaluate();
    void backspace();
    void clearAll();
    void toggleSign();
    void applyPercent();

    // ---- 内部工具 ----
    void initConnections();
    void setDisplayText(const QString &text);
    void setExpressionText(const QString &text);
    double currentValue() const;
    void showError(const QString &message);
    static bool compute(double lhs, double rhs, Op op, double &result);
    static QChar opSymbol(Op op);

    Ui::Calculator *ui;

    double m_accumulator = 0.0;   ///< 第一个操作数 / 累计结果
    Op m_pendingOp = Op::None;    ///< 尚未参与计算的运算符
    bool m_freshOperand = true;   ///< true 表示下一次数字输入应替换显示屏内容
    bool m_error = false;         ///< 是否处于错误提示状态

    static constexpr int kMaxInputLength = 16; ///< 单个操作数的最大显示长度
};

#endif // CALCULATOR_H
