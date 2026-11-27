$ErrorActionPreference = 'Stop'
$version = '0.4.0'
$bundle = Get-ChildItem win-build -Recurse -Directory -Filter 'cs80maths.vst3' | Where-Object { Test-Path (Join-Path $_.FullName 'Contents/x86_64-win/cs80maths.vst3') } | Select-Object -First 1
if (!$bundle) { throw 'Built VST3 bundle not found' }
$stage = Join-Path $PWD 'win-release/cs80maths-Windows'
New-Item -ItemType Directory -Force $stage | Out-Null
Copy-Item $bundle.FullName $stage -Recurse -Force
Copy-Item 'scripts/installers/*.cmd' $stage
Copy-Item LICENSE "$stage/LICENSE.txt"
Copy-Item 'docs/RELEASE-NOTES.md' "$stage/Release notes.md"
Copy-Item 'docs/WINDOWS.md' "$stage/Read me.md"
Copy-Item 'sdk/LICENSE.txt' "$stage/VST3 SDK license.txt"
# Test installation and removal in an isolated directory, never a runner plugin location.
$env:CS80MATHS_TEST_ROOT = Join-Path $PWD 'win-release/install-test'
$env:CS80MATHS_TEST_MODE = '1'
New-Item -ItemType Directory -Force "$env:CS80MATHS_TEST_ROOT/presets" | Out-Null
Set-Content "$env:CS80MATHS_TEST_ROOT/presets/keep.txt" 'User data'
& "$stage/Install cs80maths.cmd"
if ($LASTEXITCODE -ne 0) { throw 'Installer failed' }
$installed = "$env:CS80MATHS_TEST_ROOT/VST3/Direct Grooves/cs80maths.vst3"
if (!(Test-Path "$installed/Contents/x86_64-win/cs80maths.vst3")) { throw 'Plugin not installed' }
& "$stage/Uninstall cs80maths.cmd"
if ($LASTEXITCODE -ne 0 -or (Test-Path $installed)) { throw 'Uninstaller failed' }
if (!(Test-Path "$env:CS80MATHS_TEST_ROOT/presets/keep.txt")) { throw 'Uninstaller removed user data' }
'PASS Windows installer/uninstaller in isolated directory; user data retained.' | Tee-Object -FilePath win-release/installer-test.txt
Compress-Archive -Path $stage -DestinationPath "win-release/cs80maths-$version-Windows-x64-VST3.zip" -Force
Get-FileHash "win-release/cs80maths-$version-Windows-x64-VST3.zip" -Algorithm SHA256 | Format-List | Out-File win-release/SHA256.txt
