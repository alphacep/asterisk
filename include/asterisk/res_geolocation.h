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

#ifndef INCLUDE_ASTERISK_RES_GEOLOCATION_H_
#define INCLUDE_ASTERISK_RES_GEOLOCATION_H_

#include "asterisk/sorcery.h"
#include "asterisk/config.h"
#include "asterisk/xml.h"
#include "asterisk/optional_api.h"

#define AST_GEOLOC_INVALID_VALUE -1

enum ast_geoloc_pidf_element {
	AST_PIDF_ELEMENT_NONE = 0,
	AST_PIDF_ELEMENT_TUPLE,
	AST_PIDF_ELEMENT_DEVICE,
	AST_PIDF_ELEMENT_PERSON
};

enum ast_geoloc_format {
	AST_GEOLOC_FORMAT_NONE = 0,
	AST_GEOLOC_FORMAT_CIVIC_ADDRESS,
	AST_GEOLOC_FORMAT_GML,
	AST_GEOLOC_FORMAT_URI,
};

enum ast_geoloc_action {
	AST_GEOLOC_ACTION_DISCARD = 0,
	AST_GEOLOC_ACTION_APPEND,
	AST_GEOLOC_ACTION_PREPEND,
	AST_GEOLOC_ACTION_REPLACE,
};

struct ast_geoloc_location {
	SORCERY_OBJECT(details);
	AST_DECLARE_STRING_FIELDS(
		AST_STRING_FIELD(method);
	);
	enum ast_geoloc_format format;
	struct ast_variable *location_info;
};

struct ast_geoloc_profile {
	SORCERY_OBJECT(details);
	AST_DECLARE_STRING_FIELDS(
		AST_STRING_FIELD(location_reference);
	);
	enum ast_geoloc_pidf_element pidf_element;
	enum ast_geoloc_action action;
	int geolocation_routing;
	int send_location;
	struct ast_variable *location_refinement;
	struct ast_variable *location_variables;
	struct ast_variable *usage_rules;
};

struct ast_geoloc_eprofile {
	AST_DECLARE_STRING_FIELDS(
		AST_STRING_FIELD(id);
		AST_STRING_FIELD(location_reference);
		AST_STRING_FIELD(method);
	);
	enum ast_geoloc_pidf_element pidf_element;
	enum ast_geoloc_action action;
	int geolocation_routing;
	int send_location;
	enum ast_geoloc_format format;
	struct ast_variable *location_info;
	struct ast_variable *location_refinement;
	struct ast_variable *location_variables;
	struct ast_variable *effective_location;
	struct ast_variable *usage_rules;
};

/*!
 * \brief Check if res_geolocation is available
 *
 * \return 1 if available, 0 otherwise.
 */
AST_OPTIONAL_API(int, ast_geoloc_is_loaded,	(void), { return 0; });

/*!
 * \brief Retrieve a geolocation location object by id.
 *
 * \param id Location object id.
 *
 * \return Location object or NULL if not found.
 */
AST_OPTIONAL_API(struct ast_geoloc_location *, ast_geoloc_get_location,
		 (const char *id),
		 { return NULL; });

/*!
 * \brief Retrieve a geolocation profile by id.
 *
 * \param id profile id.
 *
 * \return Profile or NULL if not found.
 */
AST_OPTIONAL_API(struct ast_geoloc_profile *, ast_geoloc_get_profile,
		 (const char *id),
		 { return NULL; });

/*!
 * \brief Given an official civicAddress code, return its friendly name.
 *
 * \param code Pointer to the code to check
 *
 * \return Pointer to the friendly name or NULL if code wasn't found.
 */
const char *ast_geoloc_civicaddr_get_name_from_code(const char *code);

/*!
 * \brief Given a civicAddress friendly name, return its official code.
 *
 * \param name Pointer to the name to check
 *
 * \return Pointer to the official code or NULL if name wasn't found.
 */
const char *ast_geoloc_civicaddr_get_code_from_name(const char *name);

/*!
 * \brief Given an unknown location variable, return its official civicAddress code.
 *
 * \param variable Pointer to the name or code to check
 *
 * \return Pointer to the official code or NULL if variable wasn't a name or code.
 */
const char *ast_geoloc_civicaddr_resolve_variable(const char *variable);

enum ast_geoloc_validate_result {
	AST_GEOLOC_VALIDATE_INVALID_VALUE = -1,
	AST_GEOLOC_VALIDATE_SUCCESS = 0,
	AST_GEOLOC_VALIDATE_MISSING_TYPE,
	AST_GEOLOC_VALIDATE_INVALID_TYPE,
	AST_GEOLOC_VALIDATE_INVALID_VARNAME,
	AST_GEOLOC_VALIDATE_NOT_ENOUGH_VARNAMES,
	AST_GEOLOC_VALIDATE_TOO_MANY_VARNAMES,
};

const char *ast_geoloc_validate_result_to_str(enum ast_geoloc_validate_result result);

/*!
 * \brief Validate that the names of the variables in the list are valid codes or synonyms
 *
 * \param varlist Variable list to check.
 * \param result[OUT] Pointer to char * to receive failing item.
 *
 * \return result code.
 */
enum ast_geoloc_validate_result ast_geoloc_civicaddr_validate_varlist(
	const struct ast_variable *varlist, const char **result);

/*!
 * \brief Validate that the variables in the list represent a valid GML shape
 *
 * \param varlist Variable list to check.
 * \param result[OUT] Pointer to char * to receive failing item.
 *
 * \return result code.
 */
enum ast_geoloc_validate_result ast_geoloc_gml_validate_varlist(const struct ast_variable *varlist,
	const char **result);


/*!
 * \brief Geolocation datastore Functions
 * @{
 */

/*!
 * \brief Create a geoloc datastore from a profile name
 *
 * \param profile_name The name of the profile to use.
 *
 * \return The datastore.
 */
struct ast_datastore *ast_geoloc_datastore_create_from_profile_name(const char *profile_name);

/*!
 * \brief Create a geoloc datastore from an effective profile.
 *
 * \param eprofile The effective profile to use.
 *
 * \return The datastore.
 */
struct ast_datastore *ast_geoloc_datastore_create_from_eprofile(
	struct ast_geoloc_eprofile *eprofile);

/*!
 * \brief Create an empty geoloc datastore.
 *
 * \param id  An id to use for the datastore.
 *
 * \return The datastore.
 */
struct ast_datastore *ast_geoloc_datastore_create(const char *id);

/*!
 * \brief Retrieve a geoloc datastore's id.
 *
 * \param ds The datastore
 *
 * \return The datastore's id.
 */
const char *ast_geoloc_datastore_get_id(struct ast_datastore *ds);

/*!
 * \brief Add an eprofile to a datastore
 *
 * \param ds       The datastore
 * \param eprofile The eprofile to add.
 *
 * \return The new number of eprofiles or -1 to indicate a failure.
 */
int ast_geoloc_datastore_add_eprofile(struct ast_datastore *ds,
	struct ast_geoloc_eprofile *eprofile);

/*!
 * \brief Insert an eprofile to a datastore at the specified position
 *
 * \param ds       The datastore
 * \param eprofile The eprofile to add.
 * \param index    The position to insert at.  Existing eprofiles will
 *                 be moved up to make room.
 *
 * \return The new number of eprofiles or -1 to indicate a failure.
 */
int ast_geoloc_datastore_insert_eprofile(struct ast_datastore *ds,
	struct ast_geoloc_eprofile *eprofile, int index);

/*!
 * \brief Retrieves the number of eprofiles in the datastore
 *
 * \param ds The datastore
 *
 * \return The number of eprofiles.
 */
int ast_geoloc_datastore_size(struct ast_datastore *ds);

/*!
 * \brief Sets the inheritance flag on the datastore
 *
 * \param ds      The datastore
 * \param inherit 1 to allow the datastore to be inherited by other channels
 *                0 to prevent the datastore to be inherited by other channels
 *
 * \return 0 if successful, -1 otherwise.
 */
int ast_geoloc_datastore_set_inheritance(struct ast_datastore *ds, int inherit);

/*!
 * \brief Retrieve a specific eprofile from a datastore by index
 *
 * \param ds The datastore
 * \param ix The index
 *
 * \return The effective profile ao2 object with its reference count bumped.
 */
struct ast_geoloc_eprofile *ast_geoloc_datastore_get_eprofile(struct ast_datastore *ds, int ix);

/*!
 * \brief Delete a specific eprofile from a datastore by index
 *
 * \param ds The datastore
 * \param ix The index
 *
 * \return 0 if succesful, -1 otherwise.
 */
int ast_geoloc_datastore_delete_eprofile(struct ast_datastore *ds, int ix);

/*!
 * \brief Retrieves the geoloc datastore from a channel, if any
 *
 * \param chan Channel
 *
 * \return datastore if found, NULL otherwise.
 */
struct ast_datastore *ast_geoloc_datastore_find(struct ast_channel *chan);

/*!
 *  @}
 */

/*!
 * \brief Geolocation Effective Profile Functions
 * @{
 */

/*!
 * \brief Allocate a new, empty effective profile.
 *
 * \param name The profile's name
 *
 * \return The effective profile ao2 object.
 */
struct ast_geoloc_eprofile *ast_geoloc_eprofile_alloc(const char *name);

/*!
 * \brief Allocate a new effective profile from an existing profile.
 *
 * \param profile The profile to use.
 *
 * \return The effective profile ao2 object.
 */
struct ast_geoloc_eprofile *ast_geoloc_eprofile_create_from_profile(struct ast_geoloc_profile *profile);

/*!
 * \brief Allocate a new effective profile from an XML PIDF-LO document
 *
 * \param pidf_xmldoc       The ast_xml_doc to use.
 * \param reference_string  An identifying string to use in error messages.
 *
 * \return The effective profile ao2 object.
 */
struct ast_geoloc_eprofile *ast_geoloc_eprofile_create_from_pidf(
	struct ast_xml_doc *pidf_xmldoc, const char *reference_string);

/*!
 * \brief Allocate a new effective profile from a URI.
 *
 * \param uri               The URI to use.
 * \param reference_string  An identifying string to use in error messages.
 *
 * \return The effective profile ao2 object.
 */
struct ast_geoloc_eprofile *ast_geoloc_eprofile_create_from_uri(const char *uri,
	const char *reference_string);

/*!
 * \brief Refresh the effective profile with any changed info.
 *
 * \param eprofile The eprofile to refresh.
 *
 * \return 0 on success, any other value on error.
 */
int ast_geoloc_eprofile_refresh_location(struct ast_geoloc_eprofile *eprofile);

/*!
 *  @}
 */

#endif /* INCLUDE_ASTERISK_RES_GEOLOCATION_H_ */
