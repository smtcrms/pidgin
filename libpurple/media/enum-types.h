/* purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#ifndef _PURPLE_MEDIA_ENUM_TYPES_H_
#define _PURPLE_MEDIA_ENUM_TYPES_H_
/**
 * SECTION:enum-types
 * @section_id: libpurple-enum-types
 * @short_description: <filename>media/enum-types.h</filename>
 * @title: Enum types for Media API
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_MEDIA_CANDIDATE_TYPE   (purple_media_candidate_type_get_type())
#define PURPLE_MEDIA_TYPE_CAPS	           (purple_media_caps_get_type())
#define PURPLE_MEDIA_TYPE_INFO_TYPE	   (purple_media_info_type_get_type())
#define PURPLE_TYPE_MEDIA_NETWORK_PROTOCOL (purple_media_network_protocol_get_type())
#define PURPLE_TYPE_MEDIA_SESSION_TYPE     (purple_media_session_type_get_type())
#define PURPLE_MEDIA_TYPE_STATE            (purple_media_state_changed_get_type())

/**
 * PurpleMediaCandidateType:
 *
 * Media candidate types
 */
typedef enum {
	PURPLE_MEDIA_CANDIDATE_TYPE_HOST,
	PURPLE_MEDIA_CANDIDATE_TYPE_SRFLX,
	PURPLE_MEDIA_CANDIDATE_TYPE_PRFLX,
	PURPLE_MEDIA_CANDIDATE_TYPE_RELAY,
	PURPLE_MEDIA_CANDIDATE_TYPE_MULTICAST
} PurpleMediaCandidateType;

/**
 * PurpleMediaCaps:
 *
 * Media caps
 */
typedef enum {
	PURPLE_MEDIA_CAPS_NONE = 0,
	PURPLE_MEDIA_CAPS_AUDIO = 1,
	PURPLE_MEDIA_CAPS_AUDIO_SINGLE_DIRECTION = 1 << 1,
	PURPLE_MEDIA_CAPS_VIDEO = 1 << 2,
	PURPLE_MEDIA_CAPS_VIDEO_SINGLE_DIRECTION = 1 << 3,
	PURPLE_MEDIA_CAPS_AUDIO_VIDEO = 1 << 4,
	PURPLE_MEDIA_CAPS_MODIFY_SESSION = 1 << 5,
	PURPLE_MEDIA_CAPS_CHANGE_DIRECTION = 1 << 6
} PurpleMediaCaps;

/**
 * PurpleMediaComponentType:
 *
 * Media component types
 */
typedef enum {
	PURPLE_MEDIA_COMPONENT_NONE = 0,
	PURPLE_MEDIA_COMPONENT_RTP = 1,
	PURPLE_MEDIA_COMPONENT_RTCP = 2
} PurpleMediaComponentType;

/**
 * PurpleMediaInfoType:
 *
 * Media info types
 */
typedef enum {
	PURPLE_MEDIA_INFO_HANGUP = 0,
	PURPLE_MEDIA_INFO_ACCEPT,
	PURPLE_MEDIA_INFO_REJECT,
	PURPLE_MEDIA_INFO_MUTE,
	PURPLE_MEDIA_INFO_UNMUTE,
	PURPLE_MEDIA_INFO_PAUSE,
	PURPLE_MEDIA_INFO_UNPAUSE,
	PURPLE_MEDIA_INFO_HOLD,
	PURPLE_MEDIA_INFO_UNHOLD
} PurpleMediaInfoType;

/**
 * PurpleMediaNetworkProtocol:
 *
 * Media network protocols
 */
typedef enum {
	PURPLE_MEDIA_NETWORK_PROTOCOL_UDP,
	PURPLE_MEDIA_NETWORK_PROTOCOL_TCP,
	PURPLE_MEDIA_NETWORK_PROTOCOL_TCP_ACTIVE = PURPLE_MEDIA_NETWORK_PROTOCOL_TCP,
	PURPLE_MEDIA_NETWORK_PROTOCOL_TCP_PASSIVE,
	PURPLE_MEDIA_NETWORK_PROTOCOL_TCP_SO,
} PurpleMediaNetworkProtocol;

/**
 * PurpleMediaSessionType:
 *
 * Media session types
 */
typedef enum {
	PURPLE_MEDIA_NONE	= 0,
	PURPLE_MEDIA_RECV_AUDIO = 1 << 0,
	PURPLE_MEDIA_SEND_AUDIO = 1 << 1,
	PURPLE_MEDIA_RECV_VIDEO = 1 << 2,
	PURPLE_MEDIA_SEND_VIDEO = 1 << 3,
	PURPLE_MEDIA_RECV_APPLICATION = 1 << 4,
	PURPLE_MEDIA_SEND_APPLICATION = 1 << 5,
	PURPLE_MEDIA_AUDIO = PURPLE_MEDIA_RECV_AUDIO | PURPLE_MEDIA_SEND_AUDIO,
	PURPLE_MEDIA_VIDEO = PURPLE_MEDIA_RECV_VIDEO | PURPLE_MEDIA_SEND_VIDEO,
	PURPLE_MEDIA_APPLICATION = PURPLE_MEDIA_RECV_APPLICATION |
                                   PURPLE_MEDIA_SEND_APPLICATION
} PurpleMediaSessionType;

/**
 * PurpleMediaState:
 *
 * Media state-changed types
 */
typedef enum {
	PURPLE_MEDIA_STATE_NEW = 0,
	PURPLE_MEDIA_STATE_CONNECTED,
	PURPLE_MEDIA_STATE_END
} PurpleMediaState;

/**
 * PurpleMediaCipher:
 *
 * Media ciphers
 */
typedef enum {
	PURPLE_MEDIA_CIPHER_NULL,
	PURPLE_MEDIA_CIPHER_AES_128_ICM,
	PURPLE_MEDIA_CIPHER_AES_256_ICM
} PurpleMediaCipher;

/**
 * PurpleMediaAuthentication:
 *
 * Media message authentication algorithms
 */
typedef enum {
	PURPLE_MEDIA_AUTHENTICATION_NULL,
	PURPLE_MEDIA_AUTHENTICATION_HMAC_SHA1_32,
	PURPLE_MEDIA_AUTHENTICATION_HMAC_SHA1_80
} PurpleMediaAuthentication;

/**
 * purple_media_candidate_type_get_type:
 *
 * Gets the media candidate type's GType
 *
 * Returns: The media candidate type's GType.
 */
GType purple_media_candidate_type_get_type(void);

/**
 * purple_media_caps_get_type:
 *
 * Gets the type of the media caps flags
 *
 * Returns: The media caps flags' GType
 */
GType purple_media_caps_get_type(void);

/**
 * purple_media_info_type_get_type:
 *
 * Gets the type of the info type enum
 *
 * Returns: The info type enum's GType
 */
GType purple_media_info_type_get_type(void);

/**
 * purple_media_network_protocol_get_type:
 *
 * Gets the media network protocol's GType
 *
 * Returns: The media network protocol's GType.
 */
GType purple_media_network_protocol_get_type(void);

/**
 * purple_media_session_type_get_type:
 *
 * Gets the media session type's GType
 *
 * Returns: The media session type's GType.
 */
GType purple_media_session_type_get_type(void);

/**
 * purple_media_state_changed_get_type:
 *
 * Gets the type of the state-changed enum
 *
 * Returns: The state-changed enum's GType
 */
GType purple_media_state_changed_get_type(void);

/**
 * purple_media_cipher_get_type:
 *
 * Gets the type of the cipher enum
 *
 * Returns: The cipher enum's GType
 */
GType purple_media_cipher_get_type(void);

/**
 * purple_media_authentication_get_type:
 *
 * Gets the type of the authentication enum
 *
 * Returns: The authentication enum's GType
 */
GType purple_media_authentication_get_type(void);

G_END_DECLS

#endif  /* _PURPLE_MEDIA_ENUM_TYPES_ */
