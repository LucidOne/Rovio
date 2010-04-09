#ifndef _W99702_CLOCK_H
#define _W99702_CLOCK_H
//-----------------------------------------------------------------------------------------------

#define CLK_MENU_OP				0	/* Clock setting profile of menu operation mode */
#define CLK_MENU_SAVE			1	/* Clock setting profile of menu power saving mode */
#define CLK_MP3_PLAY_OP			2	/* Clock setting profile of mp3 play operation mode */
#define CLK_MP3_PLAY_SAVE		3	/* Clock setting profile of mp3 play power saving mode */
#define CLK_AAC_PLAY_OP			4	/* Clock setting profile of aac play operation mode */
#define CLK_AAC_PLAY_SAVE		5	/* Clock setting profile of aac play power saving mode */
#define CLK_AACPLUS_PLAY_OP		6	/* Clock setting profile of aac plus play operation mode */
#define CLK_AACPLUS_PLAY_SAVE	7	/* Clock setting profile of aac plus play power saving mode */
#define CLK_EAACPLUS_PLAY_OP	8	/* Clock setting profile of enhanced aac plus play operation mode */
#define CLK_EAACPLUS_PLAY_SAVE	9	/* Clock setting profile of enhanced aac plus play power saving mode */
#define CLK_WMA_PLAY_OP			10	/* Clock setting profile of wma play operation mode */
#define CLK_WMA_PLAY_SAVE		11	/* Clock setting profile of wma play power saving mode */
#define CLK_MP4_PLAY_OP_LO		12	/* Clock setting profile of mp4 play low operation speed mode */
#define CLK_MP4_PLAY_OP_HI		13	/* Clock setting profile of mp4 play high operation speed mode */
#define CLK_3GP_REC_OP_LO		14	/* Clock setting profile of 3GP Rec low operation speed mode */
#define CLK_3GP_REC_OP_HI		15	/* Clock setting profile of 3GP Rec high operation speed mode */
#define CLK_JPG_VIEW_OP_LO		16	/* Clock setting profile of jpeg viewer low operation speed mode */
#define CLK_JPG_VIEW_OP_HI		17	/* Clock setting profile of jpeg viewer high operation speed mode */
#define CLK_JPG_CAP_OP			18	/* Clock setting profile of JPEG Capture operation speed mode */


#define CLK_TV_MENU_OP				19	/* Clock setting profile of menu operation mode */
#define CLK_TV_MENU_SAVE			20	/* Clock setting profile of menu power saving mode */
#define CLK_TV_MP3_PLAY_OP			21	/* Clock setting profile of mp3 play operation mode */
#define CLK_TV_MP3_PLAY_SAVE		22	/* Clock setting profile of mp3 play power saving mode */
#define CLK_TV_AAC_PLAY_OP			23	/* Clock setting profile of aac play operation mode */
#define CLK_TV_AAC_PLAY_SAVE		24	/* Clock setting profile of aac play power saving mode */
#define CLK_TV_AACPLUS_PLAY_OP		25	/* Clock setting profile of aac plus play operation mode */
#define CLK_TV_AACPLUS_PLAY_SAVE	26	/* Clock setting profile of aac plus play power saving mode */
#define CLK_TV_EAACPLUS_PLAY_OP		27	/* Clock setting profile of enhanced aac plus play operation mode */
#define CLK_TV_EAACPLUS_PLAY_SAVE	28	/* Clock setting profile of enhanced aac plus play power saving mode */
#define CLK_TV_WMA_PLAY_OP			29	/* Clock setting profile of wma play operation mode */
#define CLK_TV_WMA_PLAY_SAVE		30	/* Clock setting profile of wma play power saving mode */
#define CLK_TV_MP4_PLAY_OP_LO		31	/* Clock setting profile of mp4 play low operation speed mode */
#define CLK_TV_MP4_PLAY_OP_HI		32	/* Clock setting profile of mp4 play high operation speed mode */
#define CLK_TV_3GP_REC_OP_LO		33	/* Clock setting profile of 3GP Rec low operation speed mode */
#define CLK_TV_3GP_REC_OP_HI		34	/* Clock setting profile of 3GP Rec high operation speed mode */
#define CLK_TV_JPG_VIEW_OP_LO		35	/* Clock setting profile of jpeg viewer low operation speed mode */
#define CLK_TV_JPG_VIEW_OP_HI		36	/* Clock setting profile of jpeg viewer high operation speed mode */
#define CLK_TV_JPG_CAP_OP			37	/* Clock setting profile of JPEG Capture operation speed mode */

#define CLK252_MENU_OP				(0 + 38)	/* Clock setting profile of menu operation mode */
#define CLK252_MENU_SAVE			(1 + 38)	/* Clock setting profile of menu power saving mode */
#define CLK252_MP3_PLAY_OP			(2 + 38)	/* Clock setting profile of mp3 play operation mode */
#define CLK252_MP3_PLAY_SAVE		(3 + 38)	/* Clock setting profile of mp3 play power saving mode */
#define CLK252_AAC_PLAY_OP			(4 + 38)	/* Clock setting profile of aac play operation mode */
#define CLK252_AAC_PLAY_SAVE		(5 + 38)	/* Clock setting profile of aac play power saving mode */
#define CLK252_AACPLUS_PLAY_OP		(6 + 38)	/* Clock setting profile of aac plus play operation mode */
#define CLK252_AACPLUS_PLAY_SAVE	(7 + 38)	/* Clock setting profile of aac plus play power saving mode */
#define CLK252_EAACPLUS_PLAY_OP		(8 + 38)	/* Clock setting profile of enhanced aac plus play operation mode */
#define CLK252_EAACPLUS_PLAY_SAVE	(9 + 38)	/* Clock setting profile of enhanced aac plus play power saving mode */
#define CLK252_WMA_PLAY_OP			(10+ 38)	/* Clock setting profile of wma play operation mode */
#define CLK252_WMA_PLAY_SAVE		(11+ 38)	/* Clock setting profile of wma play power saving mode */
#define CLK252_MP4_PLAY_OP_LO		(12+ 38)	/* Clock setting profile of mp4 play low operation speed mode */
#define CLK252_MP4_PLAY_OP_HI		(13+ 38)	/* Clock setting profile of mp4 play high operation speed mode */
#define CLK252_3GP_REC_OP_LO		(14+ 38)	/* Clock setting profile of 3GP Rec low operation speed mode */
#define CLK252_3GP_REC_OP_HI		(15+ 38)	/* Clock setting profile of 3GP Rec high operation speed mode */
#define CLK252_JPG_VIEW_OP_LO		(16+ 38)	/* Clock setting profile of jpeg viewer low operation speed mode */
#define CLK252_JPG_VIEW_OP_HI		(17+ 38)	/* Clock setting profile of jpeg viewer high operation speed mode */
#define CLK252_JPG_CAP_OP			(18+ 38)	/* Clock setting profile of JPEG Capture operation speed mode */


typedef struct {
	unsigned int clkcon;
	unsigned int apllcon;
	unsigned int upllcon;
	unsigned int clksel;
	unsigned int clkdiv0;
	unsigned int clkdiv1;
	unsigned int sdcon;
	unsigned int sdtime0;
} w99702_clk_t;


extern void pwr_set_clk(w99702_clk_t *cfg);
extern void pwr_set_clk_profile(int profile);
extern int pwr_get_current_clk_profile(void);


void pwr_power_saving(void);


//-----------------------------------------------------------------------------------------------
#endif