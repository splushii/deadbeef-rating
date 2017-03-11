#include <deadbeef.h>
#include <string.h>
#include <stdlib.h>

// #define trace(...) { fprintf(stderr, __VA_ARGS__); }

static DB_misc_t plugin;
static DB_functions_t *deadbeef;

DB_plugin_t *
rating_load(DB_functions_t *api)
{
    deadbeef = api;
    return DB_PLUGIN(&plugin);
}

static int
rating_start(void)
{
    return 0;
}

static int
rating_stop(void)
{
    return 0;
}

// Removes rating tag when rating == -1, otherwise sets specified rating.
static int
rating_action_rate_helper(DB_plugin_action_t *action, int ctx, int rating)
{
    DB_playItem_t *it = NULL;
    ddb_playlist_t *plt = NULL;
    int num = 0;
    if (ctx == DDB_ACTION_CTX_SELECTION) {
        plt = deadbeef->plt_get_curr();
        if (plt) {
            num = deadbeef->plt_getselcount(plt);
            it = deadbeef->plt_get_first(plt, PL_MAIN);
            while (it) {
                if (deadbeef->pl_is_selected(it)) {
                    break;
                }
                DB_playItem_t *next = deadbeef->pl_get_next(it, PL_MAIN);
                deadbeef->pl_item_unref(it);
                it = next;
            }
            deadbeef->plt_unref(plt);
        }
    } else if (ctx == DDB_ACTION_CTX_NOWPLAYING) {
        it = deadbeef->streamer_get_playing_track();
        plt = deadbeef->plt_get_curr();
        num = 1;
    }
    if (!it || !plt || num < 1) {
        goto out;
    }
    int count = 0;
    while (it) {
        if (deadbeef->pl_is_selected(it) || ctx == DDB_ACTION_CTX_NOWPLAYING) {
            if (rating == -1) {
                deadbeef->pl_delete_meta(it, "rating");
            } else {
                deadbeef->pl_set_meta_int(it, "rating", rating);
            }
            deadbeef->pl_lock();
            const char *dec = deadbeef->pl_find_meta_raw(it, ":DECODER");
            char decoder_id[100];
            if (dec) {
                strncpy(decoder_id, dec, sizeof(decoder_id));
            }
            int match = it && dec;
            deadbeef->pl_unlock();
            if (match) {
                int is_subtrack = deadbeef->pl_get_item_flags(it)
		    & DDB_IS_SUBTRACK;
                if (is_subtrack) {
                    continue;
                }
                DB_decoder_t *dec = NULL;
                DB_decoder_t **decoders = deadbeef->plug_get_decoder_list();
                for (int i = 0; decoders[i]; i++) {
                    if (!strcmp(decoders[i]->plugin.id, decoder_id)) {
                        dec = decoders[i];
                        if (dec->write_metadata) {
                            dec->write_metadata(it);
                        }
                        break;
                    }
                }
            }
            count++;
            if (count >= num) {
                break;
            }
        }
        DB_playItem_t *next = deadbeef->pl_get_next(it, PL_MAIN);
        deadbeef->pl_item_unref(it);
        it = next;
    }
    if (plt) {
        deadbeef->plt_modified(plt);
    }
out:
    if (it) {
        deadbeef->pl_item_unref(it);
    }
    return 0;
}

static int
rating_action_rate0(DB_plugin_action_t *action, int ctx)
{
    return rating_action_rate_helper(action, ctx, 0);
}

static int
rating_action_rate1(DB_plugin_action_t *action, int ctx)
{
    return rating_action_rate_helper(action, ctx, 1);
}

static int
rating_action_rate2(DB_plugin_action_t *action, int ctx)
{
    return rating_action_rate_helper(action, ctx, 2);
}

static int
rating_action_rate3(DB_plugin_action_t *action, int ctx)
{
    return rating_action_rate_helper(action, ctx, 3);
}

static int
rating_action_rate4(DB_plugin_action_t *action, int ctx)
{
    return rating_action_rate_helper(action, ctx, 4);
}

static int
rating_action_rate5(DB_plugin_action_t *action, int ctx)
{
    return rating_action_rate_helper(action, ctx, 5);
}

static int
rating_action_remove(DB_plugin_action_t *action, int ctx)
{
    return rating_action_rate_helper(action, ctx, -1);
}

static DB_plugin_action_t remove_rating_action = {
    .title = "Remove rating tag",
    .name = "rating_remove",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS
    | DB_ACTION_ADD_MENU,
    .callback2 = rating_action_remove,
    .next = NULL
};

static DB_plugin_action_t rate5_action = {
    .title = "Rate 5",
    .name = "rating_rate5",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS
    | DB_ACTION_ADD_MENU,
    .callback2 = rating_action_rate5,
    .next = &remove_rating_action
};

static DB_plugin_action_t rate4_action = {
    .title = "Rate 4",
    .name = "rating_rate4",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS
    | DB_ACTION_ADD_MENU,
    .callback2 = rating_action_rate4,
    .next = &rate5_action
};

static DB_plugin_action_t rate3_action = {
    .title = "Rate 3",
    .name = "rating_rate3",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS
    | DB_ACTION_ADD_MENU,
    .callback2 = rating_action_rate3,
    .next = &rate4_action
};

static DB_plugin_action_t rate2_action = {
    .title = "Rate 2",
    .name = "rating_rate2",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS
    | DB_ACTION_ADD_MENU,
    .callback2 = rating_action_rate2,
    .next = &rate3_action
};

static DB_plugin_action_t rate1_action = {
    .title = "Rate 1",
    .name = "rating_rate1",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS
    | DB_ACTION_ADD_MENU,
    .callback2 = rating_action_rate1,
    .next = &rate2_action
};

static DB_plugin_action_t rate0_action = {
    .title = "Rate 0",
    .name = "rating_rate0",
    .flags = DB_ACTION_SINGLE_TRACK | DB_ACTION_MULTIPLE_TRACKS
    | DB_ACTION_ADD_MENU,
    .callback2 = rating_action_rate0,
    .next = &rate1_action
};

static DB_plugin_action_t *
rating_get_actions(DB_playItem_t *it)
{
    return &rate0_action;
}

static DB_misc_t plugin = {
    .plugin.api_vmajor = 1,
    .plugin.api_vminor = 5,
    .plugin.version_major = 1,
    .plugin.version_minor = 0,
    .plugin.type = DB_PLUGIN_MISC,
    .plugin.name = "rating",
    .plugin.descr = "Enables commands to rate song(s) by editing the metadata tag rating.",
    .plugin.copyright =
    "Rating plugin for DeaDBeeF Player\n"
    "Author: Christian Hernvall\n"
    "\n"
    "This program is free software; you can redistribute it and/or\n"
    "modify it under the terms of the GNU General Public License\n"
    "as published by the Free Software Foundation; either version 2\n"
    "of the License, or (at your option) any later version.\n"
    "\n"
    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with this program; if not, write to the Free Software\n"
    "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"
    ,
    .plugin.website = "http://deadbeef.sf.net",
    .plugin.start = rating_start,
    .plugin.stop = rating_stop,
    .plugin.get_actions = rating_get_actions,
};
