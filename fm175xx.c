/*************************************************************/
//2014.03.06�޸İ�
/*************************************************************/
#include "spi.h"
#include "main.h"
#include "fm175xx.h"
#include "STC15F2K60S2.h"
#include "pin.h"
#include <string.h>	
#include "uart.h"

 char wde=0;
/*************************************************************/
/*��������	    Read_Reg
/*���ܣ�	    ���Ĵ�������
/*���������	reg_add���Ĵ�����ַ
/*����ֵ��	    �Ĵ�����ֵ
/*************************************************************/
unsigned char Read_Reg(unsigned char reg_add)
{
unsigned char data reg_value;	   
 reg_value=SPIRead(reg_add);
 return reg_value;
}
/*************************************************************/
/*��������	    Read_Reg All
/*���ܣ�	    ��64���Ĵ�������
/*���������	��
/*�������		*reg_value���Ĵ�����ֵָ��
/*����ֵ��	    OK
/*************************************************************/
unsigned char Read_Reg_All(unsigned char *reg_value)
{
unsigned char data i;
	for (i=0;i<64;i++)	   
		*(reg_value+i)=SPIRead(i);
 return OK;
}
/*************************************************************/
/*��������	    Write_Reg									 */
/*���ܣ�	    д�Ĵ�������								 */
/*															 */
/*���������	reg_add���Ĵ�����ַ��reg_value���Ĵ�����ֵ	 */
/*����ֵ��	    OK											 */
/*				ERROR										 */
/*************************************************************/
unsigned char Write_Reg(unsigned char reg_add,unsigned char reg_value)
{
	SPIWrite(reg_add,reg_value);
	return OK;
}
/*************************************************************/
/*��������	    Read_FIFO									 */
/*���ܣ�	    ��ȡFIFO  									 */
/*															 */
/*���������	length����ȡ���ݳ���						 */
/*���������	*fifo_data�����ݴ��ָ��					 */
/*����ֵ��	    											 */
/*															 */
/*************************************************************/
void Read_FIFO(unsigned char length,unsigned char *fifo_data)
{	 
	SPIRead_Sequence(length,FIFODataReg,fifo_data);
	return;
}
/*************************************************************/
/*��������	    Write_FIFO									 */
/*���ܣ�	    д��FIFO  									 */
/*															 */
/*���������	length����ȡ���ݳ��ȣ�*fifo_data�����ݴ��ָ��*/
/*���������												 */
/*����ֵ��	    											 */
/*															 */
/*************************************************************/
void Write_FIFO(unsigned char length,unsigned char *fifo_data)
{
	SPIWrite_Sequence(length,FIFODataReg,fifo_data);
	return;
}
/*************************************************************/
/*��������	    Clear_FIFO									 */
/*���ܣ�	    ���FIFO  									 */
/*															 */
/*���������	��											*/
/*���������												 */
/*����ֵ��	    OK											 */
/*				ERROR										 */
/*************************************************************/
unsigned char Clear_FIFO(void)
{
	 Set_BitMask(FIFOLevelReg,0x80);//���FIFO����
	 if (SPIRead(FIFOLevelReg)==0)
	 	return OK;
	else
		return ERROR;
}
/*************************************************************/
/*��������	    Set_BitMask									 */
/*���ܣ�	    ��λ�Ĵ�������								 */
/*���������	reg_add���Ĵ�����ַ��mask���Ĵ�����λ		 */
/*����ֵ��	    OK											 */
/*				ERROR										 */
/*************************************************************/
unsigned char Set_BitMask(unsigned char reg_add,unsigned char mask)
{
    unsigned char data result;
    result=Write_Reg(reg_add,Read_Reg(reg_add) | mask);  // set bit mask
	return result;
}
/*********************************************/
/*��������	    Clear_BitMask
/*���ܣ�	    ���λ�Ĵ�������
				
/*���������	reg_add���Ĵ�����ַ��mask���Ĵ������λ
/*����ֵ��	    OK
				ERROR
/*********************************************/
unsigned char Clear_BitMask(unsigned char reg_add,unsigned char mask)
{
    unsigned char data result;
    result=Write_Reg(reg_add,Read_Reg(reg_add) & ~mask);  // clear bit mask
	return result;
}
/*********************************************/
/*��������	    Set_RF
/*���ܣ�	    ������Ƶ���
				
/*���������	mode����Ƶ���ģʽ
				0���ر����
				3��TX1��TX2�������TX2Ϊ�������
/*����ֵ��	    OK
				ERROR
/*********************************************/

unsigned char Set_Rf(unsigned char mode)
{
unsigned char data result;
if	((Read_Reg(TxControlReg)&0x03)==mode)
	return OK;
if (mode==0)
	{
	result=Clear_BitMask(TxControlReg,0x03); //�ر�TX1��TX2���
	}
if (mode==1)
	{
	result=Set_BitMask(TxControlReg,0x01); //����TX1���
	result=Clear_BitMask(TxControlReg,0x02);
	}
if (mode==2)
	{
	result=Clear_BitMask(TxControlReg,0x01);
	result=Set_BitMask(TxControlReg,0x02); //����TX2���
	}
if (mode==3)
	{
	result=Set_BitMask(TxControlReg,0x03); //��TX1��TX2���
	}
    Delay_100us(1000);
return result;
}
/*********************************************/
/*��������	    Pcd_Comm
/*���ܣ�	    ������ͨ��
				
/*���������	Command��ͨ�Ų������
				pInData�������������飻
				InLenByte���������������ֽڳ��ȣ�
				pOutData�������������飻
				pOutLenBit���������ݵ�λ����
/*����ֵ��	    OK
				ERROR
/*********************************************/  
unsigned char Pcd_Comm(	unsigned char Command, 
                 		unsigned char *pInData, 
                 		unsigned char InLenByte,
                 		unsigned char *pOutData, 
                 		unsigned int *pOutLenBit)
{
	unsigned char data result;
	unsigned char data rx_temp=0;//��ʱ�����ֽڳ���
	unsigned char data rx_len=0;//���������ֽڳ���
	unsigned char data lastBits=0;//��������λ����
	unsigned char data irq;
	
  Debug_Send_char1('\\');
  Debug_Send_char1(Command);
  
	Clear_FIFO();
	Write_Reg(CommandReg,Idle);
	Write_Reg(WaterLevelReg,0x20);//����FIFOLevel=32�ֽ�
	Write_Reg(ComIrqReg,0x7F);//���IRQ��־

	if(Command==MFAuthent)
	{
		Write_FIFO(InLenByte,pInData);//������֤��Կ
		Set_BitMask(BitFramingReg,0x80);//��������
	}
	Set_BitMask(TModeReg,0x80);//�Զ�������ʱ��
	Write_Reg(CommandReg,Command);
  
  Debug_Send_char1('\\');
  Debug_Send_char1(Command); 
   
	while(1)//ѭ���ж��жϱ�ʶ
	{
    Debug_Send_char1('0');
		irq = Read_Reg(ComIrqReg);//��ѯ�жϱ�־
    Debug_Send_char1(irq);
		if(irq&0x01)		//TimerIRq  ��ʱ��ʱ���þ�
		{
			result=ERROR;
      Debug_Send_char1('3');
    Debug_Send_char1(Command);
      		
			break;
		}
		if(Command==MFAuthent)
		{
			if(irq&0x10)	//IdelIRq  command�Ĵ���Ϊ���У�ָ��������
			{
				result=OK;
				break;
			}
		}
		if(Command==Transmit)
		{
					if((irq&0x04)&&(InLenByte>0))//LoAlertIrq+�����ֽ�������0
						{
							if (InLenByte<32)
								{
									Write_FIFO(InLenByte,pInData);	
									InLenByte=0;
								}
								else
								{
									Write_FIFO(32,pInData);
									InLenByte=InLenByte-32;
									pInData=pInData+32;
								}
							Write_Reg(ComIrqReg,0x04);//���LoAlertIrq
							Set_BitMask(BitFramingReg,0x80);//��������
						
						}	
					
					if((irq&0x40)&&(InLenByte==0))		//TxIRq
					{
						result=OK;
					  	break;
					}
				}
							  
			if(Command==Transceive)
				{
					if((irq&0x04)&&(InLenByte>0))//LoAlertIrq+�����ֽ�������0
					{	
						if (InLenByte>32)
							{
								Write_FIFO(32,pInData);
								InLenByte=InLenByte-32;
								pInData=pInData+32;
							}
						else
							{
								Write_FIFO(InLenByte,pInData);
								InLenByte=0;
							}
						Set_BitMask(BitFramingReg,0x80);//��������
						Write_Reg(ComIrqReg,0x04);//���LoAlertIrq
					}
					if(irq&0x08)	//HiAlertIRq
					{
						 if((irq&0x40)&&(InLenByte==0)&&(Read_Reg(FIFOLevelReg)>32))//TxIRq    ԭ��
					
						  	{
								Read_FIFO(32,pOutData+rx_len); //����FIFO����
								rx_len=rx_len+32;
								Write_Reg(ComIrqReg,0x08);	//��� HiAlertIRq
							}
						}
				    if((irq&0x20)&&(InLenByte==0))	//RxIRq=1
						{	wde=1;
							result=OK;
					  		break;
						}
				    }
				}
        Debug_Send_char1('2');
//    if (Read_Reg(ErrorReg)&0x0F)
//        {
// 			result = ERROR;
//        }
//        else

	    {   
			 if (Command == Transceive)
			 	{
        Debug_Send_char1('4');
        rx_len=0;//
					rx_temp=Read_Reg(FIFOLevelReg);
					Debug_Send_char1('5');
          Debug_Send_char1(rx_temp);
          Read_FIFO(rx_temp,pOutData+rx_len); //����FIFO����
          Debug_Send_char1('6');
					rx_len=rx_len+rx_temp;//���ճ����ۼ�
                
              		lastBits = Read_Reg(ControlReg) & 0x07;
                if (lastBits)
	                *pOutLenBit = (rx_len-1)*(unsigned int)8 + lastBits;  
			    else
	                *pOutLenBit = rx_len*(unsigned int)8;   
				}
		}
    Debug_Send_char1('1');
    Set_BitMask(ControlReg,0x80);           // stop timer now
    Write_Reg(CommandReg,Idle); 
 	Clear_BitMask(BitFramingReg,0x80);//�رշ���
	return result;
}
/*********************************************/
/*��������	    Pcd_SetTimer
/*���ܣ�	    ���ý�����ʱ
/*���������	delaytime����ʱʱ�䣨��λΪ���룩
/*����ֵ��	    OK
/*********************************************/
 unsigned char Pcd_SetTimer(unsigned long delaytime)//�趨��ʱʱ�䣨ms��
{
	unsigned long data TimeReload;
	unsigned int data Prescaler;

	Prescaler=0;
	TimeReload=0;
	while(Prescaler<0xfff)
	{
		TimeReload = ((delaytime*(long)13560)-1)/(Prescaler*2+1);
		
		if( TimeReload<0xffff)
			break;
		Prescaler++;
	}
		TimeReload=TimeReload&0xFFFF;
		Set_BitMask(TModeReg,Prescaler>>8);
		Write_Reg(TPrescalerReg,Prescaler&0xFF);					
		Write_Reg(TReloadMSBReg,TimeReload>>8);
		Write_Reg(TReloadLSBReg,TimeReload&0xFF);
	return OK;
}
/*********************************************/
/*��������	    Pcd_ConfigISOType
/*���ܣ�	    ���ò���Э��
/*���������	type 0��ISO14443AЭ�飻
/*					 1��ISO14443BЭ�飻
/*����ֵ��	    OK
/*********************************************/
unsigned char Pcd_ConfigISOType(unsigned char type)
{
	
   if (type == 0)                     //ISO14443_A
   { 
   		Set_BitMask(ControlReg, 0x10); //ControlReg 0x0C ����readerģʽ
		Set_BitMask(TxAutoReg, 0x40); //TxASKReg 0x15 ����100%ASK��Ч
	    Write_Reg(TxModeReg, 0x00);  //TxModeReg 0x12 ����TX CRC��Ч��TX FRAMING =TYPE A
	    Write_Reg(RxModeReg, 0x00); //RxModeReg 0x13 ����RX CRC��Ч��RX FRAMING =TYPE A
   }
   if (type == 1)                     //ISO14443_B
   	{ 
   		Write_Reg(ControlReg, 0x10); //ControlReg 0x0C ����readerģʽ
	    Write_Reg(TxModeReg, 0x83); //TxModeReg 0x12 ����TX CRC��Ч��TX FRAMING =TYPE B
		Write_Reg(RxModeReg, 0x83); //RxModeReg 0x13 ����RX CRC��Ч��RX FRAMING =TYPE B
        Write_Reg(GsNReg, 0xF4); //GsNReg 0x27 ����ON�絼
        Write_Reg(GsNOffReg, 0xF4); //GsNOffReg 0x23 ����OFF�絼
        Write_Reg(TxAutoReg, 0x00);// TxASKReg 0x15 ����100%ASK��Ч
	}
   if (type == 2)                     //Felica
   	{ 
   		Write_Reg(ControlReg, 0x10); //ControlReg 0x0C ����readerģʽ
	    Write_Reg(TxModeReg, 0x92); //TxModeReg 0x12 ����TX CRC��Ч��212kbps,TX FRAMING =Felica
		Write_Reg(RxModeReg, 0x96); //RxModeReg 0x13 ����RX CRC��Ч��212kbps,Rx Multiple Enable,RX FRAMING =Felica
        Write_Reg(GsNReg, 0xF4); //GsNReg 0x27 ����ON�絼
		Write_Reg(CWGsPReg, 0x20); //
		Write_Reg(GsNOffReg, 0x4F); //GsNOffReg 0x23 ����OFF�絼
		Write_Reg(ModGsPReg, 0x20); 
        Write_Reg(TxAutoReg, 0x07);// TxASKReg 0x15 ����100%ASK��Ч
	}

   return OK;
}
/*********************************************/
/*��������	    FM175X_SoftReset
/*���ܣ�	    ����λ����
				
/*���������	
				

/*����ֵ��	    OK
				ERROR
/*********************************************/

unsigned char  FM175X_SoftReset(void)
  {	
	Write_Reg(CommandReg,SoftReset);//
	Set_BitMask(ControlReg,0x10);//17520����ΪREADER
	if (Read_Reg(ControlReg)&0x10)
		return OK;
	else
		return	ERROR;
  }

unsigned char FM175X_HardReset(void)
  {	
	NPD=0;
	Delay_100us(1);
	NPD=1;
	Delay_100us(1);
	return OK;
  }
/*********************************************/
/*��������	    FM175X_SoftPowerdown
/*���ܣ�	    �����͹��Ĳ���
/*���������	
/*����ֵ��	    OK������͹���ģʽ��
/*				ERROR���˳��͹���ģʽ��
/*********************************************/
unsigned char FM175X_SoftPowerdown(void)
{
	if (Read_Reg(CommandReg)&0x10)
		{
		Clear_BitMask(CommandReg,0x10);//�˳��͹���ģʽ
		return ERROR;
		}
	else
		Set_BitMask(CommandReg,0x10);//����͹���ģʽ
		return OK;
}
/*********************************************/
/*��������	    FM175X_HardPowerdown
/*���ܣ�	    Ӳ���͹��Ĳ���
/*���������	
/*����ֵ��	    OK������͹���ģʽ��
/*				ERROR���˳��͹���ģʽ��
/*********************************************/
unsigned char FM175X_HardPowerdown(void)
{	
	NPD=~NPD;
	if(NPD==1)
		return OK; //����͹���ģʽ
	else
		return ERROR;//�˳��͹���ģʽ
}
/*********************************************/
/*��������	    Read_Ext_Reg
/*���ܣ�	    ��ȡ��չ�Ĵ���
/*���������	reg_add���Ĵ�����ַ
/*����ֵ��	    �Ĵ�����ֵ
/*********************************************/
unsigned char Read_Ext_Reg(unsigned char reg_add)
{
 	Write_Reg(0x0F,0x80+reg_add);
 	return Read_Reg(0x0F);
}
/*********************************************/
/*��������	    Write_Ext_Reg
/*���ܣ�	    д����չ�Ĵ���
/*���������	reg_add���Ĵ�����ַ��reg_value���Ĵ�����ֵ
/*����ֵ��	    OK
				ERROR
/*********************************************/
unsigned char Write_Ext_Reg(unsigned char reg_add,unsigned char reg_value)
{
	Write_Reg(0x0F,0x40+reg_add);
	Write_Reg(0x0F,0xC0+reg_value);
	return OK; 
}