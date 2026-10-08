# 带键盘事件的计算器（Qt 实验一）

东莞理工学院计算机科学与技术学院《Qt 应用程序开发》实验一作品。
使用 **Qt 6 + Qt Widgets** 实现一个同时支持**鼠标点击**和**键盘输入**的计算器。

## 功能

- 四则运算：加、减、乘、除（按输入顺序从左到右计算）
- 小数输入、百分号（%）
- 退格（删除最后一位）、清除（C，一键重置）
- 鼠标点击与键盘按键**共用同一套处理逻辑**，两种输入方式结果一致
- 异常输入处理：
  - 同一操作数只允许出现**一个小数点**（`1..2` → `1.2`）
  - 连续按下运算符时**只替换运算符**，不会报错（`5 + − × 3 =` → `15`）
  - **除数为 0** 时显示“错误”，并提示“除数不能为 0”
  - 出错后直接输入数字即可开始新的计算
  - 得到结果后继续输入数字会**开始新的计算**；继续按运算符会**接着结果计算**
  - 输入过程中可以随时退格，删空后自动回到 `0`
- 输出结果自动去掉多余的尾随 0，并避免 `0.1 + 0.2` 显示成 `0.30000000000000004`
- 使用 Qt 样式表（QSS）美化界面

## 键盘操作

| 键盘 | 功能 |
| --- | --- |
| `0`–`9` | 输入数字 |
| `.`（句点 / 小键盘） | 小数点 |
| `+` `-` `*`（或 `x`）`/` | 加、减、乘、除 |
| `Enter` / `=` | 等于 |
| `Backspace` | 退格 |
| `Esc` / `Delete` / `C` | 清除 |
| `%` | 百分号 |

## 目录结构

```
.
├── main.cpp              程序入口
├── calculator.h/.cpp     计算器窗口类（信号与槽、键盘事件、运算逻辑）
├── calculator.ui         UI 设计器界面文件（布局与控件）
├── style.qss             界面样式表
├── resources.qrc         资源文件（把样式表打进可执行文件）
├── Calculator.pro        qmake 工程文件
├── CMakeLists.txt        CMake 工程文件（Qt Creator 默认）
├── tests/                自动化行为测试
└── tools/                生成实验报告截图的辅助程序
```

## 编译运行

### 方式一：Qt Creator

用 Qt Creator 打开 `CMakeLists.txt` 或 `Calculator.pro`，选择 Qt 6 套件后直接
“构建并运行”。

### 方式二：命令行（qmake + MinGW）

```bash
mkdir build && cd build
qmake ../Calculator.pro
mingw32-make
./release/Calculator.exe
```

> 若要在没有 Qt 环境的机器上运行，需要用 `windeployqt Calculator.exe`
> 把所需的 DLL 一并复制到程序目录。

## 运行测试

```bash
cd tests
mkdir build && cd build
qmake ../tests.pro
mingw32-make
./release/calculator_tests.exe
```

测试共 23 项，覆盖四则运算、小数、异常输入、退格、清除、除零、
连续运算，以及鼠标与键盘输入结果一致性。
