- EmuFramework\include\emuframework
	- [1.1 AppKeyCode.hh](#AppKeyCode_hh)
	- [1.2 Cheats.hh](#Cheats_hh)
	- [1.3 DataPathSelectView.hh](#DataPathSelectView_hh)
	- [1.4 EmuAppInlines.hh](#EmuAppInlines_hh)
	- [1.5 keyRemappingUtils.hh](#keyRemappingUtils_hh)
	- [1.6 UserPathSelectView.hh](#UserPathSelectView_hh)
	- [1.7 VController.hh](#VController_hh)
	- [1.8 viewUtils.hh](#viewUtils_hh)
- EmuFramework\src
	- [2.1 AutosaveManager.cc](#AutosaveManager_cc)
	- [2.2 EmuApp.cc](#EmuApp_cc)
	- [2.3 EmuInput.cc](#EmuInput_cc)
	- [2.4 EmuSystem.cc](#EmuSystem_cc)
	- [2.5 InputDeviceConfig.cc](#InputDeviceConfig_cc)
- EmuFramework\src\gui
	- [3.1 AudioOptionView.cc](#AudioOptionView_cc)
	- [3.2 AutosaveSlotView.cc](#AutosaveSlotView_cc)
	- [3.3 BundledGamesView.cc](#BundledGamesView_cc)
	- [3.4 ButtonConfigView.cc](#ButtonConfigView_cc)
	- [3.5 CPUAffinityView.cc](#CPUAffinityView_cc)
	- [3.6 FilePathOptionView.cc](#FilePathOptionView_cc)
	- [3.7 FrameTimingView.cc](#FrameTimingView_cc)
	- [3.8 GUIOptionView.cc](#GUIOptionView_cc)
	- [3.9 InputManagerView.cc](#InputManagerView_cc)
	- [3.10 InputOverridesView.cc](#InputOverridesView_cc)
	- [3.11 LoadProgressView.cc](#LoadProgressView_cc)
	- [3.12 MainMenuView.cc](#MainMenuView_cc)
	- [3.13 PlaceVControlsView.cc](#PlaceVControlsView_cc)
	- [3.14 PlaceVideoView.cc](#PlaceVideoView_cc)
	- [3.15 ProfileSelectView.hh](#ProfileSelectView_hh)
	- [3.16 RecentContentView.cc](#RecentContentView_cc)
	- [3.17 ResetAlertView.hh](#ResetAlertView_hh)
	- [3.18 StateSlotView.cc](#StateSlotView_cc)
	- [3.19 SystemActionsView.cc](#SystemActionsView_cc)
	- [3.20 SystemOptionView.cc](#SystemOptionView_cc)
	- [3.21 TouchConfigView.cc](#TouchConfigView_cc)
	- [3.22 VideoOptionView.cc](#VideoOptionView_cc)
- [EmuFramework\src\shared\mednafen-emuex\MDFNUtils.hh](#MDFNUtils_hh)


<h2 id="AppKeyCode_hh">1.1 EmuFramework\include\emuframework\AppKeyCode.hh</h2>

UI_TEXT("In-Emulation Actions")

UI_TEXT("Open Menu")

UI_TEXT("Toggle Fast-forward")

UI_TEXT("Rewind One State")


<h2 id="Cheats_hh">1.2 EmuFramework\include\emuframework\Cheats.hh</h2>

UI_TEXT("Cheats")

UI_TEXT("Add/Edit")

UI_TEXT("Edit Cheats")

UI_TEXT("Input description")

UI_TEXT("A cheat with name already exists")

UI_TEXT("Input description")

UI_TEXT("A cheat with name already exists")	

UI_TEXT("Delete")

UI_TEXT("Really delete this cheat?")


<h2 id="DataPathSelectView_hh">1.3 EmuFramework\include\emuframework\DataPathSelectView.hh</h2>

UI_TEXT("Select Folder")

UI_TEXT("Select File")	

UI_TEXT("Select Archive File")

UI_TEXT("File doesn't have a valid extension")	

UI_TEXT("Unset")


<h2 id="EmuAppInlines_hh">1.4 EmuFramework\include\emuframework\EmuAppInlines.hh</h2>

UI_TEXT(u"" CONFIG_APP_NAME " " IMAGINE_VERSION)


<h2 id="keyRemappingUtils_hh">1.5 EmuFramework\include\emuframework\keyRemappingUtils.hh</h2>

UI_TEXT("PC Keyboard")

UI_TEXT("Generic Gamepad")

UI_TEXT("PS3 Controller")

UI_TEXT("OUYA Controller")

UI_TEXT("Xperia Play")

UI_TEXT("Pandora Keys")

UI_TEXT("Default")

UI_TEXT("Default")

UI_TEXT("Default")

UI_TEXT("Default")

UI_TEXT("Default")

UI_TEXT("Default")


<h2 id="UserPathSelectView_hh">1.6 EmuFramework\include\emuframework\UserPathSelectView.hh</h2>

UI_TEXT("Select Folder")

UI_TEXT("Same As Content")

UI_TEXT("Same As Saves")

UI_TEXT("Content Folder")

UI_TEXT("Saves Folder")

UI_TEXT("Cheats: {}")

UI_TEXT("Patches: {}")

UI_TEXT("Palettes: {}")


<h2 id="VController_hh">1.7 EmuFramework\include\emuframework\VController.hh</h2>

UI_TEXT("D-Pad")


<h2 id="viewUtils_hh">1.8 EmuFramework\include\emuframework\viewUtils.hh</h2>

UI_TEXT("Enter a value")

UI_TEXT("Value not in range")

UI_TEXT("Values not in range")	


<h2 id="AutosaveManager_cc">2.1 EmuFramework\src\AutosaveManager.cc</h2>

UI_TEXT("Save folder inaccessible, please set it in Options➔File Paths➔Saves")	

UI_TEXT("Error writing autosave state")	

UI_TEXT("Error loading autosave state:\n{}")

UI_TEXT("No Save")

UI_TEXT("Main")


<h2 id="EmuApp_cc">2.2 EmuFramework\src\EmuApp.cc</h2>

UI_TEXT("System Actions")
	
UI_TEXT("Really Exit? (Push Back/Escape again to confirm)")

UI_TEXT("Yes")

UI_TEXT("No")

UI_TEXT("Close Menu")

UI_TEXT("{} was suspended")

UI_TEXT("{} {}")

UI_TEXT("connected")

UI_TEXT("disconnected")

UI_TEXT("{} had a connection error")

UI_TEXT("Autosave state timestamp is older than the contents of backup memory, really load it even though progress may be lost?")

UI_TEXT("Can't access path name for:\n{}")

UI_TEXT("Not enough memory for rewind states")	

UI_TEXT("This option takes effect next time the system starts. Restart it now?")

UI_TEXT("{}{}")

UI_TEXT("Wrote screenshot at ")

UI_TEXT("Error writing screenshot at ")

UI_TEXT("File doesn't have a valid extension")

UI_TEXT("Error opening {}, please verify save path has write access")

UI_TEXT("System not running")

UI_TEXT("State Saved")

UI_TEXT("Can't save state:\n{}")

UI_TEXT("System not running")

UI_TEXT("Save folder inaccessible, please set it in Options➔File Paths➔Saves")

UI_TEXT("Can't load state:\n{}")

UI_TEXT("Really close current content?")


<h2 id="EmuInput_cc">2.3 EmuFramework\src\EmuInput.cc</h2>

UI_TEXT("Really Overwrite State?")

UI_TEXT("State Slot: {}")

UI_TEXT("State Slot: {}")

UI_TEXT("Really Exit?")

UI_TEXT("Please set rewind states in Options➔System")

UI_TEXT(" + ")

UI_TEXT(" (Turbo Toggle)")

UI_TEXT(" (Turbo)")

UI_TEXT(" (Toggle)")

UI_TEXT("Open Content")

UI_TEXT("Close Content")

UI_TEXT("Open System Actions")

UI_TEXT("Save State")

UI_TEXT("Load State")

UI_TEXT("Decrement State Slot")

UI_TEXT("Increment State Slot")

UI_TEXT("Fast-forward")

UI_TEXT("Take Screenshot")

UI_TEXT("Open Menu")

UI_TEXT("Toggle Fast-forward")

UI_TEXT("Turbo Modifier")

UI_TEXT("Exit App")

UI_TEXT("Slow-motion")

UI_TEXT("Toggle Slow-motion")

UI_TEXT("Rewind One State")

UI_TEXT("Soft Reset")

UI_TEXT("Hard Reset")

UI_TEXT("Open Reset Menu")


<h2 id="EmuSystem_cc">2.4 EmuFramework\src\EmuSystem.cc</h2>

UI_TEXT("Invalid state size from header")

UI_TEXT("Error uncompressing state")

UI_TEXT("Invalid state size")

UI_TEXT("No recognized file extensions in archive")

UI_TEXT("Error reading file")

UI_TEXT("Error writing file")

UI_TEXT("This content must be opened with a folder, \"Browse For File\" isn't supported")


<h2 id="InputDeviceConfig_cc">2.5 EmuFramework\src\InputDeviceConfig.cc</h2>

UI_TEXT("Custom {}")

UI_TEXT("Automatically created profile: {}")


<h2 id="AudioOptionView_cc">3.1 EmuFramework\src\gui\AudioOptionView.cc</h2>

UI_TEXT("Audio Options")

UI_TEXT("Sound")

UI_TEXT("Sound During Fast/Slow Mode")

UI_TEXT("100%")

UI_TEXT("50%")

UI_TEXT("25%")

UI_TEXT("Custom Value")

UI_TEXT("Input 0 to 125")

UI_TEXT("Volume")

UI_TEXT("1")

UI_TEXT("2")

UI_TEXT("3")

UI_TEXT("4")

UI_TEXT("5")

UI_TEXT("6")

UI_TEXT("7")

UI_TEXT("Buffer Size In Frames")

UI_TEXT("Auto-increase Buffer Size")

UI_TEXT("Device Native")

UI_TEXT("22KHz")

UI_TEXT("32KHz")

UI_TEXT("44KHz")

UI_TEXT("48KHz")

UI_TEXT("Sound Rate")

UI_TEXT("Mix With Other Apps")

UI_TEXT("Auto")

UI_TEXT("Audio Driver")


<h2 id="AutosaveSlotView_cc">3.2 EmuFramework\src\gui\AutosaveSlotView.cc</h2>

UI_TEXT("Rename")

UI_TEXT("Input name")

UI_TEXT("A save slot with that name already exists")

UI_TEXT("Error renaming save slot")

UI_TEXT("Delete")

UI_TEXT("Can't delete the currently active save slot")

UI_TEXT("Really delete this save slot?")

UI_TEXT("Manage Save Slots")

UI_TEXT("No saved state")

UI_TEXT("{}: {}")

UI_TEXT("Autosave Slot")

UI_TEXT("Create New Save Slot")

UI_TEXT("Save Slot Name")

UI_TEXT("A save slot with that name already exists")

UI_TEXT("Error creating save slot")

UI_TEXT("Manage Save Slots")

UI_TEXT("No extra save slots exist")

UI_TEXT("Actions")

UI_TEXT("Main: {}")

UI_TEXT("{}: {}")

UI_TEXT("No Save")

UI_TEXT("{}: {}")


<h2 id="BundledGamesView_cc">3.3 EmuFramework\src\gui\BundledGamesView.cc</h2>

UI_TEXT("Bundled Content")


<h2 id="ButtonConfigView_cc">3.4 EmuFramework\src\gui\ButtonConfigView.cc</h2>

UI_TEXT("Unbind All")

UI_TEXT("Really unbind all keys in this category?")

UI_TEXT("Reset Defaults")

UI_TEXT("Really reset all keys in this category to defaults?")

UI_TEXT("Unbind")

UI_TEXT("Cancel")

UI_TEXT("You pushed a key from device:\n{}\nPush another from it to open its config menu")

UI_TEXT("Push up to 3 keys, release any to set:\n{}")

UI_TEXT("Push up to 3 keys, release any to set:\n{}\n\nTo unbind:\nQuickly push [Left] key twice in previous menu")


<h2 id="CPUAffinityView_cc">3.5 EmuFramework\src\gui\CPUAffinityView.cc</h2>

UI_TEXT("Configure CPU Affinity")

UI_TEXT("Auto (Use only performance cores or hints for low latency)")

UI_TEXT("Any (Use any core even if it increases latency)")

UI_TEXT("Manual (Use cores set in previous menu)")

UI_TEXT("CPU Affinity Mode")

UI_TEXT("Manual CPU Affinity")

UI_TEXT("{} (Offline)")

UI_TEXT("{} ({}MHz)")


<h2 id="FilePathOptionView_cc">3.6 EmuFramework\src\gui\FilePathOptionView.cc</h2>	

UI_TEXT("App Folder")

UI_TEXT("Content Folder")

UI_TEXT("Saves: {}")

UI_TEXT("Screenshots: {}")

UI_TEXT("File Path Options")

UI_TEXT("Saves")

UI_TEXT("Select Folder")

UI_TEXT("This folder lacks write access")

UI_TEXT("Same As Content")

UI_TEXT("App Folder")

UI_TEXT("Legacy Game Data Folder")

UI_TEXT("Please select the \"Game Data/{}\" folder from an old version of the app to use its existing saves ")

UI_TEXT("and convert it to a regular save path (this is only needed once)")

UI_TEXT("This folder lacks write access")

UI_TEXT("Please select the {} folder")

UI_TEXT("Screenshots")

UI_TEXT("App Folder:\n{}")


<h2 id="FrameTimingView_cc">3.7 EmuFramework\src\gui\FrameTimingView.cc</h2>

UI_TEXT("Preparing to detect frame rate...")

UI_TEXT("{:g}fps")

UI_TEXT("0fps")

UI_TEXT("Auto")

UI_TEXT("Original")

UI_TEXT("{:g}Hz")

UI_TEXT("Frame Timing Options")

UI_TEXT("Full (No Skip)")

UI_TEXT("Full")

UI_TEXT("1/2")

UI_TEXT("1/3")

UI_TEXT("1/4")

UI_TEXT("Frame Rate Target")

UI_TEXT("Auto (Match screen when rates are similar)")

UI_TEXT("Reported rate potentially unreliable, ")

UI_TEXT("using the detected rate may give better results")

UI_TEXT("Original (Use emulated system's rate)")

UI_TEXT("Detect Custom Rate")

UI_TEXT("Detected rate too unstable to use")

UI_TEXT("Custom Rate")

UI_TEXT("Input decimal or fraction")

UI_TEXT("Frame Rate")

UI_TEXT("Frame Rate (PAL)")

UI_TEXT("Show Frame Time Stats"

UI_TEXT("Auto")

UI_TEXT("Screen (Less latency & power use)")

UI_TEXT("Timer (Best for VRR displays)")

UI_TEXT("Renderer (May buffer multiple frames)")

UI_TEXT("Frame Clock")

UI_TEXT("Auto")

UI_TEXT("Immediate (Less compositor latency, may drop frames)")

UI_TEXT("Queued (Better frame rate stability)")

UI_TEXT("Present Mode")

UI_TEXT("Queued")

UI_TEXT("Immediate")

UI_TEXT("Off")

UI_TEXT("{:g}Hz")

UI_TEXT("Override Screen Frame Rate")

UI_TEXT("Full (Apply to all frame rate targets)")

UI_TEXT("Basic (Only apply to lower frame rate targets)")

UI_TEXT("Off")

UI_TEXT("Precise Frame Pacing")

UI_TEXT("Full")

UI_TEXT("Basic")

UI_TEXT("Allow Blank Frame Insertion")

UI_TEXT("Advanced")

UI_TEXT("{:g}Hz not in valid range")


<h2 id="GUIOptionView_cc">3.8 EmuFramework\src\gui\GUIOptionView.cc</h2>

UI_TEXT("Landscape")

UI_TEXT("90° Left")

UI_TEXT("Landscape 2")

UI_TEXT("90° Right")

UI_TEXT("Portrait")

UI_TEXT("Standard")

UI_TEXT("Portrait 2")

UI_TEXT("Upside Down")

UI_TEXT("GUI Options")

UI_TEXT("Pause if unfocused")

UI_TEXT("Custom Value")

UI_TEXT("Input 2.0 to 10.0")

UI_TEXT("Font Size")

UI_TEXT("Suspended App Icon"

UI_TEXT("Off")

UI_TEXT("In Emu")

UI_TEXT("On")

UI_TEXT("Hide Status Bar")

UI_TEXT("Off")

UI_TEXT("In Emu")

UI_TEXT("On")

UI_TEXT("Dim OS UI")

UI_TEXT("Off")

UI_TEXT("In Emu")

UI_TEXT("On")

UI_TEXT("Hide OS Navigation")

UI_TEXT("Allow Screen Timeout In Emulation")

UI_TEXT("Title Bar")

UI_TEXT("Title Back Navigation")

UI_TEXT("Default Menu")

UI_TEXT("Last Used")

UI_TEXT("Show Bundled Content")

UI_TEXT("Show Bluetooth Menu Items")

UI_TEXT("Show Hidden Files")

UI_TEXT("Max Recent Content Items")

UI_TEXT("Input 1 to 100")

UI_TEXT("Orientation")

UI_TEXT("Auto")

UI_TEXT("In Menu")

UI_TEXT("Auto")

UI_TEXT("In Emu")

UI_TEXT("Display Behind OS UI")

UI_TEXT("Set Window Size")

UI_TEXT("Input Width & Height")

UI_TEXT("Toggle Full Screen")


<h2 id="InputManagerView_cc">3.9 EmuFramework\src\gui\InputManagerView.cc</h2>

UI_TEXT("Delete device settings from the configuration file? Any key profiles in use are kept")

UI_TEXT("Delete profile from the configuration file? Devices using it will revert to their default profile")

UI_TEXT("Push a key on any input device enter its configuration menu")

UI_TEXT("Key/Gamepad Input Setup")

UI_TEXT("Delete Saved Device Settings")

UI_TEXT("No saved device settings")

UI_TEXT("Delete Saved Key Profile")

UI_TEXT("No saved profiles")

UI_TEXT("Re-scan OS Input Devices")

UI_TEXT("{} OS devices present")

UI_TEXT("Auto-detect Device To Setup")

UI_TEXT("General Options")

UI_TEXT("Individual Device Settings")

UI_TEXT("General Input Options")

UI_TEXT("MOGA Controller Support")

UI_TEXT("Install the MOGA Pivot app from Google Play to use your MOGA Pocket. ")

UI_TEXT("For MOGA Pro or newer, set switch to mode B and pair in the Android Bluetooth settings app instead.")

UI_TEXT("Notify If Devices Change")

UI_TEXT("In-app Bluetooth Options")

UI_TEXT("Keep Connections In Background")

UI_TEXT("Scan Time")

UI_TEXT("Cache Scan Results")

UI_TEXT("Swap Confirm/Cancel Keys")

UI_TEXT("Multiple")

UI_TEXT("Player")

UI_TEXT("Rename Profile")

UI_TEXT("Can't rename a built-in profile")

UI_TEXT("Input name")

UI_TEXT("Another profile is already using this name")

UI_TEXT("New Profile")

UI_TEXT("Create a new profile? All keys from the current profile will be copied over.")

UI_TEXT("Input name")

UI_TEXT("Another profile is already using this name")

UI_TEXT("Delete Profile")

UI_TEXT("Can't delete a built-in profile")

UI_TEXT("iCade Mode")

UI_TEXT("This mode allows input from an iCade-compatible Bluetooth device, don't enable if this isn't an iCade")

UI_TEXT("Enable")

UI_TEXT("Cancel")

UI_TEXT("Handle Unbound Keys")

UI_TEXT("Stick 1 as D-Pad")

UI_TEXT("Stick 2 as D-Pad")

UI_TEXT("POV Hat as D-Pad")

UI_TEXT("L/R Triggers as L2/R2")

UI_TEXT("Brake/Gas as L2/R2")

UI_TEXT("Action Categories")

UI_TEXT("Options")

UI_TEXT("Joystick Axis Setup")

UI_TEXT("Profile: {}")

UI_TEXT("Profile: {}")


<h2 id="InputOverridesView_cc">3.10 EmuFramework\src\gui\InputOverridesView.cc</h2>

UI_TEXT("Input Overrides (Per Content)")

UI_TEXT("Delete Saved Device Settings")

UI_TEXT("No saved device settings")

UI_TEXT("Individual Device Settings")

UI_TEXT("Multiple")

UI_TEXT("Default")

UI_TEXT("Multiple")

UI_TEXT("Player")

UI_TEXT("Profile: {}")

UI_TEXT("Profile: {}")


<h2 id="LoadProgressView_cc">3.11 EmuFramework\src\gui\LoadProgressView.cc</h2>

UI_TEXT("Loading...")

UI_TEXT("Loading...")


<h2 id="MainMenuView_cc">3.12 EmuFramework\src\gui\MainMenuView.cc</h2>

UI_TEXT("Unable to initialize Bluetooth adapter")

UI_TEXT("BTstack not found, open Cydia and install?")

UI_TEXT("Open Content")

UI_TEXT("Recent Content")

UI_TEXT("Bundled Content")

UI_TEXT("Options")

UI_TEXT("On-screen Input Setup")

UI_TEXT("Key/Gamepad Input Setup")

UI_TEXT("Benchmark Content")

UI_TEXT("Scan for Wiimotes/iCP/JS1")

UI_TEXT("Starting Scan...\n(see website for device-specific help)")

UI_TEXT("Still scanning")

UI_TEXT("Disconnect Bluetooth")

UI_TEXT("Really disconnect {} Bluetooth device(s)?")

UI_TEXT("Scan for PS3 Controller")

UI_TEXT("Prepare to push the PS button")

UI_TEXT("Unable to register server, make sure this executable has cap_net_bind_service enabled and bluetoothd isn't running")

UI_TEXT("Bluetooth setup failed")

UI_TEXT("Push the PS button on your controller\n(see website for pairing help)")

UI_TEXT("Still scanning")

UI_TEXT("About")

UI_TEXT("Exit")

UI_TEXT("BTstack power on failed, make sure the iOS Bluetooth stack is not active")

UI_TEXT("Scan failed")

UI_TEXT("No devices found")

UI_TEXT("Checking {} device(s)...")

UI_TEXT("Failed reading a device name")

UI_TEXT("Connecting to {} device(s)...")

UI_TEXT("Scan complete, no recognized devices")

UI_TEXT("Options")

UI_TEXT("Frame Timing")

UI_TEXT("Video")

UI_TEXT("Audio")

UI_TEXT("System")

UI_TEXT("File Paths")

UI_TEXT("GUI")

UI_TEXT("Online Documentation")

UI_TEXT("Beta Testing Opt-in/out")


<h2 id="PlaceVControlsView_cc">3.13 EmuFramework\src\gui\PlaceVControlsView.cc</h2>

UI_TEXT("Exit")

UI_TEXT("Snap: 0px")

UI_TEXT("Snap: {}px")


<h2 id="PlaceVideoView_cc">3.14 EmuFramework\src\gui\PlaceVideoView.cc</h2>

UI_TEXT("Exit")

UI_TEXT("Reset")	


<h2 id="ProfileSelectView_hh">3.15 EmuFramework\src\gui\ProfileSelectView.hh</h2>

UI_TEXT("Key Profile")

UI_TEXT("Default")


<h2 id="RecentContentView_cc">3.16 EmuFramework\src\gui\RecentContentView.cc</h2>

UI_TEXT("Recent Content")

UI_TEXT("Clear List")

UI_TEXT("Really clear the list?")


<h2 id="ResetAlertView_hh">3.17 EmuFramework\src\gui\ResetAlertView.hh</h2>

UI_TEXT("Soft Reset")

UI_TEXT("Hard Reset")

UI_TEXT("Cancel")

UI_TEXT("Really reset?")

UI_TEXT("Really reset?")


<h2 id="StateSlotView_cc">3.18 EmuFramework\src\gui\StateSlotView.cc</h2>

UI_TEXT("Set State Slot ({})")

UI_TEXT("Save States")

UI_TEXT("Load State")

UI_TEXT("Really load state?")

UI_TEXT("Save State")

UI_TEXT("Really overwrite state?")


<h2 id="SystemActionsView_cc">3.19 EmuFramework\src\gui\SystemActionsView.cc</h2>

UI_TEXT("Autosave Slot ({})")

UI_TEXT("Save Autosave State")

UI_TEXT("Save Autosave State (Timer In {:%M:%S})")

UI_TEXT("Cheats")

UI_TEXT("Reset")

UI_TEXT("Really save state?")

UI_TEXT("Load Autosave State")

UI_TEXT("No saved state")

UI_TEXT("Really load state from: {}?")

UI_TEXT("Manual Save States")

UI_TEXT("Input Overrides")

UI_TEXT("Add Content Shortcut To Launcher")

UI_TEXT("Shortcut Name")

UI_TEXT("Added shortcut:\n{}")

UI_TEXT("Screenshot Next Frame")

UI_TEXT("Save path isn't valid")

UI_TEXT("Save screenshot to folder {}?")

UI_TEXT("Reset Saved Options")

UI_TEXT("Reset saved options for the currently running system to defaults? Some options only take effect next time the system loads.")

UI_TEXT("Close Content")


<h2 id="SystemOptionView_cc">3.20 EmuFramework\src\gui\SystemOptionView.cc</h2>

UI_TEXT("System Options")

UI_TEXT("Off")

UI_TEXT("5min")

UI_TEXT("10min")

UI_TEXT("15min")

UI_TEXT("Custom Value")

UI_TEXT("Input 0 to 720")

UI_TEXT("Timer")

UI_TEXT("Main Slot")

UI_TEXT("Main Slot (No State)")

UI_TEXT("No Save Slot")

UI_TEXT("Select Slot")

UI_TEXT("Launch Mode")

UI_TEXT("Content")

UI_TEXT("State & Backup RAM")

UI_TEXT("Only Backup RAM")

UI_TEXT("Confirm Overwrite State")

UI_TEXT("1.5x")

UI_TEXT("2x")

UI_TEXT("4x")

UI_TEXT("8x")

UI_TEXT("16x")

UI_TEXT("Custom Value")

UI_TEXT("Input above 1.0 to 20.0")

UI_TEXT("Fast-forward Speed")

UI_TEXT("{:g}x")

UI_TEXT("0.25x")

UI_TEXT("0.50x")

UI_TEXT("Custom Value")

UI_TEXT("Input 0.05 up to 1.0")

UI_TEXT("Value not in range")

UI_TEXT("Slow-motion Speed")

UI_TEXT("{:g}x")

UI_TEXT("0")

UI_TEXT("30")

UI_TEXT("60")

UI_TEXT("Custom Value")

UI_TEXT("Input 0 to 50000")

UI_TEXT("States")

UI_TEXT("State Interval (Seconds)")

UI_TEXT("Input 1 to 60")

UI_TEXT("Performance Mode")

UI_TEXT("Normal")

UI_TEXT("Sustained")

UI_TEXT("No-op Thread (Experimental)")

UI_TEXT("Configure CPU Affinity")

UI_TEXT("Autosave Options")

UI_TEXT("Rewind Options")

UI_TEXT("Other Options")


<h2 id="TouchConfigView_cc">3.21 EmuFramework\src\gui\TouchConfigView.cc</h2>

UI_TEXT("Off")

UI_TEXT("On")

UI_TEXT("Hidden")

UI_TEXT("None")

UI_TEXT("10%")

UI_TEXT("20%")

UI_TEXT("30%")

UI_TEXT("Edit D-Pad")

UI_TEXT("1mm")

UI_TEXT("1.35mm")

UI_TEXT("1.6mm")

UI_TEXT("Custom Value")

UI_TEXT("Input 1.0 to 3.0")

UI_TEXT("Deadzone")

UI_TEXT("{:g}mm")

UI_TEXT("None")

UI_TEXT("33% (Low)")

UI_TEXT("43% (Medium-Low)")

UI_TEXT("50% (Medium)")

UI_TEXT("60% (High)")

UI_TEXT("Custom Value")

UI_TEXT("Input 0 to 99.0")

UI_TEXT("Diagonal Sensitivity")

UI_TEXT("{:g}%")

UI_TEXT("State")

UI_TEXT("Show Bounding Area")

UI_TEXT("Remove This D-Pad")

UI_TEXT("Really remove this d-pad?")

UI_TEXT("D-Pad Actions")

UI_TEXT("Up")

UI_TEXT("Right")

UI_TEXT("Down")

UI_TEXT("Left")

UI_TEXT("Assign Action")

UI_TEXT("Edit Button")

UI_TEXT("Action")

UI_TEXT("Assign Action")

UI_TEXT("Turbo")

UI_TEXT("Toggle")

UI_TEXT("Remove This Button")

UI_TEXT("Really remove this button?")

UI_TEXT("Edit Buttons")

UI_TEXT("State")

UI_TEXT("1")

UI_TEXT("2")

UI_TEXT("3")

UI_TEXT("4")

UI_TEXT("5")

UI_TEXT("Buttons Per Row")

UI_TEXT("1mm")

UI_TEXT("2mm")

UI_TEXT("3mm")

UI_TEXT("4mm")

UI_TEXT("Custom Value")

UI_TEXT("Input 0 to 8")

UI_TEXT("Spacing")

UI_TEXT("-0.75x V")

UI_TEXT("-0.5x V")

UI_TEXT("0")

UI_TEXT("0.5x V")

UI_TEXT("0.75x V")

UI_TEXT("1x H&V")

UI_TEXT("Stagger")

UI_TEXT("Custom Value")

UI_TEXT("Input 0 to 30")

UI_TEXT("Extended H Bounds")

UI_TEXT("{}%")

UI_TEXT("Custom Value")

UI_TEXT("Input 0 to 30")

UI_TEXT("Extended V Bounds")

UI_TEXT("{}%")

UI_TEXT("Show Bounding Area")

UI_TEXT("Add Button To This Group")

UI_TEXT("Add Button")

UI_TEXT("Remove This Button Group")

UI_TEXT("Really remove this button group?")

UI_TEXT("Buttons In Group")

UI_TEXT("Add New Button Group")

UI_TEXT("On-screen Input Setup")

UI_TEXT("Off")

UI_TEXT("On")

UI_TEXT("Auto")

UI_TEXT("Use Virtual Gamepad")

UI_TEXT("Virtual Gamepad Player")

UI_TEXT("6.5mm")

UI_TEXT("7mm")

UI_TEXT("7.5mm")

UI_TEXT("8mm")

UI_TEXT("8.5mm")

UI_TEXT("9mm")

UI_TEXT("10mm")

UI_TEXT("12mm")

UI_TEXT("14mm")

UI_TEXT("15mm")

UI_TEXT("Custom Value")

UI_TEXT("Input 3.0 to 30.0")

UI_TEXT("Button Size")

UI_TEXT("{:g}mm")

UI_TEXT("Vibration")

UI_TEXT("Show Gamepad If Screen Touched")

UI_TEXT("Highlight Pushed Buttons")

UI_TEXT("0%")

UI_TEXT("10%")

UI_TEXT("25%")

UI_TEXT("50%")

UI_TEXT("65%")

UI_TEXT("75%")

UI_TEXT("Blend Amount")

UI_TEXT("Set Button Positions")

UI_TEXT("Set Video Position")

UI_TEXT("Add New Button Group")

UI_TEXT("Allow Buttons In Display Cutout Area")

UI_TEXT("Reset Emulated Device Positions")

UI_TEXT("Reset buttons to default positions?")

UI_TEXT("Reset Emulated Device Groups")

UI_TEXT("Reset buttons groups to default?")

UI_TEXT("Reset UI Positions")

UI_TEXT("Reset buttons to default positions?")

UI_TEXT("Reset UI Groups")

UI_TEXT("Reset buttons groups to default?")

UI_TEXT("Emulated Device Button Groups")

UI_TEXT("UI Button Groups")

UI_TEXT("Other Options")


<h2 id="VideoOptionView_cc">3.22 EmuFramework\src\gui\VideoOptionView.cc</h2>

UI_TEXT("RGB565")

UI_TEXT("RGBA8888")

UI_TEXT("Video Options")

UI_TEXT("Auto (Set optimal mode)")

UI_TEXT("GPU Copy Mode")

UI_TEXT("Square Pixels")

UI_TEXT("Fill Display")

UI_TEXT("Custom Value")

UI_TEXT("Input decimal or fraction")

UI_TEXT("Value not in range")

UI_TEXT("Aspect Ratio")

UI_TEXT("{:g}")

UI_TEXT("100%")

UI_TEXT("90%")

UI_TEXT("80%")

UI_TEXT("Integer-only")

UI_TEXT("Integer-only (Height)")

UI_TEXT("Custom Value")

UI_TEXT("Input 10 to 200")

UI_TEXT("Content Scale")

UI_TEXT("100%")

UI_TEXT("95%")

UI_TEXT("90%")

UI_TEXT("Custom Value")

UI_TEXT("Input 50 to 100")

UI_TEXT("Menu Scale")

UI_TEXT("Auto")

UI_TEXT("Standard")

UI_TEXT("90° Right")

UI_TEXT("Upside Down")

UI_TEXT("90° Left")

UI_TEXT("Content Rotation")

UI_TEXT("Set Video Position")

UI_TEXT("Image Interpolation")

UI_TEXT("None")

UI_TEXT("Linear")

UI_TEXT("Off")

UI_TEXT("hq2x")

UI_TEXT("Scale2x")

UI_TEXT("Prescale 2x")

UI_TEXT("Prescale 3x")

UI_TEXT("Prescale 4x")

UI_TEXT("Image Effect")

UI_TEXT("Off")

UI_TEXT("Scanlines")

UI_TEXT("Scanlines 2x")

UI_TEXT("LCD Grid")

UI_TEXT("CRT Mask")

UI_TEXT("CRT Mask .5x")

UI_TEXT("CRT Grille")

UI_TEXT("CRT Grille .5x")

UI_TEXT("Overlay Effect")

UI_TEXT("100%")

UI_TEXT("75%"

UI_TEXT("50%")

UI_TEXT("25%")

UI_TEXT("Custom Value")

UI_TEXT("Input 0 to 100")

UI_TEXT("Overlay Effect Level")

UI_TEXT("{}%")

UI_TEXT("Auto (Match display format)")

UI_TEXT("RGBA8888")

UI_TEXT("RGB565")

UI_TEXT("Effect Color Format")

UI_TEXT("Restart app for option to take effect")

UI_TEXT("Auto")

UI_TEXT("Display Color Format")

UI_TEXT("2nd Window (for testing only)")

UI_TEXT("External Screen")

UI_TEXT("OS Managed")

UI_TEXT("Emu Content")

UI_TEXT("Auto (Match display format)")

UI_TEXT("RGBA8888")

UI_TEXT("RGB565")

UI_TEXT("Render Color Format")

UI_TEXT("Default")

UI_TEXT("Custom Value")

UI_TEXT("Default")

UI_TEXT("Custom Value")

UI_TEXT("Default")

UI_TEXT("Custom Value")

UI_TEXT("Default")

UI_TEXT("Custom Value")

UI_TEXT("Set All Levels")

UI_TEXT("All Levels")

UI_TEXT("Red")

UI_TEXT("{}%")

UI_TEXT("Green")

UI_TEXT("{}%")

UI_TEXT("Blue")

UI_TEXT("{}%")

UI_TEXT("Color Levels")

UI_TEXT("Advanced")

UI_TEXT("System-specific")

UI_TEXT("Input 0 to 200")


<h2 id="MDFNUtils_hh">4. EmuFramework\src\shared\mednafen-emuex\MDFNUtils.hh</h2>			

UI_TEXT("Save Filename Type")

UI_TEXT("Default")

UI_TEXT("No MD5")

UI_TEXT("Error uncompressing state")

UI_TEXT("Invalid state size")

UI_TEXT("Bad state header size, got {} but expected {}")
