/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 2022, Sangoma Technologies Corporation
 *
 * George Joseph <gjoseph@sangoma.com>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */

#include "asterisk.h"
#include "asterisk/astobj2.h"
#include "asterisk/datastore.h"
#include "asterisk/res_geolocation.h"
#include "asterisk/vector.h"
#include "geoloc_private.h"

#define GEOLOC_DS_TYPE "geoloc_eprofiles"

struct ast_sorcery *geoloc_sorcery;

struct eprofiles_datastore {
	const char *id;
	AST_VECTOR(geoloc_eprofiles, struct ast_geoloc_eprofile *) eprofiles;
};

static void geoloc_datastore_free(void *obj)
{
	struct eprofiles_datastore *eds = obj;

	AST_VECTOR_RESET(&eds->eprofiles, ao2_cleanup);
	AST_VECTOR_FREE(&eds->eprofiles);
	ast_free(eds);
}

static const struct ast_datastore_info geoloc_datastore_info = {
	.type = GEOLOC_DS_TYPE,
	.destroy = geoloc_datastore_free
};

struct ast_datastore *ast_geoloc_datastore_create(const char *id)
{
	struct ast_datastore *ds = NULL;
	struct eprofiles_datastore *eds = NULL;
	int rc = 0;

	if (ast_strlen_zero(id)) {
		ast_log(LOG_ERROR, "A geoloc datastore can't be allocated with a NULL or empty id\n");
		return NULL;
	}

	ds = ast_datastore_alloc(&geoloc_datastore_info, NULL);
	if (!ds) {
		ast_log(LOG_ERROR, "Geoloc datastore '%s' couldn't be allocated\n", id);
		return NULL;
	}

	eds = ast_calloc(1, sizeof(*eds));
	if (!eds) {
		ast_datastore_free(ds);
		ast_log(LOG_ERROR, "Private structure for geoloc datastore '%s' couldn't be allocated\n", id);
		return NULL;
	}
	ds->data = eds;


	rc = AST_VECTOR_INIT(&eds->eprofiles, 2);
	if (rc != 0) {
		ast_datastore_free(ds);
		ast_log(LOG_ERROR, "Vector for geoloc datastore '%s' couldn't be initialized\n", id);
		return NULL;
	}

	return ds;
}

int ast_geoloc_datastore_add_eprofile(struct ast_datastore *ds,
	struct ast_geoloc_eprofile *eprofile)
{
	struct eprofiles_datastore *eds = NULL;
	int rc = 0;

	if (!ds || !ast_strings_equal(ds->info->type, GEOLOC_DS_TYPE) || !ds->data || !eprofile) {
		return -1;
	}

	eds = ds->data;
	rc = AST_VECTOR_APPEND(&eds->eprofiles, eprofile);
	if (rc != 0) {
		ast_log(LOG_ERROR, "Couldn't add eprofile '%s' to geoloc datastore '%s'\n", eprofile->id, eds->id);
	}

	return rc;
}

int ast_geoloc_datastore_size(struct ast_datastore *ds)
{
	struct eprofiles_datastore *eds = NULL;

	if (!ds || !ast_strings_equal(ds->info->type, GEOLOC_DS_TYPE) || !ds->data) {
		return -1;
	}

	eds = ds->data;

	return AST_VECTOR_SIZE(&eds->eprofiles);
}

struct ast_geoloc_eprofile *ast_geoloc_datastore_get_eprofile(struct ast_datastore *ds, int ix)
{
	struct eprofiles_datastore *eds = NULL;
	struct ast_geoloc_eprofile *eprofile;

	if (!ds || !ast_strings_equal(ds->info->type, GEOLOC_DS_TYPE) || !ds->data) {
		return NULL;
	}

	eds = ds->data;

	if (ix >= AST_VECTOR_SIZE(&eds->eprofiles)) {
		return NULL;
	}

	eprofile  = AST_VECTOR_GET(&eds->eprofiles, ix);
	return ao2_bump(eprofile);
}

struct ast_datastore *ast_geoloc_datastore_create_from_eprofile(
	struct ast_geoloc_eprofile *eprofile)
{
	struct ast_datastore *ds;
	int rc = 0;

	if (!eprofile) {
		return NULL;
	}

	ds = ast_geoloc_datastore_create(eprofile->id);
	if (!ds) {
		return NULL;
	}

	rc = ast_geoloc_datastore_add_eprofile(ds, eprofile);
	if (rc != 0) {
		ast_datastore_free(ds);
		ds = NULL;
	}

	return ds;
}

struct ast_datastore *ast_geoloc_datastore_create_from_profile_name(const char *profile_name)
{
	struct ast_datastore *ds = NULL;
	struct ast_geoloc_eprofile *eprofile = NULL;
	struct ast_geoloc_profile *profile = NULL;
	int rc = 0;

	if (ast_strlen_zero(profile_name)) {
		return NULL;
	}

	profile = ast_sorcery_retrieve_by_id(geoloc_sorcery, "profile", profile_name);
	if (!profile) {
		ast_log(LOG_ERROR, "A profile with the name '%s' was not found\n", profile_name);
		return NULL;
	}

	ds = ast_geoloc_datastore_create(profile_name);
	if (!ds) {
		ast_log(LOG_ERROR, "A datasatore couldn't be allocated for profile '%s'\n", profile_name);
		ao2_ref(profile, -1);
		return NULL;
	}

	eprofile = ast_geoloc_eprofile_create_from_profile(profile);
	ao2_ref(profile, -1);
	if (!eprofile) {
		ast_datastore_free(ds);
		ast_log(LOG_ERROR, "An effective profile with the name '%s' couldn't be allocated\n", profile_name);
		return NULL;
	}

	rc = ast_geoloc_datastore_add_eprofile(ds, eprofile);
	ao2_ref(eprofile, -1);
	if (rc != 0) {
		ast_datastore_free(ds);
		ds = NULL;
	}

	return ds;
}

int geoloc_channel_unload(void)
{
	if (geoloc_sorcery) {
		ast_sorcery_unref(geoloc_sorcery);
	}
	return AST_MODULE_LOAD_SUCCESS;
}

int geoloc_channel_load(void)
{
	geoloc_sorcery = geoloc_get_sorcery();
	return AST_MODULE_LOAD_SUCCESS;
}

int geoloc_channel_reload(void)
{
	return AST_MODULE_LOAD_SUCCESS;
}
