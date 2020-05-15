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

ErrorElement(ERR_INVALID_HANDLE,"invalid handle"),
ErrorElement(ERR_TIMEOUT,"time out"),
ErrorElement(ERR_NOSOURCE,"system source unavailiable"),
ErrorElement(ERR_OPENFILE,"can not open file"),
ErrorElement(ERR_BADTHREAD, "thread error") ,
ErrorElement(ERR_LIMITED,"limited by syste") ,
ErrorElement(ERR_USER,"user define error") ,
ErrorElement(ERR_INVALID_INPUT, "input data invalid"),
ErrorElement(ERR_IO, "IO error"),
ErrorElement(ERR_WOULD_BLOCK,"would block"),
ErrorElement(ERR_CLOSED, "socket closed by peer"),
ErrorElement(ERR_BADPACKET,"net packet error"),
ErrorElement(ERR_BADSOCKET, "invalid socket"),
ErrorElement(ERR_READ , "read error") ,
ErrorElement(ERR_WRITE ,"write error") ,	
ErrorElement(ERR_NO_PRIVILAGE, "no privilege "),
ErrorElement(ERR_RESET, "be reset"),
ErrorElement(ERR_VERSION, "version not match"),


ErrorElement(ERR_USER_BREAK, "break by user"),
ErrorElement(ERR_UNHANDLED_MSG, "message handle function not register"),
ErrorElement(ERR_HOST_CRASH, "PROGRAM crash"),
ErrorElement(ERR_HOST_SHUTDOWN, "remote server shutdown"),
ErrorElement(ERR_HOST_UNAVAILABLE, "remote host unavailiable"),

ErrorElement(ERR_OS_API, "server OS API ERROR"),
ErrorElement(ERR_SYSTEM, "OS system error"),

ErrorElement(ERR_FILE_NOT_EXIST, "file not exist"),
ErrorElement(ERR_DIR_NOT_EXIST, "path not exist"),
ErrorElement(ERR_MYSQL, "database error"),
ErrorElement(ERR_SWITCH_SERVER, "switch to aim server error"),
ErrorElement(ERR_SCRIPT_INSTRUCT, "error in script"),
ErrorElement(ERR_SYSTEM_CONFIG, "program config error"),
ErrorElement(ERR_NOT_SURPORT, "the sub-system not support"),
ErrorElement(ERR_IN_DEVELOPPING, "the sub-system in developping"),
ErrorElement(ERR_NEED_LOGIN, "the account need login"),
ErrorElement(ERR_FORBID_LOGIN, "the account forbid login"),
ErrorElement(ERR_LOGOUT, "already logout"),
ErrorElement(ERR_SERVER_UNREADY, "remote server not ready"),
ErrorElement(ERR_CANNOT_ENTER_SERVER, "remote server can not enter"),
ErrorElement(ERR_ALREADY_LOGIN, "account already login"),
ErrorElement(ERR_INUSING, "account in using"),
ErrorElement(ERR_NAME_EXIST, "name already exist"),
ErrorElement(ERR_LOGIN_OTHER_CLIENT, "login from other client"),
ErrorElement(ERR_USERNAME, "user name or password error"),
ErrorElement(ERR_NOUSER, "user name not exist"),

ErrorElement(ERR_PROGRAM_OBJ_NOT_FOUND, "the program internal-object not found"),
ErrorElement(ERR_EVENT_ERROR, "event error"),

ErrorElement(ERR_VARIANT_NOT_EXIST, "variant not found"),
ErrorElement(ERR_PARSE_STRING, "parse input string error"),
ErrorElement(ERR_READ_STREAM, "read stream error"),
ErrorElement(ERR_PARAM_NOT_EXIST, "param not found"),
ErrorElement(ERR_PARAM_NUMBER_ZERO, "param is zero (input error)"),
ErrorElement(ERR_FUNCTION_NAME, "function name error"),
ErrorElement(ERR_FUNCTION_NOT_FOUND, "function not found"),
ErrorElement(ERR_NOT_INIT, "program not be initialized"),
ErrorElement(ERR_USER_DEFINE_ERROR, "user define error"),
ErrorElement(ERR_BAD_GAME_OBJECT, "the input object is not match"),
ErrorElement(ERR_FEW_PARAMS, "to few params"),
ErrorElement(ERR_PARAM_TYPE_NOT_MATCH, "params not match"),
ErrorElement(ERR_PARAM_INVALID, "param invalid"),

ErrorElement(ERR_ILLEGAL_REQUEST, "illegal request"),
ErrorElement(ERR_NETMSG_FORMAT, "message format not match"),

ErrorElement(ERR_FUNCTION_CLOSED, "the sub-system be closed"),
ErrorElement(ERR_SELECT_SERVER_GROUP, "can not enter the server group"),
ErrorElement(ERR_ACCESS_DATA_NOT_EXIST, "the data not exist"),
ErrorElement(ERR_TASK_ALREADY_DONE, "the project already done"),
ErrorElement(ERR_PROGRAM_DATA_ERROR, "the program internal data error"),
ErrorElement(ERR_OPERATE_FAILED_TRY_AGAIN, "operate failed try again"),
ErrorElement(ERR_SKIP, "skip this error"),
ErrorElement(ERR_BAD_FILE, "the file is bad"),

ErrorElement(ERR_KEY_UNMATCH, "crypt key not match,need update program"),
ErrorElement(ERR_INITIAL_ERROR, "program initialed error,restart program"),
ErrorElement(ERR_UNKNOWN, "unknowwing error"),
