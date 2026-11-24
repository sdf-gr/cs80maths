#!/usr/bin/env python3
"""Package macOS/source and a verified Windows artifact; stage only in the local site."""
import argparse, hashlib, json, plistlib, re, shutil, sqlite3, subprocess, tempfile, zipfile
from pathlib import Path
root=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--site',type=Path,help='Local website new/ directory');p.add_argument('--windows',type=Path,required=True,help='Successful remote Windows release ZIP');args=p.parse_args()
app=root/'dist/cs80maths.app';version=plistlib.loads((app/'Contents/Info.plist').read_bytes())['CFBundleShortVersionString']
windows=args.windows.resolve();assert windows.name==f'cs80maths-{version}-Windows-x64-VST3.zip','Windows version mismatch'
with zipfile.ZipFile(windows) as z:
 names=z.namelist()
 for suffix in ['Install cs80maths.cmd','Uninstall cs80maths.cmd','Contents/x86_64-win/cs80maths.vst3']:
  assert any(n.endswith(suffix) for n in names),'Incomplete Windows package: '+suffix
out=root/'dist/releases';out.mkdir(exist_ok=True)
with tempfile.TemporaryDirectory(prefix='cs80maths-release-') as tmp:
 stage=Path(tmp)/'cs80maths';stage.mkdir()
 shutil.copy2(root/'LICENSE',stage/'LICENSE.txt');shutil.copy2(root/'docs/RELEASE-NOTES.md',stage/'Release notes.md')
 shutil.copy2(root/'scripts/installers/Uninstall cs80maths.command',stage/'Uninstall cs80maths.command')
 installer=stage/'Install cs80maths.command'
 installer.write_text('''#!/bin/zsh
set -euo pipefail
cd -- "${0:A:h}"
mkdir -p "$HOME/Applications"
target="$HOME/Applications/cs80maths.app"
/usr/bin/codesign --verify --deep --strict "cs80maths.app"
if [[ -e "$target" ]]; then
  /usr/bin/ditto "$target" "$HOME/Applications/cs80maths-backup-$(date +%Y%m%d-%H%M%S).app"
fi
/usr/bin/ditto "cs80maths.app" "$target"
/usr/bin/codesign --verify --deep --strict "$target"
uninstall_home="$HOME/Library/Application Support/Direct Grooves/cs80maths"
mkdir -p "$uninstall_home"
cp "Uninstall cs80maths.command" "$uninstall_home/Uninstall cs80maths.command"
chmod +x "$uninstall_home/Uninstall cs80maths.command"
echo "Installed cs80maths. Open it once from Applications, then restart/rescan your Audio Unit host."
echo "Uninstaller included in this download and copied to $uninstall_home."
echo "This installer does not change macOS security settings."
''');installer.chmod(0o755)
 mac_readme=f'''cs80maths {version} — Direct Grooves\n\nmacOS 13+, Apple Silicon and Intel. Quit the old app and your DAW before installation. Copy cs80maths.app to Applications, or run Install cs80maths.command to install in your home Applications folder with a backup of an existing copy. Open the app once to discover its embedded AUv3; reopen/rescan your DAW and look for Direct Grooves: cs80maths. No separate .component file is needed. Keep the app bundle intact.\n\nAn uninstaller is included: Uninstall cs80maths.command. Close the app and DAW, run it and type YES. It moves recognized cs80maths apps from ~/Applications and /Applications to the Trash, removing the embedded AU along with them. Presets/settings, backups and copies elsewhere are retained. The installer also saves the uninstaller in ~/Library/Application Support/Direct Grooves/cs80maths. If a system-wide copy cannot be moved, move it to Trash with Finder.\n\nFor standalone use, choose your MIDI input and click Start Audio. APC64 polyphonic pressure is supported.\n\nThis build is ad-hoc signed and not notarized. Only open software you trust; see https://support.apple.com/guide/mac-help/open-a-mac-app-from-an-unidentified-developer-mh40616/mac . Scripts do not bypass security checks.\n\nSource is free under MIT. This is an uncalibrated software approximation, independent of Yamaha.\n'''
 (stage/'Read me.txt').write_text(mac_readme)
 subprocess.run(['ditto',str(app),str(stage/'cs80maths.app')],check=True)
 binary=out/f'cs80maths-{version}-macOS.zip';subprocess.run(['ditto','-c','-k','--sequesterRsrc','--keepParent',str(stage),str(binary)],check=True)
 source=out/f'cs80maths-{version}-source.zip'
 with zipfile.ZipFile(source,'w',zipfile.ZIP_DEFLATED) as z:
  for entry in ['CS80','CS80.xcodeproj','Windows','CMakeLists.txt','.github/workflows','scripts','docs','assets','README.md','LICENSE','.gitignore']:
   path=root/entry
   for f in ([path] if path.is_file() else sorted(path.rglob('*'))):
    if f.is_file() and not any(x in f.parts for x in ['xcuserdata','__pycache__']) and f.name!='.DS_Store':z.write(f,Path('cs80maths-source')/f.relative_to(root))
 assets=[]
 def asset(f,title):
  digest=hashlib.sha256(f.read_bytes()).hexdigest();meta=dict(title=title,version=version,original_name=f.name,file_size=f.stat().st_size,sha256=digest,filename=f'{f.stem}-{digest[:16]}{f.suffix}');assets.append((f,meta));return meta
 downloads=[asset(source,'Free complete source code · MIT license'),asset(binary,'macOS app + Audio Unit · Uninstaller included'),asset(windows,'Windows x64 VST3 · Installer + uninstaller included'),asset(root/'docs/cs80-electronics.html','Text-only essay · How the CS-80 works (HTML)')]
 photos=[]
 for filename,caption,order in [('cs80maths-screenshot.png','cs80maths fader interface · macOS screenshot',1),('cs80maths-logo.png','App logo',0)]:
  f=root/'assets'/filename;digest=hashlib.sha256(f.read_bytes()).hexdigest();photos.append(dict(filename=f'{f.stem}-{digest[:16]}.png',sha256=digest,caption=caption,order=order))
 notes=(root/'docs/RELEASE-NOTES.md').read_text().split('\n\n',2)[2]
 windows_instructions='\n'.join(re.sub(r'^#{1,6}\s+', '', line) for line in (root/'docs/WINDOWS.md').read_text().split('## Build from free source')[0].splitlines()).replace('`','').replace('**','')
 manifest=dict(name='cs80maths',summary='Free mathematical CS-80-inspired synthesizer from Direct Grooves for macOS and Windows. Both downloads include an uninstaller.',version=version,platform='macOS 13+ · Apple Silicon + Intel · AUv3 + standalone | Windows 10/11 x64 · VST3 instrument',order=12,store_url='https://directgrooves.com/releases/cs80maths/how-the-cs80-works.html',store_label='Read the essay: how the CS-80 works',description='cs80maths is a free, open-source synthesizer from Direct Grooves. Sound is generated mathematically rather than played from sampled waveform recordings.\n\nFEATURES\n• Eight-note polyphony with two independent synthesizer layers.\n• Mathematical saw, pulse and sine oscillators, PWM and a noise source.\n• High-pass and low-pass filters, envelopes, ring modulation, ribbon and stereo effects.\n• Colored vertical faders, both voice panels, tone selectors and four editable memories.\n• Velocity, polyphonic aftertouch, channel pressure, MIDI fader mappings and optional MPE compatibility.\n• Eight original cinematic presets, including Horizon Brass.\n• macOS standalone app and AUv3; Windows x64 VST3 with a native fader editor. Windows requires a VST3 DAW and has no standalone app.\n• Installer and uninstaller included in BOTH platform downloads. Uninstallers keep user presets and settings.\n\nWHAT’S NEW\n'+notes+'\n\nFREE SOURCE AND ESSAY\nComplete original source, Xcode and CMake projects, remote Windows build workflow, tests, installers and uninstallers are included under MIT. The VST3 SDK is fetched at a pinned revision and its license is retained. The text-only essay describes the original CS-80 electronics and connections. Yamaha manuals are not redistributed.\n\nVALIDATION AND LIMITS\nmacOS passed 32 offline checks and Apple Audio Unit validation. The Windows build runs on GitHub’s Windows runner with Steinberg’s validator, wrapper/editor tests and an isolated install/uninstall check. This is not a listening comparison in your particular Windows DAW. Intel macOS was compiled and signature-checked, not tested on Intel hardware. Windows is x64, not ARM or 32-bit. The instrument remains an uncalibrated software approximation. Windows and macOS editors differ in layout; both use the same synthesis engine.',install_instructions='MACOS\n'+mac_readme+'\nWINDOWS\n'+windows_instructions,downloads=downloads,photos=photos,replace_managed_assets=True)
 (out/'software.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False)+'\n')
 if args.site:
  site=args.site.resolve();dbpath=site/'data/site.sqlite'
  with sqlite3.connect(dbpath) as db:
   row=db.execute('select id from software where name=?',('cs80maths',)).fetchone()
   appid=row[0] if row else db.execute('insert into software (name,summary) values (?,?)',('cs80maths',manifest['summary'])).lastrowid
  upload=site/'uploads/software'/str(appid);upload.mkdir(parents=True,exist_ok=True)
  for f,meta in assets:shutil.copy2(f,upload/meta['filename'])
  photo_dir=site/'uploads/photos/software'/str(appid);photo_dir.mkdir(parents=True,exist_ok=True)
  for name,meta in zip(['cs80maths-screenshot.png','cs80maths-logo.png'],photos):shutil.copy2(root/'assets'/name,photo_dir/meta['filename'])
  release=site/'releases/cs80maths';release.mkdir(parents=True,exist_ok=True);shutil.copy2(out/'software.json',release/'software.json');shutil.copy2(root/'docs/cs80-electronics.html',release/'how-the-cs80-works.html')
  subprocess.run(['python3',str(site/'scripts/sync-software-manifests.py'),str(dbpath),str(site/'releases'),'cs80maths'],check=True)
  with sqlite3.connect(dbpath) as db:
   filenames=[x['filename'] for x in downloads];db.execute('delete from software_files where software_id=? and filename not in ('+','.join('?'*len(filenames))+')',[appid,*filenames])
  print(f'Updated local software #{appid}; deploy using website deploy.sh after local checks.')
 print('Packaged cs80maths '+version)
