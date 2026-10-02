# Signed-distance shader on Ethos-U85

This project replaces Pac-Man with an image-tensor port of Inigo Quilez's [Raymarching – Primitives](https://www.shadertoy.com/view/Xds3zN). The supplied source is retained in `shadertoy.txt`, including its MIT license. The E8 uses its existing 480 x 800 RGB565 display and M55_HP application. The camera renders an 800 x 480 landscape scene, rotated 90 degrees clockwise onto the panel so the scene's width follows the LCD's long dimension.

## CMSIS-Executorch integration

The structure follows [Arm-Examples/CMSIS-Executorch](https://github.com/Arm-Examples/CMSIS-Executorch). There is no nested copy of that project or build of another ExecuTorch source tree. `PyTorch::ExecuTorch@1.4.1` supplies the runtime, kernel utilities, operator registration and Ethos-U backend; its version matches the Python exporter.

- `Blinky.csolution.yml`: packs, target variables and MLOps options for Ethos-U85-256 with the Ensemble SRAM/MRAM Vela configuration.
- `M55_HP/M55_HP.cproject.yml`: application sources and Board/AI layers.
- `board/DevKit-E8/Board.clayer.yml`: display, joystick, startup, services and NPU driver. This project's configured RTE files moved here with the layer.
- `board/DevKit-E8/RTE/Device/AE822FA0E5597LS0_M55_HP/linker_ac6_mram.sct.src`: CMSIS Build preprocesses this scatter source using the target's defines and region header. SRAM0 at `0x02000000` holds NPU scratch; SRAM1 at `0x08000000` holds CPU planned tensors and framebuffers. The exporter checks each 4 MiB bank separately. These arenas are uninitialized at startup and used after the banks are powered; firmware explicitly clears the framebuffers. The pack's original `.sct` remains intact.
- `board/DevKit-E8/ethos_support.c`: NPU initialization, IRQ and cache hooks.
- `board/DevKit-E8/stdio_support.c`: nonblocking CMSIS-Compiler stream hooks for the LCD demo. Output is discarded and input returns EOF. AC6 semihosting is disabled at link time, so runtime/driver STDIO dependencies cannot stop startup at a semihosting breakpoint.
- `create_ai_layer.py`: reads the Toolbox-generated `Blinky.cbuild-mlops.yml`, calibrates/quantizes the image stages, delegates them with ExecuTorch/Vela, and generates the AI layer and embedded model. Publication requires one complete Ethos delegate per tensor stage; only application IO quantize/dequantize operations may remain on M55_HP.
- `scripts/prepare_shader_graph.py`: converts scalar masks, one-sided clamps and tuple splits into tensor operators before PT2E annotation. Explicit Arm scale sharing is retained; TorchAO's inferred sharing is disabled to avoid cyclic scale groups in recurrent state slices.
- `ai_layer/`: generated runtime/operator component selection, embedded program, dimensions and delegation report.
- `M55_HP/model_runtime.cpp`: small project-owned runner using the pack's Program/Method API. It checks memory requirements and reuses pools when switching methods.
- `scripts/inspect_program.py`: checks every serialized delegate input/output against Vela's integer element size and pixel count, and reports serialized operators and arena requirements. The exporter sizes the planned-memory pool from the PTE and the temporary pool from the largest Ethos delegate scratch buffer plus a 512 KiB allowance for CPU kernels.
- `M55_HP/sdf_renderer.cpp`: prepares global camera rays, orchestrates tile iterations, and assembles RGB565 output.

The generated program contains all eleven methods and is 1,920,764 bytes. Each method has one complete Ethos delegate and only IO quantize/dequantize CPU kernels; all 22 delegate input/output descriptors pass the serialized IO check. The HP executable/model region is 3 MiB at `0x80200000`; the HE's 2 MiB region is preserved, with 512 KiB reserved for user data at `0x80500000`. The current tile configuration requires 38,400 bytes of planned memory and a 3,094,528-byte temporary arena. Full-resolution normals account for the largest delegate scratch requirement (2,570,240 bytes). Re-exporting records updated requirements in `ai_layer/memory.json`.

## Python environment and export

Run `./setup_venv.ps1`, or the **SDF: Setup Python 3.12 with uv** task. It runs `uv venv --python 3.12`, installs `requirements.txt`, then installs `requirements-arm-tosa.txt` with `--no-deps`, as required by the reference project's Arm export flow. The separate TOSA installation preserves flatbuffers 24.3.25.

Open `Blinky.csolution.yml` and select **DevKit-E8@Release**. CMSIS Solution generates `Blinky.cbuild-mlops.yml` on solution resolution/build. Then run **SDF: Create AI layer**, or:

```powershell
uv run --active --no-project python create_ai_layer.py Blinky.cbuild-mlops.yml
```

Use CMSIS Solution **Build**, then **Load and Run**. Re-export after changing the model, tile dimensions or MLOps settings. The exporter publishes the layer only after every method compiles, so a partial export does not replace a working program. To investigate one method without replacing the layer:

```powershell
uv run --active --no-project python create_ai_layer.py Blinky.cbuild-mlops.yml --probe --methods march_half
```

`CMakeLists.txt` supplies `check_model` and `create_ai_layer` convenience targets for the Python stages. Firmware compilation is performed by CMSIS Solution's generated CMake build and the selected pack components. **DevKit-E8@Release** adds `-O3 -ffast-math` to C and C++ compilation, including the MCU camera/tile orchestration and runtime IO conversion. Debug retains its separate optimization settings.

## Tiles and rendering variants

Edit `model/render_config.json` to set the physical display dimensions, `rotation_degrees` (0 or 90 clockwise), and logical tile width/height. Dimensions are positive even integers. The default is an 80 x 8 output tile over an 800 x 480 logical image on a 480 x 800 display. The camera uses the logical image's aspect ratio. Both resolution modes rotate during RGB565 assembly, without an extra image buffer or Ethos invocation. Increasing tile size reduces invocation overhead but increases working memory; dimensions must fit the target's CPU and NPU memory.

Changing only `rotation_degrees` preserves network tensor shapes and can reuse the existing PTE. Run `uv run --active --no-project python scripts/update_display_config.py` to update generated orientation metadata, then rebuild firmware. The script rejects changes that require re-export. Future exports calibrate the selected logical camera aspect; the currently retained PTE was calibrated with the original portrait view.

Full resolution evaluates 80 x 8 rays per tile. Half resolution evaluates a 42 x 6 native tile: a 40 x 4 core with a one-pixel halo on every side. The NPU bilinear layer enlarges it to 84 x 12, then firmware crops the two-pixel enlarged halo to produce the same 80 x 8 output rectangle. Partial tiles at the right/bottom edges are clipped when copied to the frame.

Tile position is a runtime camera input. `rays(width, height, row=..., rows=..., col=..., cols=...)` and `render(...)` use coordinates normalized by the complete image dimensions, so tiles preserve camera projection and checker footprints. The firmware visits rectangles across the whole image; the network shape is reused for every position. Image-edge halos duplicate the nearest valid pixel.

Left selects half resolution; right selects full resolution; center freezes/resumes animation. Mode and time are captured for the whole frame. The debugger can also set `sdf_half_resolution`, `sdf_animate` and `sdf_time`.

## Tensor stages and precision

The program has full/half versions of ray marching, normals, AO, shadow marching and shading, plus a 2x bilinear upscale method. Every SDF and shading stage processes all pixels of a tile. Pixel branches become tensor masks (`torch.where`, comparisons and clipping). Both selected and ignored paths are computed. The primitive bounding-box branches are removed because masks would not save their work. The host skips remaining ray, AO and shadow iterations when every pixel in a tile is inactive, and skips the normals stage for a tile containing only sky/floor. In particular, sky-only tiles skip all AO scene evaluations.

The shader's 22 primitive instances, 70 ray steps, five AO samples and two 24-step shadow passes are retained. Four tetrahedral normal samples use the original 0.0005 spacing; `normal_epsilon` is configurable for quantization experiments. The normal samples are packed into one wider image tensor so the exported graph contains a single scene evaluation over all four samples. Material sine values are precomputed because material IDs are constant. Camera/ray setup, analytic floor checker footprints, final format conversion and iteration orchestration run on M55_HP. Supersampling anti-aliasing is disabled; the shader's analytic checker filtering is retained.

Geometry uses 16-bit activations and the upscale stage uses 8-bit activations. The first integration intentionally keeps float application IO boundaries to make accuracy and timing measurable. Shader tensor operations, including masks and clipping, must execute on Ethos. The exporter rejects CPU shader fallbacks, multiple partial delegates and integer/float delegate IO mismatches. Consult `ai_layer/delegation.json` (or `out/export/delegation.json` for a probe) for the validated partitioning.

Quantization is experimental. With graph normalization and explicit scale sharing, a 32 x 53 PT2E render at the held-out camera time 7 produced finite output but only 11.4 dB PSNR versus float (MAE 0.149, maximum error 0.863), with substantial normal/lighting and geometry error. The original normal sample spacing is particularly demanding for integer arithmetic. This is a host quantization result, not an on-board quality/performance measurement. The shader is not yet visually faithful after quantization; later export or calibration changes can change that result.

## Checks and measurements

```powershell
uv run --active --no-project python tests/test_tiles.py
uv run --active --no-project python tests/test_export_contract.py
uv run --active --no-project python model/validate.py --width 96
uv run --active --no-project python model/validate.py --width 32 --quantized
```

Tile checks cover exact global rays, float render agreement, landscape calibration dimensions, clockwise corner placement, and complete rotated bilinear tile coverage including partial edge tiles. Previews and host quality metrics go to `out/validation/`. `float.png` and `pt2e.png` show the logical view; the corresponding `_display.png` files show physical LCD orientation. Use `--output-dir` to preserve an earlier comparison.

`app_stage`, `app_error`, `app_service_error`, `frames_presented`, `npu_interrupts`, `npu_error` and `sdf_metrics` are debugger-visible. The metrics separate complete render time from model-call time and count tile, ray and shadow invocations. Display waiting is excluded from `render_ms`; model loading/switching and float quantization boundaries are included in `inference_ms`.

The Release build passes. Before the landscape rotation, live validation of the normalized program completed and submitted both variants at camera time 0, with animation frozen and no application/model errors. Each portrait frame contains 600 tiles; every model call has a matching NPU interrupt. These pre-rotation measurements are recorded in `out/validation/hardware.json`; they are not landscape timings.

| Variant | Render time | Model-call time | NPU calls | FPS |
| --- | ---: | ---: | ---: | ---: |
| Half resolution + Ethos upscale | 112.000 s | 93.239 s | 50,262 | 0.0089 |
| Full resolution | 196.540 s | 172.471 s | 49,150 | 0.0051 |

The 90-degree landscape view also completed both modes with no errors and 600 tiles per frame. The user confirmed the wider view and orientation on the LCD. `out/validation/hardware_landscape.json` records the following results at camera time 0 using the existing PTE:

| Variant | Render time | Model-call time | NPU calls |
| --- | ---: | ---: | ---: |
| Landscape half + upscale, before explicit fast-math flags | 121.702 s | 101.260 s | 54,418 |
| Landscape full, before explicit fast-math flags | 213.313 s | 187.377 s | 53,186 |
| Landscape half + upscale, Release `-O3 -ffast-math`, animation enabled | 121.482 s | 101.140 s | 54,418 |

The optimized half frame completed without errors, with one NPU interrupt per model call. Animation advanced `sdf_time` to 121.483 seconds and continued after removing the measurement breakpoint. The fast-math build did not materially improve frame time: roughly 101 of the 121 seconds remain inside model calls. The board is left running half-resolution animation for visual inspection; updates take about two minutes at this camera pose.

This direct tensor translation runs on Ethos but is too slow for animation. Shader quality also remains poor after quantization, as the host comparison above shows. No anti-aliasing is enabled. These figures include orchestration and application IO conversion, rather than Vela estimates of an isolated delegate.

Two earlier runtime blockers are fixed: SRAM1 NPU scratch caused a bus abort, and partial delegation produced int16 outputs with Float destinations. Scratch now uses SRAM0, and the normalized export retains correctly typed integer delegate boundaries with CPU IO Q/DQ. The exporter rejects CPU shader fallback and IO mismatches before publication.

When the debugger shows only `??` frames, check the GDB-server log's discovered processor rather than trusting the launch configuration's name. A session named M55_HP can connect to M55_HE if the HP access port is unavailable. In the observed failure the server discovered only M55_HE, the live vector table was at `0x80000000` instead of the renderer's `0x80200000`, and renderer state in DTCM was uninitialized. Stop debugging and power-cycle the board before retrying the M55_HP session.

In Release, one source breakpoint in `fail()` expands to many inlined locations and can exhaust hardware breakpoints even when VS Code shows only two source entries. A breakpoint at `++frames_presented` is a single location suitable for frame measurements. Pause to inspect `app_error` if rendering does not complete.
