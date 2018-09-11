/**
为另外一块单片机实验板(YL-39)移植此时钟程序，仅注释修改部分。
注：因为用不上，删去了用蜂鸣器整点报时的功能。加上了按下K5转换是否显示时间状态以节省耗能。
作者：nijigenirubasho @Github 2018-09-11
*/
#include <reg52.h>
#include <intrins.h>
#define lsd_seg P0
#define ledx P1
#define lsd_pos P2
//修改校准值
#define adjust_offset 10805
//修改按键变量名及针脚
sbit key3=P3^5;
sbit key4=P3^4;
sbit key5=P3^3;
sbit key6=P3^2;
sbit led1=P1^0;
sbit led4=P1^3;
sbit led5=P1^4;
sbit led7=P1^6;
//sbit beep=P2^3;
int fiftyms_count=0,pendulum,flash,set_mode,hour_alarm,adjust=0;
int hour,min,sec;
//显示状态
bit is_display=1;
//修改数码管位选数组
unsigned char code lsd_list[]= {0xf8,0xf4,0xf2,0xf1};
//修改为共阳数码管码表
unsigned char code charset[]= {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90};
void delay(unsigned int ms)
{
    unsigned int x;
    while(ms--)
        for(x=125; x>0; x--);
}
void lsd_display(int which,int body)
{
    //修改段选置空方式
    lsd_seg=0xff;
    //修改为P2.0到3置1
    lsd_pos|=0x0f;
    //修改要位置0
    lsd_pos=~(lsd_pos&lsd_list[which]);
    switch(set_mode)
    {
    case 0:
        if(which==2&&pendulum)
            //修改加点方式
            lsd_seg=charset[body]-0x80;
        else
            lsd_seg=charset[body];
        break;
    case 1:
        if((which==2||which==3)&&pendulum)
            lsd_seg=charset[body];
        else
            //修改清空数码管方式
            lsd_seg=0xff;
        break;
    case 2:
        if((which==0||which==1)&&pendulum)
            lsd_seg=charset[body];
        else
            lsd_seg=0xff;
    }
    delay(1);
}
void lsd_show_all(int a,int b,int c,int d)
{
    lsd_display(3,a);
    lsd_display(2,b);
    lsd_display(1,c);
    lsd_display(0,d);
}
void set_timer_para()
{
    TH0=76;
    TL0=0;
}
void serial_init()
{
    SCON=0x50;
    TMOD|=0x20;
    PCON=0x00;
    TH1=0xfd;
    TL1=0xfd;
    ES=1;
    TR1=1;
}
void main()
{
    EA=1;
    TMOD=0x01;
    set_timer_para();
    serial_init();
    hour=11;
    min=59;
    sec=57;
    ET0=1;
    TR0=1;
    while(1)
    {
        //不在显示状态的话消隐数码管
        if(is_display)
            lsd_show_all(hour/10,hour%10,min/10,min%10);
        else
            lsd_seg=0xff;
        if(key3==0)
        {
            //如果不在显示状态则不做任何操作
            if(is_display)
            {
                delay(10);
                if(key3==0)
                {
                    set_mode++;
                    if(set_mode==3)
                    {
                        set_mode=0;
                        sec=0;
                        fiftyms_count=0;
                        adjust=0;
                    }
                }
            }
            while(key3==0);
        }
        if(key4==0)
        {
            if(is_display)
            {
                delay(10);
                if(key4==0)
                {
                    switch(set_mode)
                    {
                    case 1:
                        hour++;
                        if(hour==24)
                            hour=0;
                        break;
                    case 2:
                        min++;
                        if(min==60)
                            min=0;
                    }
                }
                while(key4==0)
                {
                    if(!set_mode)
                    {
                        lsd_show_all(min/10,min%10,sec/10,sec%10);
                        led7=0;
                    }
                }
                led7=1;
            }
        }
        if(key5==0)
        {
            delay(10);
            if(key5==0)
            {
                switch(set_mode)
                {
                case 0:
                    //显示状态反转
                    is_display=!is_display;
                    break;
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
            while(key5==0)
            {
                if(!set_mode)
                {
                    unsigned char percent=adjust/(adjust_offset/100+1);
                    lsd_show_all(7,5,percent/10,percent%10);
                }
            }
        }
        if(key6==0)
        {
            delay(10);
            if(key6==0)
            {
                if(set_mode)
                    set_mode=0;
                else if(min!=0||(sec!=0&&sec!=1))
                {
                    hour_alarm=!hour_alarm;
                    led5=!hour_alarm;
                }
                while(key6==0);
            }
        }
    }
}
void timer0() interrupt 1 using 1
{
    set_timer_para();
    fiftyms_count++;
    if(fiftyms_count==20)
    {
        sec++;
        adjust++;
        if(adjust==adjust_offset)
        {
            led4=0;
            adjust=0;
            sec++;
        }
        else
            led4=1;
        pendulum=!pendulum;
        fiftyms_count=0;
        if(sec==60)
        {
            min++;
            sec=0;
        }
        if(min==60)
        {
            hour++;
            min=0;
        }
        if(hour==24)
            hour=0;
        if(hour_alarm&&!set_mode)
            if(min==0&&(sec==0||sec==1))
            {
                //beep=~beep;
                ledx=~ledx;
            }
    }
    if(led1==0)
        led1=1;
    if(pendulum!=flash)
    {
        led1=~led1;
        flash=pendulum;
    }
}
void serial_io() interrupt 4 using 2
{
    unsigned char dat;
    dat=SBUF;
    RI=0;
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
    case 7:
        SBUF=adjust/(adjust_offset/255+1);
        break;
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
                adjust=0;
                fiftyms_count=0;
            }
            SBUF='!';
        }
        else
            SBUF='?';
    }
    _nop_();
    while(!TI);
    TI=0;
}
