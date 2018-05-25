# 调试脚本,在已连接设备后使用
$count = 0
$com = [System.IO.Ports.SerialPort]::GetPortNames()[0]
$host.UI.RawUI.WindowTitle = "51MCUClock:DebugScript,serial[0]=$com"
for ()
{
  "# count:$count"
  $port = New-Object System.IO.Ports.SerialPort $com,9600,None,8,One
  $port.Open()
  # 八个调试变量
  $debug = 7;
  $tag = "hour","min","sec","pendulum","flash","set_mode","hour_alarm","adjust(255)"
  [Byte[]]$array = 0..$debug
  $port.Write($array,0,$array.Count)
  for ($i = 0; $i -le $debug; $i++) { "{0}={1}" -f $tag[$i],$port.ReadByte() }
  $port.Close()
  Pause
  $count++
  Clear-Host
}
