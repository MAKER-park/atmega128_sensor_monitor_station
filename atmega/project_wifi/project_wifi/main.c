/*
 * project_wifi.c
 *
 * Created: 2022-07-27 오후 7:02:29
 * Author : KCCI22
 */ 
#define F_CPU 16000000UL // 16 MHz
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
//
volatile unsigned char rx_cnt=0,rx_buf[15],temp_buf[15],hum_buf[15],pm_buf[15];
volatile char AT_C=0;

int wait_flag = 0, dot_count=0,get_count = 0,f_count= 0;
//
void TX0_CHAR(char ch){ while(!(UCSR0A&0x20)); UDR0=ch; }
void TX0_STR(char *s){ while(*s) TX0_CHAR(*s++); }
const char str[][100] PROGMEM = {//1 0 3 4 5
	"AT\r\n", //AT TEST커멘드. - 0
	"AT+RST\r\n", // 리셋 - 1
	"AT+GMR\r\n", // 버전확인 - 2
	"AT+CWMODE=1\r\n", //WIFI 모드 설정1,2,3 (Station/softAP/station+softAP)- 3
	"AT+CIPMUX=0\r\n", //1->복수 연결상태  0->싱글 연결상태 -4
	"AT+CIPSERVER=1,1111\r\n", //ESP8266 TCP서버모드 구동
	//첫번째 0-> 서버 삭제 1-> 서버 생성
	//두번째 포트번호 설정 초기값은 333
	"AT+CWSAP=\"TEST_V1.5\",\"0000\",3,0\r\n",// softAP  모드 설정
	//이 명령은 softAP    모드가 활성화 된 상태에서만 응답합니다.
	//설정 후에는 이 명령의 실행을 위해서 반드시AT+RST 로 재시작 명령을 실행 해야 합니다.
	//첫번째 모듈의 이름설정
	//두번째 비밀번호 설정
	"AT+CWJAP=\"iot6\",\"iot60000\"\r\n",//wifi connection - 7
	"AT+CIFSR\r\n",//check wifi ip - 8
	//connect webpage - 9
	//"AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n"
	"AT+CIPSTART=\"TCP\",\"cloud.park-cloud.co19.kr\",80\r\n",
	//send how many byte -10
	"AT+CIPSEND=67\r\n",
	//connect path - 11
	//"GET /update?api_key=%s&field%d=%u\r\n"
	"GET /project/view2.php HTTP/1.0\r\nHost: cloud.park-cloud.co19.kr\r\n\r\n",
 };
 
 void TXDATA(const char SEND_AT[])
 {
	 char str1[100];
	 strcpy_P(str1, SEND_AT);
	 TX0_STR(str1);
	 _delay_ms(10);
 }
 
 
 ISR(USART0_RX_vect){ // get respon data!!!
	 char rx;//one letter
	 rx=UDR0;
	 if(rx=='\n'){ //'\n'=0x0A
		 rx_buf[rx_cnt]='\0';
		 if(strcmp(rx_buf, "OK\r") == 0){//if get respone data ok...
			 wait_flag = 0;
			 PORTA = 0x00;
		 }
		 if(strcmp(rx_buf, "IP\r") == 0){//if get respone data ok...
			 wait_flag = 0;
			 PORTA = 0x0F;
			 }
			 if(strcmp(rx_buf, "CLOSED\r") == 0){//if get respone html page data				 
				 wait_flag = 0;
				 PORTA = 0x0F;				 
			 }			 
			 rx_cnt=0;
			 }
			 else{
				 if(rx_cnt<9)rx_buf[rx_cnt++]=rx;
				 }
			if(rx == ','){
				dot_count++;
				get_count = 0; //init
			}if(2<dot_count && dot_count<7){
				if(dot_count == 3 && get_count <15){
					temp_buf[get_count] = rx;
					get_count++;
				}
				if(dot_count == 4&& get_count <15){
					hum_buf[get_count] = rx;
					get_count++;
				}if(dot_count == 5&& get_count <15){
					pm_buf[get_count] = rx;
					get_count++;
				}if(dot_count == 6){
					dot_count=0;
					f_count = 0;
					char *ptr = strtok(temp_buf,"\"");
					while(ptr != NULL){
						f_count++;
						//TX1_STR(ptr);
						ptr = strtok(NULL,"\"");
						if(f_count == 3){
							//TX1_STR(ptr);
							TX1_STR("n0.val=");
							TX1_STR(ptr);
							for(int index = 0 ; index < 3; index++){
								UART1_tX(0xFF);
							}
						}
					}
					_delay_ms(100);
					
					f_count = 0;
					
					ptr = strtok(hum_buf,"\"");
					while(ptr != NULL){
						f_count++;
						//TX1_STR(ptr);
						ptr = strtok(NULL,"\"");
						if(f_count == 3){
							//TX1_STR(ptr);
							TX1_STR("n1.val=");
							TX1_STR(ptr);
							for(int index = 0 ; index < 3; index++){
								UART1_tX(0xFF);
							}
						}
					}
					_delay_ms(100);
					//TX1_STR(hum_buf);
					//_delay_ms(100);
					
					f_count = 0;
					ptr = strtok(pm_buf,"\"");
					while(ptr != NULL){
						f_count++;
						//TX1_STR(ptr);
						ptr = strtok(NULL,"\"");
						if(f_count == 3){
							//TX1_STR(ptr);
							TX1_STR("n2.val=");
							TX1_STR(ptr);
							for(int index = 0 ; index < 3; index++){
								UART1_tX(0xFF);
							}
						}
					}
					_delay_ms(100);
					//TX1_STR(pm_buf);
				}	
			}
				
			}

void UART1_tX(char data){
	//if you want send data
	while(!(UCSR1A & (1<<UDRE1)));//empty_status
	UDR1 = data;
}
void TX1_CHAR(char ch){ while(!(UCSR1A&0x20)); UDR1=ch; }
void TX1_STR(char *s){ while(*s) TX1_CHAR(*s++); }

/*
uint8_t UART1_rx(void){
	//if you got a data
	while(!(UCSR1A & (1<<RXC1))){}
	return UDR1;
}*/

int main(void)
{
    /* Replace with your application code */
	//check status
	DDRA = 0xFF;
	PORTA = 0xFF;
	//for uart0 setting
	DDRE = 0xFE;
	UCSR0A = 0x00;
	UCSR0B |= (1<<RXCIE0)|(1<<TXEN0)|(1<<RXEN0);//recvie send enable
	UBRR0H = 0x00;
	UBRR0L = 103;// 9600 BAUD 
	UCSR0B = 0x98;
	
	//for uart1 setting
	DDRD = 0x06;
	UCSR1A = 0x00;
	UCSR1B |= (1<<TXEN0)|(1<<RXEN0);//recvie send enable
	UBRR1H = 0x00;
	UBRR1L = 103;// 9600 BAUD
	
	sei();
	char data[80];//send command
	
	//connection
	PORTA = 0xFF;
	wait_flag = 1;
	TXDATA(str[1]);//reset 
	while(wait_flag==1);
	_delay_ms(500);
		
	PORTA = 0xFF;
	wait_flag = 1;
	TXDATA(str[0]);//check AT
	while(wait_flag==1);
	_delay_ms(100);
	
	PORTA = 0xFF;
	wait_flag = 1;
	TXDATA(str[3]);//mode = 1
	while(wait_flag==1);
	_delay_ms(100);
	
	PORTA = 0xF0;
	wait_flag = 1;
	TXDATA(str[7]);//wifi connect
	while(wait_flag==1);
	_delay_ms(100);
	
	PORTA = 0xFF;
	wait_flag = 1;
	TXDATA(str[4]);//singel = 1
	while(wait_flag==1);
	_delay_ms(100);
	
    while (1) 
    {
		//connect to website - 9
		PORTA = 0xFF;
		wait_flag = 1;
		TXDATA(str[9]);//singel = 1
		while(wait_flag==1);
		dot_count=0;
		_delay_ms(100);
		
		//send lenth
		PORTA = 0xFF;
		wait_flag = 1;
		TXDATA(str[10]);//set byte = 1
		while(wait_flag==1);
		_delay_ms(1);
		//connect to path
		PORTA = 0xFF;
		wait_flag = 1;
		TXDATA(str[11]);//set byte = 1
		while(wait_flag==1);
		_delay_ms(100);
		
	}
}


