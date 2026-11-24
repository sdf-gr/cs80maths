#!/bin/zsh
set -euo pipefail
echo "Quit cs80maths and your DAW before uninstalling."
echo "This moves installed cs80maths apps to the Trash and keeps presets/settings."
if [[ "${CS80MATHS_TEST_MODE:-}" != 1 ]]; then
 read 'reply?Uninstall cs80maths? Type YES to continue: '
 [[ "$reply" == YES ]] || exit 0
fi
app_home="${CS80MATHS_TEST_HOME:-$HOME}"
mkdir -p "$app_home/.Trash"
for target in "$app_home/Applications/cs80maths.app" "${CS80MATHS_TEST_SYSTEM_APPS:-/Applications}/cs80maths.app"; do
 [[ -d "$target" ]] || continue
 identifier=$(/usr/libexec/PlistBuddy -c 'Print :CFBundleIdentifier' "$target/Contents/Info.plist" 2>/dev/null || true)
 [[ "$identifier" == com.djazz.cs80 ]] || { echo "Skipped unrecognized app: $target"; continue; }
 destination="$app_home/.Trash/cs80maths-uninstalled-$(date +%Y%m%d-%H%M%S)-$RANDOM.app"
 if mv -- "$target" "$destination"; then echo "Moved to Trash: $target"; else echo "Could not move $target. Move it to the Trash manually using Finder."; exit 1; fi
done
echo "Done. The embedded Audio Unit was removed with the app. Restart/rescan your DAW."
echo "Copies kept elsewhere and user presets/settings were left alone."
