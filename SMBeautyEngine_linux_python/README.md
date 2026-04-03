# SMBeautyEngine Linux Python

这个目录提供 Linux 下的 Python bridge 和 CLI，供当前 FastAPI 服务调用 PixelFree Linux SDK 处理图片和视频。

核心入口：
- `processor_cli.py`
- `bridge.py`
- `native_bridge/pixel_free_bridge.cpp`

桥接库会编译输出到：
- `lib/pixel_free_bridge.so`
