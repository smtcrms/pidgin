/* purpleOB
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301 USA
 */

#include "smiley-custom.h"

#include "debug.h"

#include <glib/gstdio.h>

#define SMILEYS_DEFAULT_FOLDER "custom_smiley"
#define SMILEYS_INDEX_FILE "smileys.xml"

static gchar *smileys_dir;
static gchar *smileys_index;

static PurpleSmileyList *smileys_list;
static gboolean disable_write = FALSE;

/*******************************************************************************
 * XML storage
 ******************************************************************************/

static void
purple_smiley_custom_load(void)
{
	PurpleXmlNode *root_node, *profile_node, *smileyset_node, *smiley_node;
	int got_smileys = 0;

	if (!g_file_test(smileys_index, G_FILE_TEST_EXISTS))
		return;

	root_node = purple_xmlnode_from_file(g_path_get_dirname(smileys_index),
		g_path_get_basename(smileys_index),
		"custom smileys index", "smiley-custom");
	g_return_if_fail(root_node);

	profile_node = purple_xmlnode_get_child(root_node, "profile");
	if (!profile_node) {
		purple_xmlnode_free(root_node);
		g_return_if_fail(profile_node);
	}

	smileyset_node = purple_xmlnode_get_child(profile_node, "smiley_set");
	if (!smileyset_node) {
		purple_xmlnode_free(root_node);
		g_return_if_fail(smileyset_node);
	}

	smiley_node = purple_xmlnode_get_child(smileyset_node, "smiley");
	while (smiley_node) {
		const gchar *shortcut, *file_name;

		shortcut = purple_xmlnode_get_attrib(smiley_node, "shortcut");
		file_name = purple_xmlnode_get_attrib(smiley_node, "filename");

		if (shortcut && file_name) {
			PurpleSmiley *smiley;
			gchar *file_path = g_build_filename(
				smileys_dir, file_name, NULL);

			got_smileys++;
			smiley = purple_smiley_new(shortcut, file_path);
			g_free(file_path);

			purple_smiley_list_add(smileys_list, smiley);
			g_object_unref(smiley);
		}

		smiley_node = purple_xmlnode_get_next_twin(smiley_node);
	}

	purple_xmlnode_free(root_node);

	purple_debug_info("smiley-custom", "Loaded %d custom smiley(s)",
		got_smileys);
}

static PurpleXmlNode *
smileys_to_xmlnode(void)
{
	PurpleXmlNode *root_node, *profile_node, *smileyset_node;
	GList *smileys, *it;

	root_node = purple_xmlnode_new("smileys");
	purple_xmlnode_set_attrib(root_node, "version", "1.0");

	profile_node = purple_xmlnode_new("profile");
	purple_xmlnode_insert_child(root_node, profile_node);
	purple_xmlnode_set_attrib(profile_node, "name", "Default");

	smileyset_node = purple_xmlnode_new("smiley_set");
	purple_xmlnode_insert_child(profile_node, smileyset_node);

	smileys = purple_smiley_list_get_unique(smileys_list);

	for (it = smileys; it; it = g_list_next(it)) {
		PurpleSmiley *smiley = PURPLE_SMILEY(it->data);
		PurpleXmlNode *smiley_node;

		smiley_node = purple_xmlnode_new("smiley");
		purple_xmlnode_set_attrib(smiley_node, "shortcut",
			purple_smiley_get_shortcut(smiley));
		purple_xmlnode_set_attrib(smiley_node, "filename",
			g_path_get_basename(purple_smiley_get_path(smiley)));
	}

	return root_node;
}

static void
purple_smiley_custom_save(void)
{
	PurpleXmlNode *root_node;
	gchar *data;
	GError *error = NULL;

	if (disable_write)
		return;

	root_node = smileys_to_xmlnode();
	g_return_if_fail(root_node != NULL);

	data = purple_xmlnode_to_formatted_str(root_node, NULL);
	g_return_if_fail(data != NULL);

	g_file_set_contents(smileys_index, data, -1, &error);

	g_free(data);
	purple_xmlnode_free(root_node);

	if (error) {
		purple_debug_error("smiley-custom",
			"Error writting custom smileys xml file");
	}
}

static gchar *
purple_smiley_custom_img_checksum(PurpleStoredImage *img)
{
	g_return_val_if_fail(PURPLE_IS_STORED_IMAGE(img), NULL);

	return g_compute_checksum_for_data(G_CHECKSUM_SHA1,
		purple_imgstore_get_data(img),
		purple_imgstore_get_size(img));
}


/*******************************************************************************
 * API implementation
 ******************************************************************************/

PurpleSmiley *
purple_smiley_custom_add(PurpleStoredImage *img, const gchar *shortcut)
{
	PurpleSmiley *existing_smiley, *smiley;
	gchar *checksum, *file_path;
	gchar file_name[256];
	const gchar *file_ext;
	GError *error = NULL;

	g_return_val_if_fail(PURPLE_IS_STORED_IMAGE(img), NULL);

	existing_smiley = purple_smiley_list_get_by_shortcut(
		smileys_list, shortcut);

	g_object_ref(img);

	if (existing_smiley) {
		disable_write = TRUE;
		purple_smiley_custom_remove(existing_smiley);
		disable_write = FALSE;
	}

	checksum = purple_smiley_custom_img_checksum(img);
	file_ext = purple_imgstore_get_extension(img);
	if (file_ext == NULL || g_strcmp0("icon", file_ext) == 0) {
		purple_debug_warning("smiley-custom", "Invalid image type");
		return NULL;
	}

	g_snprintf(file_name, sizeof(file_name), "%s.%s", checksum, file_ext);
	file_path = g_build_filename(smileys_dir, file_name, NULL);

	g_file_set_contents(file_path, purple_imgstore_get_data(img),
		purple_imgstore_get_size(img), &error);

	g_free(checksum);
	g_object_unref(img);

	if (error) {
		purple_debug_error("smiley-custom", "Failed writing smiley "
			"file %s: %s", file_path, error->message);
		g_free(file_path);
		return NULL;
	}

	smiley = purple_smiley_new(shortcut, file_path);
	g_free(file_path);
	purple_smiley_list_add(smileys_list, smiley);
	g_object_unref(smiley);

	purple_smiley_custom_save();

	return smiley;
}

void
purple_smiley_custom_remove(PurpleSmiley *smiley)
{
	PurpleSmiley *existing_smiley;
	const gchar *smiley_shortcut;

	g_return_if_fail(PURPLE_IS_SMILEY(smiley));

	smiley_shortcut = purple_smiley_get_shortcut(smiley);
	existing_smiley = purple_smiley_list_get_by_shortcut(
		smileys_list, smiley_shortcut);
	if (existing_smiley == NULL)
		return;
	if (existing_smiley != smiley) {
		purple_debug_warning("smiley-custom", "Smiley is not in store");
		return;
	}

	g_unlink(purple_smiley_get_path(smiley));
	purple_smiley_list_remove(smileys_list, smiley);
	purple_smiley_custom_save();
}

PurpleSmileyList *
purple_smiley_custom_get_list(void)
{
	return smileys_list;
}


/*******************************************************************************
 * Custom smiley subsystem
 ******************************************************************************/

void
purple_smiley_custom_init(void)
{
	smileys_dir = g_build_filename(purple_user_dir(),
		SMILEYS_DEFAULT_FOLDER, NULL);
	smileys_index = g_build_filename(purple_user_dir(),
		SMILEYS_INDEX_FILE, NULL);
	smileys_list = purple_smiley_list_new();

	purple_smiley_custom_load();
}

void
purple_smiley_custom_uninit(void)
{
	g_free(smileys_dir);
	g_free(smileys_index);
	g_object_unref(smileys_list);
}