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

ErrorElement(ERR_INVALID_HANDLE,"无效句柄"),
ErrorElement(ERR_TIMEOUT,"超时"),
ErrorElement(ERR_NOSOURCE,"系统资源不足"),
ErrorElement(ERR_OPENFILE,"不能打开文件"),
ErrorElement(ERR_BADTHREAD, "线程错误") ,
ErrorElement(ERR_LIMITED,"超过系统限制") ,
ErrorElement(ERR_USER,"用户定义错误") ,
ErrorElement(ERR_INVALID_INPUT, "输入错误"),
ErrorElement(ERR_IO, "IO错误或者是系统错误"),
ErrorElement(ERR_WOULD_BLOCK,"需要阻塞"),
ErrorElement(ERR_CLOSED, "socket closed by peer"),
ErrorElement(ERR_BADPACKET,"网络封包错误"),
ErrorElement(ERR_BADSOCKET, "无效的socket,或者已经被关闭"),
ErrorElement(ERR_READ , "从文件或者网络读取数据错误") ,
ErrorElement(ERR_WRITE ,"往socket写入数据错误") ,	
ErrorElement(ERR_NO_PRIVILAGE, "没有权限"),
ErrorElement(ERR_RESET, "被重置"),
ErrorElement(ERR_VERSION, "版本错误"),


ErrorElement(ERR_USER_BREAK, "运用层中断"),
ErrorElement(ERR_UNHANDLED_MSG, "该网络消息没有对应的处理行数"),
ErrorElement(ERR_HOST_CRASH, "PROGRAM crash"),
ErrorElement(ERR_HOST_SHUTDOWN, "服务器关闭"),
ErrorElement(ERR_HOST_UNAVAILABLE, "无法连接服务器"),

ErrorElement(ERR_OS_API, "server OS API ERROR"),
ErrorElement(ERR_SYSTEM, "系统错误"),

ErrorElement(ERR_FILE_NOT_EXIST, "文件不存在"),
ErrorElement(ERR_DIR_NOT_EXIST, "目录不存在"),
ErrorElement(ERR_MYSQL, "mysql数据库错误"),
ErrorElement(ERR_SWITCH_SERVER, "切换服务器错误"),
ErrorElement(ERR_SCRIPT_INSTRUCT, "脚本错误"),
ErrorElement(ERR_SYSTEM_CONFIG, "程序配置数据错误"),
ErrorElement(ERR_NOT_SURPORT, "该功能不支持"),
ErrorElement(ERR_IN_DEVELOPPING, "功能开发中"),
ErrorElement(ERR_NEED_LOGIN, "需要登录"),
ErrorElement(ERR_FORBID_LOGIN, "该账号禁止登陆"),
ErrorElement(ERR_LOGOUT, "账号已经登出"),
ErrorElement(ERR_SERVER_UNREADY, "服务器没有启动好"),
ErrorElement(ERR_CANNOT_ENTER_SERVER, "目前无法进入该服务器"),
ErrorElement(ERR_ALREADY_LOGIN, "账号已经登陆"),
ErrorElement(ERR_INUSING, "账号使用中"),
ErrorElement(ERR_NAME_EXIST, "角色名字已经存在"),
ErrorElement(ERR_LOGIN_OTHER_CLIENT, "其他终端登录"),
ErrorElement(ERR_USERNAME, "用户名或者密码错误"),
ErrorElement(ERR_NOUSER, "用户不存在"),

ErrorElement(ERR_PROGRAM_OBJ_NOT_FOUND, "程序运行时内部对象没找到"),
ErrorElement(ERR_EVENT_ERROR, "事件错误"),

ErrorElement(ERR_VARIANT_NOT_EXIST, "variant not found"),
ErrorElement(ERR_PARSE_STRING, "parse input string error"),
ErrorElement(ERR_READ_STREAM, "read stream error"),
ErrorElement(ERR_PARAM_NOT_EXIST, "param not found"),
ErrorElement(ERR_PARAM_NUMBER_ZERO, "param is zero (input error)"),
ErrorElement(ERR_FUNCTION_NAME, "function name error"),
ErrorElement(ERR_FUNCTION_NOT_FOUND, "function not found"),
ErrorElement(ERR_NOT_INIT, "程序没有初始化"),
ErrorElement(ERR_USER_DEFINE_ERROR, "user define error"),

ErrorElement(ERR_UNKNOWN, "unknowwing error"),
