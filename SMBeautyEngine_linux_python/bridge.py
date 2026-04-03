from __future__ import annotations

import ctypes
from enum import IntEnum
from pathlib import Path

import numpy as np


class PFRotationMode(IntEnum):
    PFRotationMode0 = 0
    PFRotationMode90 = 1
    PFRotationMode180 = 2
    PFRotationMode270 = 3


class PFBeautyFiterType(IntEnum):
    PFBeautyFiterTypeFace_EyeStrength = 0
    PFBeautyFiterTypeFace_thinning = 1
    PFBeautyFiterTypeFace_narrow = 2
    PFBeautyFiterTypeFace_chin = 3
    PFBeautyFiterTypeFace_V = 4
    PFBeautyFiterTypeFace_small = 5
    PFBeautyFiterTypeFace_nose = 6
    PFBeautyFiterTypeFace_forehead = 7
    PFBeautyFiterTypeFace_mouth = 8
    PFBeautyFiterTypeFace_philtrum = 9
    PFBeautyFiterTypeFace_long_nose = 10
    PFBeautyFiterTypeFace_eye_space = 11
    PFBeautyFiterTypeFace_smile = 12
    PFBeautyFiterTypeFace_eye_rotate = 13
    PFBeautyFiterTypeFace_canthus = 14
    PFBeautyFiterTypeFaceBlurStrength = 15
    PFBeautyFiterTypeFaceWhitenStrength = 16
    PFBeautyFiterTypeFaceRuddyStrength = 17
    PFBeautyFiterTypeFaceSharpenStrength = 18
    PFBeautyFiterTypeFaceM_newWhitenStrength = 19
    PFBeautyFiterTypeFaceH_qualityStrength = 20
    PFBeautyFiterTypeFaceEyeBrighten = 21
    PFBeautyFiterName = 22
    PFBeautyFiterStrength = 23
    PFBeautyFiterTypeOneKey = 26


class PFBeautyTypeOneKey(IntEnum):
    PFBeautyTypeOneKeyNormal = 0
    PFBeautyTypeOneKeyNatural = 1
    PFBeautyTypeOneKeyCute = 2
    PFBeautyTypeOneKeyGoddess = 3
    PFBeautyTypeOneKeyFair = 4


class PixelFreeBridge:
    def __init__(self, so_path: str | None = None, auth_path: str | None = None, filter_path: str | None = None):
        base_dir = Path(__file__).resolve().parent
        linux_sdk_dir = base_dir.parent / "SMBeautyEngine_linux"
        self.so_path = Path(so_path) if so_path else base_dir / "lib" / "pixel_free_bridge.so"
        self.pixel_free_lib_path = linux_sdk_dir / "pixelfreeLib" / "libPixelFree.so"
        self.auth_path = Path(auth_path) if auth_path else linux_sdk_dir / "Res" / "pixelfreeAuth.lic"
        self.filter_path = Path(filter_path) if filter_path else linux_sdk_dir / "Res" / "filter_model.bundle"

        if not self.pixel_free_lib_path.exists():
            raise FileNotFoundError(f"PixelFree shared library not found: {self.pixel_free_lib_path}")
        if not self.so_path.exists():
            raise FileNotFoundError(f"Python bridge shared library not found: {self.so_path}")

        self._pixel_free = ctypes.CDLL(str(self.pixel_free_lib_path), mode=ctypes.RTLD_GLOBAL)
        self._dll = ctypes.CDLL(str(self.so_path))
        self._dll.PFPy_GetLastError.restype = ctypes.c_char_p
        self._dll.PFPy_CreateEngine.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        self._dll.PFPy_CreateEngine.restype = ctypes.c_void_p
        self._dll.PFPy_SetFloatParam.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_float]
        self._dll.PFPy_SetFloatParam.restype = ctypes.c_int
        self._dll.PFPy_SetIntParam.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        self._dll.PFPy_SetIntParam.restype = ctypes.c_int
        self._dll.PFPy_SetStringParam.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_char_p]
        self._dll.PFPy_SetStringParam.restype = ctypes.c_int
        self._dll.PFPy_ProcessImageRGBA.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_uint8),
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
            ctypes.c_int,
        ]
        self._dll.PFPy_ProcessImageRGBA.restype = ctypes.c_int
        self._dll.PFPy_DestroyEngine.argtypes = [ctypes.c_void_p]
        self._dll.PFPy_DestroyEngine.restype = None

        self._handle = self._dll.PFPy_CreateEngine(
            str(self.auth_path).encode("utf-8"),
            str(self.filter_path).encode("utf-8"),
        )
        if not self._handle:
            raise RuntimeError(self.last_error())

    def last_error(self) -> str:
        message = self._dll.PFPy_GetLastError()
        return message.decode("utf-8", errors="ignore") if message else "Unknown bridge error"

    def set_filter_name(self, name: str) -> None:
        ok = self._dll.PFPy_SetStringParam(
            self._handle,
            PFBeautyFiterType.PFBeautyFiterName.value,
            name.encode("utf-8"),
        )
        if not ok:
            raise RuntimeError(self.last_error())

    def set_float_param(self, param_type: PFBeautyFiterType, value: float) -> None:
        ok = self._dll.PFPy_SetFloatParam(self._handle, int(param_type), float(value))
        if not ok:
            raise RuntimeError(self.last_error())

    def set_int_param(self, param_type: PFBeautyFiterType, value: int) -> None:
        ok = self._dll.PFPy_SetIntParam(self._handle, int(param_type), int(value))
        if not ok:
            raise RuntimeError(self.last_error())

    def process_rgba(self, rgba_image: np.ndarray, rotation: PFRotationMode = PFRotationMode.PFRotationMode0) -> np.ndarray:
        frame = np.ascontiguousarray(rgba_image, dtype=np.uint8)
        if frame.ndim != 3 or frame.shape[2] != 4:
            raise ValueError("Expected RGBA image with shape (H, W, 4)")

        height, width = frame.shape[:2]
        ptr = frame.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8))
        ok = self._dll.PFPy_ProcessImageRGBA(
            self._handle,
            ptr,
            width,
            height,
            width * 4,
            int(rotation),
        )
        if not ok:
            raise RuntimeError(self.last_error())
        return frame

    def close(self) -> None:
        if self._handle:
            self._dll.PFPy_DestroyEngine(self._handle)
            self._handle = None

    def __enter__(self) -> "PixelFreeBridge":
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()
