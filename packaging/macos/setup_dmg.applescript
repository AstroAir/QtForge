-- QtPlugin DMG Setup Script
-- Configures the appearance of the DMG installer

tell application "Finder"
    tell disk "QtPlugin"
        open
        set current view of container window to icon view
        set toolbar visible of container window to false
        set statusbar visible of container window to false
        set the bounds of container window to {400, 100, 920, 440}
        set viewOptions to the icon view options of container window
        set arrangement of viewOptions to not arranged
        set icon size of viewOptions to 72
        set background picture of viewOptions to file ".background:dmg_background.png"
        
        -- Position icons
        set position of item "QtPlugin" to {160, 205}
        set position of item "Applications" to {360, 205}
        
        -- Hide background image file
        set the extension hidden of item ".background" to true
        
        close
        open
        update without registering applications
        delay 2
    end tell
end tell
