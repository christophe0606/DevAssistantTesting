$ErrorActionPreference = 'Stop'
Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    $manifest = Get-Content third_party/xscreensaver/inventory.json -Raw | ConvertFrom-Json
    New-Item -ItemType Directory -Force out/xscreensaver-port/syntax | Out-Null
    $failures = 0
    foreach ($entry in $manifest.cpu) {
        $source = 'third_party/xscreensaver/' + $entry.source
        $ErrorActionPreference = 'Continue'
        $diagnostics = & clang -fsyntax-only -std=gnu11 -Wno-everything -D_CRT_SECURE_NO_WARNINGS -DSTANDALONE -DHAVE_CONFIG_H -I M55_HP/xs_port -I third_party/xscreensaver/hacks -I third_party/xscreensaver/utils -I third_party/xscreensaver/jwxyz $source 2>&1
        $ErrorActionPreference = 'Stop'
        $code = $LASTEXITCODE
        [IO.File]::WriteAllText((Join-Path (Get-Location) "out/xscreensaver-port/syntax/$($entry.name).log"), ($diagnostics -join "`n"))
        if ($code -ne 0) { ++$failures; Write-Output "FAIL $($entry.name): $($diagnostics | Select-Object -First 1)" }
    }
    Write-Output "$failures / $($manifest.cpu.Count) modules failed syntax"
} finally { Pop-Location }
