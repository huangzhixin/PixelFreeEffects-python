import ctypes
import os
from enum import IntEnum

import numpy as np
from PIL import Image


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


class PixelFreeBridge:
    def __init__(self, dll_path=None, auth_path=None, filter_path=None):
        base_dir = os.path.dirname(os.path.abspath(__file__))
        self.dll_path = dll_path or os.path.join(base_dir, "lib", "pixel_free_bridge.dll")
        self.auth_path = auth_path or os.path.join(base_dir, "res", "pixelfreeAuth.lic")
        self.filter_path = filter_path or os.path.join(base_dir, "res", "filter_model.bundle")

        self._dll = ctypes.CDLL(self.dll_path)
        self._dll.PFPy_GetLastError.restype = ctypes.c_char_p
        self._dll.PFPy_CreateEngine.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
        self._dll.PFPy_CreateEngine.restype = ctypes.c_void_p
        self._dll.PFPy_SetFloatParam.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_float]
        self._dll.PFPy_SetFloatParam.restype = ctypes.c_int
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
            self.auth_path.encode("utf-8"),
            self.filter_path.encode("utf-8"),
        )
        if not self._handle:
            raise RuntimeError(self.last_error())

    def last_error(self):
        message = self._dll.PFPy_GetLastError()
        return message.decode("utf-8", errors="ignore") if message else "Unknown bridge error"

    def set_filter_name(self, name):
        ok = self._dll.PFPy_SetStringParam(
            self._handle,
            PFBeautyFiterType.PFBeautyFiterName.value,
            name.encode("utf-8"),
        )
        if not ok:
            raise RuntimeError(self.last_error())

    def set_float_param(self, param_type, value):
        ok = self._dll.PFPy_SetFloatParam(self._handle, int(param_type), float(value))
        if not ok:
            raise RuntimeError(self.last_error())

    def process_rgba(self, rgba_image, rotation=PFRotationMode.PFRotationMode0):
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

    def close(self):
        if self._handle:
            self._dll.PFPy_DestroyEngine(self._handle)
            self._handle = None

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc, tb):
        self.close()


def process_image_file(input_path, output_path):
    with PixelFreeBridge() as bridge:
        bridge.set_filter_name("heibai1")
        bridge.set_float_param(PFBeautyFiterType.PFBeautyFiterTypeFace_narrow, 1.0)
        bridge.set_float_param(PFBeautyFiterType.PFBeautyFiterTypeFace_V, 1.0)

        image = Image.open(input_path).convert("RGBA")
        frame = np.array(image, dtype=np.uint8)
        result = bridge.process_rgba(frame)
        Image.fromarray(result, "RGBA").save(output_path)
