param([string]$PackRoot = $env:CMSIS_PACK_ROOT)
$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    New-Item -ItemType Directory -Force out | Out-Null
    $dsp = Join-Path $PackRoot 'ARM/CMSIS-DSP/1.18.0'
    $sources = @(
        'TransformFunctions/arm_rfft_fast_init_f32.c', 'TransformFunctions/arm_rfft_fast_f32.c',
        'TransformFunctions/arm_cfft_init_f32.c', 'TransformFunctions/arm_cfft_f32.c',
        'TransformFunctions/arm_cfft_radix8_f32.c', 'TransformFunctions/arm_bitreversal2.c',
        'CommonTables/arm_common_tables.c', 'CommonTables/arm_const_structs.c',
        'WindowFunctions/arm_hanning_f32.c', 'SupportFunctions/arm_q15_to_float.c',
        'StatisticsFunctions/arm_mean_f32.c', 'StatisticsFunctions/arm_max_f32.c', 'BasicMathFunctions/arm_offset_f32.c',
        'BasicMathFunctions/arm_mult_f32.c', 'BasicMathFunctions/arm_scale_f32.c',
        'ComplexMathFunctions/arm_cmplx_mag_f32.c', 'SupportFunctions/arm_fill_q15.c'
    ) | ForEach-Object { Join-Path $dsp "Source/$_" }
    & clang -std=c11 -O3 -ffast-math -D_CRT_SECURE_NO_WARNINGS -I M55_HP -I "$dsp/Include" -I "$dsp/PrivateInclude" -I "$PackRoot/ARM/CMSIS/6.3.0/CMSIS/Core/Include" tests/test_spectrum.c M55_HP/spectrum.c M55_HP/spectrum_ui.c @sources -o out/test_spectrum.exe
    if ($LASTEXITCODE -ne 0) { throw 'Host compilation failed' }
    & ./out/test_spectrum.exe
    if ($LASTEXITCODE -ne 0) { throw 'Spectrum checks failed' }
} finally {
    Pop-Location
}
