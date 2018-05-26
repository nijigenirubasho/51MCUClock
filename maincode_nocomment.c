
#include <reg52.h>
#define lsd_seg P0
#define ledx P1
#define lsd_pos P2
#define adjust_offset 2789
sbit key2=P3^4;
sbit key3=P3^5;
sbit key4=P3^6;
sbit key5=P3^7;
sbit led1=P1^0;
sbit led4=P1^3;
sbit led5=P1^4;
sbit led7=P1^6;
sbit beep=P2^3;
int fiftyms_count=0,pendulum,flash,set_mode,hour_alarm,adjust=0;
int hour,min,sec;
unsigned char code lsd_list[]= {0x8f,0x4f,0x2f,0x1f};
unsigned char code charset[]= {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
void delay(int i)
{
    int j,k;
    for(j=i; j>0; j--)
        for(k=124; k>0; k--);
}
void lsd_display(int which,int body)
{
    lsd_seg=0x00;
    lsd_pos|=0xf0;
    lsd_pos=lsd_pos&lsd_list[which];
    switch(set_mode)
    {
    case 0:
        if(which==2&&pendulum)
            lsd_seg=charset[body]+0x80;
        else
            lsd_seg=charset[body];
        break;
    case 1:
        if((which==2||which==3)&&pendulum)
            lsd_seg=charset[body];
        else
            lsd_seg=0x00;
        break;
    case 2:
        if((which==0||which==1)&&pendulum)
            lsd_seg=charset[body];
        else
            lsd_seg=0x00;
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
        lsd_show_all(hour/10,hour%10,min/10,min%10);
        if(key2==0)
        {
            delay(10);
            if(key2==0)
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
            while(key2==0);
        }
        if(key3==0)
        {
            delay(10);
            if(key3==0)
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
            while(key3==0)
            {
                if(!set_mode)
                {
                    lsd_show_all(min/10,min%10,sec/10,sec%10);
                    led7=0;
                }
            }
            led7=1;
        }
        if(key4==0)
        {
            delay(10);
            if(key4==0)
            {
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
            while(key4==0)
            {
                if(!set_mode)
                {
                    unsigned char percent=adjust/(adjust_offset/100);
                    lsd_show_all(7,5,percent/10,percent%10);
                }
            }
        }
        if(key5==0)
        {
            delay(10);
            if(key5==0)
            {
                if(set_mode)
                    set_mode=0;
                else
                    if(min!=0||(sec!=0&&sec!=1))
                    {
                        hour_alarm=!hour_alarm;
                        led5=!hour_alarm;
                    }
                while(key5==0);
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
                beep=~beep;
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
        SBUF=adjust/(adjust_offset/255);
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
    while(!TI);
    TI=0;
}
