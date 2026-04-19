import argparse
import math
import os
import shutil
import subprocess
import tempfile
import time
from pathlib import Path

import imageio.v2 as imageio
import numpy as np
from PIL import Image

from bridge import PFBeautyFiterType, PFRotationMode, PixelFreeBridge


os.environ.setdefault("IMAGEIO_FFMPEG_EXE", "ffmpeg")

IMAGE_SUFFIXES = {".png", ".jpg", ".jpeg", ".bmp", ".webp"}
VIDEO_SUFFIXES = {".mp4", ".mov", ".avi", ".mkv", ".webm", ".m4v"}


def parse_args():
    parser = argparse.ArgumentParser(
        description="Process images or videos with PixelFree through a native Python bridge."
    )
    parser.add_argument("--input", required=True, help="Input image or video path")
    parser.add_argument("--output", required=True, help="Output image or video path")
    parser.add_argument("--auth", help="Auth license path")
    parser.add_argument("--filter-bundle", help="filter_model.bundle path")
    parser.add_argument("--filter-name", default=None, help="Built-in filter name")
    parser.add_argument("--filter-strength", type=float, default=None, help="Filter strength 0.0-1.0")
    parser.add_argument("--eye", type=float, default=None, help="Big eye strength")
    parser.add_argument("--thin", type=float, default=None, help="Face thinning strength")
    parser.add_argument("--narrow", type=float, default=None, help="Face narrow strength")
    parser.add_argument("--chin", type=float, default=None)
    parser.add_argument("--small-face", type=float, default=None, dest="small_face")
    parser.add_argument("--nose", type=float, default=None)
    parser.add_argument("--forehead", type=float, default=None)
    parser.add_argument("--mouth", type=float, default=None)
    parser.add_argument("--philtrum", type=float, default=None)
    parser.add_argument("--long-nose", type=float, default=None, dest="long_nose")
    parser.add_argument("--eye-space", type=float, default=None, dest="eye_space")
    parser.add_argument("--smile", type=float, default=None)
    parser.add_argument("--eye-rotate", type=float, default=None, dest="eye_rotate")
    parser.add_argument("--canthus", type=float, default=None)
    parser.add_argument("--v-face", type=float, default=None, dest="v_face", help="V face strength")
    parser.add_argument("--white", type=float, default=None, help="Whitening strength")
    parser.add_argument("--blur", type=float, default=None, help="Skin smoothing strength")
    parser.add_argument("--ruddy", type=float, default=None, help="Ruddy strength")
    parser.add_argument("--sharpen", type=float, default=None)
    parser.add_argument("--new-whiten", type=float, default=None, dest="new_whiten")
    parser.add_argument("--quality", type=float, default=None)
    parser.add_argument("--eye-brighten", type=float, default=None, dest="eye_brighten")
    parser.add_argument("--one-key", type=int, choices=[0, 1, 2, 3, 4], default=0, dest="one_key")
    parser.add_argument(
        "--rotation",
        type=int,
        choices=[0, 90, 180, 270],
        default=0,
        help="Frame rotation mode for face detection",
    )
    parser.add_argument(
        "--progress-every",
        type=int,
        default=30,
        help="Print progress every N frames when processing video",
    )
    return parser.parse_args()


def rotation_to_enum(rotation):
    mapping = {
        0: PFRotationMode.PFRotationMode0,
        90: PFRotationMode.PFRotationMode90,
        180: PFRotationMode.PFRotationMode180,
        270: PFRotationMode.PFRotationMode270,
    }
    return mapping[rotation]


def apply_params(bridge, args):
    if args.filter_name:
        bridge.set_filter_name(args.filter_name)

    float_params = [
        (args.filter_strength, PFBeautyFiterType.PFBeautyFiterStrength),
        (args.eye, PFBeautyFiterType.PFBeautyFiterTypeFace_EyeStrength),
        (args.thin, PFBeautyFiterType.PFBeautyFiterTypeFace_thinning),
        (args.narrow, PFBeautyFiterType.PFBeautyFiterTypeFace_narrow),
        (args.chin, PFBeautyFiterType.PFBeautyFiterTypeFace_chin),
        (args.v_face, PFBeautyFiterType.PFBeautyFiterTypeFace_V),
        (args.small_face, PFBeautyFiterType.PFBeautyFiterTypeFace_small),
        (args.nose, PFBeautyFiterType.PFBeautyFiterTypeFace_nose),
        (args.forehead, PFBeautyFiterType.PFBeautyFiterTypeFace_forehead),
        (args.mouth, PFBeautyFiterType.PFBeautyFiterTypeFace_mouth),
        (args.philtrum, PFBeautyFiterType.PFBeautyFiterTypeFace_philtrum),
        (args.long_nose, PFBeautyFiterType.PFBeautyFiterTypeFace_long_nose),
        (args.eye_space, PFBeautyFiterType.PFBeautyFiterTypeFace_eye_space),
        (args.smile, PFBeautyFiterType.PFBeautyFiterTypeFace_smile),
        (args.eye_rotate, PFBeautyFiterType.PFBeautyFiterTypeFace_eye_rotate),
        (args.canthus, PFBeautyFiterType.PFBeautyFiterTypeFace_canthus),
        (args.white, PFBeautyFiterType.PFBeautyFiterTypeFaceWhitenStrength),
        (args.blur, PFBeautyFiterType.PFBeautyFiterTypeFaceBlurStrength),
        (args.ruddy, PFBeautyFiterType.PFBeautyFiterTypeFaceRuddyStrength),
        (args.sharpen, PFBeautyFiterType.PFBeautyFiterTypeFaceSharpenStrength),
        (args.new_whiten, PFBeautyFiterType.PFBeautyFiterTypeFaceM_newWhitenStrength),
        (args.quality, PFBeautyFiterType.PFBeautyFiterTypeFaceH_qualityStrength),
        (args.eye_brighten, PFBeautyFiterType.PFBeautyFiterTypeFaceEyeBrighten),
    ]
    for value, param_type in float_params:
        if value is not None:
            bridge.set_float_param(param_type, value)

    if args.one_key:
        raise ValueError("one_key beauty is not supported by the Windows Python bridge yet; use one_key=0.")


def process_image(input_path, output_path, args):
    with PixelFreeBridge(auth_path=args.auth, filter_path=args.filter_bundle) as bridge:
        apply_params(bridge, args)
        image = Image.open(input_path).convert("RGBA")
        frame = np.array(image, dtype=np.uint8)
        result = bridge.process_rgba(frame, rotation=rotation_to_enum(args.rotation))
        Image.fromarray(result, "RGBA").save(output_path)


def mux_audio_from_source(video_without_audio: Path, source_video: Path, final_output: Path) -> None:
    temp_output = final_output.with_name(f"{final_output.stem}_mux{final_output.suffix}")
    command = [
        os.environ.get("IMAGEIO_FFMPEG_EXE", "ffmpeg"),
        "-y",
        "-i",
        str(video_without_audio),
        "-i",
        str(source_video),
        "-map",
        "0:v:0",
        "-map",
        "1:a:0?",
        "-c:v",
        "copy",
        "-c:a",
        "aac",
        "-shortest",
        str(temp_output),
    ]
    completed = subprocess.run(command, text=True, capture_output=True)
    if completed.returncode != 0:
        shutil.move(str(video_without_audio), str(final_output))
        return

    video_without_audio.unlink(missing_ok=True)
    if final_output.exists():
        final_output.unlink()
    temp_output.replace(final_output)


def process_video(input_path, output_path, args):
    reader = imageio.get_reader(input_path)
    meta = reader.get_meta_data()
    fps = meta.get("fps", 25)
    size = meta.get("size") or meta.get("source_size")
    duration = meta.get("duration")
    nframes = meta.get("nframes")
    total_frames = None
    if isinstance(nframes, (int, float)) and math.isfinite(nframes):
        total_frames = int(nframes)
    elif duration and fps:
        total_frames = max(1, int(round(duration * fps)))

    started_at = time.time()
    if size:
        print(
            f"Processing video {Path(input_path).name}: "
            f"{size[0]}x{size[1]}, {fps:.2f} fps"
            + (f", ~{total_frames} frames" if total_frames else "")
        )
    else:
        print(
            f"Processing video {Path(input_path).name}: {fps:.2f} fps"
            + (f", ~{total_frames} frames" if total_frames else "")
        )

    with tempfile.TemporaryDirectory(prefix="beauty_video_") as temp_dir:
        temp_video = Path(temp_dir) / Path(output_path).name
        with PixelFreeBridge(auth_path=args.auth, filter_path=args.filter_bundle) as bridge:
            apply_params(bridge, args)

            writer = None
            try:
                for index, frame in enumerate(reader):
                    frame_array = np.asarray(frame, dtype=np.uint8)
                    if frame_array.ndim != 3:
                        raise ValueError(f"Unexpected frame shape at index {index}: {frame_array.shape}")
                    if frame_array.shape[2] not in (3, 4):
                        raise ValueError(f"Unsupported channel count at index {index}: {frame_array.shape[2]}")

                    if frame_array.shape[2] == 4:
                        pil_frame = Image.fromarray(frame_array, "RGBA")
                    else:
                        pil_frame = Image.fromarray(frame_array, "RGB")

                    rgba = np.array(pil_frame.convert("RGBA"), dtype=np.uint8)

                    processed = bridge.process_rgba(rgba, rotation=rotation_to_enum(args.rotation))
                    rgb = processed[:, :, :3]

                    if writer is None:
                        writer = imageio.get_writer(
                            str(temp_video),
                            fps=fps,
                            codec="libx264",
                            quality=8,
                            macro_block_size=1,
                        )

                    writer.append_data(rgb)

                    frame_index = index + 1
                    if args.progress_every > 0 and (
                        frame_index == 1 or frame_index % args.progress_every == 0
                    ):
                        elapsed = max(time.time() - started_at, 1e-6)
                        speed = frame_index / elapsed
                        if total_frames:
                            percent = frame_index / total_frames * 100
                            eta_seconds = max(total_frames - frame_index, 0) / max(speed, 1e-6)
                            print(
                                f"[{frame_index}/{total_frames} | {percent:.1f}%] "
                                f"{speed:.2f} fps, ETA {eta_seconds:.1f}s"
                            )
                        else:
                            print(f"[{frame_index} frames] {speed:.2f} fps")
            finally:
                reader.close()
                if writer is not None:
                    writer.close()

        mux_audio_from_source(temp_video, Path(input_path), Path(output_path))

    elapsed = time.time() - started_at
    print(f"Video processing finished in {elapsed:.1f}s")


def validate_paths(args):
    input_path = Path(args.input).resolve()
    output_path = Path(args.output).resolve()
    if not input_path.exists():
        raise FileNotFoundError(f"Input file not found: {input_path}")

    output_path.parent.mkdir(parents=True, exist_ok=True)

    if args.auth is None:
        args.auth = str((Path(__file__).resolve().parent / "res" / "pixelfreeAuth.lic"))
    if args.filter_bundle is None:
        args.filter_bundle = str((Path(__file__).resolve().parent / "res" / "filter_model.bundle"))

    if not Path(args.auth).exists():
        raise FileNotFoundError(f"Auth file not found: {args.auth}")
    if not Path(args.filter_bundle).exists():
        raise FileNotFoundError(f"Filter bundle not found: {args.filter_bundle}")

    return input_path, output_path


def main():
    args = parse_args()
    input_path, output_path = validate_paths(args)
    suffix = input_path.suffix.lower()

    if suffix in IMAGE_SUFFIXES:
        process_image(str(input_path), str(output_path), args)
        print(f"Processed image written to: {output_path}")
        return

    if suffix in VIDEO_SUFFIXES:
        process_video(str(input_path), str(output_path), args)
        print(f"Processed video written to: {output_path}")
        return

    raise ValueError(f"Unsupported input type: {input_path.suffix}")


if __name__ == "__main__":
    main()
