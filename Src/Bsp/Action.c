

#include "Action.h"
#include "math.h"
#include "EEPROM.h"
#include "Encoder.h"
#include "PutOffAct.h"
#include "u_log.h"
struct STADATA sys_stadata;						// ÏµÍ³²ÎÊý

struct SYSATTR g_sys_para;

struct LIHEDATA	    TTlehe;							//Ê¹ÓÃÌ½Í·µÄÀëºÏ¼ÆÊýÄ£Ê½,ÐÞ¸ÄÀëºÏµãºó£¬±£´æËùÐè²ÎÊý
struct LIHEDATA	    TMlehe;		    				// TMÀëºÏ¶¯×÷£¬±£´æËùÐè²ÎÊý

struct PRPTECT_HANGTU Prtop;						//ÀëºÏµ½¶¥µÄÊý¾Ý¼ÇÂ¼  Terry 2019.7.24

volatile uint32_t g_halt = 0;

volatile uint8_t sys_fbsta;						// Íâ²¿·´À¡ÐÅºÅ

//extern uint32_t savecnt;						//±£´æÊý¾Ý
int8_t Get_Fbsignal(uint8_t MASK);
SYS_STA Get_Action_Sta(void);
extern int8_t Get_Fbsignal(uint8_t MASK);
extern struct RECORD s_record;

uint32_t getmilsec(uint32_t pretime)
{
	uint32_t milsec;	
	milsec = osKernelSysTick();
	if(pretime > milsec)
		milsec = (0xffffffff - pretime) + milsec;  			/*³¬¹ýÒ»ÌìµÄ¿ç¶È*/
	else
		milsec = milsec - pretime;			
	return 	milsec;
}


/*»ñµÃ±àÂëÆ÷¼ÆÊý£¬È·¶¨»ù×¼·½Ïò*/
/**
  * º¯Êý¹¦ÄÜ »ñµÃ±àÂëÆ÷¼ÆÊý£¬È·¶¨»ù×¼·½Ïò
  * ÊäÈë²ÎÊý£ºdir:»ù×¼·½Ïò
  *           *p:Ó²¼þ±àÂëÆ÷¼ÆÊýÊý¾Ý
  * ·µ»ØÖµ: None
  * ËµÃ÷: ÎÞ
  */
  
/*  ÀåÃ××ª³ÝÊý  */
int32_t cm2num(int32_t cm)
{
	int32_t tmp;
	
	tmp = cm * g_sys_para.s_numchi / g_sys_para.s_zhou;
	
	return tmp;
}



/********************************************************
Function	: PICtr()
Description	: ¶ÔËÉ³Ú¶ÈµÄÎó²î½øÐÐPI¿ØÖÆ£¬³ÝÊý
Input		: sub	¹ýËÉÎªÕý£¬¹ý½ôÎª¸ºÊý
Return		: ÀëºÏµãµÄÐÞÕýÁ¿£¬µ¥Î» ³ÝÊý£¬·µ»ØÀëºÏµãµ÷ÕûµÄÔöÁ¿
Others		: ÊÖ¶¯ÉèÖÃÀëºÏµãÊ±£¬ÔÝÍ£Ò»¶ÎÊ±¼ä²»½øÐÐ×Ô¶¯µ÷Õû
5¸ö³ÝÒÔÄÚµÄËÉ³Ú¶ÈÎó²î£¬ÔÊÐí
*********************************************************/
int32_t PICtr(int32_t subs)
{
	static float Totals = 0;
	
	float ret;
	int32_t tmp;
	
	if(subs > 5)
		Totals += (subs - 5);					//¶ÔÎó²î½øÐÐ»ý·Ö
	else if(subs < 0)
		Totals += subs;
	else										/*ÔÚ 0 µ½ 5 µÄÇø¼äÄÚ*/
	{
		if(Totals < 0)							/*ÔöÇ¿ÎÈ¶¨ÐÔ*/
			Totals += subs;
		else if(Totals > 20)
			Totals -= (5 - subs);	
	}
	
	
	if(subs > 5)								//±È½ÏËÉµÄÊ±ºò
	{
		ret = 0.03 * (subs - 5);					
	}
	else if(subs < 0)
	{
		ret = 0.03 * subs;
	}
	else
		ret = 0;
	
	ret = 0.05 * Totals + ret;					//Ö÷ÒªÊÇ¿¿ÀÛ¼ÆÎó²îÈ¥µ÷Õû
	
	tmp = (int32_t)ret;
	
	if(tmp != 0)
	{
		Totals = Totals / 2;
	}
	Debug("PI %d  ",tmp);
	return tmp;
}


int32_t PITCtr(int32_t subs)
{
	static int32_t last_err = 0;
	
	float ftmp;
	int32_t ret;
	
	ftmp = 0.04*((subs - last_err) + 0.1 * subs);
	if(ftmp > 20)
		ftmp = 20;
	else if(ftmp < -20)
		ftmp = -20;
	
	ftmp = ftmp / 3;
	
	if(ftmp > 12)
		ftmp = 12;
	else if(ftmp < -12)
		ftmp = -12;
	
	last_err = subs;
	
	ret = -ftmp;
	
	return ret;
}

/*»ñµÃµ±Ç°¹¦ÂÊ*/
void GetPower(struct EtrCnt *p)
{
	static uint32_t lastspeed = 0;
	
	p->Speed = sys_Etrcnt.Speed + lastspeed;				//Ë«±¶ËÙ¶È
	lastspeed = sys_Etrcnt.Speed;
}


/*¼ÆËã³öÓÐ¹¦¹¦ÂÊ±ÈÖµ¶ÔÓ¦µÄÊµ¼Ê¹¦ÂÊ*/
int32_t epower(int16_t  ratio)
{
	uint32_t pw;
	
	pw = (g_sys_para.s_pfull - g_sys_para.s_pnull) * ratio / 100;
	pw += g_sys_para.s_pnull;
	
	return pw;
}







/*
¿¨´¸Ê±
ÖÐ¶ÏÉÏÀ­
À­É²³µ ËÉÀëºÏ

À­ÀëºÏ  ËÉÉ²³µ
*/
void pullbreak(void)
{
	G_SHACHE(ACT_ON,0);
	G_LIHE(ACT_OFF,LIHEDLY / 2);
	osDelay(1500);
	G_LIHE(ACT_ON,0);
	G_SHACHE(ACT_OFF,SHACHEDLY / 2);
}
/******************************************ÀëºÏµã×Ô¶¯¼ÆËã  ÓÐÌ½Í·Ä£Ê½ ***************************************/
/**
  * º¯Êý¹¦ÄÜ ·µ»ØÓÐÐ§ÁËÀëºÏµã¸ß¶È£¨³ÝÊý£©
  * ÊäÈë²ÎÊý£º relax:Êµ¼ÊËÉ³Ú¶È
  *             *cnt:ÔÊÐíµ÷ÕûµÄµ¹¼ÆÊýÖ¸Õë

  * ·µ»ØÖµ: int32_t Êµ¼ÊµÄÀëºÏµã
  * ËµÃ÷: ÎÞ
  */

int32_t getlihenum(void)
{
	static int32_t lihenum;
	
	liheupdate();
	lihenum = TTlehe.lihe;									//³õÊ¼µÄÀëºÏµã

	return lihenum;
}

/**
  * º¯Êý¹¦ÄÜ ¸üÐÂÀëºÏµãµÄ¼ÆÊýÖµ
  * ÊäÈë²ÎÊý£º ÎÞ
  * ·µ»ØÖµ: ÎÞ
  * ËµÃ÷: Ã¿´ÎÐÞ¸ÄÀëºÏµãÊýÖµºó£¬¶¼»áÖØÐÂ¼ÆËã  ÀëºÏµãµÄ×Ô¶¯µ÷ÕûÇø¼äÎª Õý¸º 20cm
  */
extern struct HANGTU s_hang;  
void liheupdate(void)
{
	int32_t Lihecmtmp = 0;
	
	TTlehe.cnt = INIT_CHUI;		//ÐÞÕýºóÄ¬ÈÏ50´ÎµÄÎÈ¶¨Ê±¼ä
	TTlehe.relaxsum = 0;		//¸ÃÖµÕý³£Îª¸ºÖµ
	TTlehe.songchi = 0;
	
	Lihecmtmp = g_sys_para.s_hlihe;
	
	if(g_sys_para.s_cmode == MOD_AUTOTAMP)
	{
		if(s_hang.dachui_cnt == g_sys_para.s_cnt)			/*µÚÒ»´Î´ò´¸,´òËÉÒ»µã  2019.9.17*/
			Lihecmtmp -= 6;
		if(s_record.deepth > -300)						/*µ½´ï¶¥²¿Ê±£¬´ò½ô´¸*/
			Lihecmtmp += 8;			
	}
	
	if(Lihecmtmp < 50)						/*×îÐ¡À­ÀëºÏµÄµãÎª50¹«·Ö  2019.12.17*/
		Lihecmtmp = 50;
	else if(Lihecmtmp > g_sys_para.s_sethighcm - 5)
		Lihecmtmp = g_sys_para.s_sethighcm - 5;
	TTlehe.lihe = cm2num(Lihecmtmp);
}


/********************************************************************************************************/

/*2018.9.1 Ôö¼ÓÌá´¸µãµÄ×´Ì¬»úÅÐ¶Ï£¬·ÀÖ¹Ìá´¸µãµÄÎó²Ù×÷ÎóÊ¶±ð*/
void Pull_Clear(int16_t *psta)
{
	static int8_t ocnt = 0;
	int32_t tmp;
	uint32_t ctime;
	
	
	ctime = osKernelSysTick();
	switch(*psta)
	{
		case PULL_IDLE:
			tmp = epower(70);
			ocnt = 0;
			
			while(sys_stadata.m_power.Speed > tmp)
			{
				if(getmilsec(ctime) > 150)	    //Á¬ÐøÓÐÐ§Ìá´¸³¬¹ý 150ms  ÈÏÎªÓÐÐ§Ìá´¸
				{
					*psta = PULL_EFFECT;
					Enc_Clr_TotalCnt1();
                    Debug("speed %d\r\n",sys_stadata.m_power.Speed);
					break;
				}
				osDelay(1);
			}
			break;
		case PULL_OBS:
			tmp = epower(80);
			Enc_Clr_TotalCnt1();
			while(sys_stadata.m_power.Speed > tmp)
			{
				if(getmilsec(ctime) > 80)	    //Á¬ÐøÓÐÐ§Ìá´¸³¬¹ý 80ms  ÈÏÎªÓÐÐ§Ìá´¸
				{
					ocnt++;
					*psta = PULL_EFFECT;
					
					break;
				}
				osDelay(1);
			}
			break;
		case PULL_EFFECT:
			tmp = epower(55);
			if(ocnt < 1)
			{
				while(sys_stadata.m_power.Speed < tmp)
				{
					if(getmilsec(ctime) > 60)	    //Á¬ÐøÓÐÐ§Ìá´¸Ð¡ÓÚ 120ms  ÈÏÎª¿Õ³Ì
					{
						*psta = PULL_OBS;
						break;
					}
					osDelay(1);
				}
				
			}
			break;
		default:*psta = PULL_IDLE;
	}
}

/*Õý³£Ìá´¸¶¯×÷*/
extern struct Locationcm user_encoder;  

SYS_STA takeup(void)
{
	SYS_STA status;
	uint32_t ctime,intime;
	int32_t loadpw,sethnum,overpw;
	int8_t kccnt = 0;
	int32_t protectT = 0;
	int32_t speedcnt = 0;
	int32_t powcnt = 0;
	int32_t starthighcm = 0;
	
	status = ERR_NONE;
	
	do
	{
		G_LIHE(ACT_ON,0);G_LIHE(ACT_ON,0);			       /*¿ØÖÆºÍÖ´ÐÐ·Ö¿ª   Á¢¼´À­ÀëºÏ*/
		G_SHACHE(ACT_OFF,SHACHEDLY);
		G_LIHE(ACT_ON,0);
		starthighcm = GetEncoderLen1Cm();	/*À­ÀëºÏµÄ¸ß¶Èµã 2019.12.19 Terry add*/
		ctime = osKernelSysTick();
		/*Á¬Ðø¼ì²éÀëºÏÊÇ·ñ¶¯×÷*/
		while(speedcnt < 3)									//´ËÊ±´¸»¹ÔÚÏÂ½µ£¬³¬Ê±4Ãë±È½ÏºÏÊÊ    Terry 2019.5.25
		{
			if(GetEncoderSpeedCm() > -10)
				speedcnt++;
			else
			{
				if(speedcnt > 0) 
					speedcnt--;
			}
			
			if(getmilsec(ctime) > 5000)						//À­ÀëºÏºó3Ãëºó£¬ÀëºÏ»¹ÊÇ·´×ª£¬Ôò±¨¹ÊÕÏ
			{				
				if(sys_stadata.m_power.Speed < (g_sys_para.s_pnull / 2))
					status |= ERR_CT;						//Ö÷»úÎ´Æô¶¯»ò»¥¸ÐÆ÷Ëð»µ
				else
					status |= ERR_TT;						//Â©µôÁËÒ»¸öÌõ¼þ£¬¸Ã´íÎóÏÂÈÏÎªÊÇÌ½Í·¹ÊÕÏ
					
				ERR_BREAK;
			}
			/*Ô¤·ÀÁïÉþµÄ´¦Àí À­ÀëºÏÐÅºÅºó£¬¼ÌÐøÏÂÐÐ6Ã× ±¨´íÎó 2019.12.17  Terry add*/
			if((starthighcm - GetEncoderLen1Cm()) > 800)		/*Èç¹ûËÉ³Ú¹ý´ó£¬Ö±½Ó±¨¾¯  À­ÀëºÏµ½×îµÍµã³¬¹ý6Ã× 2019.12.19*/
			{
				status |= ERR_LS;
				ERR_BREAK;
			}
			HALT_BREAK;
			osDelay(CALUTICK);
		}
		
		ERR_BREAK;										/*Òì³£Ö±½ÓÌø³öÑ­»·  do while*/		
		
		ctime = osKernelSysTick();
		Enc_Clr_TotalCnt1();
		loadpw = epower(70);
		while( powcnt < 4)								//µÈ´ý 80%ÓÐÐ§Ìá´¸µã  300ms
		{
			if(sys_stadata.m_power.Speed > loadpw)
				powcnt++;								/*  ³¬¹ý70%µÄÀ­Á¦,ÈÏÎªÓÐÐ§Ìá´¸*/
			else
			{
				if(powcnt > 1) 
					powcnt -= 1;
				else
					Enc_Clr_TotalCnt1();
			}
			
			//8Ãë³¬Ê±
			if(getmilsec(ctime) > 6000)						/*À­´¸±£»¤Ê±¼ä 6Ãë  Èç¹û´òµÃºÜËÉ£¬×Ô¶¯±£»¤*/
			{
				status |= ERR_CT;							// ÀëºÏ¹ýËÉ»òÏìÓ¦¹ýÂý
				break;
			}
			
			if(GetEncoderLen1Cm() < -300)		/*Èç¹ûËÉ³Ú¹ý´ó£¬Ö±½Ó±¨¾¯*/
			{
				status |= ERR_CT;
				ERR_BREAK;
			}
			
			HALT_BREAK;
			osDelay(CALUTICK);
		}
		ERR_BREAK;													/*Òì³£Ö±½ÓÌø³öÑ­»·  do while*/		
		sethnum = cm2num(g_sys_para.s_sethighcm);						//Éè¶¨¸ß¶È£¬»»Ëã³É³ÝÊý
        protectT = g_sys_para.s_sethighcm * 30 + 5000;       			//ÐÞ¸ÄBug   2018.11.9
		sys_stadata.clihe = getlihenum();  //µÃµ½ÏÂÂäÊ±£¬ÀëºÏµãµÄ¸ß¶È
		
		
		/*¼ÇÂ¼µ±Ç°µÄÉî¶ÈÎª×îµÍµã   Terry 2019.6.5*/
		s_record.deepth = GetEncoderLen2Cm();				/*µ¥Î» ÀåÃ× */
		
Debug("upnum %d\r\n",tmp);
Debug("autolihe %d",sys_stadata.clihe);

		speedcnt = 0;
		ctime = osKernelSysTick();
		while(Enc_Get_CNT1() < sethnum)				//Éè¶¨¸ß¶È  ±È½ÏµÄ³ÝÊý
		{
			/******************ÉÏÀ­³¬Ê±ÅÐ¶Ï**************************/
			if(getmilsec(ctime) > protectT)			    		//Ìá´¸Ê±¼ä³¬¹ý8Ãë£¬±¨¹ÊÕÏ
			{
				status |= ERR_LS;
                Debug("err 2");
				break;		/*Ö±½ÓÌø³ö*/
			}
			ERR_BREAK;
			HALT_BREAK;

			
			/*×Ô¶¯º»ÍÁÊ±µÄ±£»¤  2019.7.24*/
			if(g_sys_para.s_cmode == MOD_AUTOTAMP)
			{
				if(GetEncoderLen1Cm() > g_sys_para.s_hprot)	/*³¬¹ý0.5Ã×£¬´¥·¢±£»¤ ¹¼ÌÐøÀ­´¸£¬ÖØÐÂ¶¨ÒåÀëºÏµãµÄ¸ß¶È 2019.10.18 Terry*/
				{
															/*¼Ç×¡µ±Ç°µÄÌá´¸¸ß¶È */
					Prtop.flg = 1;
					Prtop.p_high = Enc_Get_CNT1();
					Prtop.p_lihe = sys_stadata.clihe *  Enc_Get_CNT1() * 11 / sethnum;  /*°´ÕÕ±ÈÀý½øÐÐÀëºÏ¿ØÖÆ*/
					Prtop.p_lihe = Prtop.p_lihe / 10;
					if(Prtop.p_lihe > sys_stadata.clihe)
						Prtop.p_lihe = sys_stadata.clihe;
					
					break;							/*Ìø³ö¼ÆÊýÑ­»· ºÜ¹Ø¼ü*/
				}
			}
			
			/**************¿¨´¸ÅÐ¶Ï***********************/
			intime = osKernelSysTick();
			overpw = epower(250);
			while(sys_stadata.m_power.Speed > overpw)
			{
				//1Ãë³¬Ê±
				if(getmilsec(intime) > 3000)				// Á¬Ðø3Ãë³¬Ê±
				{
					status |= ERR_KC;						// ¿¨´¸ÁË

					Debug("up kc\r\n");
					kccnt++;
					break;
				}
				HALT_BREAK;
				ERR_BREAK;				/*Ìø³ö±¾Ñ­»·*/
				osDelay(CALUTICK);
			}
			// pull break;  ¿¨´¸´¦Àí
			if((status & ERR_KC) && (kccnt < 3))
			{
				pullbreak();
				status &= ~ERR_KC;
				protectT += 3000;			/*³¬Ê±ÅÐ¶ÏÔö¼Ó 3Ãë */
			}
			else
			{
				/******************ÖÐÍ¾Áï´¸ÅÐ¶Ï**************************/
				if(Enc_Get_SpeedE1() < 0)
					speedcnt++;
				else
				{
					if(speedcnt > 3)  
						speedcnt-=3;
				}
				
				if(speedcnt > 800)		/*Á¬ÐøÌá3Ãë£¬Ìá²»ÆðÀ´¾Í±¨¾¯*/
				{
					status |= ERR_LS;
					Debug("Áï´¸ÁË\r\n");
				}
					
				ERR_BREAK;
			}
			osDelay(5);
			ERR_BREAK;
		}
	}while(0);
	
	Debug("high %d \r\n",sys_stadata.m_high.TotalCount);
	
	return status;
}


/*Ò»¼üÆô¶¯*/
SYS_STA starttaking(void)
{
	SYS_STA status;
	int32_t avepow;
	uint16_t i;
	int32_t sethnum,cnt;
	int32_t noloadpw;
	uint32_t ctime,intime;
	
	status = ERR_NONE;
	cnt = 0;
	/*ÇóÆ½¾ù¿ÕÔØ¹¦ÂÊ*/
	avepow = 0;
	
	IOT_FUNC_ENTRY;
	for(i = 0;i < 30;i++)
	{
		avepow += sys_stadata.m_power.Speed;
		osDelay(CALUTICK);
	}
	g_sys_para.s_pnull = avepow / 30;
    avepow = 0;
	Debug("null %d\r\n",g_sys_para.s_pnull);

	//À­ÀëºÏ 0.4ÃëºóËÉÉ²³µ
	G_LIHE(ACT_ON,0);
	G_SHACHE(ACT_OFF,30);
	Enc_Clr_TotalCnt1();
	osDelay(1000);
	//µÈ´ýÓÐÐ§À­Á¦
	do
	{
		ctime = osKernelSysTick();
		noloadpw = g_sys_para.s_pnull * 3 / 2 + 1;			//µÈ´ý1.5±¶µÄ¿ÕÔØ¹¦ÂÊ  £¿£¿£¿
		while(sys_stadata.m_power.Speed < noloadpw)			
		{
			if(getmilsec(ctime) > 3000)		/*Á¬Ðø4Ãë¼ì²â²»µ½À­Á¦£¬¾Í±¨¾¯  2019.11.7*/
			{
				status |= ERR_CS;
				break;
			}
			osDelay(CALUTICK);
			HALT_BREAK;
		}
		ERR_BREAK;
		
		for(i = 0;i < 20;i++)
		{
			HALT_BREAK;
			osDelay(CALUTICK);
		}

		cnt = 0;
		avepow = 0;
//		savecnt = 0;


//		CheckDir(&sys_stadata.m_high);								//¸Ä±ä¼ÆÊý·½ÏòÖµ		
		sethnum = cm2num(g_sys_para.s_sethighcm);							//½«¸ß¶È»»Ëã³É³ÝÊý
		ctime = osKernelSysTick();
		while(Enc_Get_CNT1() < sethnum)					//Éè¶¨¸ß¶È
		{
			avepow += sys_stadata.m_power.Speed;
			cnt++;
			/******************ÖÐÍ¾Áï´¸ÅÐ¶Ï**************************/
			intime = osKernelSysTick();
			while(Enc_Get_SpeedE1() < 1) 		//ÖÐÍ¾·¢ÏÖÁï´¸
			{
                Debug("hsed %d ",sys_stadata.m_high.Speed);
				if(getmilsec(intime) > 4000)			// 1Ãë³¬Ê±
				{
					status |= ERR_LS;						// ÀëºÏ¹ýËÉ,Ðèµ÷½ô
					break;
				}
				HALT_BREAK;
				osDelay(5);
			}
			if(status) break;

			/***************ÉÏÀ­³¬Ê±ÅÐ¶Ï***********/
			if(getmilsec(ctime) > (g_sys_para.s_sethighcm * 30 + 5000))   		/*ÉÏÀ­³¬Ê±ÅÐ¶Ï*/         
			{
				status |= ERR_CS;
				break;
			}
			
			if(status) break;
			osDelay(2);
		}
		
	}while(0);
	
	if(status == ERR_NONE)
	{
		if(cnt)
		  g_sys_para.s_pfull = avepow / cnt;
		else
		  g_sys_para.s_pfull = sys_stadata.m_power.Speed;
		
		sys_stadata.clihe = cm2num(g_sys_para.s_hlihe);
//		savecnt = 20;											/*ÎÞ´íÎó¼´¿ªÊ¼±£´æÊý¾Ý*/
	}
	
	IOT_FUNC_EXIT_RC(status);
}



/********************************************************
Function	: putdown
Description	: ´ò´¸³ÌÐò(µ¥´òºÍË«´ò)
Input		: delay	  ¼ä¸ôÊ±¼ä£¬µ¥´òÎª 0 £¬Ë«´ò Îª 300+
Return		: SYS_STA
Others		: ¹ÊÕÏÀàÐÍ

1. ð¤ÀëºÏ
2. ÏÂ½µ³¬Ê±
*********************************************************/
SYS_STA putdown(int32_t delay)
{
	SYS_STA status;
	uint32_t ctime;
	int32_t tmp;
	
	IOT_FUNC_ENTRY;
	G_LIHE(ACT_OFF,0);
	status = ERR_NONE;
	
	do
	{
		ctime = osKernelSysTick();
		/*¼ì²â´¸ÊÇ·ñÍÑ¿ª*/
		while((sys_stadata.m_power.Speed > epower(LALIMAXPER)) ||(Enc_Get_SpeedE1() > 20))	/*2019.11.15  Í¬Ê±¼ì²âËÙ¶ÈÁ¿*/
		{
			if(getmilsec(ctime) > 1500)				//Èç¹ûÌá´¸Ê±¼ä³¬¹ý0.8Ãë  Ö»Ö´ÐÐ1´Î
			{
				Debug("ð¤ÀëºÏ\r\t");
				G_SHACHE(ACT_ON,0);
				if(getmilsec(ctime) > 3000)			//ð¤ÀëºÏ 3Ãë³¬Ê±
					status = ERR_NC;				//ÀëºÏËÉ²»¿ª  
			}
			ERR_BREAK;
			HALT_BREAK;
			osDelay(CALUTICK);
		}
		if(status == ERR_NONE)						//ÎÞ´íÎóºóËÉÉ²³µ
			G_SHACHE(ACT_OFF,0);
		
		ERR_BREAK;
		HALT_BREAK;
		
		/*µÈ´ýÀëºÏµã  Terry 2019.7.24*/
		if(Prtop.flg)
		{
			tmp = Prtop.p_lihe;							/*ÖØÐÂ¶¨ÒåÀëºÏµãµÄ¸ß¶È*/
			Prtop.flg = 0;
		}
		else
		{
			tmp = sys_stadata.clihe;                   /*´«µÝ¹ýÀ´µÄÀëºÏµã  Õý³£µÄÊ±ºò*/
		}

		ctime = osKernelSysTick();

		Debug("lihenum %d \r\n",tmp);

		while(Enc_Get_CNT1() > tmp)			//ÀëºÏµãµÄ¸ß¶È
		{
			if(getmilsec(ctime) > (5200))					// ÏÂ½µÏÂ½µÊ±¼ä³¬¹ý5Ãë£¬ÅÐ¶¨¿¨´¸ÁË
			{
				status |= ERR_KC;							//ÏÂ½µÊ±¿¨´¸ÁË
			}
			ERR_BREAK;
			HALT_BREAK;
			osDelay(2);
		}
		
	}while(0);
	
	IOT_FUNC_EXIT_RC(status);
}



/*³õÊ¼µÄÊôÐÔ²ÎÊýÉèÖÃ
±»µ÷ÓÃ£ºdefault task,³õÊ¼»¯ÏµÍ³ÊôÐÔ²ÎÊý   2017.11.8
*/
void sysattr_init(uint16_t flg)
{
	HAL_StatusTypeDef status;
	uint32_t ID;
	
	osDelay(500);
	FB_CHECK();
	/*Ä¬ÈÏÖµ*/
	g_sys_para.s_dir = 0;
	g_sys_para.s_intval = 5;                  //ËÍÁÏÊ±¼ä  Terry 2019.6.6
	g_sys_para.s_numchi = 70;					//Ã¿È¦³ÝÊý
	
	if(flg == 0)
		g_sys_para.s_chickid = CHK_ID;
		
	g_sys_para.s_sethighcm = 300;
	g_sys_para.s_zhou = 106;
	g_sys_para.s_pnull = 50;
	g_sys_para.s_pfull = 500;
	g_sys_para.s_cmode = MOD_FREE;
	g_sys_para.s_hlihe = g_sys_para.s_sethighcm * 2 / 3;	/*Ê¹ÓÃÌ½Í·Ê±µÄÀëºÏµã cm*/
	g_sys_para.s_mode = 0;							/*0:×Ô¶¯´ò´¸ 1:È«×Ô¶¯Ç¿º»*/
    g_sys_para.s_cnt = 6;         /*º»ÍÁ´ÎÊý Terry 2019.5.21*/
	g_sys_para.s_hprot = 150;		/*Ä¬ÈÏ¸ß¶È±£»¤ÉèÖÃ Terry 2019.7.6*/
	g_sys_para.s_pset = 0;		/*Ð£ÑéÎÞÐ§ Terry 2019.7.9*/
	
	/*¼ì²éÄÚ´æÊÇ·ñÍêÕû   2017.11.8  */
	status = EE_DatasRead(DATA_ADDRESS,(uint8_t *)&ID, 4);
	osDelay(5);
	if(status == HAL_OK)
	{
		if(ID == CHK_ID)
		{
			status = EE_DatasRead(DATA_ADDRESS,(uint8_t *)&g_sys_para,sizeof(g_sys_para));
			osDelay(2);
			Debug("EEread ok\r\n");
		}
		else
			status = HAL_ERROR;
	}
	
	if(status)		//Ð£Ñé´íÎóÊ±£¬ÖØÐÂÐ´ÈëÄÚ´æ
	{
		status = EE_DatasWrite(DATA_ADDRESS,(uint8_t *)&g_sys_para,sizeof(g_sys_para));
		osDelay(2);
		
		while(status != HAL_OK)
		{
			Debug("EEread err\r\n");
			status = EE_DatasWrite(DATA_ADDRESS,(uint8_t *)&g_sys_para,sizeof(g_sys_para));
			osDelay(1000);
		}
	}
	
	Enc_Clr_TotalCnt1();

	/*ÀëºÏÉ²³µ¶¯×÷³õÊ¼»¯*/
	
	
    
    G_SHACHE(ACT_ON,0);         //³õÊ¼ÎªÀ­É²³µ        2018.12.24 Terry
    
	FB_CHECK();
    
	sys_stadata.TTCHK = 1;
	g_sys_para.s_cmode = MOD_FREE;		//ÉÏµçºó±£³Ö¿ÕÏÐÄ£Ê½
}

/********************************************************
Function	: FB_CHECK
Description	: ¼ì²â¿ØÖÆÆ÷Ó²¼þ·´À¡ÐÅºÅ
Input		: None
Return		: None
Others		: ³ÌÐòÖÐÁ¬Ðø¼ì²â
*********************************************************/
extern void clear_all(void);
void FB_CHECK(void)
{
	/*µçÔ´ÊÇ·ñÕý³£*/
	if(PG_POWEROK())
		sys_fbsta |= FB_24VOK;
	else
		sys_fbsta &= ~FB_24VOK;

	/*ÀëºÏÓÐÎÞÐÅºÅ*/
	if(PG_FBLIHE())
		sys_fbsta |= FB_LIHE;
	else
		sys_fbsta &= ~FB_LIHE;
	/*É²³µÓÐÎÞÐÅºÅ*/
	if(PG_FBSC())
		sys_fbsta |= FB_SHACHE;
	else
		sys_fbsta &= ~FB_SHACHE;
	/*¼±Í£ÓÐÎÞÐÅºÅ*/
	if(PG_STOP())
	{
		if((sys_fbsta & FB_RUN) == 0)
		{
			sys_fbsta |= FB_RUN;
			clear_all();
		}
	}
	else
	{
		sys_fbsta &= ~FB_RUN;
	}
}


