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

用 Qt Creator 打开 **`Calculator.pro`**（本机装的 Qt Creator 9.0.0 + Qt 6.8.3
MinGW，用 qmake 工程即可），选择桌面套件后直接“构建并运行”。

> `CMakeLists.txt` 是为方便其他机器准备的。本机没有安装 CMake，Qt Creator
> 里也没有配置 CMake 工具，所以请不要用它打开工程，否则会提示找不到 CMake。

### 方式二：命令行（qmake + MinGW）

```bash
mkdir build && cd build
qmake ../Calculator.pro
mingw32-make
./release/Calculator.exe
```

> 若要在没有 Qt 环境的机器上运行，需要用 `windeployqt Calculator.exe`
> 把所需的 DLL 一并复制到程序目录。

## 常见问题

**构建时提示 `Cannot find file: ..\qt???1\Calculator.pro`，或者干脆不生成 exe。**

这是路径里有中文引起的：MinGW 的 `mingw32-make` 在中文版 Windows 上无法正确
处理非 ASCII 路径，中文会变成 `???`，于是 make 找不到工程文件，编译一步都不会
执行（所以也就没有程序可以运行）。本项目目录名含有“实验1”，属于这种情况。

两种解决办法，任选其一：

1. 让构建目录落在工程内部（本项目已经这么设置好了）。在 Qt Creator 里：
   “项目” → “Build Settings” → “Build directory”改为
   `C:\Users\12828\Documents\ChatGPT\qt实验1\build\qtcreator-debug`，
   然后重新构建。此时相对路径是 `..\..\Calculator.pro`，不含中文，可以正常编译。
2. 把整个工程放到纯英文路径下，例如 `C:\QtProjects\Calculator`，再重新打开。

第 1 种办法改动最小，本项目的 `Calculator.pro.user` 已经改成这种配置，正常情况
下直接打开 `Calculator.pro` 按“构建并运行”即可。

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
