/*
 * gaim
 *
 * Some code copyright (C) 1998-1999, Mark Spencer <markster@marko.net>
 * libfaim code copyright 1998, 1999 Adam Fritzler <afritz@auk.cx>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifdef USE_OSCAR

#include <netdb.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "gaim.h"
#include "aim.h"
#include "gnome_applet_mgr.h"

static int inpa = -1;
struct aim_session_t *gaim_sess;
struct aim_conn_t    *gaim_conn;

static int gaim_parse_auth_resp  (struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_auth_server_ready(struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_server_ready     (struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_handle_redirect  (struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_parse_oncoming   (struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_parse_offgoing   (struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_parse_incoming_im(struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_parse_misses     (struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_parse_user_info  (struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_parse_motd       (struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_chatnav_info     (struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_chat_join        (struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_chat_leave       (struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_chat_info_update (struct aim_session_t *, struct command_rx_struct *, ...);
static int gaim_chat_incoming_msg(struct aim_session_t *, struct command_rx_struct *, ...);

static void oscar_callback(gpointer data, gint source,
				GdkInputCondition condition) {
	struct aim_session_t *sess = (struct aim_session_t *)data;
	struct aim_conn_t *conn = NULL;
	struct timeval tv;
	int selstat;

	tv.tv_sec = 0; tv.tv_usec = 0;
	conn = aim_select(sess, &tv, &selstat);

	switch(selstat) {
		case -1: /* error */
			signoff();
			hide_login_progress("Disconnected.");
			aim_logoff(sess);
			gdk_input_remove(inpa);
			break;
		case 0:
			gdk_input_remove(inpa);
			while (gtk_events_pending())
				gtk_main_iteration();
			inpa = gdk_input_add(gaim_conn->fd,
					GDK_INPUT_READ | GDK_INPUT_WRITE |
					GDK_INPUT_EXCEPTION,
					oscar_callback, sess);
			break;
		case 1: /* outgoing data pending */
			aim_tx_flushqueue(sess);
			break;
		case 2: /* incoming data pending */
			if (aim_get_command(sess, conn) < 0) {
				debug_print("connection error!\n");
				signoff();
				hide_login_progress("Disconnected.");
				aim_logoff(sess);
				gdk_input_remove(inpa);
			} else
				aim_rxdispatch(sess);
			break;
	}
}

int oscar_login(char *username, char *password) {
	struct aim_session_t *sess;
	struct aim_conn_t *conn;
	struct client_info_s info = {"Gaim/Faim", 3, 5, 1670, "us", "en"};
	char buf[256];

	sess = g_new0(struct aim_session_t, 1);
	aim_session_init(sess);

	sprintf(buf, "Looking up %s", FAIM_LOGIN_SERVER);
	set_login_progress(1, buf);
	conn = aim_newconn(sess, AIM_CONN_TYPE_AUTH, FAIM_LOGIN_SERVER);

	if (conn == NULL) {
		debug_print("internal connection error\n");
#ifdef USE_APPLET
		setUserState(offline);
#endif
		set_state(STATE_OFFLINE);
		hide_login_progress("Unable to login to AIM");
		return -1;
	} else if (conn->fd == -1) {
#ifdef USE_APPLET
		setUserState(offline);
#endif
		set_state(STATE_OFFLINE);
		if (conn->status & AIM_CONN_STATUS_RESOLVERR) {
			sprintf(debug_buff, "couldn't resolve host\n");
			debug_print(debug_buff);
			hide_login_progress(debug_buff);
		} else if (conn->status & AIM_CONN_STATUS_CONNERR) {
			sprintf(debug_buff, "couldn't connect to host\n");
			debug_print(debug_buff);
			hide_login_progress(debug_buff);
		}
		return -1;
	}
	g_snprintf(buf, sizeof(buf), "Signon: %s", username);
	set_login_progress(2, buf);

	aim_conn_addhandler(sess, conn, AIM_CB_FAM_SPECIAL,
				AIM_CB_SPECIAL_AUTHSUCCESS,
				gaim_parse_auth_resp, 0);
	aim_conn_addhandler(sess, conn, AIM_CB_FAM_GEN,
				AIM_CB_GEN_SERVERREADY,
				gaim_auth_server_ready, 0);
	aim_send_login(sess, conn, username, password, &info);

	gaim_conn = conn;
	inpa = gdk_input_add(conn->fd,
				GDK_INPUT_READ | GDK_INPUT_EXCEPTION,
				oscar_callback, sess);

	if (!current_user) {
		current_user = g_new0(struct aim_user, 1);
		g_snprintf(current_user->username, sizeof(current_user->username),
				DEFAULT_INFO);
		aim_users = g_list_append(aim_users, current_user);
	}
	g_snprintf(current_user->username, sizeof(current_user->username),
				"%s", username);
	g_snprintf(current_user->password, sizeof(current_user->password),
				"%s", password);
	save_prefs();

	return 0;
}

int oscar_send_im(char *name, char *msg, int away) {
	if (away)
		aim_send_im(gaim_sess, gaim_conn, name, AIM_IMFLAGS_AWAY, msg);
	else
		aim_send_im(gaim_sess, gaim_conn, name, 0, msg);
}

void oscar_close() {
#ifdef USE_APPLET
	setUserState(offline);
#endif
	set_state(STATE_OFFLINE);
	if (inpa > 0)
		gdk_input_remove(inpa);
	inpa = -1;
	aim_logoff(gaim_sess);
}

int gaim_parse_auth_resp(struct aim_session_t *sess,
			 struct command_rx_struct *command, ...) {
	struct aim_conn_t *bosconn = NULL;
	sprintf(debug_buff, "inside auth_resp (Screen name: %s)\n",
			sess->logininfo.screen_name);
	debug_print(debug_buff);

	if (sess->logininfo.errorcode) {
		sprintf(debug_buff, "Login Error Code 0x%04x\n",
				sess->logininfo.errorcode);
		debug_print(debug_buff);
		sprintf(debug_buff, "Error URL: %s\n",
				sess->logininfo.errorurl);
		debug_print(debug_buff);
#ifdef USE_APPLET
		setUserState(offline);
#endif
		set_state(STATE_OFFLINE);
		hide_login_progress("Authentication Failed");
		gdk_input_remove(inpa);
		aim_conn_close(command->conn);
		return 0;
	}


	sprintf(debug_buff, "Email: %s\n", sess->logininfo.email);
	debug_print(debug_buff);
	sprintf(debug_buff, "Closing auth connection...\n");
	debug_print(debug_buff);
	gdk_input_remove(inpa);
	aim_conn_close(command->conn);

	bosconn = aim_newconn(sess, AIM_CONN_TYPE_BOS, sess->logininfo.BOSIP);
	if (bosconn == NULL) {
#ifdef USE_APPLET
		setUserState(offline);
#endif
		set_state(STATE_OFFLINE);
		hide_login_progress("Internal Error");
		return -1;
	} else if (bosconn->status != 0) {
#ifdef USE_APPLET
		setUserState(offline);
#endif
		set_state(STATE_OFFLINE);
		hide_login_progress("Could Not Connect");
		return -1;
	}

	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_ACK, AIM_CB_ACK_ACK, NULL, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_GEN, AIM_CB_GEN_SERVERREADY, gaim_server_ready, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_GEN, AIM_CB_GEN_RATEINFO, NULL, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_GEN, AIM_CB_GEN_REDIRECT, gaim_handle_redirect, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_STS, AIM_CB_STS_SETREPORTINTERVAL, NULL, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_BUD, AIM_CB_BUD_ONCOMING, gaim_parse_oncoming, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_BUD, AIM_CB_BUD_OFFGOING, gaim_parse_offgoing, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_MSG, AIM_CB_MSG_INCOMING, gaim_parse_incoming_im, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_LOC, AIM_CB_LOC_ERROR, gaim_parse_misses, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_MSG, AIM_CB_MSG_MISSEDCALL, gaim_parse_misses, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_GEN, AIM_CB_GEN_RATECHANGE, gaim_parse_misses, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_MSG, AIM_CB_MSG_ERROR, gaim_parse_misses, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_LOC, AIM_CB_LOC_USERINFO, gaim_parse_user_info, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_CTN, AIM_CB_CTN_DEFAULT, aim_parse_unknown, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_SPECIAL, AIM_CB_SPECIAL_DEFAULT, aim_parse_unknown, 0);
	aim_conn_addhandler(sess, bosconn, AIM_CB_FAM_GEN, AIM_CB_GEN_MOTD, gaim_parse_motd, 0);

	aim_auth_sendcookie(sess, bosconn, sess->logininfo.cookie);
	inpa = gdk_input_add(bosconn->fd,
			GDK_INPUT_READ | GDK_INPUT_WRITE | GDK_INPUT_EXCEPTION,
			oscar_callback, sess);
	set_login_progress(4, "Connection established, cookie sent");
	return 1;
}

int gaim_auth_server_ready(struct aim_session_t *sess,
			   struct command_rx_struct *command, ...) {
	aim_auth_clientready(sess, command->conn);
	return 1;
}

int gaim_server_ready(struct aim_session_t *sess,
		      struct command_rx_struct *command, ...) {
	switch (command->conn->type) {
	case AIM_CONN_TYPE_BOS:
		aim_bos_reqrate(sess, command->conn);
		aim_bos_ackrateresp(sess, command->conn);
		aim_bos_setprivacyflags(sess, command->conn, 0x00000003);
		aim_bos_reqservice(sess, command->conn, AIM_CONN_TYPE_ADS);
		aim_setversions(sess, command->conn);
		aim_bos_setgroupperm(sess, command->conn, 0x1f);
		debug_print("done with BOS ServerReady\n");
		break;
	case AIM_CONN_TYPE_CHATNAV:
		aim_conn_addhandler(sess, command->conn, AIM_CB_FAM_CTN, AIM_CB_CTN_INFO, gaim_chatnav_info, 0);
		aim_bos_reqrate(sess, command->conn);
		aim_bos_ackrateresp(sess, command->conn);
		aim_chatnav_clientready(sess, command->conn);
		aim_chatnav_reqrights(sess, command->conn);
		break;
	case AIM_CONN_TYPE_CHAT:
		aim_conn_addhandler(sess, command->conn, AIM_CB_FAM_CHT, AIM_CB_CHT_USERJOIN, gaim_chat_join, 0);
		aim_conn_addhandler(sess, command->conn, AIM_CB_FAM_CHT, AIM_CB_CHT_USERLEAVE, gaim_chat_leave, 0);
		aim_conn_addhandler(sess, command->conn, AIM_CB_FAM_CHT, AIM_CB_CHT_ROOMINFOUPDATE, gaim_chat_info_update, 0);
		aim_conn_addhandler(sess, command->conn, AIM_CB_FAM_CHT, AIM_CB_CHT_INCOMINGMSG, gaim_chat_incoming_msg, 0);
		aim_bos_reqrate(sess, command->conn);
		aim_bos_ackrateresp(sess, command->conn);
		aim_chat_clientready(sess, command->conn);
		break;
	default: /* huh? */
		break;
	}
	return 1;
}

int gaim_handle_redirect(struct aim_session_t *sess,
			 struct command_rx_struct *command, ...) {
	va_list ap;
	int serviceid;
	char *ip;
	char *cookie;

	char buddies[] = "EWarmenhoven&";
	char profile[] = "Hello";

	va_start(ap, command);
	serviceid = va_arg(ap, int);
	ip        = va_arg(ap, char *);
	cookie    = va_arg(ap, char *);

	switch(serviceid) {
	case 0x0005: /* Ads */
		aim_bos_setbuddylist(sess, command->conn, buddies);
		aim_bos_setprofile(sess, command->conn, profile,
					NULL, AIM_CAPS_CHAT);

		aim_bos_clientready(sess, command->conn);

		gaim_sess = sess;
		gaim_conn = command->conn;

		debug_print("Roger that, all systems go\n");
#ifdef USE_APPLET
		make_buddy();
		if (general_options & OPT_GEN_APP_BUDDY_SHOW) {
			gnome_buddy_show();
			createOnlinePopup();
			set_applet_draw_open();
		} else {
			gnome_buddy_hide();
			set_applet_draw_closed();
		}
		setUserState(online);
		gtk_widget_hide(mainwindow);
#else
		gtk_widget_hide(mainwindow);
		show_buddy_list();
		refresh_buddy_window();
#endif
		serv_finish_login();
		if (bud_list_cache_exists())
			do_import(NULL, 0);

		break;
	case 0x7: /* Authorizer */
		{
		struct aim_conn_t *tstconn = aim_newconn(sess, AIM_CONN_TYPE_AUTH, ip);
		if (tstconn == NULL || tstconn->status >= AIM_CONN_STATUS_RESOLVERR)
			debug_print("unable to reconnect with authorizer\n");
		else
			aim_auth_sendcookie(sess, tstconn, cookie);
		}
		break;
	case 0xd: /* ChatNav */
		{
		struct aim_conn_t *tstconn = aim_newconn(sess, AIM_CONN_TYPE_CHATNAV, ip);
		if (tstconn == NULL || tstconn->status >= AIM_CONN_STATUS_RESOLVERR) {
			debug_print("unable to connect to chatnav server\n");
			return 1;
		}
		aim_conn_addhandler(sess, tstconn, 0x0001, 0x0003, gaim_server_ready, 0);
		aim_auth_sendcookie(sess, tstconn, cookie);
		}
		debug_print("chatnav: connected\n");
		break;
	case 0xe: /* Chat */
		{
		struct aim_conn_t *tstconn = aim_newconn(sess, AIM_CONN_TYPE_CHAT, ip);
		static int id = 1;
		char *roomname = va_arg(ap, char *);
		if (tstconn == NULL || tstconn->status >= AIM_CONN_STATUS_RESOLVERR) {
			debug_print("unable to connect to chat server\n");
			return 1;
		}
		aim_chat_attachname(tstconn, roomname);
		aim_conn_addhandler(sess, tstconn, 0x0001, 0x0003, gaim_server_ready, 0);
		aim_auth_sendcookie(sess, tstconn, cookie);
		serv_got_joined_chat(id++, roomname);
		}
		break;
	default: /* huh? */
		sprintf(debug_buff, "got redirect for unknown service 0x%04x\n",
				serviceid);
		debug_print(debug_buff);
		break;
	}

	va_end(ap);

	return 1;
}

int gaim_parse_oncoming(struct aim_session_t *sess,
			struct command_rx_struct *command, ...) {
	struct aim_userinfo_s *info;
	time_t time_idle;
	int type = 0;

	va_list ap;
	va_start(ap, command);
	info = va_arg(ap, struct aim_userinfo_s *);
	va_end(ap);

	if (info->class & AIM_CLASS_TRIAL)
		type |= UC_UNCONFIRMED;
	if (info->class & AIM_CLASS_AOL)
		type |= UC_AOL;
	if (info->class & AIM_CLASS_FREE)
		type |= UC_NORMAL;
	if (info->class & AIM_CLASS_AWAY)
		type |= UC_UNAVAILABLE;

	if (info->idletime) {
		time(&time_idle);
		time_idle -= info->idletime*60;
	} else
		time_idle = 0;

	serv_got_update(info->sn, 1, info->warnlevel, info->onlinesince,
			time_idle, type);

	return 1;
}

int gaim_parse_offgoing(struct aim_session_t *sess,
			struct command_rx_struct *command, ...) {
	char *sn;
	va_list ap;

	va_start(ap, command);
	sn = va_arg(ap, char *);
	va_end(ap);

	serv_got_update(sn, 0, 0, 0, 0, 0);

	return 1;
}

int gaim_parse_incoming_im(struct aim_session_t *sess,
			   struct command_rx_struct *command, ...) {
	int channel;
	va_list ap;

	va_start(ap, command);
	channel = va_arg(ap, int);

	/* channel 1: standard message */
	if (channel == 1) {
		struct aim_userinfo_s *userinfo;
		char *msg = NULL;
		u_int icbmflags = 0;
		char *tmpstr = NULL;
		u_short flag1, flag2;

		userinfo  = va_arg(ap, struct aim_userinfo_s *);
		msg       = va_arg(ap, char *);
		icbmflags = va_arg(ap, u_int);
		flag1     = va_arg(ap, u_short);
		flag2     = va_arg(ap, u_short);
		va_end(ap);

		serv_got_im(userinfo->sn, msg, icbmflags & AIM_IMFLAGS_AWAY);
	}

	return 1;
}

int gaim_parse_misses(struct aim_session_t *sess,
		      struct command_rx_struct *command, ...) {
	u_short family;
	u_short subtype;

	char buf[2048], buf2[256];
	sprintf(buf2, "Gaim - Error");
	buf[0] = 0;

	family  = aimutil_get16(command->data+0);
	subtype = aimutil_get16(command->data+2);

	switch (family) {
/*	case 0x0001:
		if (subtype == 0x000a)
			sprintf(buf, "You are sending messages too fast.");
		break;
*/	case 0x0002:
		if (subtype == 0x0001)
			sprintf(buf, "Unknown SNAC error (I'm hungry)");
		break;
	case 0x0004:
		if (subtype == 0x0001)
			sprintf(buf, "User is not online.");
		else if (subtype == 0x000a)
			sprintf(buf, "A message has been dropped.");
		break;
	}

	if (buf[0] != 0)
		do_error_dialog(buf, buf2);

	return 1;
}

int gaim_parse_user_info(struct aim_session_t *sess,
			 struct command_rx_struct *command, ...) {
	struct aim_userinfo_s *info;
	char *prof_enc = NULL, *prof = NULL;
	char buf[BUF_LONG];
	va_list ap;

	va_start(ap, command);
	info = va_arg(ap, struct aim_userinfo_s *);
	prof_enc = va_arg(ap, char *);
	prof = va_arg(ap, char *);
	va_end(ap);

	snprintf(buf, sizeof buf, "Username : <B>%s</B>\n<BR>"
				  "Warning Level : <B>%d %%</B>\n<BR>"
				  "Online Since : <B>%s</B><BR>"
				  "Idle Minutes : <B>%d</B>\n<BR><HR><BR>"
				  "%s\n",
				  info->sn,
				  info->warnlevel,
				  asctime(localtime(&info->onlinesince)),
				  info->idletime,
				  prof);
	g_show_info_text(buf);

	return 1;
}

int gaim_parse_motd(struct aim_session_t *sess,
		    struct command_rx_struct *command, ...) {
	char *msg;
	u_short id;
	va_list ap;

	va_start(ap, command);
	id  = va_arg(ap, u_short);
	msg = va_arg(ap, char *);
	va_end(ap);

	sprintf(debug_buff, "MOTD: %s\n", msg);
	debug_print(debug_buff);

	return 1;
}

int gaim_chatnav_info(struct aim_session_t *sess,
		      struct command_rx_struct *command, ...) {
	/* FIXME */
	debug_print("inside chatnav_info\n");
	return 1;
}

int gaim_chat_join(struct aim_session_t *sess,
		   struct command_rx_struct *command, ...) {
	va_list ap;
	int count, i = 0;
	struct aim_userinfo_s *info;

	GList *bcs = buddy_chats;
	struct buddy_chat *b = NULL;

	va_start(ap, command);
	count = va_arg(ap, int);
	info  = va_arg(ap, struct aim_userinfo_s *);
	va_end(ap);

	while(bcs) {
		b = (struct buddy_chat *)bcs->data;
		if (!strcasecmp(b->name, (char *)command->conn->priv))
			break;	
		bcs = bcs->next;
		b = NULL;
	}
	if (!b)
		return 1;
		
	while (i < count)
		add_chat_buddy(b, info[i++].sn);

	return 1;
}

int gaim_chat_leave(struct aim_session_t *sess,
		    struct command_rx_struct *command, ...) {
	va_list ap;
	int count, i = 0;
	struct aim_userinfo_s *info;

	GList *bcs = buddy_chats;
	struct buddy_chat *b = NULL;

	va_start(ap, command);
	count = va_arg(ap, int);
	info  = va_arg(ap, struct aim_userinfo_s *);
	va_end(ap);

	while(bcs) {
		b = (struct buddy_chat *)bcs->data;
		if (!strcasecmp(b->name, (char *)command->conn->priv))
			break;	
		bcs = bcs->next;
		b = NULL;
	}
	if (!b)
		return 1;
		
	while (i < count)
		remove_chat_buddy(b, info[i++].sn);

	return 1;
}

int gaim_chat_info_update(struct aim_session_t *sess,
			  struct command_rx_struct *command, ...) {
	/* FIXME */
	debug_print("inside chat_info_update\n");
	return 1;
}

int gaim_chat_incoming_msg(struct aim_session_t *sess,
			   struct command_rx_struct *command, ...) {
	va_list ap;
	struct aim_userinfo_s *info;
	char *msg;

	GList *bcs = buddy_chats;
	struct buddy_chat *b = NULL;

	va_start(ap, command);
	info = va_arg(ap, struct aim_userinfo_s *);
	msg  = va_arg(ap, char *);

	while(bcs) {
		b = (struct buddy_chat *)bcs->data;
		if (!strcasecmp(b->name, (char *)command->conn->priv))
			break;
		bcs = bcs->next;
		b = NULL;
	}
	if (!b)
		return;

	serv_got_chat_in(b->id, info->sn, 0, msg);

	return 1;
}

#endif /* USE_OSCAR */
