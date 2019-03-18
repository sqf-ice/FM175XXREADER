/*************************************************************/
//2014.07.15�޸İ�
/*************************************************************/
#include "mifare_card.h"
#include "STC15F2K60S2.h"
#include "pcd.h"
/*****************************************************************************************/
/*���ƣ�Mifare_Auth																		 */
/*���ܣ�Mifare_Auth��Ƭ��֤																 */
/*���룺mode����֤ģʽ��0��key A��֤��1��key B��֤����sector����֤�������ţ�0~15��		 */
/*		*mifare_key��6�ֽ���֤��Կ���飻*card_uid��4�ֽڿ�ƬUID����						 */
/*���:																					 */
/*		OK    :��֤�ɹ�																	 */
/*		ERROR :��֤ʧ��																	 */
/*****************************************************************************************/
 unsigned char Mifare_Auth(unsigned char mode,unsigned char sector,unsigned char *mifare_key,unsigned char *card_uid)
{
	unsigned char data send_buff[12],rece_buff[1],result;
	unsigned int data rece_bitlen;
	if(mode==0x0)
		send_buff[0]=0x60;//60 kayA��ָ֤��
	if(mode==0x1)
		send_buff[0]=0x61;//61 keyB��ָ֤��
  	send_buff[1]=sector*4;//��֤�����Ŀ�0��ַ
	send_buff[2]=mifare_key[0];
	send_buff[3]=mifare_key[1];
	send_buff[4]=mifare_key[2];
	send_buff[5]=mifare_key[3];
	send_buff[6]=mifare_key[4];
	send_buff[7]=mifare_key[5];
	send_buff[8]=card_uid[0];
	send_buff[9]=card_uid[1];
	send_buff[10]=card_uid[2];
	send_buff[11]=card_uid[3];

	result =PcdComCmd(PCD_AUTHENT,send_buff,12,rece_buff,&rece_bitlen);//Authent��֤
	if (result==0)
	{
		if(ReadRawRC(Status2Reg)&0x08)//�жϼ��ܱ�־λ��ȷ����֤���
			return 0;
		else
			return 1;
	}
	return 1;
}
/*****************************************************************************************/
/*���ƣ�Mifare_Blockset									 */
/*���ܣ�Mifare_Blockset��Ƭ��ֵ����							 */
/*���룺block����ţ�*buff����Ҫ���õ�4�ֽ���ֵ����					 */
/*											 */
/*���:											 */
/*		OK    :���óɹ�								 */
/*		ERROR :����ʧ��								 */
/*****************************************************************************************/
 unsigned char Mifare_Blockset(unsigned char block,unsigned char *buff)
 {
  unsigned char data block_data[16],result;
	block_data[0]=buff[3];
	block_data[1]=buff[2];
	block_data[2]=buff[1];
	block_data[3]=buff[0];
	block_data[4]=~buff[3];
	block_data[5]=~buff[2];
	block_data[6]=~buff[1];
	block_data[7]=~buff[0];
   	block_data[8]=buff[3];
	block_data[9]=buff[2];
	block_data[10]=buff[1];
	block_data[11]=buff[0];
	block_data[12]=block;
	block_data[13]=~block;
	block_data[14]=block;
	block_data[15]=~block;
  result= Mifare_Blockwrite(block,block_data);
  return result;
 }
/*****************************************************************************************/
/*���ƣ�Mifare_Blockread																 */
/*���ܣ�Mifare_Blockread��Ƭ�������													 */
/*���룺block����ţ�0x00~0x3F����buff��16�ֽڶ�����������								 */
/*���:																					 */
/*		OK    :�ɹ�																		 */
/*		ERROR :ʧ��																		 */
/*****************************************************************************************/
unsigned char Mifare_Blockread(unsigned char block,unsigned char *buff)
{	
	unsigned char data send_buff[2],result;
	unsigned int data rece_bitlen;
	WriteRawRC(TxModeReg,0x80);
	WriteRawRC(RxModeReg,0x80);
    WriteRawRC(BitFramingReg,0x00);
//	Pcd_SetTimer(1);
	send_buff[0]=0x30;//30 ����ָ��
	send_buff[1]=block;//���ַ
	result =PcdComCmd(PCD_TRANSCEIVE,send_buff,2,buff,&rece_bitlen);//
	if ((result!=0 )|(rece_bitlen!=16*8)) //���յ������ݳ���Ϊ16
		return 1;
	return 0;
}
/*****************************************************************************************/
/*���ƣ�mifare_blockwrite																 */
/*���ܣ�Mifare��Ƭд�����																 */
/*���룺block����ţ�0x00~0x3F����buff��16�ֽ�д����������								 */
/*���:																					 */
/*		OK    :�ɹ�																		 */
/*		ERROR :ʧ��																		 */
/*****************************************************************************************/
unsigned char Mifare_Blockwrite(unsigned char block,unsigned char *buff)
{	
	unsigned char data result,send_buff[16],rece_buff[1];
	unsigned int data rece_bitlen;
//	Pcd_SetTimer(5);
	send_buff[0]=0xA0;//A0 д��ָ��
	send_buff[1]=block;//���ַ
  
  	result =PcdComCmd(PCD_TRANSCEIVE,send_buff,2,rece_buff,&rece_bitlen);//
	if ((result!=0)|((rece_buff[0]&0x0F)!=0x0A))	//���δ���յ�0x0A����ʾ��ACK
	return(1);
	
//	Pcd_SetTimer(10);

	result =PcdComCmd(PCD_TRANSCEIVE,buff,16,rece_buff,&rece_bitlen);//
	if ((result!=0 )|((rece_buff[0]&0x0F)!=0x0A)) //���δ���յ�0x0A����ʾ��ACK
		return 1;
	return 0;
}
/*****************************************************************************************/
/*���ƣ�																				 */
/*���ܣ�Mifare ��Ƭ��ֵ����																 */
/*���룺block����ţ�0x00~0x3F����buff��4�ֽ���ֵ��������								 */
/*���:																					 */
/*		OK    :�ɹ�																		 */
/*		ERROR :ʧ��																		 */
/*****************************************************************************************/
unsigned char Mifare_Blockinc(unsigned char block,unsigned char *buff)
{	
	unsigned char data result,send_buff[2],rece_buff[1];
	unsigned int data rece_bitlen;
//	Pcd_SetTimer(5);
	send_buff[0]=0xc1;// C1 ��ֵָ��
	send_buff[1]=block;//���ַ
	result=PcdComCmd(PCD_TRANSCEIVE,send_buff,2,rece_buff,&rece_bitlen);
	if ((result!=0 )|((rece_buff[0]&0x0F)!=0x0A))	//���δ���յ�0x0A����ʾ��ACK
		return 1;
//	Pcd_SetTimer(5);
	PcdComCmd(PCD_TRANSCEIVE,buff,4,rece_buff,&rece_bitlen);

	return result;
}
/*****************************************************************************************/
/*���ƣ�mifare_blockdec																	 */
/*���ܣ�Mifare ��Ƭ��ֵ����																 */
/*���룺block����ţ�0x00~0x3F����buff��4�ֽڼ�ֵ��������								 */
/*���:																					 */
/*		OK    :�ɹ�																		 */
/*		ERROR :ʧ��																		 */
/*****************************************************************************************/
unsigned char Mifare_Blockdec(unsigned char block,unsigned char *buff)
{	
	unsigned char data result,send_buff[2],rece_buff[1];
	unsigned int data rece_bitlen;
//	Pcd_SetTimer(5);
	send_buff[0]=0xc0;//
	send_buff[1]=block;//���ַ
	result=PcdComCmd(PCD_TRANSCEIVE,send_buff,2,rece_buff,&rece_bitlen);
	if ((result!=0 )|((rece_buff[0]&0x0F)!=0x0A))	//���δ���յ�0x0A����ʾ��ACK
		return 1;
//	Pcd_SetTimer(5);
	PcdComCmd(PCD_TRANSCEIVE,buff,4,rece_buff,&rece_bitlen);

	return result;
}
/*****************************************************************************************/
/*���ƣ�mifare_transfer																	 */
/*���ܣ�Mifare ��Ƭtransfer����															 */
/*���룺block����ţ�0x00~0x3F��														 */
/*���:																					 */
/*		OK    :�ɹ�																		 */
/*		ERROR :ʧ��																		 */
/*****************************************************************************************/
unsigned char Mifare_Transfer(unsigned char block)
{	
	unsigned char data result,send_buff[2],rece_buff[1];
	unsigned int data rece_bitlen;
//	Pcd_SetTimer(5);
	send_buff[0]=0xb0;//
	send_buff[1]=block;//���ַ
	result=PcdComCmd(PCD_TRANSCEIVE,send_buff,2,rece_buff,&rece_bitlen);
	if ((result!=0 )|((rece_buff[0]&0x0F)!=0x0A))	//���δ���յ�0x0A����ʾ��ACK
		return 1;
	return result;
}
/*****************************************************************************************/
/*���ƣ�mifare_restore																	 */
/*���ܣ�Mifare ��Ƭrestore����															 */
/*���룺block����ţ�0x00~0x3F��														 */
/*���:																					 */
/*		OK    :�ɹ�																		 */
/*		ERROR :ʧ��																		 */
/*****************************************************************************************/

unsigned char Mifare_Restore(unsigned char block)
{	
	unsigned char data result,send_buff[4],rece_buff[1];
	unsigned int data rece_bitlen;
//	Pcd_SetTimer(5);
	send_buff[0]=0xc2;//
	send_buff[1]=block;//���ַ
	result=PcdComCmd(PCD_TRANSCEIVE,send_buff,2,rece_buff,&rece_bitlen);
	if ((result!=0 )|((rece_buff[0]&0x0F)!=0x0A))	//���δ���յ�0x0A����ʾ��ACK
		return 1;
//	Pcd_SetTimer(5);
	send_buff[0]=0x00;
	send_buff[1]=0x00;
	send_buff[2]=0x00;
	send_buff[3]=0x00;
	PcdComCmd(PCD_TRANSCEIVE,send_buff,4,rece_buff,&rece_bitlen);
	return 1;
}