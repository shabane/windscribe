#!/bin/bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_PATH="$DIR/build/src/client/Windscribe.app"

if [ "$EUID" -ne 0 ]; then
  echo "❌ لطفاً این اسکریپت را با دسترسی sudo اجرا کنید:"
  echo "   sudo ./install_dev.sh"
  exit 1
fi

echo "🚀 در حال نصب Windscribe (با پشتیبانی sing-box)..."

if [ ! -d "$APP_PATH" ]; then
  echo "❌ مسیر $APP_PATH یافت نشد. لطفاً مطمئن شوید بیلد کامل شده است."
  exit 1
fi

# 1. بستن پروسه‌های باز قبلی
echo "🛑 در حال بستن پروسه‌های قبلی..."
pkill -f Windscribe 2>/dev/null || true
pkill -f windscribesingbox 2>/dev/null || true

# 2. کپی اپلیکیشن به /Applications
echo "📦 در حال کپی Windscribe.app به /Applications..."
rm -rf /Applications/Windscribe.app
cp -R "$APP_PATH" /Applications/

# 3. به‌روزرسانی Privileged Helper Tool و باینری sing-box در سیستم
echo "⚙️ در حال به‌روزرسانی Helper Tool و باینری‌ها..."
mkdir -p /Library/PrivilegedHelperTools

cp "$APP_PATH/Contents/Library/LaunchServices/com.windscribe.helper.macos" /Library/PrivilegedHelperTools/
chmod 755 /Library/PrivilegedHelperTools/com.windscribe.helper.macos
chown root:wheel /Library/PrivilegedHelperTools/com.windscribe.helper.macos

cp "$APP_PATH/Contents/Helpers/windscribesingbox" /Library/PrivilegedHelperTools/
chmod 755 /Library/PrivilegedHelperTools/windscribesingbox
chown root:wheel /Library/PrivilegedHelperTools/windscribesingbox

# ری‌استارت سرویس در launchd
launchctl kickstart -k system/com.windscribe.helper.macos 2>/dev/null || true

# 4. رفع محدودیت Gatekeeper / Quarantine
xattr -cr /Applications/Windscribe.app 2>/dev/null || true

echo ""
echo "✅ نصب با موفقیت به پایان رسید!"
echo "اکنون می‌توانید نرم‌افزار را اجرا کنید:"
echo "   open /Applications/Windscribe.app"
