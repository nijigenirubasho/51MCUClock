# 时间设置脚本
# 只能够在Windows7或以上版本系统中右键单击本脚本选择使用PowerShell运行，并且只有一个连接着的作为串口设备的待设置单片机
$host.UI.RawUI.WindowTitle = "51MCUClock:SetLocalTimeScript"
$h = (Get-Date).Hour
$m = (Get-Date).Minute
$s = (Get-Date).Second
"Set the time to {0:d2}:{1:d2}:{2:d2} ..." -f $h,$m,$s
$com = [System.IO.Ports.SerialPort]::GetPortNames()[0]
"Available Serial Port[0]:$com"
$port = New-Object System.IO.Ports.SerialPort $com,9600,None,8,one
$port.Open()
[Byte[]]$data = ($h + 8),($m + 32),($s + 92)
$port.Write($data,0,$data.Count)
$port.Close()
"Press ENTER key to exit"
Pause
