//
//  _nderr.h
//  Define error-code number
//
//  Created by duanxiuyun on 14-12-19.
//  Copyright (c) 2014 duanxiuyun. All rights reserved.
//
#ifdef _MSC_VER
#pragma  warning(disable: 4819)
#endif

//

ErrorElement(ERR_INVALID_HANDLE),	//无效句柄
ErrorElement(ERR_TIMEOUT),		//超时
ErrorElement(ERR_NOSOURCE),		//没有足够资源
ErrorElement(ERR_OPENFILE),			//不能打开文件
ErrorElement(ERR_BADTHREAD),		//线程错误
ErrorElement(ERR_LIMITED),			//限制使用
ErrorElement(ERR_USER),				//没用用户
ErrorElement(ERR_INVALID_INPUT),	//无效的输入(DATA IS TO BIG OR ZERO
ErrorElement(ERR_IO),				//IO bad SYSTEM IO BAD
ErrorElement(ERR_WOULD_BLOCK),		//需要阻塞	
ErrorElement(ERR_CLOSED),			//socket closed by peer
ErrorElement(ERR_BADPACKET),		//封包错误(too long or short)
ErrorElement(ERR_BADSOCKET),		//无效的socket
ErrorElement(ERR_READ),				//read error
ErrorElement(ERR_WRITE),			//write error	
ErrorElement(ERR_NO_PRIVILAGE),		//没有权限
ErrorElement(ERR_RESET),			//被重置
ErrorElement(ERR_VERSION),			//版本错误


ErrorElement(ERR_USER_BREAK),		//用户中断
ErrorElement(ERR_UNHANDLED_MSG),			//unknow message
ErrorElement(ERR_HOST_CRASH),			//PROGRAM crash
ErrorElement(ERR_HOST_SHUTDOWN),			//Shutdown by manual
ErrorElement(ERR_HOST_UNAVAILABLE),			//server unavaliable

ErrorElement(ERR_OS_API),			//server OS API ERROR
ErrorElement(ERR_SYSTEM),			//server OS error

ErrorElement(ERR_FILE_NOT_EXIST),			//文件不存在
ErrorElement(ERR_DIR_NOT_EXIST),			//目录不存在
ErrorElement(ERR_MYSQL),		//mysql数据库错误
ErrorElement(ERR_SWITCH_SERVER),	//切换服务器错误
ErrorElement(ERR_SCRIPT_INSTRUCT),	//脚本错误
ErrorElement(ERR_GAME_CONFIG),		//游戏配置数据错误
ErrorElement(ERR_NOT_SURPORT),		//该功能不支持
ErrorElement(ERR_IN_DEVELOPPING),	//功能开发中
ErrorElement(ERR_NEED_LOGIN),		//需要登录
ErrorElement(ERR_FORBID_LOGIN),	//禁止登陆
ErrorElement(ERR_LOGOUT),			//账号已经登出
ErrorElement(ERR_SERVER_UNREADY),	//服务器没有启动好
ErrorElement(ERR_CANNOT_ENTER_SERVER),	//不能进入该服务器
ErrorElement(ERR_ALREADY_LOGIN),	//已经登陆
ErrorElement(ERR_INUSING),			//账号使用中
ErrorElement(ERR_NAME_EXIST),		//角色名字已经存在
ErrorElement(ERR_LOGIN_OTHER_CLIENT),	//其他终端登录
ErrorElement(ERR_USERNAME),			//用户名错误
ErrorElement(ERR_NOUSER),			//用户不存在

ErrorElement(ERR_PROGRAM_OBJ_NOT_FOUND),			//程序运行对象没找到
ErrorElement(ERR_EVENT_ERROR),					//事件错误

ErrorElement(ERR_VARIANT_NOT_EXIST),	//variant not found 
ErrorElement(ERR_PARSE_STRING),			// parse input string error
ErrorElement(ERR_READ_STREAM),			//read stream error 
ErrorElement(ERR_PARAM_NOT_EXIST),		//param not found
ErrorElement(ERR_PARAM_NUMBER_ZERO),	//param is zero (input error)
ErrorElement(ERR_FUNCTION_NAME),		//function name error 
ErrorElement(ERR_FUNCTION_NOT_FOUND),	//function not found
ErrorElement(ERR_USER_DEFINE_ERROR),	//user define error
ErrorElement(ERR_NOT_INIT),	//NOT INIT

ErrorElement(ERR_UNKNOWN),				//unknowwing error