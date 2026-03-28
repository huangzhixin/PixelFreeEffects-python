# SMBeautyEngine Win Python

这个目录是 Windows 下的 Python 交付版本，目标是让 Python 原生调用 PixelFree SDK，并支持图片和视频逐帧处理。

当前推荐使用的入口只有两条：

1. `processor_cli.py`
   用命令行处理图片或视频
2. `bridge.py`
   在你自己的 Python 业务代码里直接调用原生接口

## 目录说明

建议交付时保留以下文件：

- `bridge.py`
- `processor_cli.py`
- `pyproject.toml`
- `requirements.txt`
- `native_bridge/pixel_free_bridge.cpp`
- `native_bridge/build_bridge.cmd`
- `res/pixelfreeAuth.lic`
- `res/filter_model.bundle`
- `README.md`

说明：

- `main.py` 是仓库里原本已有的示例文件，不作为当前交付入口。
- `lib/pixel_free_bridge.dll` 是编译产物，不一定会进 Git；如果同事本地没有这个 DLL，需要按下面的步骤先编译。

## 环境要求

- Windows 10 或更高版本
- Python 3.10 及以上
- `uv`
- Visual Studio 2022 Build Tools
- 可用的 OpenGL 运行环境

推荐安装项：

- `Desktop development with C++`
- MSVC x64 编译工具链
- Windows SDK

## 首次部署

### 推荐方案：使用 uv

先安装 `uv`。

如果本机还没有 `uv`，可以参考官方安装方式，安装完成后在本目录执行：

```powershell
uv venv --python 3.11
uv sync
```

说明：

- `uv venv --python 3.11` 会在当前目录创建 `.venv`
- `uv sync` 会根据 `pyproject.toml` 安装依赖
- 后续运行 Python 命令时，推荐使用 `uv run`

然后编译原生 bridge：

```powershell
cd native_bridge
cmd /c build_bridge.cmd
cd ..
```

### 兼容方案：继续使用 requirements.txt

在 `SMBeautyEngine_win_python` 目录执行：

```powershell
python -m pip install -r requirements.txt
```

然后编译原生 bridge：

```powershell
cd native_bridge
cmd /c build_bridge.cmd
```

编译成功后，应该能看到：

```text
SMBeautyEngine_win_python/lib/pixel_free_bridge.dll
```

如果这个文件不存在，`bridge.py` 和 `processor_cli.py` 都无法运行。

## 推荐运行方式

如果你使用 `uv` 管理环境，推荐这样执行：

```powershell
uv run python processor_cli.py --input .\res\test.png --output .\res\result.png --filter-name heibai1 --filter-strength 1.0
```

## 直接运行

### 处理图片

```powershell
python processor_cli.py --input .\res\test.png --output .\res\result.png --filter-name heibai1 --filter-strength 1.0
```

使用 `uv` 时：

```powershell
uv run python processor_cli.py --input .\res\test.png --output .\res\result.png --filter-name heibai1 --filter-strength 1.0
```

### 处理视频

```powershell
python processor_cli.py --input .\res\test.mp4 --output .\res\result.mp4 --filter-name heibai1 --filter-strength 1.0 --narrow 0.9 --v-face 0.9 --white 0.3 --blur 0.4
```

使用 `uv` 时：

```powershell
uv run python processor_cli.py --input .\res\test.mp4 --output .\res\result.mp4 --filter-name heibai1 --filter-strength 1.0 --narrow 0.9 --v-face 0.9 --white 0.3 --blur 0.4
```

长视频建议带上进度输出：

```powershell
python processor_cli.py --input .\res\test.mp4 --output .\res\result.mp4 --filter-name heibai1 --filter-strength 1.0 --progress-every 10
```

使用 `uv` 时：

```powershell
uv run python processor_cli.py --input .\res\test.mp4 --output .\res\result.mp4 --filter-name heibai1 --filter-strength 1.0 --progress-every 10
```

## 参数说明

常用参数：

- `--input`
  输入图片或视频路径
- `--output`
  输出图片或视频路径
- `--auth`
  授权文件路径，默认使用 `res/pixelfreeAuth.lic`
- `--filter-bundle`
  滤镜资源路径，默认使用 `res/filter_model.bundle`
- `--filter-name`
  滤镜名，例如 `heibai1`
- `--filter-strength`
  滤镜强度，范围通常为 `0.0 ~ 1.0`
- `--eye`
  大眼
- `--thin`
  瘦脸
- `--narrow`
  窄脸
- `--v-face`
  V 脸
- `--white`
  美白
- `--blur`
  磨皮
- `--ruddy`
  红润
- `--rotation`
  人脸检测方向，可选 `0`、`90`、`180`、`270`
- `--progress-every`
  视频处理时每多少帧打印一次进度

说明：

- 常规美颜参数按 `0.0 ~ 1.0` 传值。
- `origin` 表示原图，不加滤镜。

## 可用滤镜名

当前仓库里能确认到的滤镜名包括：

- `origin`
- `chengshi`
- `chulian`
- `chuxin`
- `fennen`
- `hupo`
- `lengku`
- `meiwei`
- `naicha`
- `pailide`
- `qingxin`
- `rixi`
- `riza`
- `weimei`
- `heibai1`

说明：

- 代码和文档里的滤镜列表示例并不完全一致，所以实际使用时如果要严格限制选项，建议结合业务侧白名单处理。

## 在 Python 代码里调用

如果不走 CLI，而是要在自己的脚本里逐帧处理视频，可以这样使用：

```python
import numpy as np
from bridge import PixelFreeBridge, PFBeautyFiterType

with PixelFreeBridge() as bridge:
    bridge.set_filter_name("heibai1")
    bridge.set_float_param(PFBeautyFiterType.PFBeautyFiterStrength, 1.0)
    bridge.set_float_param(PFBeautyFiterType.PFBeautyFiterTypeFace_narrow, 0.9)
    bridge.set_float_param(PFBeautyFiterType.PFBeautyFiterTypeFace_V, 0.9)

    rgba = np.zeros((720, 1280, 4), dtype=np.uint8)
    result = bridge.process_rgba(rgba)
```

输入要求：

- 类型：`numpy.ndarray`
- 形状：`(H, W, 4)`
- 通道：RGBA
- 数据类型：`uint8`

## 性能说明

当前实现是逐帧处理，长视频耗时会比较明显。

例如：

- `2244x1242`
- `30fps`
- `32.5s`

这类视频处理时间通常会远高于视频本身时长。终端如果长时间没有输出，不一定是卡死，可能只是还在逐帧处理。可以通过 `--progress-every` 观察进度。

## 常见问题

### 1. 找不到 `pixel_free_bridge.dll`

先执行：

```powershell
cd native_bridge
cmd /c build_bridge.cmd
```

### 2. 视频处理看起来像卡住

先确认终端是否在持续输出进度。如果没有进度输出，请加：

```powershell
--progress-every 10
```

### 3. 处理结果上下颠倒

这个问题已经在当前 bridge 版本中修复。如果再次出现，请确认本地使用的是最新编译出来的 `lib/pixel_free_bridge.dll`。

### 4. 同事下载代码后怎么快速创建环境

推荐直接执行：

```powershell
uv venv --python 3.11
uv sync
cd native_bridge
cmd /c build_bridge.cmd
cd ..
uv run python processor_cli.py --input .\res\test.png --output .\res\result.png --filter-name heibai1 --filter-strength 1.0
```

## 交付建议

建议以新分支交付，并只提交这些内容：

- Python 源码
- `native_bridge` 源码和构建脚本
- `pyproject.toml`
- `requirements.txt`
- 本 README

不建议提交：

- `__pycache__`
- `.obj`
- `.exp`
- `.lib`
- 临时输出图片和视频
- 排查阶段使用的旧 bridge 或 runner 方案
