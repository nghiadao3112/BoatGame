	/****************************************************************************
	 * @file     main.c
	 * @version  V3.00
	 * $Revision: 10 $
	 * $Date: 15/06/04 1:56p $
	 * @brief
	 *           Transmit and receive data in UART RS485 mode.
	 *           This sample code needs to work with UART_RS485_Master.
	 * @note
	 * Copyright (C) 2011 Nuvoton Technology Corp. All rights reserved.
	 *
	 ******************************************************************************/
	#include <stdio.h>
	#include "NUC131.h"
	#include <string.h>


	#define PLL_CLOCK   50000000


	#define IS_USE_RS485NMM   0      //1:Select NMM_Mode , 0:Select AAD_Mode
	#define MATCH_ADDRSS1     0xC0
	#define MATCH_ADDRSS2     0xA2
	#define UNMATCH_ADDRSS1   0xB1
	#define UNMATCH_ADDRSS2   0xD3


	/*---------------------------------------------------------------------------------------------------------*/
	/* Define functions prototype                                                                              */
	/*---------------------------------------------------------------------------------------------------------*/
	extern char GetChar(void);
	void RS485_HANDLE(void);
	void A351_HANDLE(void);
	void RS485_9bitModeSlave(void);
	void RS485_FunctionTest(void);

	volatile int32_t g_bWait = TRUE;
	volatile int32_t g_OK = TRUE;
	volatile int32_t g_check=TRUE;
	volatile int32_t g_check_bi_ban=TRUE;
	volatile int checktime=0;
	volatile int state=0;
	char* ok ="OK\r\n";
	char* check="trung\n";
	char* check_bi_ban="biban\n";
	int len_ok = 4;
	int c_ok =0;
	int c_check=0;
	int c_check_biban=0;
	int i = 0;
	char* send_string;
	int len = 0;
	char* setcookie ="set-cookie: ";
	int len_setcookie = 12;
	int c_setcookie = 0;
	char cookie[150];
	int len_cookie = 0;
	int count1;
	int32_t check_setcookie = FALSE;
	/*---------------------------------------------------------------------------------------------------------*/
	/* ISR to handle UART Channel 1 interrupt event                                                            */
	/*---------------------------------------------------------------------------------------------------------*/
	void PWM0_IRQHandler(void)
	{
			static uint32_t cnt;
			static uint32_t out;
			
			// Channel 0 frequency is 100Hz, every 1 second enter this IRQ handler 100 times.
			if(++cnt == 100)
			{
							if(out)PB13=1;
				
					else PB13=0;
						 
					out ^= 1;
					cnt = 0;
					checktime++;
			}
			if (checktime==state){
				PWM_Stop(PWM0, PWM_CH_0_MASK );
			}
			// Clear channel 0 period interrupt flag
			PWM_ClearPeriodIntFlag(PWM0, 0);
	}
	
	void UART3_IRQHandler(void)
	{
			A351_HANDLE();
	}
	void UART02_IRQHandler(void)
	{
		RS485_HANDLE();
	}




	void A351_HANDLE()
	{
		char u8InChar = 0xFF;
		uint32_t u32IntSts = UART3->ISR;
			
		if(u32IntSts & UART_ISR_RDA_INT_Msk)
		{

			/* Get all the input characters */
			while(UART_IS_RX_READY(UART3))
			{
				/* Get the character from UART Buffer */
				u8InChar = UART_READ(UART3);

				//send_string[j++]=u8InChar;
							printf("%c",u8InChar);
							if(g_OK)
							{
								if(u8InChar == ok[c_ok])
								{
									c_ok++;
									if(c_ok == len_ok)
									{
										g_OK = FALSE;
									}
								}
								else
										c_ok=0;
							}
							if(g_check==TRUE){
								if (u8InChar==check[c_check]){
									c_check++;
									if(c_check==6){
										g_check=FALSE;
									}
								}
								else 
									c_check=0;
							}
							if(g_check_bi_ban==TRUE){
								if (u8InChar==check_bi_ban[c_check_biban]){
									c_check_biban++;
									if(c_check_biban==6){
										g_check_bi_ban=FALSE;
									}
								}
								else 
									c_check_biban=0;
							}
							
							
				

				/* Check if buffer full */
				
			}
			
		}

		if(u32IntSts & UART_ISR_THRE_INT_Msk)
		{
			
				if(i>=len-1)
							{
								g_bWait = FALSE;
								UART_DISABLE_INT(UART3, 0x00000002); 
							}
				while(UART_IS_TX_FULL(UART3));  // Wait Tx is not full to transmit data                           
				//UART_WRITE(UART3, send_string[i++]);
							UART3->THR = send_string[i++];
			   
		 }
	}
	void sendATcmd(char* ATcmd)
	{
		i = 0;
		send_string = ATcmd;
		len = strlen(ATcmd);
		g_bWait = TRUE;
		UART_EnableInt(UART3, UART_IER_THRE_IEN_Msk);
		while (g_bWait);
		
	}
	void RS485_HANDLE()
	{
		
		char u8InChar = 0xFF;
		uint32_t u32IntSts = UART0->ISR;

		if(u32IntSts & UART_ISR_RDA_INT_Msk)
		{

			/* Get all the input characters */
			while(UART_IS_RX_READY(UART0))
			{
				/* Get the character from UART Buffer */
				u8InChar = UART_READ(UART0);

				//send_string[j++]=u8InChar;
							printf("%c",u8InChar);
				

				/* Check if buffer full */
				
			}
			
		}

	 }









	void SYS_Init(void)
	{
		/*---------------------------------------------------------------------------------------------------------*/
		/* Init System Clock                                                                                       */
		/*---------------------------------------------------------------------------------------------------------*/

		/* Enable Internal RC 22.1184MHz clock */
		CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);

		/* Waiting for Internal RC clock ready */
		CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

		/* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
		CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

		/* Enable external XTAL 12MHz clock */
		CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);

		/* Waiting for external XTAL clock ready */
		CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);

		/* Set core clock as PLL_CLOCK from PLL */
		CLK_SetCoreClock(PLL_CLOCK);
			CLK_SetSysTickClockSrc(CLK_CLKSEL0_STCLK_S_HCLK_DIV2);

		/* Enable UART module clock */
		CLK_EnableModuleClock(UART0_MODULE);
			CLK_EnableModuleClock(UART3_MODULE);
			CLK_EnableModuleClock(TMR0_MODULE);
			CLK_EnableModuleClock(TMR1_MODULE);
			CLK_EnableModuleClock(TMR2_MODULE);
			CLK_EnableModuleClock(PWM0_MODULE);
		
		

		/* Select UART module clock source */
		CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART_S_HXT, CLK_CLKDIV_UART(1));
			CLK_SetModuleClock(UART3_MODULE, CLK_CLKSEL1_UART_S_HXT, CLK_CLKDIV_UART(1));
			CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0_S_HCLK, 0);
			CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1_S_HXT, 0);
			CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR2_S_HXT, 0);
			CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL3_PWM0_S_PLL, 0);
		
		SYS_ResetModule(PWM0_RST);
		/*---------------------------------------------------------------------------------------------------------*/
		/* Init I/O Multi-function                                                                                 */
		/*---------------------------------------------------------------------------------------------------------*/

		/* Set GPB multi-function pins for UART0 RXD(PB.0) and TXD(PB.1) */
		/* Set GPB multi-function pins for UART1 RXD(PB.4), TXD(PB.5), nRTS(PB.6) and nCTS(PB.7) */

			SYS->GPA_MFP &= ~(SYS_GPA_MFP_PA2_Msk | SYS_GPA_MFP_PA3_Msk);
		SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB0_Msk | SYS_GPB_MFP_PB1_Msk);

		SYS->GPB_MFP |= (SYS_GPB_MFP_PB0_UART0_RXD | SYS_GPB_MFP_PB1_UART0_TXD
						  );
			SYS->GPA_MFP |= (
											SYS_GPA_MFP_PA3_UART3_RXD | SYS_GPA_MFP_PA2_UART3_TXD
						  );
			SYS->ALT_MFP4|=(SYS_ALT_MFP4_PA3_UART3_RXD| SYS_ALT_MFP4_PA2_UART3_TXD);
	}

	unsigned char KEYS_PAD[4][4] ={'D', '#' , '0' , '*',
								   'C' ,'9' , '8', '7',
								   'B' ,'6' ,'5', '4',
								   'A' , '1', '2' , '3' };
	void quetphim_Init(volatile uint32_t ** output,volatile uint32_t ** input)
	{
			GPIO_SetMode(PA, BIT10, GPIO_PMD_INPUT);
			GPIO_SetMode(PA, BIT11, GPIO_PMD_INPUT);
			GPIO_SetMode(PA, BIT15, GPIO_PMD_INPUT);
			GPIO_SetMode(PE, BIT5, GPIO_PMD_INPUT);
		
			GPIO_SetMode(PB, BIT11, GPIO_PMD_OUTPUT);
			GPIO_SetMode(PA, BIT14, GPIO_PMD_OUTPUT);
			GPIO_SetMode(PA, BIT13, GPIO_PMD_OUTPUT);
			GPIO_SetMode(PA, BIT12, GPIO_PMD_OUTPUT);
			
			GPIO_SetMode(PB, BIT13, GPIO_PMD_OUTPUT);
			PB13=0;
		
			PB11 = 0;
			PA14 = 0;
			PA13 = 0;
			PA12 = 0;
		
			input[0] = &PA10;
			input[1] = &PA11;
			input[2]=	&PA15;
			input[3]=	&PE5;
		
			output[0] = &PB11;
			output[1] = &PA14;
			output[2]=	&PA13;
			output[3]=	&PA12;
	}
	unsigned char quetphim(volatile uint32_t ** output,volatile uint32_t ** input)
	{
			PA10 = 1;
			PA11 = 1;
			PA15 = 1;
			PE5 = 1;
		
			PB11 = 0;
			PA14 = 0;
			PA13 = 0;
			PA12 = 0;		
			TIMER_Delay(TIMER0, 2);
			if(*input[0] == 0||*input[1] == 0||*input[2] == 0||*input[3] == 0)
			{
				  TIMER_Delay(TIMER0, 1000);
					if(*input[0] == 0||*input[1] == 0||*input[2] == 0||*input[3] == 0)
				{
					for(int i = 0;i<4;i++)
					{
						for(int c = 0; c<4;c++)
						{
							*output[c] = 1;
							if(i==c)
								*output[c] = 0;
						}
						TIMER_Delay(TIMER0, 1000);
						for(int j = 0;j<4;j++)
						{
							if(*input[j]==0)
							{
								TIMER_SET_CMP_VALUE(TIMER1, 0xFFFFFF);
								while(*input[j]==0);
								int u32DelayTime = TIMER_GetCounter(TIMER1) / 1000;
								if(u32DelayTime < 100)
									return 'a';
								return KEYS_PAD[i][j];
							}
						}
					}
				}
			}
			return 'a';
	}

	void UART0_Init()
	{
		/*---------------------------------------------------------------------------------------------------------*/
		/* Init UART                                                                                               */
		/*---------------------------------------------------------------------------------------------------------*/
		/* Reset UART0 module */
		SYS_ResetModule(UART0_RST);

		/* Configure UART0 and set UART0 Baudrate */
		UART_Open(UART0, 9600);
	}
	void UART3_Init()
	{
		/*---------------------------------------------------------------------------------------------------------*/
		/* Init UART                                                                                               */
		/*---------------------------------------------------------------------------------------------------------*/
		/* Reset UART0 module */
		SYS_ResetModule(UART3_RST);

		/* Configure UART0 and set UART0 Baudrate */
		UART_Open(UART3, 9600);
	}

	/*---------------------------------------------------------------------------------------------------------*/
	/* MAIN function                                                                                           */
	/*---------------------------------------------------------------------------------------------------------*/

void setupPWM(){
		PWM_ENABLE_COMPLEMENTARY_MODE(PWM0);
    // PWM0 channel 0 frequency is 100Hz, duty 30%,
    PWM_ConfigOutputChannel(PWM0, 0, 100, 30);
		PWM_EnablePeriodInt(PWM0, 0, 0);
    NVIC_EnableIRQ(PWM0_IRQn);
}	
	uint32_t count = 0;
int32_t main(void)
	{

		/* Unlock protected registers */
		SYS_UnlockReg();

		/* Init System, peripheral clock and multi-function I/O */
		SYS_Init();

		/* Lock protected registers */
		SYS_LockReg();

		/* Init UART0 for printf */
		UART0_Init();
			UART3_Init();
		/* Init UART1 for testing */
		

		/*---------------------------------------------------------------------------------------------------------*/
		/* SAMPLE CODE                                                                                             */
		/*---------------------------------------------------------------------------------------------------------*/

	 
			
		//UART_ENABLE_INT(UART0, (UART_IER_RDA_IEN_Msk | UART_IER_TOUT_IEN_Msk));

		
		//NVIC_EnableIRQ(UART02_IRQn);
			//UART0->FCR &=~0x00000100;
			//printf("AT+RST\r\n");
		//TIMER_Delay(TIMER0, 100000);
			volatile uint32_t **output=(volatile uint32_t **) malloc(4*sizeof(uint32_t));
			volatile uint32_t **input= (volatile uint32_t **) malloc(4*sizeof(uint32_t));
		quetphim_Init(output,input);
		setupPWM();
			char key;
			TIMER1->TCSR = TIMER_PERIODIC_MODE | (12 - 1);
		TIMER_SET_CMP_VALUE(TIMER1, 0xFFFFFF);
		TIMER_Start(TIMER1);
			TIMER2->TCSR = TIMER_PERIODIC_MODE | (12 - 1);
		TIMER_SET_CMP_VALUE(TIMER2, 0xFFFFFF);
		TIMER_Start(TIMER2);
			g_OK = FALSE;
			UART_ENABLE_INT(UART3, (UART_IER_RDA_IEN_Msk | UART_IER_TOUT_IEN_Msk));
			NVIC_EnableIRQ(UART3_IRQn);
			UART3->FCR &=~0x00000100;
			
			sendATcmd("AT+RST\r\n");
			count=0;
			while(count<5)
			{
				TIMER_Delay(TIMER0, 1000000);
				count++;
			}
			g_OK = TRUE;
			sendATcmd("AT+CWMODE=1\r\n");
			
			while(g_OK);
			g_OK = TRUE;
			sendATcmd("AT+CIPMUX=0\r\n");
			while(g_OK);
			g_OK = TRUE;
			sendATcmd("AT+CWJAP=\"aaaaaaaa\",\"12345678\"\r\n");
			while(g_OK);
			g_OK = TRUE;
			
			
			sendATcmd("AT+CIPSTART=\"TCP\",\"3.0.101.80\",5555\r\n");
			while(g_OK);
			g_OK = TRUE;
			TIMER_SET_CMP_VALUE(TIMER2, 0xFFFFFF);
			while(1)
			{
				if (g_check_bi_ban==FALSE){
					checktime=0;
					state=3;
					PWM_Start(PWM0, PWM_CH_0_MASK);
					while (checktime!=state);
					PB13=0;
					g_check_bi_ban=TRUE;
					
				}
				else if (g_check==FALSE){
					checktime=0;
					state=5;
					PWM_Start(PWM0, PWM_CH_0_MASK);
					while (checktime!=state);
					PB13=0;
					g_check=TRUE;
				}
				key = quetphim(output,input);
				if(key == '2'||key == '4'||key == '6'||key == '8'||key=='5')
				{
					TIMER_SET_CMP_VALUE(TIMER2, 0xFFFFFF);
					if(key == '2')
						key ='2';
					if(key == '4')
						key ='4';
					if(key == '6')
						key ='6';
					if(key == '8')
						key ='8';
					if(key == '5')
						key ='5';
					sendATcmd("AT+CIPSEND=");
					sendATcmd("1\r\n");
					while(g_OK);
					g_OK = TRUE;
					sendATcmd(&key);
					while(g_OK);
					g_OK = TRUE;
				}
			}
	}