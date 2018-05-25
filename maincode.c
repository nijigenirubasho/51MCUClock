/**--------------------------------------------------------------------------------------------------------------------------------------------------------------------
用51单片机(STC89C52,亚博智能BST-M51教材定制版开发板)制作电子时钟

作者：nijigenirubasho @Github 2018-05-25

功能简述：1.小时/分钟显示和设置。2.每秒闪光一次，可以以此读秒。3.有每小时响铃一次的功能。

使用说明：
    1.LED灯：D1作为秒针指示，每秒发出50毫秒的闪光。D4校时提示，亮时将多跳一秒时间（一下过了两秒），请不要在这时计量秒数和操作设备。
			 D5显示每小时响铃模式状态，变亮标示此功能开启。D7表示是否显示秒钟。其他灯留作每小时响铃的辅助提示。
    2.有源蜂鸣器：如果每小时响铃功能开启，整点过去时发出持续1秒的响铃。反之持续静默。
    3.按键：K2切换（显示时间/设置小时/设置分钟）模式，从设置分钟切回显示时间模式时秒针置零，可以以此精确校时。
            K3和K4在设置时间模式中负责加减当前设置的项目，但在显示时间模式中，按住K3会显示秒钟的值。
			K5在显示时间模式切换每小时响铃模式状态，但在设置模式中按下可以直接退出设置模式而不重置秒针，当进入设置模式成为一种误操作时可以用这个取消时间的修改。
    4.LED数码管：显示时间模式下显示(小时.分钟)或在按住K3时显示出(分钟.秒钟)。中间的点一秒钟切换一次(亮/按)状态。在设置时间模式，只有被设置项以1Hz的频率在闪烁。
    5.串口通讯：开发者用，9600波特率，用来快速设置时间校准，HEX模式的值有三种情况：1.小时数+8 2.分钟数+20 3.秒钟数+5C。只允许单个char一次设置一个对应的（时/分/秒）的值。

误差：显示时间=真实时间±(<1s/day)（注意：这是根据我手上的机器和环境测出来的，如果实际不符合请自行修改adjust_offset，数值为隔多少秒额外加一秒）

开源协议：MIT（国内等同于WTFPL）
---------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
//STC89C52用头文件
// > Header file for generic 80C52 and 80C32 microcontroller.
#include <reg52.h>
//数码管段选锁存器
#define lsd_seg P0
//LED阵列
#define ledx P1
//数码管位选锁存器
#define lsd_pos P2
//计时误差偏移量
//测试一：经粗测，7小时慢了9秒，也就是说每7*3600/9=2800秒就要慢一秒
//测试二：在测试一基础下，经过17时40分，慢了两秒，(17*3600+40*60)/2=31800秒就慢一秒，减去31800/2800=11秒，得2800-11=2789秒
#define adjust_offset 2789
//按键key2到key5对应实体标号K2到K5
sbit key2=P3^4;
sbit key3=P3^5;
sbit key4=P3^6;
sbit key5=P3^7;
//秒针指示，闪一下代表+1s
sbit led1=P1^0;
//校时提示
sbit led4=P1^3;
//每小时响铃指示：开启则亮
sbit led5=P1^4;
//是否显示了秒钟数
sbit led7=P1^6;
//有源蜂鸣器：用于指示每小时响铃
sbit beep=P2^3;
//其他变量：50ms计数，1s记录（钟摆），闪光（秒针指示），设置模式，每小时响铃模式，校准
int fiftyms_count=0,pendulum,flash,set_mode,hour_alarm,adjust=0;
//时间存储：时分秒
int hour,min,sec;
//led数码管(LED Segment Displays)从右至左位列表
unsigned char code lsd_list[]= {0x8f,0x4f,0x2f,0x1f};
//字符集码表：0123456789
unsigned char code charset[]= {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
//软件延时：CPU时间占用延迟(ms)
void delay(int i)
{
    int j,k;
    for(j=i; j>0; j--)
        for(k=124; k>0; k--);
}
//显示一位数字
void lsd_display(int which,int body)
{
    lsd_seg=0x00;//段选置0防止残影
    lsd_pos|=0xf0;//P2^4~P2^7置1
    lsd_pos=lsd_pos&lsd_list[which];//要位置1其他清0
    /*
    设置模式：
    	0：时间显示
    	1：小时设置
    	2：分钟设置
    */
    switch(set_mode)
    {
    case 0:
        //钟摆决定显示分割用的点，在第三个数码管上
        if(which==2&&pendulum)
            //加点：0x80 点
            lsd_seg=charset[body]+0x80;
        else
            lsd_seg=charset[body];
        break;
    case 1:
        //23：小时闪烁，其余数字消失
        if((which==2||which==3)&&pendulum)
            lsd_seg=charset[body];
        else
            lsd_seg=0x00;
        break;
    case 2:
        //01：分钟闪烁，其余数字消失
        if((which==0||which==1)&&pendulum)
            lsd_seg=charset[body];
        else
            lsd_seg=0x00;
    }
    //延迟保证显示亮度和视觉暂留作用
    delay(1);
}
//显示所有4位数字，参数排列顺序按照开发板正面数码管位置
void lsd_show_all(int a,int b,int c,int d)
{
    lsd_display(3,a);
    lsd_display(2,b);
    lsd_display(1,c);
    lsd_display(0,d);
}
//设置定时器初值参数
void set_timer_para()
{
    /*晶振：11.0592MHz
      TH:(65536-ms*1000*11.0592/12)/256
      TL:(65536-ms*1000*11.0592/12)%256
      50ms，注意不要溢出*/
    TH0=76;
    TL0=0;
}
//初始化串行端口
void serial_init()
{
    //0101 0000 方式1，允许接收串行
    SCON=0x50;
    //0001 0100 T1 方式二 计数，保留定时器工作方式
    TMOD|=0x20;
    //SMOD=0
    PCON=0x00;
    //9600波特率：查表11-6得出
    TH1=0xfd;
    TL1=0xfd;
    //接收中断打开
    ES=1;
    //计数器打开
    TR1=1;
}
//主函数
void main()
{
    //总中断打开
    EA=1;
    //0000 0001 T0 定时 方式一
    TMOD=0x01;
    //初始化定时器初值和串行
    set_timer_para();
    serial_init();
    //设置初始时间：11:59:57
    hour=11;
    min=59;
    sec=57;
    //定时器T0中断开启
    ET0=1;
    //T0开启
    TR0=1;
    //轮询
    while(1)
    {
        //小时十位数，小时个位数，分钟十位数，分钟个位数
        lsd_show_all(hour/10,hour%10,min/10,min%10);
        //key2监听
        if(key2==0)
        {
            //消抖
            delay(10);
            //再次监听
            if(key2==0)
            {
                //设置模式循环：0 1 2
                set_mode++;
                if(set_mode==3)
                {
                    set_mode=0;
                    //秒和相关计数置0方便校时
                    sec=0;
                    fiftyms_count=0;
                    adjust=0;
                }
            }
            //暂停，放开才继续可视化操作
            while(key2==0);
        }
        //key3监听
        if(key3==0)
        {
            delay(10);
            if(key3==0)
            {
                //设置数值增加模式
                switch(set_mode)
                {
                //小时
                case 1:
                    hour++;
                    //满24清零
                    if(hour==24)
                        hour=0;
                    break;
                //分钟
                case 2:
                    min++;
                    //满60清零
                    if(min==60)
                        min=0;
                }
            }
            //在非设置模式时，按住key3，显示分秒，并且亮起led7
            while(key3==0)
            {
                if(!set_mode)
                {
                    lsd_show_all(min/10,min%10,sec/10,sec%10);
                    led7=0;
                }
            }
            //熄灭led7
            led7=1;
        }
        //key4监听
        if(key4==0)
        {
            delay(10);
            if(key4==0)
            {
                //设置数值减少模式
                switch(set_mode)
                {
                case 1:
                    hour--;
                    if(hour==-1)
                        hour=23;
                    break;
                case 2:
                    min--;
                    if(min==-1)
                        min=59;
                }
            }
            //调试：按住key4显示adjust百分比
            while(key4==0)
            {
                //不是设置模式且不是100%
                if(!set_mode)
                {
                    //一定要考虑溢出的问题：adjust*100/adjust_offset不可用
                    unsigned char percent=adjust/(adjust_offset/100);
                    //由于adjust_offset非整百，因此不必判断100%，因为永远不可能达到
                    //if(percent/10!=10)
                    //75：adjust的字母顺序和
                    lsd_show_all(7,5,percent/10,percent%10);
                }
            }
        }
        //key5监听
        if(key5==0)
        {
            delay(10);
            if(key5==0)
            {
                //如果在设置模式，不将秒置零就退出，误操作的后悔药
                if(set_mode)
                    set_mode=0;
                else
                    //在不符合整小时情况下才能设置，
                    //防止在整小时情况下改为了开启报时后在一秒后无法重置状态的问题
                    if(min!=0||(sec!=0&&sec!=1))
                    {
                        //反转每小时响铃模式设置
                        hour_alarm=!hour_alarm;
                        //注意LED置0才能亮
                        led5=!hour_alarm;
                    }
                while(key5==0);
            }
        }
    }
}
//T0中断 50ms，使用using节省处理周期，不过不同中断优先级必须使用不同的寄存器组并且尽量避免使用函数。否则参数传递可能出错。主函数已经占用0寄存器。
void timer0() interrupt 1 using 1
{
    //防止计数溢出，所以要重置初值
    set_timer_para();
    //50ms计数+1
    fiftyms_count++;
    //50ms*20=1000ms=1s 以下if范围中事件每秒执行一次
    if(fiftyms_count==20)
    {
        //秒+1
        sec++;
        adjust++;
        //校准
        if(adjust==adjust_offset)
        {
            //打开led4，显示校时信号
            led4=0;
            adjust=0;
            sec++;
        }
        else
            //持续一秒后关闭led4
            led4=1;
        //钟摆反转（摆动一下）
        pendulum=!pendulum;
        //重置50ms计数
        fiftyms_count=0;
        //60s=1m
        if(sec==60)
        {
            min++;
            sec=0;
        }
        //60m=1h
        if(min==60)
        {
            hour++;
            min=0;
        }
        //h loop 0~23
        if(hour==24)
            hour=0;
        //如果小时响铃开启并且不在设置模式进入，防止在调时间时受到惊吓
        if(hour_alarm&&!set_mode)
            //分钟为零，秒为0或1（1用来重置状态）
            if(min==0&&(sec==0||sec==1))
            {
                //反转响铃和LED阵列
                beep=~beep;
                ledx=~ledx;
                /*0：响铃，LED阵列大半都发光
                  1：关闭响铃，led恢复*/
            }
    }
    //1s结束
    //如果秒针指示发光，则关闭之
    if(led1==0)
        led1=1;
    //钟摆是否和闪光同步
    if(pendulum!=flash)
    {
        //不同步反转led1
        /*
        保证在1s只反转一次，并且反转发光在下一个50ms后被上个判断关闭秒针指示
        */
        led1=~led1;
        flash=pendulum;
    }
}
//串口中断，波特率9600，调试信息通过串行端口输出，时间设置信号输入，选择0~7输出变量，8~151设置时间，可能会拖慢CPU造成误差增大，不要频繁串口IO
void serial_io() interrupt 4 using 2
{
    //取出串行数据缓冲寄存器的数据至dat，只允许读写一个char
    unsigned char dat;
    dat=SBUF;
    //复位接收中断标志
    RI=0;
    //调试：根据条件，存回缓冲器相应变量信息
    switch(dat)
    {
    case 0:
        SBUF=hour;
        break;
    case 1:
        SBUF=min;
        break;
    case 2:
        SBUF=sec;
        break;
    case 3:
        SBUF=pendulum;
        break;
    case 4:
        SBUF=flash;
        break;
    case 5:
        SBUF=set_mode;
        break;
    case 6:
        SBUF=hour_alarm;
        break;
    //调试：adjust大致的值，255（char容积）分度
    case 7:
        SBUF=adjust/(adjust_offset/255);
        break;
    //设置：根据接收设置相应的时间数值
    /*
    h=8(0)~31(23)	小时：偏移量-8
    m=32(0)~91(59)	分钟：偏移量-32
    s=92(0)~151(59)	秒钟：偏移量-92
    */
    default:
        if(dat<152)
        {
            if(dat<32)
                hour=dat-8;
            else if(dat<92)
                min=dat-32;
            else
            {
                sec=dat-92;
                //只有设置秒时，相关计数才会置0
                adjust=0;
                fiftyms_count=0;
            }
            //有效命令指示
            SBUF='!';
        }
        else
            //无效命令指示
            SBUF='?';
    }
    while(!TI);//等待发送数据完成
    //复位发送中断标志
    TI=0;
}
