$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$upstream = Join-Path $root 'third_party/xscreensaver'
$inventoryPath = Join-Path $upstream 'inventory.json'
$previous = if (Test-Path -LiteralPath $inventoryPath) { Get-Content -LiteralPath $inventoryPath -Raw | ConvertFrom-Json } else { $null }
$makeText = [IO.File]::ReadAllText((Join-Path $upstream 'hacks/Makefile.in'))
$exeText = [regex]::Match($makeText, '(?ms)^EXES\s*=\s*(.*?)(?=^JPEG_EXES)').Groups[1].Value
$names = @($exeText.Replace('\', ' ') -split '\s+' | Where-Object { $_ -and $_ -notmatch '^@' -and $_ -ne 'xscreensaver-getimage' })
$retiredText = [regex]::Match($makeText, '(?ms)^RETIRED_EXES\s*=\s*(.*?)(?=^HACK_OBJS_1)').Groups[1].Value
$retiredNames = @($retiredText.Replace('\', ' ') -split '\s+' | Where-Object { $_ })
$names += $retiredNames
$names += 'mismunch'
$entries = foreach ($name in $names) {
    $source = if ($name -eq 'apple2') { 'apple2-main.c' } elseif ($name -eq 'mismunch') { 'munch.c' } else { "$name.c" }
    $path = Join-Path $upstream "hacks/$source"
    if (!(Test-Path -LiteralPath $path)) { throw "Missing upstream source for $name" }
    $prior = if ($previous) { $previous.cpu | Where-Object { $_.name -eq $name } | Select-Object -First 1 } else { $null }
    [pscustomobject]@{ name=$name; source="hacks/$source"; kind=$(if ($name -in $retiredNames) {'retired-cpu'} else {'cpu'}); status=$(if ($prior) { $prior.status } else { 'pending' }) }
}
$allNames = @(Get-ChildItem (Join-Path $upstream 'hacks/config') -Filter *.xml | ForEach-Object { $_.BaseName })
$excluded = @($allNames | Where-Object { $_ -notin $names -and $_ -notin @('webcollage', 'vidwhacker') })
$manifest = [ordered]@{ version='6.16'; source='https://www.jwz.org/xscreensaver/xscreensaver-6.16.tar.gz'; cpu=$entries; scripts=@('webcollage','vidwhacker'); excluded=$excluded }
if ($previous.validation) { $manifest.validation = $previous.validation }
$reasons = [ordered]@{}
foreach ($name in $excluded) {
    $xml = [IO.File]::ReadAllText((Join-Path $upstream "hacks/config/$name.xml"))
    $reasons[$name] = if ($xml.Contains('gl="yes"')) { 'OpenGL/GPU renderer' } else { 'upstream X11 drawing diagnostic' }
}
$manifest.exclusion_reasons = $reasons
[IO.File]::WriteAllText((Join-Path $upstream 'inventory.json'), ($manifest | ConvertTo-Json -Depth 5) + "`n")
Write-Output "$($names.Count) CPU modules, 2 scripts, $($excluded.Count) other catalog entries"
