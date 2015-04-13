//
//  _nderr.h
//  Define error-code number
//
//  Created by duanxiuyun on 14-12-19.
//  Copyright (c) 2014年 duanxiuyun. All rights reserved.
//
#ifdef _MSC_VER
#pragma  warning(disable: 4819)
#endif

//

ErrorElement(NDERR_INVALID_HANDLE),	//Œﬁ–ßæ‰±˙
ErrorElement(NDERR_TIMEOUT)   ,		//≥¨ ±
ErrorElement(NDERR_NOSOURCE) ,		//√ª”–◊„πª◊ ‘¥
ErrorElement(NDERR_OPENFILE),			//≤ªƒ‹¥Úø™Œƒº˛
ErrorElement(NDERR_BADTHREAD),		//≤ªƒ‹¥Úø™œﬂ≥Ã
ErrorElement(NDERR_LIMITED),			//◊ ‘¥≥¨π˝…œœﬁ
ErrorElement(NDERR_USER),				//¥¶¿Ì”√ªß ˝æ›≥ˆ¥Ì(œ˚œ¢ªÿµ˜∫Ø ˝∑µªÿ-1
ErrorElement(NDERR_INVALID_INPUT) ,	//Œﬁ–ßµƒ ‰»Î(DATA IS TO BIG OR ZERO
ErrorElement(NDERR_IO)		,		//IO bad SYSTEM IO BAD
ErrorElement(NDERR_WUOLD_BLOCK) ,		//–Ë“™◊Ë»˚	
ErrorElement(NDERR_CLOSED),			//socket closed by peer
ErrorElement(NDERR_BADPACKET)  ,		//Õ¯¬Á ‰»Î ˝æ›¥ÌŒÛ(too long or short)
ErrorElement(NDERR_BADSOCKET) ,		//Œﬁ–ßµƒsocket
ErrorElement(NDERR_READ),				//read error
ErrorElement(NDERR_WRITE),			//write error	
ErrorElement(NDERR_NO_PRIVILAGE),		//√ª”–»®œﬁ
ErrorElement(NDERR_RESET),			//±ª÷ÿ÷√
ErrorElement(NDERR_USER_BREAK),		//”√ªß÷–∂œ(ÕÀ≥ˆ—≠ª∑)
ErrorElement(NDERR_VERSION),			//∞Ê±æ∫≈¥ÌŒÛ
ErrorElement(NDERR_UNHANDLED_MSG),			//unknow message
ErrorElement(NDERR_UNKNOW)			//unknowwing error