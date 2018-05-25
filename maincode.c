/**--------------------------------------------------------------------------------------------------------------------------------------------------------------------
��51��Ƭ��(STC89C52,�ǲ�����BST-M51�̲Ķ��ư濪����)��������ʱ��

���ߣ�nijigenirubasho @Github 2018-05-25

���ܼ�����1.Сʱ/������ʾ�����á�2.ÿ������һ�Σ������Դ˶��롣3.��ÿСʱ����һ�εĹ��ܡ�

ʹ��˵����
    1.LED�ƣ�D1��Ϊ����ָʾ��ÿ�뷢��50��������⡣D4Уʱ��ʾ����ʱ������һ��ʱ�䣨һ�¹������룩���벻Ҫ����ʱ���������Ͳ����豸��
						 D5��ʾÿСʱ����ģʽ״̬��������ʾ�˹��ܿ�����D7��ʾ�Ƿ���ʾ���ӡ�����������ÿСʱ����ĸ�����ʾ��
    2.��Դ�����������ÿСʱ���幦�ܿ����������ȥʱ��������1������塣��֮������Ĭ��
    3.������K2�л�����ʾʱ��/����Сʱ/���÷��ӣ�ģʽ�������÷����л���ʾʱ��ģʽʱ�������㣬�����Դ˾�ȷУʱ��
            K3��K4������ʱ��ģʽ�и���Ӽ���ǰ���õ���Ŀ��������ʾʱ��ģʽ�У���סK3����ʾ���ӵ�ֵ��
						K5����ʾʱ��ģʽ�л�ÿСʱ����ģʽ״̬����������ģʽ�а��¿���ֱ���˳�����ģʽ�����������룬����������ģʽ��Ϊһ�������ʱ���������ȡ��ʱ����޸ġ�
    4.LED����ܣ���ʾʱ��ģʽ����ʾ(Сʱ.����)���ڰ�סK3ʱ��ʾ��(����.����)���м�ĵ�һ�����л�һ��(��/��)״̬��������ʱ��ģʽ��ֻ�б���������1Hz��Ƶ������˸��
    5.����ͨѶ���������ã�9600�����ʣ�������������ʱ��У׼��HEXģʽ��ֵ�����������1.Сʱ��+8 2.������+20 3.������+5C��ֻ������charһ������һ����Ӧ�ģ�ʱ/��/�룩��ֵ��

����ʾʱ��=��ʵʱ���(<1s/day)��ע�⣺���Ǹ��������ϵĻ����ͻ���������ģ����ʵ�ʲ������������޸�adjust_offset����ֵΪ������������һ�룩

��ԴЭ�飺MIT�����ڵ�ͬ��WTCPL��
---------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
//STC89C52��ͷ�ļ�
// > Header file for generic 80C52 and 80C32 microcontroller.
#include <reg52.h>
//����ܶ�ѡ������
#define lsd_seg P0
//LED����
#define ledx P1
//�����λѡ������
#define lsd_pos P2
//��ʱ���ƫ����
//����һ�����ֲ⣬7Сʱ����9�룬Ҳ����˵ÿ7*3600/9=2800���Ҫ��һ��
//���Զ����ڲ���һ�����£�����17ʱ40�֣��������룬(17*3600+40*60)/2=31800�����һ�룬��ȥ31800/2800=11�룬��2800-11=2789��
#define adjust_offset 2789
//����key2��key5��Ӧʵ����K2��K5
sbit key2=P3^4;
sbit key3=P3^5;
sbit key4=P3^6;
sbit key5=P3^7;
//����ָʾ����һ�´���+1s
sbit led1=P1^0;
//Уʱ��ʾ
sbit led4=P1^3;
//ÿСʱ����ָʾ����������
sbit led5=P1^4;
//�Ƿ���ʾ��������
sbit led7=P1^6;
//��Դ������������ָʾÿСʱ����
sbit beep=P2^3;
//����������50ms������1s��¼���Ӱڣ������⣨����ָʾ��������ģʽ��ÿСʱ����ģʽ��У׼
int fiftyms_count=0,pendulum,flash,set_mode,hour_alarm,adjust=0;
//ʱ��洢��ʱ����
int hour,min,sec;
//led�����(LED Segment Displays)��������λ�б�
unsigned char code lsd_list[]= {0x8f,0x4f,0x2f,0x1f};
//�ַ������0123456789
unsigned char code charset[]= {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
//�����ʱ��CPUʱ��ռ���ӳ�(ms)
void delay(int i)
{
    int j,k;
    for(j=i; j>0; j--)
        for(k=124; k>0; k--);
}
//��ʾһλ����
void lsd_display(int which,int body)
{
    lsd_seg=0x00;//��ѡ��0��ֹ��Ӱ
    lsd_pos|=0xf0;//P2^4~P2^7��1
    lsd_pos=lsd_pos&lsd_list[which];//Ҫλ��1������0
    /*
    ����ģʽ��
    	0��ʱ����ʾ
    	1��Сʱ����
    	2����������
    */
    switch(set_mode)
    {
    case 0:
        //�Ӱھ�����ʾ�ָ��õĵ㣬�ڵ������������
        if(which==2&&pendulum)
            //�ӵ㣺0x80 ��
            lsd_seg=charset[body]+0x80;
        else
            lsd_seg=charset[body];
        break;
    case 1:
        //23��Сʱ��˸������������ʧ
        if((which==2||which==3)&&pendulum)
            lsd_seg=charset[body];
        else
            lsd_seg=0x00;
        break;
    case 2:
        //01��������˸������������ʧ
        if((which==0||which==1)&&pendulum)
            lsd_seg=charset[body];
        else
            lsd_seg=0x00;
    }
    //�ӳٱ�֤��ʾ���Ⱥ��Ӿ���������
    delay(1);
}
//��ʾ����4λ���֣���������˳���տ��������������λ��
void lsd_show_all(int a,int b,int c,int d)
{
    lsd_display(3,a);
    lsd_display(2,b);
    lsd_display(1,c);
    lsd_display(0,d);
}
//���ö�ʱ����ֵ����
void set_timer_para()
{
    /*����11.0592MHz
      TH:(65536-ms*1000*11.0592/12)/256
      TL:(65536-ms*1000*11.0592/12)%256
      50ms��ע�ⲻҪ���*/
    TH0=76;
    TL0=0;
}
//��ʼ�����ж˿�
void serial_init()
{
    //0101 0000 ��ʽ1��������մ���
    SCON=0x50;
    //0001 0100 T1 ��ʽ�� ������������ʱ��������ʽ
    TMOD|=0x20;
    //SMOD=0
    PCON=0x00;
    //9600�����ʣ����11-6�ó�
    TH1=0xfd;
    TL1=0xfd;
    //�����жϴ�
    ES=1;
    //��������
    TR1=1;
}
//������
void main()
{
    //���жϴ�
    EA=1;
    //0000 0001 T0 ��ʱ ��ʽһ
    TMOD=0x01;
    //��ʼ����ʱ����ֵ�ʹ���
    set_timer_para();
    serial_init();
    //���ó�ʼʱ�䣺11:59:57
    hour=11;
    min=59;
    sec=57;
    //��ʱ��T0�жϿ���
    ET0=1;
    //T0����
    TR0=1;
    //��ѯ
    while(1)
    {
        //Сʱʮλ����Сʱ��λ��������ʮλ�������Ӹ�λ��
        lsd_show_all(hour/10,hour%10,min/10,min%10);
        //key2����
        if(key2==0)
        {
            //����
            delay(10);
            //�ٴμ���
            if(key2==0)
            {
                //����ģʽѭ����0 1 2
                set_mode++;
                if(set_mode==3)
                {
                    set_mode=0;
                    //�����ؼ�����0����Уʱ
                    sec=0;
                    fiftyms_count=0;
                    adjust=0;
                }
            }
            //��ͣ���ſ��ż������ӻ�����
            while(key2==0);
        }
        //key3����
        if(key3==0)
        {
            delay(10);
            if(key3==0)
            {
                //������ֵ����ģʽ
                switch(set_mode)
                {
                //Сʱ
                case 1:
                    hour++;
                    //��24����
                    if(hour==24)
                        hour=0;
                    break;
                //����
                case 2:
                    min++;
                    //��60����
                    if(min==60)
                        min=0;
                }
            }
            //�ڷ�����ģʽʱ����סkey3����ʾ���룬��������led7
            while(key3==0)
            {
                if(!set_mode)
                {
                    lsd_show_all(min/10,min%10,sec/10,sec%10);
                    led7=0;
                }
            }
            //Ϩ��led7
            led7=1;
        }
        //key4����
        if(key4==0)
        {
            delay(10);
            if(key4==0)
            {
                //������ֵ����ģʽ
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
            //���ԣ���סkey4��ʾadjust�ٷֱ�
            while(key4==0)
            {
                //��������ģʽ�Ҳ���100%
                if(!set_mode)
                {
                    //һ��Ҫ������������⣺adjust*100/adjust_offset������
                    unsigned char percent=adjust/(adjust_offset/100);
                    //����adjust_offset�����٣���˲����ж�100%����Ϊ��Զ�����ܴﵽ
                    //if(percent/10!=10)
                    //75��adjust����ĸ˳���
                    lsd_show_all(7,5,percent/10,percent%10);
                }
            }
        }
        //key5����
        if(key5==0)
        {
            delay(10);
            if(key5==0)
            {
                //���������ģʽ��������������˳���������ĺ��ҩ
                if(set_mode)
                    set_mode=0;
                else
                    //�ڲ�������Сʱ����²������ã�
                    //��ֹ����Сʱ����¸�Ϊ�˿�����ʱ����һ����޷�����״̬������
                    if(min!=0||(sec!=0&&sec!=1))
                    {
                        //��תÿСʱ����ģʽ����
                        hour_alarm=!hour_alarm;
                        //ע��LED��0������
                        led5=!hour_alarm;
                    }
                while(key5==0);
            }
        }
    }
}
//T0�ж� 50ms��ʹ��using��ʡ�������ڣ�������ͬ�ж����ȼ�����ʹ�ò�ͬ�ļĴ����鲢�Ҿ�������ʹ�ú���������������ݿ��ܳ����������Ѿ�ռ��0�Ĵ�����
void timer0() interrupt 1 using 1
{
    //��ֹ�������������Ҫ���ó�ֵ
    set_timer_para();
    //50ms����+1
    fiftyms_count++;
    //50ms*20=1000ms=1s ����if��Χ���¼�ÿ��ִ��һ��
    if(fiftyms_count==20)
    {
        //��+1
        sec++;
        adjust++;
        //У׼
        if(adjust==adjust_offset)
        {
            //��led4����ʾУʱ�ź�
            led4=0;
            adjust=0;
            sec++;
        }
        else
            //����һ���ر�led4
            led4=1;
        //�Ӱڷ�ת���ڶ�һ�£�
        pendulum=!pendulum;
        //����50ms����
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
        //���Сʱ���忪�����Ҳ�������ģʽ���룬��ֹ�ڵ�ʱ��ʱ�ܵ�����
        if(hour_alarm&&!set_mode)
            //����Ϊ�㣬��Ϊ0��1��1��������״̬��
            if(min==0&&(sec==0||sec==1))
            {
                //��ת�����LED����
                beep=~beep;
                ledx=~ledx;
                /*0�����壬LED���д�붼����
                  1���ر����壬led�ָ�*/
            }
    }
    //1s����
    //�������ָʾ���⣬��ر�֮
    if(led1==0)
        led1=1;
    //�Ӱ��Ƿ������ͬ��
    if(pendulum!=flash)
    {
        //��ͬ����תled1
        /*
        ��֤��1sֻ��תһ�Σ����ҷ�ת��������һ��50ms���ϸ��жϹر�����ָʾ
        */
        led1=~led1;
        flash=pendulum;
    }
}
//�����жϣ�������9600��������Ϣͨ�����ж˿������ʱ�������ź����룬ѡ��0~7���������8~151����ʱ�䣬���ܻ�����CPU���������󣬲�ҪƵ������IO
void serial_io() interrupt 4 using 2
{
    //ȡ���������ݻ���Ĵ�����������dat��ֻ�����дһ��char
    unsigned char dat;
    dat=SBUF;
    //��λ�����жϱ�־
    RI=0;
    //���ԣ�������������ػ�������Ӧ������Ϣ
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
    //���ԣ�adjust���µ�ֵ��255��char�ݻ����ֶ�
    case 7:
        SBUF=adjust/(adjust_offset/255);
        break;
    //���ã����ݽ���������Ӧ��ʱ����ֵ
    /*
    h=8(0)~31(23)	Сʱ��ƫ����-8
    m=32(0)~91(59)	���ӣ�ƫ����-32
    s=92(0)~151(59)	���ӣ�ƫ����-92
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
                //ֻ��������ʱ����ؼ����Ż���0
                adjust=0;
                fiftyms_count=0;
            }
            //��Ч����ָʾ
            SBUF='!';
        }
        else
            //��Ч����ָʾ
            SBUF='?';
    }
    while(!TI);//�ȴ������������
    //��λ�����жϱ�־
    TI=0;
}
