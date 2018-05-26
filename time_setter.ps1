# 时间设置脚本
# 只能够在Windows7或以上版本系统中右键单击本脚本选择使用PowerShell运行，并且只有一个连接着的作为串口设备的待设置单片机
$host.UI.RawUI.WindowTitle = "51MCUClock:SetLocalTimeScript"
$com = [System.IO.Ports.SerialPort]::GetPortNames()[0]
"Available Serial Port[0]:$com"
$port = New-Object System.IO.Ports.SerialPort $com,9600,None,8,One
$port.Open()
# 等待至整秒的时候再调整，这样校时几乎没有偏差
"Waiting for 000 ms ..."
$stmp = (Get-Date).Second
while ((Get-Date).Second -eq $stmp) {}
[Byte[]]$data = ((Get-Date).Hour + 8),((Get-Date).Minute + 32),((Get-Date).Second + 92)
$port.Write($data,0,$data.Count)
$port.Close()
$h = (Get-Date).Hour
$m = (Get-Date).Minute
$s = (Get-Date).Second
"Set the time to {0:d2}:{1:d2}:{2:d2}" -f $h,$m,$s
"Press ENTER key to exit"
Pause
