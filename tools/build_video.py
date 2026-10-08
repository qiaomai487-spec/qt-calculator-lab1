"""
把 tools/make_video_frames 生成的原始帧合成带字幕的演示视频。

用法：
    python tools/build_video.py <项目根目录>

输出：
    docs/演示视频.mp4       （约 2~3 分钟的演示视频）
    docs/git-history.png    （提交历史截图，供实验报告使用）
"""

import io
import os
import subprocess
import sys

import cv2
import numpy as np
from PIL import Image, ImageDraw, ImageFont

WIDTH, HEIGHT = 1200, 800
FPS = 10                 # 视频帧率
MIN_HOLD_MS = 1000       # 每张画面最短停留时间

YAHEI = r"C:\Windows\Fonts\msyh.ttc"
CONSOLA = r"C:\Windows\Fonts\consola.ttf"

BG = (16, 18, 24)
PANEL = (26, 28, 36)
ACCENT = (255, 159, 10)
TEXT = (238, 240, 246)
MUTED = (138, 143, 156)


def font(path, size):
    try:
        return ImageFont.truetype(path, size)
    except OSError:
        return ImageFont.load_default()


def wrap(draw, text, fnt, max_width):
    """按字符宽度换行（兼容中英文混排）。"""
    lines, line = [], ""
    for ch in text:
        if ch == "\n":
            lines.append(line)
            line = ""
            continue
        if draw.textlength(line + ch, font=fnt) <= max_width:
            line += ch
        else:
            lines.append(line)
            line = ch
    if line:
        lines.append(line)
    return lines


def read_steps(frames_dir):
    steps = []
    with io.open(os.path.join(frames_dir, "steps.txt"), encoding="utf-8") as fh:
        for raw in fh:
            raw = raw.rstrip("\n")
            if not raw:
                continue
            name, hold, caption = raw.split("\t", 2)
            steps.append((os.path.join(frames_dir, name), int(hold), caption))
    return steps


def build_slide(calc_path, caption, index, total, hold_ms):
    """左侧放计算器画面，右侧放字幕。"""
    slide = Image.new("RGB", (WIDTH, HEIGHT), BG)
    draw = ImageDraw.Draw(slide)

    # 左侧计算器截图
    calc = Image.open(calc_path).convert("RGB")
    target_h = 700
    target_w = int(calc.width * target_h / calc.height)
    calc = calc.resize((target_w, target_h), Image.LANCZOS)
    left_x, left_y = 70, 50
    draw.rounded_rectangle(
        [left_x - 10, left_y - 10, left_x + target_w + 10, left_y + target_h + 10],
        radius=16, fill=PANEL)
    slide.paste(calc, (left_x, left_y))

    # 右侧文字区
    x0 = left_x + target_w + 70
    max_w = WIDTH - x0 - 60

    f_title = font(YAHEI, 38)
    f_step = font(YAHEI, 24)
    f_body = font(YAHEI, 31)
    f_small = font(YAHEI, 22)

    draw.text((x0, 60), "实验一：带键盘事件的计算器", font=f_title, fill=ACCENT)
    draw.line([(x0, 120), (WIDTH - 60, 120)], fill=(60, 64, 76), width=2)
    draw.text((x0, 140), f"步骤 {index} / {total}", font=f_step, fill=MUTED)

    lines = wrap(draw, caption, f_body, max_w)
    y = 205
    for line in lines:
        draw.text((x0, y), line, font=f_body, fill=TEXT)
        y += 48

    draw.text((x0, HEIGHT - 90), "鼠标输入与键盘输入共用同一套处理逻辑", font=f_small, fill=MUTED)
    draw.text((x0, HEIGHT - 55), "Qt 6 · Qt Widgets · qmake / CMake", font=f_small, fill=MUTED)

    return slide


def git_log_lines(repo):
    fmt = "%h  %ad  %s"
    out = subprocess.run(
        ["git", "-C", repo, "log", "--date=short", f"--pretty=format:{fmt}", "--reverse"],
        capture_output=True, text=True, encoding="utf-8")
    return [line for line in out.stdout.splitlines() if line.strip()]


def build_git_slide(repo):
    slide = Image.new("RGB", (WIDTH, HEIGHT), BG)
    draw = ImageDraw.Draw(slide)
    f_title = font(YAHEI, 40)
    f_mono = font(CONSOLA, 26)
    f_msg = font(YAHEI, 25)      # Consolas 没有中文字形，提交说明必须用中文字体
    f_small = font(YAHEI, 22)

    draw.text((70, 50), "Git 提交历史（实验一）", font=f_title, fill=ACCENT)
    draw.line([(70, 112), (WIDTH - 70, 112)], fill=(60, 64, 76), width=2)

    y = 150
    for line in git_log_lines(repo):
        parts = line.split("  ", 2)
        short, date, message = parts if len(parts) == 3 else ("", "", line)
        prefix = f"{short}  {date}  "
        draw.text((70, y), prefix, font=f_mono, fill=MUTED)
        draw.text((70 + draw.textlength(prefix, font=f_mono), y), message,
                  font=f_msg, fill=TEXT)
        y += 46

    draw.text((70, HEIGHT - 70), "提交按开发阶段组织：功能实现 → 调试修复 → 测试 → 文档",
              font=f_small, fill=MUTED)
    return slide


def save_png(image, path):
    image.save(path, "PNG")


def main():
    repo = sys.argv[1] if len(sys.argv) > 1 else os.getcwd()
    frames_dir = os.path.join(repo, "docs", "video_frames")
    docs = os.path.join(repo, "docs")

    steps = read_steps(frames_dir)
    if not steps:
        print("没有找到原始帧，请先运行 tools/make_video_frames")
        return 1

    total_raw = sum(hold for _, hold, _ in steps)
    scale = max(1.0, 165000.0 / total_raw)      # 目标时长约 165 秒

    slides = []
    for i, (path, hold, caption) in enumerate(steps, start=1):
        slides.append((build_slide(path, caption, i, len(steps), hold),
                       max(MIN_HOLD_MS, int(hold * scale))))

    git_slide = build_git_slide(repo)
    save_png(git_slide, os.path.join(docs, "git-history.png"))
    slides.append((git_slide, 8000))

    out_path = os.path.join(docs, "演示视频.mp4")
    writer = cv2.VideoWriter(out_path, cv2.VideoWriter_fourcc(*"mp4v"), FPS, (WIDTH, HEIGHT))
    if not writer.isOpened():
        print("无法创建视频文件（缺少编码器）")
        return 1

    written = 0
    for image, hold_ms in slides:
        frame = np.array(image)[:, :, ::-1]           # RGB -> BGR
        repeat = max(1, int(round(hold_ms * FPS / 1000.0)))
        for _ in range(repeat):
            writer.write(frame)
            written += 1
    writer.release()

    print(f"共 {len(slides)} 个画面，{written} 帧，时长约 {written / FPS:.0f} 秒")
    print(f"已生成 {out_path}")
    print(f"已生成 {os.path.join(docs, 'git-history.png')}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
