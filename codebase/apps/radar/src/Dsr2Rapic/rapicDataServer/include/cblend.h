/*
 * cblend.h
 */

#ifndef __CBLEND_H__
#define __CBLEND_H__

#include "SatRGBModeSimpleWindowClass.h"
#include "BlendFnSimpleWindowClass.h"
#include "bool.h"

/*
 * This class contains paramateres for blend function
 * of the form
 * 
 * ((((gaina*a)+ofsa) oper ((gainb*b)+ofsb)) * gainc) + ofsc
 * 
 * where
 * 
 * a=channel1 value, gaina & ofsa gain/offset values for ch1
 * b=channel2 value, gainb & ofsb gain/offset values for ch2
 * oper from mult, div, add, sub
 * 
 * gainc and ofsc may be used for scaling result to 
 * typical 0-255 range
 * 
 */
 
enum CBlendOperator {OP_ADD, OP_SUB, OP_MULT, OP_DIV};
enum CBlendMode {BLENDMODE_2CH_ARITH, BLENDMODE_3CH_RGB};

class CBlend : public VkCallbackObject {
public:
	CBlendMode mode;
	float	gaina, ofsa, 
			gainb, ofsb, 
			gainc, ofsc;
	CBlendOperator Op;
	// Normal Blend operation
	// ch1 & ch2 used for arithmetic blend, with gain/ofs a & b per ch
	// and gain/ofs c for overall gain/ofs scaling
	// RGB blend operation
	// ch1, ch2, ch3 used for 3 channel RGB output.
	// can use gain/ofs abc for scaling 3 channel RGB
	int		ch1, ch2, ch3;		
	bool	shouldAutoSetGainOfs;
	BlendFnSimpleWindowClass *blend_editor;
	SatRGBModeSimpleWindowClass *rgb_editor;
	bool	valueChanged;
	static const char *const blendChangedCallback;
	static const char *const blendApplyImgCallback;
	static const char *const blendApplySeqCallback;
	
	/*
	 * Callbacks called by editors
	 * These callCallbackss in turn to notify of
	 * changes and apply actions
	 */
	void	blendChanged(VkCallbackObject *, void *, void *);
	void	blendApplyImg(VkCallbackObject *, void *, void *);
	void	blendApplySeq(VkCallbackObject *, void *, void *);
	
		
	
	/*
	 * BLENDMODE_2CH_ARITH mode constructor
	 */
	CBlend(int ch1 = 1, int ch2 = 2,  
			float GAINA = 1.0, float OFSA = 0.0, 
			float GAINB = 1.0, float OFSB = 0.0,
			float GAINC = 0.5, float OFSC = 0.0, 
			CBlendOperator OP = OP_ADD);
	/*
	 * BLENDMODE_3CH_RGB mode constructor
	 */
	CBlend(int ch1, int ch2, int ch3, 
			float GAINA = 1.0, float OFSA = 0.0, 
			float GAINB = 1.0, float OFSB = 0.0,
			float GAINC = 1.0, float OFSC = 0.0);
	CBlend(CBlend *setblend);
	~CBlend();
	virtual void Set(int ch1 = 1, int ch2 = 2, 
			float gaina = 1.0, float ofsa = 0.0, 
			float gainb = 1.0, float ofsb = 0.0,
			float gainc = 0.5, float ofsc = 0.0, 
			CBlendOperator Op = OP_ADD);
	virtual void Set(int ch1, int ch2, int ch3, 
			float gaina = 1.0, float ofsa = 0.0, 
			float gainb = 1.0, float ofsb = 0.0,
			float gainc = 0.5, float ofsc = 0.0);
	
	virtual void Set(CBlend *setblend);
			
	virtual bool IsSame(CBlend *setblend);

	unsigned char Blend(int vala, int valb);
	float Blend(float vala, float valb, bool applyscaling = true);
	
//	float BlendtoFloat(int vala, int valb);
	void BlendLines(unsigned char *line_a, unsigned char *line_b, 
		int linelength, unsigned char *line_c = 0);
	
	void	OpenBlendEditor();
	void	CloseBlendEditor();
	void	BlendEditorDeleted(VkCallbackObject *, void *, void *);		// called by editor if it is closed by user
	void	BlendEditorChanged(VkCallbackObject *, void *, void *);
	void	BlendEditorApplyImg(VkCallbackObject *, void *, void *);
	void	BlendEditorApplySeq(VkCallbackObject *, void *, void *);
	void	OpenRGBEditor();
	void	CloseRGBEditor();
	void	RGBEditorDeleted(VkCallbackObject *, void *, void *);		// called by editor if it is closed by user
	void	RGBEditorChanged(VkCallbackObject *, void *, void *);
	void	RGBEditorApplyImg(VkCallbackObject *, void *, void *);
	void	RGBEditorApplySeq(VkCallbackObject *, void *, void *);
	void	readFile(int fildes);
	void	writeFile(int fildes);
	void	CalcAutoGainOfs();	
};

inline unsigned char CBlend::Blend(int vala, int valb)
{
	float proda, prodb, prodc;
	
	proda = (gaina * float(vala)) + ofsa;
	prodb = (gainb * float(valb)) + ofsb;
	switch (Op) {
		case OP_ADD:
			prodc = ((proda + prodb) * gainc) + ofsc;
			break;
		case OP_SUB:
			prodc = ((proda - prodb) * gainc) + ofsc;
			break;
		case OP_MULT:
			prodc = ((proda * prodb) * gainc) + ofsc;
			break;
		case OP_DIV:
			if (prodb != 0.0)
				prodc = ((proda / prodb) * gainc) + ofsc;
			else
				prodc = 255;
			break;
		}
	if (prodc < 0)
		prodc = 0;
	if (prodc > 255)
		prodc = 255;
	return (unsigned char)(prodc);
};

inline float CBlend::Blend(float vala, float valb, bool applyscaling)
{
	float proda, prodb, prodc, 
			tempgainc = gainc, tempofsc = ofsc;
	
	proda = (gaina * vala) + ofsa;
	prodb = (gainb * valb) + ofsb;
	if (!applyscaling)
	{
		tempgainc = 1.0;
		tempofsc = 0.0;
	}
	switch (Op) {
		case OP_ADD:
			prodc = ((proda + prodb) * tempgainc) + tempofsc;
			break;
		case OP_SUB:
			prodc = ((proda - prodb) * tempgainc) + tempofsc;
			break;
		case OP_MULT:
			prodc = ((proda * prodb) * tempgainc) + tempofsc;
			break;
		case OP_DIV:
			if (prodb != 0.0)
				prodc = ((proda / prodb) * tempgainc) + tempofsc;
			else
				prodc = 255;
			break;
		}
	return prodc;
};

/*
inline void CBlend::BlendLines(unsigned char *line_a, unsigned char *line_b, 
	int linelength, unsigned char *line_c)
{
	unsigned char *dest = 0;
	
	if (line_c)
		dest = line_c;
	else
		dest = line_a;
		
	while (linelength--)
	{
		*(dest++) = Blend(*(line_a++), *(line_b++));
	}
};
*/

#endif

