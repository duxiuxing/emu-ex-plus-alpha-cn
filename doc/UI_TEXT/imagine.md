- imagine\include\imagine\gui
    - [1.1 AlertView.hh](#AlertView_hh)
    - [1.2 MenuItem.hh](#MenuItem_hh)
- imagine\src
    - [2.1 base\android\ApplicationContext.cc](#ApplicationContext_cc)
    - [2.2 gfx\opengl\PixmapBufferTexture.cc](#PixmapBufferTexture_cc)
    - [2.3 gui\FSPicker.cc](#FSPicker_cc)
    - [2.4 input\Device.cc](#Device_cc)
    - [2.5 input\Input.cc](#Input_cc)
    - [2.6 io\AAssetIO.cc](#AAssetIO_cc)
    - [2.7 io\ArchiveIO.cc](#ArchiveIO_cc)


<h2 id="AlertView_hh">1.1 imagine\include\imagine\gui\AlertView.hh</h2>

UI_TEXT(u"Yes")

u"是"

UI_TEXT(u"No")

u"否"


<h2 id="MenuItem_hh">1.2 imagine\include\imagine\gui\MenuItem.hh</h2>

UI_TEXT(u"On")

u"开"

UI_TEXT(u"Off")

u"关"

UI_TEXT(u"Off")

u"关"

UI_TEXT(u"On")

u"开"


<h2 id="ApplicationContext_cc">2.1 imagine\src\base\android\ApplicationContext.cc</h2>

UI_TEXT("Storage Media")

"存储空间"

UI_TEXT("Media")

"媒体文件夹"

UI_TEXT("App Media Folder")

"应用程序媒体文件夹"

UI_TEXT("Media")

"媒体文件夹"

UI_TEXT("Storage Devices")

"存储设备"

UI_TEXT("Storage")

"存储"


<h2 id="PixmapBufferTexture_cc">2.2 imagine\src\gfx\opengl\PixmapBufferTexture.cc</h2>

UI_TEXT("System Memory")

"系统内存"

UI_TEXT("OpenGL PBO")

"OpenGL 像素缓冲区对象"

UI_TEXT("Hardware Buffer")

"硬件缓冲区"

UI_TEXT("Surface Texture")

"表面纹理"


<h2 id="FSPicker_cc">2.3 imagine\src\gui\FSPicker.cc</h2>

UI_TEXT("Select File Location")

"点击选择游戏文件位置"

UI_TEXT("No folder is set")

"未设置文件夹"

UI_TEXT("File Locations")

"游戏文件位置"

UI_TEXT("This device doesn't have a document browser, please select a media folder instead")

"此设备没有文件管理器，请从“存储空间”选择一个文件夹"

UI_TEXT("Browse For Folder")

"浏览文件夹"

UI_TEXT("Browse For File")

"浏览文件"

UI_TEXT("Root Filesystem")

"根目录"

UI_TEXT("Custom Path")

"自定义路径"

UI_TEXT("Input a directory path")

"请输入一个有效的目录路径"

UI_TEXT("Empty Directory")

"空目录"

UI_TEXT("\nPick a path from the top bar")

"\n点击顶部标题栏重新选择路径"

UI_TEXT("Can't open directory:\n{}{}")

"无法打开目录：\n系统错误 - {}{}"


<h2 id="Device_cc">2.4 imagine\src\input\Device.cc</h2>

UI_TEXT("None")

"无"

UI_TEXT("Space")

"空格键"

UI_TEXT("a")

"a 键"

UI_TEXT("b")

"b 键"

UI_TEXT("c")

"c 键"

UI_TEXT("d")

"d 键"

UI_TEXT("e")

"e 键"

UI_TEXT("f")

"f 键"

UI_TEXT("g")

"g 键"

UI_TEXT("h")

"h 键"

UI_TEXT("i")

"i 键"

UI_TEXT("j")

"j 键"

UI_TEXT("k")

"k 键"

UI_TEXT("l")

"l 键"

UI_TEXT("m")

"m 键"

UI_TEXT("n")

"n 键"

UI_TEXT("o")

"o 键"

UI_TEXT("p")

"p 键"

UI_TEXT("q")

"q 键"

UI_TEXT("r")

"r 键"

UI_TEXT("s")

"s 键"

UI_TEXT("t")

"t 键"

UI_TEXT("u")

"u 键"

UI_TEXT("v")

"v 键"

UI_TEXT("w")

"w 键"

UI_TEXT("x")

"x 键"

UI_TEXT("y")

"y 键"

UI_TEXT("z")

"z 键"

UI_TEXT("'")

"' 键 (单引号)"

UI_TEXT(",")

", 键"

UI_TEXT("-")

"- 键"

UI_TEXT(".")

". 键"

UI_TEXT("/")

"/ 键"

UI_TEXT("0")

"0 键"

UI_TEXT("1")

"1 键"

UI_TEXT("2")

"2 键"

UI_TEXT("3")

"3 键"

UI_TEXT("4")

"4 键"

UI_TEXT("5")

"5 键"

UI_TEXT("6")

"6 键"

UI_TEXT("7")

"7 键"

UI_TEXT("8")

"8 键"

UI_TEXT("9")

"9 键"

UI_TEXT(";")

"; 键"

UI_TEXT("=")

"= 键"

UI_TEXT("[")

"[ 键"

UI_TEXT("\\")

"\\ 键"

UI_TEXT("]")

"] 键"

UI_TEXT("`")

"` 键 (反引号)"

UI_TEXT("Back")

"返回键"

UI_TEXT("Escape")

"Esc 键"

UI_TEXT("Enter")

"回车键"

UI_TEXT("Left Alt")

"左 Alt 键"

UI_TEXT("Right Alt")

"右 Alt 键"

UI_TEXT("Left Shift")

"左 Shift 键"

UI_TEXT("Right Shift")

"右 Shift 键"

UI_TEXT("Left Ctrl")

"左 Ctrl 键"

UI_TEXT("Right Ctrl")

"右 Ctrl 键"

UI_TEXT("Up")

"方向键的上"

UI_TEXT("Right")

"方向键的右"

UI_TEXT("Down")

"方向键的下"

UI_TEXT("Left")

"方向键的左"

UI_TEXT("Back Space")

"退格键"

UI_TEXT("Menu")

"菜单键"

UI_TEXT("Home")

"Home 键"

UI_TEXT("End")

"End 键"

UI_TEXT("Insert")

"插入键"

UI_TEXT("Delete")

"删除键"

UI_TEXT("Tab")

"Tab 键"

UI_TEXT("Scroll Lock")

"滑动锁定键"

UI_TEXT("Caps Lock")

"大写切换键"

UI_TEXT("Pause")

"暂停键"

UI_TEXT("Left Start/Option")

"左边的开始/选项键"

UI_TEXT("Right Start/Option")

"右边的开始/选项键"

UI_TEXT("Page Up")

"向上翻页键"

UI_TEXT("Page Down")

"向下翻页键"

UI_TEXT("Print Screen")

"截屏键"

UI_TEXT("Num Lock")

"数字锁定键"

UI_TEXT("Numpad 0")

"0 键 (数字键盘)"

UI_TEXT("Numpad 1")

"1 键 (数字键盘)"

UI_TEXT("Numpad 2")

"2 键 (数字键盘)"

UI_TEXT("Numpad 3")

"3 键 (数字键盘)"

UI_TEXT("Numpad 4")

"4 键 (数字键盘)"

UI_TEXT("Numpad 5")

"5 键 (数字键盘)"

UI_TEXT("Numpad 6")

"6 键 (数字键盘)"

UI_TEXT("Numpad 7")

"7 键 (数字键盘)"

UI_TEXT("Numpad 8")

"8 键 (数字键盘)"

UI_TEXT("Numpad 9")

"9 键 (数字键盘)"

UI_TEXT("Numpad /")

"/ 键 (数字键盘)"

UI_TEXT("Numpad *")

"* 键 (数字键盘)"

UI_TEXT("Numpad -")

"- 键 (数字键盘)"

UI_TEXT("Numpad +")

"+ 键 (数字键盘)"

UI_TEXT("Numpad .")

". 键 (数字键盘)"

UI_TEXT("Numpad ,")

", 键 (数字键盘)"

UI_TEXT("Numpad Enter")

"回车键 (数字键盘)"

UI_TEXT("Numpad =")

"= 键 (数字键盘)"

UI_TEXT("@")

"@ 键"

I_TEXT("*")

"* 键"

UI_TEXT("+")

"+ 键"

UI_TEXT("#")

"# 键"

UI_TEXT("(")

"( 键"

UI_TEXT(")")

") 键"

UI_TEXT("Search")

"搜索键"

UI_TEXT("Clear")

"清除键"

UI_TEXT("Explorer")

"Explorer 键"

UI_TEXT("Mail")

"Mail 键"

UI_TEXT("Vol Up")

"提高音量键"

UI_TEXT("Vol Down")

"降低音量键"

UI_TEXT("Numpad Insert")

"插入键 (数字键盘)"

UI_TEXT("Numpad Delete")

"删除键 (数字键盘)"

UI_TEXT("Numpad Begin")

"Begin 键 (数字键盘)"

UI_TEXT("Numpad Home")

"Home 键 (数字键盘)"

UI_TEXT("Numpad End")

"End 键 (数字键盘)"

UI_TEXT("Numpad Page Up")

"向上翻页键 (数字键盘)"

UI_TEXT("Numpad Page Down")

"向下翻页键 (数字键盘)"

UI_TEXT("Numpad Up")

"方向键的上 (数字键盘)"

UI_TEXT("Numpad Right")

"方向键的右 (数字键盘)"

UI_TEXT("Numpad Down")

"方向键的下 (数字键盘)"

UI_TEXT("Numpad Left")

"方向键的左 (数字键盘)"

UI_TEXT("Left Meta")

"左 Meta 键"

UI_TEXT("Right Meta")

"右 Meta 键"

UI_TEXT("F1")

"F1 键"

UI_TEXT("F2")

"F2 键"

UI_TEXT("F3")

"F3 键"

UI_TEXT("F4")

"F4 键"

UI_TEXT("F5")

"F5 键"

UI_TEXT("F6")

"F6 键"

UI_TEXT("F7")

"F7 键"

UI_TEXT("F8")

"F8 键"

UI_TEXT("F9")

"F9 键"

UI_TEXT("F10")

"F10 键"

UI_TEXT("F11")

"F11 键"

UI_TEXT("F12")

"F12 键"

UI_TEXT("A")

"A 键"

UI_TEXT("B")

"B 键"

UI_TEXT("C")

"C 键"

UI_TEXT("X")

"X 键"

UI_TEXT("Y")

"Y 键"

UI_TEXT("Z")

"Z 键"

UI_TEXT("L1")

"L1 键"

UI_TEXT("R1")

"R1 键"

UI_TEXT("L2")

"L2 键"

UI_TEXT("R2")

"R2 键"

UI_TEXT("L-Thumb")

"L-Thumb 键"

UI_TEXT("R-Thumb")

"R-Thumb 键"

UI_TEXT("Start")

"开始键"

UI_TEXT("Select")

"选择键"

UI_TEXT("Mode")

"模式键"

UI_TEXT("G1")

"G1 键"

UI_TEXT("G2")

"G2 键"

UI_TEXT("G3")

"G3 键"

UI_TEXT("G4")

"G4 键"

UI_TEXT("G5")

"G5 键"

UI_TEXT("G6")

"G6 键"

UI_TEXT("G7")

"G7 键"

UI_TEXT("G8")

"G8 键"

UI_TEXT("G9")

"G9 键"

UI_TEXT("G10")

"G10 键"

UI_TEXT("G11")

"G11 键"

UI_TEXT("G12")

"G12 键"

UI_TEXT("G13")

"G13 键"

UI_TEXT("G14")

"G14 键"

UI_TEXT("G15")

"G15 键"

UI_TEXT("G16")

"G16 键"

UI_TEXT("X Axis+")

"左摇杆的右"

UI_TEXT("X Axis-")

"左摇杆的左"

UI_TEXT("Y Axis+")

"左摇杆的下"

UI_TEXT("Y Axis-")

"左摇杆的上"

UI_TEXT("X Axis+ 2")

"右摇杆的右"

UI_TEXT("X Axis- 2")

"右摇杆的左"

UI_TEXT("Y Axis+ 2")

"右摇杆的下"

UI_TEXT("Y Axis- 2")

"右摇杆的上"

UI_TEXT("X Axis+ 3")

UI_TEXT("X Axis- 3")

UI_TEXT("Y Axis+ 3")

UI_TEXT("Y Axis- 3")

UI_TEXT("POV Right")

"苦力帽的右"

UI_TEXT("POV Left")

"苦力帽的左"

UI_TEXT("POV Down")

"苦力帽的下"

UI_TEXT("POV Up")

"苦力帽的上"

UI_TEXT("Rudder Right")

"方向舵的右"

UI_TEXT("Rudder Left")

"方向舵的左"

UI_TEXT("Wheel Right")

"滚轮右拨"

UI_TEXT("Wheel Left")

"滚轮左拨"

UI_TEXT("L Trigger")

"左扳机"
 
UI_TEXT("R Trigger")

"右扳机"

UI_TEXT("Gas")

"油门"

UI_TEXT("Brake")

"刹车"

UI_TEXT("Sym")

"系统键"

UI_TEXT("Num")

"数字锁定键"

UI_TEXT("Function")

"功能键"

UI_TEXT("Center")

"中央键"

UI_TEXT("Camera")

"相机键"

UI_TEXT("Call")

"呼叫键"

UI_TEXT("End Call")

"结束呼叫键"

UI_TEXT("Focus")

"对焦键"

UI_TEXT("Headset Hook")

"插入耳机"

UI_TEXT("Play")

"播放键"

UI_TEXT("Stop")

"停止键"

UI_TEXT("Next")

"下一个文件"

UI_TEXT("Previous")

"上一个文件"

UI_TEXT("Rewind")

"倒带键"

UI_TEXT("Fast Forward")

"快进键"

UI_TEXT("Cross")

"交叉键"

UI_TEXT("Circle")

"圆圈键"

UI_TEXT("Square")

"正方键"

UI_TEXT("Triangle")

"三角键"

UI_TEXT("PS")

"PS 键"

UI_TEXT("L3")

"L3 键"

UI_TEXT("R3")

"R3 键"

UI_TEXT("Cross")

"交叉键"

UI_TEXT("Circle")

"圆圈键"

UI_TEXT("Square")

"正方键"

UI_TEXT("Triangle")

"三角键"

UI_TEXT("O")

"O 键"

UI_TEXT("U")

"U 键"

UI_TEXT("Y")

"Y 键"

UI_TEXT("A")

"A 键"

UI_TEXT("L3")

"L3 键"

UI_TEXT("R3")

"R3 键"

UI_TEXT("System")

"系统键"

UI_TEXT("L")

"L 键"

UI_TEXT("R")

"R 键"

UI_TEXT("A")

"A 键"

UI_TEXT("B")

"B 键"

UI_TEXT("Y")

"Y 键"

UI_TEXT("X")

"X 键"

UI_TEXT("Select")

"选择键"

UI_TEXT("Start")

"开始键"

UI_TEXT("Logo")

"Logo 键"

UI_TEXT("Alt")

"Alt 键"

UI_TEXT("Shift")

"Shift 键"

UI_TEXT("Ctrl")

"Ctrl 键"

UI_TEXT("Key Code {:#X}")

"按键码 {:#X}"

UI_TEXT("{} #{}")


<h2 id="Input_cc">2.5 imagine\src\input\Input.cc</h2>

UI_TEXT("Unknown")

UI_TEXT("Key Input")

UI_TEXT("Pointer")

UI_TEXT("Relative Pointer")

UI_TEXT("Wiimote")

UI_TEXT("Classic / Wii U Pro Controller")

UI_TEXT("iControlPad")

UI_TEXT("Zeemote JS1")

UI_TEXT("PS3 Gamepad")

UI_TEXT("Apple Game Controller")

UI_TEXT("Keyboard")

UI_TEXT("Gamepad")

UI_TEXT("Mouse")

UI_TEXT("Touchscreen")

UI_TEXT("Navigation")

UI_TEXT("Joystick")

UI_TEXT("Unknown")

UI_TEXT("Unused")

UI_TEXT("Released")

UI_TEXT("Pushed")

UI_TEXT("Moved")

UI_TEXT("Moved Relative")

UI_TEXT("Exit View")

UI_TEXT("Enter View")

UI_TEXT("Scroll Up")

UI_TEXT("Scroll Down")

UI_TEXT("Canceled")

UI_TEXT("Unknown")


<h2 id="AAssetIO_cc">2.6 imagine\src\io\AAssetIO.cc</h2>    

UI_TEXT("Error opening asset: {}")

"打开资源时出错：{}"


<h2 id="ArchiveIO_cc">2.7 imagine\src\io\ArchiveIO.cc</h2>

UI_TEXT("Error opening archive: {}")

"打开档案时出错：{}"
