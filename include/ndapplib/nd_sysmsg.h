/* file nd_sysmsg.cpp
* message handler of system for ndnet
*
* create by duan
*
*/

#ifndef _ND_SYS_MSG_H_
#define _ND_SYS_MSG_H_

MSG_ENTRY_DECLARE(nd_echo_handler);
MSG_ENTRY_DECLARE(nd_transfer_to_msgproc);
MSG_ENTRY_DECLARE(nd_transfer_to_client);

MSG_ENTRY_DECLARE(nd_get_message_name_handler);
MSG_ENTRY_DECLARE(nd_get_app_ver_handler);

MSG_ENTRY_DECLARE(nd_get_server_rlimit);
MSG_ENTRY_DECLARE(nd_set_netmsg_log);
MSG_ENTRY_DECLARE(nd_set_netmsg_print);

MSG_ENTRY_DECLARE(nd_quicken_inst_time);
MSG_ENTRY_DECLARE(nd_get_sys_time);
MSG_ENTRY_DECLARE(nd_get_game_time);
MSG_ENTRY_DECLARE(app_statics_begin);
MSG_ENTRY_DECLARE(app_statics_end);
MSG_ENTRY_DECLARE(default_close_handler);

MSG_ENTRY_DECLARE(nd_open_log_handler);

MSG_ENTRY_DECLARE(nd_redirect_msglog_to_me);
MSG_ENTRY_DECLARE(nd_close_exist_msg_handler);

#endif

