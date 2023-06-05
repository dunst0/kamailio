/**
 * @file ruuid_mod.c
 * @author Rick Barenthin <rick@ng-voice.com>
 * @author Rick Barenthin <dunst0@gmail.com>
 * @date 16.05.2023
 * @brief This file the implementation of a UUID module,
 *        which is using the ruuid rust library.
 */


#include "ruuid.h"

#include "../../core/sr_module.h"
#include "../../core/dprint.h"
#include "../../core/parser/parse_from.h"
#include "../../core/mod_fix.h"

MODULE_VERSION

static int mod_init(void);
static int child_init(int);
static void mod_destroy(void);

int pv_get_uuid_nil(sip_msg_t *msg, pv_param_t *param, pv_value_t *res);
int pv_get_uuid4(sip_msg_t *msg, pv_param_t *param, pv_value_t *res);
int pv_get_uuid5_sip(sip_msg_t *msg, pv_param_t *param, pv_value_t *res);
int pv_parse_uuid_name(pv_spec_p sp, str *in);

static int w_uuid_is_nil(struct sip_msg *msg, char *_uuid, char *_dummy);

/**
 * @brief The available name flags for uuids
 */
enum uuid_name_flags
{
	UUID_NIL = 1 << 0,
	UUID_VERSION_4 = 1 << 1,
	UUID_VERSION_5_SIP = 1 << 2,
	UUID_SIMPLE = 1 << 3,
	UUID_HYPHENATED = 1 << 4,
	UUID_URN = 1 << 5,
	UUID_BRACED = 1 << 6,
	UUID_SIP_FROM = 1 << 7,
	UUID_SIP_TO = 1 << 8,
};

/**
 * @brief A bitmask to test for version flags
 */
#define VERSION_FLAG_BITMASK (UUID_NIL | UUID_VERSION_4 | UUID_VERSION_5_SIP)

/**
 * @brief A bitmask to test for format flags
 */
#define FORMAT_FLAG_BITMASK \
	(UUID_SIMPLE | UUID_HYPHENATED | UUID_URN | UUID_BRACED)

/**
 * @brief Storage for uuid string
 */
static char uuid_string[RUUID_FORMATTING_MAX_LENGTH] = {0};

/* clang-format off */
static pv_export_t mod_pvs[] = {
{ {"uuid_nil", (sizeof("uuid_nil")-1)}, PVT_OTHER, pv_get_uuid_nil, 0, pv_parse_uuid_name, 0, 0, 0},
{ {"uuid4", (sizeof("uuid4")-1)}, PVT_OTHER, pv_get_uuid4, 0, pv_parse_uuid_name, 0, 0, 0},
{ {"uuid5_sip", (sizeof("uuid5_sip")-1)}, PVT_OTHER, pv_get_uuid5_sip, 0, pv_parse_uuid_name, 0, 0, 0},
{ {0, 0}, 0, 0, 0, 0, 0, 0, 0 }
};

static cmd_export_t cmds[] = {
	{"uuid_is_nil", (cmd_function)w_uuid_is_nil, 1, fixup_var_pve_str_12, 0, ANY_ROUTE},
	{0, 0, 0, 0, 0, 0}
};

static param_export_t params[] = {
	{0, 0, 0}
};

struct module_exports exports = {
	"ruuid",
	DEFAULT_DLFLAGS, /* dlopen flags */
	cmds,
	params,
	0,              /* exported RPC methods */
	mod_pvs,        /* exported pseudo-variables */
	0,              /* response function */
	mod_init,       /* module initialization function */
	child_init,     /* per child init function */
	mod_destroy     /* destroy function */
};
/* clang-format on */


/**
 * @brief Init module function
 */
static int mod_init(void)
{
	return 0;
}

/**
 * @brief Initialize async module children
 */
static int child_init(int rank)
{
	if(rank != PROC_MAIN) {
		return 0;
	}
	return 0;
}

/**
 * @brief Destroy module function
 */
static void mod_destroy(void)
{
	return;
}

/**
 * @brief Output the uuid in the requested format
 * @param uuid The UUID to format
 * @param format The format to use
 * @return Number of bytes written or -1 on error
 */
static inline int format_uuid(ruuid *uuid, int format)
{
	int ret = -1;

	memset(uuid_string, 0, RUUID_FORMATTING_MAX_LENGTH);

	switch(format & FORMAT_FLAG_BITMASK) {
		case UUID_SIMPLE:
			ret = ruuid_get_simple(
					uuid, uuid_string, RUUID_FORMATTING_MAX_LENGTH);
			break;
		case UUID_URN:
			ret = ruuid_get_urn(uuid, uuid_string, RUUID_FORMATTING_MAX_LENGTH);
			break;
		case UUID_BRACED:
			ret = ruuid_get_braced(
					uuid, uuid_string, RUUID_FORMATTING_MAX_LENGTH);
			break;
		case UUID_HYPHENATED:
		default:
			ret = ruuid_get_hyphenated(
					uuid, uuid_string, RUUID_FORMATTING_MAX_LENGTH);
	}

	return ret;
}

/**
 * @brief Create a uuid name for a version 5 uuid
 * @param msg The current handled SIP msg
 * @param name What tag to use for the uuid name creation
 * @return a pointer to a C string or NULL on error
 */
static inline char *get_uuid_name(sip_msg_t *msg, int name)
{
	to_body_t *xbody = NULL;

	if(msg == NULL) {
		return NULL;
	}

	if(name & UUID_SIP_FROM) {
		if(parse_from_header(msg) < 0) {
			LM_ERR("cannot parse From header\n");
			return NULL;
		}
		if(msg->from == NULL || get_from(msg) == NULL) {
			LM_DBG("no From header\n");
			return NULL;
		}

		xbody = get_from(msg);
	} else {
		if(parse_to_header(msg) < 0) {
			LM_ERR("cannot parse To header\n");
			return NULL;
		}
		if(msg->to == NULL || get_to(msg) == NULL) {
			LM_DBG("no To header\n");
			return NULL;
		}

		xbody = get_to(msg);
	}

	char *nameValue =
			pkg_mallocxz(msg->callid->body.len + xbody->tag_value.len + 1);
	if(!nameValue) {
		return NULL;
	}

	snprintf(nameValue, msg->callid->body.len + xbody->tag_value.len + 1,
			"%.*s%.*s", STR_FMT(&msg->callid->body),
			STR_FMT(&xbody->tag_value));
	return nameValue;
}

/**
 * @brief Generate a UUID for with the requested version
 * @param uuid_flags The uuid flags to get the version
 * @param name The name part for the Version 5 SIP UUID
 * @return The generated UUID or NULL on error
 */
static inline ruuid *generate_uuid(int uuid_flags, char *name)
{
	switch(uuid_flags & VERSION_FLAG_BITMASK) {
		case UUID_NIL:
			return ruuid_generate_nil();
		case UUID_VERSION_4:
			return ruuid_generate_version_4();
		case UUID_VERSION_5_SIP:
			if(!name) {
				LM_BUG("missing name for version 5 uuid\n");
				return NULL;
			}
			return ruuid_generate_version_5_sip(name);
		default:
			LM_BUG("not implemented uuid version\n");
	}

	return NULL;
}

/**
 * @brief Test whether the given UUID is nil
 * @param msg The current handled SIP msg
 * @param _uuid The string to check if it is the nil UUID
 * @param _dummy
 * @retval 1 if it's the nil UUID
 * @retval -1 if it's not the nil UUID
 */
static int w_uuid_is_nil(struct sip_msg *msg, char *_uuid, char *_dummy)
{
	int ret = -1;
	str uuid_str = STR_NULL;

	if(get_str_fparam(&uuid_str, msg, (fparam_t *)_uuid) < 0) {
		LM_ERR("failed to get UUID\n");
		return -1;
	}
	// uuid_str.s is NUL terminated

	ruuid *uuid = ruuid_parse(uuid_str.s);
	if(!uuid) {
		LM_BUG("Failed to parse UUID\n");
		return -1;
	}

	memset(uuid_string, 0, RUUID_FORMATTING_MAX_LENGTH);
	if (ruuid_get_hyphenated(
			uuid, uuid_string, RUUID_FORMATTING_MAX_LENGTH) > 0) {
		LM_BUG("The parsed UUI is: %s\n", uuid_string);
	}

	ret = ruuid_is_nil(uuid);
	ruuid_destroy(uuid);

	return ret > 0 ? 1 : -1;
}

/**
 * @brief Parse the name of the $uuid_nil(name), $uuid4(name) and $uuid5_sip(name)
 * @param sp The PV spec that describes the pseudo-variable
 * @param in The name of the pseudo-variable
 * @retval 0 if parsed successful
 * @retval -1 on error
 */
int pv_parse_uuid_name(pv_spec_p sp, str *in)
{
	if(sp == NULL || in == NULL || in->len < 1) {
		return -1;
	}

	switch(in->s[0]) {
		case 's':
		case 'S':
			sp->pvp.pvn.u.isname.name.n = UUID_SIMPLE;
			break;
		case 'u':
		case 'U':
			sp->pvp.pvn.u.isname.name.n = UUID_URN;
			break;
		case 'b':
		case 'B':
			sp->pvp.pvn.u.isname.name.n = UUID_BRACED;
			break;
		case 'h':
		case 'H':
		default:
			sp->pvp.pvn.u.isname.name.n = UUID_HYPHENATED;
	}

	if(in->len == 2) {
		switch(in->s[1]) {
			case 'f':
			case 'F':
				sp->pvp.pvn.u.isname.name.n |= UUID_SIP_FROM;
				break;
			case 't':
			case 'T':
				sp->pvp.pvn.u.isname.name.n |= UUID_SIP_TO;
				break;
		}
	}

	sp->pvp.pvn.type = PV_NAME_INTSTR;
	sp->pvp.pvn.u.isname.type = 0;

	return 0;
}

/**
 * @brief Generate the requested uuid into the pseudo-variable
 * @param msg The current handled SIP msg
 * @param param The parameter of the pseudo-variable
 * @param res The resulting data for the pseudo-variable
 * @retval 0 on success
 * @retval -1 on error
 */
static inline int pv_get_uuid(
		sip_msg_t *msg, pv_param_t *param, pv_value_t *res)
{
	char *name = NULL;
	int ret = -1;

	if(param->pvn.u.isname.name.n & UUID_VERSION_5_SIP) {
		name = get_uuid_name(msg, param->pvn.u.isname.name.n);
		if(!name) {
			goto cleanup;
		}
	}

	ruuid *uuid = generate_uuid(param->pvn.u.isname.name.n, name);
	if(!uuid) {
		goto cleanup;
	}
	ret = format_uuid(uuid, param->pvn.u.isname.name.n);
	ruuid_destroy(uuid);

cleanup:
	if(name) {
		pkg_free(name);
	}

	if(ret < 0) {
		return pv_get_null(msg, param, res);
	}

	return pv_get_strzval(msg, param, res, uuid_string);
}

/**
 * @brief Return the value of $uuid_nil(name)
 * @param msg The current handled SIP msg
 * @param param The parameter of the pseudo-variable
 * @param res The resulting data for the pseudo-variable
 * @retval 0 on success
 * @retval -1 on error
 */
int pv_get_uuid_nil(sip_msg_t *msg, pv_param_t *param, pv_value_t *res)
{
	if(param == NULL) {
		return -1;
	}
	param->pvn.u.isname.name.n |= UUID_NIL;
	return pv_get_uuid(msg, param, res);
}

/**
 * @brief Return the value of $uuid4(name)
 * @param msg The current handled SIP msg
 * @param param The parameter of the pseudo-variable
 * @param res The resulting data for the pseudo-variable
 * @retval 0 on success
 * @retval -1 on error
 */
int pv_get_uuid4(sip_msg_t *msg, pv_param_t *param, pv_value_t *res)
{
	if(param == NULL) {
		return -1;
	}
	param->pvn.u.isname.name.n |= UUID_VERSION_4;
	return pv_get_uuid(msg, param, res);
}

/**
 * @brief Return the value of $uuid5_sip(name)
 * @param msg The current handled SIP msg
 * @param param The parameter of the pseudo-variable
 * @param res The resulting data for the pseudo-variable
 * @retval 0 on success
 * @retval -1 on error
 */
int pv_get_uuid5_sip(sip_msg_t *msg, pv_param_t *param, pv_value_t *res)
{
	if(param == NULL) {
		return -1;
	}
	param->pvn.u.isname.name.n |= UUID_VERSION_5_SIP;
	return pv_get_uuid(msg, param, res);
}
